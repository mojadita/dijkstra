/* dijkstra.c -- routine implementing Dijkstra algorithm.
 * Author: Luis Colorado <luiscoloradourcola@gmail.com>
 * Date: Thu May 14 16:01:14 EEST 2020
 * Copyright: (C) 2020 LUIS COLORADO.  All rights reserved.
 * License: BSD.
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <avl.h>

#include "dijkstra.h"

#define DEFAULT_CAP		2

#define FLAG_NEEDS_SORT		(1 << 0)
#define FLAG_ALREADY_RUN	(1 << 1)
#define FLAG_FRONTIER		(1 << 2)

struct d_graph {
	AVL_TREE	   	 db;       /* database of nodes */
	char          	*name; 	   /* name of this graph */
	struct d_node	*frontier; /* the list of nodes in the frontier */
	int				 nodes;	   /* num of nodes of this graph */
};

struct d_graph *
d_new_graph(
		char *name)
{
	struct d_graph *res = malloc(sizeof *res + strlen(name) + 1);
	if (!res) return NULL;

	strcpy((char *)(res + 1), name);
	res->name = (char *)(res + 1); /* make it point to the extra
									* bytes after the structure */
	res->db = new_avl_tree( /* allocate the database instance */
			(AVL_FCOMP) strcmp,  /* comparator */
			NULL,
			NULL,
			NULL);
	res->nodes = 0;

	return res;
}

struct d_node *
d_lookup_node(
		struct d_graph   		*graph,
		const char     			*name)
{
	struct d_node *res = avl_tree_get(graph->db, name);
	if (!res) {
		res = malloc(strlen(name) + 1 + sizeof *res);
		assert(res != NULL);
		res->name     = (char *)(res + 1);
		strcpy(res->name, name);
		avl_tree_put(graph->db, res->name, res);
		res->next_n   = 0;
		res->next_cap = DEFAULT_CAP;
		res->next     = malloc(DEFAULT_CAP * sizeof *res->next);
        assert(res->next != NULL);
		res->back     = NULL;
		res->graph    = graph;
		graph->nodes++;
	}
	return res;
}

struct d_link *
d_add_link(
		struct d_node 			*from,
		struct d_node 			*to,
		int			   			 weight)
{
	/* first check that the link is not already present in the
	 * array. */
	struct d_link *res;
	int i;
	for (i = 0, res = from->next; i < from->next_n; ++i, ++res) {
		if (res->from == from && res->to == to) {
			/* change the weight, set needs to sort */
			res->weight       = weight;
			res->from->flags |= FLAG_NEEDS_SORT;
			return res;
		}
	}
	/* let's check the capacity for the need of expansion. */
	if (from->next_n == from->next_cap) {
		/* need to expand */
		from->next_cap <<= 1;  /* double it */
		from->next = realloc(  /* and extend */
				from->next,
				from->next_cap
					* (sizeof *from->next));
        assert(from->next != NULL);
	    res = from->next + from->next_n;
	}
    /* res points to the next slot position, despite we go
     * through the if above or not */
	res->weight = weight;
	res->from = from;
	res->to = to;
	from->flags |= FLAG_NEEDS_SORT;
    from->next_n++;

    return res;
}

static int
cmp_node(const void *a, const void *b)
{
	const struct d_link
		*A = a,
		*B = b;
	return A->weight - B->weight;
}

void
d_sort(
		struct d_graph 			*graph)
{
	AVL_ITERATOR it;
	for (it = avl_tree_first(graph->db);
		 it;
		 it = avl_iterator_next(it))
	{
		struct d_node *n = avl_iterator_data(it);
		if (n->flags & FLAG_NEEDS_SORT) {
			qsort(n->next, n->next_n, sizeof *n->next,
					cmp_node);
			n->flags &= ~FLAG_NEEDS_SORT;
		}
	}
} /* d_sort */

void
d_reset(
        struct d_graph   *graph)
{
	AVL_ITERATOR it;
	for (it = avl_tree_first(graph->db);
		 it;
		 it = avl_iterator_next(it))
	{
		struct d_node *n = avl_iterator_data(it);
		if (n->flags & FLAG_NEEDS_SORT)
			qsort(n->next, n->next_n, sizeof *n->next,
					cmp_node);
		n->back = NULL;
		n->cost = 0;
		n->flags = 0;
	}
}

ssize_t
d_print_graph(struct d_graph *graph, FILE *out)
{
	ssize_t res = fprintf(out, "Graph %s:\n", graph->name);
	AVL_ITERATOR it;
	for (it = avl_tree_first(graph->db);
		 it;
		 it = avl_iterator_next(it))
	{
		struct d_node *n = avl_iterator_data(it);
		res += fprintf(out,
				"  Node %s: flags=0x%x\n",
				n->name, n->flags);
		struct d_link *l = n->next;
		int i;
		for (i = 0; i < n->next_n; ++i, ++l)
			res += fprintf(out,
					"    Next=%s, wgt=%d\n",
					l->to->name, l->weight);
	}
	return res;
}
