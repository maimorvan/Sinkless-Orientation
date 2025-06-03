#ifndef NODE_H
#define NODE_H

#include "edge.h"
#include <stdbool.h>

typedef struct _Node {
    int id;
    NeighborInfo** neighbors; 
    int neighbor_count;
} Node;

Node* create_node(int id);

void print_node(const Node* node);

void free_node(Node* node);

#endif // NODE_H