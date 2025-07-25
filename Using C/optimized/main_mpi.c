#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "metrics.h"
#include "gemini_chatbot.h"
#include "MPI_chatbot.h"

#define MAX_PROMPTS 100
#define MAX_LINE_LEN 2048
#define MAX_RESPONSE_LEN 4096

typedef struct {
    int index;
    double elapsed;
    char response[MAX_RESPONSE_LEN];
} Response;

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
    char prompts[MAX_PROMPTS][MAX_LINE_LEN];
    Response responses[MAX_PROMPTS] = {0};

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    double global_start = MPI_Wtime();

    if (rank == 0) {
        const char *filename = (argc > 1) ? argv[1] : "prompt.txt";
        int num_prompts = read_prompts(filename, prompts);
        if (num_prompts <= 0) {
            fprintf(stderr, "No prompts found.\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        printf("Master: Distributing %d prompts to %d workers...\n", num_prompts, size - 1);

        int sent = 0, received = 0;
        MPI_Status status;

        // Initial distribution
        for (int i = 1; i < size && sent < num_prompts; i++) {
            int len = strlen(prompts[sent]) + 1;
            MPI_Send(&sent, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            MPI_Send(&len, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            MPI_Send(prompts[sent], len, MPI_CHAR, i, 0, MPI_COMM_WORLD);
            sent++;
        }

        while (received < num_prompts) {
            Response temp;
            MPI_Recv(&temp, sizeof(Response), MPI_BYTE, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
            responses[temp.index] = temp;
            received++;

            if (sent < num_prompts) {
                int len = strlen(prompts[sent]) + 1;
                MPI_Send(&sent, 1, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
                MPI_Send(&len, 1, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
                MPI_Send(prompts[sent], len, MPI_CHAR, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
                sent++;
            } else {
                int stop = -1;
                MPI_Send(&stop, 1, MPI_INT, status.MPI_SOURCE, 1, MPI_COMM_WORLD);
            }
        }

        // Summary
        Metrics metrics = {0};
        for (int i = 0; i < num_prompts; i++) {
            printf("Prompt %d: %s\n", i + 1, prompts[i]);
            printf("Bot: %s\n", responses[i].response);
            printf("Time: %.3f seconds\n\n", responses[i].elapsed);
            metrics.total_prompts++;
            metrics.total_time += responses[i].elapsed;
        }

        print_summary(metrics);
        printf("Total MPI wall time: %.3f seconds\n", MPI_Wtime() - global_start);
    } else {
        run_mpi_worker();
    }

    MPI_Finalize();
    return 0;
}
