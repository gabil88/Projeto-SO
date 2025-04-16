#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include "../../include/client/dclientUtils.h"


#define SERVER_PATH "/tmp/server_fifo"

typedef struct {
    char request[256];
    char pipe[128];
} Request;


int main(int argc, char *argv[]) {
    // Verify command line arguments
    if(!verifyInput(argc, argv)){
        printf("Invalid input\n");
        return -1;
    }

    // Create a unique client pipe name using PID
    char client_pipe_name[64];
    sprintf(client_pipe_name, "/tmp/client_fifo_%d",getpid());
    
    // Create client-specific FIFO for receiving responses
    if (mkfifo(client_pipe_name, 0666) == -1) {
        perror("Error creating client FIFO");
        return -1;
    }
    
    // Prepare request buffer
    char request_buffer[1024] = {0};
    
    // Concatenate command line arguments to request
    for (int i = 1; i < argc; i++) {
        strncat(request_buffer, argv[i], sizeof(request_buffer) - strlen(request_buffer) - 1);
        if (i < argc - 1) {
            strncat(request_buffer, " ", sizeof(request_buffer) - strlen(request_buffer) - 1);
        }
    }

    // struct com pedido e caminho de resposta
    Request req;
    strncpy(req.request, request_buffer, sizeof(req.request) - 1);
    req.request[sizeof(req.request) - 1] = '\0';
    strncpy(req.pipe, client_pipe_name, sizeof(req.pipe) - 1);
    req.pipe[sizeof(req.pipe) - 1] = '\0';


    // Open server FIFO for writing (global server pipe)
    // Até 4Kb o kernel garante que a escrita é atomica, ou seja, não vai misturar varias escritas
    int server_fifo = open(SERVER_PATH, O_WRONLY);
    if (server_fifo < 0) {
        perror("Error opening server FIFO - is the server running?");
        unlink(client_pipe_name); 
        return -1;
    }

    // Write the Request struct to the server FIFO
    if (write(server_fifo, &req, sizeof(req)) != sizeof(req)) {
        perror("Error writing to server FIFO");
        close(server_fifo);
        unlink(client_pipe_name);
        return -1;
    }

    close(server_fifo);
    // Open client FIFO to read the response
    int client_fifo = open(client_pipe_name, O_RDONLY);
    if (client_fifo < 0) {
        perror("Error opening client FIFO for reading");
        unlink(client_pipe_name);
        return -1;
    }

    /* Read and display server response
    *
    * Read bloqueia até algo ser escrito, talvez precise de um metodo caso o servidor abandone
    * o cliente chilladamente, time out! 
    */
    char response_buffer[4096] = {0};
    ssize_t bytes_read = read(client_fifo, response_buffer, sizeof(response_buffer) - 1);
    
    if (bytes_read > 0) {
        response_buffer[bytes_read] = '\0';
        printf("%s\n", response_buffer);
    } else if (bytes_read < 0) {
        perror("Error reading from client FIFO");
    } else {
        printf("No response received from server\n");
    }

    // Clean up
    close(client_fifo);
    unlink(client_pipe_name);

    return 0;
}