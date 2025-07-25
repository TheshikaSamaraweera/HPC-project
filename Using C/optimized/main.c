#include <stdlib.h>
#include <stdio.h>
#include "OMP_parallel.h"


int main(int argc, char *argv[]) {
    const char *filename = (argc > 1) ? argv[1] : "prompt.txt";
    int threads = (argc > 2) ? atoi(argv[2]) : 4;
    run_parallel_chatbot(filename, threads);
    return 0;
}

