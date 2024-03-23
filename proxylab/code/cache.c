#include <string.h>
#include <stdlib.h>
#include "cache.h"

unsigned int BKDRHash(const char *_str)
{
    unsigned int seed = 131; // 31 131 1313 13131 131313 etc..
    unsigned int hash = 0;
    char *str = (char *)_str;
    
    while (*str)
    {
        hash = hash * seed + (*str++);
    }
    
    return (hash & 0x7FFFFFFF);
}

void cache_init(cache_t *pcache) {
    for (int i = 0; i != MAX_CACHE_SIZE; i++) {
        pcache->cache_data[i].identify_str = NULL;
        pcache->cache_data[i].data = NULL;
    }
    pthread_rwlock_init(&pcache->rwlock, NULL);
}

void cache_deinit(cache_t *pcache) {
    for (int i = 0; i != MAX_CACHE_SIZE; i++) {
        if (pcache->cache_data[i].identify_str != NULL) {
            cache_data_free(&pcache->cache_data[i]);
        }
    }
    pthread_rwlock_destroy(&pcache->rwlock);
}

void cache_data_free(cache_data_t *cache_data) {
    free(cache_data->data);
    free(cache_data->identify_str);
    cache_data->data = NULL;
    cache_data->identify_str = NULL;
}

const cache_data_t *cache_get_data(cache_t *pcache, const char *identify_str) {
    pthread_rwlock_rdlock(&pcache->rwlock);
    cache_data_t *ret = NULL;
    unsigned int hash_code = BKDRHash(identify_str);
    hash_code = hash_code % MAX_CACHE_SIZE;

    if (pcache->cache_data[hash_code].identify_str == NULL ){
        pthread_rwlock_unlock(&pcache->rwlock);
        return NULL;
    }
    
    if (strcmp(pcache->cache_data[hash_code].identify_str, identify_str) == 0) {
        ret = &pcache->cache_data[hash_code];
    }
    else {
        ret = NULL;
    }
    pthread_rwlock_unlock(&pcache->rwlock);
    return ret;
}


// size should be the byte size
void cache_insert(cache_t *pcache, const char *identify_str, unsigned int identify_str_size, const char *data, unsigned int data_size) {
    pthread_rwlock_wrlock(&pcache->rwlock);
    if (identify_str == NULL || data == NULL) {
        pthread_rwlock_unlock(&pcache->rwlock);
        return;
    }
    unsigned int hash_code = BKDRHash(identify_str);
    hash_code = hash_code % MAX_CACHE_SIZE;
    char *new_identify_str = (char *) malloc(identify_str_size);
    char *new_data = (char *) malloc(data_size);

    if (pcache->cache_data[hash_code].identify_str != NULL) {
        cache_data_free(&pcache->cache_data[hash_code]);
    }

    memcpy(new_identify_str, identify_str, identify_str_size * sizeof(char));
    pcache->cache_data[hash_code].identify_str = new_identify_str;
    pcache->cache_data[hash_code].identify_str_size = identify_str_size;
    
    memcpy(new_data, data, data_size);
    pcache->cache_data[hash_code].data = new_data;
    pcache->cache_data[hash_code].data_size = data_size;

    pthread_rwlock_unlock(&pcache->rwlock);
}