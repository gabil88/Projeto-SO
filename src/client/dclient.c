#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include "dclientUtils.h"

/*

faz o parsing esperado na string de entrada
em caso de problemas provavelmente esta no write
*/



int main(int argc, char *argv[]) {
    // Verifica se a quantidade de argumentos está correta
    if(!verifyInput(argc, argv)){
        printf("Invalid input\n");
        return -1;
    }

    // Cria o FIFO
    int fifo_fd;
    if (mkfifo("fifo", 0666) == -1) {
        perror("Error creating FIFO");
        return -1;
    }

    // Abre o FIFO para escrita
    fifo_fd = open("fifo", O_WRONLY);
    if (fifo_fd < 0) {
        perror("Error opening FIFO");
        return -1;
    }

    // Buffer para guardar a request
    char request_buffer[1024] = {0};

    // contatena os argumentos da linha de comando
    for (int i = 1; i < argc; i++) {
        strncat(request_buffer, argv[i], sizeof(request_buffer) - strlen(request_buffer) - 1);
        if (i < argc - 1) {
            strncat(request_buffer, " ", sizeof(request_buffer) - strlen(request_buffer) - 1);
        }
    }

    // Escreve a request no FIFO
    if (write(fifo_fd, request_buffer, strlen(request_buffer)) < 0) {
        perror("Error writing to FIFO");
        close(fifo_fd);
        return -1;
    }
    
    close(fifo_fd);

    //printf("Request: %s\n", request_buffer);

    return 0;
}