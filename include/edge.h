#ifndef EDGE_H
#define EDGE_H

typedef enum {
    UNKNOWN,
    OUTGOING,
    INCOMING
} EdgeDirection;

typedef struct {
    int neighbor_id;
    EdgeDirection direction;
} NeighborInfo;

void print_edge_direction(EdgeDirection dir); 

NeighborInfo* create_neighbor_info(int neighbor_id);

void print_neighbor_info(const NeighborInfo* info);


#endif // EDGE_H