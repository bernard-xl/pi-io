#ifndef IO_TCP_H
#define IO_TCP_H

#include <types.h>

int io_tcp_server(io_loop_t *loop, io_handle_t *handle, const char *addr, const char *serv);

int io_tcp_client(io_loop_t *loop, io_handle_t *handle, const char *addr, const char *serv);

int io_tcp_accept(io_handle_t *server, io_handle_t *client, char *addr, int len);

int io_tcp_close(io_handle_t *handle);

#endif
