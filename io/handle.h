#ifndef IO_HANDLE_H
#define IO_HANDLE_H

#include <types.h>

int io__handle_init(io_handle_t *handle, io_loop_t *loop, int fd);

int io__handle_start(io_handle_t *handle, io_task_t *task);

int io__handle_stop(io_handle_t *handle);

int io__handle_req(io_handle_t *handle, io_req_t *req);

int io__handle_done(io_handle_t *handle);

int io__handle_giveup(io_handle_t *handle);

int io__handle_close(io_handle_t *handle);

#endif
