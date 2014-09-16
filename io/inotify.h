#ifndef IO_INOTIFY_H
#define IO_INOTIFY_H

#include <handle.h>
#include <sys/inotify.h>

int io_inotify(io_loop_t *loop, io_handle_t *handle);

#endif
