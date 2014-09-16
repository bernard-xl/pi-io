#include <serial.h>
#include <handle.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int io_serial(io_loop_t *loop, io_handle_t *handle, const char *path, int baudrate, int config) {
   struct termios toptions;

   int fd = open(path, O_RDWR | O_NOCTTY | O_NONBLOCK);
   if(fd == -1) return -1;

   if(tcgetattr(fd, &toptions) == -1) goto serial_fail;

   if(cfsetispeed(&toptions, baudrate) == -1) goto serial_fail;
   if(cfsetispeed(&toptions, baudrate) == -1) goto serial_fail;

   toptions.c_cflag &= ~(PARENB | CSIZE | CSTOPB | PARODD);
   toptions.c_cflag |= config;

#ifdef CRTSCTS
   toptions.c_cflag &= ~CRTSCTS;
#endif

   toptions.c_cflag |= CREAD | CLOCAL;
   toptions.c_iflag &= ~(IXON | IXOFF | IXANY);

   toptions.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
   toptions.c_oflag &= ~OPOST;

   if(tcsetattr(fd, TCSANOW, &toptions) == -1) goto serial_fail;

   if(io__handle_init(handle, loop, fd) == -1) goto serial_fail;
   return 0;

serial_fail:
   close(fd);
   return -1;
}

int io_serial_close(io_handle_t *handle) {
   io__handle_close(handle);
   close(handle->fd);
   return 0;
}
