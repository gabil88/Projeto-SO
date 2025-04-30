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

#define FIFO_PATH "/tmp/server_fifo"
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

int main() {
    
    // Remove the FIFO if it already exists
    unlink(FIFO_PATH);
    
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

    Cache* cache = cache_init(); // Inicializa a cache 

    GArray* deleted_keys = g_array_new(FALSE, FALSE, sizeof(int));

    int max_key = load_deleted_keys(deleted_keys);

    printf("Server started. Waiting for requests...\n");
    

    while (1) {
 
        Request req;
        ssize_t bytes_read = read(fifo_fd, &req, sizeof(Request));
        if (bytes_read == sizeof(Request)) {
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
            printf("Parsed type: %d\n", type);
            int status = -1;
            pid_t pid; 

            if (type == 0 || type == 1 || type == 3 || type == 6) {
                // Handle type 1 and 3 in the parent process
                switch(type){
                    case 0:
                        g_array_free(deleted_keys, TRUE);
                        cache_flush_all_dirty(cache);
                        free(cache);
                        close(fifo_fd);
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
                        print_cache_state(cache); //debugging
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
                            send_message(pipe_name, "Document removed successfully.");
                            g_array_append_val(deleted_keys, doc->key);
                        } else {
                            send_message(pipe_name, "Error removing document.");
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
                    pid_t grandchild_pid = fork();
                    if (grandchild_pid < 0) {
                        perror("Error during grandchild fork");
                        free(doc);
                        exit(1);
                    }
                    if (grandchild_pid == 0) {
                        // Grandchild process
                        switch(type) {
                            case 2: {
                                Document* found_doc = cache_get(cache, doc->key);
                                if (found_doc != NULL) {
                                    char message[256];
                                    snprintf(message, sizeof(message), "Document found: Key: %d, Title: %s", found_doc->key, found_doc->title);
                                    send_message(pipe_name, message);
                                    free(found_doc);
                                } else {
                                    send_message(pipe_name, "Document not found.");
                                }
                                break;
                            }
                            case 4: {
                                // bleh bleh
                            }
                            case 5: {
                                // blah blah
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
                        strncpy(notify_req.request, notify, sizeof(notify_req.request) - 1);
                        strncpy(notify_req.pipe, pipe_name, sizeof(notify_req.pipe) - 1);

                        // Open FIFO for writing only when sending notification
                        int notify_fd = open(FIFO_PATH, O_WRONLY);
                        if (notify_fd != -1) {
                            write(notify_fd, &notify_req, sizeof(Request));
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

