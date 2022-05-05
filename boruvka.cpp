/* Run make
 * Usage: ./boruvka -f <filename> -n <num_threads>
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <bits/stdc++.h>
#include <omp.h>
#include <chrono> 

typedef struct edge {
	unsigned int v1;
	unsigned int v2;
	int w;
} edge_t;

unsigned int n;
unsigned int m;
int maxWeight;
edge_t *edges;
std::vector<std::pair<unsigned int, unsigned int>> parent;
unsigned int nsets;
std::vector<edge> mst;
unsigned int mstWeight;

/* @brief Returns supervertex for given vertex */
unsigned int findParent(unsigned int v){
	if(parent[v].first == v){
		return v;
	} else {
		return findParent(parent[v].first);
	}
}

/* @brief Sets the vertex with higher rank to be the supervertex of the other */
void unionVerts(unsigned int v1, unsigned int v2){
	int pv1 = findParent(v1);
	int pv2 = findParent(v2);
	if(pv1 == pv2){
		return;
	}
	if(parent[pv1].second > parent[pv2].second){
		parent[pv2].first = pv1;
		parent[pv1].second += parent[pv2].second;
	} else {
		parent[pv1].first = pv2;
		parent[pv2].second += parent[pv1].second;
	}	
}

/* @brief Computes the minimum spanning tree using Boruvka's algorithm */
void findMST(unsigned int num_threads){
	while(nsets > 1){
		std::vector<unsigned int> cheapest(n, UINT_MAX);
		omp_lock_t lock[n];

		for(unsigned int i=0; i < n; i++){
			omp_init_lock(&(lock[i]));
		}

		// Iterates through all the edges and updates the cheapest edges for the
		// associated endpoints
		#pragma omp parallel num_threads (num_threads)
		{
			unsigned int threadId = omp_get_thread_num();
			for(unsigned int i = threadId % m; i < m; i += num_threads){
				unsigned int v1 = edges[i].v1;
				unsigned int v2 = edges[i].v2;
				int w = edges[i].w;

				unsigned int pv1 = findParent(v1);
				unsigned int pv2 = findParent(v2);
				if(parent[pv1].first != parent[pv2].first){
					//#pragma omp critical
					//{
						omp_set_lock(&lock[pv1]);
						if(cheapest[pv1] == UINT_MAX || edges[cheapest[pv1]].w > w){
							cheapest[pv1] = i;
						}
						omp_unset_lock(&lock[pv1]);
					//}


					//#pragma omp critical
					//{
						omp_set_lock(&lock[pv2]);
						if(cheapest[pv2] == UINT_MAX || edges[cheapest[pv2]].w > w){
							cheapest[pv2] = i;
						}
						omp_unset_lock(&lock[pv2]);
					//}
				}
			}
		}	

		// For each vertex, add the cheapest edge to the MST, if possible
		for(unsigned int j = 0; j < n; j++){
			unsigned int i = cheapest[j];
			if(i != UINT_MAX){
				unsigned int v1 = edges[i].v1;
				unsigned int v2 = edges[i].v2;
				int w = edges[i].w;

				unsigned int pv1 = findParent(v1);
				unsigned int pv2 = findParent(v2);
				//printf("edge: %d, v1: %d, v2: %d, w: %d\n", i, v1, v2, w);
				//printf("pv1: %d, pv2: %d\n", pv1, pv2);
				if(parent[pv1].first != parent[pv2].first){
					mst.push_back(edges[i]);
					mstWeight += w;
					unionVerts(v1, v2);
					nsets--;
					if(nsets == 0){
						break;
					}
				}
			}
		}

		for(unsigned int i=0; i < n; i++){
			omp_destroy_lock(&(lock[i]));
		}

	}
}

/* @brief Reads input file and initializes graph data structures */
void readInput(char *inputFilename){
	FILE *input = fopen(inputFilename, "r");
	if(!input){
		fprintf(stderr, "Unable to open file: %s\n", inputFilename);
		exit(EXIT_FAILURE);
	}

	// initialize global variables
	int inputLine = fscanf(input, "%i %i %d\n", &n, &m, &maxWeight);
	if(inputLine != 3){
		fprintf(stderr, "Input file %s is formatted incorrectly\n", inputFilename);
		exit(EXIT_FAILURE);
	}

	// initialize list of edges
	edges = (edge_t*)calloc(m, sizeof(edge_t));
	for(unsigned int i = 0; i < m; i++){
		int inputLine = fscanf(input, "%i %i %d\n", &edges[i].v1, &edges[i].v2, &edges[i].w);
		if(inputLine != 3){
			fprintf(stderr, "Input file %s is formatted incorrectly\n", inputFilename);
			exit(EXIT_FAILURE);
		}
	}

	// initialize vertex sets
	for(unsigned int i = 0; i < n; i++){
		parent.push_back(std::make_pair(i, 1));
	}
	nsets = n;
}

/* @brief Writes MST and weight to output file */
void writeOutput(){
	char outputFilename[80];
	sprintf(outputFilename, "output_%i_%i_%d.txt", n, m, maxWeight);

	FILE *output = fopen(outputFilename, "w");
	if(!output){
		fprintf(stderr, "Unable to open output file\n");
		exit(EXIT_FAILURE);
	}

	// write mst to output file
	fprintf(output, "%i %i %i\n", n, m, mstWeight);
	for( auto e : mst ){
		fprintf(output, "%i %i %d\n", e.v1, e.v2, e.w);
	}

	fclose(output);
}

int main(int argc, char *argv[]){
	using namespace std::chrono;
	typedef std::chrono::high_resolution_clock Clock;
	typedef std::chrono::duration<double> dsec;

	int opt;
	char *inputFilename = NULL;
	int num_threads = 1;

	while((opt = getopt(argc, argv, "f:n:")) != -1){
		switch(opt){
			case 'f':
				inputFilename = optarg;
				break;
			case 'n':
				num_threads = atoi(optarg);
				break;
			default:
				fprintf(stderr, "Usage: %s -f <filename> -n <num_threads>\n", argv[0]);
				exit(EXIT_FAILURE);
		}
	}

	if (inputFilename == NULL){
		fprintf(stderr, "Input filename is required\n");
		exit(EXIT_FAILURE);
	}

	readInput(inputFilename);
	auto compute_start = Clock::now();
	double compute_time = 0;
	findMST(num_threads);
	compute_time += duration_cast<dsec>(Clock::now() - compute_start).count();
	printf("Computation Time: %lf.\n", compute_time);
	writeOutput();

	return 0;
}
