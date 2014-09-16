#define _GNU_SOURCE
#include <loop.h>
#include <signal.h>
#include <string.h>
#include <sys/epoll.h>

int io_loop_init(io_loop_t *loop) {
    memset(loop, 0, sizeof(io_loop_t));
    if((loop->fd = epoll_create1(0)) == -1) return -1;
    INIT_LIST_HEAD(&loop->active_handles);
    return 0;
}

int io_loop_run(io_loop_t *loop) {
    int i;
    struct epoll_event event[EPOLL_MAXEVENT];

    loop->thread = pthread_self();
    loop->running = 1;

    while(loop->handle_count && loop->running) {
        int n = epoll_wait(loop->fd, event, EPOLL_MAXEVENT, -1);
        if(n == -1) continue;

        for(i = 0; i < n; i++) {
            io_handle_t *handle = (io_handle_t*)event[i].data.ptr;
            io_task_t *task = handle->task;
            io_req_t *req = handle->req;

            if((event[i].events & task->event) || (event[i].events & EV_ERR)) 
                (*(task->progress_cb))(handle, task);

            while(req) {
                if(event[i].events & req->event) {
                    (*(req->progress_cb))(handle, (io_task_t*)req);
                    if(!req->done) break;
                    io__handle_done(handle);
                    req = handle->req;
                } else {
                    req = req->next;
                }
            }
        }
    }
}

int io_loop_break(io_loop_t *loop) {
    if(!loop->thread) return 0;
    loop->running = 0;
    if(pthread_kill(loop->thread, SIGUSR1) == -1) return -1;
    loop->thread = 0;
    return 0;
}

int io_loop_close(io_loop_t *loop) {
    close(loop->fd);
    loop->running = 0;
}

int io_loop_foreach(io_loop_t *loop, io_loop_walk walk) {
    list_t *pos, *n;
    list_for_each_safe(pos, n, &loop->active_handles) {
        io_handle_t *handle = list_entry(pos, io_handle_t, active_handles);
        (*walk)(handle);
    }
    return 0;
}




