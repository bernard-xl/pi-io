#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <reactive.h>
#include <utils/check.h>

io_loop_t loop;
io_handle_t server, client1, client2;
int client_accepted;

void data_spliced(io_handle_t *from, io_handle_t *to, int status) {
   if(status == 0) {
      io_tcp_close(from);
      puts("connection dropped");
      return;
   }

   if(from == &client1)
      puts("splice from client1 to client2");
   else
      puts("splice from client2 to client1");
}

void incoming_connection(io_handle_t *server) {
    char buf[30];
    io_handle_t *client = (client_accepted)? &client1 : &client2; 

    RET_IF_ERR(io_tcp_accept(server, client, buf, 30));

    printf("new connection from %s\n", buf);
    printf("handle count: %i\n", loop.handle_count);

    client_accepted = (client_accepted + 1) % 2;
    if(client == &client1)
      RET_IF_ERR(io_splice_start(&client1, &client2, data_spliced));
    else
      RET_IF_ERR(io_splice_start(&client2, &client1, data_spliced));
}

void close_all(io_handle_t *handle) {
   printf("active handle count: %i closing handle...\n", loop.handle_count);
   io_tcp_close(handle);
}

void sigint_handler(int sig) {
    //io_loop_break(&loop);
    io_loop_foreach(&loop, close_all);
}

int main(int argc, char **argv) {
    struct sigaction act;

    client_accepted = 0;

    memset(&act, 0, sizeof(act));
    act.sa_handler = sigint_handler;

    DIE_IF_ERR(io_loop_init(&loop));
    DIE_IF_ERR(io_tcp_server(&loop, &server, "0.0.0.0", "3000"));
    DIE_IF_ERR(io_listen_start(&server, incoming_connection));
    DIE_IF_ERR(sigaction(SIGINT, &act, NULL));
    puts("splice-test server is up...");
    DIE_IF_ERR(io_loop_run(&loop));
    return 0;
}
