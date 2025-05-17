#ifndef SERVER_HANDLERS_H
#define SERVER_HANDLERS_H

#include "server_utils.h"
#include <glib.h>
#include "cache.h"
#include "document_manager.h"

/**
 * @brief Handles parent process request types.
 *
 * Processes requests received by the parent process, performing actions such as
 * cache management, document operations, or inter-process communication based on the request type.
 *
 * @param type The type of request to handle.
 * @param cache Pointer to the cache structure.
 * @param deleted_keys Array of deleted keys (GArray).
 * @param max_key Pointer to the maximum key value.
 * @param fifo_fd File descriptor for the main FIFO.
 * @param internal_fifo_fd File descriptor for the internal FIFO.
 * @param pipe_name Name of the pipe to use for communication.
 * @param request The request string to process.
 * @param doc Pointer to the document structure involved in the request.
 */
void handle_parent_types(int type, Cache* cache, GArray* deleted_keys, int* max_key, int fifo_fd, int internal_fifo_fd, char* pipe_name, char* request, Document* doc);

/**
 * @brief Handles child process request types.
 *
 * Processes requests received by the child process, performing actions such as
 * responding to client requests or updating the cache, based on the request type.
 *
 * @param type The type of request to handle.
 * @param req Pointer to the request structure.
 * @param cache Pointer to the cache structure.
 * @param max_key The maximum key value.
 */
void handle_child_types(int type, Request* req, Cache* cache, int max_key);

#endif // SERVER_HANDLERS_H
