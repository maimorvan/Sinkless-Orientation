#ifndef GRAPH_H
#define GRAPH_H

#include "node.h"

typedef struct {
    Node** nodes;      
    int node_count;    
} Graph;

Graph* create_random_graph(int node_count, int min_nb_neighbors);

void free_graph(Graph* graph);

void print_graph(const Graph* graph);

int has_neighbor(Node* node, int neighbor_id);

void add_neighbor(Node* node, int neighbor_id);

void ensure_neighbors_symmetry(Graph* graph);


#endif // GRAPH_H