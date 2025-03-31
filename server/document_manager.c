#include "document_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>


#define pathToDoc "storage/documents.txt"

int add_document(Document *doc) {
    char filepath[] = pathToDoc;
    int fd = open(filepath, O_RDWR | O_CREAT, 0644);
    if (fd < 0) {
        perror("Error opening file");
        return -1;
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
            return -1;
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


int remove_document(char *title) {
    char filepath[] = pathToDoc;
    int fd = open(filepath, O_RDWR);
    if (fd < 0) {
        perror("Error opening file");
        return -1;
    }
    
    Document temp;
    int read_result;
    int found = 0;
    
    lseek(fd, 0, SEEK_SET);
    
    while ((read_result = read(fd, &temp, sizeof(Document))) > 0) {
        if (strcmp(temp.title, title) == 0) {
            temp.flag_deleted = 1;
            
            memset(temp.title, 0, sizeof(temp.title));

            lseek(fd, -sizeof(Document), SEEK_CUR);
            write(fd, &temp, sizeof(Document));
            
            found = 1;
            break;
        }
    }
    
    close(fd);
    
    if (!found) {
        return -1;
    }
    
    return 0;
}

