#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include "document_manager.h"
#include "parsing.h"

int parsing(char* argv, Document *doc){
    if(strcmp(argv[1], "-a") == 0){
        strcpy(doc->title, argv[1]);
        strcpy(doc->author, argv[2]);
        doc->year = atoi(argv[4]);
        doc->key = -1;
        doc->flag_deleted = 0;
        return 1;
    }
    if(strcmp(argv[1], "-c") == 0){ // -c 
        doc->key = atoi(argv[2]);
        doc->flag_deleted = 0;
        return 2;
    }
    if(strcmp(argv[1], "-d")){ // -d
        doc->key = atoi(argv[2]);
        doc->flag_deleted = 0;
        return 3;
    }
    else return -1;
}

  //  -a "title" "authors" "year" "path"