#ifndef CHECK_H
#define CHECK_H

#include <stdlib.h>
#include <stdio.h>

#define DIE_IF_ERR(x)                          \
   do {                                        \
      if((x) == -1) {                          \
         perror(#x);                           \
         exit(-1);                             \
      }                                        \
   } while(0)

#define RET_IF_ERR(x)                          \
   do {                                        \
      if((x) == -1) {                          \
         perror(#x);                           \
         return;                               \
      }                                        \
   } while(0)

#define TELL_IF_ERR(x)                         \
   do {                                        \
      if((x) == -1) {                          \
         perror(#x);                           \
      }                                        \
   } while(0)                                  

#endif
