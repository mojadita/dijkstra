/* dijkstra.c -- routine implementing Dijkstra algorithm.
 * Author: Luis Colorado <luiscoloradourcola@gmail.com>
 * Date: Thu May 14 16:01:14 EEST 2020
 * Copyright: (C) 2020 LUIS COLORADO.  All rights reserved.
 * License: BSD.
 */

#include <stdlib.h>
#include <avl.h>

#include "dijkstra.h"

struct d_graph {
	char          	*name; 	   /* name of this graph */
	AVL_TREE	   	 db;       /* database of nodes */
	struct d_node	*frontier; /* the list of nodes in the frontier */
};

#define DEFAULT_CAP		4

struct d_graph *d_new_graph(const char *name)
{
	struct d_graph *res = malloc(sizeof *res + strlen(name) + 1);
	assert(res != NULL);
	strcpy((char *)(res + 1), name);
	res->name = (char *)(res + 1);
	res->db = new_avl_tree(
			strcmp,
			NULL,
			NULL,
			NULL);
	return res;
}

struct d_link *
add_link(
		struct d_node 			*from,
		struct d_node 			*to,
		int			   			 weight)
{
	/* let's check the capacity */
	if (from->next_n == from->next_cap) {
		/* need to expand */
		from->next_cap <<= 1;
		from->next = realloc(
				from->next,
				from->next_cap
					* (sizeof *from->next));
        assert(from->next != NULL);
	}
	struct d_link *p = from->next + from->next_n++;
	p->weight = weight;
	p->from = from;
	p->to = to;

    return p;
}

struct d_node *
lookup_node(
		struct d_graph   		*graph,
		const char     			*name)
{
	struct d_node *res = avl_tree_get(name);
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

void d_sort()
{
	if (!nodes) {
		nodes = new_avl_tree(
				strcmp,
				NULL,
				NULL,
				NULL);
	}
	AVL_ITERATOR it;
	for (it = avl_tree_first(nodes); it; it = avl_it_next(it)) {
		struct d_node *n = avl_iterator_data(it);
		printf("Node [%s]:\n", n->name);
		qsort(n->next, n->next_n, sizeof *n->next, cmp_node);
		int i;
		struct d_linkk *p;
		for (i = 1, p = n->next; i <= n->next_n; ++i, ++p) {
			printf("  %d: wgt=%d -> [%s]\n",
					i, p->weight, p->next->name);
		}
	}
}
