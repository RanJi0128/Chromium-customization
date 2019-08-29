/*
    parsehtml.c

    To compile with GCC, execute:
    gcc -std=c99 -Wall -pedantic -DTEST parsehtml.c avl.c -o parsehtml

    To run: ./parsehtml input.html

    Provided by Rhyscitlema
    @ http://rhyscitlema.com

    USE AT YOUR OWN RISK!
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include "parsehtml.h"


#ifdef LIB_STD
// used only for debugging
#include <_malloc.h>
#else
#include <malloc.h>
#define _malloc malloc
#define _realloc realloc
#define _free free
#define memory_alloc(str)
#define memory_freed(str)
#define memory_print()
#endif

#define REMOVECONST(type, rvalue) (type)(rvalue)


#ifdef TEST

bool file_to_array (const  char* filename,  char** filecontent_ptr, size_t *contentSize_ptr)
{
    size_t size;
    char* filecontent;

    FILE* file = fopen (filename, "rb");
    if(file==NULL) return false;

    fseek (file, 0, SEEK_END);
    size = ftell (file);
    fseek (file, 0, SEEK_SET);
    if(size==0x7FFFFFFF) { fclose(file); return false; }

    if(contentSize_ptr) *contentSize_ptr = size;
    if(filecontent_ptr)
    {
        filecontent = (char*) _realloc (*filecontent_ptr, size+1);
        *filecontent_ptr = filecontent;

        if(fread (filecontent, 1, size, file) != size)
        {
            _free(filecontent);
            *filecontent_ptr=NULL;
            fclose(file);
            return false;
        }
        filecontent[size]=0;
    }
    fclose(file);
    return true;
}


int main (int argc, char** argv)
{
    if(argc != 2) printf("Usage: %s input.html\n", argv[0]);
    else
    {   char* filecontent = NULL;
        if(!file_to_array(argv[1], &filecontent, NULL))
            printf("Error: file '%s' could not be opened.\n", argv[1]);
        else
        {
            DOMNode* document = parsehtml(filecontent, NULL);
            dom_tree_print(document, 4, 0);
            _free(filecontent);
        }
    }
    return 0;
}

#endif



typedef struct _NaVa // TODO: consider changing to StrMap smap
{ char* name;
  char* value;
} NaVa;

static int nava_compare (const void* key1, const void* key2, const void* arg)
{ return strcmpCI( ((const NaVa*)key1)->name, ((const NaVa*)key2)->name ); }

static inline void str_free (char* str)
{ if(str) { _free(str); memory_freed("char*"); } }

static void add_nava (NaVa nava, AVLT* tree)
{
    NaVa* nv = (NaVa*)avl_do(AVL_ADD, tree, &nava, sizeof(nava), NULL, nava_compare);
    if(!nv) return;
    // if did not add because node already exists,
    // then replace the node's content:
    str_free(nv->name);
    str_free(nv->value);
    nv->name = nava.name;
    nv->value= nava.value;
}

static DOMNode* newNode (DOMNode* parent)
{
    DOMNode* node = _malloc(sizeof(*node));
    memset(node, 0, sizeof(*node));
    memory_alloc("DOMNode");
    if(parent)
    {
        node->parent = parent;
        node->prevSibling = parent->lastChild;
        if(!parent->headChild)
            parent->headChild = node;
        else parent->lastChild->nextSibling = node;
        parent->lastChild = node;
    }
    return node;
}

static inline char* newString (const char* start, const char* stop)
{
    int size = stop - start;
    if(size<=0) return NULL;
    char* str = (char*)_malloc(size+1);
    memory_alloc("char*");
    memcpy(str, start, size);
    str[size] = 0;
    return str;
}

#define isSpace(c) (c==' ' || c=='\t' || c=='\r' || c=='\n')

static inline char isTag (const char* html)
{ char c = *(html+1); return (*html=='<' && c!='>' && !isSpace(c)) ? c : 0; }



void collapse_spaces (char* str)
{
    if(!str) return;
    bool space = true;
    char* out = str;
    for(out=str; *str; str++)
    {
        if(!isSpace(*str))
        {
            space = false;
            *out++ = *str;
        }
        else if(!space)
        {
            space = true;
            *out++ = ' ';
        }
    }
    if(space) out--;
    *out = 0;
}

int strcmpCI (const  char* str1, const  char* str2)
{
    if(str1==NULL)
    {   if(str2==NULL) return 0;
        else return -1;
    } else if (str2==NULL) return 1;

    char c=0, d=0;
    while(true)
    {
        c = *str1++; if('A'<=c && c<='Z') c += 'a'-'A';
        d = *str2++; if('A'<=d && d<='Z') d += 'a'-'A';
        if(c==0 || c!=d) break;
    }
    if(c < d) return -1;
    if(c > d) return +1;
    return 0;
}



static char* extractInt (char* str, bool hex, unsigned int* n_ptr)
{
    if(!str) return NULL;
    unsigned int base = hex ? 16 : 10;
    unsigned int n = 0;
    char c=0;
    while(true)
    {
        c = *str;
             if(       '0'<=c && c<='9') n = n*base + c-'0';
        else if(hex && 'A'<=c && c<='F') n = n*base + c-'A'+10;
        else if(hex && 'a'<=c && c<='f') n = n*base + c-'a'+10;
        else break;
        str++;
    }
    if(c!=';') str=NULL; // if failure
    else { str++; *n_ptr = n; } // if success
    return str;
}

static void collapse_special_chars (char* str)
{
    if(!str) return;
    char* out = str;
    while(*str)
    {
        unsigned int c=0;

        if(*str!='&' || isSpace(*(str+1)));

        else if(*(str+1)=='#')
        {
            char* end;
            if(*(str+2)=='x')
                 end = extractInt(str+3, true , &c);
            else end = extractInt(str+2, false, &c);
            if(end) str = end;
            else c=0;
        }
        else if(0==memcmp(str, "&nbsp;", 6)) { c = 0xA0; str += 6; }
        else if(0==memcmp(str, "&copy;", 6)) { c = 0xA9; str += 6; }
        else if(0==memcmp(str, "&quot;", 6)) { c = '"' ; str += 6; }
        else if(0==memcmp(str, "&apos;", 6)) { c = '\''; str += 6; }
        else if(0==memcmp(str, "&amp;" , 5)) { c = '&' ; str += 5; }
        else if(0==memcmp(str, "&lt;"  , 4)) { c = '<' ; str += 4; }
        else if(0==memcmp(str, "&gt;"  , 4)) { c = '>' ; str += 4; }

        if(c==0) *out++ = *str++;
        else
        {
            if(0x0000<= c && c <=0x007F) { *out++ = (char)c; }

            if(0x0080<= c && c <=0x07FF) { *out++ = 0xC0|((c>> 6)&0x1F);
                                           *out++ = 0x80|((c    )&0x3F); }

            if(0x0800<= c && c <=0xFFFF) { *out++ = 0xE0|((c>>12)&0x0F);
                                           *out++ = 0x80|((c>> 6)&0x3F);
                                           *out++ = 0x80|((c    )&0x3F); }
        }
    }
    *out = 0;
}



static void extract_specific (AVLT* tree, const char* name, const char* specific, const char* value, int skip)
{
    char Str[1000], *str = Str;
    int size = strlen(name)-skip;
    memcpy(str, name, size);
    str += size;
    name += size;
    *str++ = '-';
    strcpy(str, specific);
    str += strlen(str);
    strcpy(str, name);
    *str = 0;
    NaVa nava;
    nava.name  = newString(Str, str);
    nava.value = newString(value, value+strlen(value));
    add_nava(nava, tree);
}

static unsigned int extract_values (char values[4][100], const char* value)
{
    unsigned int count=0;
    while(true)
    {
        while(isSpace(*value)) value++;
        if(*value==0) break;
        const char* start = value;
        while(!isSpace(*value) && *value) value++;
        int size = value-start;
        memcpy(values[count], start, size);
        values[count++][size]=0;
    }
    return count;
}

static void extract_border (AVLT* tree, const char* name, const char* value)
{
    char values[4][100];
    unsigned int count = extract_values(values, value);
    int i;
    for(i=0; i<count; i++)
    {
        const char* specific;

        if('0'<=values[i][0] && values[i][0]<='9')
            specific = "width";

        else if(0==strcmpCI(values[i], "dotted")
             || 0==strcmpCI(values[i], "dashed")
             || 0==strcmpCI(values[i], "solid" )
             || 0==strcmpCI(values[i], "double")
             || 0==strcmpCI(values[i], "groove")
             || 0==strcmpCI(values[i], "ridge" )
             || 0==strcmpCI(values[i], "inset" )
             || 0==strcmpCI(values[i], "outset")
             || 0==strcmpCI(values[i], "none"  )
             || 0==strcmpCI(values[i], "hidden")
            ) specific = "style";

        else specific = "color";

        extract_specific(tree, name, specific, values[i], 0);
    }
}

static void extract_all_four (AVLT* tree, const char* name, const char* value, int skip)
{
    char values[4][100];
    unsigned int count = extract_values(values, value);
    if(count==0) return;
    int t, r, b, l;
    switch(count)
    {
    case 1: t=r=b=l=0; break;
    case 2: t=b=0; r=l=1; break;
    case 3: t=0; r=l=1; b=2; break;
    default: t=0; r=1; b=2; l=3; break;
    }
    extract_specific(tree, name, "top"   , values[t], skip);
    extract_specific(tree, name, "right" , values[r], skip);
    extract_specific(tree, name, "bottom", values[b], skip);
    extract_specific(tree, name, "left"  , values[l], skip);
}



static void parsecss (DOMNode* node)
{
    if(!node && !node->attris.size) return;
    const char *start, *css;

    css = "style";
    NaVa* nv = (NaVa*)avl_do(AVL_FIND, &node->attris, &css, 0, 0, nava_compare);
    if(nv && nv->value && *nv->value)
    {
        css = nv->value;
        while(true)
        {
            NaVa nava;
            while(*css==':' || isSpace(*css)) css++;
            if(*css==0) break;

            // get the style name
            start = css;
            while(*css && *css!=':' && !isSpace(*css)) css++;
            nava.name = newString(start, css);
            nava.value = NULL;

            while(isSpace(*css)) css++;
            if(*css==':')
            {
                css++; // skip the ':'

                // get the style value
                start = css;
                while(*css && *css!=';') css++;
                nava.value = newString(start, css);

                // remove leading and trailing spaces
                collapse_spaces(nava.value);

                // remove leading and trailing quotes
                char c = *nava.value;
                if(c=='"' || c=='\'')
                {
                    int i;
                    for(i=0; nava.value[i]; i++)
                        nava.value[i] = nava.value[i+1];
                    for(i--; i>=0; i--)
                        if(nava.value[i]==c)
                            { nava.value[i]=0; break; }
                }

                if(*css==';') css++; // skip the ';'
            }
            add_nava(nava, &node->styles);

            AVLT* tree = &node->styles;
            if(0==strcmpCI(nava.name, "border"))
            {
                extract_border(tree, "border-top"   , nava.value);
                extract_border(tree, "border-right" , nava.value);
                extract_border(tree, "border-bottom", nava.value);
                extract_border(tree, "border-left"  , nava.value);
            }
            else if(0==strcmpCI(nava.name, "border-top"   )) extract_border(tree, "border-top"   , nava.value);
            else if(0==strcmpCI(nava.name, "border-right" )) extract_border(tree, "border-right" , nava.value);
            else if(0==strcmpCI(nava.name, "border-bottom")) extract_border(tree, "border-bottom", nava.value);
            else if(0==strcmpCI(nava.name, "border-left"  )) extract_border(tree, "border-left"  , nava.value);

            else if(0==strcmpCI(nava.name, "border-style" )) extract_all_four(tree, "border-style", nava.value, 6);
            else if(0==strcmpCI(nava.name, "border-width" )) extract_all_four(tree, "border-width", nava.value, 6);
            else if(0==strcmpCI(nava.name, "border-color" )) extract_all_four(tree, "border-color", nava.value, 6);

            else if(0==strcmpCI(nava.name, "margin" )) extract_all_four(tree, "margin" , nava.value, 0);
            else if(0==strcmpCI(nava.name, "padding")) extract_all_four(tree, "padding", nava.value, 0);
        }
    }
}



DOMNode* parsehtml (const char* html, const char* (*openFile)(const char* fileName))
{
    if(!html || !*html) return NULL;
    DOMNode *node;
    char *name;
    const char *start;

    DOMNode* stack[1000];
    node = newNode(NULL);
    node->type = DOM_DOCUMENT;
    stack[0] = node;
    int stackSize=1;
    node = NULL;

    while(*html)
    {
        if(isTag(html)) // if a DOM Element (has a name but no value)
        {
            html++; // skip '<'
            if(*html=='/')
            {
                html++; // skip '/'

                // get the element name
                start = html;
                while(*html && *html!='>') html++;
                name = newString(start, html);
                if(*html=='>') html++; // skip '>'

                collapse_spaces(name);

                if(0==strcmpCI(name, stack[stackSize-1]->name))
                    stackSize--;

                str_free(name); name=NULL;
            }
            else
            {
                // get the element name
                start = html;
                while(*html && *html!='>' && !isSpace(*html)) html++;
                name = newString(start, html);

                node = newNode(stack[stackSize-1]);
                node->type = DOM_ELEMENT;
                node->name = name;

                bool starts_with_letter =
                       ('a'<=*name && *name<='z')
                    || ('A'<=*name && *name<='Z');

                // get the element's attributes
                if(!starts_with_letter)
                {
                    node->type = DOM_IGNORED;
                    start = html;
                    while(*html && *html!='>') html++;
                    node->value = newString(start, html);
                }
                else while(true)
                {
                    NaVa nava;
                    while(*html=='=' || isSpace(*html)) html++;
                    if(*html==0 || *html=='>') break;

                    // get the attribute name
                    start = html;
                    while(*html && *html!='>' && *html!='=' && !isSpace(*html)) html++;
                    nava.name = newString(start, html);
                    nava.value = NULL;

                    while(isSpace(*html)) html++;
                    if(*html=='=')
                    {
                        html++; // skip the '='
                        while(isSpace(*html)) html++;

                        // get the attribute value
                        if(*html=='"' || *html=='\'')
                        {
                            char c = *html++; // skip the starting " or '
                            start = html;
                            while(*html && *html!=c) html++;
                            nava.value = newString(start, html);
                            collapse_special_chars(nava.value);
                            collapse_spaces(nava.value);
                            if(*html==c) html++; // skip the stopping " or '
                        }
                        else
                        {
                            start = html;
                            while(*html && *html!='>' && !isSpace(*html)) html++;
                            nava.value = newString(start, html);
                            collapse_special_chars(nava.value);
                        }
                    }
                    add_nava(nava, &node->attris);
                }

                name = node->name;
                if(!(*(html-1)=='/' && *html=='>')
                && starts_with_letter
                && 0!=strcmpCI(name, "meta")
                && 0!=strcmpCI(name, "link")
                && 0!=strcmpCI(name, "img")
                && 0!=strcmpCI(name, "hr")
                && 0!=strcmpCI(name, "br")
                && 0!=strcmpCI(name, "wbr")
                && 0!=strcmpCI(name, "col")
                && 0!=strcmpCI(name, "area")
                && 0!=strcmpCI(name, "base")
                && 0!=strcmpCI(name, "input")
                && 0!=strcmpCI(name, "command")
                && 0!=strcmpCI(name, "keygen")
                && 0!=strcmpCI(name, "embed")
                && 0!=strcmpCI(name, "param")
                && 0!=strcmpCI(name, "track")
                && 0!=strcmpCI(name, "source"))
                    stack[stackSize++] = node;

                parsecss(node);
                node = NULL;
                if(*html=='>') html++; // skip '>'
            }
        }
        else // else is a DOM Text (has no name but has a value)
        {
            start = html;
            while(*html && !isTag(html)) html++;
            if(start != html) // if there is text to load
            {
                // get the text as the node value
                node = newNode(stack[stackSize-1]);
                node->type = DOM_TEXT;
                node->value = newString(start, html);
                collapse_special_chars(node->value);
                node = NULL; // for safety reasons!
            }
        }
    }
    return stack[0];
}



const char* dom_node_get_attri (const DOMNode* node, const char* attri_name)
{
    if(!node) return NULL;
    AVLT* attris = REMOVECONST(AVLT*, &node->attris);
    void* ptr = avl_do(AVL_FIND, attris, &attri_name, 0, 0, nava_compare);
    return ptr ? ((const NaVa*)ptr)->value : NULL;
}

const char* dom_node_get_style (const DOMNode* node, const char* style_name)
{
    if(!node) return NULL;
    AVLT* styles = REMOVECONST(AVLT*, &node->styles);
    void* ptr = avl_do(AVL_FIND, styles, &style_name, 0, 0, nava_compare);
    return ptr ? ((const NaVa*)ptr)->value : NULL;
}

const char* dom_node_get_render_style (const DOMNode* node, const char* style_name, ViewPort view)
{
    const char* value = dom_node_get_style(node, style_name);

    if(value && *value) ;
    else if(0==strcmpCI(style_name, "width"           )) value = dom_node_get_attri(node, "width");
    else if(0==strcmpCI(style_name, "height"          )) value = dom_node_get_attri(node, "height");
    else if(0==strcmpCI(style_name, "color"           )) value = dom_node_get_attri(node, "color");
    else if(0==strcmpCI(style_name, "background-color")) value = dom_node_get_attri(node, "bgcolor");
    else if(0==strcmpCI(style_name, "background-image")) value = dom_node_get_attri(node, "background");
    else if(0==strcmpCI(style_name, "text-align"      )) value = dom_node_get_attri(node, "align");
    else if(0==strcmpCI(style_name, "vertical-align"  )) value = dom_node_get_attri(node, "valign");
    else if(0==strcmpCI(style_name, "font-family"     )) value = dom_node_get_attri(node, "face");
    else if(0==strcmpCI(style_name, "list-style-type"))
    {
        value = dom_node_get_attri(node, "type");
        if(!value) ;
        else if(0==strcmp(value, "a")) value = "lower-alpha";
        else if(0==strcmp(value, "A")) value = "upper-alpha";
        else if(0==strcmp(value, "i")) value = "lower-roman";
        else if(0==strcmp(value, "I")) value = "upper-roman";
    }
    return value;
}



static inline int hexToInt (int h)
{
         if('0'<=h && h<='9') h +=  0-'0';
    else if('A'<=h && h<='F') h += 10-'A';
    else if('a'<=h && h<='f') h += 10-'a';
    else h = 0;
    return h;
}

#define SUBMIT_COLOR(_name, _value) \
{   nava.name  = _name; \
    nava.value = _value; \
    avl_do(AVL_ADD, &colors, &nava, sizeof(nava), 0, nava_compare); \
}

static AVLT colors = {0};

Color getHtmlColor (const char* name)
{
    if(colors.size==0)
    {
        NaVa nava;
        SUBMIT_COLOR("Black"         , "#000000");
        SUBMIT_COLOR("White"         , "#FFFFFF");
        SUBMIT_COLOR("Red"           , "#FF0000");
        SUBMIT_COLOR("DarkRed"       , "#8B0000");
        SUBMIT_COLOR("Green"         , "#008000");
        SUBMIT_COLOR("DarkGreen"     , "#006400");
        SUBMIT_COLOR("Blue"          , "#0000FF");
        SUBMIT_COLOR("DarkBlue"      , "#00008B");
        SUBMIT_COLOR("Cyan"          , "#00FFFF");
        SUBMIT_COLOR("DarkCyan"      , "#008B8B");
        SUBMIT_COLOR("Yellow"        , "#FFFF00");
        SUBMIT_COLOR("LightYellow"   , "#FFFFE0");
        SUBMIT_COLOR("Gray"          , "#808080");
        SUBMIT_COLOR("LightGray"     , "#D3D3D3");
        SUBMIT_COLOR("Magenta"       , "#FF00FF");
        SUBMIT_COLOR("DarkMagenta"   , "#8B008B");
        SUBMIT_COLOR("Brown"         , "#A52A2A");
    }
    Color color={0};

    if(0==memcmp(name, "rgb", 3))
    {
        char str[10];

        int i=0;
        while(!('0'<=*name && *name<='9') && *name) name++;
        while( ('0'<=*name && *name<='9')) str[i++] = *name++;
        str[i] = 0;
        color.r = atoi(str);

        i=0;
        while(!('0'<=*name && *name<='9') && *name) name++;
        while( ('0'<=*name && *name<='9')) str[i++] = *name++;
        str[i] = 0;
        color.g = atoi(str);

        i=0;
        while(!('0'<=*name && *name<='9') && *name) name++;
        while( ('0'<=*name && *name<='9')) str[i++] = *name++;
        str[i] = 0;
        color.b = atoi(str);
    }
    else
    {
        NaVa* nv = avl_do(AVL_FIND, &colors, &name, 0, 0, nava_compare);
        if(nv) { name = nv->value; assert(!(!name || *name!='#' || strlen(name)!=7)); }
        if(!name || *name!='#' || strlen(name)!=7)
            return color;

        color.r = hexToInt(name[1])*16 + hexToInt(name[2]);
        color.g = hexToInt(name[3])*16 + hexToInt(name[4]);
        color.b = hexToInt(name[5])*16 + hexToInt(name[6]);
    }
    color.a = 0xFF;
    return color;
}



static void nava_free (AVLT* tree)
{
    NaVa* nava = (NaVa*)avl_min(tree);
    for( ; nava!=NULL; nava = (NaVa*)avl_next(nava))
    { str_free(nava->name); str_free(nava->value); }
    avl_free(tree);
}

void dom_tree_destroy (DOMNode* node)
{
    if(!node) return;
    DOMNode *parent, *next, *prev = node->headChild;
    while(prev)
    {
        next = prev->nextSibling;
        dom_tree_destroy(prev);
        prev = next;
    }
    parent = node->parent;
    prev = node->prevSibling;
    next = node->nextSibling;
    if(prev) prev->nextSibling = next;
    if(next) next->prevSibling = prev;
    if(parent)
    {
        if(parent->headChild==node) parent->headChild = next;
        if(parent->lastChild==node) parent->lastChild = prev;
    }
    str_free(node->name);
    str_free(node->value);
    nava_free(&node->attris);
    nava_free(&node->styles);
    memset(node, 0, sizeof(*node));
    _free(node);
    memory_freed("DOMNode");
    if(!parent) { avl_free(&colors); memory_print(); }
}



static inline void PI (int indent) // reads Print Indentation
{ int i; for(i=0; i<indent; i++) putchar(' '); }

static inline void PNV (const char* format, const char* name, const char* value)
{ if(!name) name = ""; if(!value) value = ""; printf(format, name, value); }

static inline void printNavaTree (const AVLT* tree, bool b, int indent)
{
    NaVa* nava = (NaVa*)avl_min(REMOVECONST(AVLT*, tree));
    for( ; nava != NULL; nava = (NaVa*)avl_next(nava))
    {
        const char* format;
        PI(indent);
        if(0==strcmpCI(nava->name, "color")
        || 0==strcmpCI(nava->name, "bgcolor"))
        {
            Color co = getHtmlColor(nava->value);
            int rgb = (co.r<<16) + (co.g<<8) + co.b;
            format = b ? " (%s=0x%x)\n" : " {%s=0x%x}\n";
            printf(format, nava->name, rgb);
        }
        else
        {   format = b ? " (%s=%s)\n" : " {%s=%s}\n";
            PNV(format, nava->name, nava->value);
        }
    }
}

void dom_tree_print (const DOMNode* node, int indent_size, int indent_level)
{
    if(!node) return;
    int indent = indent_size * indent_level;

    PI(indent);
    printf("[%d:", node->type);
    PNV("%s:%s]\n", node->name, node->value);

    if(node->attris.size) printNavaTree(&node->attris, 1, indent);
    if(node->styles.size) printNavaTree(&node->styles, 0, indent);

    for(node = node->headChild; node!=NULL; node = node->nextSibling)
        dom_tree_print(node, indent_size, indent_level+1);
}

