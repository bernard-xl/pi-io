#ifndef IO_LOOP_H
#define IO_LOOP_H

#include <types.h>

#define EPOLL_MAXEVENT	5

typedef void (*io_loop_walk)(io_handle_t *handle);

int io_loop_init(io_loop_t *loop);

int io_loop_run(io_loop_t *loop);

int io_loop_break(io_loop_t *loop);

int io_loop_close(io_loop_t *loop);

int io_loop_foreach(io_loop_t *loop, io_loop_walk walk);

#endif
