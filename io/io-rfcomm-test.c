#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <reactive.h>
#include <utils/check.h>
#include <sys/epoll.h>
io_loop_t loop;
io_handle_t server;

void echo_sent(io_handle_t *handle, io_buf_t *buf, int status) {
    free(buf->base);
}

void echo_alloc(io_handle_t *handle, io_buf_t *buf) {
    buf->base = malloc(512);
    buf->len = 512;
}

void echo_read(io_handle_t *handle, io_buf_t *buf, int status) {
    if(status == -1) {
        io_rfcomm_close(handle);
        free(handle);
        free(buf->base);
        perror("echo_read");
        return;
    }
   
    buf->len = status;
    io_write(handle, buf->base, status, echo_sent);
}

void incoming_connection(io_handle_t *server) {
    char buf[30];
    io_handle_t *client = (io_handle_t*)calloc(1, sizeof(io_handle_t));

    RET_IF_ERR(io_rfcomm_accept(server, client, buf, 30));

    printf("new connection from %s\n", buf);
    printf("handle count: %i\n", loop.handle_count);

    RET_IF_ERR(io_read_start(client, echo_alloc, echo_read));
}

void close_all(io_handle_t *handle) {
   printf("active handle count: %i closing handle...\n", loop.handle_count);
   io_rfcomm_close(handle);
}

void sigint_handler(int sig) {
    //io_loop_break(&loop);
    io_loop_foreach(&loop, close_all);
}

int main(int argc, char **argv) {
    sdp_session_t *session;
    struct sigaction act;
    static const uint32_t uuid128[] = {0x01110000,0x00100000,0x80000080,0xfb349b5f};
    static const char *name = "MdpGrp5 RaspberryPi";
    static const char *desc = "Coordinate connection between devices.";

    printf("EV_READ:\t%d\n", EV_READ);
    printf("EV_WRITE:\t%d\n", EV_WRITE);

    memset(&act, 0, sizeof(act));
    act.sa_handler = sigint_handler;

    DIE_IF_ERR(io_loop_init(&loop));
    DIE_IF_ERR(io_rfcomm_server(&loop, &server, BDADDR_ANY, 4));
    DIE_IF_ERR(io_listen_start(&server, incoming_connection));
    DIE_IF_ERR(sigaction(SIGINT, &act, NULL));
    puts("echo server is up...");
    session = io_rfcomm_advertise(4, name, desc, uuid128);
    DIE_IF_ERR(io_loop_run(&loop));
    io_rfcomm_unadvertise(session);
    return 0;
}
