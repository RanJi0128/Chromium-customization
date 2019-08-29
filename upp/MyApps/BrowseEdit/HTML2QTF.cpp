/*
    HTML2QTF.cpp

    To compile with GCC, execute:
    1) gcc -Wall -pedantic -std=c99 avl.c -c -o avl.o
    2) gcc -Wall -pedantic -std=c99 parsehtml.c -c -o parsehtml.o
    3) g++ -Wall -pedantic -std=c++98 -DTEST HTML2QTF.cpp -o html2qtf parsehtml.o avl.o

    To run: ./html2qtf input.html

    Provided by Rhyscitlema
    @ http://rhyscitlema.com

    USE AT YOUR OWN RISK!
*/

#include <sstream>
#include <string>
#include <cstring>
#include <algorithm>
#include <vector>
#include <set>

extern "C" {
#include "parsehtml.h"
}


#ifdef TEST

#include <iostream>
#include <fstream>

std::string HTML2QTF (std::string input, std::string directory);

int main (int argc, char** argv)
{
    if(argc != 2) std::cout << "Usage: " << argv[0] << " input.html " << std::endl;
    else
    {
        std::ifstream file(argv[1]);
        if(!file.is_open())
        {
            std::cout << "Error: file '" << argv[1] << "' could not be opened." << std::endl;
            std::cout << HTML2QTF(argv[1], "") << std::endl;
        }
        else
        {
            // read entire file content
            std::istreambuf_iterator<char> eos;
            std::string input(std::istreambuf_iterator<char>(file), eos);
            file.close();
            // convert HTML to QTF
            std::cout << HTML2QTF(input, "") << std::endl;
        }
    }
    return 0;
}

#endif



ViewPort view = {4000, 0};

#define FAIL -0x7FFFFFFF

#define isSpace(c) (c==' ' || c=='\t' || c=='\r' || c=='\n')

static int toDots (const char* value)
{
    if(!value || !*value) return FAIL;
    char temp[100], *str = temp;

    while(isSpace(*value)) value++;   // skip spaces if any

    while('0'<=*value && *value<='9') // extract integer
        *str++ = *value++;

    if(*value=='.')     // if decimal point encountered
    {
        *str++ = *value++;
        while('0'<=*value && *value<='9') // extract float
            *str++ = *value++;
    }
    *str = 0;
    if(str==temp) return FAIL;  // if no integer found
    float n = atof(temp);       // else obtain integer

         if(value[0]=='%') n = view.width*n/100;      // if in percentage
    else if(value[0]=='p' && value[1]=='x') n=n*5;    // if in pixels
    else if(value[0]=='p' && value[1]=='t') n=n*8;    // if in points
    else if(value[0]=='e' && value[1]=='m') n=n*12*7; // if in em
    return (int)n;
}

static std::string toColor (const char* value)
{
    char color[20];
    Color co = getHtmlColor(value);
    sprintf(color, "(%d.%d.%d)", co.r, co.g, co.b);
    return color;
}



static std::string directory;

static std::string extract_image (const DOMNode* node)
{
    int n, width=0, height=0;
    std::stringstream output;

    const char* value = dom_node_get_render_style(node, "background-image", view);
    if(!(value && *value) && 0==strcmpCI(dom_node_get_name(node), "img"))
        value = dom_node_get_attri(node, "src");
    if(!(value && *value)) return "";

    n = toDots(dom_node_get_render_style(node, "width", view));
    if(n!=FAIL) width = n*5;

    n = toDots(dom_node_get_render_style(node, "height", view));
    if(n!=FAIL) height = n*5;

    const char* start = "data:image/raw;base64,";
    n = strlen(start);
    if(0==memcmp(value, start, n))
    {
        value += n; while(*value==' ') value++;
        output << "@@rawimage:" << width << "&" << height << "\n(" << value << ")\n";
    }
    else if(strlen(value) < 1000)
    {
        output << "@@imagefile:" << width << "&" << height << "`" << directory;
        if(0==memcmp(value, "url", 3))
        {
            value += 3;
            while(*value=='(' || isSpace(*value)) value++;
            char c = *value;
            if(c=='"' || c=='\'') value++; else c=0;
            start = value;
            if(c) { while(*value && *value!=')' && *value!=c) value++; }
            else { while(*value && *value!=')' && !isSpace(*value)) value++; }
            output << std::string(start).substr(0, value-start);
        }
        else output << value;
        output << "`";
    }
    return output.str();
}



struct CellPosition
{
    int row, col;

    bool operator< (const CellPosition & cellP) const
    {
    return (row < cellP.row) ? 1 :
           (row > cellP.row) ? 0 :
           (col < cellP.col) ? 1 :
           (col > cellP.col) ? 0 : 0;
    }
};

struct Rectangle
{
    int top;
    int right;
    int bottom;
    int left;

    bool zeros() const
    { return top==0
          && right==0
          && bottom==0
          && left==0;
    }

    Rectangle()
    {
        top = FAIL;
        right = FAIL;
        bottom = FAIL;
        left = FAIL;
    }
};

struct StyleInfo
{
    // alignment: -1 or 0 or +1
    int hori_align; // = +2 for justify
    int vert_align;

    const char* list_type;
    std::string font_family;

    std::string color;
    std::string backgr_color;
    std::string backgr_image;

    std::string border_color_top;
    std::string border_color_right;
    std::string border_color_bottom;
    std::string border_color_left;

    Rectangle margin;
    Rectangle border;
    Rectangle padding;
    int text_indent;

    int font_size;
    const char* font_weight;
    const char* font_style;
    const char* text_decoration;
    int minimum_height;

    StyleInfo()
    {
        margin = Rectangle();
        border = Rectangle();
        padding= Rectangle();
        hori_align = FAIL;
        vert_align = FAIL;
        font_size = FAIL;
        list_type = NULL;
        font_style = NULL;
        font_weight = NULL;
        text_decoration = NULL;
        text_indent = 0;
        minimum_height = 0;
    }
};

struct Table
{
    std::vector<int> equalize;
    std::set<CellPosition> hidden;
    CellPosition cellP;
    int header_rows;
};

static Table EmptyTable;



static void getStyle (const DOMNode* node, StyleInfo & style, bool includeBorder)
{
    const char* value;
    int n;

    n = toDots(dom_node_get_render_style(node, "padding-top", view));
    if(n!=FAIL) style.padding.top = n;

    n = toDots(dom_node_get_render_style(node, "padding-right", view));
    if(n!=FAIL) style.padding.right = n;

    n = toDots(dom_node_get_render_style(node, "padding-bottom", view));
    if(n!=FAIL) style.padding.bottom = n;

    n = toDots(dom_node_get_render_style(node, "padding-left", view));
    if(n!=FAIL) style.padding.left = n;

    n = toDots(dom_node_get_render_style(node, "margin-top", view));
    if(n!=FAIL) style.margin.top = n;

    n = toDots(dom_node_get_render_style(node, "margin-right", view));
    if(n!=FAIL) style.margin.right = n;

    n = toDots(dom_node_get_render_style(node, "margin-bottom", view));
    if(n!=FAIL) style.margin.bottom = n;

    n = toDots(dom_node_get_render_style(node, "margin-left", view));
    if(n!=FAIL) style.margin.left = n;

    if(includeBorder)
    {
        n = toDots(dom_node_get_render_style(node, "border-top-width", view));
        if(n!=FAIL) style.border.top = n;

        n = toDots(dom_node_get_render_style(node, "border-right-width", view));
        if(n!=FAIL) style.border.right = n;

        n = toDots(dom_node_get_render_style(node, "border-bottom-width", view));
        if(n!=FAIL) style.border.bottom = n;

        n = toDots(dom_node_get_render_style(node, "border-left-width", view));
        if(n!=FAIL) style.border.left = n;

        value = dom_node_get_render_style(node, "border-top-color", view);
        if(value && * value) style.border_color_top = toColor(value);

        value = dom_node_get_render_style(node, "border-right-color", view);
        if(value && * value) style.border_color_right = toColor(value);

        value = dom_node_get_render_style(node, "border-bottom-color", view);
        if(value && * value) style.border_color_bottom = toColor(value);

        value = dom_node_get_render_style(node, "border-left-color", view);
        if(value && * value) style.border_color_left = toColor(value);
    }

    value = dom_node_get_render_style(node, "color", view);
    if(value && *value) style.color = toColor(value);

    value = dom_node_get_render_style(node, "background-color", view);
    if(value && *value) style.backgr_color = toColor(value);

    style.backgr_image = extract_image(node);

    n = toDots(dom_node_get_render_style(node, "height", view));
    if(n > style.minimum_height) style.minimum_height = n;

    value = dom_node_get_render_style(node, "text-align", view);
    if(value && *value)
    {
        if(0==strcmpCI(value, "left"   )) style.hori_align = -1;
        if(0==strcmpCI(value, "center" )) style.hori_align =  0;
        if(0==strcmpCI(value, "right"  )) style.hori_align =  1;
        if(0==strcmpCI(value, "justify")) style.hori_align =  2;
    }

    value = dom_node_get_render_style(node, "vertical-align", view);
    if(value && *value)
    {
        if(0==strcmpCI(value, "top"   )) style.vert_align = -1;
        if(0==strcmpCI(value, "middle")) style.vert_align =  0;
        if(0==strcmpCI(value, "bottom")) style.vert_align =  1;
    }

    n = toDots(dom_node_get_render_style(node, "font-size", view));
    if(n!=FAIL) style.font_size = n;

    value = dom_node_get_render_style(node, "font-family", view);
    if(value && *value)
    {
             if(0==strcmpCI(value, "Arial"          )) style.font_family = "A";
             if(0==strcmpCI(value, "sans-serif"     )) style.font_family = "A";
        else if(0==strcmpCI(value, "Courier"        )) style.font_family = "C";
        else if(0==strcmpCI(value, "monospace"      )) style.font_family = "C";
        else if(0==strcmpCI(value, "serif"          )) style.font_family = "R";
        else if(0==strcmpCI(value, "Times New Roman")) style.font_family = "R";
        else if(0==strcmpCI(value, "Times"          )) style.font_family = "R";
        else style.font_family = std::string("!") + value + "!";
    }

    value = dom_node_get_render_style(node, "font-style", view);
    if(value && *value)
    {
        if(0==strcmpCI(value, "oblique")) value = "underline";
        style.font_style = value;
    }

    value = dom_node_get_render_style(node, "font-weight", view);
    if(value && *value)
    {
        if(0==strcmpCI(value, "bolder")) value = "bold";
        else
        {	n = toDots(value);
            if(n>=600) value = "bold";
            else if(n>0) value = "normal";
        }
        style.font_weight = value;
    }

    value = dom_node_get_render_style(node, "text-decoration", view);
    if(value && *value) style.text_decoration = value;

    n = toDots(dom_node_get_render_style(node, "text-indent", view));
    if(n!=FAIL) style.text_indent = n;

    value = dom_node_get_render_style(node, "list-style-type", view);
    if(value && *value) style.list_type = value;
}



static char doHTML2QTF (std::stringstream & output, const DOMNode* node, const StyleInfo& oldSt, char prev);

static void extract_table_subtags (std::stringstream & content, const DOMNode* node, Table & table, StyleInfo style)
{
    const char* name = dom_node_get_name(node);

    if(0==strcmpCI(name, "thead")
    || 0==strcmpCI(name, "tbody")
    || 0==strcmpCI(name, "tfoot"))
    {
        getStyle(node, style, false);
        for(node = node->headChild; node != NULL; node = node->nextSibling)
            extract_table_subtags(content, node, table, style);
    }
    else if(0==strcmpCI(name, "tr"))
    {
        getStyle(node, style, true);

        node = node->headChild;
        name = dom_node_get_name(node);
        if(0==strcmpCI(name, "th")) table.header_rows++;

        table.cellP.col=0;
        while(true)
        {
            while(true)
            {
                // check if current cellP is not hidden
                if(table.hidden.find(table.cellP) == table.hidden.end()) break;
                content << ":: ";
                table.cellP.col++;
            }
            if(node==NULL) break;
            extract_table_subtags(content, node, table, style);
            node = node->nextSibling;
        }
        table.cellP.row++;
    }
    else if(0==strcmpCI(name, "th") || 0==strcmpCI(name, "td"))
    {
        if(table.cellP.row || table.cellP.col)
            content << "::"; // marks the start of the cell formatting sequence

        getStyle(node, style, true);

        int n = style.minimum_height;
        if(n<0) n=0;
        content << 'H' << n << ';';

        if(style.vert_align==-1) content << '^';
        if(style.vert_align== 0) content << '=';
        if(style.vert_align== 1) content << 'v';

        content << 't' << style.border.top    << '/' << style.padding.top    << ';';
        content << 'r' << style.border.right  << '/' << style.padding.right  << ';';
        content << 'b' << style.border.bottom << '/' << style.padding.bottom << ';';
        content << 'l' << style.border.left   << '/' << style.padding.left   << ';';

        if(!style.border_color_top.empty())    content << "R_T" << style.border_color_top << ';';
        if(!style.border_color_right.empty())  content << "R_R" << style.border_color_right << ';';
        if(!style.border_color_bottom.empty()) content << "R_B" << style.border_color_bottom << ';';
        if(!style.border_color_left.empty())   content << "R_L" << style.border_color_left << ';';

        if(!style.backgr_color.empty()) content << '@'  << style.backgr_color << ';';
        if(!style.backgr_image.empty()) content << style.backgr_image;

        n = toDots(dom_node_get_attri(node, "rowspan"));
        if(n>1)
        {
            content << "|" << n-1 << ";";
            CellPosition cellP = table.cellP;
            while(n-- > 1) // loop through the row span
            { cellP.row++; table.hidden.insert(cellP); }
        }

        n = toDots(dom_node_get_attri(node, "colspan"));
        if(n<1) n=1;
        int colspan = n;
        if(n>1)
        {
            content << "-" << n-1 << ";";
            CellPosition cellP = table.cellP;
            while(n-- > 1) // loop through the col span
            { cellP.col++; table.hidden.insert(cellP); }
        }

        if(table.cellP.row==0)
        {
            n = toDots(dom_node_get_render_style(node, "width", view));
            if(n<0) n=0;
            int i;
            for(i=0; i < colspan; i++)
            {
                unsigned int j = table.cellP.col + i;
                if(j >= table.equalize.size()) break;
                if(table.equalize[j] < n)
                   table.equalize[j] = n;
            }
            for( ; i < colspan; i++)
                table.equalize.push_back(n);
        }

        content << ' '; // marks the end of the cell formatting sequence

        for(node = node->headChild; node != NULL; node = node->nextSibling)
            doHTML2QTF(content, node, style, '&');

        table.cellP.col++;
    }
    else if(0==strcmpCI(name, "colgroup"))
    {
        table.cellP.col=0;
        for(node = node->headChild; node != NULL; node = node->nextSibling)
            extract_table_subtags(content, node, table, style);
    }
    else if(0==strcmpCI(name, "col"))
    {
        getStyle(node, style, true);

        int n;
        n = toDots(dom_node_get_attri(node, "colspan"));
        if(n<1) n=1;
        int colspan = n;

        n = toDots(dom_node_get_render_style(node, "width", view));
        if(n<0) n=0;
        int i;
        for(i=0; i < colspan; i++)
        {
            unsigned int j = table.cellP.col + i;
            if(j >= table.equalize.size()) break;
            if(table.equalize[j] < n)
               table.equalize[j] = n;
        }
        for( ; i < colspan; i++)
            table.equalize.push_back(n);

        table.cellP.col++;
    }
}



static void extract_table (std::stringstream & output, const DOMNode* node)
{
    std::stringstream content;
    const char* name;
    int n, m;

    n = toDots(dom_node_get_render_style(node, "margin-top", view));
    if(n!=FAIL) content << 'B' << n << ";";

    n = toDots(dom_node_get_render_style(node, "margin-right", view));
    if(n!=FAIL) content << '>' << n << ";";

    n = toDots(dom_node_get_render_style(node, "margin-bottom", view));
    if(n!=FAIL) content << 'A' << n << ";";

    n = toDots(dom_node_get_render_style(node, "margin-left", view));
    if(n!=FAIL) content << '<' << n << ";";

    n = toDots(dom_node_get_render_style(node, "border-top-width", view));
    m = toDots(dom_node_get_attri(node, "border"));
    if(n==FAIL) { if(m==FAIL) n=0; else n=m*8; }
    content << 'f' << n << ";";

    if(m==FAIL) n=0;
    else if(n!=0) n = toDots(dom_node_get_attri(node, "cellspacing"));
    if(n!=FAIL && !(n==0 && m>0)) content << 'g' << n << ";";

    name = dom_node_get_render_style(node, "border-top-color", view);
    if(!(name && *name)) dom_node_get_attri(node, "bordercolor");
    if(  name && *name )
    {
        std::string color = toColor(name);
        if(color != "(0.0.0)")
        {
            content << 'F' << color << ';';
            content << 'G' << color << ';';
        }
    }

    Table table = EmptyTable;

    StyleInfo style = StyleInfo();
    style.border.top = 0;
    style.border.right = 0;
    style.border.bottom = 0;
    style.border.left = 0;
    style.padding.top = 15;
    style.padding.right = 25;
    style.padding.bottom = 15;
    style.padding.left = 25;
    getStyle(node, style, false);

    for(node = node->headChild; node != NULL; node = node->nextSibling)
        extract_table_subtags(content, node, table, style);

    output << "{{";

    // set column equalization
    n=0;
    m=0;
    unsigned int i;
    for(i=0; i < table.equalize.size(); i++)
    {
        if(table.equalize[i])
            m += table.equalize[i];
        else n++;
    }
    if(n)
    {   n = (view.width - m)/n;
        for(i=0; i < table.equalize.size(); i++)
            if(!table.equalize[i])
                table.equalize[i] = n;
    }
    for(i=0; i < table.equalize.size(); i++)
    {
        if(i>0) output << ':';
        output << table.equalize[i];
    }
    output << ';';

    if(table.header_rows>0)
        output << "h" << table.header_rows << ';';

    output << content.str() << "}}";
}

static bool isLastNodeOfBlock (const DOMNode* node)
{
    if(node==NULL) return false;
    const char* str = dom_node_get_name(node);
    if(0==strcmpCI(str, "td")) return true;
    const DOMNode* parent = node->parent;
    while(true)
    {
        node = node->nextSibling;
        if(!node) break;
        str = dom_node_get_value(node);
        if(!str) return false;
        for( ; *str; str++) if(!isSpace(*str)) return false;
    }
    return isLastNodeOfBlock(parent);
}



static char doHTML2QTF (std::stringstream & output, const DOMNode* node, const StyleInfo& oldSt, char prev)
{
    if(node==NULL) return 0;

    bool block = false;     // if not block then is iniline
    bool opened = false;    // if '[' is/was sent to output
    const char *name, *value;
    int n;
    DOM_NODE_TYPE type = dom_node_get_type(node);

    StyleInfo style = StyleInfo();

    if(type==DOM_ELEMENT)
    {
        // get name and convert it to lower case
        name = dom_node_get_name(node);

             if(0==strcmpCI(name, "head")) return prev;
        else if(0==strcmpCI(name, "link")) return prev;
        else if(0==strcmpCI(name, "script")) return prev;

        else if(0==strcmpCI(name, "table"))
        {
            if(prev!='&') output << '&';
            extract_table(output, node);
            return 0;
        }
        else if(0==strcmpCI(name, "br")) { output << '&'; return '&'; }

        else if(0==strcmpCI(name, "hr"))
        {
            if(prev!='&') output << '&';
            output << "[H1 &]";
            return '&';
        }
        else if(0==strcmpCI(name, "img")) { output << '\n' << extract_image(node); return 0; }
        else
        {
            // collect formatting sequence
            std::stringstream format;

                 if(0==strcmpCI(name, "b")) style.font_weight = "bold";
            else if(0==strcmpCI(name, "i")) style.font_style = "italic";
            else if(0==strcmpCI(name, "u")) style.text_decoration = "underline";
            else if(0==strcmpCI(name, "strike")) style.text_decoration = "line-through";
            else if(0==strcmpCI(name, "sup")) format << '`';
            else if(0==strcmpCI(name, "sub")) format << ',';
            else if(0==strcmpCI(name, "a"))
            {
                value = dom_node_get_attri(node, "href");
                if(value && *value) format << '^' << value << '^';
                value = dom_node_get_attri(node, "name");
                if(value && *value) format << 'I' << value << ';';
            }
            else if(0==strcmpCI(name, "h1"))
            {
                block = true;
                style.font_weight = "bold";
                style.font_size = 234;
                style.margin.top = 70;
                style.margin.bottom = 70;
            }
            else if(0==strcmpCI(name, "h2"))
            {
                block = true;
                style.font_weight = "bold";
                style.font_size = 200;
                style.margin.top = 60;
                style.margin.bottom = 60;
            }
            else if(0==strcmpCI(name, "h3"))
            {
                block = true;
                style.font_weight = "bold";
                style.font_size = 167;
                style.margin.top = 50;
                style.margin.bottom = 50;
            }
            else if(0==strcmpCI(name, "pre"))
            {
                style.font_family = "C";
                style.text_indent = 200;
                style.margin.top = 40;
                style.margin.bottom = 40;
                block = true;
            }
            else if(0==strcmpCI(name, "ul"))
            {
                block = true;
                style.text_indent = 200;
                style.list_type = "disc"; // default
                //top_bottom_margin = 30;
            }
            else if(0==strcmpCI(name, "ol"))
            {
                block = true;
                style.text_indent = 200;
                style.list_type = "decimal"; // default
            }
            else if(0==strcmpCI(name, "li"))
            {
                block = true;
                style.text_indent = 200;
            }
            else if(0==strcmpCI(name, "p"))
            {
                block = true;
                style.margin.top = 40;
                style.margin.bottom = 40;
            }

            if(block && prev!='&')
            { output << '&'; prev = '&'; }

            getStyle(node, style, true);


            if(0==strcmpCI(style.font_weight, "bold")
            && 0!=strcmpCI(oldSt.font_weight, "bold")) format << '*';

            if(0==strcmpCI(style.font_weight, "normal")
            && 0==strcmpCI(oldSt.font_weight, "bold")) format << '*';

            if(0==strcmpCI(style.font_style, "italic")
            && 0!=strcmpCI(oldSt.font_style, "italic")) format << '/';

            if(0==strcmpCI(style.font_style, "normal")
            && 0==strcmpCI(oldSt.font_style, "italic")) format << '/';

            if(0==strcmpCI(style.text_decoration, "underline"))
            {   if(0==strcmpCI(oldSt.text_decoration, "line-through")) format << '-';
                format << '_';
            }
            if(0==strcmpCI(style.text_decoration, "line-through"))
            {   if(0==strcmpCI(oldSt.text_decoration, "underline")) format << '_';
                format << '-';
            }
            if(0==strcmpCI(style.text_decoration, "none"))
            {   if(0==strcmpCI(oldSt.text_decoration, "underline")) format << '_';
                if(0==strcmpCI(oldSt.text_decoration, "line-through")) format << '-';
            }

            n = style.margin.top;    if(n!=FAIL && n) format << 'b' << n << ';';
            n = style.margin.right;  if(n!=FAIL && n) format << 'r' << n << ';';
            n = style.margin.bottom; if(n!=FAIL && n) format << 'a' << n << ';';
            n = style.margin.left;   if(n!=FAIL && n) format << 'l' << n << ';';
            n = style.text_indent;   if(n!=FAIL && n) format << 'i' << n << ';';

            n = style.font_size;     if(n!=FAIL && n) format << '+' << n << ';';

            if(!style.font_family.empty()) format << style.font_family;

            if(!style.color.empty()) format << '@' << style.color << ';';

            if(!style.backgr_color.empty()) format << '$' << style.backgr_color << ';';

            n = style.hori_align;
            if(n!=FAIL)
            {
	            if(n==-1) format << '<';
	            if(n== 0) format << '=';
	            if(n== 1) format << '>';
	            if(n== 2) format << '#';
            }

            if(style.list_type)
            {
	                 if(0==strcmpCI(style.list_type, "none"   )) format << "O_;";
	            else if(0==strcmpCI(style.list_type, "disc"   )) format << "O0;";
	            else if(0==strcmpCI(style.list_type, "circle" )) format << "O1;";
	            else if(0==strcmpCI(style.list_type, "square" )) format << "O2;";
	            else if(0==strcmpCI(style.list_type, "decimal")) format << "N1;";
	            else if(0==strcmpCI(style.list_type, "decimal-leading-zero")) format << "N0;";
	            else if(0==strcmpCI(style.list_type, "lower-alpha")) format << "Na;";
	            else if(0==strcmpCI(style.list_type, "upper-alpha")) format << "NA;";
	            else if(0==strcmpCI(style.list_type, "lower-roman")) format << "Ni;";
	            else if(0==strcmpCI(style.list_type, "upper-roman")) format << "NI;";
	            else format << "O0;";
            }

            std::string result = format.str();
            if(!result.empty())
            {
                output << '[' << result << ' ';
                opened = true; // mark as opened so to be closed
            }
        }
    }
    else if(type==DOM_TEXT)
    {
        value = dom_node_get_value(node);
        while(value && *value)
        {
            char c = *value++;

            if(c=='\n')
            {
                output << c;
                if(*value==' ' && prev!='&') output << *value++;
            }
            else if(!isSpace(c) || (!isSpace(prev) && prev!='&'))
            {
                // if a special character then escape it
                if((unsigned char)c > ' '
                && (unsigned char)c < 128
                && !('0'<=c && c<='9')
                && !('A'<=c && c<='Z')
                && !('a'<=c && c<='z'))
                    output << '`';

                if(c==(char)0xC2 && *value==(char)0xA0) // if c == &nbsp;
                { value++; output << ' '; } // then put normal space character

                else if((unsigned char)c >= ' ') output << c;
            }
            if(!isSpace(c)) prev = 0;
            else if(prev!='&') prev = ' ';
        }
    }

    if(!style.font_weight) style.font_weight = oldSt.font_weight;
    if(!style.font_style) style.font_style = oldSt.font_style;
    if(!style.text_decoration) style.text_decoration = oldSt.text_decoration;

    const DOMNode* current = node;
    for(node = node->headChild; node != NULL; node = node->nextSibling)
        prev = doHTML2QTF(output, node, style, prev);

    if(opened)
    {
        if(block && prev!='&' && !isLastNodeOfBlock(current))
        { output << '&'; prev = '&'; }
        output << ']';
    }
    return prev;
}

std::string HTML2QTF (std::string input, std::string _directory)
{
    DOMNode* document = parsehtml(input.c_str(), NULL);

    #ifdef DOM_PRINT
    dom_tree_print(document, 4, 0);
    #endif

    std::stringstream output;

    directory = _directory;

    doHTML2QTF(output, document, StyleInfo(), '&');

    dom_tree_destroy(document);

    return output.str();
}

