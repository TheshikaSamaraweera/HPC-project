#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
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
            exit(1);
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

int main() {
    double times[NUM_PROMPTS];
    char *responses[NUM_PROMPTS];

    // Allocate buffer per prompt for responses
    for (int i = 0; i < NUM_PROMPTS; i++) {
        responses[i] = (char*)calloc(MAX_RESPONSE, sizeof(char));
        if (!responses[i]) {
            perror("Memory allocation failed");
            exit(1);
        }
    }

    double start = omp_get_wtime();

    #pragma omp parallel for
    for (int i = 0; i < NUM_PROMPTS; i++) {
        double t1 = omp_get_wtime();

        char command[2048];
        snprintf(command, sizeof(command), "python3 chatbot.py \"%s\"", read_prompt(i));
        FILE* fp = popen(command, "r");
        if (!fp) {
            perror("popen failed");
            continue;
        }

        char buffer[512];
        size_t offset = 0;
        while (fgets(buffer, sizeof(buffer), fp)) {
            size_t len = strlen(buffer);
            if (offset + len < MAX_RESPONSE - 1) {
                strcpy(responses[i] + offset, buffer);
                offset += len;
            }
        }
        pclose(fp);

        double t2 = omp_get_wtime();
        times[i] = t2 - t1;
    }

    double end = omp_get_wtime();

    // Print results IN ORDER
    for (int i = 0; i < NUM_PROMPTS; i++) {
        printf("\nPrompt %2d: %s\nResponse:\n%s\n", i, read_prompt(i), responses[i]);
    }

    print_metrics(times, NUM_PROMPTS, end - start, "OpenMP");

    // Free buffers
    for (int i = 0; i < NUM_PROMPTS; i++) {
        free(responses[i]);
    }

    return 0;
}
