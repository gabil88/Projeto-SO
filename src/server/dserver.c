#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <glib.h>

#include "../../include/server/document_manager.h"
#include "../../include/server/parsing.h"
#include "../../include/server/cache.h"
#include "../../include/server/querie_handler.h"

#define FIFO_PATH "/tmp/server_fifo"
#define INTERNAL_FIFO_PATH "/tmp/internal_fifo"
#define MAX_ITEMS_IN_MEMORY 10

typedef struct {
    char request[256];
    char pipe[128];
} Request;

// Function to send a message to the client
void send_message(const char* pipe_name, const char* message) {
    int reply_fd = open(pipe_name, O_WRONLY);
    if (reply_fd != -1) {
        ssize_t bytes_written = write(reply_fd, message, strlen(message));
        if (bytes_written == -1) {
            perror("Error writing to reply FIFO");
        } else {
            printf("Sent message: %s\n", message);
        }
        close(reply_fd);
    } else {
        perror("Error opening reply FIFO");
    }
}

// Function used for debugging purposes
void print_cache_state(Cache* cache) {
    printf("Cache state:\n");
    for (int i = 0; i < CACHE_SIZE; i++) {
        printf("Slot %d: %s\n", i, cache->items[i].doc ? cache->items[i].doc->title : "NULL");
    }
    fflush(stdout);
}

// Special parsing
char** special_parsing(char* request) {
    char** result = malloc(2 * sizeof(char*));
    if (!result) {
        perror("Memory allocation failed");
        return NULL;
    }

    char* token = strtok(request, "/");

    // Get subsequent tokens
    result[0] = token ? strdup(strtok(NULL, "/")) : NULL;  // Second token
    result[1] = token ? strdup(strtok(NULL, "/")) : NULL;  // Third token

    if (!result[0] || !result[1]) {
        free(result[0]);
        free(result[1]);
        free(result);
        return NULL;
    }

    return result;
}

void send_internal_request_to_server(const char* request) {
    Request req = {0};
    snprintf(req.request, sizeof(req.request), "%s", request);
    snprintf(req.pipe, sizeof(req.pipe), "/tmp/server_response");  // Example default pipe

    int req_fd = open(INTERNAL_FIFO_PATH, O_WRONLY);
    if (req_fd != -1) {
        ssize_t bytes_written = write(req_fd, &req, sizeof(Request));
        if (bytes_written == -1) {
            perror("Error writing to request FIFO");
        } else if (bytes_written != sizeof(Request)) {
            printf("Partial write to FIFO\n");
        }
        close(req_fd);
    } else {
        perror("Error opening FIFO for request");
    }
}

// Function to initialize FIFOs and return file descriptors
void initialize_fifos(int* fifo_fd, int* internal_fifo_fd) {
    // Remove old FIFOs
    unlink(FIFO_PATH);
    unlink(INTERNAL_FIFO_PATH);

    // Create main FIFO
    if (mkfifo(FIFO_PATH, 0666) == -1) {
        perror("Error creating FIFO");
        exit(-1);
    }
    *fifo_fd = open(FIFO_PATH, O_RDONLY);
    if (*fifo_fd == -1) {
        perror("Error opening FIFO");
        exit(-1);
    }
    // Create internal FIFO
    if (mkfifo(INTERNAL_FIFO_PATH, 0666) == -1) {
        perror("Error creating internal FIFO");
        close(*fifo_fd);
        unlink(FIFO_PATH);
        exit(-1);
    }
    *internal_fifo_fd = open(INTERNAL_FIFO_PATH, O_RDONLY | O_NONBLOCK);
    if (*internal_fifo_fd == -1) {
        perror("Error opening internal FIFO");
        close(*fifo_fd);
        unlink(FIFO_PATH);
        exit(-1);
    }
}

// Function to handle parent types (type 0, 1, 3, 6, 7, 8)
void handle_parent_types(int type, Request* req, Cache* cache, GArray* deleted_keys, int* max_key) {
    char* request = req->request;
    char* pipe_name = req->pipe;
    int status = -1;
    pid_t pid;
    Document* doc = malloc(sizeof(Document));
    if (doc == NULL) {
        perror("Error allocating memory for document");
        send_message(pipe_name, "Server error: Unable to allocate memory.");
        return;
    }
    initialize_document(doc);
    parsing(request, doc);
    switch (type) {
        case 0:
            g_array_free(deleted_keys, TRUE);
            cache_flush_all_dirty(cache);
            free(cache);
            send_message(pipe_name, "Server shutting down.");
            exit(0);
            break;
        case 1: // Add document
            if (deleted_keys->len > 0) {
                doc->key = g_array_index(deleted_keys, int, deleted_keys->len - 1);
                g_array_remove_index(deleted_keys, deleted_keys->len - 1);
            } else {
                doc->key = *max_key + 1;
            }
            status = cache_add(cache, doc, 0);
            if (status == 0) {
                status = add_document(doc);
            }
            if (status == 0) {
                char message[256];
                snprintf(message, sizeof(message), "Document added successfully with key: %d", doc->key);
                send_message(pipe_name, message);
                (*max_key)++;
            } else if (status == 2) {
                send_message(pipe_name, "Document already exists in cache.");
            } else {
                send_message(pipe_name, "Unexpected Error adding document.");
            }
            break;
        case 3: // Remove document
            status = cache_remove(cache, doc->key);
            print_cache_state(cache);
            if (status == 0) {
                send_message(pipe_name, "Document removed successfully from disk.");
                g_array_append_val(deleted_keys, doc->key);
            } else if (status == 1) {
                send_message(pipe_name, "Document removed from disk and cache.");
            } else {
                send_message(pipe_name, "Document not found in cache or disk.");
            }
            break;
        case 6: // Wait for child
            pid = atoi(request + 3);
            if (pid > 0) {
                int status;
                int ret = waitpid(pid, &status, 0);
                if (ret == -1) {
                    perror("waitpid failed");
                }
            }
            break;
        case 7: // Add from disk to cache
        {
            int key_to_add = atoi(request + 4);
            doc = consult_document(key_to_add);
            status = cache_add(cache, doc, 1);
            break;
        }
        case 8: // Update time in cache
        {
            int key_to_update = atoi(request + 4);
            cache_update_time(cache, key_to_update);
            break;
        }
    }
    free(doc);
}

// Function to handle child types (type 2, 4, 5)
void handle_child_types(int type, Request* req, Cache* cache, int max_key) {
    char* request = req->request;
    char* pipe_name = req->pipe;
    Document* doc = malloc(sizeof(Document));
    if (doc == NULL) {
        perror("Error allocating memory for document");
        send_message(pipe_name, "Server error: Unable to allocate memory.");
        exit(1);
    }
    initialize_document(doc);
    parsing(request, doc);
    pid_t child_pid = fork();
    if (child_pid < 0) {
        perror("Error during fork");
        free(doc);
        return;
    }
    if (child_pid == 0) {
        // Child process
        pid_t grandchild_pid = fork();
        if (grandchild_pid < 0) {
            perror("Error during grandchild fork");
            free(doc);
            exit(1);
        }
        if (grandchild_pid == 0) {
            // Grandchild process
            switch (type) {
                case 2: {
                    Document* found_doc = cache_get(cache, doc->key);
                    if (found_doc != NULL) {
                        char message[256];
                        snprintf(message, sizeof(message), "Document found in cache with Key: %d and Title: %s", found_doc->key, found_doc->title);
                        send_message(pipe_name, message);
                        char updateCache[256];
                        snprintf(updateCache, sizeof(updateCache), "-uc/%d", found_doc->key);
                        send_internal_request_to_server(updateCache);
                    } else {
                        found_doc = consult_document(doc->key);
                        if (found_doc != NULL) {
                            char message[256];
                            snprintf(message, sizeof(message), "Document found in storage with Key: %d and Title: %s", found_doc->key, found_doc->title);
                            send_message(pipe_name, message);
                            char addCache[256];
                            snprintf(addCache, sizeof(addCache), "-ac/%d", found_doc->key);
                            send_internal_request_to_server(addCache);
                            free(found_doc);
                        } else {
                            send_message(pipe_name, "Document not found.");
                        }
                    }
                    break;
                }
                case 4: {
                    char** extracted = special_parsing(request);
                    if (!extracted) {
                        send_message(pipe_name, "Invalid request format.");
                        free(doc);
                        exit(1);
                    }
                    int key = atoi(extracted[0]);
                    Document* extracted_doc = cache_get(cache, key);
                    if (extracted_doc != NULL) {
                        char updateCache[256];
                        snprintf(updateCache, sizeof(updateCache), "-uc/%d", key);
                        send_internal_request_to_server(updateCache);
                        int count = get_number_of_lines_with_keyword(extracted_doc, extracted[1]);
                        if (count == -1) {
                            send_message(pipe_name, "Error reading file.");
                        } else {
                            char message[256];
                            snprintf(message, sizeof(message), "Number of lines with keyword '%s': %d", extracted[1], count);
                            send_message(pipe_name, message);
                        }
                    } else {
                        extracted_doc = consult_document(key);
                        if (extracted_doc == NULL) {
                            send_message(pipe_name, "Document not found.");
                        } else {
                            char addCache[256];
                            snprintf(addCache, sizeof(addCache), "-ac/%d", key);
                            send_internal_request_to_server(addCache);
                            int count = get_number_of_lines_with_keyword(extracted_doc, extracted[1]);
                            if (count == -1) {
                                send_message(pipe_name, "Error reading file.");
                            } else {
                                char message[256];
                                snprintf(message, sizeof(message), "Number of lines with keyword '%s': %d", extracted[1], count);
                                send_message(pipe_name, message);
                            }
                            free(extracted_doc);
                        }
                    }
                    free(extracted[0]);
                    free(extracted[1]);
                    free(extracted);
                    break;
                }
                case 5: {
                    char** extracted = special_parsing(request);
                    int nr_process = atoi(extracted[1]);
                    int count = get_number_of_docs_with_keyword(extracted[0], nr_process, max_key);
                    if (count == -1) {
                        send_message(pipe_name, "Error reading file.");
                    } else {
                        char message[256];
                        snprintf(message, sizeof(message), "Number of documents with keyword '%s': %d", extracted[0], count);
                        send_message(pipe_name, message);
                    }
                    free(extracted[0]);
                    free(extracted[1]);
                    free(extracted);
                    break;
                }
            }
            free(doc);
            exit(0);
        } else {
            // Child process waits for grandchild
            int status;
            waitpid(grandchild_pid, &status, 0);
            char notify[256];
            snprintf(notify, sizeof(notify), "-w/%d", getpid());
            Request notify_req = {0};
            snprintf(notify_req.request, sizeof(notify_req.request), "%s", notify);
            snprintf(notify_req.pipe, sizeof(notify_req.pipe), "%s", pipe_name);
            int notify_fd = open(FIFO_PATH, O_WRONLY);
            if (notify_fd != -1) {
                write(notify_fd, &notify_req, sizeof(Request));
                close(notify_fd);
            }
            free(doc);
            exit(0);
        }
    }
}

// Main server function
int main() {
    int fifo_fd, internal_fifo_fd;
    initialize_fifos(&fifo_fd, &internal_fifo_fd);
    Cache* cache = cache_init();
    GArray* deleted_keys = g_array_new(FALSE, FALSE, sizeof(int));
    int max_key = load_deleted_keys(deleted_keys);
    printf("Server started. Waiting for requests...\n");
    while (1) {
        Request req;
        ssize_t bytes_read = read(internal_fifo_fd, &req, sizeof(Request));
        if (bytes_read != sizeof(Request)) {
            bytes_read = read(fifo_fd, &req, sizeof(Request));
        }
        if (bytes_read == sizeof(Request)) {
            int type = parsing(req.request, NULL);
            if (type == 0 || type == 1 || type == 3 || type == 6 || type == 7 || type == 8) {
                handle_parent_types(type, &req, cache, deleted_keys, &max_key);
            } else {
                handle_child_types(type, &req, cache, max_key);
            }
        }
    }
}

