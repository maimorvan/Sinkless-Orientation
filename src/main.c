#include <stdio.h>
#include <stdlib.h>
#include "../include/graph.h"

int main() {
    int node_count = 20;
    int min_neighbors = 1;

    Graph* graph = create_random_graph(node_count, min_neighbors);

    print_graph(graph);

    free_graph(graph);

    return 0;
}