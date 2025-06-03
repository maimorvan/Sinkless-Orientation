#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../include/node.h"

Node* create_node(int id) {
    Node* node = (Node*) malloc(sizeof(Node));
    node->id = id;
    node->neighbor_count = 0;
    node->neighbors = NULL;
    return node;
}

void print_node(const Node* node) {
    printf("Node ID: %d\n", node->id);
    printf("Neighbors (%d):\n", node->neighbor_count);
    for (int i = 0; i < node->neighbor_count; ++i) {
        print_neighbor_info(node->neighbors[i]);
    }
}


void free_node(Node* node) {
    for (int i = 0; i < node->neighbor_count; ++i) {
        free(node->neighbors[i]);
    }
    free(node->neighbors);
    free(node);
}

