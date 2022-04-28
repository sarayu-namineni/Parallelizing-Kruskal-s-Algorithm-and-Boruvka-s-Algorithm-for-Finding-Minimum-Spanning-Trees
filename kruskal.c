// Currently takes exGraph1.txt as input
// gcc -fopenmp kruskal.c -o kruskalc -std=c99
// ./kruskalc -f exGraph1.txt
#include <assert.h>
#include <omp.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <time.h>



typedef struct edge {
    int u;
    int v;
    int w;
} edge;

int n = 6; // num of vertices
int m = 8; // num of edges


int find(int* parentRepList, int vertToFind) {
    if(parentRepList == NULL) {
        printf("null in find\n");
    }
    if(parentRepList[vertToFind] == vertToFind) {
        return vertToFind;
    }
    find(parentRepList, parentRepList[vertToFind]);
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
    edge *leftPart = malloc(leftLen*sizeof(edge));
    edge *rightPart = malloc(rightLen*sizeof(edge));

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

void mergeSort(edge *edgeList, int start, int end) {
    // end is exclusive
    if(start >= end-1) {
        return;
    }
    int mid = ((end-2)+start)/2;

    // start 2 parallel tasks to sort 2 halves
    #pragma omp task
    mergeSort(edgeList, start, mid+1);
    #pragma omp task
    mergeSort(edgeList, mid+1, end);
    #pragma omp taskwait
    merge(edgeList, start, mid, end); // merge start to mid WITH mid to end
}



int main(int argc, char *argv[]) {
    // CREATING EDGE LIST
    edge* edgeList = malloc((m*2)*sizeof(edge));
    if(edgeList == NULL) {
        printf("malloc error");
        return 0;
    }

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

    FILE *input = fopen(inputFilename, "r");
    if(!input){
        fprintf(stderr, "Unable to open file:  %s\n", "hmm");
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



    // UNION FIND SET UP
    int *parentRepList = (int*)malloc(n*sizeof(int));
    if(parentRepList == NULL) {
        printf("malloc error");
        return 0;
    }
    for(int i = 0; i < n; i++) {
        parentRepList[i] = i;
    }
    int *depthAtVertList = (int*)malloc(n*sizeof(int));
    if(depthAtVertList == NULL) {
        printf("malloc error");
        return 0;
    }
    for(int i = 0; i < n; i++) {
        depthAtVertList[i] = 0;
    }

    // RUN KRUSKAL
    edge* resultList = malloc((n-1)*sizeof(edge));

    // Sort edge list (length 2*m since including undirected edges)
    clock_t start = clock();
    #pragma omp parallel
    {
        mergeSort(edgeList, 0, (2*m));
    }
    clock_t end = clock();
    double timeTaken = (double)((end-start)/CLOCKS_PER_SEC);
    printf("Time taken %f", timeTaken);


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
    printf("RESULT\n");
    for(int j = 0; j < n-1; j++) {
        printf("%d , %d , %d\n", resultList[j].u, resultList[j].v, resultList[j].w);
    }


    return 0;
}