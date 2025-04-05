#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "dserver_utils.h"
#include "document_manager.h"
#include "miniParsing.h"
#include <process.h>

#define FIFO_PATH "/tmp/server_fifo"


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
    
    while (1) {
        // Lé o FIFO e guarda na queue o pedido;
        ssize_t bytes_read = read(fifo_fd, buffer, sizeof(buffer) - 1);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0'; 
            enqueue(&queue, buffer);
            printf("Message enqueued: %s\n", buffer);

            pid_t pid = fork();
            if (pid == 0) {
                char *message = dequeue(&queue);
                if (message != NULL) {
                    Document* doc = malloc(sizeof(Document));
                    if (doc == NULL) {
                        perror("Error allocating memory for Document");
                        exit(EXIT_FAILURE);
                    }
                    int type = parsing(message, doc);
                    process_request(type, doc);
                    free(doc);
                }
                exit(0); 
            } else if (pid < 0) {
                perror("Error creating child process");
            }
        } else if (bytes_read == 0) {
            // No data received, continua a andar
            printf("No data received. Waiting for new messages...\n");
            sleep(1); 
        } else {
            perror("Error reading from FIFO");
        }
    }

    // Close the FIFO
    close(fifo_fd);

    return 0;
}
