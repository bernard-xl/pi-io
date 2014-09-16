#include <tcp.h>
#include <handle.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>

#define BACK_LOG    5

static inline 
int inet_resolve(struct addrinfo **result, const char *hostname, const char *sername, int socktype) {
    struct addrinfo hint;
    int ret;

    memset(&hint, 0, sizeof(hint));
    hint.ai_family = AF_UNSPEC;
    hint.ai_socktype = socktype;

    if((ret = getaddrinfo(hostname, sername, &hint, result)) != 0) {
        errno = ret;                        
        return -1;                                   
    }
    return 0;
}

int io_tcp_server(io_loop_t *loop, io_handle_t *handle, const char *addr, const char *serv) {
    struct addrinfo *result, *iter;
    int fd;
    int one = 1;

    if(inet_resolve(&result, addr, serv, SOCK_STREAM) == -1) return -1;
    
    for(iter = result; iter != NULL; iter = iter->ai_next) {
        fd = socket(iter->ai_family, (iter->ai_socktype) | SOCK_NONBLOCK, iter->ai_protocol);
        if(fd != -1) break;
    }

    if(fd == -1) {
        freeaddrinfo(result);
        return -1;
    }

    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    if(bind(fd, iter->ai_addr, iter->ai_addrlen) == -1) goto tcp_server_fail; 

    if(listen(fd, BACK_LOG) == -1) goto tcp_server_fail;

    io__handle_init(handle, loop, fd);
    freeaddrinfo(result);
    return 0;

tcp_server_fail:
    freeaddrinfo(result);
    close(fd);
    return -1;
}

int io_tcp_client(io_loop_t *loop, io_handle_t *handle, const char *addr, const char *serv) {
    struct addrinfo *result, *iter;
    int fd;
    int one = 1;

    if(inet_resolve(&result, addr, serv, SOCK_STREAM) == -1) return -1;
    
    for(iter = result; iter != NULL; iter = iter->ai_next) {
        fd = socket(iter->ai_family, iter->ai_family, iter->ai_socktype | SOCK_NONBLOCK);
        if(fd != -1) break;
    }

    if(fd == -1) {
        freeaddrinfo(result);
        return -1;
    }

    if(connect(fd, iter->ai_addr, iter->ai_addrlen) == -1) {
        if(errno != EINPROGRESS) goto tcp_client_fail;
    }
    
    freeaddrinfo(result);
    io__handle_init(handle, loop, fd);
    return 0;

tcp_client_fail:
    freeaddrinfo(result);
    close(fd);
    return -1;

}

int io_tcp_accept(io_handle_t *server, io_handle_t *client, char *addr, int len) {
    struct sockaddr_storage sockaddr;
    int fd;
    socklen_t socklen = sizeof(sockaddr);
    
    if((fd = accept(server->fd, (struct sockaddr*)&sockaddr, &socklen)) == -1) return -1;
    
    getnameinfo((struct sockaddr*)&sockaddr, socklen, addr, len, NULL, 0, NI_NUMERICHOST);
    
    if(io__handle_init(client, server->loop, fd) == -1) {
        close(fd);
        return -1;
    }
    return 0;
}

int io_tcp_close(io_handle_t *handle) {
    io__handle_close(handle);
    close(handle->fd);
}
