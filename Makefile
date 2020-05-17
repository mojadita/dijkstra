# Makefile -- build file for the Dijkstra algorithm program.
# Author: Luis Colorado <luiscoloradourcola@gmail.com>
# Date: Thu May 14 15:45:45 EEST 2020
# Copyright: (C) 2020 Luis Colorado.  All rights reserved.
# License: BSD.

targets             = dijkstra
toclean             = $(targets)
RM                 ?= rm -f

all: $(targets)
clean:
	$(RM) $(toclean)

dijkstra_deps       =
dijkstra_objs       = main.o dijkstra.o
dijkstra_libs       = -lavl
dijkstra_ldflags    = -Lavl_c

toclean            += $(dijkstra_objs)

dijkstra: $(dijkstra_deps) $(dijkstra_objs)
	$(CC) $(LDFLAGS) $($@_ldflags) -o $@ $($@_objs) $($@_libs)
