#include "common.h"
#include <stdbool.h>
#include <endian.h>
#include "packets.h"
#include "client_structures.h"


void tcp_send_CONN(client_conn_params* conn_params) {
  CONN conn = {
      1,
      conn_params->net_session_identificator,
      1,
      conn_params->buffer_length
  };

  if(!conn_params->local_connection) {
    conn.session_identificator = htobe64(conn.session_identificator);
    conn.message_length = htobe64(conn.message_length);
  }

  ssize_t n = writen(conn_params->socket_fd, &conn, sizeof(conn));

  if (n < 0) {
    error("noticed not sending RJT packet"
          "(error in communication, closing connection)");
    conn_params->error = true;
  }

}

void tcp_send_DATA(client_conn_params* conn_params) {
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

  ssize_t n = writen(conn_params->socket_fd,
                     &data_header,
                     sizeof(data_header));

  if (n < 0) {
    error("noticed not sending header of DATA packet"
          "(error in communication, closing connection)");
    conn_params->error = true;
    return;
  }

  n = writen(conn_params->socket_fd,
             (conn_params->buffer_list)->buffer,
             (size_t) (conn_params->buffer_list)->size);

  if (n < 0) {
    error("noticed not sending data in DATA packet"
          "(error in communication, closing connection)");
    conn_params->error = true;
  }
}