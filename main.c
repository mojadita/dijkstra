#include <errno.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <avl.h>

#include "dijkstra.h"

#define F(_fmt) __FILE__":%d:%s: "_fmt,__LINE__,__func__

#define SEP_STRING      ", \t\n"
#define FINALSEP_STRING " \t\n"
#define STDIN_TOKEN     "-"
#define STDIN_NAME      "stdin"

#define FLAG_PRINT_GRAPH    (1 << 0)

int flags;

void do_help(char *prg, int code)
{
    fprintf(stderr,
        "Usage: %s [ opts ] [ file ... ]\n"
        "Where options are the options below and file is one file per\n"
        "graph.\n"
        "Options:\n"
        " -s src uses the named src node as start of the dijkstra\n"
        "    algorithm.\n"
        " -d dst uses the named dst node as the destination of the\n"
        "    dijkstra algorithm.\n"
        "File can be any readable file or '-' to indicate standard input.\n"
        "\n",
        prg);
    exit(code);
}

int pr_route(struct d_node *nod, void *not_used)
{
    printf("Node %s(c=%d): ", nod->name, nod->cost);
    d_print_route(stdout, nod);
    puts("");
    return 0;
}

void process(FILE *f, char *name, char *start, char *end)
{
    char line[256];
    int lineno = 0;
    struct d_graph *g = d_new_graph(name, flags);

    while (fgets(line, sizeof line, f)) {
        ++lineno;

        char *p = strtok(line, "\n");
        /* first param is source node */
        char *from = strtok(p, SEP_STRING);
        if (!from || from[0] == '#') continue;

        /* second param is destination node */
        char *to = strtok(NULL, SEP_STRING);
        if (!to) {
            fprintf(stderr, F("WARNING: %s:%d: no 'to' node"
                    " name.  Skipping this entry.\n"),
                    name, lineno);
            continue;
        }

        /* now, the weight of the arc */
        int weight = 1; /* default weight */
        char *rest = strtok(NULL, FINALSEP_STRING);
        if (rest) {
            sscanf(rest, "%d", &weight);
        }
        d_add_link(
                d_lookup_node(g, from, flags),
                d_lookup_node(g, to, flags),
                weight,
                flags);
    }
    d_sort(g, flags);
    if (flags & D_FLAG_DEBUG)
        d_print_graph(g, stdout);
    d_sort(g, flags);
    if (start) {
        struct d_node *snod = d_lookup_node(g, start, flags);
        if (end) {
            struct d_node *enod = d_lookup_node(g, end, flags);
            int iter = d_dijkstra(g, snod, enod, flags);
            if (flags & D_FLAG_DEBUG)
                printf(F("%d Iterations\n"), iter);
            d_print_route(stdout, enod);
            puts("");
        } else {
            int iter = d_dijkstra(g, snod, NULL, flags);
            if (flags & D_FLAG_DEBUG)
                printf(F("%d Iterations\n"), iter);
            d_foreach_node(g, pr_route, NULL);
        }
    }
} /* process */


int main(int argc, char **argv)
{
    int opt;
    char *prog = argv[0];
    char *source = NULL;
    char *destination = NULL;

    while ((opt = getopt(argc, argv, "Dd:hs:")) >= 0) {
        switch (opt) {
        case 'D': flags |= D_FLAG_DEBUG; break;
        case 'd': destination = optarg; break;
        case 'h': do_help(prog, EXIT_SUCCESS); break;
        case 's': source = optarg; break;
        }
    }

    argc -= optind; argv += optind;

    if (argc > 0) {
        int i;
        for (i = 0; i < argc; ++i) {
            bool is_normal_file = strcmp(
                    argv[i],
                    STDIN_TOKEN) != 0;
            FILE *f = is_normal_file
                    ? fopen(argv[i], "r")
                    : stdin;

            if (!f) {
                fprintf(stderr,
                        F("FOPEN: %s: %s\n"),
                        argv[i],
                        strerror(errno));
                exit(EXIT_FAILURE);
            }
            process(f, is_normal_file
                    ? argv[i]
                    : STDIN_NAME,
                    source,
                    destination);
            if (is_normal_file) {
                fclose(f);
            }
        }
    } else {
        process(stdin, STDIN_NAME,
            source, destination);
    }
    exit(EXIT_SUCCESS);
}
