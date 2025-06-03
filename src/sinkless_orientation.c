#include "../include/sinkless_orientation.h"
#include <stdlib.h>
#include <stdio.h>

SinklessGraph* graph_to_sinklessgraph(const Graph* graph) {
    SinklessGraph* sg = malloc(sizeof(SinklessGraph));
    sg->node_count = graph->node_count;
    sg->nodes = malloc(sg->node_count * sizeof(SinklessNode*));
    for (int i = 0; i < sg->node_count; ++i) {
        sg->nodes[i] = malloc(sizeof(SinklessNode));
        sg->nodes[i]->status = NODE_UNORIENTED;
        sg->nodes[i]->pathlists = NULL;
        sg->nodes[i]->node = graph->nodes[i];
    }
    return sg;
}

void print_message(const Message* msg) {
    switch (msg->type) {
        case MSG_PATHLIST:
            printf("Message: PATHLIST, %d paths\n", msg->paths ? msg->paths->count : 0);
            if (msg->paths) {
                for (int i = 0; i < msg->paths->count; ++i) {
                    printf("  Path %d: [", i);
                    for (int j = 0; j < msg->paths->paths[i].length; ++j) {
                        printf("%d", msg->paths->paths[i].ids[j]);
                        if (j < msg->paths->paths[i].length - 1) printf(",");
                    }
                    printf("]\n");
                }
            }
            break;
        case MSG_CYCLE:
            printf("Message: CYCLE, length %d, ids: [", msg->cycle_length);
            for (int i = 0; i < msg->cycle_length; ++i) {
                printf("%d", msg->cycle_ids[i]);
                if (i < msg->cycle_length - 1) printf(",");
            }
            printf("]\n");
            break;
        case MSG_HAS_OUTGOING:
            printf("Message: HAS_OUTGOING\n");
            break;
        default:
            printf("Message: UNKNOWN\n");
    }
}

void print_sinklessgraph(const SinklessGraph* sg) {
    printf("SinklessGraph with %d nodes:\n", sg->node_count);
    for (int i = 0; i < sg->node_count; ++i) {
        SinklessNode* sn = sg->nodes[i];
        print_node(sg->nodes[i]->node);
        printf("status=");
        switch (sn->status) {
            case NODE_UNORIENTED: printf("UNORIENTED"); break;
            case NODE_LEAF: printf("LEAF"); break;
            case NODE_IN_CYCLE: printf("IN_CYCLE"); break;
            case NODE_PARTIALLY_ORIENTED: printf("PATIALLY ORIENTED"); break;
            case NODE_ORIENTED: printf("ORIENTED"); break;
            default: printf("UNKNOWN");
        }
        if (sn->pathlists)
            printf(", pathlist_count=%d\n", sn->pathlists->count);
        else
            printf(", pathlist_count=0\n");
        // Affiche les chemins si besoin
        if (sn->pathlists) {
            for (int j = 0; j < sn->pathlists->count; ++j) {
                printf("  Path %d: [", j);
                for (int k = 0; k < sn->pathlists->paths[j].length; ++k) {
                    printf("%d", sn->pathlists->paths[j].ids[k]);
                    if (k <  sn->pathlists->paths[j].length) printf(",");
                }
                printf("]\n");
            }
        }
        printf("\n");
    }
}

int is_leaf(const SinklessNode* sn) {
    if (!sn || !sn->node) return 0;
    return (sn->node->neighbor_count == 1);
}

int is_partially_oriented(const SinklessNode* sn) {
    if (!sn) return 0;
    return (sn->status == NODE_PARTIALLY_ORIENTED);
}

void update_is_a_leaf(SinklessNode* sn) {
    if (!sn || !sn->node || sn->node->neighbor_count != 1) {
        printf("problem in orienting the leaf");
    }
    // orient your edge incomming
    sn->node->neighbors[0]->direction = INCOMING;

    sn->status = NODE_LEAF;
    
}

void update_is_partially_oriented(SinklessNode* sn) {
    if (!sn || !sn->node) return;

    int has_outgoing = 0;
    int n_neighbors = sn->node->neighbor_count;

    // Vérifie s'il y a déjà une arête OUTGOING
    for (int i = 0; i < n_neighbors; ++i) {
        if (sn->node->neighbors[i]->direction == OUTGOING) {
            has_outgoing = 1;
            break;
        }
    }
    if (!has_outgoing) return; // Rien à faire si aucune arête OUTGOING

    // Oriente toutes les autres arêtes vers le nœud courant
    for (int i = 0; i < n_neighbors; ++i) {
        if (sn->node->neighbors[i]->direction != OUTGOING) {
            if (sn->node->neighbors[i]->neighbor_id < sn->node->id){
                sn->node->neighbors[i]->direction = INCOMING;
            }
        }
    }
    // regarde si toute les arete on été orientes 
    int unoriented = 0 ; 
    for (int i = 0; i < n_neighbors; ++i) {
        if (sn->node->neighbors[i]->direction == UNKNOWN) {
            unoriented ++;
        }
    }
    if (unoriented == 0){
        sn->status = NODE_ORIENTED;
    }else{
        sn->status = NODE_PARTIALLY_ORIENTED;
    }
}

void print_all_message(Message*** outgoing, int n, SinklessGraph* SG) {
    for (int i = 0; i < n; ++i) {
        SinklessNode* node = SG->nodes[i];
        printf("Messages envoyés par le noeud %d :\n", node->node->id);
        for (int j = 0; j < node->node->neighbor_count; ++j) {
            if (outgoing[i][j]) {
                printf("  Vers voisin %d : ", node->node->neighbors[j]->neighbor_id);
                print_message(outgoing[i][j]);
            } else {
                printf("  Vers voisin %d : (aucun message)\n", node->node->neighbors[j]->neighbor_id);
            }
        }
    }
}


int run_sinkless_orientation(Graph* graph) {
    if (!graph) {
        printf("Error with the creation of the sinkless grapgh\n");
        return -1;
    }

    // Transforme le graph en sinklessgraph
    SinklessGraph* SG = graph_to_sinklessgraph(graph);

    //print_sinklessgraph(SG);

    int round = 0;
    int changed = 1;
    int n = SG->node_count;

    // Tableau des messages à envoyer à chaque voisin
    Message*** outgoing = malloc(n * sizeof(Message**));

    while (changed) {
        changed = 0;
        printf("\n--- Round %d ---\n", round);

        // 1. Phase A : chaque node évalue son état local
        int* became_oriented_leaf = calloc(n, sizeof(int));


        for (int i = 0; i < n; ++i) {
            SinklessNode* node = SG->nodes[i];
            if (node->status == NODE_ORIENTED || node->status == NODE_LEAF) continue;

            // Détection feuille
            if (is_leaf(node)) {
                printf("\n--- Node %d is a leaf ---\n", node->node->id);
                update_is_a_leaf(node);
                became_oriented_leaf[i] = 1;
                changed = 1;
                continue;
            }
            // Detection is partially oriented 
            if (is_partially_oriented(node)){
                printf("\n--- Node %d is a partially oriented ---\n", node->node->id);
                update_is_partially_oriented(node);
                became_oriented_leaf[i] = 1;
                changed = 1;
                continue;
            }


            // Détection cycle
            
            }

        // 2. Phase B : préparation des messages à envoyer à chaque voisin
        for (int i = 0; i < n; ++i) {
            SinklessNode* node = SG->nodes[i];
            outgoing[i] = malloc(node->node->neighbor_count* sizeof(Message*));
            for (int j = 0; j < node->node->neighbor_count; ++j) {
                outgoing[i][j] = NULL;
                // The node became oriented (was a leaf or was partially oriented)
                if (became_oriented_leaf[i]) {
                    if (node->node->neighbors[j]->direction == INCOMING){
                        Message* msg = malloc(sizeof(Message));
                        msg->type = MSG_HAS_OUTGOING;
                        msg->paths = NULL;
                        msg->cycle_ids = NULL;
                        msg->cycle_length = 0;
                        outgoing[i][j] = msg;
                    }
                }
                // Si le node vient de détecter un cycle, envoie MSG_CYCLE à chaque voisin du cycle
                
                // Sinon, envoie les listes de chemins comme d'habitude
                
            }
            }

        // 3. Phase C : réception des messages (chaque node reçoit de ses voisins)
        print_all_message(outgoing, SG->node_count, SG);

        for (int i = 0; i < n; ++i) {
            SinklessNode* node = SG->nodes[i];

            if (node->status == NODE_ORIENTED|| node->status == NODE_LEAF) continue;

            for (int j = 0; j < node->node->neighbor_count; ++j) {
                int neighbor_id = node->node->neighbors[j]->neighbor_id;
                Node* neighbor = SG->nodes[neighbor_id]->node;
                // on récupere l'index de la position de node dans la liste des voisins de node voisin 
                int idx = -1;
                for (int k = 0; k < neighbor->neighbor_count; ++k) {
                    if (neighbor->neighbors[k]->neighbor_id == node->node->id) {
                        idx = k;
                        break;
                    }
                }

                if (idx != -1 && outgoing[neighbor_id][idx] != NULL) { // Le voisin a un message pour node !
                    Message* msg = outgoing[neighbor_id][idx];

                    // Le message est un MSG_HAS_OUTGOING
                    if (msg->type == MSG_HAS_OUTGOING) {
                        // Oriente l'arête sortante vers ce voisin
                        for (int m = 0; m < node->node->neighbor_count; ++m) {
                            if (node->node->neighbors[m]->neighbor_id == neighbor_id) {
                                node->node->neighbors[m]->direction = OUTGOING;
                                node->status = NODE_PARTIALLY_ORIENTED;
                                break;
                            }
                        }
                    }
                    // Le message est un MSG_PATHLIST

                    // Le message est un MSG_CYCLE
                    }
                }
            }

        // Libération des messages temporaires et cycles
        for (int i = 0; i < n; ++i) {
            SinklessNode* node = SG->nodes[i];
            for (int j = 0; j < node->node->neighbor_count; ++j) {
                if (outgoing[i][j]) {
                    if (outgoing[i][j]->type == MSG_PATHLIST && outgoing[i][j]->paths) {
                        for (int k = 0; k < outgoing[i][j]->paths->count; ++k) {
                            free(outgoing[i][j]->paths->paths[k].ids);
                        }
                        free(outgoing[i][j]->paths->paths);
                        free(outgoing[i][j]->paths);
                    }
                    if (outgoing[i][j]->type == MSG_CYCLE && outgoing[i][j]->cycle_ids) {
                        free(outgoing[i][j]->cycle_ids);
                    }
                    free(outgoing[i][j]);
                }
            }
            free(outgoing[i]);
        }
        
        free(became_oriented_leaf);
        //print_sinklessgraph(SG);
        round++;
        }
    
    free(outgoing);

    printf("\n--- Orientation terminée en %d rounds ---\n", round);
    
    orient_graph_from_sinklessgraph(graph, SG);

    //print_sinklessgraph(SG);

    free_sinklessgraph(SG);

    return round;
}

void orient_graph_from_sinklessgraph(Graph* graph, const SinklessGraph* SG) {
    if (!graph || !SG) return;
    for (int i = 0; i < graph->node_count; ++i) {
        Node* node = graph->nodes[i];
        SinklessNode* sn = SG->nodes[i];
        for (int j = 0; j < node->neighbor_count; ++j) {
            node->neighbors[j]->direction = sn->node->neighbors[j]->direction;
        }
    }
}

void free_sinklessgraph(SinklessGraph* sg) {
    if (!sg) return;
    for (int i = 0; i < sg->node_count; ++i) {
        SinklessNode* sn = sg->nodes[i];
        if (sn) {
            // Libère les pathlists si elles existent
            if (sn->pathlists) {
                for (int j = 0; j < sn->pathlists->count; ++j) {
                    free(sn->pathlists->paths[j].ids);
                }
                free(sn->pathlists->paths);
                free(sn->pathlists);
            }
            free(sn);
        }
    }
    free(sg->nodes);
    free(sg);
}

/*
void run_sinkless_orientation(Graph* graph) {
    int round = 0;
    int changed = 1;
    int n = graph->node_count;

    // Tableau des messages à envoyer à chaque voisin
    Message*** outgoing = malloc(n * sizeof(Message**));

    while (changed) {
        changed = 0;
        printf("\n--- Round %d ---\n", round);

        // 1. Phase A : chaque node évalue son état local
        int* became_leaf = calloc(n, sizeof(int));
        int* became_cycle = calloc(n, sizeof(int));
        int** cycles = malloc(n * sizeof(int*));
        int* cycle_lengths = calloc(n, sizeof(int));

        for (int i = 0; i < n; ++i) {
            Node* node = graph->nodes[i];
            if (node->isOriented) continue;

            // Détection feuille
            int leaf = detect_is_a_leaf(node);
            if (leaf == 1) {
                printf("\n--- Node %d is a leaf ---\n", node->id);
                update_is_a_leaf(node);
                became_leaf[i] = 1;
                changed = 1;
                continue;
            }

            // Détection cycle
            if (detect_cycle_in_pathlists(node)) {
                printf("\n--- Node %d has a cycle ---\n", node->id);
                int length_cycle;
                int* cycle = extract_cycle_ids(node->li, node->id, &length_cycle);
                node->isCircleNode = 1;
                node->isOriented = 1;
                update_node_in_cycle(node, cycle, length_cycle);
                became_cycle[i] = 1;
                cycles[i] = cycle;
                cycle_lengths[i] = length_cycle;
                changed = 1;
                continue;
            }
        }

        // 2. Phase B : préparation des messages à envoyer à chaque voisin
        for (int i = 0; i < n; ++i) {
            Node* node = graph->nodes[i];
            outgoing[i] = malloc(node->neighbor_count * sizeof(Message*));
            for (int j = 0; j < node->neighbor_count; ++j) {
                outgoing[i][j] = NULL;
                // Si le node vient de devenir feuille, envoie MSG_HAS_OUTGOING à son unique voisin
                if (became_leaf[i] && node->neighbors[j]->direction == INCOMING) {
                    Message* msg = malloc(sizeof(Message));
                    msg->type = MSG_HAS_OUTGOING;
                    msg->paths = NULL;
                    msg->path_count = 0;
                    msg->cycle_ids = NULL;
                    msg->cycle_length = 0;
                    outgoing[i][j] = msg;
                }
                // Si le node vient de détecter un cycle, envoie MSG_CYCLE à chaque voisin du cycle
                else if (became_cycle[i] && node->neighbors[j]->direction == OUTGOING) {
                    Message* msg = malloc(sizeof(Message));
                    msg->type = MSG_CYCLE;
                    msg->paths = NULL;
                    msg->path_count = 0;
                    msg->cycle_length = cycle_lengths[i];
                    msg->cycle_ids = malloc(msg->cycle_length * sizeof(int));
                    memcpy(msg->cycle_ids, cycles[i], msg->cycle_length * sizeof(int));
                    outgoing[i][j] = msg;
                }
                // Sinon, envoie les listes de chemins comme d'habitude
                else if (node->neighbors[j]->direction == UNKNOWN && !node->isOriented) {
                    int count;
                    PathList* to_send = copy_pathlists(node, &count);
                    for (int k = 0; k < count; ++k) {
                        to_send[k].ids = realloc(to_send[k].ids, (to_send[k].length + 1) * sizeof(int));
                        to_send[k].ids[to_send[k].length] = node->id;
                        to_send[k].length++;
                    }
                    Message* msg = malloc(sizeof(Message));
                    msg->type = MSG_PATHLIST;
                    msg->paths = to_send;
                    msg->path_count = count;
                    msg->cycle_ids = NULL;
                    msg->cycle_length = 0;
                    outgoing[i][j] = msg;
                }
            }
        }

        // 3. Phase C : réception des messages (chaque node reçoit de ses voisins)
        for (int i = 0; i < n; ++i) {
            Node* node = graph->nodes[i];
            if (node->isOriented) continue;
            for (int j = 0; j < node->neighbor_count; ++j) {
                int neighbor_id = node->neighbors[j]->neighbor_id;
                Node* neighbor = graph->nodes[neighbor_id];
                int idx = -1;
                for (int k = 0; k < neighbor->neighbor_count; ++k) {
                    if (neighbor->neighbors[k]->neighbor_id == node->id) {
                        idx = k;
                        break;
                    }
                }
                if (idx != -1 && outgoing[neighbor_id][idx] != NULL) {
                    Message* msg = outgoing[neighbor_id][idx];
                    if (msg->type == MSG_PATHLIST) {
                        merge_pathlists(node, msg->paths, msg->path_count);
                    } else if (msg->type == MSG_HAS_OUTGOING) {
                        // Oriente l'arête sortante vers ce voisin
                        for (int m = 0; m < node->neighbor_count; ++m) {
                            if (node->neighbors[m]->neighbor_id == neighbor_id) {
                                node->neighbors[m]->direction = OUTGOING;
                                node->isOriented = 1;
                                break;
                            }
                        }
                    } else if (msg->type == MSG_CYCLE) {
                        update_node_in_cycle(node, msg->cycle_ids, msg->cycle_length);
                    }
                }
            }
        }

        // Libération des messages temporaires et cycles
        for (int i = 0; i < n; ++i) {
            Node* node = graph->nodes[i];
            for (int j = 0; j < node->neighbor_count; ++j) {
                if (outgoing[i][j]) {
                    if (outgoing[i][j]->type == MSG_PATHLIST && outgoing[i][j]->paths) {
                        for (int k = 0; k < outgoing[i][j]->path_count; ++k) {
                            free(outgoing[i][j]->paths[k].ids);
                        }
                        free(outgoing[i][j]->paths);
                    }
                    if (outgoing[i][j]->type == MSG_CYCLE && outgoing[i][j]->cycle_ids) {
                        free(outgoing[i][j]->cycle_ids);
                    }
                    free(outgoing[i][j]);
                }
            }
            free(outgoing[i]);
            if (cycles[i]) free(cycles[i]);
        }
        free(outgoing);
        outgoing = malloc(n * sizeof(Message**));
        free(became_leaf);
        free(became_cycle);
        free(cycles);
        free(cycle_lengths);

        round++;
    }

    printf("\n--- Orientation terminée en %d rounds ---\n", round);
}*/