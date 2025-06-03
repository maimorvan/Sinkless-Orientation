#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "../include/graph.h"

Graph* create_random_graph(int node_count, int min_nb_neighbors) {
    Node** nodes = (Node**)malloc(node_count * sizeof(Node*));
    srand(time(NULL));

    for (int i = 0; i < node_count; ++i) {
        nodes[i] = create_node(i);
        for (int j = 0; j < min_nb_neighbors; ++j) {
            int neighbor_id;
            int unique;
            do {
                unique = 1;
                neighbor_id = rand() % node_count;
                if (neighbor_id == i) {
                    unique = 0;
                    continue;
                }

                if(has_neighbor(nodes[i], neighbor_id)){
                    unique = 1;
                }
            } while (!unique);

            add_neighbor_one_side(nodes[i], neighbor_id);
        }
    }
    Graph* graph = (Graph*)malloc(sizeof(Graph));
    graph->nodes = nodes;
    graph->node_count = node_count;

    ensure_neighbors_symmetry(graph);

    return graph;
}

Graph* create_random_graph_delta(int node_count, int delta) {
    if (delta >= node_count || node_count == 0 || (node_count * delta) % 2 != 0) {
        printf("Impossible parameters\n");
        return NULL;
    }

    Node** nodes = malloc(node_count * sizeof(Node*));
    int* degrees = calloc(node_count, sizeof(int));

    for (int i = 0; i < node_count; ++i)
        nodes[i] = create_node(i);

    // Génère la liste de toutes les arêtes possibles (i < j)
    int max_edges = node_count * (node_count - 1) / 2;
    int (*edges)[2] = malloc(max_edges * sizeof(int[2]));
    int edge_count = 0;
    for (int i = 0; i < node_count; ++i){
        for (int j = i + 1; j < node_count; ++j){
            edges[edge_count][0] = i, edges[edge_count][1] = j;
            edge_count ++;
        }
    }

    // Mélange le tableau d'arêtes (Fisher-Yates)
    srand(time(NULL));
    for (int i = edge_count - 1; i > 0; --i) {
        int j = rand() % (i + 1);
        int tmp0 = edges[i][0], tmp1 = edges[i][1];
        edges[i][0] = edges[j][0]; edges[i][1] = edges[j][1];
        edges[j][0] = tmp0;        edges[j][1] = tmp1;
    }

    // Ajoute les arêtes tant que possible sans dépasser delta
    for (int i = 0; i < edge_count; ++i) {
        int a = edges[i][0], b = edges[i][1];
        if (degrees[a] < delta && degrees[b] < delta) {
            add_neighbor_both_side(nodes[a], nodes[b]);
            degrees[a]++;
            degrees[b]++;
        }
    }

    // Vérifie que tous les degrés sont bons
    int ok = 1;
    for (int i = 0; i < node_count; ++i)
        if (degrees[i] != delta) ok = 0;

    free(degrees);
    free(edges);

    if (!ok) {
        printf("Impossible de générer un graphe régulier avec ces paramètres.\n");
        for (int i = 0; i < node_count; ++i) free_node(nodes[i]);
        free(nodes);
        return NULL;
    }

    Graph* graph = malloc(sizeof(Graph));
    graph->nodes = nodes;
    graph->node_count = node_count;
    return graph;
}

void free_graph(Graph* graph) {
    for (int i = 0; i < graph->node_count; ++i) {
        free_node(graph->nodes[i]);
    }
    free(graph->nodes);
    free(graph);
}

void print_graph(const Graph* graph) {
    if (!graph) {
        printf("Erreur lors de la création du graphe.\n");
    }else{
    printf("Graph with %d nodes:\n", graph->node_count);
    for (int i = 0; i < graph->node_count; ++i) {
        print_node(graph->nodes[i]);
        printf("\n");
        }
    }
}


int has_neighbor(Node* node, int neighbor_id) {
    for (int i = 0; i < node->neighbor_count; ++i) {
        if (node->neighbors[i]->neighbor_id == neighbor_id) {
            return 1;
        }
    }
    return 0;
}

void add_neighbor_one_side(Node* node, int neighbor_id) {
    node->neighbors = realloc(node->neighbors, (node->neighbor_count + 1) * sizeof(NeighborInfo*));
    node->neighbors[node->neighbor_count] = create_neighbor_info(neighbor_id);
    node->neighbor_count++;
}

void add_neighbor_both_side(Node* node, Node* neighbor) {
    node->neighbors = realloc(node->neighbors, (node->neighbor_count + 1) * sizeof(NeighborInfo*));
    node->neighbors[node->neighbor_count] = create_neighbor_info(neighbor->id);
    node->neighbor_count++;
    neighbor->neighbors = realloc(neighbor->neighbors, (neighbor->neighbor_count + 1) * sizeof(NeighborInfo*));
    neighbor->neighbors[neighbor->neighbor_count] = create_neighbor_info(node->id);
    neighbor->neighbor_count++;
}

void ensure_neighbors_symmetry(Graph* graph) {
    for (int i = 0; i < graph->node_count; ++i) {
        Node* node = graph->nodes[i];
        for (int j = 0; j < node->neighbor_count; ++j) {
            int neighbor_id = node->neighbors[j]->neighbor_id;
            Node* neighbor_node = graph->nodes[neighbor_id];
            if (!has_neighbor(neighbor_node, node->id)) {
                add_neighbor_one_side(neighbor_node, node->id);
            }
        }
    }
}

int try_neighbors_symmetry(const Graph* graph) {
    for (int i = 0; i < graph->node_count; ++i) {
        Node* node = graph->nodes[i];
        for (int j = 0; j < node->neighbor_count; ++j) {
            int neighbor_id = node->neighbors[j]->neighbor_id;
            Node* neighbor_node = graph->nodes[neighbor_id];
            if (!has_neighbor(neighbor_node, node->id)) {
                return 1;
            }
        }
    }
    return 0;
}


void save_graph(const Graph* graph, const char* filename) {
    FILE* f = fopen(filename, "w");
    if (!f) {
        printf("Erreur ouverture fichier %s\n", filename);
        return;
    }
    fprintf(f, "%d\n", graph->node_count);
    for (int i = 0; i < graph->node_count; ++i) {
        Node* node = graph->nodes[i];
        fprintf(f, "%d", node->neighbor_count);
        for (int j = 0; j < node->neighbor_count; ++j) {
            fprintf(f, " %d", node->neighbors[j]->neighbor_id);
        }
        fprintf(f, "\n");
    }
    fclose(f);
}

Graph* load_graph(const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) {
        printf("Erreur ouverture fichier %s\n", filename);
        return NULL;
    }
    int node_count;
    fscanf(f, "%d", &node_count);
    Node** nodes = malloc(node_count * sizeof(Node*));
    for (int i = 0; i < node_count; ++i) {
        nodes[i] = create_node(i);
    }
    for (int i = 0; i < node_count; ++i) {
        int neighbor_count;
        fscanf(f, "%d", &neighbor_count);
        for (int j = 0; j < neighbor_count; ++j) {
            int neighbor_id;
            fscanf(f, "%d", &neighbor_id);
            if (!has_neighbor(nodes[i], neighbor_id)) {
                add_neighbor_one_side(nodes[i], neighbor_id);
            }
        }
    }
    fclose(f);
    Graph* graph = malloc(sizeof(Graph));
    graph->nodes = nodes;
    graph->node_count = node_count;
    if (try_neighbors_symmetry(graph) == 1) {
        printf("Warning : The graoh is not symetric");
    }
    return graph;
}