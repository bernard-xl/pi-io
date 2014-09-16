#define _GNU_SOURCE
#include <operation.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/inotify.h>

#define SPLICE_SIZE     512
#define SPLICE_FLAGS    (SPLICE_F_MOVE | SPLICE_F_MORE | SPLICE_F_NONBLOCK)

#define new_task(name) ((name*)calloc(1, sizeof(name)))

static void read_progress(io_handle_t *handle, io_task_t *task) {
    io_buf_t buf = { NULL, 0 };
    io_read_t *readt = (io_read_t*)task;
    (*readt->alloc_cb)(handle, &buf);

    if(buf.len != 0) {
        int n = read(handle->fd, buf.base, buf.len);
        (*readt->read_cb)(handle, &buf, n);
    }
}

static void write_progress(io_handle_t *handle, io_task_t *task) {
   int n, len;
   char *base;
   io_write_t *writet = (io_write_t*)task;
   io_buf_t *buf = &writet->buf;
   
   base = ((char*)buf->base) + writet->progress;
   len = buf->len - writet->progress;

   n = write(handle->fd, base, len);

   if((n <= 0 && errno != EWOULDBLOCK) || n == len) {
      writet->done = 1;
      if(writet->write_cb) (*writet->write_cb)(handle, buf, buf->len);
      return;
   }

   writet->progress += n;
}

static void send_progress(io_handle_t *handle, io_task_t *task) {
   io_send_t *sendt = (io_send_t*)task;
   io_handle_t *from = sendt->from;
   int len = sendt->len - sendt->offset;
   ssize_t n = sendfile(handle->fd, from->fd, sendt->offset, len);
   
   if((n <= 0 && errno != EWOULDBLOCK) || n == len) {
      sendt->done = 1;
      if(sendt->send_cb) (*sendt->send_cb)(from, handle, sendt->len);
      return;  
   }

   sendt->offset += n;
}

static void listen_progress(io_handle_t *handle, io_task_t *task) {
    io_listen_t *listent = (io_listen_t*)task;
    (*listent->listen_cb)(handle);
}

static void splice_progress(io_handle_t *handle, io_task_t *task) {
    int n;
    io_splice_t *splicet = (io_splice_t*)task;

    n = splice(handle->fd, NULL, splicet->pipe[1], NULL, SPLICE_SIZE, SPLICE_FLAGS);
    if(n == -1) goto splice_return;

    n = splice(splicet->pipe[0], NULL, splicet->to->fd, NULL, n, SPLICE_FLAGS);
    if(n == -1) goto splice_return;

splice_return:
    (*splicet->splice_cb)(handle, splicet->to, n);
}

static void splice_end(io_handle_t *handle, io_task_t *task) {
    io_splice_t *splicet = (io_splice_t*)task;
    close(splicet->pipe[0]);
    close(splicet->pipe[1]);
}

static void watch_progress(io_handle_t *handle, io_task_t *task) {
   #define bufsize sizeof(struct inotify_event) + 512
   static char wbuf[bufsize];
   int n;
   int i = 0;
   io_watch_t *watcht = (io_watch_t*)task;
      
   if((n = read(handle->fd, wbuf, bufsize)) != -1) {
      while(i < n) {
         struct inotify_event *ev = (struct inotify_event*)&wbuf[i];
         (*watcht->watch_cb)(handle, ev->name, ev->mask);
         i += sizeof(struct inotify_event) + ev->len;
      }
   }
   #undef bufsize
}

static void publish_progress(io_handle_t *handle, io_task_t *task) {
   int n, nn;
   list_t *iter;
   io_publish_t *pubt = (io_publish_t*)task;
   io_channel_t *ch = pubt->channel;
      
   n = splice(handle->fd, NULL, pubt->pipe[1], NULL, SPLICE_SIZE, SPLICE_FLAGS);
   if(n == -1) return;
   
   list_for_each(iter, &ch->subscribers) {
      io__sub_t *sub = list_entry(iter, io__sub_t, subscribers);
      nn = tee(pubt->pipe[0],sub->pipe[1], n, SPLICE_FLAGS);
      if(nn == -1) goto sub_cb;
      nn = splice(sub->pipe[0], NULL, sub->handle->fd, NULL, n, SPLICE_FLAGS);
      sub_cb: (*sub->channel_cb)(ch, sub->handle, nn);
   }
   if(pubt->pub_cb) (*pubt->pub_cb)(handle, n);
   splice(pubt->pipe[0], NULL, pubt->nullfd, NULL, n, SPLICE_FLAGS);
}

static void publish_end(io_handle_t *handle, io_task_t *task) {
   io_publish_t *pubt = (io_publish_t*)task;

   close(pubt->pipe[0]);
   close(pubt->pipe[1]);
   close(pubt->nullfd);
}

int io_read_start(io_handle_t *handle, io_alloc_cb alloc_cb, io_read_cb read_cb) {
    io_read_t *readt = new_task(io_read_t);
    readt->progress_cb = read_progress;
    readt->alloc_cb = alloc_cb;
    readt->read_cb = read_cb;
    readt->event = EV_READ;

    if(io__handle_start(handle, readt) == -1) {
        free(readt);
        return -1;
    }
    return 0;
}

int io_read_stop(io_handle_t *handle) {
    return io__handle_stop(handle);
}

int io_write(io_handle_t *handle, void *data, ssize_t len, io_write_cb write_cb) {
    int n = write(handle->fd, data, len);
    if(n <= 0 || n < len) {
        io_write_t *writet = new_task(io_write_t);
        writet->progress_cb = write_progress;
        writet->write_cb = write_cb;
        writet->buf.base = data;
        writet->buf.len = len;
        writet->progress = (n == -1)? 0 : n;
        writet->event = EV_WRITE;

        if(io__handle_req(handle, writet) == -1) {
            free(writet);
            return -1;
        }
        return 0;
    }

    if(write_cb) {
        io_buf_t buf = { data, len };
        (*write_cb)(handle, &buf, n);
    }
    return 0;
}

int io_sendall(io_handle_t *from, io_handle_t *to, io_send_cb send_cb) {
   off_t off = 0;
   size_t len = lseek(from->fd, 0, SEEK_END);
   ssize_t n = sendfile(to->fd, from->fd, &off, len);

   if(n <= 0 || n < len) {
      io_send_t *sendt = new_task(io_send_t);
      sendt->progress_cb = send_progress;
      sendt->send_cb = send_cb;
      sendt->offset = (n == -1)? 0 : n; 
      sendt->from = from;
      sendt->len = len;
      sendt->event = EV_WRITE;

      if(io__handle_req(to, sendt) == -1) {
         free(sendt);
         return -1;
      }
      return 0;
   }

   if(send_cb) (*send_cb)(from, to, n);
}

int io_listen_start(io_handle_t *handle, io_listen_cb listen_cb) {
    io_listen_t *listent = new_task(io_listen_t);
    listent->progress_cb = listen_progress;
    listent->listen_cb = listen_cb;
    listent->event = EV_READ;
    
    if(io__handle_start(handle, listent) == -1) {
        free(listent);
        return -1;
    }
    return 0;
}

int io_listen_stop(io_handle_t *handle) {
    return io__handle_stop(handle);
}

int io_splice_start(io_handle_t *from, io_handle_t *to, io_splice_cb splice_cb) {
    io_splice_t *splicet = new_task(io_splice_t);
    splicet->progress_cb = splice_progress;
    splicet->to = to;
    splicet->splice_cb = splice_cb;
    splicet->event = EV_READ;

    if(pipe(splicet->pipe) == -1) {
        free(splicet);
        return -1;
    }

    if(io__handle_start(from, splicet) == -1) {
        free(splicet);
        return -1;
    }
    return 0;
}

int io_splice_stop(io_handle_t *handle) {
    return io__handle_stop(handle);
}

int io_splice_drain(io_handle_t *handle) {
    static int nullfd = -1;
    io_splice_t *splicet = (io_splice_t*)(handle->task);
    if(nullfd == -1) nullfd = open("/dev/null", O_WRONLY);
    return splice(splicet->pipe[0], NULL, nullfd, NULL, SPLICE_SIZE, SPLICE_FLAGS);
}

int io_watch_start(io_handle_t *handle, const char *path, int events, io_watch_cb watch_cb) {
   if(inotify_add_watch(handle->fd, path, events) == -1)
      return -1;

   if(!handle->task) {
      io_watch_t *watcht = new_task(io_watch_t);
      watcht->progress_cb = watch_progress;
      watcht->watch_cb = watch_cb;
      watcht->event = EV_READ;
      if(io__handle_start(handle, watcht) == -1) {
         free(watcht);
         return -1;
      }
   }
   
   return 0;
}

int io_watch_stop(io_handle_t *handle, const char *path) {
   int wd = inotify_add_watch(handle->fd, path, 0);
   if(wd == -1) return -1;
   if(inotify_rm_watch(handle->fd, wd) == -1) return -1;
   return 0;
}

int io_publish_start(io_handle_t *handle, io_channel_t *channel, io_pub_cb pub_cb) {
   io_publish_t *pubt = new_task(io_publish_t);

   if(pipe(pubt->pipe) == -1) goto publish_fail1;
   if((pubt->nullfd = open("/dev/null", O_WRONLY)) == -1) goto publish_fail2;

   pubt->progress_cb = publish_progress;
   pubt->end_cb = publish_end;
   pubt->channel = channel;
   pubt->pub_cb = pub_cb;
   pubt->event = EV_READ;

   if(io__handle_start(handle, pubt) == -1) goto publish_fail3;
   return 0;

 publish_fail3:
   free(pubt);
 publish_fail2:
   close(pubt->pipe[0]);
   close(pubt->pipe[1]);
 publish_fail1:
   return -1;
}

int io_publish_stop(io_handle_t *handle) {
   return io__handle_stop(handle);
}

int io_channel(io_channel_t *channel) {
   INIT_LIST_HEAD(&channel->subscribers);
   channel->sub_count = 0;
   return 0;
}

int io_channel_close(io_channel_t *channel) {
   list_t *iter, *tmp;
   list_for_each_safe(iter, tmp, &channel->subscribers) {
      io__sub_t *sub = list_entry(iter, io__sub_t, subscribers);
      list_del(iter);
      free(sub);
   }

   channel->sub_count = 0;
   return 0;
}

int io_channel_join(io_channel_t *channel, io_handle_t *handle, io_channel_cb channel_cb) {
   io__sub_t *sub = (io__sub_t*)malloc(sizeof(io__sub_t));
   sub->handle = handle;
   sub->channel_cb = channel_cb;

   if(pipe(sub->pipe) == -1) {
      free(sub);
      return -1;
   }

   list_add(&sub->subscribers, &channel->subscribers);
   channel->sub_count++;
   return 0;
}
