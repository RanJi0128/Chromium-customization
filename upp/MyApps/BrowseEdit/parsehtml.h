/*
    parsehtml.h

    DOM-Node:

    A DOM-Document has no name, no value,
    no attributes, no styles, may have no child.

    A DOM-Text has no name but has a value. It
    has no attributes, no styles and no child.

    A DOM-Element has a name but has no value.
    It may have attributes, may have styles,
    and may have children.

    A DOM-Ignored has a name and a value, but
    has no attributes, no styles and no child.

    Notes:

    If a valid attribute or style name is
    queried but was not explicitly defined,
    then NULL, not the default value, will be
    returned. You are advised to first query
    for an attribute before querying for the
    style of the same name (or purpose).

    Every text is as it originally was upon loading,
    except for attributes and styles values which are
    trimmed of any leading or trailing space-type
    character, and those occurring in-between words
    are collapsed into a single space character,
    using the function collapse_string().

    You are to take care of case sensitivity issues.
    You can use the case-insensitive strcmpCI().

    UTF-8 is the character set used.

    Provided by Rhyscitlema
    @ http://rhyscitlema.com

    USE AT YOUR OWN RISK!
*/

#include "avl.h"


enum DOM_NODE_TYPE
{
    DOM_NONE,
    DOM_DOCUMENT,
    DOM_ELEMENT,
    DOM_TEXT,
    DOM_IGNORED
};


typedef struct _DOMNode
{
    enum DOM_NODE_TYPE type;
    char* name;
    char* value;
    AVLT attris;
    AVLT styles;

    struct _DOMNode* parent;
    struct _DOMNode* headChild;
    struct _DOMNode* lastChild;
    struct _DOMNode* prevSibling;
    struct _DOMNode* nextSibling;
} DOMNode;


/* Given a html string, parse it and return a DOM Document Node.

   A function pointer is also given, which is responsible for
   all text file loading. Upon failure, NULL must be returned.
   The fileName is preceeded with a path that is relative to
   that which existed upon the function being called.
*/
DOMNode* parsehtml (const char* html, const char* (*openFile)(const char* fileName));


/* Given a node, return its type */
static inline enum DOM_NODE_TYPE dom_node_get_type (const DOMNode* node) { return node ? node->type : DOM_NONE; }

/* Given a node, return its name */
static inline const char* dom_node_get_name (const DOMNode* node) { return node ? node->name : NULL; }

/* Given a node, return its value */
static inline const char* dom_node_get_value (const DOMNode* node) { return node ? node->value : NULL; }


/* Given attribute name, return its value, NULL if none */
const char* dom_node_get_attri (const DOMNode* node, const char* attri_name);

/* Given style name, return style value, NULL if none */
const char* dom_node_get_style (const DOMNode* node, const char* style_name);


/* Given a node, remove the tree held by it */
void dom_tree_destroy (DOMNode* node);

/* Given a node, print the tree held by it */
void dom_tree_print (const DOMNode* node, int indent_size, int indent_level);


typedef struct _ViewPort
{   int width;
    int height;
} ViewPort;

/* Given style name and viewport/viewpage info, return render style value, NULL if none */
const char* dom_node_get_render_style (const DOMNode* node, const char* style_name, ViewPort view);


typedef struct _Color
{   unsigned char a,r,g,b;
} Color;

/* Given a html color as a string, return its ARGB color. */
Color getHtmlColor (const char* name);


/* Case In-sensitive string compare */
int strcmpCI (const  char* str1, const  char* str2);

/* Trim leading and trailing spaces,
   and collapse spaces in-between words
*/
void collapse_spaces (char* str);

