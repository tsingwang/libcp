#include "graph.h"
#include "testhelp.h"

static void graph_test_directed_runtime(void) {
    graph *g = graph_new(GRAPH_DIRECTED, 6);
    const vector *adj;
    size_t degree = 0;
    const graph_edge *neighbors;

    test_cond("graph new", g != NULL);
    test_cond("graph add edge", graph_add_edge(g, 0, 1, 2) == 0);
    test_cond("graph add edge", graph_add_edge(g, 0, 2, 5) == 0);
    test_cond("graph add edge", graph_add_edge(g, 1, 2, 1) == 0);
    test_cond("graph add edge", graph_add_edge(g, 1, 3, 2) == 0);
    test_cond("graph add edge", graph_add_edge(g, 2, 3, 1) == 0);
    test_cond("graph add edge", graph_add_edge(g, 2, 5, 10) == 0);
    test_cond("graph add edge", graph_add_edge(g, 3, 4, 3) == 0);
    test_cond("graph add duplicate weighted edge", graph_add_edge(g, 1, 3, 9) == 0);
    test_cond("graph vertex count", graph_vertex_count(g) == 6);
    test_cond("graph edge count", graph_edge_count(g) == 8);
    test_cond("graph directed", graph_directed(g) == 1);
    test_cond("graph degree", graph_degree(g, 0) == 2);
    test_cond("graph degree invalid", graph_degree(g, 9) == GRAPH_INVALID_VERTEX);
    test_cond("graph has edge", graph_has_edge(g, 1, 3) == 1);
    test_cond("graph missing edge", graph_has_edge(g, 3, 1) == 0);
    test_cond("graph delete weighted edge", graph_delete_edge(g, 1, 3, 9) == 1);
    test_cond("graph delete weighted edge count", graph_edge_count(g) == 7);
    test_cond("graph delete missing weighted edge", graph_delete_edge(g, 1, 3, 9) == 0);

    adj = graph_adj_list(g, 1);
    test_cond("graph adj list", adj != NULL);
    test_cond("graph adj list size", vector_size(adj) == 2);
    degree = graph_degree(g, 1);
    neighbors = graph_neighbors(g, 1);
    test_cond("graph neighbors count", degree == 2);
    test_cond("graph neighbors values",
              neighbors[0].to == 2 && neighbors[0].weight == 1 &&
              neighbors[1].to == 3 && neighbors[1].weight == 2);

    graph_free(g);
}

static void graph_test_undirected_runtime(void) {
    graph g;
    size_t degree = 0;
    const graph_edge *neighbors;

    test_cond("graph undirected init", graph_init(&g, GRAPH_UNDIRECTED, 4) == 0);
    test_cond("graph undirected add", graph_add_edge(&g, 0, 1, 7) == 0);
    test_cond("graph undirected add", graph_add_edge(&g, 1, 2, 4) == 0);
    test_cond("graph undirected add dup", graph_add_edge(&g, 1, 2, 8) == 0);
    test_cond("graph undirected delete exact", graph_delete_edge(&g, 1, 2, 8) == 1);
    test_cond("graph undirected delete exact reverse", graph_has_edge(&g, 2, 1) == 1);
    test_cond("graph undirected edge count", graph_edge_count(&g) == 2);
    test_cond("graph undirected degree", graph_degree(&g, 1) == 2);
    test_cond("graph undirected reverse edge", graph_has_edge(&g, 1, 0) == 1);
    degree = graph_degree(&g, 1);
    neighbors = graph_neighbors(&g, 1);
    test_cond("graph undirected neighbors count", degree == 2);
    test_cond("graph undirected neighbors values",
              neighbors[0].to == 0 && neighbors[0].weight == 7 &&
              neighbors[1].to == 2 && neighbors[1].weight == 4);
    graph_destroy(&g);
}

static void graph_test_copy_clone(void) {
    graph src;
    graph dst;
    graph *clone;

    test_cond("graph copy init src", graph_init(&src, GRAPH_DIRECTED, 3) == 0);
    test_cond("graph copy init dst", graph_init(&dst, GRAPH_UNDIRECTED, 1) == 0);
    test_cond("graph copy fill", graph_add_edge(&src, 0, 1, 2) == 0);
    test_cond("graph copy fill", graph_add_edge(&src, 1, 2, 3) == 0);

    test_cond("graph copy null dst", graph_copy(NULL, &src) == -1);
    test_cond("graph copy null src", graph_copy(&dst, NULL) == -1);
    test_cond("graph copy success", graph_copy(&dst, &src) == 0);
    test_cond("graph copy metadata",
              graph_directed(&dst) == graph_directed(&src) &&
              graph_vertex_count(&dst) == graph_vertex_count(&src) &&
              graph_edge_count(&dst) == graph_edge_count(&src));
    test_cond("graph copy contents",
              graph_has_edge(&dst, 0, 1) == 1 && graph_has_edge(&dst, 1, 2) == 1);

    clone = graph_clone(&src);
    test_cond("graph clone alloc", clone != NULL);
    test_cond("graph clone contents", graph_has_edge(clone, 0, 1) == 1);
    test_cond("graph clone update", graph_add_edge(clone, 0, 2, 9) == 0);
    test_cond("graph clone independent",
              graph_has_edge(clone, 0, 2) == 1 && graph_has_edge(&src, 0, 2) == 0);

    graph_clear(clone);
    test_cond("graph clear edge count", graph_edge_count(clone) == 0);
    test_cond("graph clear degree", graph_degree(clone, 0) == 0);

    graph_free(clone);
    graph_destroy(&dst);
    graph_destroy(&src);
}

static void graph_test_runtime_failures(void) {
    graph g;

    test_cond("graph init invalid", graph_init(NULL, GRAPH_DIRECTED, 2) == -1);
    test_cond("graph init invalid kind", graph_init(&g, (graph_kind)99, 2) == -1);
    test_cond("graph new invalid", graph_new((graph_kind)99, 2) == NULL);
    test_cond("graph init stack", graph_init(&g, GRAPH_DIRECTED, 3) == 0);
    test_cond("graph add invalid from", graph_add_edge(&g, 3, 0, 1) == -1);
    test_cond("graph add invalid to", graph_add_edge(&g, 0, 3, 1) == -1);
    test_cond("graph delete invalid from", graph_delete_edge(&g, 3, 0, 1) == -1);
    test_cond("graph has invalid", graph_has_edge(&g, 0, 3) == 0);
    test_cond("graph adj list invalid", graph_adj_list(&g, 9) == NULL);
    test_cond("graph neighbors invalid", graph_neighbors(&g, 9) == NULL);
    graph_destroy(&g);
}

int main(void) {
    graph_test_directed_runtime();
    graph_test_undirected_runtime();
    graph_test_copy_clone();
    graph_test_runtime_failures();
    test_report();
}
