#include "avl.h"

/* macros doing swap */
#define SWAP(x, y) do { \
    *(int64_t *)x = *(int64_t *)x ^ *(int64_t *)y;       \
    *(int64_t *)y = *(int64_t *)x ^ *(int64_t *)y;       \
    *(int64_t *)x = *(int64_t *)x ^ *(int64_t *)y; } while(0)

typedef enum {
    AVLTREE_UNCHANGED = 0,
    AVLTREE_CHANGED = 1,
    AVLTREE_ADJUSTED = 2,
#if 0
    AVLTREE_SEARCH_PREDECESSOR = 3,
    AVLTREE_SEARCH_SUCCESSOR = 4,
#endif
} avl_status;

/* forward declarations */
static void avl_insert(AVLTree tree, void *val);
static bool avl_remove(AVLTree tree, void *val);
static bool avl_exists(AVLTree tree, void *val);
static int avl_nodecmp(void *val1, void *val2);
static void avl_traverse(AVLTree tree);
static void avl_free(AVLTree *treeptr);
static void *avl_search_predecessor(AVLTree tree, void *val);
static void *avl_search_successor(AVLTree tree, void *val);

static void LL_Rotate(AVLNode *nodeptr);
static void RR_Rotate(AVLNode *nodeptr);
static void LR_Rotate(AVLNode *nodeptr);
static void RL_Rotate(AVLNode *nodeptr);

static void update_depth(AVLNode node);
static int balance_factor(AVLNode node);

static void avl_walk_free(AVLNode *nodeptr);
static void avl_walk_print(AVLNode node, unsigned int nodes);
static AVLNode avl_search_node(AVLNode node, void *val);
static avl_status avl_walk_insert(AVLNode *walkptr, void *val);
static avl_status avl_walk_remove(AVLNode *walkptr, void *val, AVLNode *rm, int *search_direction);

/* free all nodes */
static void
avl_walk_free(AVLNode *nodeptr)
{
    if(*nodeptr == NULL)
        return ;

    avl_walk_free(&(*nodeptr)->left_kid);
    avl_walk_free(&(*nodeptr)->right_kid);
    free(*nodeptr);

    *nodeptr = NULL;
}

/* traverse all nodes */
static void
avl_walk_print(AVLNode node, unsigned int nodes)
{
    // start a static counter
    static unsigned int counter = 0;
    if(node == NULL)
        return ;

    avl_walk_print(node->left_kid, nodes);

    printf(++counter==nodes? "%lld, ":"%lld", (int64_t)node->value);

    avl_walk_print(node->right_kid, nodes);
}

/* search avl node by value */
static AVLNode
avl_search_node(AVLNode node, void *val)
{
    if(node == NULL)
        return NULL;

    int diff = avl_nodecmp(node->value, val);
    if(diff == 0)
        return node;

    if(diff > 0)
        return avl_search_node(node->left_kid, val);
    else
        return avl_search_node(node->right_kid, val);
}

/* update node depth */
static void
update_depth(AVLNode node)
{
    if(node) {
        unsigned left_depth = node->left_kid? node->left_kid->depth:0;
        unsigned right_depth = node->right_kid? node->right_kid->depth:0;

        node->depth = (left_depth>right_depth? left_depth:right_depth) + 1;
    }
}

/* get node balance factor */
static int
balance_factor(AVLNode node)
{
    if(node == NULL)
        return 0;

    unsigned left_depth = node->left_kid? node->left_kid->depth:0;
    unsigned right_depth = node->right_kid? node->right_kid->depth:0;

    return left_depth - right_depth;
}

/* inserting to left subtree of node's left kid, then tree lose balance, do LL rotate */
static void
LL_Rotate(AVLNode *nodeptr)
{
    AVLNode node = *nodeptr;
    // alias node's left kid as `lson`
    AVLNode lson = node->left_kid;

    /* 1. update relation between lson's right kid and node */
    // move lson's right kid to be node's new left kid
    node->left_kid = lson->right_kid;
#if 0
    // set lson's right kid's father pointer
    if(lson->right_kid != NULL) lson->right_kid->father = node;
#endif
    // update node depth
    update_depth(node);

    /* 2. update relation between lson and node */
    // move node to be lson's right son
    lson->right_kid = node;
#if 0
    // set lson's father pointer, lift up lson, downgrade node
    lson->father = node->father;
    // update node's father
    if(node->father != NULL) {
        if(node->father->left_kid == node) // node is his father's left kid
            node->father->left_kid = lson;
        else
            node->father->right_kid = lson;
    }

    // set node's father pointer
    node->father = lson;
#endif
    // update lson depth
    update_depth(lson);
}

/* inserting to right subtree of node's right kid, then tree lose balance, do RR rotate */
static void
RR_Rotate(AVLNode *nodeptr)
{
    AVLNode node = *nodeptr;
    // alias node's right kid to 'rson'
    AVLNode rson = node->right_kid;

    /* 1. update relation between rson's left_kid and node */
    // move rson's left kid to be node's new right kid
    node->right_kid = rson->left_kid;
#if 0
    // set rson's left kid's father pointer
    if(rson->left_kid != NULL) rson->left_kid->father = node;
#endif
    // update node depth
    update_depth(node);

    /* 2. update relation between rson and node */
    // move node to be rson's left kid
    rson->left_kid = node;
#if 0
    // set rson's father pointer, lift up rson, downgrade node
    rson->father = node->father;
    // update node's father
    if(node->father != NULL) {
        if(node->father->left_kid == node) // node is his father's left kid
            node->father->left_kid = rson;
        else
            node->father->right_kid = rson;
    }

    // set node's father pointer
    node->father = rson;
#endif
    // update rson depth
    update_depth(rson);
}

/* inserting to right subtree of node's left kid, then tree lose balance, do LR rotate */
static void
LR_Rotate(AVLNode *nodeptr)
{
    // left rotate node's left kid first
    RR_Rotate(&(*nodeptr)->left_kid);
    // then right rotate node
    LL_Rotate(nodeptr);
}

/* inserting to left subtree of node's right kid, then tree lose balance, do RL rotate */
static void
RL_Rotate(AVLNode *nodeptr)
{
    // right rotate node's right kid first
    LL_Rotate(&(*nodeptr)->right_kid);
    // then left rotate node
    RR_Rotate(nodeptr);
}

/* initialize AVL tree */
AVLTree
init_avl_tree(COMPARE nodecmp)
{
    AVLTree tree = newAVLTree();
    /* bind AVL methods */
    tree->nodes = tree->depth = 0;
    tree->root = NULL;
    tree->insert = avl_insert;
    tree->remove = avl_remove;
    tree->exists = avl_exists;
    tree->nodecmp = nodecmp? nodecmp:avl_nodecmp;
    tree->traverse = avl_traverse;
    tree->freespace = avl_free;
    tree->predecessor = avl_search_predecessor;
    tree->successor = avl_search_successor;

    return tree;
}

/* insert node by walking recursively */
static avl_status
avl_walk_insert(AVLNode *walkptr, void *val)
{
    AVLNode *nodeptr = NULL, *nextptr = NULL;
    do {
        if(*walkptr == NULL) {
            nodeptr = walkptr;
            break;
        }
        int diff = avl_nodecmp((*walkptr)->value, val);
        // found a duplicate
        if(diff == 0) {
            (*walkptr)->occurance ++;
            // just return, don't insert
            return AVLTREE_UNCHANGED;
        }
        // val less than node, go left kid
        if(diff > 0) {
            if((*walkptr)->left_kid) {
                // if left kid is not NULL, go downwards
                nextptr = &(*walkptr)->left_kid;
            } else {
                // insert to node's left kid
                nodeptr = &(*walkptr)->left_kid;
            }
        } else {
            if((*walkptr)->right_kid) {
                // if right kid is not NULL, go downwards
                nextptr = &(*walkptr)->right_kid;
            } else {
                // insert to node's right kid
                nodeptr = &(*walkptr)->right_kid;
            }
        }
    } while(0);

    // walk to leaf node, do insert
    if(nodeptr) {
        AVLNode new = newAVLNode();
        new->value = val;
        new->depth = 1;
        new->occurance = 1;
        new->left_kid = new->right_kid = NULL;
        *nodeptr = new;
        // inserted
        return AVLTREE_CHANGED;
    }

    // recursively continue downwards
    avl_status status = nextptr? avl_walk_insert(nextptr, val):AVLTREE_UNCHANGED;

    // update depth and do adjust
    if(status == AVLTREE_CHANGED) {
        // update node's depth since his subtree grows
        update_depth(*walkptr);
        // check balance factor
        int bf = balance_factor(*walkptr);
        // found the unbalanced node while tracing back
        if(bf == 2) {
            int left_bf = balance_factor((*walkptr)->left_kid);
            switch(bf + left_bf) {
                case 3: // left subtree of node's left kid is higher, 2 + 1, do LL Rotate
                    LL_Rotate(walkptr);
                    break;
                case 1: // right subtree of node's left kid is higher, 2 + (-1), do LR Rotate
                    LR_Rotate(walkptr);
                    break;
            }
            status = AVLTREE_ADJUSTED;
        } else if(bf == -2) {
            int right_bf = balance_factor((*walkptr)->right_kid);
            switch(bf + right_bf) {
                case -3: // right subtree of node's right kid is higher, -2 + (-1), do RR Rotate
                    RR_Rotate(walkptr);
                    break;
                case -1: // left subtree of node's right kid is higher, -2 + 1, do RL Rotate
                    RL_Rotate(walkptr);
                    break;
            }
            status = AVLTREE_ADJUSTED;
        }
    }
    return status;
}

/* remove node by walking recursively */
static avl_status
avl_walk_remove(AVLNode *walkptr, void *val, AVLNode *rm, int *search_direction)
{
    AVLNode *nextptr = NULL;    // next node pointer
    /* still finding the remove node */
    if(*rm == NULL) {
        // remove node not found
        if(*walkptr == NULL)
            return AVLTREE_UNCHANGED;

        int diff = avl_nodecmp((*walkptr)->value, val);
        if(diff > 0) {
            nextptr = &(*walkptr)->left_kid;
        } else if(diff < 0) {
            nextptr = &(*walkptr)->right_kid;
        } else {
            // okay, we found this node
            // this node occurs to be a leaf node, just remove
            if((*walkptr)->left_kid==NULL && (*walkptr)->right_kid==NULL) {
                (*walkptr)->value = NULL;
                free(*walkptr);
                *walkptr = NULL;

                return AVLTREE_CHANGED;
            }

            *rm = *walkptr;
            if((*walkptr)->left_kid) {
                // continue to find predecessor
                *search_direction = 1;
                nextptr = &(*walkptr)->left_kid;
            } else {
                // continue to find successor
                *search_direction = -1;
                nextptr = &(*walkptr)->right_kid;
            }
        }
    } else {
        // already found remove node
        if(nextptr == NULL) {
            AVLNode remove = NULL;
            if(*search_direction == 1) {
                // searching the predecessor
                if((*walkptr)->right_kid == NULL) {
                    remove = *walkptr;
                    *walkptr = (*walkptr)->left_kid;
                } else {
                    nextptr = &(*walkptr)->right_kid;
                }
            } else if(*search_direction == -1) {
                // searching the successor
                if((*walkptr)->left_kid == NULL) {
                    remove = *walkptr;
                    *walkptr = (*walkptr)->right_kid;
                } else {
                    nextptr = &(*walkptr)->left_kid;
                }
            }
            // found the predecessor/successor, do swap and remove
            if(remove) {
                SWAP(&(*rm)->value, &remove->value);
                SWAP(&(*rm)->occurance, &remove->occurance);
                remove->value = NULL;
                free(remove);

                return AVLTREE_CHANGED;
            }
        }
    }
    /* go downwards recursively */
    avl_status status = nextptr? avl_walk_remove(nextptr, val, rm, search_direction):AVLTREE_UNCHANGED;
    /* if node has been removed */
    if(status == AVLTREE_CHANGED) {
        update_depth(*walkptr);
        int bf = balance_factor(*walkptr);
        if(abs(bf) == 2) {
            int left_depth = (*walkptr)->left_kid? (*walkptr)->left_kid->depth:0;
            int right_depth = (*walkptr)->right_kid? (*walkptr)->right_kid->depth:0;
            AVLNode kid = left_depth>right_depth? (*walkptr)->left_kid:(*walkptr)->right_kid;
            int bfdiff = bf + balance_factor(kid);
            switch(bfdiff) {
                case 3: // left subtree of node's left kid is higher, 2 + 1, do LL Rotate
                    LL_Rotate(walkptr);
                    break;
                case 1: // right subtree of node's left kid is higher, 2 + (-1), do LR Rotate
                    LR_Rotate(walkptr);
                    break;
                case -1: // left subtree of node's right kid is higher, -2 + 1, do RL Rotate
                    RL_Rotate(walkptr);
                    break;
                case -3: // right subtree of node's right kid is higher, -2 + (-1), do RR Rotate
                    RR_Rotate(walkptr);
                    break;
            }
        }
    }
    return status;
}

/* insert node to AVL tree */
static void
avl_insert(AVLTree tree, void *val)
{
    avl_status status = avl_walk_insert(&tree->root, val);
    if(status==AVLTREE_CHANGED || status==AVLTREE_ADJUSTED)
        tree->nodes ++;

    tree->depth = tree->root->depth;
}

/* remove node from AVL tree */
static bool
avl_remove(AVLTree tree, void *val)
{
    int direction = 0;
    AVLNode *rm = NULL;

    avl_status status = avl_walk_remove(&tree->root, val, rm, &direction);
    if(status==AVLTREE_UNCHANGED)
        return false;

    tree->nodes --;
    tree->depth = tree->root->depth;

    return true;
}

/* exists method */
static bool
avl_exists(AVLTree tree, void *val)
{
    return avl_search_node(tree->root, val)? true:false;
}

/* compare method */
static int
avl_nodecmp(void *val1, void *val2)
{
    return (int64_t)val1 - (int64_t)val2;
}

/* walk iterator method */
static void
avl_traverse(AVLTree tree)
{
    printf("WALKING:\n");
    printf("\t[ ");
    avl_walk_print(tree->root, tree->nodes);
    printf(" ]\n");
}

/* free malloc space */
static void
avl_free(AVLTree *treeptr)
{
    avl_walk_free(&(*treeptr)->root);
    free(*treeptr);
    *treeptr = NULL;
}

/* search predecessor */
static void *
avl_search_predecessor(AVLTree tree, void *val)
{
    AVLNode found = avl_search_node(tree->root, val);
    // no such node or no node less than val, no predecessor
    if(found==NULL || found->left_kid==NULL)
        return NULL;

    AVLNode next = found->left_kid;
    while(next) {
        if(next->right_kid)
            next = next->right_kid;
        else
            return next->value;
    }

    return next;
}

/* search successor */
static void *
avl_search_successor(AVLTree tree, void *val)
{
    AVLNode found = avl_search_node(tree->root, val);
    // no such node or no node more than val, no successor
    if(found==NULL || found->right_kid==NULL)
        return NULL;

    AVLNode next = found->right_kid;
    while(next) {
        if(next->left_kid)
            next = next->left_kid;
        else
            return next->value;
    }

    return next;
}
