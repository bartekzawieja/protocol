#ifndef SERVER_STRUCTURES_H
#define SERVER_STRUCTURES_H
#include <netinet/in.h>
#include <stdbool.h>

typedef struct server_conn_params {
  int socket_fd;
  int client_socket_fd;
  bool error_with_rjt;
  bool error_with_conrjt;
  bool error_without_message;
  bool local_connection;
  uint8_t server_protocol_identificator;
  uint8_t client_protocol_identificator;
  uint64_t retransmissions_done;
  uint64_t net_session_identificator;
  uint64_t awaited_packet_number;
  uint64_t awaited_number_of_bytes;
  struct sockaddr_in client_address;
  socklen_t client_address_length;
} server_conn_params;

typedef struct server_rcvd_params {
  uint64_t net_session_identificator;
  uint64_t packet_number;
  bool local_connection;
  struct sockaddr_in client_address;
  socklen_t client_address_length;
} server_rcvd_params;

#endif // SERVER_STRUCTURES_H