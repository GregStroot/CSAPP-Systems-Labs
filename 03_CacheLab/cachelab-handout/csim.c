#include "cachelab.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <getopt.h>

typedef struct {
    int valid; //valid bit 0 or 1
    long tag;
    int lru_stamp; //time stamp of last used
} Line;



int main(int argc, char *argv[])
{
    if (argc < 4) {
        fprintf(stderr, "Usage: %s -s <s> -E <E> -b <b> -t <trace>\n", argv[0]);
        exit(1);
    }

    int opt;
    extern char *optarg;
    int s, E, b;
    char *trace_file = NULL;

    // Take in input args
    // TODO: We need to deal with -v argument
    while ((opt = getopt(argc, argv, "s:E:b:t:")) != -1) {
        switch (opt) {
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
    int S = 1 << s;

    //First initalise it based on our size
    Line **cache = (Line **)malloc( S * sizeof(Line *));

    for (int i=0; i < S; i++) {
        cache[i] = (Line *)malloc(E * sizeof(Line));
    }


    //Open trace for processing

    FILE *traceFile = fopen(trace_file, "r");

    //Safety
    if (traceFile == NULL) {
        fprintf(stderr, "Error: Could not open file %s \n", trace_file);
        exit(1);
    }

    //Result args
    int miss = 0;
    int hit = 0;
    int eviction = 0;

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
        printf("%c %lx -> Set Index: %lu, Tag: %lu\n", operation, address, setIdx, tag);

        if (operation == 'I') {
            //We are only simulating d-cache and not i-cache
            continue;
        }
        else if (operation == 'S') {
            //Store
            //access_cache();
            printf("S\n");
        }
        else if (operation == 'L') {
            //Load
            //access_cache();
            printf("L\n");
        }
        else if (operation == 'M') {
            //E.G. Load then Store
            //access_cache();
            //access_cache();
            printf("M\n");
        }
    }


    printSummary(miss, hit, eviction);
    return 0;
}

