#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H

#include "cache.h"

typedef struct {
    char request[256];
    char pipe[128];
} Request;

#define FIFO_PATH "/tmp/server_fifo"

#define INTERNAL_FIFO_PATH "/tmp/internal_fifo"
/**
 * @brief Sends a message to a client through a named pipe (FIFO).
 *
 * Opens the specified pipe for writing and sends the provided message string.
 * If the pipe cannot be opened or written to, an error is printed to stderr.
 *
 * @param pipe_name The name (path) of the FIFO to write to.
 * @param message The message string to send to the client.
 */
void send_message(const char* pipe_name, const char* message);

/**
 * @brief Prints the current state of the cache to stdout.
 *
 * Iterates through all cache slots and prints the title of the document in each slot,
 * or NULL if the slot is empty. Useful for debugging and monitoring cache contents.
 *
 * @param cache Pointer to the Cache structure to be printed.
 */
void print_cache_state(Cache* cache);

/**
 * @brief Sends an internal request to the server using the internal FIFO.
 *
 * Prepares a Request struct with the given request string and a default response pipe,
 * then writes it to the internal FIFO for server-side handling. Used for internal
 * communication between server processes.
 *
 * @param request The request string to send internally to the server.
 */
void send_internal_request_to_server(const char* request);

#endif // SERVER_UTILS_H
