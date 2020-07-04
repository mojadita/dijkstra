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

#define DEFAULT_CAP         4

#define FLAG_NEEDS_SORT     (1 << 0)
#define FLAG_NODE_REACHED   (1 << 1)


struct d_graph {
    AVL_TREE         db;       /* database of nodes */
    char            *name;     /* name of this graph */
    struct d_node   *fr_start; /* the list of nodes in the frontier */
    struct d_node   *fr_end;   /* the last node of the frontier */
    int              nodes;    /* num of nodes of this graph */
}; /* struct d_graph */

struct d_graph *
d_new_graph(
        char *name,
        int flags)
{
    struct d_graph *res = malloc(sizeof *res + strlen(name) + 1);
    if (!res) return NULL;

    strcpy((char *)(res + 1), name); /* copy the name */
    res->name = (char *)(res + 1); /* make it point to the extra
                                    * bytes after the structure */
    res->db = new_avl_tree( /* allocate the database instance */
            (AVL_FCOMP) strcmp,  /* comparator */
            NULL,
            NULL,
            NULL);
    res->nodes = 0;
    if (flags & (D_FLAG_DEBUG | D_FLAG_NEW_GRAPH))
        printf(F("Graph %s created\n"), res->name);

    return res;
} /* d_new_graph */

struct d_node *
d_lookup_node(
        struct d_graph          *graph,
        const char              *name,
        int                      flags)
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
        res->flags    = 0;
        graph->nodes++;
        if (flags & (D_FLAG_DEBUG | D_FLAG_ALLOC_NODE))
            printf(F("Graph %s, allocating node %s => %p\n"),
                graph->name, name, res);
    }
    if (flags & (D_FLAG_DEBUG | D_FLAG_LOOKUP_NODE))
        printf(F("Graph %s, lookup node %s => %p\n"),
            graph->name, res->name, res);
    return res;
} /* d_lookup_node */

struct d_link *
d_add_link(
        struct d_node           *from,
        struct d_node           *to,
        int                      weight,
        int                      flags)
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
            if (flags & (D_FLAG_DEBUG | D_FLAG_ADD_ALREADY_IN_DB))
                printf(F("Link from %s to %s already in node, "
                    "just adjust weight to %d\n"),
                    from->name, to->name, weight);
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
        if (flags & (D_FLAG_DEBUG | D_FLAG_ADD_INCREASING_CAP))
            printf(F("Node %s increasing capacity to %d\n"),
                from->name, from->next_cap);
    }
    /* res points to the next slot position, despite we go
     * through the if above or not */
    res->weight = weight;
    res->from = from;
    res->to = to;
    from->flags |= FLAG_NEEDS_SORT;
    from->next_n++;
    if (flags & (D_FLAG_DEBUG | D_FLAG_ADD))
        printf(F("Add link from %s to %s with weight = %d\n"),
            from->name, to->name, weight);
    return res;
} /* d_add_link */

static int
cmp_node(const void *a, const void *b)
{
    const struct d_link
        *A = a,
        *B = b;
    return A->weight - B->weight;
} /* cmp_node */

static int
sort_node(struct d_node *n, void *call_data)
{
    int flags = *(int *)call_data;

    if (n->flags & FLAG_NEEDS_SORT) {
        qsort(n->next, n->next_n, sizeof *n->next,
                cmp_node);
        n->flags &= ~FLAG_NEEDS_SORT;
        if (flags & (D_FLAG_DEBUG | D_FLAG_SORT_NODE)) {
            printf(F("Sorting node %s\n"),
                n->name);
        }
    }
    return 0;
} /* sort_node */

static int
reset_node(struct d_node *n, void *call_data)
{
    n->back          = NULL;
    n->next_l        = n->next;
    n->cost          = 0;
    n->flags         = 0;
    return sort_node(n, call_data);
} /* reset_node */

void
d_sort(
        struct d_graph          *graph,
        int                      flags)
{
    d_foreach_node(graph, sort_node, &flags);
} /* d_sort */

void
d_reset(
        struct d_graph   *graph,
        int               flags)
{
    d_foreach_node(graph, reset_node, &flags);
} /* d_reset */

struct call_data {
    int printed_chars;
    FILE *out_file;
};

static int
print_node(struct d_node *n, void *call_data)
{
    struct call_data *p = call_data;
    int res;
    res += fprintf(p->out_file,
            "  Node %s: flags=0x%x\n",
            n->name, n->flags);
    struct d_link *l = n->next;
    int i;
    for (i = 0; i < n->next_n; ++i, ++l)
        res += fprintf(p->out_file,
                "    Next=%s, wgt=%d\n",
                l->to->name, l->weight);
    p->printed_chars += res;
    return 0;
} /* print_node */

ssize_t
d_print_graph(struct d_graph *graph, FILE *out)
{
    struct call_data data = {
        .printed_chars = 0,
        .out_file = out,
    };
    ssize_t res = fprintf(out, "Graph %s:\n", graph->name);
    d_foreach_node(graph, print_node, &data);

    return res + data.printed_chars;
} /* d_print_graph */

int
d_dijkstra(
        struct d_graph   *graph,
        struct d_node    *orig,
        struct d_node    *dest,
        int               flags)
{
    d_reset(graph, flags);

    if (flags & (D_FLAG_DEBUG | D_FLAG_ADD_NODE_FRONTIER))
        printf(F("Add start node %s to the frontier\n"),
                orig->name);
    struct d_node *fr_first = orig;
    struct d_node *fr_last  = orig;
    orig->fr_next = orig->fr_prev = NULL;
    orig->flags |= FLAG_NODE_REACHED;

    int pass = 0;
    struct d_link *cand;  /* candidate */
    int n_nodes = 1;
    do {
        int cost = INT_MAX;
        pass++; /* increment the iteration */

        cand = NULL;
        if (flags & (D_FLAG_DEBUG | D_FLAG_PASS_START))
            printf(F("Pass #%d START (%d nodes in the frontier)\n"),
                    pass, n_nodes);

        /* for all nodes in the frontier */
        struct d_node *nod;
        for (nod = fr_first; nod != NULL; nod = nod->fr_next) {

            if (flags & (D_FLAG_DEBUG | D_FLAG_PASS_NODE))
                printf(F(" - Frontier Node %s:\n"),
                        nod->name);
            struct d_link *l;
            struct d_link *end = nod->next + nod->next_n;
            for (l = nod->next_l; l < end; ++l)
            {
                if (l->to->flags & FLAG_NODE_REACHED) {
                    if (flags
                        & (D_FLAG_DEBUG
                            | D_FLAG_PASS_ALREADY_VISITED))
                    {
                        printf(F("     Node %s already visited, "
                                "skipping link\n"),
                                l->to->name);
                    }
                    /* need to invalidate this link */
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
                    if (flags & (D_FLAG_DEBUG |
                            D_FLAG_PASS_GOT_CANDIDATE))
                        printf(F("   Got a candidate: %s(c=%d) "
                                "-[w=%d]-> %s(c=%d)\n"),
                                cand->from->name, cand->from->cost,
                                cand->weight,
                                cand->to->name, cost);
                    /* exit the loop */
                    break;
                }
            }
            if (nod->next_l == end) {
                /* we exhausted this node, unlink it from the doubly linked list */
                if (flags & (D_FLAG_DEBUG | D_FLAG_PASS_NODE_EXHAUSTED))
                    printf(F("   Eliminate node %s from the frontier\n"),
                            nod->name);
                if (nod == fr_first) fr_first = nod->fr_next;
                if (nod == fr_last) fr_last = nod->fr_prev;
                if (nod->fr_prev) nod->fr_prev->fr_next = nod->fr_next;
                if (nod->fr_next) nod->fr_next->fr_prev = nod->fr_prev;
                /* we leave the pointers departin from the node out,
                 * so the for loop still works */
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
            cand->to->cost = cost;
            /* print the candidate selected */
            if (flags & (D_FLAG_DEBUG |
                    D_FLAG_PASS_ADD_CANDIDATE))
            {
                printf(F(" - Adding selected candidate "
                        "%s(c=%d) >=[w=%d]=> %s(c=%d) => <<<%s>>> "
                        "to the frontier\n"),
                        cand->from->name, cand->from->cost,
                        cand->weight,
                        cand->to->name, cost,
                        cand->to->name);
            }
            n_nodes++;
        }
    } while (cand && cand->to != dest);
    if (flags & (D_FLAG_DEBUG | D_FLAG_PASS_END)) {
        printf(F("Pass #%d END (%d nodes in the frontier)\n"),
                pass, n_nodes);
    }
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
        struct d_graph         *g,
        int                   (*callback)(struct d_node *, void *),
        void                   *calldata)
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
} /* d_foreach_node */
