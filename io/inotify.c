#include <inotify.h>

int io_inotify(io_loop_t *loop, io_handle_t *handle) {
   int fd;

   if((fd = inotify_init1(IN_NONBLOCK)) == -1)
      return -1;

   return io__handle_init(handle, loop, fd);
}
