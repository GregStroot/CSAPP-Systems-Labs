#include "cachelab.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <getopt.h>

long global_timer = 0;
long hit = 0;
long miss = 0;
long eviction = 0;

typedef struct {
    int valid; //valid bit 0 or 1
    long tag;
    int lru_stamp; //time stamp of last used
} Line;

void access_cache(Line** cache, int* E, uint64_t* setIdx, uint64_t* tag, bool* verbose){
    global_timer++;

    Line* current_set = cache[*setIdx];

    int emptyIdx = -1;
    int lruIdx = 0;
    int min_stamp = current_set[0].lru_stamp;

    for (int i=0; i < *E; i++) {
        // 1. Check for hit
        if (current_set[i].valid && current_set[i].tag == *tag) {
            //Cache hit
            if (*verbose) {
                printf("hit ");
            }
            hit++;
            //Update LRU of this one
            current_set[i].lru_stamp = global_timer ;
            return;
        }

        //2. Check for empty
        if (current_set[i].valid == 0 && emptyIdx == -1) {
            emptyIdx = i;
        }

        //3. Check for lowest lru
        if (current_set[i].lru_stamp < min_stamp){
            min_stamp = current_set[i].lru_stamp;
            lruIdx = i;
        }
    }

    //Cache miss
    if (emptyIdx != -1) {
        //No eviction needed
        current_set[emptyIdx].valid = 1;
        current_set[emptyIdx].tag = *tag;
        current_set[emptyIdx].lru_stamp = global_timer;

        if (*verbose) {
            printf("miss ");
        }
        miss++;
    }
    else {
        //Eviction needed
        current_set[lruIdx].lru_stamp = global_timer;
        current_set[lruIdx].tag = *tag;
        current_set[lruIdx].valid = 1;
        if (*verbose) {
            printf("miss eviction ");
        }
        miss++;
        eviction++;
    }


};



int main(int argc, char *argv[])
{
    if (argc < 4) {
        fprintf(stderr, "Usage: %s -s <s> -E <E> -b <b> -t <trace>\n", argv[0]);
        exit(1);
    }

    int opt;
    extern char *optarg;
    int s, E, b;
    bool verbose = false;
    char *trace_file = NULL;

    // Take in input args
    while ((opt = getopt(argc, argv, "vs:E:b:t:")) != -1) {
        switch (opt) {
            case 'v':
                verbose = true;
                break;
            case 's':
                s = atoi(optarg);
                break;
            case 'E':
                E = atoi(optarg);
                break;
            case 'b':
                b = atoi(optarg);
                break;
            case 't':
                trace_file = optarg;
                break;
        }
    }

    //S = 2^s
    uint64_t S = 1 << s;

    //First initalise it based on our size
    Line **cache = (Line **)malloc( S * sizeof(Line *));

    for (int i=0; i < S; i++) {
        cache[i] = (Line *)malloc(E * sizeof(Line));
        //Default init.
        for (int j=0; j < E; j++) {
            cache[i]->valid = 0;
            cache[i]->tag = 0;
            cache[i]->lru_stamp = 0;
        }
    }


    //Open trace for processing

    FILE *traceFile = fopen(trace_file, "r");

    //Safety
    if (traceFile == NULL) {
        fprintf(stderr, "Error: Could not open file %s \n", trace_file);
        exit(1);
    }

    // Trace arguments
    uint64_t setIdx;
    uint64_t tag;
    char operation;
    uint64_t address;
    int size;
    //Scaning
    while (fscanf(traceFile, " %c %lx,%d", &operation, &address, &size) > 0 ) {

        setIdx = (address >> b) & ( S-1 );
        tag = (address >> (b+s));
        //printf("%c %lx -> Set Index: %lu, Tag: %lu\n", operation, address, setIdx, tag);

        if (verbose) {
            printf("%c %lx,%lu ", operation, address, setIdx);
        }

        if (operation == 'I') {
            //We are only simulating d-cache and not i-cache
            continue;
        }
        else if (operation == 'S') {
            //Store
            access_cache(cache, &E, &setIdx, &tag, &verbose);
        }
        else if (operation == 'L') {
            //Load
            access_cache(cache, &E, &setIdx, &tag, &verbose);
        }
        else if (operation == 'M') {
            //E.G. Load then Store
            access_cache(cache, &E, &setIdx, &tag, &verbose);
            access_cache(cache, &E, &setIdx, &tag, &verbose);
        }

        if (verbose) {
            printf("\n");
        }
    }


    printSummary(hit, miss, eviction);
    return 0;
}

