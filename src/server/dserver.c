#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define FIFO_PATH "/path/to/your/fifo"

int main() {
    char buffer[256];
    int fifo_fd;

    // Open the FIFO for reading
    fifo_fd = open(FIFO_PATH, O_RDONLY);
    if (fifo_fd == -1) {
        perror("Error opening FIFO");
        exit(EXIT_FAILURE);
    }

    // Read from the FIFO
    ssize_t bytes_read = read(fifo_fd, buffer, sizeof(buffer) - 1);
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0'; // Null-terminate the string
        printf("Received message: %s\n", buffer);
    } else if (bytes_read == 0) {
        printf("No data received.\n");
    } else {
        perror("Error reading from FIFO");
    }

    // Close the FIFO
    close(fifo_fd);

    return 0;
}