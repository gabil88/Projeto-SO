#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>



int main(char argc, char *argv[]) {
    // Create a named pipe (FIFO) with read and write permissions for all users
    if (mkfifo("fifo", 0666) == -1) {
        perror("mkfifo failed");
        return 1;
    }

    open("fifo", O_WRONLY);    

    if(argv[1] == "a" && argc == 6){
        char request_buffer[1024];
        snprintf(request_buffer, sizeof(request_buffer), "%s %s %s %s", argv[2], argv[3], argv[4], argv[5]);
        
    }
    else return -1;
}

// ./dclient -a "title" "authors" "year" "path"