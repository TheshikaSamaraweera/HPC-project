// --------------------- opt_hybrid.c ---------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <mpi.h>
#include "metrics.h"

#define NUM_PROMPTS 16
#define MAX_LEN 1024
#define MAX_RESPONSE 8192

char* read_prompt(int index) {
    static char prompts[NUM_PROMPTS][MAX_LEN];
    static int loaded = 0;

    if (!loaded) {
        FILE* fp = fopen("prompts.txt", "r");
        if (!fp) {
            perror("prompts.txt not found");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        for (int i = 0; i < NUM_PROMPTS; i++) {
            fgets(prompts[i], MAX_LEN, fp);
            prompts[i][strcspn(prompts[i], "\n")] = '\0';
        }
        fclose(fp);
        loaded = 1;
    }
    return prompts[index];
}

int main(int argc, char* argv[]) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    double total_start = MPI_Wtime();

    int prompts_per_rank = (NUM_PROMPTS + size - 1) / size;
    int start = rank * prompts_per_rank;
    int end = (start + prompts_per_rank > NUM_PROMPTS) ? NUM_PROMPTS : start + prompts_per_rank;

    double local_times[NUM_PROMPTS] = {0};
    char responses[NUM_PROMPTS][MAX_RESPONSE] = {0};

    #pragma omp parallel for
    for (int i = start; i < end; i++) {
        double t1 = omp_get_wtime();

        char command[2048];
        snprintf(command, sizeof(command), "python3 chatbot.py \"%s\"", read_prompt(i));
        FILE* fp = popen(command, "r");
        if (!fp) {
            perror("popen failed");
            continue;
        }

        size_t offset = 0;
        char buffer[512];
        while (fgets(buffer, sizeof(buffer), fp)) {
            size_t len = strlen(buffer);
            if (offset + len < MAX_RESPONSE - 1) {
                memcpy(responses[i] + offset, buffer, len);
                offset += len;
            }
        }
        responses[i][offset] = '\0';
        pclose(fp);

        double t2 = omp_get_wtime();
        local_times[i] = t2 - t1;
    }

    double total_end = MPI_Wtime();

    double global_times[NUM_PROMPTS];
    MPI_Reduce(local_times, global_times, NUM_PROMPTS, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    int local_lens[NUM_PROMPTS];
    for (int i = 0; i < NUM_PROMPTS; i++) {
        local_lens[i] = strlen(responses[i]);
    }

    int global_lens[NUM_PROMPTS] = {0};
    MPI_Reduce(local_lens, global_lens, NUM_PROMPTS, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    int total_len = 0, displs[NUM_PROMPTS] = {0};
    char* all_responses = NULL;

    if (rank == 0) {
        for (int i = 0; i < NUM_PROMPTS; i++) total_len += global_lens[i];
        all_responses = malloc(total_len + 1);
    }

    char local_contig[NUM_PROMPTS * MAX_RESPONSE] = {0};
    int pos = 0;
    for (int i = start; i < end; i++) {
        int len = local_lens[i];
        memcpy(local_contig + pos, responses[i], len);
        pos += len;
    }

    int local_total_len = pos;
    int* recvcounts = NULL;
    int* recvdispls = NULL;
    if (rank == 0) {
        recvcounts = malloc(size * sizeof(int));
        recvdispls = malloc(size * sizeof(int));
    }

    MPI_Gather(&local_total_len, 1, MPI_INT, recvcounts, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        recvdispls[0] = 0;
        for (int i = 1; i < size; i++) {
            recvdispls[i] = recvdispls[i - 1] + recvcounts[i - 1];
        }
    }

    MPI_Gatherv(local_contig, local_total_len, MPI_CHAR,
                all_responses, recvcounts, recvdispls, MPI_CHAR,
                0, MPI_COMM_WORLD);

    if (rank == 0) {
        int offset = 0;
        for (int i = 0; i < NUM_PROMPTS; i++) {
            printf("\nPrompt %2d: %s\nResponse:\n%.*s\n", i, read_prompt(i), global_lens[i], all_responses + offset);
            offset += global_lens[i];
        }
        print_metrics(global_times, NUM_PROMPTS, total_end - total_start, "MPI + OpenMP");
        free(all_responses);
        free(recvcounts);
        free(recvdispls);
    }

    MPI_Finalize();
    return 0;
}
