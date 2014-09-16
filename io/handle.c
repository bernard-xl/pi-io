#define _GNU_SOURCE
#include <handle.h>
#include <sys/epoll.h>
#include <stdlib.h>
#include <string.h>

int io__handle_init(io_handle_t *handle, io_loop_t *loop, int fd) {
   handle->fd = fd;
   handle->loop = loop;
   handle->events = 0;
   handle->task = NULL;
   handle->req = NULL;
   list_add(&handle->active_handles, &loop->active_handles);
   (loop->handle_count)++;
   return 0;
}

int io__handle_start(io_handle_t *handle, io_task_t *task) {
   struct epoll_event event;
   event.data.ptr = handle;
   event.events = handle->events | task->event;

   if(handle->events) {
      if(epoll_ctl(handle->loop->fd, EPOLL_CTL_MOD, handle->fd, &event) == -1)
         return -1;
   } else {
      if(epoll_ctl(handle->loop->fd, EPOLL_CTL_ADD, handle->fd, &event) == -1)
         return -1;
   }

   handle->task = task;
   handle->events |= task->event;
   return 0;
}

int io__handle_stop(io_handle_t *handle) {
   io_task_t *task = handle->task;
   if(task->end_cb) (*task->end_cb)(handle, task);
   handle->events &= ~(task->event);

   if(handle->events) {
      struct epoll_event event;
      event.data.ptr = handle;
      event.events = handle->events;

      if(epoll_ctl(handle->loop->fd, EPOLL_CTL_MOD, handle->fd, &event) == -1) {
         handle->events |= task->event;
         return -1;
      }
   } else {
      if(epoll_ctl(handle->loop->fd, EPOLL_CTL_DEL, handle->fd, NULL) == -1) {
         handle->events |= task->event;
         return -1;
      }
   }

   free(task);
   handle->task = NULL;
   return 0;
}

int io__handle_req(io_handle_t *handle, io_req_t *req) {
   if(!handle->req) {
      struct epoll_event event;
      event.data.ptr = handle;
      event.events = handle->events | req->event;

      if(handle->events) {
         if(epoll_ctl(handle->loop->fd, EPOLL_CTL_MOD, handle->fd, &event) == -1)
            return -1;
      } else {
         if(epoll_ctl(handle->loop->fd, EPOLL_CTL_ADD, handle->fd, &event) == -1)
            return -1;
      }
      handle->events |= req->event;
      handle->req = req;
   } else {
      handle->req->next = req;
   }

   return 0;
}

int io__handle_done(io_handle_t *handle) {
   io_req_t *next, *curr;

   curr = handle->req;
   next = curr->next;

   if(curr->end_cb) (*curr->end_cb)(handle, (io_task_t*)curr);

   if(next && (curr->event != next->event)) {
      int orig = handle->events;
      struct epoll_event event;
      event.data.ptr = handle;
      event.events = (handle->events | next->event) & ~(curr->event);
      epoll_ctl(handle->loop->fd, EPOLL_CTL_MOD, handle->fd, &event);
      handle->events = event.events; 
   } else if(!next) {
      handle->events &= ~(curr->event);
      if(handle->events) {
         struct epoll_event event;
         event.data.ptr = handle;
         event.events = handle->events;
         epoll_ctl(handle->loop->fd, EPOLL_CTL_MOD, handle->fd, &event);
      } else {
         epoll_ctl(handle->loop->fd, EPOLL_CTL_DEL, handle->fd, NULL);
      }
   }

   handle->req = next;
   free(curr);
   return 0;
}

int io__handle_giveup(io_handle_t *handle) {
   if(handle->req) {
      io_req_t *req = handle->req;
      io_req_t *tmp = NULL;
      handle->events &= ~(req->event);

      if(handle->events) {
         struct epoll_event event;
         event.data.ptr = handle;
         event.events = handle->events;

         if(epoll_ctl(handle->loop->fd, EPOLL_CTL_MOD, handle->fd, &event) == -1) {
            handle->events |= req->event;
            return -1;
         }
      } else {
         if(epoll_ctl(handle->loop->fd, EPOLL_CTL_DEL, handle->fd, NULL) == -1) {
            handle->events |= req->event;
            return -1;
         }
      }

      while(req) {
         if(req->end_cb) (*req->end_cb)(handle, (io_task_t*)req);
         tmp = req;
         req = req->next;
         free(tmp);
      }

   }
   return 0;
}

int io__handle_close(io_handle_t *handle) {
   if(handle->task) io__handle_stop(handle);
   if(handle->req) io__handle_giveup(handle);
   (handle->loop->handle_count)--;
   list_del(&handle->active_handles);
   return 0;
}
