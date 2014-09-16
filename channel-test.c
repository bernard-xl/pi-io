#include <io/reactive.h>
#include <utils/storage.h>
#include <utils/check.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>

storage_t storage = { 0 };

io_loop_t loop = { 0 };
io_channel_t ch = { 0 };
io_handle_t tcp1 = { -1 };
io_handle_t tcp2 = { -1 };
io_handle_t tcp3 = { -1 };
io_handle_t rfcm = { -1 };
io_handle_t serl = { -1 };
io_handle_t backup = { -1 };

static const uint32_t uuid128[] = {0x01110000,0x00100000,0x80000080,0xfb349b5f};
static const char *name = "MdpGrp5 RaspberryPi";
static const char *desc = "Coordinate connection between devices.";

static const char *ttyACM = "ttyACM";

char addr[128];


void tcp1_spliced(io_handle_t *from, io_handle_t *to, int status) {
   if(status == -1) {
      if(serl.fd == -1) {
         puts("arduino not connected.");
         io_splice_drain(from);
      } else perror("tcp1_splice");
   } else if(status == 0) {
      io_tcp_close(&tcp1);
      tcp1.fd = -1;
      puts("tcp1 disconnected.");
   } else {
      puts("tcp1 sent to arduino.");
   }
}

void backup_spliced(io_handle_t *from, io_handle_t *to, int status) {
   if(status == -1) perror("backup_spliced");
   else printf("%d bytes spliced to backup\n", status);
}

void tcp2_spliced(io_handle_t *from, io_handle_t *to, int status) {
   if(status == -1) {
      if(rfcm.fd == -1) {
          puts("bluetooth not connected.");
          io_splice_drain(from);
      } else perror("tcp2_splice");
   } else if(status == 0) {
      io_tcp_close(&tcp2);
      tcp2.fd = -1;
      puts("tcp2 disconnected.");
   } else {
      puts("tcp2 sent to bluetooth.");
   }
}

void serl_spliced(io_handle_t *from, io_handle_t *to, int status) {
   if(status == -1) {
      if(tcp1.fd == -1) {
          puts("tcp1 not connected.");
          io_splice_drain(from);
      } else perror("serl_splice");
   } else if(status == 0) {
      io_serial_close(&serl);
      serl.fd = -1;
      puts("arduino disconnected.");
   } else {
      puts("arduino sent to tcp1.");
   }
}

void rfcm_alloc(io_handle_t *handle, io_buf_t *buf) {
      buf->base = storage_alloc(&storage);
      buf->len = (buf->base)? 512 : 0;
}

void rfcm_written(io_handle_t *handle, io_buf_t *buf, int status) {
   if(status > 0) {
      storage_free(&storage, buf->base);
      puts("bluetooth sent to tcp1.");
   } else perror("rfcm_written");
}

void rfcm_read(io_handle_t *handle, io_buf_t *buf, int status) {
   if(status == -1) {
      storage_free(&storage, buf->base);
      io_rfcomm_close(&rfcm);
      rfcm.fd = -1;
      puts("bluetooth disconnected.");
   } else {
      if(tcp2.fd != -1) {
	  ((char*)buf->base)[status++] = '\n';
          TELL_IF_ERR(io_write(&tcp2, buf->base, status, rfcm_written));
      } else {
	  puts("tcp2 not connected.");
          storage_free(&storage, buf->base);
      }
   }
}

void new_tcp1_conn(io_handle_t *server) {
   if(tcp1.fd != -1) io_tcp_close(&tcp1);
   RET_IF_ERR(io_tcp_accept(server, &tcp1, addr, 30));
   printf("tcp1 connected: %s\n", addr);
   RET_IF_ERR(io_splice_start(&tcp1, &serl, tcp1_spliced));
}

void new_tcp2_conn(io_handle_t *server) {
   if(tcp2.fd != -1) io_tcp_close(&tcp2);
   RET_IF_ERR(io_tcp_accept(server, &tcp2, addr, 30));
   printf("tcp2 connected: %s\n", addr);
   RET_IF_ERR(io_splice_start(&tcp2, &rfcm, tcp2_spliced));
}

void new_tcp3_conn(io_handle_t *server) {
   if(tcp3.fd != -1) io_tcp_close(&tcp3);
   RET_IF_ERR(io_tcp_accept(server, &tcp3, addr, 30));
   printf("tcp3 connected: %s\n", addr);
   RET_IF_ERR(io_publish_start(&tcp3, &ch));
   io_channel_join(&ch, &tcp1, tcp1_spliced);
   io_channel_join(&ch, &backup, backup_spliced);
}

void new_rfcm_conn(io_handle_t *server) {
   if(rfcm.fd != -1) io_rfcomm_close(&rfcm);
   RET_IF_ERR(io_rfcomm_accept(server, &rfcm, addr, 30));
   printf("rfcm connected: %s\n", addr);
   RET_IF_ERR(io_read_start(&rfcm, rfcm_alloc, rfcm_read));
}

void file_changed(io_handle_t *handle, const char *path, int events) {
   if(strncmp(path, ttyACM, 6) != 0) return;

   if((events & IN_CREATE) && serl.fd == -1) {
      strcpy(addr, "/dev/");
      strcat(addr, path);
      printf("arduino plugged in (%s).\n", path);
      RET_IF_ERR(io_serial(&loop, &serl, addr, B115200, SERIAL_8N1));  
      RET_IF_ERR(io_splice_start(&serl, &tcp1, serl_spliced));
   }
}

void close_all(io_handle_t *handle) {
   printf("active handle count: %i closing handle...\n", loop.handle_count);
   io_tcp_close(handle);
}

void sigint_handler(int sig) {
   io_loop_foreach(&loop, close_all);
}

int try_serial(io_loop_t *loop, io_handle_t *handle) {
   if(io_serial(loop, handle, "/dev/ttyACM0", B115200, SERIAL_8N1) == 0)
      return 0;
   if(io_serial(loop, handle, "/dev/ttyACM1", B115200, SERIAL_8N1) == 0)
      return 0;
   return -1; 
}

int main(int argc, char **argv) {
   struct sigaction act;
   io_handle_t serv1, serv2, serv3, servr, watcher;
   sdp_session_t *session;

   memset(&act, 0, sizeof(act));
   act.sa_handler = sigint_handler;
   
   DIE_IF_ERR(storage_segregate(&storage, 5, 512));

   DIE_IF_ERR(io_loop_init(&loop));
   DIE_IF_ERR(io_inotify(&loop, &watcher));
   DIE_IF_ERR(io_file(&loop, &backup, "log.txt", O_RDWR));
   DIE_IF_ERR(io_tcp_server(&loop, &serv1, "0.0.0.0", "3000"));
   DIE_IF_ERR(io_tcp_server(&loop, &serv2, "0.0.0.0", "4000"));
   DIE_IF_ERR(io_tcp_server(&loop, &serv3, "0.0.0.0", "5000"));
   DIE_IF_ERR(io_rfcomm_server(&loop, &servr, BDADDR_ANY, 4));
   
   DIE_IF_ERR(io_watch_start(&watcher, "/dev", IN_CREATE | IN_DELETE, file_changed));

   if(try_serial(&loop, &serl) == -1) puts("warning: arduino not connected.");
   else DIE_IF_ERR(io_splice_start(&serl, &tcp1, serl_spliced));

   DIE_IF_ERR(io_listen_start(&serv1, new_tcp1_conn));
   DIE_IF_ERR(io_listen_start(&serv2, new_tcp2_conn));
   DIE_IF_ERR(io_listen_start(&serv3, new_tcp3_conn));
   DIE_IF_ERR(io_listen_start(&servr, new_rfcm_conn));

   session = io_rfcomm_advertise(4, name, desc, uuid128);
   DIE_IF_ERR(sigaction(SIGINT, &act, NULL));
   puts("pi server is up...");
   DIE_IF_ERR(io_loop_run(&loop));

   storage_collapse(&storage);
   io_rfcomm_unadvertise(session);
   return 0;
}

