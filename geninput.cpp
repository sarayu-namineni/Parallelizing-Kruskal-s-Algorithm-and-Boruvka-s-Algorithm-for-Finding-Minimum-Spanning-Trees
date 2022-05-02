/* Compile: make geninput
 * Usage: ./geninput -n <number of vertices> -m <number of edges> -w
 * <maximum edge weight>
 *
 * Given valid arguments, this program generates a graph with the given number
 * of vertices and edges with weights that do not exceed the given maximum. The
 * graph it generates is written to the file input_n_m_w.txt where n, m, and w
 * are the above command-line arguments. The output file consists of a list of
 * edges. Each edge is a represented by its two vertex endpoints and its weight.
 *
 */

#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<random>
#include<climits>
#include<list>
#include<map>
#include<algorithm>

typedef struct edge {
	unsigned int v1;
	unsigned int v2;
	int w;
} edge_t;

int main(int argc, char *argv[]){
	int opt, maxWeight;
   	unsigned int n, m;
	
	n = 0;
	m = 0;
	maxWeight = 1;
	while((opt = getopt(argc, argv, "n:m:w:")) != -1){
		switch(opt) {
			case 'n':
				n = std::stoul(optarg);
				break;
			case 'm':
				m = std::stoul(optarg);
				break;
			case 'w':
				maxWeight = atoi(optarg);
				break;
			default:
				fprintf(stderr, "Usage: %s -n <number of vertices> -m <number of edges> -w <maximum edge weight>\n", argv[0]);
				exit(EXIT_FAILURE);
			}
	}

	// check validity of arguments
	if(n < 0 || m < 0){
		fprintf(stderr, "Invalid args: Must input a non-negative number of nodes and edges\n");
		exit(EXIT_FAILURE);
	}
	if(maxWeight <= 0){
		fprintf(stderr, "Invalid args: Must input a positive maximum weight\n");
		exit(EXIT_FAILURE);
	}
	if(m > (n*(n-1))/2){
		fprintf(stderr, "Invalid args: Not possible to place %i edges among %i nodes\n", m, n);
		exit(EXIT_FAILURE);
	}

	/*int *adj = (int *)calloc(n*n, sizeof(int));
	if(adj == NULL){
		printf("Could not allocate adjacency matrix\n");
		exit(EXIT_FAILURE);
	}*/

	std::map<unsigned int, std::list<unsigned int>> adjList;
	std::map<unsigned int, std::list<unsigned int>>::iterator is_u;
	std::map<unsigned int, std::list<unsigned int>>::iterator is_v;
	edge_t* edges = (edge_t *)calloc(m, sizeof(edge_t));

	// keep track of number of edges placed
	unsigned int numEdges = 0;
	std::default_random_engine generator;
	std::uniform_int_distribution<unsigned int> distribution(0, UINT_MAX);
	while(numEdges < m){
		unsigned int u = distribution(generator) % n; // chooses node between 0 and n-1
		unsigned int v = distribution(generator) % n;
		int w = (distribution(generator) % maxWeight) + 1;

		is_u = adjList.find(u);
		is_v = adjList.find(v);
		if(is_u != adjList.end() && is_v != adjList.end()){
			auto ue = std::find(std::begin(adjList[u]), std::end(adjList[u]), v);
			auto ve = std::find(std::begin(adjList[v]), std::end(adjList[v]), u);
			if(ue == adjList[u].end() && ve == adjList[v].end()){
				adjList[u].push_back(v);
				adjList[v].push_back(u);
				edges[numEdges].v1 = u;
				edges[numEdges].v2 = v;
				edges[numEdges].w = w;
				numEdges++;
			} else if (ue == adjList[u].end() || ve == adjList[v].end()){
				printf("Hash map is inconsistent\n");
				exit(EXIT_FAILURE);
			}
		} else if(is_u != adjList.end() || is_v != adjList.end()){
			/*
			printf("Hash map is inconsistent\n");
			exit(EXIT_FAILURE);
			*/
		} else {
			adjList.insert(std::pair<unsigned int, std::list<unsigned int>>(u, { v }));
			adjList.insert(std::pair<unsigned int, std::list<unsigned int>>(v, { u }));
			edges[numEdges].v1 = u;
			edges[numEdges].v2 = v;
			edges[numEdges].w = w;
			numEdges++;
		}
	}

	char outputFilename[80];
	sprintf(outputFilename, "input_%i_%i_%d.txt", n, m, maxWeight);

	FILE *output = fopen(outputFilename, "w");
	if(!output){
		fprintf(stderr, "Unable to open output file\n");
		exit(EXIT_FAILURE);
	}

	// write edges and weights to output file
	fprintf(output, "%i %i %d\n", n, m, maxWeight);
	for(unsigned int i = 0; i < m; i++){
		fprintf(output, "%i %i %d\n", edges[i].v1, edges[i].v2, edges[i].w);
	}

	fclose(output);
	free(edges);

	return 0;
}
