#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>

int verifyInput(int argc, char *argv[]) {
    // Must have at least 2 arguments (program name + command)
    if (argc < 2) {
        printf("Usage: \n");
        printf("  Add document:    %s -a <title> <author> <year> [path]\n", argv[0]);
        printf("  Consult doc:     %s -c <key>\n", argv[0]);
        printf("  Remove doc:      %s -d <key>\n", argv[0]);
        return 0;
    }
    
    // Check command types
    if (strcmp(argv[1], "-a") == 0) {
        // Add document: needs 6 args (command, -a, title, author, year, path)
        if (argc == 6) {
            return 1;
        } else {
            printf("Usage for add: %s -a <title> <author> <year> [path]\n", argv[0]);
            return 0;
        }
    }
    else if (strcmp(argv[1], "-s") == 0) {
        // Accept -s <keyword> or -s <keyword> <nr_processes>
        if (argc == 3 || argc == 4) {
            return 1;
        } else {
            printf("Usage for Number of docs with a certain keyword: %s -s <keyword> [nr_processes]\n", argv[0]);
            return 0;
        }
    }
    else if (strcmp(argv[1], "-l") == 0) {
        // List documents: needs 2 args (command, -l, "key", "keyword")
        if (argc == 4) {
            return 1;
        } else {
            printf("Usage for Querie1: %s -l <key> <keyword>\n", argv[0]);
            return 0;
        }
    }
    else if (strcmp(argv[1], "-c") == 0 || 
             strcmp(argv[1], "-d") == 0){
        // Consult, Delete needs 3 args (command, -[c|d], key)
        if (argc == 3) {
            return 1;
        } else {
            char* cmd_type = (argv[1][1] == 'c') ? "consult" : 
                             (argv[1][1] == 'd') ? "delete" : "replace";
            printf("Usage for %s: %s %s <key>\n", cmd_type, argv[0], argv[1]);
            return 0;
        }
    }
    else if(strcmp(argv[1], "-f") == 0){
        if (argc == 2) {
            return 1;
        } else {
            printf("Usage for flush: %s -f\n", argv[0]);
            return 0;
        }
    }
    else {
        printf("Unknown command: %s\n", argv[1]);
        printf("Valid commands: -a (add), -l (list), -c (consult), -d (delete), -r (replace)\n");
        return 0;
    }
}