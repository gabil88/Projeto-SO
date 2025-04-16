#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include "../../include/server/document_manager.h"

#define pathToDoc "../../storage/documents.txt"


/* 
*   Função que adiciona um documento ao arquivo de documentos persistente
*   Retorna 0 se o documento foi adicionado com sucesso
*   Retorna -1 se houve um erro ao adicionar o documento
*   Retorna -2 se o documento já existe
*/

int add_document(Document *doc) {

    // Ensure the storage directory exists
    if (mkdir("../../storage", 0755) == -1 && errno != EEXIST) {
        perror("Error ensuring storage directory");
        return -1;
    }

    // Open or create the file
    int fd = open(pathToDoc, O_RDWR | O_CREAT, 0644);
    if (fd < 0) {
        perror("Error opening/creating file");
        return -1;
    }

    printf("File opened successfully\n");

    printf("Document title: %s , document key: %d\n", doc->title, doc->key);

    // Check if the file is empty
    off_t file_size = lseek(fd, 0, SEEK_END);
    if (file_size == 0) {
        printf("File is empty, writing first document\n");
        if (write(fd, doc, sizeof(Document)) != sizeof(Document)) {
            perror("Error writing to file");
            close(fd);
            return -1;
        }
        close(fd);
        return 0; // Success
    }
    
    Document temp;
    int read_result;
    off_t deleted_pos = -1;
    
    lseek(fd, 0, SEEK_SET);
    while ((read_result = read(fd, &temp, sizeof(Document))) > 0) {
        if (temp.flag_deleted == 1 && deleted_pos == -1) {
            // Record the position of the first deleted document
            off_t current_pos = lseek(fd, 0, SEEK_CUR);
            if (current_pos == -1) {
                perror("Error getting current position");
                close(fd);
                return -3; // Error: lseek failed
            }
            deleted_pos = current_pos - sizeof(Document);
        } else if (strcmp(temp.title, doc->title) == 0 && temp.flag_deleted == 0) {
            // Document with the same title already exists
            printf("Document with the same title already exists\n");
            close(fd);
            return -2;
        }
    }
    
    printf("End of file reached, first deleted posistion: %ld\n", deleted_pos);
    

    if (deleted_pos != -1) {
        lseek(fd, deleted_pos, SEEK_SET);
    } else {
        lseek(fd, 0, SEEK_END);
    }
    
    printf("Writing document at position: %ld , with key %d\n", lseek(fd, 0, SEEK_CUR), doc->key);

    ssize_t written = write(fd, doc, sizeof(Document));
    if (written < 0) {
        perror("Error writing to file");
        close(fd);
        return -1;
    }
    
    if (written != sizeof(Document)) {
        fprintf(stderr, "Partial write occurred\n");
        close(fd);
        return -1;
    }
    
    close(fd);
    
    printf("Document added successfully\n");

    return 0;
}


/*
*   Função que remove um documento do arquivo de documentos persistente
*   Retorna 0 se o documento foi removido com sucesso
*   Retorna -1 se houve um erro ao remover o documento
*/
int remove_document(int key) {
    int fd = open(pathToDoc, O_RDWR);
    if (fd < 0) {
        perror("Error opening file");
        return -1;
    }
    
    Document temp;
    int read_result;
    int found = 0;
    
    lseek(fd, 0, SEEK_SET);
    
    while ((read_result = read(fd, &temp, sizeof(Document))) > 0 && !found) {
        if (temp.key == key && temp.flag_deleted == 0) {
            temp.flag_deleted = 1;
            
            memset(temp.title, 0, sizeof(temp.title));

            lseek(fd, -sizeof(Document), SEEK_CUR);
            write(fd, &temp, sizeof(Document));
            
            found = 1;
        }
    }
    
    close(fd);
    
    if (!found) {
        return -1;
    }
    
    return 0;
}

/*
*   Função que consulta um documento no arquivo de documentos persistente
*   Retorna um ponteiro para o documento se ele foi encontrado
*   Retorna NULL se o documento não foi encontrado
*/

Document* consult_document(int key){
    int fd = open(pathToDoc, O_RDONLY);
    if (fd < 0) {
        perror("Error opening file");
        return NULL;  // Return NULL, not -1 (function returns Document*)
    }

    Document temp;
    int read_result;

    lseek(fd, 0, SEEK_SET);
    while ((read_result = read(fd, &temp, sizeof(Document))) > 0) {
        if (temp.key == key && temp.flag_deleted == 0) {            
            Document *doc = (Document*) malloc(sizeof(Document));
            memcpy(doc, &temp, sizeof(Document));
            close(fd);
            return doc;
        }
    }
    
    close(fd);
    return NULL;  // Document not found
}

Document* initialize_document(Document *doc, int count){
    doc->key = count;
    doc->flag_deleted = 0;
    doc->year = 0;
    memset(doc->title, 0, sizeof(doc->title));
    memset(doc->author, 0, sizeof(doc->author));
    memset(doc->path, 0, sizeof(doc->path));
    return doc;
}

int generate_unique_key() {
    // Abre o arquivo que armazena a última chave usada
    int key_fd = open("../../storage/next_key.dat", O_RDWR | O_CREAT, 0644);
    if (key_fd < 0) {
        perror("Error opening key file");
        return rand() % 10000;  // Fallback: usar um número aleatório
    }

    // Lê a última chave usada
    int next_key = 1;  // Valor padrão se o arquivo estiver vazio
    char buf[16] = {0};
    ssize_t bytes_read = read(key_fd, buf, sizeof(buf) - 1);
    
    if (bytes_read > 0) {
        next_key = atoi(buf) + 1;
    }

    // Escreve a próxima chave no arquivo
    lseek(key_fd, 0, SEEK_SET);
    snprintf(buf, sizeof(buf), "%d", next_key);
    write(key_fd, buf, strlen(buf));

    close(key_fd);

    printf("Generated new unique key: %d\n", next_key);
    return next_key;
}