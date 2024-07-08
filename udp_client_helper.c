#include <endian.h>
#include "common.h"
#include "packets.h"
#include "client_structures.h"

void udp_send_CONN(client_conn_params* conn_params) {
  CONN conn = {
      1,
      conn_params->net_session_identificator,
      2,
      conn_params->buffer_length
  };

  if(!conn_params->local_connection) {
    conn.session_identificator = htobe64(conn.session_identificator);
    conn.message_length = htobe64(conn.message_length);
  }

  conn_params->server_address_length =
      (socklen_t) sizeof(conn_params->server_address);
  ssize_t n = sendto(conn_params->socket_fd,
                     &conn,
                     sizeof(conn),
                     0,
                     (struct sockaddr*) &conn_params->server_address,
                         conn_params->server_address_length);

  if (n < 0) {
    error("noticed not sending CONN packet"
          "(error in communication, closing connection)");
    conn_params->error = true;
  }

}

void udpr_send_CONN(client_conn_params* conn_params) {
  CONN conn = {
      1,
      conn_params->net_session_identificator,
      3,
      conn_params->buffer_length
  };

  if(!conn_params->local_connection) {
    conn.session_identificator = htobe64(conn.session_identificator);
    conn.message_length = htobe64(conn.message_length);
  }

  conn_params->server_address_length =
      (socklen_t) sizeof(conn_params->server_address);
  ssize_t n = sendto(conn_params->socket_fd,
                     &conn,
                     sizeof(conn),
                     0,
                     (struct sockaddr*) &conn_params->server_address,
                         conn_params->server_address_length);

  if (n < 0) {
    error("noticed not sending CONN packet"
          "(error in communication, closing connection)");
    conn_params->error = true;
  }
}

void udp_send_DATA(client_conn_params* conn_params, char* data_packet) {
  DATA_HEADER data_header = {
      4,
      conn_params->net_session_identificator,
      (conn_params->buffer_list)->index,
      (conn_params->buffer_list)->size
  };

  if(!conn_params->local_connection) {
    data_header.session_identificator =
        htobe64(data_header.session_identificator);
    data_header.packet_number = htobe64(data_header.packet_number);
    data_header.bytes_in_packet = htonl(data_header.bytes_in_packet);
  }

  memcpy(data_packet,
         &data_header,
         sizeof(data_header));
  memcpy(data_packet+sizeof(data_header),
         (conn_params->buffer_list)->buffer,
         (size_t) (conn_params->buffer_list)->size);

  conn_params->server_address_length =
      (socklen_t) sizeof(conn_params->server_address);
  ssize_t n = sendto(conn_params->socket_fd,
                     data_packet,
                     (size_t) (sizeof(data_header) + (conn_params->buffer_list)->size),
                     0,
                     (struct sockaddr*) &conn_params->server_address,
                         conn_params->server_address_length);

  if (n < 0) {
    error("noticed not sending DATA packet"
          "(error in communication, closing connection)");
    conn_params->error = true;
  }
}

