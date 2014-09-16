#include <storage.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/resource.h>


#define TEST_SIZE 5000

double get_time()
{
    struct timeval t;
    struct timezone tzp;
    gettimeofday(&t, &tzp);
    return t.tv_sec + t.tv_usec*1e-6;
}


int main(int argc, char **argv) {
   storage_t storage;
   void* allocated1[TEST_SIZE];
   void* allocated2[TEST_SIZE];
   void* allocated3[TEST_SIZE];
   double start, end;
   int i;

   storage_segregate(&storage, TEST_SIZE, 512);

   start = get_time();

   for(i = 0; i < TEST_SIZE; i++) 
      allocated1[i] = storage_alloc(&storage);

   for(i = 0; i < TEST_SIZE; i++)
      storage_free(&storage, allocated1[i]);

   for(i = 0; i < TEST_SIZE; i++)
      allocated2[i] = storage_alloc(&storage);

   for(i = 0; i < TEST_SIZE; i++)
      storage_free(&storage, allocated2[i]);

   for(i = 0; i < TEST_SIZE; i++) 
      allocated3[i] = storage_alloc(&storage);

   end = get_time();
   
   printf("time elapsed: %.4lf\n", end - start);
   return 0;
}

