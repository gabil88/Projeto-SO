#include "document_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>


#define pathToDoc "../../storage/documents.txt"


/* 
*   Função que adiciona um documento ao arquivo de documentos persistente
*   Retorna 0 se o documento foi adicionado com sucesso
*   Retorna -1 se houve um erro ao adicionar o documento
*   Retorna -2 se o documento já existe
*/

int add_document(Document *doc) {
    int fd = open(pathToDoc, O_RDWR | O_CREAT, 0644);
    if (fd < 0) {
        perror("Error opening file");
        return -1;  // Add return statement here
    }
    
    Document temp;
    int read_result;
    off_t deleted_pos = -1;
    
    lseek(fd, 0, SEEK_SET);
    while ((read_result = read(fd, &temp, sizeof(Document))) > 0) {
        if (temp.flag_deleted == 1 && deleted_pos == -1) {
            deleted_pos = lseek(fd, 0, SEEK_CUR) - sizeof(Document);
        } else if (strcmp(temp.title, doc->title) == 0 && temp.flag_deleted == 0) {
            close(fd);
            return -2;
        }
    }
    
    

    if (deleted_pos != -1) {
        lseek(fd, deleted_pos, SEEK_SET);
    } else {
        lseek(fd, 0, SEEK_END);
    }
    
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


/*
int DebugMemoriaPersistente() {
    // Create a test document
    Document test_doc;
    test_doc.key = 1;
    strcpy(test_doc.title, "Test Document");
    test_doc.flag_deleted = 0;
    
    // Try to add the document
    printf("Attempting to add test document...\n");
    int add_result = add_document(&test_doc);
    printf("Add result: %d\n", add_result);
    
    // Debug info to check file and storage
    printf("\n--- Debug Information ---\n");
    
    // Check if file exists
    int check_fd = open(pathToDoc, O_RDONLY);
    if (check_fd < 0) {
        perror("Debug - File doesn't exist");
    } else {
        printf("Debug - File exists at: %s\n", pathToDoc);
        
        // Get file size
        struct stat file_stat;
        if (fstat(check_fd, &file_stat) == 0) {
            printf("File size: %ld bytes\n", file_stat.st_size);
            printf("Expected Document size: %ld bytes\n", sizeof(Document));
        } else {
            perror("Failed to get file stats");
        }
        
        // Check file contents
        Document debug_doc;
        int debug_read;
        lseek(check_fd, 0, SEEK_SET);
        printf("Debug - File contents:\n");
        int record_count = 0;
        while ((debug_read = read(check_fd, &debug_doc, sizeof(Document))) > 0) {
            printf("  Record %d: Key: %d, Title: %s, Deleted: %d\n", 
                   ++record_count, debug_doc.key, debug_doc.title, debug_doc.flag_deleted);
        }
        
        printf("================================================\n");

        Document *found_doc = consult_document(1);
        if (found_doc != NULL) {
            printf("  Found document: Key: %d, Title: %s, Deleted: %d\n", 
                   found_doc->key, found_doc->title, found_doc->flag_deleted);
            free(found_doc);
        } else {
            printf("  Document not found.\n");
        }
        
        printf("================================================\n");

        int remove_result = remove_document(1);
        printf("Remove result: %d\n", remove_result);

        printf("================================================\n");

        Document *found_doc1 = consult_document(1);
        if (found_doc1 != NULL) {
            printf("  Found document: Key: %d, Title: %s, Deleted: %d\n", 
                   found_doc1->key, found_doc1->title, found_doc1->flag_deleted);
            free(found_doc1);
        } else {
            printf("  Document not found.\n");
        }

        printf("================================================\n");


        if (record_count == 0) {
            printf("  No records found in file.\n");
        }
        
        close(check_fd);
    }
    
    return 0;
}
*/