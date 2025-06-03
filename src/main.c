#include <stdio.h>
#include <stdlib.h>
#include "../include/graph.h"

int main() {
    Graph* graph = load_graph("graph/tree.txt");

    if (!graph) {
        printf("Erreur lors de la création du graphe.\n");
        return 1;
    }
    print_graph(graph);
    free_graph(graph);

    return 0;
}