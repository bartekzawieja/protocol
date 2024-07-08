#ifndef CLIENT_STRUCTURES_H
#define CLIENT_STRUCTURES_H
#include <stdint.h>
#include <stdbool.h>
#include <netinet/in.h>

typedef struct buffer_node {
  char *buffer;
  uint64_t index;
  uint32_t size;
  struct buffer_node *next;
} buffer_node;

typedef struct client_conn_params {
  int socket_fd;
  bool error;
  uint8_t protocol_identificator;
  uint64_t retransmissions_done;
  uint64_t net_session_identificator;
  struct sockaddr_in server_address;
  socklen_t server_address_length;
  uint64_t buffer_length;
  buffer_node *buffer_list;
  uint64_t last_packet_number;
  bool established;
  bool finished;
  bool local_connection;
} client_conn_params;

typedef struct client_rcvd_params {
  struct sockaddr_in server_address;
  socklen_t server_address_length;
  bool local_connection;
} client_rcvd_params;


#endif // CLIENT_STRUCTURES_H