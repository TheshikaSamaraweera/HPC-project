#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
    double prompt_time = 0;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    char response[MAX_RESPONSE] = {0};
    int resp_len = 0;

    double total_start = MPI_Wtime();

    if (rank < NUM_PROMPTS) {
        double t1 = MPI_Wtime();

        char command[2048];
        snprintf(command, sizeof(command), "python3 chatbot.py \"%s\"", read_prompt(rank));
        FILE* fp = popen(command, "r");
        if (!fp) {
            perror("popen failed");
        } else {
            size_t offset = 0;
            char buffer[512];
            while (fgets(buffer, sizeof(buffer), fp)) {
                size_t len = strlen(buffer);
                if (offset + len < MAX_RESPONSE - 1) {
                    memcpy(response + offset, buffer, len);
                    offset += len;
                }
            }
            response[offset] = '\0';
            resp_len = (int)offset;
            pclose(fp);
        }

        double t2 = MPI_Wtime();
        prompt_time = t2 - t1;
    }

    // Gather times
    double all_times[NUM_PROMPTS] = {0};
    MPI_Gather(&prompt_time, 1, MPI_DOUBLE, all_times, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Gather lengths of responses
    int all_lens[NUM_PROMPTS] = {0};
    MPI_Gather(&resp_len, 1, MPI_INT, all_lens, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Prepare buffer for all responses on root
    char *all_responses = NULL;
    int displs[NUM_PROMPTS] = {0};
    int total_len = 0;

    if (rank == 0) {
        for (int i = 0; i < NUM_PROMPTS; i++) total_len += all_lens[i];
        all_responses = malloc(total_len + 1);
        if (!all_responses) {
            fprintf(stderr, "Memory allocation failed on root\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }

    // Calculate displacements
    if (rank == 0) {
        displs[0] = 0;
        for (int i = 1; i < NUM_PROMPTS; i++) {
            displs[i] = displs[i-1] + all_lens[i-1];
        }
    }

    // Gather all response strings
    MPI_Gatherv(response, resp_len, MPI_CHAR,
                all_responses, all_lens, displs, MPI_CHAR,
                0, MPI_COMM_WORLD);

    double total_end = MPI_Wtime();

    if (rank == 0) {
        // Print in order
        int offset = 0;
        for (int i = 0; i < NUM_PROMPTS; i++) {
            printf("\nPrompt %2d: %s\nResponse:\n%.*s\n", i, read_prompt(i), all_lens[i], all_responses + offset);
            offset += all_lens[i];
        }

        print_metrics(all_times, NUM_PROMPTS, total_end - total_start, "MPI");
        free(all_responses);
    }

    MPI_Finalize();
    return 0;
}
