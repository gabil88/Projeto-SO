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
        if(bytes_written == -1) {
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

int main() {
    
    // Remove the FIFO if it already exists
    unlink(FIFO_PATH);
    unlink(INTERNAL_FIFO_PATH); // Remove internal FIFO if it exists
    
    // Create the FIFO 
    if (mkfifo(FIFO_PATH, 0666) == -1) {
        perror("Error creating FIFO");
        return -1;
    }

    // Open the FIFO for reading
    int fifo_fd = open(FIFO_PATH, O_RDONLY);
    if (fifo_fd == -1) {
        perror("Error opening FIFO");
        return -1;
    }

    if (mkfifo(INTERNAL_FIFO_PATH, 0666) == -1) {
        perror("Error creating internal FIFO");
        close(fifo_fd);
        unlink(FIFO_PATH);
        return -1;
    }
    
    int internal_fifo_fd = open(INTERNAL_FIFO_PATH, O_RDONLY | O_NONBLOCK);
    if (internal_fifo_fd == -1) {
        perror("Error opening internal FIFO");
        close(fifo_fd);
        unlink(FIFO_PATH);
    }
    
    Cache* cache = cache_init(); // Inicializa a cache 

    GArray* deleted_keys = g_array_new(FALSE, FALSE, sizeof(int));

    int max_key = load_deleted_keys(deleted_keys);

    printf("Server started. Waiting for requests...\n");
    

    while (1) {
        Request req;
        ssize_t bytes_read = read(internal_fifo_fd, &req, sizeof(Request));
        if (bytes_read != sizeof(Request)) {
            // If nothing in internal FIFO, try normal FIFO (blocking)
            bytes_read = read(fifo_fd, &req, sizeof(Request));
        }
        if (bytes_read == sizeof(Request)) {
            // Process the request (from either FIFO)
            printf("Received request: %s from pipe %s\n", req.request, req.pipe);
            char* request = req.request;
            char* pipe_name = req.pipe;

            Document* doc = malloc(sizeof(Document));
            if (doc == NULL) {
                perror("Error allocating memory for document");
                send_message(req.pipe, "Server error: Unable to allocate memory.");
                continue;
            }

            initialize_document(doc);
            
            int type = parsing(request, doc);
            printf("Parsed type: %d\n", type);;
            int status = -1;
            pid_t pid; 

            if (type == 0 || type == 1 || type == 3 || type == 6 || type == 7 || type == 8) {
                // Handle type 1 and 3 in the parent process
                switch(type){
                    case 0:
                        g_array_free(deleted_keys, TRUE);
                        cache_flush_all_dirty(cache);
                        free(cache);
                        close(fifo_fd);
                        close(internal_fifo_fd);
                        unlink(FIFO_PATH);
                        send_message(pipe_name, "Server shutting down.");            
                        exit(0);
                        break;
                    case 1: // Add document
                        if (deleted_keys->len > 0){
                            doc->key = g_array_index(deleted_keys, int, deleted_keys->len - 1);
                            g_array_remove_index(deleted_keys, deleted_keys->len - 1);
                        } else {
                            doc->key = max_key + 1;
                        }
                        status = cache_add(cache, doc, 0);
                        if (status == 0) {
                            status = add_document(doc);
                        }
                        printf("cache_add status: %d\n", status);
                        if (status == 0){
                            char message[256];
                            snprintf(message, sizeof(message), "Document added successfully with key: %d", doc->key);
                            send_message(pipe_name, message);
                            max_key++;
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
                    case 6: // Wait for lost child
                        pid = atoi(request + 3);
                        printf("Waiting for child with PID: %d\n", pid);
                        fflush(stdout); // Ensure the message is printed immediately
                        if (pid > 0) {
                            int status;
                            int ret = waitpid(pid, &status, 0);
                            if (ret == -1) {
                                perror("waitpid failed");
                            } else if (WIFEXITED(status)) {
                                printf("Child process terminated.\n");
                            } else {
                                printf("Child process terminated abnormally.\n");
                            }
                        } else {
                            printf("Invalid PID.\n");
                        }
                        break;
                    case 7: // Add to cache
                        printf("Adding from disk to cache: %s\n", req.request);
                        int key_to_add = atoi(req.request + 4); // Example -> string "-ac/323" -> atoi("323") -> 323 
                        doc = consult_document(key_to_add);
                        printf("Document to add to cache: %s\n", doc->title);
                        status = cache_add(cache, doc, 1); // 1 means it skips the storage check
                        print_cache_state(cache);
                        if (status == 0) {
                            printf("Document added to cache successfully.\n");
                        } else {
                            printf("Unexpected Error adding document to cache.\n");
                        }
                        break;
                    case 8:
                        printf("Updating time in cache: %s\n", req.request);
                        int key_to_update = atoi(req.request + 4); // Example -> string "-uc/323" -> atoi("323") -> 323 
                        int status = cache_update_time(cache, key_to_update);
                        if (status == 0) {
                            printf("Document updated in cache successfully.\n");
                        } else {
                            printf("Unexpected Error updating document time in cache.\n");
                        }
                        break;
                }
            } else {
                pid_t child_pid = fork();
                if (child_pid < 0) {
                    perror("Error during fork");
                    free(doc);
                    continue;
                }
                if (child_pid == 0) {
                    // Child process
                    printf("Child process created with PID: %d\n", getpid());
                    pid_t grandchild_pid = fork();
                    if (grandchild_pid < 0) {
                        perror("Error during grandchild fork");
                        free(doc);
                        exit(1);
                    }
                    if (grandchild_pid == 0) {
                        printf("Grandchild process created with PID: %d\n", getpid());
                        // Grandchild process
                        switch(type) {
                            case 2: {
                                Document* found_doc = cache_get(cache, doc->key);
                                if (found_doc != NULL) {
                                    char message[256];
                                    snprintf(message, sizeof(message), "Document found in cache with Key: %d and Title: %s", found_doc->key, found_doc->title);
                                    send_message(pipe_name, message);

                                    char updateCache[256];
                                    snprintf(updateCache, sizeof(updateCache), "-uc/%d", found_doc->key);
                                    send_internal_request_to_server(updateCache);
                                    // Do NOT free(found_doc) here! It's owned by the cache.
                                } else {
                                    found_doc = consult_document(doc->key);
                                    if (found_doc != NULL) {
                                        char message[256];
                                        snprintf(message, sizeof(message), "Document found in storage with Key: %d and Title: %s", found_doc->key, found_doc->title);
                                        send_message(pipe_name, message);

                                        char addCache[256];
                                        snprintf(addCache, sizeof(addCache), "-ac/%d", found_doc->key);
                                        send_internal_request_to_server(addCache);

                                        free(found_doc); // Only free if it came from consult_document
                                    } else {
                                        send_message(pipe_name, "Document not found.");    
                                    }
                                }
                                break;
                            }
                            case 4: {
                                printf("Received request for -l: %s\n", request);
                                
                                char** extracted = special_parsing(request);
                                if (!extracted) {
                                    send_message(pipe_name, "Invalid request format.");
                                    free(doc);
                                    exit(1);
                                }
                                
                                printf("Extracted key: %s\n", extracted[0]);
                                printf("Extracted keyword: %s\n", extracted[1]);
                                
                                int key = atoi(extracted[0]);
                                Document* extracted_doc = cache_get(cache, key);
                                
                                if (extracted_doc != NULL) {
                                    // Document found in cache
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
                                    // DO NOT free(extracted_doc) here!
                                } else {
                                    // Check storage
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
                                printf("Received request for -s: %s\n", request);
                                
                                char** extracted = special_parsing(request);
                                printf("Extracted keyword: %s\n", extracted[0]);
                                printf("Extracted number of processes: %s\n", extracted[1]);
                                int nr_process = atoi(extracted[1]);

                                int count = get_number_of_docs_with_keyword(extracted[0], nr_process, max_key);
                                if (count == -1) {
                                    send_message(pipe_name, "Error reading file.");
                                } else {
                                    char message[256];
                                    snprintf(message, sizeof(message), "Number of documents with keyword '%s': %d", extracted[0], count);
                                    send_message(pipe_name, message);
                                }
                                // Free the extracted strings
                                free(extracted[0]);
                                free(extracted[1]);
                                free(extracted);
                            }
                        }
                        free(doc);
                        exit(0);
                    } else {
                        // Child process waits for grandchild
                        int status;
                        waitpid(grandchild_pid, &status, 0);

                        // Notify parent process that child has finished
                        char notify[256];
                        snprintf(notify, sizeof(notify), "-w/%d", getpid());

                        Request notify_req = {0};
                        snprintf(notify_req.request, sizeof(notify_req.request), "%s", notify); 
                        snprintf(notify_req.pipe, sizeof(notify_req.pipe), "%s", pipe_name);

                        // Open FIFO for writing only when sending notification
                        int notify_fd = open(FIFO_PATH, O_WRONLY);
                        if (notify_fd != -1) {
                            ssize_t bytes_written = write(notify_fd, &notify_req, sizeof(Request));
                            if (bytes_written == -1) {
                                perror("Error writing to notification FIFO");
                            } else if (bytes_written != sizeof(Request)) {
                                printf("Partial write to FIFO\n");
                            }
                            close(notify_fd);
                        } else {
                            perror("Error opening FIFO for notification");
                        }

                        free(doc);
                        exit(0);
                    }
                }
            } 
        }
    }
}

