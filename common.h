#ifndef COMMON_H
#define COMMON_H
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdbool.h>

void               error(const char* fmt, ...);
bool               is_ip_local(struct sockaddr_in* addr);
uint16_t           read_port(char const *string);
bool               addr_equal(struct sockaddr_in *addr1, struct sockaddr_in *addr2);
struct sockaddr_in get_server_address(char const *host, uint16_t port);
ssize_t            readn(int fd, void *vptr, size_t n);
ssize_t            writen(int fd, const void *vptr, size_t n);
uint64_t           random_uint64();
int                set_timeout(int sockfd, int seconds);


#endif // COMMON_H