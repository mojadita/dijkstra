#include <errno.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dijkstra.h"

#define F(_fmt) __FILE__":%d:%s: "_fmt,__LINE__,__func__

#define SEP_STRING		", \t\n"
#define FINALSEP_STRING " \t\n"
#define STDIN_TOKEN		"-"
#define STDIN_NAME		"stdin"

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

void process(FILE *f, char *name)
{
	char line[256];
	int lineno = 0;
	struct d_graph *g = g_new_graph();

	while (fgets(line, sizeof line, f)) {
		++lineno;

		/* first param is source node */
		char *from = strtok(line, SEP_STRING);
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
		char *rest = strtok(line, FINALSEP_STRING);
		if (rest) {
			sscanf(rest, "%d", &weight);
		}
		if ((res = g_add_link(
                        g_lookup_node(g, from),
                        g_lookup_node(g, to),
                        weight)) != 0) {
			fprintf(stderr,
					"WARNING: add_link(%s, %s) => %d\n",
					from, to, weight);
		}
	}
	init_dijkstra(g);
	free_dijkstra(g);
}
	

int main(int argc, char **argv)
{
	int opt;
	char *prog = argv[0];
	char *source = NULL;
	char *destination = NULL;

	while ((opt = getopt(argc, argv, "d:hs:")) >= 0) {
		switch (opt) {
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
					: STDIN_NAME);
			if (is_normal_file) {
				fclose(f);
			}
		}
	} else {
		process(stdin, STDIN_NAME)
	}
	exit(EXIT_SUCCESS);
}
