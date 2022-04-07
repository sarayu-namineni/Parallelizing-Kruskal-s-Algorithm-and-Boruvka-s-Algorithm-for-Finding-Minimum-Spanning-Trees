/* Compile: gcc -o generate-input generate-input.c
 * Usage: ./generate-input -n [number of vertices] -m [number of edges] -w
 * [maximum edge weight]
 *
 * Given valid arguments, this program generates a graph with the given number
 * of vertices and edges with weights that do not exceed the given maximum. The
 * graph it generates is written to the file output_n_m_w.txt where n, m, and w
 * are the above command-line arguments. The output file consists of a list of
 * edges. Each edge is a represented by its two vertex endpoints and its weight.
 *
 */

#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>

int main(int argc, char *argv[]){
	int opt, n, m, maxWeight;
	
	n = 0;
	m = 0;
	maxWeight = 1;
	while((opt = getopt(argc, argv, "n:m:w:")) != -1){
		switch(opt) {
			case 'n':
				n = atoi(optarg);
				break;
			case 'm':
				m = atoi(optarg);
				break;
			case 'w':
				maxWeight = atoi(optarg);
				break;
			default:
				fprintf(stderr, "Usage: %s [-n verts] [-m edges] [-w maxWeight]\n", argv[0]);
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
		fprintf(stderr, "Invalid args: Not possible to place %d edges among %d nodes\n", m, n);
		exit(EXIT_FAILURE);
	}

	int *adj = calloc(n*n, sizeof(int));

	// keep track of number of edges placed
	int numEdges = 0;
	while(numEdges < m){
		int u = rand() % n; // chooses node between 0 and n-1
		int v = rand() % n;
		int w = rand() % maxWeight;

		// no self-edges
		if(u != v){
			adj[u*n+v] = w;
			numEdges++;
		}
	}

	char outputFilename[80];
	sprintf(outputFilename, "output_%d_%d_%d.txt", n, m, maxWeight);

	FILE *output = fopen(outputFilename, "w");

	// write edges and weights to output file
	for(int i = 0; i < n; i++){
		for(int j = 0; j < n; j++){
			if(adj[i*n+j] > 0){
				fprintf(output, "%d %d %d\n", i, j, adj[i*n+j]);
			}
		}
	}

	fclose(output);
	free(adj);

	return 0;
}
