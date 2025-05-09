#ifndef CACHE_H_
#define CACHE_H_

#include "document_manager.h"
#include  <time.h>

#define CACHE_SIZE 10

typedef struct {
    Document* doc;
    time_t last_access_time; 
    int access_count; 
    int is_dirty;
} CacheItem;


typedef struct {
    CacheItem items[CACHE_SIZE];
    int count;
    // USADO PARA CALCULAR HIT RATE FUTURAMENTE PARA PERCEBER SE ISTO TA PICA
    int misses;
    int hits;
} Cache;

/*
@brief
 * @brief Initializes the cache system.
 *
 * This function allocates memory for the cache and initializes its fields.
 * It should be called before using any other cache functions.
 */
Cache* cache_init();

/*
 * @brief Cleans up the cache system.
 *
 * This function frees the memory allocated for the cache and its items.
 * It should be called when the cache is no longer needed.
 */
void cache_clean(Cache* cache);

/*
 * @brief Adds a document to the cache.
 *
 * This function adds a document to the cache. If the cache is full,
 * it replaces the least recently used item.
 *
 * @param cache A pointer to the cache.
 * @param doc A pointer to the document to be added.
 * @return 0 on success, -1 on failure.
 */
int cache_add(Cache* cache, Document *doc, int skip_check);

/*
* @brief Removes a document from the cache.
*
* This function removes a document from the cache based on its key.
*
* @param cache A pointer to the cache.
* @param key The key of the document to be removed.
* @return 0 on success, -1 on failure.
*/
int cache_remove(Cache* cache, int key);


/*
* @brief Consults a document in the cache.
*
* This function retrieves a document from the cache based on its key.
*
* @param cache A pointer to the cache.
* @param key The key of the document to be consulted.
* @return A pointer to the document if found, NULL otherwise.
*/
Document* cache_get(Cache* cache, int key);

/*
 * @brief Flushes the cache.
 *
 * This function flushes the cache, updating all dirty items.
 *
 * @param cache A pointer to the cache.
 * @return 0 on success, -1 on failure.
 */
int cache_flush_all_dirty(Cache* cache); 


int cache_update_time(Cache* cache,int key);

#endif 