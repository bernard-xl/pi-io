#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <reactive.h>
#include <utils/check.h>
#include <sys/epoll.h>
io_loop_t loop;
io_handle_t serial;
io_buf_t buf;
char buff[512];

void echo_sent(io_handle_t *handle, io_buf_t *buf, int status) {
    printf("sent: %s\n", buf->base);   
}

void echo_alloc(io_handle_t *handle, io_buf_t *buf) {
    buf->base = buff;
    buf->len = 512;
}

void echo_read(io_handle_t *handle, io_buf_t *buf, int status) {

    if(status == 0 || status == -1) {
        io_serial_close(handle);
        puts("client disconnected.");
        printf("handle count: %i\n", loop.handle_count);
        return;
    }
   
    printf("received: %s\n", (char*)buf->base);
}

void close_all(io_handle_t *handle) {
   io_serial_close(handle);
}

void sigint_handler(int sig) {
   io_loop_foreach(&loop, close_all);
}

void write_string(io_handle_t *handle, char *msg) {
   buf.base = msg;
   buf.len = strlen(msg) + 1;
   io_write(handle, msg, strlen(msg) + 1, echo_sent);
}

int main(int argc, char **argv) {
    struct sigaction act;

    memset(&act, 0, sizeof(act));
    act.sa_handler = sigint_handler;

   printf("EV_READ:\t%d\n", EV_READ);
   printf("EV_WRITE:\t%d\n", EV_WRITE);

    DIE_IF_ERR(io_loop_init(&loop));
    DIE_IF_ERR(io_serial(&loop, &serial, "/dev/ttyACM0", B115200, SERIAL_8N1));
    DIE_IF_ERR(io_read_start(&serial, echo_alloc, echo_read));
    write_string(&serial, "Hello world\n");
    DIE_IF_ERR(sigaction(SIGINT, &act, NULL));
    puts("echo server is up...");
    sleep(3);
    DIE_IF_ERR(io_loop_run(&loop));
    return 0;
}
