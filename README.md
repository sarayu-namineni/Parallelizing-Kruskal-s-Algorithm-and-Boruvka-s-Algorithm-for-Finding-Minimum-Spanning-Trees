# Parallelizing-Kruskal-s-Algorithm-and-Boruvka-s-Algorithm-for-Finding-Minimum-Spanning-Trees
## SUMMARY

In our project, we plan to parallelize minimum spanning tree algorithms by, first, parallelizing Kruskal’s algorithm and, then, parallelizing Boruvka’s algorithm. In implementing Kruskal’s algorithm, we plan to parallelize the sorting step, and in implementing Boruvka’s algorithm, we plan to explore and utilize tree and/or star contraction techniques; in both, we plan to use C++ and OpenMP to attempt to optimize performance effectively and improve speedup from the sequential to parallel version. After implementation, we plan to compare the aforementioned approaches via running them on various sized graphs with random weights and measuring and analyzing performance and speedup on various numbers of threads on the GHC and/or PSC machines.

## BACKGROUND

In our project, we are implementing and parallelizing Kruskal’s algorithm and Boruvka’s algorithm, which are minimum spanning tree algorithms. 

To understand these algorithms’ goals, we must first define the notion of a tree, which is an undirected graph that has no cycles. Given a connected, undirected graph G = (V, E) (where the graph is defined by its set of vertices i.e. V and set of edges i.e. E), we can identify a subset of the edges in G that, together, create a tree while also including all the vertices in G. This tree is called a spanning tree and, more formally, we can say that a spanning tree of G is defined by a tree T = (V, E’) where E’ ⊆ E. We can also note that the number of edges in a spanning tree is |V|-1, since a tree is acyclic. Since a graph can have many spanning trees, we are sometimes interested in finding the minimum spanning tree (MST) of G, or the spanning tree of G with the smallest total weight (where weight of the tree is defined by the sum of the weights of its edges).

Most MST algorithms utilize the Light-Edge Property, which states that if G is a connected, undirected, weighted graph (where each edge weight is distinct), for any cut of G, the minimum weight edge that crosses the cut is contained in G’s MST. In other words, upon partitioning G’s vertices into two nonempty subsets, out of the edges that cross the cut (i.e. one vertex is in one subset and the other vertex is in the other subset), the minimum weight edge must be in the MST.

This property allows us to introduce Kruskal’s algorithm, which builds the MST of G via adding the least-weight edge that doesn’t create a cycle with the edges already chosen. The Light-Edge property is relevant here, as it showcases how among the edges that are being considered that don't create a cycle, we know the minimum weight one must be in the MST, since it’s connecting a vertex amongst the edges already chosen (which from a connected subgraph so far) and the vertex from the rest of the edges that haven’t been chosen (thereby going across a cut). Using Union-Find can allow us to efficiently determine if an edge forms a cycle by checking whether the edge’s vertices are in the same connected component. However, the Light-Edge property also indicates the importance of finding the minimum weight edge, a task that can benefit from parallelism, as we can implement parallel sorting algorithms like parallel quicksort, parallel merge sort, and/or parallel samplesort instead of sorting sequentially. This can help improve speedup, since any run of Kruskal’s must sort the graph’s edges by weight.

Note that the Light-Edge property, as well as the aforementioned parallel sorting, helps imply that there exist parallel algorithms to compute the MST of G that can choose, in parallel, which certain edges should be included based on their weight and add them at once. Particularly, by defining a cut consisting of vertex in G (let’s call vertex u) and all the other vertices in the graph, we can say that the edges on this specific cut are incident on u but also, by the Light-Edge property, out of these edges, the minimum weight one must be in MST. One aspect here that can benefit from parallelism is how each vertex is finding its own minimum weight incident edge, and this could potentially be done all at once, since they are independent tasks. 

Relating to this, note that in the sequential version of Boruvka’s, general graph contraction would be used, choosing the minimum weight edge incident on any vertex would create a partition of the graph such that these partitions can be condensed into 1 representative vertex and the minimum weight cross edge could be chosen. However, reducing all the way to a single vertex can be sped up via tree contraction, in which we apply contraction solely on the “tree edges”, which are essentially the edges within each partition created by how for every vertex, the minimum weight edge is chosen to be in the MST. Since star contraction is used to contract the block, the number of edges importantly decreases as the number of vertices decreases. Star contraction without tree contraction could also be used to contract the subgraph created, which can further improve speedup. 

Overall, then, with Boruvka’s, what benefits from parallelism is how each thread can work on finding the minimum weight edge incident on each vertex in G in parallel, meaning the edges that need to be added to the MST can be chosen and added in parallel, theoretically drastically improving speedup over the sequential version where each vertex finds its minimum weight edge sequentially. We can conduct this parallelism, because of how each vertex is essentially independent and is simply choosing the minimum weight edge incident to it. Every vertex defines a cut, and Boruvka’s uses this fact to construct the MST in parallel. 

## THE CHALLENGE

Kruskal’s algorithm consists of sorting the edges by weight and iterating over all of the edges in sorted order, adding an edge to our minimum spanning tree if it does not create a cycle. One potential area for parallelism is during the sorting step; the rest of the algorithm, however, is inherently sequential since determining whether a certain edge creates a cycle is dependent on the edges that have already been added. Implementing Kruskal’s still poses a challenge since it makes use of data structures such as the union-find structure, which we will have to not only use when implementing Boruvka’s algorithm but also figure out if we can avoid contention on these structures and avoid frequent updates to try to obtain larger speedups and prevent cache misses from occurring. Another potential challenge here is exploring different parallel sorting algorithms and utilizing divide and conquer methods to try to achieve speedup over sequential sorting.

Boruvka’s algorithm offers more opportunities for parallelism along with more challenges. Boruvka’s algorithm consists of finding the lightest adjacent edge for every vertex in a graph and then contracting those edges. We can find the lightest adjacent edge for every vertex in parallel, but when it comes to contracting those edges, we could run into race conditions when multiple edges try to contract the same vertex. A potential challenge is, thus, figuring out how to correctly parallelize this contraction, as well as exploring whether certain types of contraction (edge, tree, and/or star) may be more effective for parallelization.

It is unclear what mapping of workload to processors would be most efficient and scalable. One idea we have is to assign different connected components to each processor, although the number and size of these components is dependent on the edges that get contracted first in our graph. We anticipate having high contention over our shared data structures, such as our graph representation and our union find structure, unless we can map our accesses within our data structures to these different processors. 

## RESOURCES:

We will be implementing Kruskal’s and Boruvka’s algorithms from scratch in C++ and OpenMP. We will refer to the 15-210 lecture notes (cited at bottom of proposal) for overarching descriptions of the algorithms. We will be testing our implementation on the GHC and/or PSC machines, which we already have access to.

## GOALS AND DELIVERABLES:

### Preliminary Goals (“75%”)

For our preliminary goals, we plan to implement a fast sequential version of Kruskal's algorithm with a union-find data structure from scratch. We also intend to parallelize the sorting step of this algorithm using C++ and OpenMP by implementing a parallel sorting algorithm from scratch. We also plan to compare the sequential version with the parallelized version of Kruskal’s using the same benchmarking described in below Plan to Achieve Goals (“100%”) section.

### Plan to Achieve Goals (“100%”)

We plan to parallelize Boruvka’s algorithm using one type of contraction (one of star, edge or tree contraction). We plan to compare the performance differences between Boruvka’s algorithm parallelized with one type of contraction and Kruskal’s algorithm with a parallel sorting step. 

We plan to compare the performance differences via benchmarking the speedup performance under different numbers of threads and the speedup performance under different input graph types (e.g. graphs with small number of vertices vs. graphs with large number of vertices where each graph has random edge weights; sparse graphs vs. dense graphs; etc). Note that the speedups for Kruskal’s algorithm with a parallel sorting step will be calculated against the time taken for this parallel version to run on 1 thread (and/or the time taken for the sequential algorithm to run). The speedups for Boruvka’s algorithm will be calculated against the time taken for this parallel algorithm to run on 1 thread. 

In comparing these speedups, we will explain why they may be increasing or decreasing with certain numbers of threads and graph types and reference our specific parallel implementations/choices when explaining. 

The performance goal in terms of this benchmarking is that we hope to achieve greater speedups for Boruvka’s algorithm (implemented with 1 type of contraction) than with Kruskal’s algorithm, as Kruskal’s algorithm has some largely sequential parts whereas with Boruvka’s, we would be parallelizing the adding of the edges themselves rather than just sorting them in parallel and sequential iterating over them.

In addition to these goals, we also plan to achieve all the goals listed in the above Preliminary Goals (“75%”) section.

### Hope to Achieve Goals (“125%”)

If we have extra time, we plan to additionally implement two or more types of parallel contractions for Boruvka’s algorithm. We plan to then compare performance results between Kruskal’s algorithm, Boruvka’s algorithm with one type of contraction, and Boruvka’s algorithm with another type of contraction (whose speedups will all be calculated as explained in the prior section). The performance will be benchmarked and compared just as described in the prior section.

### Demo We Plan to Show at the Poster Session

At the poster session, we will show speedup graphs of the performance of Kruskal’s algorithm on various different numbers of threads and on various types of graphs. We will show speedup graphs of the performance of Boruvka’s algorithm (with one type of contraction) on various different numbers of threads and on various types of graphs. If we end up achieving implementation of another type of contraction for Boruvka’s algorithm, we will include speedup graphs for that too. In addition, we may showcase additional graphs or tables of data that compare specific speedup numbers (via values like the mean, median, or standard deviation) across different algorithms we implemented or across different input types within the same algorithm.

Particularly, we feel that by these above deliverables and by showcasing that a reasonable speedup occurs and that this speedup generally increases as the number of threads increases, we will demonstrate that we did a good job. If we’re not able to obtain speedup, we will attempt to conduct analysis via profiling our code to demonstrate/explain limitations in our implementation that may be leading to parallel overhead and bottlenecking in regards to our testing on different numbers of threads/graph types.

### What We Are Hoping to Learn About the Workload Being Studied

In addition to analyzing speedups on different numbers of threads, we also are hoping to learn more about the effect of parallelization on different input graph types and connecting that to how we can map the workload to shared graph and union find data structures in our parallel implementations of finding a minimum spanning tree, especially as many threads may be accessing and modifying these structures at once.

## PLATFORM CHOICE:

We will be using the C++ programming language and the OpenMP parallel programming framework to implement our algorithms. These languages/frameworks are good for our needs, as C++, for example, allows us to implement the sequential versions of our algorithms and OpenMP allows us to directly modify our C++ code to employ parallelization techniques using different numbers of threads. 

We will use the GHC and/or the PSC machines to test our implementation and run it on different numbers of threads to obtain our performance metrics. If testing with a large number of cores, it makes sense for us to use PSC machines. Further, we’ve used the PSC machines for a prior homework assignment involving OpenMP. 

## SCHEDULE:

<table>
  <tr>
   <td>Due Date
   </td>
   <td>Task
   </td>
  </tr>
  <tr>
    <td> 3/23
    </td>
    <td> Project proposal
    </td>
  </tr>
  <tr>
   <td>3/28
   </td>
   <td>Formulate graph representation and APIs for data structures
   </td>
  </tr>
  <tr>
   <td>3/28
   </td>
   <td>Implement a graph input generator; Try generating different types of graphs
   </td>
  </tr>
  <tr>
   <td>4/4
   </td>
   <td>Implement a sequential version of Kruskal’s algorithm
   </td>
  </tr>
  <tr>
   <td>4/11
   </td>
   <td>Parallelize the sorting step in Kruskal’s algorithm
   </td>
  </tr>
  <tr>
   <td>4/11
   </td>
   <td>Benchmark and profile Kruskal’s algorithm
   </td>
  </tr>
  <tr>
   <td>4/11
   </td>
   <td>Project checkpoint report
   </td>
  </tr>
  <tr>
   <td>4/15
   </td>
   <td>Implement a sequential version of Boruvka’s algorithm
   </td>
  </tr>
  <tr>
   <td>4/22
   </td>
   <td>Implement a parallel version of Boruvka’s algorithm (using one type of contraction)
   </td>
  </tr>
  <tr>
   <td>4/29
   </td>
   <td>Benchmark and profile Boruvka’s algorithm
   </td>
  </tr>
  <tr>
   <td>4/29
   </td>
   <td>Final report
   </td>
  </tr>
</table>

## CITATIONS:
[1] (2021). Textbook (III): Part 4: Minimum Spanning Trees. Diderot: 15210. 
https://www.diderot.one/courses/89/books/352/chapter/4581 

