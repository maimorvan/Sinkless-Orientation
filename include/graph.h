#ifndef GRAPH_H
#define GRAPH_H

#include "node.h"

typedef struct {
    Node** nodes;      
    int node_count;    
} Graph;

Graph* create_random_graph(int node_count, int min_nb_neighbors);

Graph* create_random_graph_delta(int node_count, int delta);

void free_graph(Graph* graph);

void print_graph(const Graph* graph);

int has_neighbor(Node* node, int neighbor_id);

void add_neighbor_one_side(Node* node, int neighbor_id);

void add_neighbor_both_side(Node* node, Node* neighbor);

void ensure_neighbors_symmetry(Graph* graph);

int try_neighbors_symmetry(const Graph* graph);

void save_graph(const Graph* graph, const char* filename);

Graph* load_graph(const char* filename);

int check_graph_orientation(const Graph* graph);

#endif // GRAPH_H