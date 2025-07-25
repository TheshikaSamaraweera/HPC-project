#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include "metrics.h"
#include "gemini_chatbot.h"  

#define MAX_PROMPTS 1000
#define MAX_LINE_LEN 2048
#define MAX_RESPONSE_LEN 32768  

int read_prompts(const char *filename, char prompts[][MAX_LINE_LEN], int max_prompts) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open prompt file");
        return -1;
    }

    int count = 0;
    while (fgets(prompts[count], MAX_LINE_LEN, file) && count < max_prompts) {
        prompts[count][strcspn(prompts[count], "\n")] = 0; 
        count++;
    }

    fclose(file);
    return count;
}

void run_parallel_chatbot(const char *filename, int thread_count) {
    printf(" Parallel Gemini Chatbot using OpenMP\n");

    static char prompts[MAX_PROMPTS][MAX_LINE_LEN];
    static char responses[MAX_PROMPTS][MAX_RESPONSE_LEN];
    double times[MAX_PROMPTS] = {0};
    Metrics global_metrics = {0};

    int num_prompts = read_prompts(filename, prompts, MAX_PROMPTS);
    if (num_prompts <= 0) {
        fprintf(stderr, "No prompts to process.\n");
        return;
    }

    //set and show no of threads in i assign
    omp_set_num_threads(thread_count);
    printf("Using %d threads\n\n", thread_count);

    //measuring the start time of the parallel execution block
    double t0 = omp_get_wtime();

    //dynamic add threads runtime on demand. each thread run 2 loop itteration at a time
    #pragma omp parallel for schedule(dynamic, thread_count)
    for (int i = 0; i < num_prompts; i++) {
        struct timespec start_time;
        start_timer(&start_time);

        send_prompt(prompts[i], responses[i]);  

        double elapsed = end_timer(start_time);
        times[i] = elapsed;

        if (strlen(responses[i]) == 0) {
            snprintf(responses[i], MAX_RESPONSE_LEN, " No response or error.");
        }

        #pragma omp critical
        {
            printf(" Thread %d -> Prompt: %s\n", omp_get_thread_num(), prompts[i]);
            printf(" Gemini: %s\n", responses[i]);
            printf(" Time: %.3f seconds\n\n", elapsed);
        }

        // Update global metrics in a safe thread-safe manner
        #pragma omp atomic
        global_metrics.total_prompts++;

        #pragma omp atomic
        global_metrics.total_time += elapsed;
    }

    double t1 = omp_get_wtime();

    print_summary(global_metrics);
    printf(" Total parallel execution time: %.3f seconds\n", t1 - t0);
}
