#ifndef DOCUMENT_MANAGER_H_
#define DOCUMENT_MANAGER_H_

typedef struct {
    char title[100];
    char author[100];
    int year;
    char path[100];
    int flag_deleted;
} Document;

#endif
