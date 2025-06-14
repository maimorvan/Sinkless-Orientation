#ifndef SINKLESS_ORIENTATION_H
#define SINKLESS_ORIENTATION_H

#include "graph.h"
#include "node.h" 

typedef struct {
    int* ids;      // A path 
    int length;    // Lengh of the path
} Path;

typedef struct {
    Path* paths;   // List of paths
    int count;     // Number of path
} PathList;

typedef enum {
    NODE_UNORIENTED,
    NODE_LEAF,
    NODE_IN_CYCLE,
    NODE_PARTIALLY_ORIENTED,
    NODE_ORIENTED
} SinklessStatus;

typedef enum {
    MSG_PATHLIST,
    MSG_CYCLE,
    MSG_HAS_OUTGOING, 
} MessageType;

typedef struct {
    MessageType type;
    PathList* paths;      // utilisé si type == MSG_PATHLIST
    int* cycle_ids;       // utilisé si type == MSG_CYCLE
    int cycle_length;
} Message;

typedef struct {
    SinklessStatus status;
    PathList* pathlists;
    Node* node;
} SinklessNode;

typedef struct {
    SinklessNode** nodes;      
    int node_count;  
} SinklessGraph;

SinklessGraph* graph_to_sinklessgraph(const Graph* graph);

void print_message(const Message* msg);

void print_sinklessgraph(const SinklessGraph* sg);

int is_leaf(const SinklessNode* sn);

int is_partially_oriented(const SinklessNode* sn);

void update_is_a_leaf(SinklessNode* sn);

void update_is_partially_oriented(SinklessNode* sn);

void print_all_message(Message*** outgoing, int n, SinklessGraph* SG);

PathList* copy_and_extend_pathlists(const PathList* src, int node_id);

PathList* concat_pathlists(const PathList* a, const PathList* b);

Message* make_pathlist_message(const PathList* pathlists, int node_id, int neighbor_id);

PathList* copy_pathlist(const PathList* src);

int* detect_cycle(const SinklessNode* node, int* len_cycle, int* orient);

void update_cycle(const SinklessNode* node, int* cycle, int len, int orientation);

int fin_algo(SinklessGraph* g);

void finalize_orientations(SinklessGraph* g);


int run_sinkless_orientation(Graph* graph);

void orient_graph_from_sinklessgraph(Graph* graph, const SinklessGraph* SG);

void free_pathlist(PathList* pl);

void free_sinklessgraph(SinklessGraph* sg);

#endif