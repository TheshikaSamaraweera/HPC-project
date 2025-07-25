#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "metrics.h"
#include "gemini_chatbot.h"  

#define MAX_PROMPTS 50
#define MAX_LINE_LEN 512
#define MAX_RESPONSE_LEN 4096  

int main() {

    //read  prompt file using file operation 
    FILE *file = fopen("prompt.txt", "r");
    if (!file) {
        perror("Failed to open prompt.txt");
        return 1;
    }

    char line[MAX_LINE_LEN];
    int count = 0;

    Metrics metrics = {0};  

    while (fgets(line, sizeof(line), file) && count < MAX_PROMPTS) {
        line[strcspn(line, "\n")] = 0;  

        printf(" Prompt #%d: %s\n", count + 1, line);

        struct timespec start_time;
        start_timer(&start_time);

        char response[MAX_RESPONSE_LEN];
        send_prompt(line, response);  

        double elapsed = end_timer(start_time);

        printf("Gemini: %s\n", response);
        printf(" Time: %.3f seconds\n\n", elapsed);

        update_metrics(&metrics, elapsed);
        count++;
    }

    fclose(file);

    print_summary(metrics);
    return 0;
}
