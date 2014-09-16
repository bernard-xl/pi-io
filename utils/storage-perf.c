#include <stdlib.h>
#include <time.h>
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
   void* allocated1[TEST_SIZE];
   void* allocated2[TEST_SIZE];
   void* allocated3[TEST_SIZE];
   double start, end;
   int i;

   start = get_time();

   for(i = 0; i < TEST_SIZE; i++) 
      allocated1[i] =malloc(512);

   for(i = 0; i < TEST_SIZE; i++)
      free(allocated1[i]);

   for(i = 0; i < TEST_SIZE; i++)
      allocated2[i] = malloc(512);

   for(i = 0; i < TEST_SIZE; i++)
      free(allocated2[i]);

   for(i = 0; i < TEST_SIZE; i++) 
      allocated3[i] = malloc(512);

   end = get_time();
   printf("time elapsed: %.4lf\n", end - start);
   return 0;
}
