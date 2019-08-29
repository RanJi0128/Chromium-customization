/*
    avl.c

    Provided by Rhyscitlema
    @ http://rhyscitlema.com

    USE AT YOUR OWN RISK!
*/
#include "avl.h"
#include <assert.h>
#include <string.h>
#include <stdbool.h>

#ifdef LIB_STD
// used only for debugging
#include <_malloc.h>
#else
#include <malloc.h>
#define _malloc malloc
#define _free free
#define memory_alloc(str)
#define memory_freed(str)
#endif


static inline void* getkey (AVL*  avl) { return avl ? avl+1 : NULL; }

static inline AVL* getnode (void* key) { return key ? (AVL*)key-1 : NULL; }


void* avl_left   (void* avl) { return avl ? getkey(getnode(avl)->left)   : NULL; }

void* avl_right  (void* avl) { return avl ? getkey(getnode(avl)->right)  : NULL; }

void* avl_parent (void* avl) { return avl ? getkey(getnode(avl)->parent) : NULL; }

void* avl_root (AVLT* tree) { return getkey(tree->root); }


void* avl_min (AVLT* tree)
{
    AVL* avl = tree->root;
    if(!avl) return NULL;
    while(avl->left)
    avl = avl->left;
    return getkey(avl);
}

void* avl_max (AVLT* tree)
{
    AVL* avl = tree->root;
    if(!avl) return NULL;
    while(avl->right)
    avl = avl->right;
    return getkey(avl);
}



void* avl_next (void* _avl)
{
    AVL* avl = getnode(_avl);
    if(!avl) return NULL;
    if(avl->right)
    {   avl = avl->right;
        while(avl->left)
        avl = avl->left;
    }
    else while(1)
    {   AVL* tavl = avl;
        avl = avl->parent;
        if(!avl || avl->left==tavl) break;
    }
    return getkey(avl);
}

void* avl_prev (void* _avl)
{
    AVL* avl = getnode(_avl);
    if(!avl) return NULL;
    if(avl->left)
    {   avl = avl->left;
        while(avl->right)
        avl = avl->right;
    }
    else while(1)
    {   AVL* tavl = avl;
        avl = avl->parent;
        if(!avl || avl->right==tavl) break;
    }
    return avl;
}



void* avl_new (const void* key1, int keysize)
{
    if(keysize<=0) return NULL;
    int size = sizeof(AVL)+keysize;
    AVL* avl = (AVL*)_malloc(size);
    memset(avl, 0, size);
    avl->keysize = keysize;
    memory_alloc("AVL_NODE");
    void* key2 = getkey(avl);
    if(key1 && key2) memcpy(key2, key1, keysize);
    return key2;
}

static void avl_remove (AVL* avl)
{
    int size = sizeof(AVL)+avl->keysize;
    memset(avl, 0, size);
    _free(avl);
    memory_freed("AVL_NODE");
}

static void avl_free_r (AVL* avl)
{
    if(avl==NULL) return;
    avl_free_r(avl->left);
    avl_free_r(avl->right);
    avl_remove(avl);
}

void avl_free (AVLT* tree)
{
    assert(tree!=NULL);
    avl_free_r (tree->root);
    avl_clear(tree);
}



static bool rebalance (AVLT* tree, AVL* node)
{
    AVL *pnode, *tnode, *snode;
    char l = (node->left ) ? node->left->height  : 0;
    char r = (node->right) ? node->right->height : 0;
    node->height = 1 + (l>r?l:r);

    if(l-r>1)       // if left is longer
    {
        tnode = node->left;
        snode = tnode->right;
        l = (tnode->left ) ? tnode->left->height  : 0;
        r = (tnode->right) ? tnode->right->height : 0;

        if(l>=r)    // do single rotation
        {
            node->left = snode; if(snode) snode->parent = node;
            tnode->right = node;
            tnode->parent = pnode = node->parent;
            if(pnode==NULL) tree->root = tnode;
            else { if(pnode->left==node) pnode->left = tnode; else pnode->right = tnode; }
            node->parent = tnode;
        }
        else        // do double rotation
        {
            snode->parent = pnode = node->parent;
            if(pnode==NULL) tree->root = snode;
            else { if(pnode->left==node) pnode->left = snode; else pnode->right = snode; }
            node->left   = pnode = snode->right; if(pnode) pnode->parent = node;
            tnode->right = pnode = snode->left;  if(pnode) pnode->parent = tnode;
            tnode->parent = snode;
            snode->left = tnode;
            snode->right = node;
            node->parent = snode;
            tnode->height -= 1;
            snode->height += 1;
        }
        node->height -= 2;
        return true;
    }
    if(r-l>1)       // if right is longer
    {
        tnode = node->right;
        snode = tnode->left;
        l = (tnode->left ) ? tnode->left->height  : 0;
        r = (tnode->right) ? tnode->right->height : 0;

        if(l<=r)    // do single rotation
        {
            node->right = snode; if(snode) snode->parent = node;
            tnode->left = node;
            tnode->parent = pnode = node->parent;
            if(pnode==NULL) tree->root = tnode;
            else { if(pnode->left==node) pnode->left = tnode; else pnode->right = tnode; }
            node->parent = tnode;
        }
        else        // do double rotation
        {
            snode->parent = pnode = node->parent;
            if(pnode==NULL) tree->root = snode;
            else { if(pnode->left==node) pnode->left = snode; else pnode->right = snode; }
            node->right = pnode = snode->left;  if(pnode) pnode->parent = node;
            tnode->left = pnode = snode->right; if(pnode) pnode->parent = tnode;
            tnode->parent = snode;
            snode->right = tnode;
            snode->left = node;
            node->parent = snode;
            tnode->height -= 1;
            snode->height += 1;
        }
        node->height -= 2;
        return true;
    }
    return false;
}



void* avl_do (enum AVL_OPR OPR,
              AVLT* tree,
              const void* key1,
              int keysize,
              const void* arg,
              int (*compare) (const void* key1, const void* key2, const void* arg))
{
    assert(tree!=NULL);
    if(!keysize) keysize = tree->keysize;
    if(!arg) arg = tree->arg;
    if(!compare) compare = tree->compare;
    if(!compare || !key1) return NULL;
    void* key2;

    AVL* node = tree->root;
    if(node==NULL)
    {
        if(OPR==AVL_ADD || OPR==AVL_INS || OPR==AVL_PUT)
        {
            if(OPR==AVL_PUT) key2 = (void*)(size_t)key1;
            else
            {   key2 = avl_new(key1, keysize);
                if(!key2) return NULL;
            }
            node = getnode(key2);
            node->height = 1;
            tree->root = node;
            tree->size++;
        }
        return NULL;
    }

    bool found=0;
    int i=0;
    while(1)
    {
        key2 = getkey(node);
        i = compare(key1, key2, arg);
        if(!found && !i) found=1;
        if(i<0)
        {
            if(node->left==NULL) break;
            else node = node->left;
        }
        else if(i>0 || OPR==AVL_INS)
        {
            if(node->right==NULL) break;
            else node = node->right;
        }
        else if(OPR==AVL_DEL) break;
        else return key2;
    }

    if(OPR==AVL_CEIL)
    {
        if(i>0) return avl_next(getkey(node));
        else return getkey(node);
    }

    if(OPR==AVL_FLOOR)
    {
        if(i<0) return avl_prev(getkey(node));
        else return getkey(node);
    }

    if(OPR==AVL_ADD || OPR==AVL_INS || OPR==AVL_PUT)
    {
        if(OPR==AVL_PUT) key2 = (void*)(size_t)key1;
        else
        {   key2 = avl_new (key1, keysize);
            if(!key2) return NULL;
        }
        tree->size++;

        AVL* pnode = node;
        node = getnode(key2);
        node->parent = pnode;
        node->height = 1;
        if(i<0) pnode->left  = node;
        else    pnode->right = node;

        do{ if(rebalance(tree, pnode)) break;
            pnode = pnode->parent;
        } while(pnode);
    }

    else if(OPR==AVL_DEL && found)
        avl_delete(tree, getkey(node));

    return (void*)found;
}



void avl_delete (AVLT* tree, void* avl)
{
    assert(avl!=NULL);
    AVL *node = getnode(avl);
    AVL *tnode, *snode;

    if(tree)
    {
        if(node->left)
        {   tnode = node->left;
            while(tnode->right)
             tnode = tnode->right;
            snode = tnode->left;
        }
        else if(node->right)
        {   tnode = node->right;
            while(tnode->left)
             tnode = tnode->left;
            snode = tnode->right;
        }
        else { tnode = node; snode = NULL; }

        AVL* pnode = tnode->parent;
        if(pnode==NULL)
        {   if( tree->root == node)
                tree->root = NULL;
        }
        else
        {
            if(pnode->left == tnode)
                pnode->left = snode;
            else pnode->right = snode;
            if(snode) snode->parent = pnode;

            do{ rebalance(tree, pnode);
                pnode = pnode->parent;
            } while(pnode);

            if(node!=tnode)
            {
                tnode->left  = snode = node->left;  if(snode) snode->parent = tnode;
                tnode->right = snode = node->right; if(snode) snode->parent = tnode;
                tnode->parent= pnode = node->parent;
                if(pnode==NULL) tree->root = tnode;
                else { if(pnode->left==node) pnode->left = tnode; else pnode->right = tnode; }
                tnode->height = node->height;
            }
        }
        tree->size--;
    }
    avl_remove(node);
}

