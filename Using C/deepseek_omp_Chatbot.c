#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>
#include "metrics.h"

#define MAX_INPUT 1024
#define API_KEY ""
#define API_URL "https://openrouter.ai/api/v1/chat/completions"


struct string {
    char *ptr;
    size_t len;
};

void init_string(struct string *s) {
    s->len = 0;
    s->ptr = malloc(1);
    if (s->ptr) s->ptr[0] = '\0';
}

size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s) {
    size_t new_len = s->len + size * nmemb;
    s->ptr = realloc(s->ptr, new_len + 1);
    if (s->ptr == NULL) return 0;
    memcpy(s->ptr + s->len, ptr, size * nmemb);
    s->ptr[new_len] = '\0';
    s->len = new_len;
    return size * nmemb;
}

void chat_request(const char *user_input) {
    CURL *curl;
    CURLcode res;
    struct string response;
    init_string(&response);

    const char *json_format = "{"
        "\"model\": \"deepseek/deepseek-r1:free\","
        "\"messages\": ["
        "  {\"role\": \"system\", \"content\": \"You are SmartBot, a fast and intelligent AI assistant. Always give accurate, short, and well-structured answers.\"},"
        "  {\"role\": \"user\", \"content\": \"%s\"}"
        "]"
    "}";

    char json_data[4096];
    snprintf(json_data, sizeof(json_data), json_format, user_input);

    curl = curl_easy_init();
    if (curl) {
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        char auth_header[256];
        snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", API_KEY);
        headers = curl_slist_append(headers, auth_header);
        headers = curl_slist_append(headers, "HTTP-Referer: http://localhost");
        headers = curl_slist_append(headers, "X-Title: Terminal Chatbot");

        curl_easy_setopt(curl, CURLOPT_URL, API_URL);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        } else {
            // Parse response JSON and extract message content
            cJSON *root = cJSON_Parse(response.ptr);
            if (root) {
                cJSON *choices = cJSON_GetObjectItem(root, "choices");
                if (cJSON_IsArray(choices)) {
                    cJSON *first_choice = cJSON_GetArrayItem(choices, 0);
                    cJSON *message = cJSON_GetObjectItem(first_choice, "message");
                    cJSON *content = cJSON_GetObjectItem(message, "content");

                    if (content && cJSON_IsString(content)) {
                        printf("\n🤖 Bot: %s\n\n", content->valuestring);
                    } else {
                        printf("Bot response not found.\n");
                    }
                }
                cJSON_Delete(root);
            } else {
                printf("Failed to parse JSON response.\n");
            }
        }

        free(response.ptr);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
}

int main() {
    run_parallel_chatbot();
    return 0;
}