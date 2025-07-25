#include "metrics.h"
#include <stdio.h>
#include <time.h>

void start_timer(struct timespec *start) {
    clock_gettime(CLOCK_MONOTONIC, start);
}

double end_timer(struct timespec start) {
    struct timespec end;
    clock_gettime(CLOCK_MONOTONIC, &end);
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}

void update_metrics(Metrics *metrics, double elapsed_time) {
    metrics->total_prompts++;
    metrics->total_time += elapsed_time;
}

void merge_metrics(Metrics *global, const Metrics *local) {
    global->total_prompts += local->total_prompts;
    global->total_time += local->total_time;
}

void print_summary(Metrics metrics) {
    printf("\nChatbot Performance Summary\n");
    printf("Total Prompts: %d\n", metrics.total_prompts);
    printf("Total Time: %.3f seconds\n", metrics.total_time);
    if (metrics.total_prompts > 0)
        printf("Average Time per Prompt: %.3f seconds\n\n", metrics.total_time / metrics.total_prompts);
}
