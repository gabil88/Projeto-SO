#ifndef PARSING
#define PARSING

#include "document_manager.h"

/**
 * @brief Parses the command-line arguments and populates the Document structure.
 * 
 * @param request String with the request.
 * @param doc Pointer to a Document structure to be populated.
 * @return int Returns the following numbers for different meanings:
 *       1 if the command is "-a" (add document).
 *       2 if the command is "-c" (consult document).
 *       3 if the command is "-d" (delete document).
 *      -1 if the command is invalid or parsing fails.
 */
int parsing(char* request, Document *doc);

/*
* @brief Parses a special request to extract the keyword and number of processes.
* 
* @param request String with the request.
* @return char** Array of strings containing the keyword and number of processes.
*/
char** special_parsing(char* request);

#endif