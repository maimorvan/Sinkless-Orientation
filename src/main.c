#include <stdio.h>
#include <stdlib.h>
#include "../include/graph.h"
#include "../include/sinkless_orientation.h"


int main() {
    
    Graph* graph = create_random_graph_delta(5000,3);

    //print_graph(graph);

    if (!graph) {
        printf("Erreur lors de la création du graphe.\n");
        return 1;
    }
    
    run_sinkless_orientation(graph);

    //print_graph(graph);

    if (check_graph_orientation(graph) == 0) {
        printf("Orientation correct !\n");
    } else {
        printf("Orientation incorrecte !\n");
    }
    if (check_sinkless_orientation(graph) == 0) {
        printf("Sinkless correct !\n");
    } else {
        printf("Sinkless incorrecte !\n");
    }


    free_graph(graph);

    return 0;
}