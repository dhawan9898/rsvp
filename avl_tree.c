#include "avl_tree.h"

avl_node* create_node(void *data) {
    avl_node* node = (avl_node*)malloc(sizeof(avl_node));
    if (!node) {
        printf("Memory allocation failed!\n");
        return NULL;
    }
    node->data = data;
    node->left = node->right = NULL;
    node->height = 1;
    return node;
}

avl_node *rotateRight(avl_node *y) {
    avl_node *x = y->left;
    y->left = x->right;
    x->right = y;
    y->height = max(getHeight(y->left), getHeight(y->right)) + 1;
    x->height = max(getHeight(x->left), getHeight(x->right)) + 1;
    return x;
}

avl_node *rotateLeft(avl_node *x) {
    avl_node *y = x->right;
    x->right = y->left;
    y->left = x;
    x->height = max(getHeight(x->left), getHeight(x->right)) + 1;
    y->height = max(getHeight(y->left), getHeight(y->right)) + 1;
    return y;
}

avl_node *insert_node(avl_node *node, void *data, compare_func cmp) {
    if (!node) return create_node(data);
    
    if (cmp(data, node->data) < 0)
        node->left = insert_node(node->left, data, cmp);
    else if (cmp(data, node->data) > 0)
        node->right = insert_node(node->right, data, cmp);
    else
        return node;

    node->height = 1 + max(getHeight(node->left), getHeight(node->right));
    
    int balance = getBalance(node);
    
    if (balance > 1 && cmp(data, node->left->data) < 0)
        return rotateRight(node);
    
    if (balance < -1 && cmp(data, node->right->data) > 0)
        return rotateLeft(node);
    
    if (balance > 1 && cmp(data, node->left->data) > 0) {
        node->left = rotateLeft(node->left);
        return rotateRight(node);
    }
    
    if (balance < -1 && cmp(data, node->right->data) < 0) {
        node->right = rotateRight(node->right);
        return rotateLeft(node);
    }
    
    return node;
}

avl_node *minValueNode(avl_node *node) {
    while (node->left)
        node = node->left;
    return node;
}

avl_node *delete_node(avl_node *root, void *data, compare_func cmp, free_func free_data) {
    if (!root) return root;

    int comparison = cmp(data, root->data);

    if (comparison < 0) {
        root->left = delete_node(root->left, data, cmp, free_data);
    } else if (comparison > 0) {
        root->right = delete_node(root->right, data, cmp, free_data);
    } else {
        if (!root->left || !root->right) {
            avl_node *temp = root->left ? root->left : root->right;
            free_data(root->data);
            free(root);
            return temp;
        }
        avl_node *temp = minValueNode(root->right);
        root->data = temp->data;
        root->right = delete_node(root->right, temp->data, cmp, free_data);
    }

    if (!root) return root;

    root->height = 1 + max(getHeight(root->left), getHeight(root->right));

    int balance = getBalance(root);

    if (balance > 1 && getBalance(root->left) >= 0)
        return rotateRight(root);

    if (balance > 1 && getBalance(root->left) < 0) {
        root->left = rotateLeft(root->left);
        return rotateRight(root);
    }

    if (balance < -1 && getBalance(root->right) <= 0)
        return rotateLeft(root);

    if (balance < -1 && getBalance(root->right) > 0) {
        root->right = rotateRight(root->right);
        return rotateLeft(root);
    }

    return root;
}

avl_node *search_node(avl_node *root, void *data, compare_func cmp) {
    if (!root || cmp(data, root->data) == 0)
        return root;
    
    return (cmp(data, root->data) < 0) ? search_node(root->left, data, cmp) : search_node(root->right, data, cmp);
}

void traverse_avl_tree(avl_node *root, process_func proc) {
    if (root == NULL) return;
    traverse_avl_tree(root->left, proc);
    proc(root->data);
    traverse_avl_tree(root->right, proc);
}

void display_info(avl_node *root, void *data, compare_func cmp, print_func print) {
    avl_node *node = search_node(root, data, cmp);
    if (node) {
        printf("Node found:\n");
        print(node->data);
    } else {
        printf("Node not found.\n");
    }
}

void free_tree(avl_node *root, free_func free_data) {
    if (!root) return;
    free_tree(root->left, free_data);
    free_tree(root->right, free_data);
    free_data(root->data);
    free(root);
}

void dump_tree(avl_node *root, int space, print_func print) {
    if (!root) return;
    space += 10;
    dump_tree(root->right, space, print);
    printf("\n");
    for (int i = 10; i < space; i++)
        printf(" ");
    print(root->data);
    dump_tree(root->left, space, print);
}

