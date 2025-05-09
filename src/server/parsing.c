#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

#include "../../include/server/document_manager.h"
#include "../../include/server/parsing.h"

int parsing(char* request, Document* doc) {
    // Save the original key
    int original_key = doc->key;

    // Make a copy of the request to avoid modifying the original string
    char request_copy[1024];
    strncpy(request_copy, request, sizeof(request_copy) - 1);
    request_copy[sizeof(request_copy) - 1] = '\0';

    // Parse the command type
    char* token = strtok(request_copy, "/");
    if (!token) return -1;

    int type = -1;
    if (strcmp(token, "-f") == 0) {
        type = 0; // Flush server
    } else if (strcmp(token, "-a") == 0) {
        type = 1; // Add document
    } else if (strcmp(token, "-c") == 0) {
        type = 2; // Consult document
    } else if (strcmp(token, "-d") == 0) {
        type = 3; // Delete document
    } else if (strcmp(token, "-l") == 0) {
        type = 4; // Querie -l
    } else if (strcmp(token, "-s") == 0) {
        type = 5; // Querie -s
    } else if (strcmp(token, "-w") == 0) {
        type = 6; // Wait for lost child
    } else if (strcmp(token, "-ac") == 0) {
        type = 7; // Exist in doc, add to cache
    } else if (strcmp(token, "-uc") == 0) {
        type = 8; // Update time in cache
    } else {
        return -1; // Invalid command
    }

    // Parse the next parameter depending on command type
    if (type == 1) { // Add document - need title, author, year, path
        // Parse title
        token = strtok(NULL, "/");
        if (!token) return -1;
        strncpy(doc->title, token, sizeof(doc->title) - 1);
        doc->title[sizeof(doc->title) - 1] = '\0';

        // Parse author
        token = strtok(NULL, "/");
        if (!token) return -1;
        strncpy(doc->author, token, sizeof(doc->author) - 1);
        doc->author[sizeof(doc->author) - 1] = '\0';

        // Parse year
        token = strtok(NULL, "/");
        if (!token) return -1;
        doc->year = atoi(token);

        // Parse path if provided
        token = strtok(NULL, "/");
        if (token) {
            strncpy(doc->path, token, sizeof(doc->path) - 1);
            doc->path[sizeof(doc->path) - 1] = '\0';
        }
    } else if (type == 2 || type == 3) { // Consult or Delete - need key
        token = strtok(NULL, "/");
        if (!token) return -1;
        doc->key = atoi(token);
    }
    // Restore the original key if we didn't explicitly set it
    if (type == 1) {
        doc->key = original_key;
    }

    return type;
}
