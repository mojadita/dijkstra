# Makefile -- build file for the Dijkstra algorithm program.
# Author: Luis Colorado <luiscoloradourcola@gmail.com>
# Date: Thu May 14 15:45:45 EEST 2020
# Copyright: (C) 2020 Luis Colorado.  All rights reserved.
# License: BSD.

targets				= dijkstra reverse
toclean				= $(targets)
RM				   ?= rm -f

all: $(targets)
clean:
	$(RM) $(toclean)

dijkstra_deps   	= avl_c
dijkstra_objs   	= main.o dijkstra.o
dijkstra_libs		= 
dijkstra_ldflags	=

reverse_deps		=
reverse_objs		= reverse.o
reverse_libs		=
reverse_ldflags		=

.for t in $(targets)
toclean			   += $($t_objs)
$t: $($t_deps) $($t_objs)
	$(CC) $(LDFLAGS) $($@_ldflags) -o $@ $($@_objs) $($@_libs)
.endfor

avl_c:
	$(MAKE) -C $@
