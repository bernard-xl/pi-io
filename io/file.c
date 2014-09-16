#include <file.h>
#include <unistd.h>
#include <fcntl.h>

int io_file(io_loop_t *loop, io_handle_t *handle, const char *path, int flags) {
   int fd = open(path, flags | O_NONBLOCK | O_CREAT, 0777);
   if(fd == -1) return -1;

   if(io__handle_init(handle, loop, fd) == -1) {
      close(fd);
      return -1;
   }

   return 0;
}

int io_file_close(io_handle_t *handle) {
   io__handle_close(handle);
   close(handle->fd);
   return 0;
}
