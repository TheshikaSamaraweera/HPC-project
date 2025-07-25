#include <stdio.h>
#include <string.h>
#include "metrics.h"

void print_metrics(double times[], int count, double total_time, const char* label) {
    double avg = 0;
    for (int i = 0; i < count; i++) avg += times[i];
    avg /= count;

    printf("\n========== PERFORMANCE REPORT (%s) ==========\n", label);
    printf("Total time: %.2f sec\n", total_time);
    printf("Average time per prompt: %.2f sec\n", avg);
    for (int i = 0; i < count; i++) {
        printf("Prompt %2d: %.2f sec\n", i, times[i]);
    }

    // CSV logging
    FILE *f = fopen("results.csv", "a");
    if (f == NULL) {
        perror("Failed to open results.csv");
        return;
    }

    for (int i = 0; i < count; i++) {
        fprintf(f, "%s,%d,%.4f,%.4f,%.4f\n", label, i, times[i], total_time, avg);
    }
    fclose(f);
}
