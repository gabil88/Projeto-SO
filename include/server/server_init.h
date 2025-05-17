#ifndef SERVER_INIT_H
#define SERVER_INIT_H

/**
 * @brief Starts the server logger.
 *
 * Initializes the logging system for the server, typically by opening a log file
 * or setting up logging resources. Returns a status code indicating success or failure.
 *
 * @return fd on success, -1 on failure.
 */
int start_logger();

/**
 * @brief Initializes the server FIFOs (named pipes).
 *
 * Creates and opens the main and internal FIFOs used for server communication.
 * The file descriptors for these FIFOs are returned via the provided pointers.
 *
 * @param fifo_fd Pointer to an int where the main FIFO file descriptor will be stored.
 * @param internal_fifo_fd Pointer to an int where the internal FIFO file descriptor will be stored.
 */
void initialize_fifos(int* fifo_fd, int* internal_fifo_fd);

#endif // SERVER_INIT_H
