#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>

int verifyInput(int argc, char *argv[]) {
    if (strcmp(argv[1], "-a") == 0 && argc == 6) {
        return 1;
    }
    else if (strcmp(argv[1], "-l") == 0 && argc == 4) {
        return 1;
    }
    else if ((strcmp(argv[1], "-r") == 0 || 
              strcmp(argv[1], "-c") == 0 || 
              strcmp(argv[1], "-d") == 0) && 
             argc == 3) {
        return 1;
    }
    else {
        return 0;
    }
}