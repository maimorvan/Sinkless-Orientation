#include <stdio.h>
#include <stdlib.h>
#include "../include/graph.h"

int main() {
    int node_count = 6;
    //int min_neighbors = 1;
    int delta = 3;

    //Graph* graph = create_random_graph(node_count, min_neighbors);
    Graph* graph = create_random_graph_delta(node_count, delta);

    if (!graph) {
        printf("Erreur lors de la création du graphe.\n");
        return 1;
    }
    print_graph(graph);
    free_graph(graph);

    return 0;
}