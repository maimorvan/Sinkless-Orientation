#ifndef NODE_H
#define NODE_H

#include "edge.h"
#include <stdbool.h>


typedef struct {
    int* ids;      
    int length;    
} PathList;

typedef struct _Node {
    int id;
    NeighborInfo** neighbors; 
    int neighbor_count;

    // For the algo of sinkless orientation
    PathList* li;        /*List of list */
    int li_count;         /*number of lists*/

    bool isLeaf;
    bool isCircleNode;
    bool isOriented;
} Node;

Node* create_node(int id, int min_neighbor_count, int max_node_id);

void print_node(const Node* node);

void free_pathlists(PathList* lists, int count);

void free_node(Node* node);

void merge_pathlists(Node* node, PathList* received, int received_count);

int detect_is_a_leaf(Node* node);

void update_is_a_leaf(Node* node);

int detect_cycle_in_pathlists(const Node* node);

void append_id_to_all_pathlists(Node* node);

void clear_pathlists(Node* node);

void set_pathlist_to_self(Node* node);

#endif // NODE_H