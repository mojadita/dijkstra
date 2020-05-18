/* node.h -- node structure to allow for building weighted
 *           graphs.
 * Author: Luis Colorado <luiscoloradourcola@gmail.com>
 * Date: Thu May 14 15:20:45 EEST 2020
 * Copyright: (C) 2020 Luis Colorado.  All rights reserved.
 * License: BSD.
 */

#ifndef _DIJKSTRA_H
#define _DIJKSTRA_H

#include <stdio.h>

#define D_FLAG_DEBUG                (1 << 0)
#define D_FLAG_NEW_GRAPH            (1 << 1)
#define D_FLAG_ALLOC_NODE           (1 << 2)
#define D_FLAG_LOOKUP_NODE          (1 << 3)
#define D_FLAG_ADD_ALREADY_IN_DB    (1 << 4)
#define D_FLAG_ADD_INCREASING_CAP   (1 << 5)
#define D_FLAG_ADD                  (1 << 6)
#define D_FLAG_SORT_NODE            (1 << 7)
#define D_FLAG_SORT_GRAPH           (1 << 8)
#define D_FLAG_RESET_GRAPH          (1 << 9)
#define D_FLAG_ADD_NODE_FRONTIER    (1 << 10)
#define D_FLAG_PASS_START           (1 << 11)
#define D_FLAG_PASS_NODE            (1 << 12)
#define D_FLAG_PASS_ALREADY_VISITED (1 << 13)
#define D_FLAG_PASS_GOT_CANDIDATE   (1 << 14)
#define D_FLAG_PASS_NODE_EXHAUSTED  (1 << 15)
#define D_FLAG_PASS_ADD_CANDIDATE   (1 << 16)
#define D_FLAG_PASS_END             (1 << 17)

struct d_graph;                /* opaque */

struct d_link;                 /* link between nodes */
struct d_node;                 /* nodes of a graph */

struct d_link {
    int             weight;    /* weight of this transition */
    struct d_node  *from,      /* next node */
                   *to;        /* origin node */
};

struct d_node {
    /* we put first the pointers, then the integers, to conserve
     * alignment space. */
    char           *name;      /* name of this node, must be unique */
    struct d_link  *next;      /* set of next nodes */
    struct d_node  *back;      /* pointer back to last node */
    struct d_graph *graph;     /* graph this node belongs to */
    struct d_node  *fr_prev;   /* previous node in the frontier */
    struct d_node  *fr_next;   /* next node in the frontier */
    int             next_n;    /* number of next nodes */
    int             next_cap;  /* capacity of next array */
    int             flags;     /* flags for this node */
    struct d_link  *next_l;    /* next i to probe */
    int             cost;      /* cost to reach this node */
};

/**
 * Create a new instance of a graph.
 *
 * The following function creates an empty instance of a
 * struct d_graph object.
 * It names the graph with the passed parameter.
 *
 * @param name is the name to be used for the graph.
 * @return a reference to the new instance, or NULL, in
 *         case it is not possible to allocate one.
 */
struct d_graph *
d_new_graph(
        char       *name,
        int         flags);

/**
 * Add link to the graph.
 *
 * The following function adds a link to the database, based on
 * the starting node, the ending node, and the weight assigned to
 * the path of the link from A to B.
 * The function searchs both nodes and creates a path from the
 * first to the second.  It fails if the path already exists.
 * @param a_name is the source node of the link.
 * @param b_name is the destination node of the link.
 * @param weight is the weight associated to this link in the
 *               direction from a to b.
 * @return 0 if successful, or != 0 on error.  Error can happen
 *           if no memory is available or a link already exists
 *           for the requested source and destination.
 */
struct d_link *
d_add_link(
        struct d_node    *from,
        struct d_node    *to,
        int               weight,
        int               flags);

/**
 * Lookup a named node in graph.
 *
 * This function searches for a node named as 'name', and
 * creates a new node in case it does not find one.
 * @param graph is the graph on which we want to finde the node.
 * @param name is the name of the node we are searching for.
 * @return a reference to the just located node.
 */
struct d_node *
d_lookup_node(
        struct d_graph   *graph,
        const char       *name,
        int               flags);

/**
 * Sorts the links of the nodes from lower weight to larger.
 *
 * This function navigates all the nodes of the graph, sorting
 * the links by weight in ascending order.  This allows to
 * select the next link to compete for the smallest grow in
 * total cost.
 * Normally, sort is needed when a graph is modified (some link
 * has been added or deleted, as the vector of links on each node
 * needs to be sorted for use in dijkstra algorithm.
 *
 * @param graph is the graph to be navigated.
 */
void
d_sort(
        struct d_graph   *graph,
        int               flags);

/**
 * Reset the node to start a new Dijkstra.
 *
 * This function resets the state of the complete graph, to
 * allow to start another dijkstra (from the same, or different
 * start node)
 *
 * Normally, a reset is only needed if dijkstra has already been
 * called on a graph in order to be able to run it again.
 *
 * @param graph is the reference to the graph to be reset.
 */
void
d_reset(
        struct d_graph   *graph,
        int               flags);

/**
 * Print a graph.
 *
 * Prints a graphs contents.  It prints the name, then a list
 * of the nodes, and for each node, it prints the links to
 * other nodes (with the weight).
 *
 * @param graph the graph to print.
 * @param out the FILE * descriptor to output the data to.
 * @return the number of characters printed.
 */
ssize_t
d_print_graph(
        struct d_graph   *graph,
        FILE             *out);

/**
 * Executes the algorithm on the graph given, with
 * orig as the origen and dest as the destination nodes.
 *
 * @param graph is the graph we are calculating for.
 * @param orig is the origin node of paths.
 * @param dest is the destination node, or NULL.  In case of
 * passing NULL as destination, the algorithm continues until all
 * nodes have been visited, so then you can show the minimum cost
 * paths for all the nodes in the graph.
 * @return the algorithm returns the number of passes make until
 * finish.
 */
int
d_dijkstra(
        struct d_graph   *graph,
        struct d_node    *orig,
        struct d_node    *dest,
        int               flags);

int
d_foreach_node(
        struct d_graph   *g,
        int             (*callback)(
                                struct d_node *,
                                void *),
        void             *calldata);

/**
 * Print the route from the origin to the specified destination.
 *
 * Dijkstra algorithm runs until it finds the destination node,
 * but it can be left running until it exhaustes all the graph
 * nodes, then, each node can be used as destination, and will
 * know the shortest route to the origin.
 *
 * @param file is the output stream to print the route to.
 * @param destination is a reference to the node to act as
 *                    destination of the shortest path.
 * @return the number of characters written to the output
 *         stream.
 */
ssize_t
d_print_route(
        FILE             *file,
        struct d_node    *destination);

#endif /* _DIJKSTRA_H */
