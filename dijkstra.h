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

struct d_graph;                /* opaque */

struct d_link;                 /* link between nodes */
struct d_node;                 /* nodes of a graph */

struct d_link {
    int             weight;    /* weight of this transition */
    struct d_node  *from,      /* next node */
                   *to;        /* origin node */
};

struct d_node {
    const char     *name;      /* name of this node, must be unique */
    struct d_link  *next;      /* set of next nodes */
    struct d_node  *back;      /* pointer back to last node */
    struct d_node  *nfrontier; /* next node in the frontier */
    int             next_n;    /* number of next nodes */
    int             next_cap;  /* capacity of next array */
	int				flags;	   /* flags for this node */
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
g_new_graph(
        const char *name);

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
int
g_add_link(
        struct d_node    *from,
        struct d_node    *to,
        int               weight);

/**
 * Lookup a named node in graph.
 *
 * This function searches for a node named as 'name', and
 * creates a new node in case it does not find one.
 * @param graph is the graph on which we want to finde the node.
 * @param name is the name of the node we are searching for.
 * @return a reference to the just located node.
 */
struct node *
g_lookup_node(
        struct d_graph   *graph,
        const char       *name);

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
g_reset(
        struct d_graph   *graph);

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
g_print(
        struct d_graph   *graph,
        FILE             *out);

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
print_route(
        FILE             *file,
        struct d_node    *destination);

#endif /* _DIJKSTRA_H */
