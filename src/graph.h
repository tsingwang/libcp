#ifndef __GRAPH_H
#define __GRAPH_H

#include "vector.h"

#include <stddef.h>
#include <stdint.h>

typedef enum {
    GRAPH_UNDIRECTED = 0,
    GRAPH_DIRECTED = 1,
} graph_kind;

typedef struct {
    size_t to;
    int64_t weight;
} graph_edge;

typedef struct {
    size_t vertex_count;    /* valid vertex ids are [0, vertex_count) */
    size_t edge_count;      /* logical edge count; undirected edges count once */
    unsigned char directed; /* whether it is directed graph */
    vector *adj_lists;      /* one adjacency vector per vertex */
} graph;

#define GRAPH_INVALID_VERTEX ((size_t)-1)
#define GRAPH_EDGE_NOT_FOUND ((size_t)-1)
#define graph_vertex_count(g) ((g)->vertex_count)
#define graph_edge_count(g) ((g)->edge_count)
#define graph_directed(g) ((g)->directed != 0)

/* Example:
 *   graph_add_edge(g, 0, 1, 3);
 *   graph_has_edge(g, 0, 1) == 1;
 */

/* Stack usage:
 *   graph g;
 *   graph_init(&g, GRAPH_DIRECTED, 4);
 *   graph_destroy(&g);
 */
int graph_init(graph *g, graph_kind kind, size_t vertex_count);
void graph_destroy(graph *g);

/* Heap usage:
 *   graph *g = graph_new(GRAPH_DIRECTED, 4);
 *   graph_free(g);
 */
graph *graph_new(graph_kind kind, size_t vertex_count);
void graph_free(graph *g);

/* copy(...) copies into an already initialized destination. */
int graph_copy(graph *dst, const graph *src);
graph *graph_clone(const graph *src);

int graph_add_edge(graph *g, size_t from, size_t to, int64_t weight);
/* delete_edge(...) removes one logical edge that exactly matches
 * (from, to, weight). In undirected graphs it removes both stored directions.
 * Returns 1 if removed, 0 if not found, and -1 on invalid input.
 */
int graph_delete_edge(graph *g, size_t from, size_t to, int64_t weight);
int graph_has_edge(const graph *g, size_t from, size_t to);
void graph_clear(graph *g);

/* adj_list(...) exposes one adjacency list as vector<graph_edge>. */
const vector *graph_adj_list(const graph *g, size_t vertex);

/* degree(...) returns out-degree, or GRAPH_INVALID_VERTEX for an invalid vertex. */
size_t graph_degree(const graph *g, size_t vertex);

/* neighbors(...) returns the contiguous adjacency list for one vertex.
 * Use graph_degree(...) for the number of edges in that list.
 */
const graph_edge *graph_neighbors(const graph *g, size_t vertex);

#endif
