#include <endian.h>
#include "packets.h"
#include "common.h"
#include "server_structures.h"

void tcp_send_CONACC(server_conn_params* conn_params) {
  CONACC_CONRJT_RCVD conacc = {
      2,
      conn_params->net_session_identificator
  };

  if(!conn_params->local_connection) {
    conacc.session_identificator = htobe64(conacc.session_identificator);
  }

  ssize_t n = writen(conn_params->client_socket_fd, &conacc, sizeof(conacc));

  if (n < 0) {
    error("noticed not sending CONACC packet"
          "(error in communication, closing connection)");
    conn_params->error_without_message = true;
  }
}

void tcp_send_RJT(server_conn_params* conn_params,
                  server_rcvd_params* rcvd_params) {
  ACC_RJT rjt = {
      6,
      rcvd_params->net_session_identificator,
      rcvd_params->packet_number
  };

  if(!conn_params->local_connection) {
    rjt.session_identificator = htobe64(rjt.session_identificator);
    rjt.packet_number = htobe64(rjt.packet_number);
  }

  ssize_t n = writen(conn_params->client_socket_fd, &rjt, sizeof(rjt));

  if (n < 0) {
    error("noticed not sending RJT packet"
          "(error in communication, closing connection)");
    conn_params->error_without_message = true;
  }
}

void tcp_send_RCVD(server_conn_params* conn_params) {
  CONACC_CONRJT_RCVD rcvd = {
      7,
      conn_params->net_session_identificator
  };

  if(!conn_params->local_connection) {
    rcvd.session_identificator = htobe64(rcvd.session_identificator);
  }

  ssize_t n = writen(conn_params->client_socket_fd, &rcvd, sizeof(rcvd));

  if (n < 0) {
    error("noticed not sending RCVD packet"
          "(error in communication, closing connection)");
    conn_params->error_without_message = true;
  }
}