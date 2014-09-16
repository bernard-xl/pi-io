#ifndef IO_RFCOMM_H
#define IO_RFCOMM_H

#include <types.h>
#include <handle.h>
#include <stdint.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>

int io_rfcomm_server(io_loop_t *loop, io_handle_t *handle, bdaddr_t *addr, uint8_t ch);

sdp_session_t* io_rfcomm_advertise(uint8_t ch, const char *name, const char *desc, const uint32_t *uuid128);

int io_rfcomm_unadvertise(sdp_session_t *session);

int io_rfcomm_accept(io_handle_t *server, io_handle_t *client, char *addr, int len);

int io_rfcomm_close(io_handle_t *handle);

#endif
