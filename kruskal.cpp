// g++ -fopenmp -o kruskal kruskal.cpp -std=c++11
// ./kruskalc -f exGraph1.txt
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <bits/stdc++.h>
#include <omp.h>
#include <chrono>



typedef struct edge {
    int u;
    int v;
    int w;
} edge;

int n; // num of vertices
int m; // num of edges
int maxWeight;
edge *resultList;
edge *edgeList;
double globalTime = 0;

int find(int* parentRepList, int vertToFind) {
    if(parentRepList == NULL) {
        printf("null in find\n");
    }
    if(parentRepList[vertToFind] == vertToFind) {
        return vertToFind;
    } else {
        return find(parentRepList, parentRepList[vertToFind]);
    }
}

void unionVerts(int* parentRepList, int* depthAtVertList, int vert1, int vert2) {
    int p1 = find(parentRepList, vert1);
    int p2 = find(parentRepList, vert2);
    if(p1 == p2) {
        return;
    }

    if(depthAtVertList[p1] == depthAtVertList[p2]) {
        parentRepList[p2] = p1;
        depthAtVertList[p1] += 1;
    } else if(depthAtVertList[p1] < depthAtVertList[p2]) {
        parentRepList[p1] = p2;
    } else {
        parentRepList[p2] = p1;
    }
}


void merge(edge *edgeList, int start, int mid, int end) {
    int leftLen = mid-start+1;
    int rightLen = end-1-mid;
    edge *leftPart = (edge*)calloc(leftLen, sizeof(edge));
    edge *rightPart = (edge*)calloc(rightLen, sizeof(edge));

    // create left and right arrays to merge and populate directly into edgeList
    for(int i = 0; i < leftLen; i++) {
        leftPart[i] = edgeList[start+i];
    }
    for(int i = 0; i < rightLen; i++) {
        rightPart[i] = edgeList[mid+i+1];
    }

    int listIdx = start;
    int leftIdx = 0;
    int rightIdx = 0;

    while((leftIdx < leftLen) && (rightIdx < rightLen)) {
        if(leftPart[leftIdx].w <= rightPart[rightIdx].w) {
            edgeList[listIdx] = leftPart[leftIdx];
            leftIdx++;
            listIdx++;
        } else if(leftPart[leftIdx].w > rightPart[rightIdx].w) {
            edgeList[listIdx] = rightPart[rightIdx];
            rightIdx++;
            listIdx++;
        }
    }

    // populate any leftover elements
    while(leftIdx < leftLen) {
        edgeList[listIdx] = leftPart[leftIdx];
        leftIdx++;
        listIdx++;
    }

    while(rightIdx < rightLen) {
        edgeList[listIdx] = rightPart[rightIdx];
        rightIdx++;
        listIdx++;
    }
}

void mergeSortSeq(edge *edgeList, int start, int end) {
    using namespace std::chrono;
    typedef std::chrono::high_resolution_clock Clock;
    typedef std::chrono::duration<double> dsec;
    if(start >= end-1) {
        return;
    }
    int mid = ((end-2)+start)/2;

    mergeSortSeq(edgeList, start, mid+1);
    mergeSortSeq(edgeList, mid+1, end);
    auto compute_start = Clock::now();
    merge(edgeList, start, mid, end);
    globalTime += duration_cast<dsec>(Clock::now() - compute_start).count();
}

// For parallelizing mergeSort using tasks, we adapted structure of
// code from slide 7 of this Oregon State University lecture
// https://web.engr.oregonstate.edu/~mjb/cs575/Handouts/tasks.1pp.pdf
// (Namely just using tasks to do 2 things at once and single to ensure
// only 1 thread enqueues the tasks.)
void mergeSort(edge *edgeList, int start, int end) {
    using namespace std::chrono;
    typedef std::chrono::high_resolution_clock Clock;
    typedef std::chrono::duration<double> dsec;
    // end is exclusive
    if(start >= end-1) {
        return;
    }
    int mid = ((end-2)+start)/2;

    omp_set_num_threads(1);
    #pragma omp parallel
    {
        #pragma omp single
        {
            // start 2 parallel tasks to sort 2 halves
            #pragma omp task
            {
                mergeSort(edgeList, start, mid+1);
            }
            #pragma omp task
            {
                mergeSort(edgeList, mid+1, end);
            }
        }
    }
    auto compute_start = Clock::now();
    merge(edgeList, start, mid, end); // merge start to mid WITH mid to end
    globalTime += duration_cast<dsec>(Clock::now() - compute_start).count();
}

void readInput(char *inputFilename) {
    FILE *input = fopen(inputFilename, "r");
    if(!input){
        fprintf(stderr, "Unable to open file:  %s\n", "hmm");
        exit(EXIT_FAILURE);
    }

    // need to populate n and m (the global vars)
    int inputLine = fscanf(input, "%d %d %d\n", &n, &m, &maxWeight);
    if(inputLine != 3) {
        fprintf(stderr, "Input file %s is formatted incorrectly\n", inputFilename);
        exit(EXIT_FAILURE);
    }


    // CREATING EDGE LIST (& reading rest of lines in input graph file)
    //edgeList = (edge*)malloc((m*2)*sizeof(edge));
    edgeList = (edge*)calloc((m*2), sizeof(edge));
    if(edgeList == NULL) {
        printf("malloc error");
        exit(EXIT_FAILURE);
    }

    int indexOfEdgeList = 0;
    for(int i = 0; i < m; i++){
        int inputLine = fscanf(input, "%d %d %d\n", &edgeList[indexOfEdgeList].u, &edgeList[indexOfEdgeList].v, &edgeList[indexOfEdgeList].w);
        if(inputLine != 3){
            fprintf(stderr, "Input file %s is formatted incorrectly\n", inputFilename);
            exit(EXIT_FAILURE);
        }
        indexOfEdgeList += 1;
        // add opposite edge since this is an undirected graph
        edgeList[indexOfEdgeList].u = edgeList[indexOfEdgeList-1].v;
        edgeList[indexOfEdgeList].v = edgeList[indexOfEdgeList-1].u;
        edgeList[indexOfEdgeList].w = edgeList[indexOfEdgeList-1].w;
        indexOfEdgeList += 1;
    }

}

void writeOutput() {
    char outputFilename[80];
    sprintf(outputFilename, "output_%d_%d_%d.txt", n, m, maxWeight);

    FILE *output = fopen(outputFilename, "w");
    if(!output){
        fprintf(stderr, "Unable to open output file\n");
        exit(EXIT_FAILURE);
    }

    // write mst result to output file
    fprintf(output, "%d %d %d\n", n, m, maxWeight);

    for(int j = 0; j < n-1; j++) {
        fprintf(output, "%d %d %d\n", resultList[j].u, resultList[j].v, resultList[j].w);
    }
    fclose(output);


}




int main(int argc, char *argv[]) {
    using namespace std::chrono;
    typedef std::chrono::high_resolution_clock Clock;
    typedef std::chrono::duration<double> dsec;

    int opt;
    char *inputFilename = NULL;
    while((opt = getopt(argc, argv, "f:")) != -1){
        switch(opt){
            case 'f':
                inputFilename = optarg;
                break;
            default:
                fprintf(stderr, "Usage: %s -f <filename>\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (inputFilename == NULL){
        fprintf(stderr, "Input filename is required\n");
        exit(EXIT_FAILURE);
    }

    readInput(inputFilename);


    // start time
    auto compute_start = Clock::now();
    double compute_time = 0;


    // UNION FIND SET UP
    int *parentRepList = (int*)calloc(n, sizeof(int));
    if(parentRepList == NULL) {
        printf("malloc error");
        return 0;
    }
    for(int i = 0; i < n; i++) {
        parentRepList[i] = i;
    }
    int *depthAtVertList = (int*)calloc(n, sizeof(int));
    if(depthAtVertList == NULL) {
        printf("malloc error");
        return 0;
    }
    for(int i = 0; i < n; i++) {
        depthAtVertList[i] = 0;
    }


    // RUN KRUSKAL
    resultList = (edge*)calloc((n-1), sizeof(edge));

    // TIME MEASURE 1 (before meerge)
    double time1 = duration_cast<dsec>(Clock::now() - compute_start).count();
    printf("Time1: %lf.\n", time1);

    // Sort edge list (length 2*m since including undirected edges)
    mergeSort(edgeList, 0, (2*m));
    //printf("\nDone with merge sort\n");

    double time2 = duration_cast<dsec>(Clock::now() - compute_start).count();
    printf("Time2: %lf.\n", time2);
    printf("MergeTime: %lf.\n", globalTime);

    int numEdgesSoFar = 0;
    int i = 0;

    // Loop until n-1 edges have been found to create the MST
    while(numEdgesSoFar < n-1) {
        int vert1 = edgeList[i].u;
        int vert2 = edgeList[i].v;
        int currW = edgeList[i].w;

        int parent1 = find(parentRepList, vert1);
        int parent2 = find(parentRepList, vert2);

        // Ensure connecting vert1 and vert2 doesn't create a cycle
        if(parent1 != parent2) {
            resultList[numEdgesSoFar].u = vert1;
            resultList[numEdgesSoFar].v = vert2;
            resultList[numEdgesSoFar].w = currW;
            numEdgesSoFar += 1;
            unionVerts(parentRepList, depthAtVertList, vert1, vert2);
        }
        i+=1;
    }


    // Print out result MST
    /*printf("RESULT\n");
    for(int j = 0; j < n-1; j++) {
        printf("%d , %d , %d\n", resultList[j].u, resultList[j].v, resultList[j].w);
    }*/

    // end time
    compute_time += duration_cast<dsec>(Clock::now() - compute_start).count();
    printf("Computation Time: %lf.\n", compute_time);


    // Write output to a file
    writeOutput();
    free(edgeList);
    free(resultList);
    free(parentRepList);
    free(depthAtVertList);


    return 0;
}
