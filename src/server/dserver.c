#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "dserver_utils.h"
#include "document_manager.h"
#include "parsing.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#define FIFO_PATH "/tmp/server_fifo"
#define MAX_ITEMS_IN_MEMORY 10

typedef struct {
    char request[256];
    char pipe[128];
} Request;

int main() {
    char buffer[256];

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

    MessageQueue queue;
    queue.front = 0;
    queue.rear = 0;
    queue.count = 0;    
    
    int count = 0;

    while (1) {
        Request req;
        ssize_t bytes_read = read(fifo_fd, &req, sizeof(Request));
        if (bytes_read == sizeof(Request)) {
            // Crie uma cópia para a fila
            Request* req_copy = malloc(sizeof(Request));
            if (req_copy != NULL) {
                memcpy(req_copy, &req, sizeof(Request));
                enqueue(&queue, req_copy);
                printf("Message enqueued: %s from pipe %s\n", req.request, req.pipe);

            // Create a child process to handle the request
            pid_t pid = fork();
            if (pid < 0) {
                perror("Error during fork");
                continue; 
            } else if (pid == 0) {
                Request* req_ptr = (Request*)dequeue(&queue);
                if (req_ptr != NULL) {
                    char* pipe_name = req_ptr->pipe;
                    char* request = req_ptr->request;

                        // Process the request here
                        Document doc;
                        initialize_document(&doc, count);
                        printf("INITI");
                        count++;

                        int type = parsing(request, &doc);
                        switch (type){
                        case 1:
                            // Add document
                            if (add_document(&doc) == 0) {
                                printf("Document added successfully\n");
                                int reply_fd = open(pipe_name, O_WRONLY);
                                if (reply_fd != -1) {
                                    write(reply_fd, "Document added successfully\n", 28);
                                    close(reply_fd);
                                } else {
                                    perror("Error opening reply FIFO");
                                }
                            } else {
                                printf("Failed to add document\n");
                                int reply_fd = open(pipe_name, O_WRONLY);
                                if (reply_fd != -1) {
                                    write(reply_fd, "Failed to add document\n", 23);
                                    close(reply_fd);
                                } else {
                                    perror("Error opening reply FIFO");
                                }
                            }
                            break;
                        case 2:
                            // Consult document
                            Document* consulted_doc = consult_document(doc.key);
                            if (consulted_doc != NULL) {
                                printf("Document found: %s\n", consulted_doc->title);
                                int reply_fd = open(pipe_name, O_WRONLY);
                                if (reply_fd != -1) {
                                    char response[256];
                                    snprintf(response, sizeof(response), "Document found: %s\n", consulted_doc->title);
                                    write(reply_fd, response, strlen(response));
                                    close(reply_fd);
                                } else {
                                    perror("Error opening reply FIFO");
                                }
                                free(consulted_doc);
                            } else {
                                printf("Document not found\n");
                                int reply_fd = open(pipe_name, O_WRONLY);
                                if (reply_fd != -1) {
                                    write(reply_fd, "Document not found\n", 20);
                                    close(reply_fd);
                                } else {
                                    perror("Error opening reply FIFO");
                                }
                            }
                            break;
                        case 3:
                            // Delete document
                            if (remove_document(doc.key) == 0) {
                                printf("Document deleted successfully\n");
                                int reply_fd = open(pipe_name, O_WRONLY);
                                if (reply_fd != -1) {
                                    write(reply_fd, "Document deleted successfully\n", 31);
                                    close(reply_fd);
                                } else {
                                    perror("Error opening reply FIFO");
                                }
                            } else {
                                printf("Failed to delete document\n");
                                int reply_fd = open(pipe_name, O_WRONLY);
                                if (reply_fd != -1) {
                                    write(reply_fd, "Failed to delete document\n", 26);
                                    close(reply_fd);
                                } else {
                                    perror("Error opening reply FIFO");
                                }
                            }
                            break;
                        }
                    }
                }
            }
        }
    }
}
