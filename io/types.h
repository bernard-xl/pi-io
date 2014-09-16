#ifndef IO_TYPES_H
#define IO_TYPES_H

#include <pthread.h>
#include <utils/list.h>

typedef struct io_handle_s io_handle_t;
typedef struct io_loop_s io_loop_t;
typedef struct io_task_s io_task_t;
typedef struct io_req_s io_req_t;


typedef void (*io_task_cb)(io_handle_t *handle, io_task_t *task);

typedef struct list_head list_t;

#define EV_READ             EPOLLIN	    //read event
#define EV_WRITE            EPOLLOUT	    //write event
#define EV_ERR              EPOLLERR	    //error event
#define EV_DOWN             EPOLLRDHUP	    //connection drop event
#define EV_EDGE_TRIG        EPOLLET	    //edge trigger flag

#define IO_HANDLE_FIELDS            \
   int fd;                          \
   io_loop_t *loop;                 \
   int events;                      \
   io_task_t *task;                 \
   io_req_t *req;                   \
   list_t active_handles;           \

struct io_handle_s {
   IO_HANDLE_FIELDS
};

#define IO_LOOP_FIELDS              \
   int fd;                          \
   pthread_t thread;                \
   int running;                     \
   int handle_count;                \
   list_t active_handles;           \
   
struct io_loop_s {
   IO_LOOP_FIELDS
};

#define IO_TASK_FIELDS              \
   io_task_cb progress_cb;          \
   io_task_cb end_cb;               \
   int event;                       \

struct io_task_s {
   IO_TASK_FIELDS
};

#define IO_REQ_FIELDS               \
    int done;                       \
    io_req_t *next;                 \

    
struct io_req_s {
    IO_TASK_FIELDS
    IO_REQ_FIELDS
};

#endif
