#ifndef IO_OPERATION_H
#define IO_OPERATION_H

#include <types.h>
#include <unistd.h>

typedef struct io_read_s io_read_t;
typedef struct io_write_s io_write_t;
typedef struct io_send_s io_send_t;
typedef struct io_listen_s io_listen_t;
typedef struct io_splice_s io_splice_t;
typedef struct io_watch_s io_watch_t;
typedef struct io_publish_s io_publish_t;

typedef struct io_buf_s io_buf_t;
typedef struct io_channel_s io_channel_t;

typedef void (*io_alloc_cb)(io_handle_t *handle, io_buf_t *buf);
typedef void (*io_read_cb)(io_handle_t *handle, io_buf_t *buf, int status);
typedef void (*io_write_cb)(io_handle_t *handle, io_buf_t *buf, int status);
typedef void (*io_send_cb)(io_handle_t *from, io_handle_t *to, int status);
typedef void (*io_listen_cb)(io_handle_t *handle);
typedef void (*io_splice_cb)(io_handle_t *from, io_handle_t *to, int status);
typedef void (*io_watch_cb)(io_handle_t *handle, const char *path, int events);
typedef void (*io_pub_cb)(io_handle_t *handle, int status);
typedef void (*io_channel_cb)(io_channel_t *channel, io_handle_t *sub, int status);


typedef struct io__sub_s io__sub_t;

#define IO_BUF_FIELDS         \
   void *base;                \
   ssize_t len;               \

struct io_buf_s {
   IO_BUF_FIELDS
};

struct io_channel_s {
   list_t subscribers;
   int sub_count;
};

struct io__sub_s {
   list_t subscribers;
   io_handle_t *handle;
   int pipe[2];
   io_channel_cb channel_cb;
};

#define IO_READ_FIELDS        \
   io_alloc_cb alloc_cb;      \
   io_read_cb read_cb;        \
   
struct io_read_s {
   IO_TASK_FIELDS
   IO_READ_FIELDS
};

#define IO_WRITE_FIELDS       \
   io_write_cb write_cb;      \
   io_buf_t buf;              \
   int progress;              \

struct io_write_s {
   IO_TASK_FIELDS
   IO_REQ_FIELDS
   IO_WRITE_FIELDS
};

#define IO_SEND_FIELDS        \
   io_send_cb send_cb;        \
   io_handle_t *from;         \
   int offset;                \
   int len;                   \
   
struct io_send_s {
   IO_TASK_FIELDS
   IO_REQ_FIELDS
   IO_SEND_FIELDS
};

#define IO_LISTEN_FIELDS      \
   io_listen_cb listen_cb;    \
   
struct io_listen_s {
   IO_TASK_FIELDS
   IO_LISTEN_FIELDS
};

#define IO_SPLICE_FIELDS      \
   io_splice_cb splice_cb;    \
   int pipe[2];               \
   io_handle_t *to;           \

struct io_splice_s {
   IO_TASK_FIELDS
   IO_SPLICE_FIELDS
};

#define IO_WATCH_FIELDS       \
   io_watch_cb watch_cb;      \

struct io_watch_s {
   IO_TASK_FIELDS
   IO_WATCH_FIELDS
};

#define IO_PUBLISH_FIELDS      \
   io_pub_cb pub_cb;           \
   int nullfd;                 \
   int pipe[2];                \
   io_channel_t *channel;      \

struct io_publish_s {
   IO_TASK_FIELDS
   IO_PUBLISH_FIELDS
};


int io_read_start(io_handle_t *handle, io_alloc_cb alloc_cb, io_read_cb read_cb);

int io_read_stop(io_handle_t *handle);

int io_write(io_handle_t *handle, void *data, ssize_t len, io_write_cb write_cb);

int io_sendall(io_handle_t *from, io_handle_t *to, io_send_cb send_cb);

int io_listen_start(io_handle_t *handle, io_listen_cb listen_cb);

int io_listen_stop(io_handle_t *handle);

int io_splice_start(io_handle_t *from, io_handle_t *to, io_splice_cb splice_cb);

int io_splice_stop(io_handle_t *handle);

int io_splice_drain(io_handle_t *handle);

int io_watch_start(io_handle_t *inotify, const char *path, int events, io_watch_cb watch_cb);

int io_watch_stop(io_handle_t *inotify, const char *path);

int io_publish_start(io_handle_t *handle, io_channel_t *channel, io_pub_cb pub_cb);

int io_publish_stop(io_handle_t *handle);

int io_channel(io_channel_t *channel);

int io_channel_close(io_channel_t *channel);

int io_channel_join(io_channel_t *channel, io_handle_t *handle, io_channel_cb channel_cb);

#endif
