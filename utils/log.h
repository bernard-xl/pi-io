#ifndef LOG_H
#define LOG_H

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

static inline
void log_redirect(int fd) {
    dup2(fd, STDERR_FILENO);
}

#ifdef DEBUG
#define log_debug(format, ...) \
    fprintf(stderr, format, ##__VA_ARGS__)
#else
#define log_debug(format, ...) 
#endif

#define log_info(format, ...) \
    fprintf(stderr, format, ##__VA_ARGS__)

#define log_error(prefix) \
    fprintf(stderr, "%s: %s\n", prefix, strerror(errno))

static inline 
void log_fatal(const char *prefix) {
    fprintf(stderr, "%s: %s\n", prefix, strerror(errno)); \
    exit(errno);
}

#endif
