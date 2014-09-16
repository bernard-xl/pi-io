#ifndef IO_SERIAL_H
#define IO_SERIAL_H

#include <types.h>
#include <termios.h>

#define SERIAL_8N1   CS8
#define SERIAL_7N1   CS7

#define SERIAL_8E1   CS8 | PARENB
#define SERIAL_7E1   CS7 | PARENB

#define SERIAL_8O1   CS8 | PARENB | PARODD
#define SERIAL_7O1   CS7 | PARENB | PARODD

int io_serial(io_loop_t *loop, io_handle_t *handle, const char *path, int baudrate, int config);

int io_serial_close(io_handle_t *handle);

#endif
