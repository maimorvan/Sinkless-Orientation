#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "../include/graph.h"

Graph* create_random_graph(int node_count, int min_nb_neighbors) {
    Node** nodes = (Node**)malloc(node_count * sizeof(Node*));
    srand(time(NULL));
    for (int i = 0; i < node_count; ++i) {
        nodes[i] = create_node(i, min_nb_neighbors, node_count);
    }
    Graph* graph = (Graph*)malloc(sizeof(Graph));
    graph->nodes = nodes;
    graph->node_count = node_count;

    ensure_neighbors_symmetry(graph);

    return graph;
}

void free_graph(Graph* graph) {
    for (int i = 0; i < graph->node_count; ++i) {
        free_node(graph->nodes[i]);
    }
    free(graph->nodes);
    free(graph);
}

void print_graph(const Graph* graph) {
    printf("Graph with %d nodes:\n", graph->node_count);
    for (int i = 0; i < graph->node_count; ++i) {
        print_node(graph->nodes[i]);
        printf("\n");
    }
}


int has_neighbor(Node* node, int neighbor_id) {
    for (int i = 0; i < node->neighbor_count; ++i) {
        if (node->neighbors[i]->neighbor_id == neighbor_id) {
            return 1;
        }
    }
    return 0;
}

void add_neighbor(Node* node, int neighbor_id) {
    node->neighbors = realloc(node->neighbors, (node->neighbor_count + 1) * sizeof(NeighborInfo*));
    node->neighbors[node->neighbor_count] = create_neighbor_info(neighbor_id);
    node->neighbor_count++;
}

void ensure_neighbors_symmetry(Graph* graph) {
    for (int i = 0; i < graph->node_count; ++i) {
        Node* node = graph->nodes[i];
        for (int j = 0; j < node->neighbor_count; ++j) {
            int neighbor_id = node->neighbors[j]->neighbor_id;
            Node* neighbor_node = graph->nodes[neighbor_id];
            if (!has_neighbor(neighbor_node, node->id)) {
                add_neighbor(neighbor_node, node->id);
            }
        }
    }
}