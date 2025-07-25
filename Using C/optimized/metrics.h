#ifndef METRICS_H
#define METRICS_H

#include <time.h>

typedef struct {
    int total_prompts;
    double total_time;
} Metrics;

void start_timer(struct timespec *start);
double end_timer(struct timespec start);
void update_metrics(Metrics *metrics, double elapsed_time);
void merge_metrics(Metrics *global, const Metrics *local);
void print_summary(Metrics metrics);

#endif
