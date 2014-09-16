#include <storage.h>
#include <errno.h>

static inline
void* segregate(void *base, size_t pcount, size_t psize) {
   char *iter, *end;
   void *old;

   old = NULL;
   end = (char*)base + (pcount * psize) - psize;

   for(iter = base; iter <= end; iter += psize) {
     *((void**)iter) = old;
     old = iter;
   }
   
   return iter - psize;
}

int storage_segregate(storage_t *storage, size_t pcount, size_t psize) {
   if(psize < sizeof(void*)) {
      errno = EINVAL;
      return -1;
   }
   
   storage->base = malloc(pcount * psize);
   storage->len = pcount * psize;
   storage->psize = psize;
   storage->first = segregate(storage->base, pcount, psize);
   return 0;
}

void* storage_alloc(storage_t *storage) {
   void *ret;
   ret = storage->first;
   if(storage->first) storage->first = *((void**)storage->first);
   return ret;
}

int storage_free(storage_t *storage, void *allocated) {
   if(!allocated) return -1;
   *((void**)allocated) = storage->first;
   storage->first = allocated;
   return 0;
}

int storage_collapse(storage_t *storage) {
   free(storage->base);
}
