#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "../../include/server/cache.h"

Cache* cache_init(){
    Cache* cache = malloc(sizeof(Cache));
    if (cache == NULL) {
        perror("Error allocating memory for cache");
        return NULL;
    }
    // Initialize the cache system
    for (int i = 0; i < CACHE_SIZE; i++) {
        cache->items[i].doc = NULL;
        cache->items[i].last_access_time = 0;
        cache->items[i].access_count = 0;
        cache->items[i].is_dirty = 0;
    }
    cache->count = 0;
    cache->misses = 0;
    cache->hits = 0;

    return cache;
}

void cache_clean(Cache* cache) {
    for (int i = 0; i < CACHE_SIZE; i++) {
        if (cache->items[i].doc != NULL) {
            free(cache->items[i].doc);
        }
    }
    free(cache);
}

int cache_add(Cache* cache, Document *doc, int skip_check) {
    if (!cache || !doc) return -1;  
    
    // Verifica se o documento já existe no cache
    for (int i = 0; i < CACHE_SIZE; i++) {
        printf("Cache item %d: %s\n", i, cache->items[i].doc ? cache->items[i].doc->title : "NULL");
        if (cache->items[i].doc != NULL && strcmp(cache->items[i].doc->title, doc->title) == 0) {
            printf("Document already exists in cache: %s\n", doc->title);
            cache->items[i].last_access_time = time(NULL);
            fflush(stdout);
            return 2;   // Documento já existe no cache
        }
    }

    if(!skip_check){
        if(consult_document_by_title(doc->title) != NULL){
            printf("Document already exists in storage: %s\n", doc->title);
            return 2; // Documento já existe no armazenamento
        }
    }

    // Always try to find an empty slot first
    for (int i = 0; i < CACHE_SIZE; i++) {
        if (cache->items[i].doc == NULL) {
            cache->items[i].doc = doc;
            cache->items[i].last_access_time = time(NULL);
            cache->items[i].access_count = 1;
            cache->items[i].is_dirty = 1;  // Mark as dirty (new item)
            cache->count++;
            return 0; 
        }
    }

    // If no empty slot, replace the LRU item
    int lru_index = 0;
    time_t oldest_time = cache->items[0].last_access_time;

    // procura o item usado ha mais tempo
    for (int i = 1; i < CACHE_SIZE; i++) {
        if (cache->items[i].last_access_time < oldest_time) {
            oldest_time = cache->items[i].last_access_time;
            lru_index = i;
        }
    }

    // Se o item for dirty, precisamos salvá-lo em disco antes de removê-lo
    if (cache->items[lru_index].is_dirty && cache->items[lru_index].doc != NULL) {
        int state = update_document(cache->items[lru_index].doc);
        if (state == -1) {
            printf("Error updating document in cache\n");
            return -1;  // Erro ao atualizar o documento
        }
        else printf("Document updated in cache\n");
    }
    
    // Replace the LRU item
    if (cache->items[lru_index].doc != NULL) {
        free(cache->items[lru_index].doc);
    }
    cache->items[lru_index].doc = doc;
    cache->items[lru_index].last_access_time = time(NULL);
    cache->items[lru_index].access_count = 1;
    cache->items[lru_index].is_dirty = 1;  // Novo item, marca como dirty
    // cache->count does not change here, since we're replacing
    return 0;  // adicionado com sucesso
}

int cache_remove(Cache* cache, int key){
    if (!cache) return -1; 

    for (int i = 0; i < CACHE_SIZE; i++) {
        if (cache->items[i].doc != NULL && cache->items[i].doc->key == key) {
            free(cache->items[i].doc);
            cache->items[i].doc = NULL;
            cache->items[i].last_access_time = 0;
            cache->items[i].access_count = 0;
            cache->items[i].is_dirty = 0;
            cache->count--;
            int res = remove_document(key);
            if (res == 0) {
                return 1; // removed from cache and disk
            } else {
                return -1; // failed to remove from disk
            }
        }
    }

    return remove_document(key); // Not in cache, try to remove from disk if 0 good if -1 bad
}

Document* cache_get(Cache* cache, int key){
    if (!cache) return NULL;  

    for (int i = 0; i < CACHE_SIZE; i++) {
        if (cache->items[i].doc != NULL && cache->items[i].doc->key == key) {
            cache->items[i].last_access_time = time(NULL);
            cache->items[i].access_count++;
            cache->hits++;
            return cache->items[i].doc;
        }
    }
    return NULL;
}

int cache_update_time(Cache* cache,int key){
    if (!cache) return -1;  

    for (int i = 0; i < CACHE_SIZE; i++) {
        if (cache->items[i].doc != NULL && cache->items[i].doc->key == key) {
            cache->items[i].last_access_time = time(NULL);
            cache->items[i].access_count++;
            return 0;  // atualizado com sucesso
        }
    }
    return -1;  // não encontrado
}

int cache_flush_all_dirty(Cache* cache) {
    if (!cache) return -1;  

    for (int i = 0; i < CACHE_SIZE; i++) {
        if (cache->items[i].doc != NULL && cache->items[i].is_dirty) {
            int state = update_document(cache->items[i].doc);
            if (state == -1) {
                printf("Error updating document in cache\n");
                return -1; 
            }
            else printf("Document updated in cache\n");
        }
    }
    return 0;  // sucesso
}

