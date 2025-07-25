#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>

#define MAX_RESPONSE_LEN 4096
#define API_KEY "" 
#define GEMINI_URL "https://generativelanguage.googleapis.com/v1beta/models/gemini-2.0-flash:generateContent?key=" API_KEY

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
    if (s->ptr) {
        memcpy(s->ptr + s->len, ptr, size * nmemb);
        s->ptr[new_len] = '\0';
        s->len = new_len;
    }
    return size * nmemb;
}

void send_prompt(const char *prompt_text, char *response_out) {
    CURL *curl = curl_easy_init();
    if (!curl) {
        snprintf(response_out, MAX_RESPONSE_LEN, "CURL init failed.");
        return;
    }

    struct string response;
    init_string(&response);

    char json_data[8192];
    snprintf(json_data, sizeof(json_data),
        "{"
        "  \"contents\": ["
        "    {\"parts\": ["
        "      {\"text\": \"%s\"}"
        "    ]}"
        "  ]"
        "}", prompt_text);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, GEMINI_URL);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        snprintf(response_out, MAX_RESPONSE_LEN, "CURL error: %s", curl_easy_strerror(res));
        goto cleanup;
    }

    cJSON *json = cJSON_Parse(response.ptr);
    if (!json) {
        snprintf(response_out, MAX_RESPONSE_LEN, "Invalid JSON response.");
        goto cleanup;
    }

    // Handle API error
    cJSON *error = cJSON_GetObjectItem(json, "error");
    if (error) {
        cJSON *msg = cJSON_GetObjectItem(error, "message");
        if (msg && cJSON_IsString(msg)) {
            snprintf(response_out, MAX_RESPONSE_LEN, "API Error: %s", msg->valuestring);
        } else {
            snprintf(response_out, MAX_RESPONSE_LEN, "API returned an error.");
        }
        cJSON_Delete(json);
        goto cleanup;
    }

    
    cJSON *candidates = cJSON_GetObjectItem(json, "candidates");
    if (cJSON_IsArray(candidates)) {
        cJSON *first_candidate = cJSON_GetArrayItem(candidates, 0);
        if (first_candidate) {
            cJSON *content = cJSON_GetObjectItem(first_candidate, "content");
            if (content) {
                cJSON *parts = cJSON_GetObjectItem(content, "parts");
                if (cJSON_IsArray(parts)) {
                    cJSON *first_part = cJSON_GetArrayItem(parts, 0);
                    if (first_part) {
                        cJSON *text = cJSON_GetObjectItem(first_part, "text");
                        if (text && cJSON_IsString(text)) {
                            snprintf(response_out, MAX_RESPONSE_LEN, "%s", text->valuestring);
                            cJSON_Delete(json);
                            goto cleanup;
                        }
                    }
                }
            }
        }
    }

    snprintf(response_out, MAX_RESPONSE_LEN, "Unexpected response format.");
    cJSON_Delete(json);

cleanup:
    free(response.ptr);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
}
