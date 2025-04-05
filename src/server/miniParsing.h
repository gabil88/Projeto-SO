#ifndef MINI_PARSING
#define MINI_PARSING

/**
 * @brief Parses the command-line arguments and populates the Document structure.
 * 
 * @param argv Array of command-line arguments.
 * @param doc Pointer to a Document structure to be populated.
 * @return int Returns the following numbers for different meanings:
 *       1 if the command is "-a" (add document).
 *       2 if the command is "-c" (consult document).
 *       3 if the command is "-d" (delete document).
 *      -1 if the command is invalid or parsing fails.
 */
int parsing(char *argv[], Document *doc);

#endif