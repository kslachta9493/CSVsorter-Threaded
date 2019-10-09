#ifndef BST_H
#define BST_H

typedef struct node
{
    CSV data;
    struct node* left;
    struct node* right;
} node;

typedef int (*comparer)(int, int);

typedef void (*callback)(node*);

#endif


