#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../include/node.h"

Node* create_node(int id, int min_neighbor_count, int max_node_id) {
    Node* node = (Node*) malloc(sizeof(Node));
    node->id = id;
    node->neighbor_count = min_neighbor_count;
    node->neighbors = (NeighborInfo**) malloc(min_neighbor_count * sizeof(NeighborInfo*));

    srand(time(NULL) + id); 

    for (int i = 0; i < min_neighbor_count; ++i) {
        int neighbor_id;
        int unique;
        do {
            unique = 1;
            neighbor_id = rand() % max_node_id;
            if (neighbor_id == id) {
                unique = 0;
                continue;
            }

            for (int j = 0; j < i; ++j) {
                if (node->neighbors[j]->neighbor_id == neighbor_id) {
                    unique = 0;
                    break;
                }
            }
        } while (!unique);

        node->neighbors[i] = create_neighbor_info(neighbor_id);
    }

    // Initialisation for the algo
    node->li = NULL;      /*No List of list of path at the begining*/
    node->li_count = 0;   
    node->isLeaf = false;
    node->isCircleNode = false;
    node->isOriented = false;

    return node;
}

void print_node(const Node* node) {
    printf("Node ID: %d\n", node->id);
    printf("Neighbors (%d):\n", node->neighbor_count);
    for (int i = 0; i < node->neighbor_count; ++i) {
        print_neighbor_info(node->neighbors[i]);
    }
}

void merge_pathlists(Node* node, PathList* received, int received_count) {
    int new_count = node->li_count + received_count;
    node->li = realloc(node->li, new_count * sizeof(PathList));
    /*update the old path_list with the recived one*/
    for (int i = 0; i < received_count; ++i) {
        node->li[node->li_count + i].length = received[i].length;
        node->li[node->li_count + i].ids = malloc(received[i].length * sizeof(int));
        for (int j = 0; j < received[i].length; ++j) {
            node->li[node->li_count + i].ids[j] = received[i].ids[j];
        }
    }
    node->li_count = new_count;
}

int detect_is_a_leaf(Node* node){
    if (node->neighbor_count==1){
        return 1;
    }
    return 0;
}

void update_is_a_leaf(Node* node) {
    node->isLeaf = true;
    node->isOriented = true;
    node->neighbors[0]->direction = INCOMING; 
}


int detect_cycle_in_pathlists(const Node* node) {
    /*return 1 if a cycle is detected in one of the path lists*/
    for (int i = 0; i < node->li_count; ++i) {
        for (int j = 0; j < node->li[i].length; ++j) {
            int id = node->li[i].ids[j];
            for (int k = j + 1; k < node->li[i].length; ++k) {
                if (node->li[i].ids[k] == id) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

/* extract_cycle à faire qui va renvoyer le cycle en question*/

void append_id_to_all_pathlists(Node* node) {
    /*update the path list with my id*/
    for (int i = 0; i < node->li_count; ++i) {
        node->li[i].ids = realloc(node->li[i].ids, (node->li[i].length + 1) * sizeof(int));
        node->li[i].ids[node->li[i].length] = node->id;
        node->li[i].length += 1;
    }
}



void clear_pathlists(Node* node) {
    if (node->li) {
        free_pathlists(node->li, node->li_count);
        node->li = NULL;
        node->li_count = 0;
    }
}


void set_pathlist_to_self(Node* node) {
    clear_pathlists(node);
    node->li = (PathList*)malloc(sizeof(PathList));
    node->li[0].length = 1;
    node->li[0].ids = (int*)malloc(sizeof(int));
    node->li[0].ids[0] = node->id;
    node->li_count = 1;
}



void free_pathlists(PathList* lists, int count) {
    for (int i = 0; i < count; ++i) {
        free(lists[i].ids);
    }
    free(lists);
}

void free_node(Node* node) {
    for (int i = 0; i < node->neighbor_count; ++i) {
        free(node->neighbors[i]);
    }
    free(node->neighbors);
    if (node->li)
        free_pathlists(node->li, node->li_count);
    free(node);
}

