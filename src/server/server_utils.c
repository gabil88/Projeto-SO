#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "../../include/server/cache.h"
#include "../../include/server/document_manager.h"
#include "../../include/server/server_utils.h"


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


void print_cache_state(Cache* cache) {
    printf("Cache state:\n");
    for (int i = 0; i < cache->max_size; i++) {
        printf("Slot %d: %s\n", i, cache->items[i].doc ? cache->items[i].doc->title : "NULL");
    }
    fflush(stdout);
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
