# Dijkstra's Minimum Cost Path Algorithm.

This is Dijkstra's Minimum Cost Path Algorithm. It is implemented
after giving a response to [this question in
StackOverflow](https://stackoverflow.com/a/61793333/3899431).
The trick here, to allow to build the back track of each node is
selecting the link (which points both, the origin node and the
destination node, and the weight of the link) instead of the
node, when searching for candidates.  This way, you can
reconstruct and store a back-link with the node, so the track
back to the origin node can be followed and so, the full path can
be reconstructed from the destination node.

The program reads a file with the links to build a full graph.
The syntax is:

* Each line stores information for a single link.  It is composed
of three fields, origin node (a string, without spaces)
destination node (same) and a weight (an integer number)
* The program constructs a database (using an AVL tree, from the
library used as git submodule) of nodes.  For each node, it
constructs an array of links (using a dynamically re/allocated array
of links, when `next_c`--capacity is reached by `next_n`--size,
the capacity is doubled, from an initial value of
`DEFAULT_CAPACITY` in `dijkstra.c`)

The algorithm maintains the arrays of links sorted, using the
standard library `qsort(3)` routine, that is executed for each
node and algorithm start.  It only sorts a node if it needs
sorting (only if the array has been modified adding links) so a
second run of dijkstra's algorithm on a differen start node,
doesn't mean to sort the arrays again.

the algorithm maintains an iterator per node that follows the
links in ascending order, so we know when a node is exhausted to
eliminate it from the set of frontier nodes (which is maintained
as a double linked list of nodes from the `d_dijkstra()`
function.  The algorithm ends once the destination node is
selected as candidate for destination, or when the frontier is
exhausted (this allows you to specify a `NULL` node as
destination, in that case the `d_dijkstra()` function doesn't
have a destination node, and the minimum path to each of the
nodes is calculated for each node.
