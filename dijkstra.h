/* node.h -- node structure to allow for building weighted
 * graphs.
 * Author: Luis Colorado <luiscoloradourcola@gmail.com>
 * Date: Thu May 14 15:20:45 EEST 2020
 * Copyright: (C) 2020 Luis Colorado.  All rights reserved.
 * License: BSD.
 */

#ifndef _DIJKSTRA_H
#define _DIJKSTRA_H

#include <stdio.h>

struct d_graph;  /* opaque */
struct d_link;
struct d_node;

struct d_link {
	int			    weight;    /* weight of this transition */
	struct d_node  *from,      /* next node */
				   *to;	       /* origin node */
};

struct d_node {
	const char	   *name;      /* name of this node, must be unique */
	int 		    next_n;    /* number of next nodes */
	int			    next_cap;  /* capacity of next array */
	struct d_link  *next;	   /* set of next nodes */
	struct d_node  *back;      /* pointer back to last node */
	struct d_node  *nfrontier; /* next node in the frontier */
};

struct d_graph *g_new_graph();

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
		struct d_node	 *from,
		struct d_node	 *to,
		int		          weight);		

struct node *
g_lookup_node(
		struct d_graph     *graph,
		const char       *name);

ssize_t
print_route(
		FILE             *file,
		struct d_node    *destination);

#endif /* _DIJKSTRA_H */
