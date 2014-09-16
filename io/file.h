#ifndef IO_FILE
#define IO_FILE

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <types.h>

int io_file(io_loop_t *loop, io_handle_t *handle, const char *path, int flags);

int io_file_close(io_handle_t *handle);

#endif
