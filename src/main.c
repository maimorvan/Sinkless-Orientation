#include <stdio.h>
#include <stdlib.h>
#include "../include/graph.h"
#include "../include/sinkless_orientation.h"


int main() {
    Graph* graph = load_graph("graph/cycle.txt");

    if (!graph) {
        printf("Erreur lors de la création du graphe.\n");
        return 1;
    }
    
    int round = run_sinkless_orientation(graph);
    printf("%d", round);

    print_graph(graph);

    if (check_graph_orientation(graph) == 0) {
        printf("Orientation correct !\n");
    } else {
        printf("Orientation incorrecte !\n");
    }
    
    free_graph(graph);

    return 0;
}