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
	int v1;
	int v2;
	int w;
} edge_t;

int n;
int m;
int maxWeight;
edge_t *edges;
std::vector<std::pair<int, int>> parent;
int nsets;
std::vector<edge> mst;
int mstWeight;

/* @brief Returns supervertex for given vertex */
int findParent(int v){
	if(parent[v].first == v){
		return v;
	} else {
		return findParent(parent[v].first);
	}
}

/* @brief Sets the vertex with higher rank to be the supervertex of the other */
void unionVerts(int v1, int v2){
	if(parent[v1].second > parent[v2].second){
		parent[v2].first = v1;
		parent[v1].second += parent[v2].second;
	} else {
		parent[v1].first = v2;
		parent[v2].second += parent[v1].second;
	}	
}

/* @brief Computes the minimum spanning tree using Boruvka's algorithm */
void findMST(int num_threads){
	while(nsets > 1){
		std::vector<int> cheapest(n, -1);
		omp_lock_t lock[n];

		for(int i=0; i < n; i++){
			omp_init_lock(&(lock[i]));
		}

		// Iterates through all the edges and updates the cheapest edges for the
		// associated endpoints
		#pragma omp parallel num_threads (num_threads)
		{
			int threadId = omp_get_thread_num();
			for(int i = threadId % m; i < m; i += num_threads){
				int v1 = edges[i].v1;
				int v2 = edges[i].v2;
				int w = edges[i].w;

				int pv1 = findParent(v1);
				int pv2 = findParent(v2);
				if(parent[pv1].first != parent[pv2].first){
					//#pragma omp critical
					//{
						omp_set_lock(&lock[v1]);
						if(cheapest[v1] == -1 || edges[cheapest[v1]].w > w){
							cheapest[v1] = i;
						}
						omp_unset_lock(&lock[v1]);
					//}


					//#pragma omp critical
					//{
						omp_set_lock(&lock[v2]);
						if(cheapest[v2] == -1 || edges[cheapest[v2]].w > w){
							cheapest[v2] = i;
						}
						omp_unset_lock(&lock[v2]);
					//}
				}
			}
		}	

		// For each vertex, add the cheapest edge to the MST, if possible
		for(int j = 0; j < n; j++){
			int i = cheapest[j];
			if(i != -1){
				int v1 = edges[i].v1;
				int v2 = edges[i].v2;
				int w = edges[i].w;

				int pv1 = findParent(v1);
				int pv2 = findParent(v2);
				//printf("edge: %d, v1: %d, v2: %d, w: %d\n", i, v1, v2, w);
				//printf("pv1: %d, pv2: %d\n", pv1, pv2);
				if(parent[pv1].first != parent[pv2].first){
					mst.push_back(edges[i]);
					mstWeight += w;
					unionVerts(v1, v2);
					nsets--;
				}
			}
		}

		for(int i=0; i < n; i++){
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
	int inputLine = fscanf(input, "%d %d %d\n", &n, &m, &maxWeight);
	if(inputLine != 3){
		fprintf(stderr, "Input file %s is formatted incorrectly\n", inputFilename);
		exit(EXIT_FAILURE);
	}

	// initialize list of edges
	edges = (edge_t*)calloc(m, sizeof(edge_t));
	for(int i = 0; i < m; i++){
		int inputLine = fscanf(input, "%d %d %d\n", &edges[i].v1, &edges[i].v2, &edges[i].w);
		if(inputLine != 3){
			fprintf(stderr, "Input file %s is formatted incorrectly\n", inputFilename);
			exit(EXIT_FAILURE);
		}
	}

	// initialize vertex sets
	for(int i = 0; i < n; i++){
		parent.push_back(std::make_pair(i, 1));
	}
	nsets = n;
}

/* @brief Writes MST and weight to output file */
void writeOutput(){
	char outputFilename[80];
	sprintf(outputFilename, "output_%d_%d_%d.txt", n, m, maxWeight);

	FILE *output = fopen(outputFilename, "w");
	if(!output){
		fprintf(stderr, "Unable to open output file\n");
		exit(EXIT_FAILURE);
	}

	// write mst to output file
	fprintf(output, "%d %d %d\n", n, m, mstWeight);
	for( auto e : mst ){
		fprintf(output, "%d %d %d\n", e.v1, e.v2, e.w);
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
