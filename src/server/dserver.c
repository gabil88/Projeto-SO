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

#include "../../include/server/dserver_utils.h"
#include "../../include/server/document_manager.h"
#include "../../include/server/parsing.h"

#define FIFO_PATH "/tmp/server_fifo"
#define MAX_ITEMS_IN_MEMORY 10


typedef struct {
    char request[256];
    char pipe[128];
} Request;
 

// Função para enviar mensagem para o cliente
void send_message(const char* pipe_name, const char* message) {
    int reply_fd = open(pipe_name, O_WRONLY);
    if (reply_fd != -1) {
        write(reply_fd, message, strlen(message));
        close(reply_fd);
    } else {
        perror("Error opening reply FIFO");
    }
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

    printf("Server started. Waiting for requests...\n");
    

    while (1) {

        while(waitpid(-1, NULL, WNOHANG) > 0); // Wait for child processes to finish

        Request req;
        ssize_t bytes_read = read(fifo_fd, &req, sizeof(Request));
        if (bytes_read == sizeof(Request)) {
            printf("Received request: %s from pipe %s\n", req.request, req.pipe);

            // Create a child process to handle the request
            pid_t pid = fork();

            if (pid < 0) {
                perror("Error during fork");
                continue;
            }
            
            else if (pid == 0) {

                // Define request and pipe_name for the child process
                char* request = req.request;
                char* pipe_name = req.pipe;

                printf("Processing request: %s from pipe %s\n", request, pipe_name);

                // Process the request
                Document* doc = malloc(sizeof(Document));
                if (doc == NULL) {
                    perror("Error allocating memory for document");
                    send_message(req.pipe, "Server error: Unable to allocate memory.");
                    exit(1);
                }

                int unique_key = generate_unique_key();
                printf("Initializing document with unique key: %d\n", unique_key);
                initialize_document(doc, unique_key);
                printf("After initialization: doc->key = %d\n", doc->key); // Verificar se a chave foi realmente atualizada

                int type = parsing(request, doc);
                int status = -1;

                switch(type){
                    case 1: // Add document
                        status = add_document(doc);
                        if (status == 0) {
                            send_message(pipe_name, "Document added successfully.");
                        } else if (status == -2) {
                            send_message(pipe_name, "Document with the same title already exists.");
                        } else {
                            send_message(pipe_name, "Error adding document.");
                        }
                        break;

                    case 2: // Consult document
                        Document* found_doc = consult_document(doc->key);
                        if (found_doc != NULL) {
                            char message[256];
                            snprintf(message, sizeof(message), "Document found: Key: %d, Title: %s", found_doc->key, found_doc->title);
                            send_message(pipe_name, message);
                            free(found_doc);
                        } else {
                            send_message(pipe_name, "Document not found.");
                        }
                        break;

                    case 3: // Remove document
                        status = remove_document(doc->key);
                        if (status == 0) {
                            send_message(pipe_name, "Document removed successfully.");
                        } else {
                            send_message(pipe_name, "Error removing document.");
                        }
                        break;

                    default:
                        send_message(pipe_name, "Invalid request.");
                }
                exit(0);
            }
        }
    }
}
