#ifndef __CACHE_H__
#define __CACHE_H__
#include <pthread.h>

#define MAX_CACHE_SIZE 36
typedef struct _cache_data_t 
{
    char *identify_str;
    char *data;
    unsigned int identify_str_size;
    unsigned int data_size;
}cache_data_t;

typedef struct _cache_t
{
    cache_data_t cache_data[MAX_CACHE_SIZE];
    pthread_rwlock_t rwlock;
}cache_t;

void cache_init(cache_t *pcache);
const cache_data_t *cache_get_data(cache_t *pcache, const char *identify_str);
void cache_data_free(cache_data_t *cache_data);
void cache_insert(cache_t *pcache, const char *identify_str, unsigned int identify_str_size, const char *data, unsigned int data_size);
void cache_deinit(cache_t *pcache);
unsigned int BKDRHash(const char *_str);

#endif