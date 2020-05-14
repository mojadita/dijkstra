/* dijkstra.c -- routine implementing Dijkstra algorithm.
 * Author: Luis Colorado <luiscoloradourcola@gmail.com>
 * Date: Thu May 14 16:01:14 EEST 2020
 * Copyright: (C) 2020 LUIS COLORADO.  All rights reserved.
 * License: BSD.
 */

#include <stdlib.h>
#include <stdio.h>

#include <avl.h>

#include "dijkstra.h"

#define FLAG_NEEDS_SORT		(1 << 0)
#define FLAG_ALREADY_RUN	(1 << 1)
#define FLAG_FRONTIER		(1 << 2)

struct d_graph {
	char          	*name; 	   /* name of this graph */
	AVL_TREE	   	 db;       /* database of nodes */
	struct d_node	*frontier; /* the list of nodes in the frontier */
};

#define DEFAULT_CAP		4

struct d_graph *
d_new_graph(
		const char *name)
{
	struct d_graph *res = malloc(sizeof *res + strlen(name) + 1);
	if (!res) return NULL;

	strcpy((char *)(res + 1), name);
	res->name = (char *)(res + 1); /* make it point to the extra
									* bytes after the structure */
	res->db = new_avl_tree( /* allocate the database instance */
			strcmp,  /* comparator */
			NULL,
			NULL,
			NULL);
	return res;
}

struct d_link *
g_add_link(
		struct d_node 			*from,
		struct d_node 			*to,
		int			   			 weight)
{
	/* let's check the capacity of the next array. */
	if (from->next_n == from->next_cap) {
		/* need to expand */
		from->next_cap <<= 1;  /* double it */
		from->next = realloc(  /* and extend */
				from->next,
				from->next_cap
					* (sizeof *from->next));
        assert(from->next != NULL);
	}
	struct d_link *res = from->next + from->next_n++;
	res->weight = weight;
	res->from = from;
	res->to = to;
	graph->flags |= FLAG_NEEDS_SORT;

    return res;
}

struct d_node *
g_lookup_node(
		struct d_graph   		*graph,
		const char     			*name)
{
	struct d_node *res = avl_tree_get(graph->db, name);
	if (!res) {
		res = malloc(strlen(name) + 1 + sizeof *res);
		avl_tree_put(name, res);
		res->name = (char *)(res + 1);
		strcpy(res->name, name);
		res->next_n = 0;
		res->next_cap = DEFAULT_CAP;
		res->next = malloc(DEFAULT_CAP * sizeof *res->next);
		res->back = NULL;
	}
	return res;
}

static int
cmp_node(const void *a, const void *b)
{
	struct d_link
		*A = a,
		*B = b;
	return A->weight - B->weight;
}

