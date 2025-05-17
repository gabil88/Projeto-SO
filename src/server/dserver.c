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
#include "../../include/server/server_utils.h"
#include "../../include/server/server_init.h"
#include "../../include/server/server_handlers.h"



int main(int argc, char *argv[]) {
    char pathToFile[4096] = "DatasetTest/Gdataset";
    int max_items_in_memory = 10;
    CachePolicy policy = CACHE_POLICY_LRU;

    // Argument parsing: ./dserver <dataset_path> <cache_size> <policy>
    if (argc >= 3) {
        strncpy(pathToFile, argv[1], sizeof(pathToFile) - 1);
        pathToFile[sizeof(pathToFile) - 1] = '\0';
        max_items_in_memory = atoi(argv[2]);
    } else {
        printf("Uso: %s <dataset_path> <cache_size>\n", argv[0]);
        printf("Exemplo: %s DatasetTest/Gdataset 5\n", argv[0]);
        return 1;
    }

    int fifo_fd, internal_fifo_fd;
    initialize_fifos(&fifo_fd, &internal_fifo_fd);
    printf("Initialzing cache with size: %d\n", max_items_in_memory);
    Cache* cache = cache_init(max_items_in_memory, policy);
    GArray* deleted_keys = g_array_new(FALSE, FALSE, sizeof(int));
    int max_key = load_deleted_keys(deleted_keys);
    int result = start_logger();
    if (result == -1) {
        fprintf(stderr, "Error starting logger.\n");
        return -1;
    }
    
    while (1) {
        Request req;
        ssize_t bytes_read = read(internal_fifo_fd, &req, sizeof(Request));
        if (bytes_read != sizeof(Request)) {
            bytes_read = read(fifo_fd, &req, sizeof(Request));
        }
        if (bytes_read == sizeof(Request)) {
            char* request = req.request;
            char* pipe_name = req.pipe;
            Document* doc = malloc(sizeof(Document));
            if (!doc) { send_message(pipe_name, "Server error: Unable to allocate memory."); return -1; }
            initialize_document(doc);
            int type = parsing(request, doc);
            printf("Request type: %d\n", type);
            if (type == 0 || type == 1 || type == 3 || type == 6 || type == 7 || type == 8) {
                handle_parent_types(type, cache, deleted_keys, &max_key, fifo_fd, internal_fifo_fd, pipe_name, request, doc);
            } else {
                printf("Child process handling request...\n");
                handle_child_types(type, &req, cache, max_key);
            }
        }
    }
}

