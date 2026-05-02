#include "graph.h"

#include <stdlib.h>

static int graph_vertex_valid(const graph *g, size_t vertex) {
    return g != NULL && vertex < g->vertex_count;
}

static int graph_add_directed_edge(graph *g, size_t from, size_t to, int64_t weight) {
    return vector_push(&g->adj_lists[from], &(graph_edge){ to, weight });
}

static size_t graph_find_directed_edge(const graph *g, size_t from, size_t to,
                                       int64_t weight, size_t start) {
    const vector *adj = &g->adj_lists[from];

    for (size_t i = start; i < vector_size(adj); i++) {
        const graph_edge *edge = vector_get_const_as(adj, i, graph_edge);
        if (edge->to == to && edge->weight == weight) return i;
    }

    return GRAPH_EDGE_NOT_FOUND;
}

int graph_init(graph *g, graph_kind kind, size_t vertex_count) {
    if (g == NULL) return -1;
    if (kind != GRAPH_UNDIRECTED && kind != GRAPH_DIRECTED) return -1;

    g->vertex_count = vertex_count;
    g->edge_count = 0;
    g->directed = kind == GRAPH_DIRECTED;
    g->adj_lists = NULL;
    if (vertex_count == 0) return 0;

    g->adj_lists = malloc(vertex_count * sizeof(*g->adj_lists));
    if (g->adj_lists == NULL) return -1;

    for (size_t i = 0; i < vertex_count; i++) {
        if (vector_init(&g->adj_lists[i], sizeof(graph_edge), NULL, NULL) != 0) {
            for (size_t j = 0; j < i; j++) {
                vector_destroy(&g->adj_lists[j]);
            }
            free(g->adj_lists);
            g->adj_lists = NULL;
            return -1;
        }
    }

    return 0;
}

void graph_destroy(graph *g) {
    if (g == NULL) return;

    if (g->adj_lists != NULL) {
        for (size_t i = 0; i < g->vertex_count; i++) {
            vector_destroy(&g->adj_lists[i]);
        }
        free(g->adj_lists);
    }

    g->vertex_count = 0;
    g->edge_count = 0;
    g->directed = 0;
    g->adj_lists = NULL;
}

graph *graph_new(graph_kind kind, size_t vertex_count) {
    graph *g = malloc(sizeof(*g));
    if (g == NULL) return NULL;
    if (graph_init(g, kind, vertex_count) != 0) {
        free(g);
        return NULL;
    }
    return g;
}

void graph_free(graph *g) {
    if (g == NULL) return;
    graph_destroy(g);
    free(g);
}

int graph_copy(graph *dst, const graph *src) {
    if (dst == NULL || src == NULL) return -1;
    if (dst == src) return 0;

    graph temp;
    if (graph_init(&temp, src->directed ? GRAPH_DIRECTED : GRAPH_UNDIRECTED,
                   src->vertex_count) != 0) {
        return -1;
    }

    temp.edge_count = src->edge_count;
    for (size_t i = 0; i < src->vertex_count; i++) {
        if (vector_copy(&temp.adj_lists[i], &src->adj_lists[i]) != 0) {
            graph_destroy(&temp);
            return -1;
        }
    }

    graph_destroy(dst);
    *dst = temp;
    return 0;
}

graph *graph_clone(const graph *src) {
    if (src == NULL) return NULL;

    graph *copy = graph_new(src->directed ? GRAPH_DIRECTED : GRAPH_UNDIRECTED,
                            src->vertex_count);
    if (copy == NULL) return NULL;
    if (graph_copy(copy, src) != 0) {
        graph_free(copy);
        return NULL;
    }
    return copy;
}

int graph_add_edge(graph *g, size_t from, size_t to, int64_t weight) {
    if (!graph_vertex_valid(g, from) || !graph_vertex_valid(g, to)) return -1;

    if (graph_add_directed_edge(g, from, to, weight) != 0) return -1;
    if (!graph_directed(g) && graph_add_directed_edge(g, to, from, weight) != 0) {
        vector_pop(&g->adj_lists[from]);
        return -1;
    }

    g->edge_count++;
    return 0;
}

int graph_delete_edge(graph *g, size_t from, size_t to, int64_t weight) {
    size_t from_index;
    size_t to_index;

    if (!graph_vertex_valid(g, from) || !graph_vertex_valid(g, to)) return -1;

    from_index = graph_find_directed_edge(g, from, to, weight, 0);
    if (from_index == GRAPH_EDGE_NOT_FOUND) return 0;

    if (graph_directed(g)) {
        vector_erase(&g->adj_lists[from], from_index);
        g->edge_count--;
        return 1;
    }

    to_index = graph_find_directed_edge(g, to, from, weight,
                                        from == to ? from_index + 1 : 0);
    if (to_index == GRAPH_EDGE_NOT_FOUND) return 0;

    if (from == to) {
        vector_erase(&g->adj_lists[from], to_index);
        vector_erase(&g->adj_lists[from], from_index);
    } else {
        vector_erase(&g->adj_lists[to], to_index);
        vector_erase(&g->adj_lists[from], from_index);
    }

    g->edge_count--;
    return 1;
}

int graph_has_edge(const graph *g, size_t from, size_t to) {
    if (!graph_vertex_valid(g, from) || !graph_vertex_valid(g, to)) return 0;

    const vector *adj = &g->adj_lists[from];
    for (size_t i = 0; i < vector_size(adj); i++) {
        const graph_edge *edge = vector_get_const_as(adj, i, graph_edge);
        if (edge->to == to) return 1;
    }

    return 0;
}

void graph_clear(graph *g) {
    if (g == NULL) return;

    for (size_t i = 0; i < g->vertex_count; i++) {
        vector_clear(&g->adj_lists[i]);
    }
    g->edge_count = 0;
}

const vector *graph_adj_list(const graph *g, size_t vertex) {
    if (!graph_vertex_valid(g, vertex)) return NULL;
    return &g->adj_lists[vertex];
}

size_t graph_degree(const graph *g, size_t vertex) {
    const vector *adj = graph_adj_list(g, vertex);
    if (adj == NULL) return GRAPH_INVALID_VERTEX;
    return vector_size(adj);
}

const graph_edge *graph_neighbors(const graph *g, size_t vertex) {
    const vector *adj = graph_adj_list(g, vertex);
    return adj == NULL ? NULL : (const graph_edge *)adj->data;
}
