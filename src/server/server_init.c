#include "../../include/server/server_init.h"
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

#include "../../include/server/server_utils.h"

int start_logger() {
    if (mkdir("logs", 0755) == -1 && errno != EEXIST) {
        perror("Error creating logs directory");
        return -1;
    }
    int log_fd = open("logs/server_log.txt", O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (log_fd == -1) {
        perror("Error opening log file");
        return -1;
    }
    if (dup2(log_fd, STDERR_FILENO) == -1) {
        perror("Error redirecting stderr to log file");
        close(log_fd);
        return -1;
    }
    return log_fd;
}

void initialize_fifos(int* fifo_fd, int* internal_fifo_fd) {
    unlink(FIFO_PATH);
    unlink(INTERNAL_FIFO_PATH);
    if (mkfifo(FIFO_PATH, 0666) == -1) { perror("Error creating FIFO"); exit(-1); }
    if (mkfifo(INTERNAL_FIFO_PATH, 0666) == -1) { perror("Error creating internal FIFO"); unlink(FIFO_PATH); exit(-1); }
    *fifo_fd = open(FIFO_PATH, O_RDONLY);
    if (*fifo_fd == -1) { perror("Error opening FIFO"); unlink(FIFO_PATH); unlink(INTERNAL_FIFO_PATH); exit(-1); }
    *internal_fifo_fd = open(INTERNAL_FIFO_PATH, O_RDONLY | O_NONBLOCK);
    if (*internal_fifo_fd == -1) { perror("Error opening internal FIFO"); close(*fifo_fd); unlink(FIFO_PATH); unlink(INTERNAL_FIFO_PATH); exit(-1); }
}
