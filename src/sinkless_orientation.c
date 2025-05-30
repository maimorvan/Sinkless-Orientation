#include "../include/sinkless_orientation.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Fonction utilitaire pour copier les PathList d'un node (pour la simulation des messages)
PathList* copy_pathlists(const Node* node, int* out_count) {
    if (node->li_count == 0) {
        *out_count = 1;
        PathList* res = malloc(sizeof(PathList));
        res[0].length = 1;
        res[0].ids = malloc(sizeof(int));
        res[0].ids[0] = node->id;
        return res;
    }
    *out_count = node->li_count;
    PathList* res = malloc(node->li_count * sizeof(PathList));
    for (int i = 0; i < node->li_count; ++i) {
        res[i].length = node->li[i].length;
        res[i].ids = malloc(res[i].length * sizeof(int));
        memcpy(res[i].ids, node->li[i].ids, res[i].length * sizeof(int));
    }
    return res;
}

// Fonction principale
void run_sinkless_orientation(Graph* graph) {
    int round = 0;
    int changed = 1;
    int n = graph->node_count;

    // Initialisation : chaque node commence avec sa propre liste
    for (int i = 0; i < n; ++i) {
        set_pathlist_to_self(graph->nodes[i]);
    }

    while (changed) {
        changed = 0;
        printf("\n--- Round %d ---\n", round);

        // 1. Phase A : chaque node évalue son état local
        for (int i = 0; i < n; ++i) {
            Node* node = graph->nodes[i];
            if (node->isOriented) continue; /*the node is already oriented, nothing to do*/

            // Détection feuille
            int leaf = detect_is_a_leaf(node);

            if (leaf == 1) {
                // C'est une feuille
                printf("\n--- Node %d  is a leaf---\n", node->id);
                update_is_a_leaf(node);
                changed = 1;
                continue;
            }

            // Détection cycle
            
            if (detect_cycle_in_pathlists(node)) {

                printf("\n--- Node %d  has a cycle---\n", node->id);
                /*int* cycle = extract_cycle(node)*/
                node->isCircleNode = 1;
                node->isOriented = 1;
                // Oriente une arête sortante dans le cycle (ici, on prend la première possible)
                /*A faire*/
                changed = 1;
                continue;
            }
        }

        // 2. Phase B : préparation des messages à envoyer à chaque voisin
        // On prépare pour chaque node les messages à envoyer à chaque voisin
        PathList*** outgoing = malloc(n * sizeof(PathList**));
        int** outgoing_count = malloc(n * sizeof(int*));
        for (int i = 0; i < n; ++i) {
            Node* node = graph->nodes[i];
            outgoing[i] = malloc(node->neighbor_count * sizeof(PathList*));
            outgoing_count[i] = malloc(node->neighbor_count * sizeof(int));
            for (int j = 0; j < node->neighbor_count; ++j) {
                // On n'envoie que sur les arêtes non orientées
                if (node->neighbors[j]->direction == UNKNOWN && !node->isOriented) {
                    int count;
                    PathList* to_send = copy_pathlists(node, &count);
                    // Ajoute son id à chaque chemin
                    for (int k = 0; k < count; ++k) {
                        to_send[k].ids = realloc(to_send[k].ids, (to_send[k].length + 1) * sizeof(int));
                        to_send[k].ids[to_send[k].length] = node->id;
                        to_send[k].length++;
                    }
                    outgoing[i][j] = to_send;
                    outgoing_count[i][j] = count;
                } else {
                    outgoing[i][j] = NULL;
                    outgoing_count[i][j] = 0;
                }
            }
        }

        // 3. Phase C : réception des messages (chaque node reçoit de ses voisins)
        for (int i = 0; i < n; ++i) {
            Node* node = graph->nodes[i];
            if (node->isOriented) continue;
            // On va fusionner tous les messages reçus
            for (int j = 0; j < node->neighbor_count; ++j) {
                int neighbor_id = node->neighbors[j]->neighbor_id;
                // Cherche l'index du node courant dans les voisins du voisin
                Node* neighbor = graph->nodes[neighbor_id];
                int idx = -1;
                for (int k = 0; k < neighbor->neighbor_count; ++k) {
                    if (neighbor->neighbors[k]->neighbor_id == node->id) {
                        idx = k;
                        break;
                    }
                }
                if (idx != -1 && outgoing[neighbor_id][idx] != NULL) {
                    merge_pathlists(node, outgoing[neighbor_id][idx], outgoing_count[neighbor_id][idx]);
                }
            }
        }

        // Libération des messages temporaires
        for (int i = 0; i < n; ++i) {
            Node* node = graph->nodes[i];
            for (int j = 0; j < node->neighbor_count; ++j) {
                if (outgoing[i][j]) {
                    for (int k = 0; k < outgoing_count[i][j]; ++k) {
                        free(outgoing[i][j][k].ids);
                    }
                    free(outgoing[i][j]);
                }
            }
            free(outgoing[i]);
            free(outgoing_count[i]);
        }
        free(outgoing);
        free(outgoing_count);

        round++;
    }

    printf("\n--- Orientation terminée en %d rounds ---\n", round);
}