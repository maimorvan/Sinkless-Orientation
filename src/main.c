#include <stdio.h>
#include <stdlib.h>
#include "../include/graph.h"
#include "../include/sinkless_orientation.h"


int main() {
    Graph* graph = load_graph("graph/tree.txt");

    if (!graph) {
        printf("Erreur lors de la création du graphe.\n");
        return 1;
    }
    
    run_sinkless_orientation(graph);
    
    free_graph(graph);

    return 0;
}