#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

#include "../../include/server/document_manager.h"
#include "../../include/server/parsing.h"

char** separateString(char* request) {
    // Alocar memória para o array de ponteiros (até 10 tokens + NULL)
    char** tokens = malloc(11 * sizeof(char*));
    if (tokens == NULL) {
        return NULL;
    }
    
    char* token;
    int i = 0;

    // Primeira chamada ao strtok - isso modifica a string!
    token = strtok(request, " ");
    
    while(token != NULL && i < 10) {
        tokens[i] = token;  // Salvar o ponteiro para o token
        token = strtok(NULL, " ");  // Obter próximo token
        i++;
    }
    tokens[i] = NULL;  // Terminar com NULL
    
    return tokens;
}

int parsing(char* request, Document *doc) {
    if(request == NULL || doc == NULL) {
        return -1;
    }

    // Criar uma cópia da string, já que strtok a modifica
    char* request_copy = strdup(request);
    if (request_copy == NULL) {
        return -1;
    }
    
    // Separar a string em tokens
    char** argv = separateString(request_copy);
    if (argv == NULL) {
        free(request_copy);
        return -1;
    }

    int result = -1;
    
    // Verificar se temos pelo menos um argumento
    if (argv[0] == NULL) {
        free(argv);
        free(request_copy);
        return -1;
    }

    // CORREÇÃO: argv[0] contém o primeiro token, não argv[1]
    if (strcmp(argv[0], "-a") == 0) {
        // Verificar se temos argumentos suficientes
        if (argv[1] != NULL && argv[2] != NULL && argv[3] != NULL) {
            strcpy(doc->title, argv[1]);
            strcpy(doc->author, argv[2]);
            doc->year = atoi(argv[3]);  // Corrigido de argv[4] para argv[3]
            result = 1;
        }
    }
    else if (strcmp(argv[0], "-c") == 0) {
        // Verificar se temos o argumento da chave
        if (argv[1] != NULL) {
            doc->key = atoi(argv[1]);  // Corrigido de argv[2] para argv[1]
            result = 2;
        }
    }
    else if (strcmp(argv[0], "-d") == 0) {  // Corrigido para usar == em vez de apenas )
        // Verificar se temos o argumento da chave
        if (argv[1] != NULL) {
            doc->key = atoi(argv[1]);  // Corrigido de argv[2] para argv[1]
            result = 3;
        }
    }

    // Liberar a memória alocad
    free(argv);
    free(request_copy);
    
    return result;
}