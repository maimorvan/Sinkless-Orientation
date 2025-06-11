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
    if (!has_outgoing){
        sn->status = NODE_UNORIENTED;
        return; // Rien à faire si aucune arête OUTGOING
    }

    // Oriente toutes les autres arêtes vers le nœud courant si le voisin est plus petit que soi
    /*
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
    }*/
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

PathList* copy_and_extend_pathlists(const PathList* src, int node_id) {
    if (!src || src->count == 0){
        PathList* res = malloc(sizeof(PathList));
        res->count = 1;
        res->paths = malloc(res->count * sizeof(Path));
        res->paths[0].length = 1;
        res->paths[0].ids = malloc(1 * sizeof(int));
        res->paths[0].ids[0] = node_id;
        return res;
    }
    PathList* res = malloc(sizeof(PathList));
    res->count = src->count;
    res->paths = malloc(res->count * sizeof(Path));
    for (int i = 0; i < src->count; ++i) {
        res->paths[i].length = src->paths[i].length + 1;
        res->paths[i].ids = malloc(res->paths[i].length * sizeof(int));
        // Copie l'ancien chemin
        for (int j = 0; j < src->paths[i].length; ++j) {
            res->paths[i].ids[j] = src->paths[i].ids[j];
        }
        // Ajoute l'id courant à la fin
        res->paths[i].ids[res->paths[i].length - 1] = node_id;
    }
    return res;
}

PathList* concat_pathlists(const PathList* a, const PathList* b) {
    if (!a && !b) return NULL;
    if (!a) return copy_pathlist(b); 
    if (!b) return  copy_pathlist(a);

    PathList* res = malloc(sizeof(PathList));
    res->count = a->count + b->count;
    res->paths = malloc(res->count * sizeof(Path));
    int idx = 0;
    for (int i = 0; i < a->count; ++i, ++idx) {
        res->paths[idx].length = a->paths[i].length;
        res->paths[idx].ids = malloc(a->paths[i].length * sizeof(int));
        for (int j = 0; j < a->paths[i].length; ++j)
            res->paths[idx].ids[j] = a->paths[i].ids[j];
    }
    for (int i = 0; i < b->count; ++i, ++idx) {
        res->paths[idx].length = b->paths[i].length;
        res->paths[idx].ids = malloc(b->paths[i].length * sizeof(int));
        for (int j = 0; j < b->paths[i].length; ++j)
            res->paths[idx].ids[j] = b->paths[i].ids[j];
    }
    return res;
}

Message* make_pathlist_message(const PathList* pathlists, int node_id, int neighbor_id) {
    if (!pathlists || pathlists->count == 0) {
        // Cas de base : on envoie juste notre id
        PathList* to_send = copy_and_extend_pathlists(NULL, node_id);
        Message* msg = malloc(sizeof(Message));
        msg->type = MSG_PATHLIST;
        msg->paths = to_send;
        msg->cycle_ids = NULL;
        msg->cycle_length = 0;
        return msg;
    }

    // Compte les chemins à garder (on ne veux pas répondre à un message qu'on vient de nous evoyer dans la meme direction)
    int count = 0;
    for (int i = 0; i < pathlists->count; ++i) {
        if (pathlists->paths[i].length == 0) continue;
        if (pathlists->paths[i].ids[pathlists->paths[i].length - 1] != neighbor_id) {
            count++;
        }
    }

    PathList* filtered = malloc(sizeof(PathList));
    filtered->count = count;
    filtered->paths = malloc(count * sizeof(Path));
    int idx = 0;
    for (int i = 0; i < pathlists->count; ++i) {
        if (pathlists->paths[i].length == 0) continue;
        if (pathlists->paths[i].ids[pathlists->paths[i].length - 1] != neighbor_id) {
            filtered->paths[idx].length = pathlists->paths[i].length + 1;
            filtered->paths[idx].ids = malloc(filtered->paths[idx].length * sizeof(int));
            for (int j = 0; j < pathlists->paths[i].length; ++j) {
                filtered->paths[idx].ids[j] = pathlists->paths[i].ids[j];
            }
            filtered->paths[idx].ids[filtered->paths[idx].length - 1] = node_id;
            idx++;
        }
    }

    // Si aucun chemin à envoyer, on envoie juste notre id
    if (filtered->count == 0) {
        free(filtered->paths);
        free(filtered);
        filtered = copy_and_extend_pathlists(NULL, node_id);
    }

    Message* msg = malloc(sizeof(Message));
    msg->type = MSG_PATHLIST;
    msg->paths = filtered;
    msg->cycle_ids = NULL;
    msg->cycle_length = 0;
    return msg;
}

PathList* copy_pathlist(const PathList* src) {
    if (!src || src->count == 0) return NULL;
    PathList* res = malloc(sizeof(PathList));
    res->count = src->count;
    res->paths = malloc(res->count * sizeof(Path));
    for (int i = 0; i < src->count; ++i) {
        res->paths[i].length = src->paths[i].length;
        res->paths[i].ids = malloc(res->paths[i].length * sizeof(int));
        for (int j = 0; j < src->paths[i].length; ++j) {
            res->paths[i].ids[j] = src->paths[i].ids[j];
        }
    }
    return res;
}


int* detect_cycle(const SinklessNode* node, int* len_cycle, int* orient) {
    if (!node || !node->pathlists) {
        *len_cycle = 0;
        *orient = 0;
        return NULL;
    }
    int my_id = node->node->id;
    for (int i = 0; i < node->pathlists->count; ++i) {
        const Path* path = &node->pathlists->paths[i];
        for (int j = 0; j < path->length - 1; ++j) {
            if (path->ids[j] == my_id) {
                int len = path->length - j;
                int* cycle = malloc(len * sizeof(int));
                int max_id = my_id; 
                int max_pos = 0;
                int orientation = 0; 
                for (int k = 0; k < len; ++k) {
                    cycle[k] = path->ids[j + k];
                    if (path->ids[j+k]>max_id){
                        max_id = path->ids[j+k]; 
                        max_pos = k; 
                    }
                }
                if (max_id == my_id){
                    if (cycle[len-1]>cycle[1]){
                        orientation = -1; 
                    }else{
                        orientation = 1; 
                    }
                }else{
                    if (cycle[max_pos-1]>cycle[(max_pos+1+len)%len]){
                            orientation = -1; 
                        }else{
                            orientation = +1;
                    }
                }
                *len_cycle = len; 
                *orient = orientation;
                return cycle;
            }
        }  
    }
    *len_cycle = 0;
    *orient = 0;
    return NULL;
}

void update_cycle(const SinklessNode* node, int* cycle, int len, int orientation){
    if (!node || !cycle || len < 2) return;
    int my_id = node->node->id;
    int pos = -1;

    // Debug print cycle 
    printf("Cycle détecté (len=%d, orientation=%d) : [", len, orientation);
    for (int i = 0; i < len; ++i) {
        printf("%d", cycle[i]);
        if (i < len - 1) printf(",");
    }
    printf("]\n");

    // Trouver la position de notre noeud dans le cycle
    for (int i = 0; i < len; ++i) {
        if (cycle[i] == my_id) {
            pos = i;
            break;
        }
    }
    if (pos == -1) return; // Notre noeud n'est pas dans le cycle

    // Oriente l'arête du cycle dans la bonne direction (OUTGOING)
    int next_pos = (pos + orientation + len) % len;
    int neighbor_id = cycle[next_pos];
    for (int i = 0; i < node->node->neighbor_count; ++i) {
        if (node->node->neighbors[i]->neighbor_id == neighbor_id) {
            node->node->neighbors[i]->direction = OUTGOING;
        }
    }

    // Oriente l'arête du cycle dans le sens opposé (INCOMING)
    int prev_pos = (pos - orientation + len) % len;
    int prev_neighbor_id = cycle[prev_pos];
    for (int i = 0; i < node->node->neighbor_count; ++i) {
        if (node->node->neighbors[i]->neighbor_id == prev_neighbor_id) {
            node->node->neighbors[i]->direction = INCOMING;
        }
    }

    // Oriente les autres arêtes (hors cycle) vers nous si l'id du voisin est inférieur
    /*
    for (int i = 0; i < node->node->neighbor_count; ++i) {
        int nid = node->node->neighbors[i]->neighbor_id;
        // Vérifie si ce voisin est dans le cycle
        int in_cycle = 0;
        for (int j = 0; j < len; ++j) {
            if (cycle[j] == nid) {
                in_cycle = 1;
                break;
            }
        }
        if (!in_cycle) {
            node->node->neighbors[i]->direction = INCOMING;
        }
    }
    */
    // Mets à jour le statut
    ((SinklessNode*)node)->status = NODE_IN_CYCLE;

}


int fin_algo(SinklessGraph* g){
    for (int i = 0; i < g->node_count; ++i) {
        if (g->nodes[i]->status == NODE_UNORIENTED) {
            return 0;
        }
    }
    return 1;
}

void finalize_orientations(SinklessGraph* g) {
    for (int i = 0; i < g->node_count; ++i) {
        SinklessNode* sn = g->nodes[i];
        Node* node = sn->node;
        for (int j = 0; j < node->neighbor_count; ++j) {
            NeighborInfo* neighbor = node->neighbors[j];
            int neighbor_id = neighbor->neighbor_id;
            Node* other = g->nodes[neighbor_id]->node;

            // Cherche l'index de node dans la liste des voisins de other
            int idx_other = -1;
            for (int k = 0; k < other->neighbor_count; ++k) {
                if (other->neighbors[k]->neighbor_id == node->id) {
                    idx_other = k;
                    break;
                }
            }
            if (idx_other == -1) continue; // sécurité

            // Si déjà cohérent, rien à faire
            if (neighbor->direction != UNKNOWN && other->neighbors[idx_other]->direction != UNKNOWN)
                continue;

            // Oriente dans la direction du plus grand id
            if (neighbor->direction == UNKNOWN && other->neighbors[idx_other]->direction == UNKNOWN) {
                if (node->id > neighbor_id) {
                    neighbor->direction = OUTGOING;
                    other->neighbors[idx_other]->direction = INCOMING;
                } else {
                    neighbor->direction = INCOMING;
                    other->neighbors[idx_other]->direction = OUTGOING;
                }
            } else if (neighbor->direction != UNKNOWN && other->neighbors[idx_other]->direction == UNKNOWN) {
                other->neighbors[idx_other]->direction = (neighbor->direction == OUTGOING) ? INCOMING : OUTGOING;
            } else if (neighbor->direction == UNKNOWN && other->neighbors[idx_other]->direction != UNKNOWN) {
                neighbor->direction = (other->neighbors[idx_other]->direction == OUTGOING) ? INCOMING : OUTGOING;
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


    while (changed < n) {
        changed +=1;
        printf("\n--- Round %d ---\n", round);

        // 1. Phase A : chaque node évalue son état local
        int* became_oriented_leaf = calloc(n, sizeof(int));
        int* became_cycle = calloc(n, sizeof(int));


        for (int i = 0; i < n; ++i) {
            SinklessNode* node = SG->nodes[i];
            if (node->status == NODE_ORIENTED || node->status == NODE_LEAF) continue;

            // Détection feuille
            if (is_leaf(node)) {
                printf("\n--- Node %d is a leaf ---\n", node->node->id);
                update_is_a_leaf(node);
                became_oriented_leaf[i] = 1;
                //changed = 1;
                continue;
            }
            // Detection is partially oriented 
            if (is_partially_oriented(node)){
                printf("\n--- Node %d is a partially oriented ---\n", node->node->id);
                update_is_partially_oriented(node);
                became_oriented_leaf[i] = 1;
                continue;
            }


            // Détection cycle
            int len_cycle = 0; 
            int orientation = 0; 
            int* cycle = detect_cycle(node,&len_cycle, &orientation);
            if (cycle != NULL){
                printf("\n--- Node %d is a cycle ---\n", node->node->id);
                update_cycle(node,cycle,len_cycle,orientation);
                became_cycle[i] = 1;
                free(cycle);
                continue;
            }
            
            }

        // 2. Phase B : préparation des messages à envoyer à chaque voisin
        for (int i = 0; i < n; ++i) {
            SinklessNode* node = SG->nodes[i];
            outgoing[i] = malloc(node->node->neighbor_count* sizeof(Message*));
            for (int j = 0; j < node->node->neighbor_count; ++j) {
                outgoing[i][j] = NULL;
                // The node became oriented (was a leaf or was partially oriented)
                if (became_oriented_leaf[i]) {
                    if (node->node->neighbors[j]->direction != OUTGOING){
                        Message* msg = malloc(sizeof(Message));
                        msg->type = MSG_HAS_OUTGOING;
                        msg->paths = NULL;
                        msg->cycle_ids = NULL;
                        msg->cycle_length = 0;
                        outgoing[i][j] = msg;
                    }
                }
                // Si le node vient de détecter un cycle, envoie MSG_CYCLE à chaque voisin du cycle
                else if (became_cycle[i]) {
                    if (node->node->neighbors[j]->direction != OUTGOING){
                        Message* msg = malloc(sizeof(Message));
                        msg->type = MSG_HAS_OUTGOING;
                        msg->paths = NULL;
                        msg->cycle_ids = NULL;
                        msg->cycle_length = 0;
                        outgoing[i][j] = msg;
                    }
                }
                
                // Sinon, envoie les listes de chemins comme d'habitude
                else if (node->node->neighbors[j]->direction == UNKNOWN && !(node->status == NODE_ORIENTED || node->status == NODE_LEAF 
                || node->status == NODE_IN_CYCLE)) {
                    Message* msg = make_pathlist_message(node->pathlists, node->node->id, node->node->neighbors[j]->neighbor_id);
                    outgoing[i][j] = msg;
                }
                
            }
            // Netoyage de notre pathlist 
            if (node->pathlists) {
                free_pathlist(node->pathlists);
                node->pathlists = NULL;
            }

            }

        // 3. Phase C : réception des messages (chaque node reçoit de ses voisins)
        //print_all_message(outgoing, SG->node_count, SG);

        for (int i = 0; i < n; ++i) {
            SinklessNode* node = SG->nodes[i];

            if (node->status != NODE_UNORIENTED) continue;

            int max_neighbor_id = -1;
            
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
                        if (neighbor_id>max_neighbor_id){
                            max_neighbor_id = neighbor_id;
                        }
                    }
                    // Le message est un MSG_PATHLIST
                    else if (msg->type == MSG_PATHLIST) {
                        // Fusionne la pathlist reçue avec la tienne
                        PathList* new_list = NULL;
                        if (!node->pathlists) {
                            new_list = copy_pathlist(msg->paths);
                        } else {
                            new_list = concat_pathlists(node->pathlists, msg->paths);
                        }
                        free_pathlist(node->pathlists);
                        node->pathlists = new_list;
                    }

                    // Le message est un MSG_CYCLE
                    }
                }
        
                for (int m = 0; m < node->node->neighbor_count; ++m) {
                    if (node->node->neighbors[m]->neighbor_id == max_neighbor_id) {
                        node->node->neighbors[m]->direction = OUTGOING;
                        node->status = NODE_PARTIALLY_ORIENTED;
                        break;
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
        free(became_cycle);
        //print_sinklessgraph(SG);

        // Fin de l'algo ? 

        if (fin_algo(SG)==1){
            printf("Sortie Fini !");
            break;
        }


        round++;
        }
    
    free(outgoing);

    printf("\n--- Orientation terminée en %d rounds ---\n", round);

    finalize_orientations(SG);
    
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

void free_pathlist(PathList* pl) {
    if (!pl) return;
    for (int i = 0; i < pl->count; ++i) {
        free(pl->paths[i].ids);
    }
    free(pl->paths);
    free(pl);
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

