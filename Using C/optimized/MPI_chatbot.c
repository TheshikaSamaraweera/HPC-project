#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include "gemini_chatbot.h"
#include "metrics.h"

#define MAX_LINE_LEN 2048
#define MAX_RESPONSE_LEN 4096

typedef struct {
    int index;
    double elapsed;
    char response[MAX_RESPONSE_LEN];
} Response;

void run_mpi_worker() {
    MPI_Status status;

    while (1) {
        int prompt_index;
        MPI_Recv(&prompt_index, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        if (status.MPI_TAG == 1 || prompt_index == -1) break;

        int prompt_len;
        MPI_Recv(&prompt_len, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        char prompt[MAX_LINE_LEN];
        MPI_Recv(prompt, prompt_len, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        Response r = {.index = prompt_index};
        struct timespec start;
        start_timer(&start);

        send_prompt(prompt, r.response);

        r.elapsed = end_timer(start);

        if (strlen(r.response) == 0) {
            snprintf(r.response, MAX_RESPONSE_LEN, "No response or error.");
        }

        MPI_Send(&r, sizeof(Response), MPI_BYTE, 0, 0, MPI_COMM_WORLD);
    }
}
