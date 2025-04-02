#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>



int main(char argc, char *argv[]) {
    // Create a named pipe (FIFO) with read and write permissions for all users
    int fd = mkfifo("fifo", 0666);
    if (fd < 0) {
        perror("Error creating FIFO");
        return -1;
    }

    open("fifo", O_WRONLY);    

    /* Este bloco de código é responsável por enviar as requisições ao servidor com a flag -a
    *  ./dclient -a "title" "authors" "year" "path"
    */
    if(argv[1] == "-a" && argc == 6){
        char request_buffer[1024];
        snprintf(request_buffer, sizeof(request_buffer), "%s %s %s %s %s",argv[1], argv[2], argv[3], argv[4], argv[5]);
        write(fd, request_buffer, strlen(request_buffer));
    }

    /* Este bloco de código é responsável por enviar as requisições ao servidor com a flag -r
    *  ./dclient -r "title"
    */
    else return -1;
}