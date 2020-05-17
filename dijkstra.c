/* dijkstra.c -- routine implementing Dijkstra algorithm.
 * Author: Luis Colorado <luiscoloradourcola@gmail.com>
 * Date: Thu May 14 16:01:14 EEST 2020
 * Copyright: (C) 2020 LUIS COLORADO.  All rights reserved.
 * License: BSD.
 */

#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <avl.h>

#include "dijkstra.h"

#define F(_fmt) __FILE__":%d:%s: "_fmt,__LINE__,__func__

#define DEFAULT_CAP		    4

#define FLAG_NEEDS_SORT		(1 << 0)
#define FLAG_NODE_REACHED	(1 << 1)


struct d_graph {
	AVL_TREE	   	 db;       /* database of nodes */
	char          	*name; 	   /* name of this graph */
	struct d_node	*fr_start; /* the list of nodes in the frontier */
	struct d_node	*fr_end;   /* the last node of the frontier */
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
		res->flags	  = 0;
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

		n->back          = NULL;
		n->next_l        = n->next;
		n->cost          = 0;
		n->flags         = 0;
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

int
d_dijkstra(
		struct d_graph	 *graph,
		struct d_node	 *orig,
		struct d_node	 *dest)
{
	printf(F("Reseting graph %s\n"), graph->name);
	d_reset(graph);

	printf(F("Add node %s to the frontier\n"), orig->name);
	struct d_node *fr_first = orig;
	struct d_node *fr_last  = orig;
	orig->fr_next = orig->fr_prev = NULL;
	orig->flags |= FLAG_NODE_REACHED;

	int pass = 0;
	struct d_link *cand;  /* candidate */
    int n_nodes = 1;
	do {
		int cost = INT_MAX;

		cand = NULL;
		printf(F("Pass #%d START (%d nodes in the frontier)\n"),
				++pass, n_nodes);

		/* for all nodes in the frontier */
		struct d_node *nod;
		for (nod = fr_first; nod != NULL; nod = nod->fr_next) {

			printf(F(" - Frontier Node %s:\n"), nod->name);
			struct d_link *l;
            struct d_link *end = nod->next + nod->next_n;
			for (l = nod->next_l; l < end; ++l)
			{
				if (l->to->flags & FLAG_NODE_REACHED) {
					nod->next_l = l + 1;
					continue;
				}
				int new_cost = l->from->cost + l->weight;
				if (new_cost < cost) {
					cost = new_cost;
					/* need to increment l before breaking
					 * the loop so we don't consider this link
					 * again */
					cand = l;
					printf(F("   Got a candidate: %s(c=%d) "
							"-[w=%d]-> %s(c=%d)\n"),
							cand->from->name, cand->from->cost,
							cand->weight,
							cand->to->name, cost);
					break;
				}
			}
            if (nod->next_l == end) {
                /* we exhausted this node, unlink it from the doubly linked list */
				printf(F("   Eliminate node %s from the frontier\n"), nod->name);
                if (nod == fr_first) fr_first = nod->fr_next;
                if (nod == fr_last) fr_last = nod->fr_prev;
                if (nod->fr_prev) nod->fr_prev->fr_next = nod->fr_next;
                if (nod->fr_next) nod->fr_next->fr_prev = nod->fr_prev;
                /* we leave the pointers from the node out, so the
                 * for loop still works */
                n_nodes--;
            }
		} /* for (it..) */
		if (cand) {
			/* register the candidate's data. */
			cand->to->cost = cost;
			cand->to->back = cand->from;
			cand->to->flags |= FLAG_NODE_REACHED; /* mark as visited */
			cand->from->next_l = cand + 1; /* where we start next */
			/* we insert it in the beginning of the frontier, so we
			 * don't process it on this pass. */
			cand->to->fr_prev = NULL;
			cand->to->fr_next = fr_first;
			cand->to->fr_next->fr_prev = cand->to;
			fr_first = cand->to;
			/* print the candidate selected */
			printf(F("   Selected candidate: %s(c=%d) >=[w=%d]=> %s(c=%d)\n"),
				cand->from->name, cand->from->cost, cand->weight,
				cand->to->name, cand->to->cost = cost);
			printf(F("   Adding node %s to the frontier\n"),
					cand->to->name);
            n_nodes++;
		}
	} while (cand && cand->to != dest);
	printf(F("Pass #%d END (%d nodes in the frontier)\n"),
			pass, n_nodes);
	return pass;
} /* d_dijkstra */

ssize_t
d_print_route(
        FILE             *file,
        struct d_node    *nod)
{
	ssize_t res = 0;
	if (nod->back) {
		res += d_print_route(file, nod->back);
		res += fprintf(file, "->");
	}
	res += fprintf(file, "[%s:c=%d]",
			nod->name, nod->cost);
	return res;
} /* d_print_route */

int
d_foreach_node(
		struct d_graph		   *g,
		int					  (*callback)(struct d_node *, void *),
		void 				   *calldata)
{
	AVL_ITERATOR it;
	for (it = avl_tree_first(g->db);
		 it != NULL;
		 it = avl_iterator_next(it))
	{
		struct d_node *nod = avl_iterator_data(it);
		int res;
		if (callback && (res = callback(nod, calldata)))
			return res;
	}
	return 0;
}
