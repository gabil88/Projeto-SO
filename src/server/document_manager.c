#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <glib.h>

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

    // Check if already exists by title
    Document temp;
    int read_result;
    lseek(fd, 0, SEEK_SET);
    while ((read_result = read(fd, &temp, sizeof(Document))) > 0) {
        if (strcmp(temp.title, doc->title) == 0 && temp.flag_deleted == 0) {
            printf("Document with the same title already exists\n");
            close(fd);
            return 2;
        }
    }

    // Calculate the offset based on the key
    off_t offset = doc->key * sizeof(Document);
    if (lseek(fd, offset, SEEK_SET) == (off_t)-1) {
        perror("Error seeking to document position");
        close(fd);
        return -1;
    }

    printf("Writing document at position: %ld , with key %d\n", (long)offset, doc->key);

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

    // Calculate the offset based on the key
    off_t offset = key * sizeof(Document);
    if (lseek(fd, offset, SEEK_SET) == (off_t)-1) { // lseek retorna (off_t)-1 em caso de erro ;)
        perror("Error seeking to document position");
        close(fd);
        return -1;
    }

    Document temp;
    ssize_t read_result = read(fd, &temp, sizeof(Document));
    if (read_result != sizeof(Document) || temp.flag_deleted == 1 || temp.key != key) {
        close(fd);
        return -1;
    }

    temp.flag_deleted = 1;
    memset(temp.title, 0, sizeof(temp.title));

    // Move back to the start of this document to overwrite
    if (lseek(fd, offset, SEEK_SET) == (off_t)-1) {
        perror("Error seeking back to document position");
        close(fd);
        return -1;
    }

    if (write(fd, &temp, sizeof(Document)) != sizeof(Document)) {
        perror("Error writing to file");
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

/*
*   Função que consulta um documento no arquivo de documentos persistente
*   Retorna um ponteiro para o documento se ele foi encontrado
*   @returns Retorna NULL se o documento não foi encontrado ou retorna o documento
*/
Document* consult_document(int key){
    int fd = open(pathToDoc, O_RDONLY);
    if (fd < 0) {
        perror("Error opening file");
        return NULL;
    }

    // Calculate the offset based on the key
    off_t offset = key * sizeof(Document);
    if (lseek(fd, offset, SEEK_SET) == (off_t)-1) {
        perror("Error seeking to document position");
        close(fd);
        return NULL;
    }

    Document temp;
    ssize_t read_result = read(fd, &temp, sizeof(Document));
    if (read_result == sizeof(Document) && temp.key == key && temp.flag_deleted == 0) {
        Document *doc = (Document*) malloc(sizeof(Document));
        memcpy(doc, &temp, sizeof(Document));
        close(fd);
        return doc;
    }

    close(fd);
    return NULL;
}

Document* initialize_document(Document *doc){
    doc->key = -1;
    doc->flag_deleted = 0;
    doc->year = 0;
    memset(doc->title, 0, sizeof(doc->title));
    memset(doc->author, 0, sizeof(doc->author));
    memset(doc->path, 0, sizeof(doc->path));
    return doc;
}

int update_document(Document *doc) {
    int fd = open(pathToDoc, O_RDWR);
    if (fd < 0) {
        perror("Error opening file");
        return -1;
    }

    // Calculate the offset based on the key
    printf("Updating document with key: %d\n", doc->key);
    off_t offset = doc->key * sizeof(Document);
    printf("Seeking to offset: %ld\n", (long)offset);
    if (lseek(fd, offset, SEEK_SET) == (off_t)-1) {
        perror("Error seeking to document position");
        close(fd);
        return -1;
    }

    
    Document temp;
    ssize_t read_result = read(fd, &temp, sizeof(Document));
    if (read_result == sizeof(Document) && temp.key == doc->key && temp.flag_deleted == 0) {
        // Update fields
        temp.year = doc->year;
        strncpy(temp.title, doc->title, sizeof(temp.title) - 1);
        temp.title[sizeof(temp.title) - 1] = '\0';
        strncpy(temp.author, doc->author, sizeof(temp.author) - 1);
        temp.author[sizeof(temp.author) - 1] = '\0';
        strncpy(temp.path, doc->path, sizeof(temp.path) - 1);
        temp.path[sizeof(temp.path) - 1] = '\0';

        // Move back to the start of this document to overwrite
        if (lseek(fd, offset, SEEK_SET) == (off_t)-1) {
            perror("Error seeking back to document position");
            close(fd);
            return -1;
        }
        if (write(fd, &temp, sizeof(Document)) != sizeof(Document)) {
            perror("Error writing to file");
            close(fd);
            return -1;
        }
        close(fd);
        return 0;
    }

    close(fd);
    // If not found, add the document
    return add_document(doc);
}

Document* consult_document_by_title(const char* title) {
    int fd = open(pathToDoc, O_RDONLY);
    if (fd < 0) return NULL;

    Document temp;
    while (read(fd, &temp, sizeof(Document)) > 0) {
        if (strcmp(temp.title, title) == 0 && temp.flag_deleted == 0) {
            Document* doc = malloc(sizeof(Document));
            memcpy(doc, &temp, sizeof(Document));
            close(fd);
            return doc;
        }
    }
    close(fd);
    return NULL;
}

int load_deleted_keys(GArray *deleted_keys) {
    int fd = open(pathToDoc, O_RDONLY);
    if (fd < 0) {
        perror("Error opening file");
        return -1;
    }

    Document temp;

    int max_key = -1;

    while (read(fd, &temp, sizeof(Document)) > 0) {
        if (temp.flag_deleted == 1) {
            g_array_append_val(deleted_keys, temp.key);
        }
        else if (temp.key > max_key) {
            max_key = temp.key;
        }
    }
    close(fd);

    return max_key;
}
