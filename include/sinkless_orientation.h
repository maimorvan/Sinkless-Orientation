#ifndef SINKLESS_ORIENTATION_H
#define SINKLESS_ORIENTATION_H

#include "graph.h"
#include "node.h" // to have detect_is_a_leaf, update_is_a_leaf

int detect_cycle_in_pathlists(const Node* node);

void run_sinkless_orientation(Graph* graph);

#endif