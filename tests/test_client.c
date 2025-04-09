#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "../src/client/dclientUtils.h"
#include "../src/server/document_manager.h"
#include "../src/server/parsing.h"

// Include client structure definition
typedef struct {
    char request[256];
    char pipe[128];
} Request;



// CONFIRMA SE O FIFO É CRIADO BEM!
void test_client_pipe_creation() {
    printf("Test: Client pipe creation\n");
    
    // Create a test pipe name
    char test_pipe[64];
    sprintf(test_pipe, "/tmp/test_client_pipe_%d", getpid());
    printf("  Creating pipe: %s\n", test_pipe);
    
    // Remove if exists
    unlink(test_pipe);
    
    // Create pipe
    int result = mkfifo(test_pipe, 0666);
    assert(result == 0 && "Failed to create test pipe");
    printf("  ✓ Pipe created successfully\n");
    
    // Verify pipe exists and has correct permissions
    struct stat st;
    result = stat(test_pipe, &st);
    assert(result == 0 && "Failed to stat pipe");
    assert(S_ISFIFO(st.st_mode) && "Not a FIFO");
    printf("  ✓ Pipe has correct type\n");
    
    // Clean up
    unlink(test_pipe);
    printf("  ✓ Pipe cleanup successful\n");
}

// VERIFICA SE A ESTRUTURA PODE SER MAL ESCRITA DEPOIS AO LÊ-LA
void test_request_structure() {
    printf("\nTest: Request structure packing\n");
    
    // Create a request
    Request req;
    strcpy(req.request, "-a \"Test Document\" \"Test Author\" 2023");
    strcpy(req.pipe, "/tmp/test_client_pipe_12345");
    printf("  Request created: request=\"%s\", pipe=\"%s\"\n", req.request, req.pipe);
    
    // Write to a temporary file
    char temp_file[] = "/tmp/test_request_struct";
    int fd = open(temp_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    assert(fd > 0 && "Failed to create temporary file");
    
    ssize_t written = write(fd, &req, sizeof(Request));
    assert(written == sizeof(Request) && "Failed to write request structure");
    close(fd);
    printf("  ✓ Request structure written to file\n");
    
    // Read it back and verify
    Request read_req;
    fd = open(temp_file, O_RDONLY);
    assert(fd > 0 && "Failed to open temporary file");
    
    ssize_t bytes_read = read(fd, &read_req, sizeof(Request));
    assert(bytes_read == sizeof(Request) && "Failed to read request structure");
    close(fd);
    
    assert(strcmp(req.request, read_req.request) == 0 && "Request field mismatch");
    assert(strcmp(req.pipe, read_req.pipe) == 0 && "Pipe field mismatch");
    printf("  ✓ Request structure verified\n");
    
    // Clean up
    unlink(temp_file);
    printf("  ✓ Temporary file cleanup successful\n");
}

// TESTA A COMUNICAÇÃO ENTRE CLIENTE E SERVIDOR
void test_client_server_communication() {
    printf("\nTest: Client-Server Communication (Simulated)\n");
    
    // Create server and client pipes
    char server_pipe[] = "/tmp/test_server_fifo";
    char client_pipe[64];
    sprintf(client_pipe, "/tmp/test_client_fifo_%d", getpid());
    printf("  Creating server pipe: %s\n", server_pipe);
    printf("  Creating client pipe: %s\n", client_pipe);
    
    unlink(server_pipe);
    unlink(client_pipe);
    
    // Create pipes
    assert(mkfifo(server_pipe, 0666) == 0 && "Failed to create server pipe");
    assert(mkfifo(client_pipe, 0666) == 0 && "Failed to create client pipe");
    printf("  ✓ Test pipes created\n");
    
    // Fork to simulate client and server
    pid_t pid = fork();
    assert(pid >= 0 && "Fork failed");
    
    if (pid == 0) {
        // Child process - simulate server
        sleep(1); // Wait for client to prepare
        
        // Open server pipe to read request
        int server_fd = open(server_pipe, O_RDONLY);
        assert(server_fd > 0 && "Server failed to open pipe");
        
        // Read request
        Request req;
        ssize_t bytes_read = read(server_fd, &req, sizeof(Request));
        assert(bytes_read == sizeof(Request) && "Server failed to read request");
        close(server_fd);
        
        // Print received request
        printf("  ✓ Server received request\n");
        printf("  Server received request: request=\"%s\", pipe=\"%s\"\n", req.request, req.pipe);
        
        // Send response through client pipe
        int client_fd = open(req.pipe, O_WRONLY);
        assert(client_fd > 0 && "Server failed to open client pipe");
        
        char response[] = "Response from server";
        printf("  Server sending response: \"%s\"\n", response);
        write(client_fd, response, sizeof(response));
        close(client_fd);
        
        exit(0);
    } else {
        // Parent process - simulate client
        // Prepare request
        Request req;
        strcpy(req.request, "test_message");
        strcpy(req.pipe, client_pipe);
        printf("  Client sending request: request=\"%s\", pipe=\"%s\"\n", req.request, req.pipe);
        
        // Open server pipe to send request
        int server_fd = open(server_pipe, O_WRONLY);
        assert(server_fd > 0 && "Client failed to open server pipe");
        
        // Send request
        ssize_t written = write(server_fd, &req, sizeof(Request));
        assert(written == sizeof(Request) && "Client failed to send request");
        close(server_fd);
        printf("  ✓ Client sent request\n");
        
        // Open client pipe to read response
        int client_fd = open(client_pipe, O_RDONLY);
        assert(client_fd > 0 && "Client failed to open its pipe");
        
        // Read response
        char response[256] = {0};
        ssize_t bytes_read = read(client_fd, response, sizeof(response));
        assert(bytes_read > 0 && "Client failed to read response");
        close(client_fd);
        
        printf("  Client received response: \"%s\"\n", response);
        assert(strcmp(response, "Response from server") == 0 && "Incorrect response");
        printf("  ✓ Client received correct response\n");
        
        // Wait for child process
        int status;
        waitpid(pid, &status, 0);
        assert(WIFEXITED(status) && WEXITSTATUS(status) == 0 && "Server process failed");
        printf("  ✓ Server process exited successfully\n");
        
        // Clean up
        unlink(server_pipe);
        unlink(client_pipe);
        printf("  ✓ Pipe cleanup successful\n");
    }
}

int main() {
    printf("Starting client tests...\n");
    
    test_client_pipe_creation();
    test_request_structure();
    test_client_server_communication();
    
    printf("\nAll client tests passed!\n");
    return 0;
}