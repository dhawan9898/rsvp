#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <netinet/in.h>
#include <math.h>
#include <arpa/inet.h>
#include <string.h>

/* AVL tree node (Generic) */
typedef struct avl_node {
    struct avl_node *left;
    struct avl_node *right;
    void *data;  // Generic data pointer
    signed char height;
} avl_node;

static inline int getHeight(avl_node *node) {
    return node ? node->height : 0;
}

static inline int max(int a, int b) {
    return (a > b) ? a : b;
}

static inline int getBalance(avl_node *node) {
    return node ? getHeight(node->left) - getHeight(node->right) : 0;
}

/* Function pointer types for customization */
typedef int (*compare_func)(void *, void *);
typedef void (*print_func)(void *);
typedef void (*free_func)(void *);

/* AVL Tree Functions */
avl_node* create_node(void *data);
avl_node* insert_node(avl_node *root, void *data, compare_func cmp);
avl_node* delete_node(avl_node *root, void *data, compare_func cmp, free_func free_data);
avl_node* search_node(avl_node *root, void *data, compare_func cmp);
void display_info(avl_node *root, void *data, compare_func cmp, print_func print);
void free_tree(avl_node *root, free_func free_data);
void dump_tree(avl_node *root, int space, print_func print);

