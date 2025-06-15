#include <stdio.h>
#include <stdlib.h>
#include "../include/graph.h"
#include "../include/sinkless_orientation.h"

void test_sinkless_rounds() {
    int min_delta = 2;
    int min_nodes = 2000;
    int max_nodes = 5000;
    int step = 100;
    int tests_per_size = 10;

    FILE *f = fopen("data/sinkless_rounds.csv", "w");
    if (!f) {
        printf("Problem opening the file sinkless_rounds.\n");
        return;
    }
    fprintf(f, "nodes,avg_rounds\n");

    for (int n = min_nodes; n <= max_nodes; n += step) {
        int total_rounds = 0;
        for (int t = 0; t < tests_per_size; t++) {
            Graph* graph = create_random_graph(n, min_delta);
            if (!graph) {
                printf("Erreur lors de la création du graphe (n=%d).\n", n);
                continue;
            }
            int rounds = run_sinkless_orientation(graph); 
            total_rounds += rounds;
            free_graph(graph);
        }
        double avg = (double)total_rounds / tests_per_size;
        fprintf(f, "%d,%.2f\n", n, avg);
        printf("Taille %d : moyenne = %.2f rounds\n", n, avg);
    }
    fclose(f);
}