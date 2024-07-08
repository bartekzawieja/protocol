#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdnoreturn.h>

static noreturn void fatal(const char* fmt, ...) {
  va_list fmt_args;

  fprintf(stderr, "\tERROR: ");

  va_start(fmt_args, fmt);
  vfprintf(stderr, fmt, fmt_args);
  va_end(fmt_args);

  fprintf(stderr, "\n");
  exit(1);
}

void error(const char* fmt, ...) {
  va_list fmt_args;
  int org_errno = errno;

  fprintf(stderr, "\tERROR: ");

  va_start(fmt_args, fmt);
  vfprintf(stderr, fmt, fmt_args);
  va_end(fmt_args);

  if (org_errno != 0) {
    fprintf(stderr, " (%d; %s)", org_errno, strerror(org_errno));
  }
  fprintf(stderr, "\n");
}

bool is_ip_local(struct sockaddr_in* addr) {
  char const *client_ip = inet_ntoa(addr->sin_addr);
  if(strcmp(client_ip, "127.0.0.1") == 0) {
    return true;
  } else {
    return false;
  }
}

uint16_t read_port(char const *string) {
  char *endptr;
  errno = 0;
  unsigned long port = strtoul(string, &endptr, 10);
  if (errno != 0 || *endptr != 0 || port > UINT16_MAX) {
    fatal("%s is not a valid port number", string);
  }
  return (uint16_t) port;
}

bool addr_equal(struct sockaddr_in *addr1, struct sockaddr_in *addr2) {
  return (addr1->sin_addr.s_addr == addr2->sin_addr.s_addr &&
      addr1->sin_port == addr2->sin_port);
}

struct sockaddr_in get_server_address(char const *host, uint16_t port) {
  struct addrinfo hints;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET; // IPv4
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;

  struct addrinfo *address_result;
  int errcode = getaddrinfo(host, NULL, &hints, &address_result);
  if (errcode != 0) {
    fatal("getaddrinfo: %s", gai_strerror(errcode));
  }

  struct sockaddr_in send_address;
  send_address.sin_family = AF_INET;
  send_address.sin_addr.s_addr =
      ((struct sockaddr_in *) (address_result->ai_addr))->sin_addr.s_addr;
  send_address.sin_port = htons(port);

  freeaddrinfo(address_result);

  return send_address;
}

ssize_t readn(int fd, void *vptr, size_t n) {
  ssize_t nleft, nread;
  char *ptr;

  ptr = vptr;
  nleft = n;
  while (nleft > 0) {
    if ((nread = read(fd, ptr, nleft)) < 0)
      return nread;
    else if (nread == 0)
      break;

    nleft -= nread;
    ptr += nread;
  }
  return n - nleft;
}

ssize_t writen(int fd, const void *vptr, size_t n){
  ssize_t nleft, nwritten;
  const char *ptr;

  ptr = vptr;
  nleft = n;
  while (nleft > 0) {
    if ((nwritten = write(fd, ptr, nleft)) <= 0)
      return nwritten;

    nleft -= nwritten;
    ptr += nwritten;
  }
  return n;
}

uint64_t random_uint64() {
  uint64_t random_value = 0;
  for (int i = 0; i < 8; i++) {
    random_value = (random_value << 8) | (rand() & 0xFF);
  }
  return random_value;
}

int set_timeout(int sockfd, int seconds) {
  struct timeval timeout;
  timeout.tv_sec = seconds;
  timeout.tv_usec = 0;
  return setsockopt(sockfd,
                    SOL_SOCKET,
                    SO_RCVTIMEO,
                    (const char*)&timeout, sizeof(timeout));
}
