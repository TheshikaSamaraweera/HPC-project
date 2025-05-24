#include <stdio.h>
#include <omp.h>
#include "metrics.h"

// Forward declaration from chatbot file
void chat_request(const char *user_input);

void run_parallel_chatbot() {
    Metrics metrics = {0};

    // Sample prompts
    const char *prompts[] = {
        "What is capital of sri lanka?",
        "answer of 5+8+4-5/8*4",
        "how many zeros for one milion",
        "what is the capital of France?",
        "who is the foundr of computer?",
    };
    int num_prompts = sizeof(prompts) / sizeof(prompts[0]);

    printf("🤖 Parallel C Chatbot using OpenMP\n");
    printf("🧵 Launching %d parallel threads...\n\n", num_prompts);

    double start_time = omp_get_wtime();

    #pragma omp parallel for
    for (int i = 0; i < num_prompts; i++) {
        struct timespec local_start;
        start_timer(&local_start);

        printf("🧵 Thread %d handling prompt %d\n", omp_get_thread_num(), i);
        chat_request(prompts[i]);

        double elapsed = end_timer(local_start);
        #pragma omp critical
        {
            update_metrics(&metrics, elapsed);
            printf("⏱️ Prompt %d Time: %.3f seconds (Thread %d)\n", i, elapsed, omp_get_thread_num());
        }
    }

    double total_elapsed = omp_get_wtime() - start_time;
    metrics.total_time = total_elapsed;
    metrics.total_prompts = num_prompts;

    print_summary(metrics);
}
