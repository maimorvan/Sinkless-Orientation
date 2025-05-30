#include <stdio.h>
#include <stdlib.h>
#include "../include/edge.h"


void print_edge_direction(EdgeDirection dir) {
    if (dir == OUTGOING)
        printf("OUTGOING");
    else if (dir == INCOMING)
        printf("INCOMING");
    else
        printf("UNKNOWN");
}

NeighborInfo* create_neighbor_info(int neighbor_id) {
    NeighborInfo * N = (NeighborInfo*) malloc(1*sizeof(NeighborInfo)); 
    N-> neighbor_id = neighbor_id;
    N-> direction = UNKNOWN;
    return N;
}


void print_neighbor_info(const NeighborInfo* info) {
    printf("Neighbor ID: %d, Direction: ", info->neighbor_id);
    print_edge_direction(info->direction);
    printf("\n");
}