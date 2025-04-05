#ifndef D_CLIENT_UTILS_H_
#define D_CLIENT_UTILS_H_


/**
 * @brief Verifies the input arguments provided to the client application.
 * 
 * This function checks the validity of the command-line arguments passed to the
 * client application. It ensures that the required arguments are present and
 * correctly formatted.
 * 
 * @param argc The number of command-line arguments.
 * @param argv The array of command-line arguments.
 * @return An integer indicating the result of the verification:
 *         - 1 if the input is valid.
 *         - 0 if the input is invalid.
 */
int verifyInput(int argc, char *argv[]);

#endif