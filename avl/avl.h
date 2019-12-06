#ifndef __AVL_H__
#define __AVL_H__

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>

/* avl tree node structure */
typedef struct avl_node {
    void *value;                /* value */

    unsigned int depth;         /* depth */
    unsigned int occurance;     /* duplicates */
    struct avl_node *left_kid;  /* left subtree */
    struct avl_node *right_kid; /* right subtree */
#if 0
    struct avl_node *father;    /* father */
#endif
} *AVLNode;                     /* pointer type */
#define newAVLNode() (AVLNode)malloc(sizeof(struct avl_node))

typedef struct avl_tree *AVLTree;
#define newAVLTree() (AVLTree)malloc(sizeof(struct avl_tree))
typedef void (*INSERT)(AVLTree tree, void *val);
typedef bool (*REMOVE)(AVLTree tree, void *val);
typedef bool (*EXISTS)(AVLTree tree, void *val);
typedef int (*COMPARE)(void *val1, void *val2);
typedef void (*TRAVERSE)(AVLTree tree);
typedef void (*FREE)(AVLTree *treeptr);
typedef void *(*ITERATOR)(AVLTree tree, void *val);

/* avl tree structure */
typedef struct avl_tree {
    unsigned int nodes;         /* node counts */
    AVLNode root;               /* root entry */
    unsigned int depth;         /* tree depth */

    INSERT insert;              /* insert method */
    REMOVE remove;              /* remove method */
    EXISTS exists;              /* exsits method */
    COMPARE nodecmp;            /* node compare method */
    FREE freespace;             /* free malloc memory */
    TRAVERSE traverse;          /* traverse method */
    ITERATOR predecessor;       /* find predecessor */
    ITERATOR successor;         /* find successor */
} *AVLTree;                     /* pointer type */

#define AVLINSERT(t, v) t->insert(t, v)
#define AVLREMOVE(t, v) t->remove(t, v)
#define AVLEXISTS(t, v) t->exists(t, v)
#define AVLTRAVERSE(t) t->traverse(t)
#define AVLPREDECESSOR(t, v) t->predecessor(t, v)
#define AVLSUCCESSOR(t, v) t->successor(t, v)

AVLTree init_avl_tree(COMPARE nodecmp);

#endif /*__AVL_H__ */
