/*
    avl.h

    Provided by Rhyscitlema
    @ http://rhyscitlema.com

    USE AT YOUR OWN RISK!
*/
#ifndef _AVL_H
#define _AVL_H


typedef struct _AVL
{   struct _AVL *left;
    struct _AVL *right;
    struct _AVL *parent;
    int keysize;
    char height;
} AVL;

typedef struct _AVLT    // AVL Tree
{   AVL* root;          // root of tree
    int size;           // number of nodes

    int keysize;
    const void* arg;
    int (*compare) (const void* key1, const void* key2, const void* arg);
} AVLT;

static inline void avl_clear (AVLT *tree)
{ if(tree) { tree->root=0; tree->size=0; } }


enum AVL_OPR
{   AVL_FIND,   // find a key that equals given key
    AVL_CEIL,   // find lowest key greater or equal
    AVL_FLOOR,  // find highest key lesser or equal
    AVL_ADD,    // add if not found, else return existing key2
    AVL_INS,    // add even if found, if found return (void*)1
    AVL_DEL,    // find one and delete, if found return (void*)1
    AVL_PUT     // put the node obtained from a call to avl_new()
};


/*  If on very first operation then set *tree = {0}.

    'arg' is only passed to the compare() function.
    'key1' is the given key, 'key2' is from existing node.

    If given key does not already exist then NULL is
    returned, else if on AVL_INS or AVL_DEL then (void*)1
    is returned, else the existing key2 is returned.

    When any of keysize, arg and compare is 0 or NULL, the
    corresponding value from the AVLT-struct is used instead.

    keysize is ignored unless on AVL_ADD or AVL_INS.
*/
void* avl_do (enum AVL_OPR OPR,
              AVLT* tree,
              const void* key1,
              int keysize,
              const void* arg,
              int (*compare) (const void* key1, const void* key2, const void* arg));


void  avl_free (AVLT* tree);    // given tree, free all nodes
void* avl_root (AVLT* tree);    // given tree, get root node
void* avl_min  (AVLT* tree);    // given tree, get min node
void* avl_max  (AVLT* tree);    // given tree, get max node
void* avl_next (void* avl);     // given node, get its next
void* avl_prev (void* avl);     // given node, get its prev
void* avl_left (void* avl);     // given node, get its left
void* avl_right (void* avl);    // given node, get its right
void* avl_parent (void* avl);   // given node, get its parent
void  avl_delete (AVLT* tree, void* avl); // given node, delete it. tree can be NULL
void* avl_new (const void* key1, int keysize); // get a new node, use AVL_PUT later


#endif
