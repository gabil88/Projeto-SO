#ifndef DOCUMENT_MANAGER_H_
#define DOCUMENT_MANAGER_H_

typedef struct {
    char title[100];
    char author[100];
    int year;
    int key;
    char path[100];
    short int flag_deleted;

} Document;

#endif
