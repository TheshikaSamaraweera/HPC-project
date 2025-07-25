#include <mpi.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "metrics.h"
#include "gemini_chatbot.h"

#define MAX_PROMPTS 1000
#define MAX_LINE_LEN 2048
#define MAX_RESPONSE_LEN 4096

int read_prompts(const char *filename, char prompts[][MAX_LINE_LEN]) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open prompt file");
        return -1;
    }

    int count = 0;
    while (fgets(prompts[count], MAX_LINE_LEN, file) && count < MAX_PROMPTS) {
        prompts[count][strcspn(prompts[count], "\n")] = 0;
        count++;
    }

    fclose(file);
    return count;
}

int main(int argc, char *argv[]) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    const char *filename = (argc > 1) ? argv[1] : "prompt.txt";
    int threads = (argc > 2) ? atoi(argv[2]) : 4;

    char (*all_prompts)[MAX_LINE_LEN] = NULL;
    char (*local_prompts)[MAX_LINE_LEN] = malloc(MAX_PROMPTS * MAX_LINE_LEN);
    char *flat_all_prompts = NULL;
    char *flat_local_prompts = NULL;

    int total_prompts = 0;
    int *counts = NULL, *displs = NULL;
    int *sendcounts = NULL, *senddispls = NULL;

    if (rank == 0) {
        all_prompts = malloc(sizeof(char[MAX_PROMPTS][MAX_LINE_LEN]));
        total_prompts = read_prompts(filename, all_prompts);
        if (total_prompts <= 0) {
            fprintf(stderr, "No prompts found.\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        // Flatten prompts
        flat_all_prompts = malloc(total_prompts * MAX_LINE_LEN);
        for (int i = 0; i < total_prompts; i++) {
            memcpy(&flat_all_prompts[i * MAX_LINE_LEN], all_prompts[i], MAX_LINE_LEN);
        }
    }

    MPI_Bcast(&total_prompts, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Calculate per-rank distribution
    int base = total_prompts / size;
    int remainder = total_prompts % size;
    int my_count = base + (rank < remainder ? 1 : 0);

    counts = malloc(size * sizeof(int));
    displs = malloc(size * sizeof(int));
    sendcounts = malloc(size * sizeof(int));
    senddispls = malloc(size * sizeof(int));

    int offset = 0;
    for (int i = 0; i < size; i++) {
        counts[i] = base + (i < remainder ? 1 : 0);
        displs[i] = offset;
        sendcounts[i] = counts[i] * MAX_LINE_LEN;
        senddispls[i] = offset * MAX_LINE_LEN;
        offset += counts[i];
    }

    flat_local_prompts = malloc(my_count * MAX_LINE_LEN);

    // Scatter flat buffer
    MPI_Scatterv(flat_all_prompts, sendcounts, senddispls, MPI_CHAR,
                 flat_local_prompts, my_count * MAX_LINE_LEN, MPI_CHAR,
                 0, MPI_COMM_WORLD);

    // Copy flat buffer to 2D array
    for (int i = 0; i < my_count; i++) {
        strncpy(local_prompts[i], &flat_local_prompts[i * MAX_LINE_LEN], MAX_LINE_LEN);
    }

    // OpenMP Parallel Section
    omp_set_num_threads(threads);
    static char responses[MAX_PROMPTS][MAX_RESPONSE_LEN];
    double times[MAX_PROMPTS] = {0};
    Metrics local_metrics = {0};

    double local_start = MPI_Wtime();

    #pragma omp parallel for schedule(dynamic, 1)
    for (int i = 0; i < my_count; i++) {
        struct timespec t_start;
        start_timer(&t_start);

        send_prompt(local_prompts[i], responses[i]);

        double elapsed = end_timer(t_start);
        times[i] = elapsed;

        #pragma omp critical
        {
            printf("Rank %d - Thread %d\nPrompt: %s\nGemini: %s\nTime: %.3f sec\n\n",
                   rank, omp_get_thread_num(), local_prompts[i], responses[i], elapsed);
        }

        #pragma omp atomic
        local_metrics.total_prompts++;

        #pragma omp atomic
        local_metrics.total_time += elapsed;
    }

    double local_end = MPI_Wtime();

    // Reduce metrics
    Metrics global_metrics = {0};
    MPI_Reduce(&local_metrics, &global_metrics, sizeof(Metrics) / sizeof(double),
               MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        print_summary(global_metrics);
        printf("Total MPI+OpenMP execution time: %.3f seconds\n", local_end - local_start);
    }

    // Cleanup
    free(counts);
    free(displs);
    free(sendcounts);
    free(senddispls);
    free(flat_local_prompts);
    if (rank == 0) {
        free(flat_all_prompts);
        free(all_prompts);
    }
    free(local_prompts);

    MPI_Finalize();
    return 0;
}
