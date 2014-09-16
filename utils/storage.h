#ifndef STORAGE_H
#define STORAGE_H

#include <memory.h>
#include <stdlib.h>

//Segregated Storage.

typedef struct sgg_storage_s {
   void *base;
   size_t len;
   size_t psize;
   void *first;
} storage_t;

int storage_segregate(storage_t *storage, size_t pcount, size_t psize);

void* storage_alloc(storage_t *storage);

int storage_free(storage_t *storage, void *allocated);

int storage_collapse(storage_t *storage);

#endif
