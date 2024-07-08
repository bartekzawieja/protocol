#include <endian.h>
#include "packets.h"
#include "common.h"
#include "server_structures.h"

void udp_send_CONACC(server_conn_params* conn_params) {
  CONACC_CONRJT_RCVD conacc = {
      2,
      conn_params->net_session_identificator
  };

  if(!conn_params->local_connection) {
    conacc.session_identificator = htobe64(conacc.session_identificator);
  }

  conn_params->client_address_length =
      (socklen_t) sizeof(conn_params->client_address);
  ssize_t n = sendto(conn_params->socket_fd,
                     &conacc,
                     sizeof(conacc),
                     0,
                     (struct sockaddr*) &conn_params->client_address,
                         conn_params->client_address_length);

  if (n < 0) {
    error("noticed not sending CONACC packet"
          "(error in communication, closing connection)");
    conn_params->error_without_message = true;
  }
}

void udp_send_CONRJT(server_conn_params* conn_params,
                     server_rcvd_params* rcvd_params) {
  CONACC_CONRJT_RCVD conrjt = {
      3,
      rcvd_params->net_session_identificator
  };

  if(!conn_params->local_connection) {
    conrjt.session_identificator = htobe64(conrjt.session_identificator);
  }

  rcvd_params->client_address_length =
      (socklen_t) sizeof(rcvd_params->client_address);
  ssize_t n = sendto(conn_params->socket_fd,
                     &conrjt,
                     sizeof(conrjt),
                     0,
                     (struct sockaddr*) &rcvd_params->client_address,
                         rcvd_params->client_address_length);

  if (n < 0) {
    error("noticed not sending CONRJT packet"
          "(error in communication, closing connection)");
    conn_params->error_without_message = true;
  }
}

void udp_send_ACC(server_conn_params* conn_params) {
  ACC_RJT acc = {
      5,
      conn_params->net_session_identificator,
      conn_params->awaited_packet_number -1
  };

  if(!conn_params->local_connection) {
    acc.session_identificator = htobe64(acc.session_identificator);
    acc.packet_number = htobe64(acc.packet_number);
  }

  conn_params->client_address_length =
      (socklen_t) sizeof(conn_params->client_address);
  ssize_t n = sendto(conn_params->socket_fd,
                     &acc,
                     sizeof(acc),
                     0,
                     (struct sockaddr*) &conn_params->client_address,
                         conn_params->client_address_length);

  if (n < 0) {
    error("noticed not sending ACC packet"
          "(error in communication, closing connection)");
    conn_params->error_without_message = true;
  }
}

void udp_send_RJT(server_conn_params* conn_params, server_rcvd_params* rcvd_params) {
  ACC_RJT rjt = {
      6,
      rcvd_params->net_session_identificator,
      rcvd_params->packet_number
  };

  if(!conn_params->local_connection) {
    rjt.session_identificator = htobe64(rjt.session_identificator);
    rjt.packet_number = htobe64(rjt.packet_number);
  }

  rcvd_params->client_address_length =
      (socklen_t) sizeof(rcvd_params->client_address);
  ssize_t n = sendto(conn_params->socket_fd,
                     &rjt,
                     sizeof(rjt),
                     0,
                     (struct sockaddr*) &rcvd_params->client_address,
                         rcvd_params->client_address_length);

  if (n < 0) {
    error("noticed not sending RJT packet"
          "(error in communication, closing connection)");
    conn_params->error_without_message = true;
  }
}

void udp_send_RCVD(server_conn_params* conn_params) {
  CONACC_CONRJT_RCVD rcvd = {
      7,
      conn_params->net_session_identificator
  };

  if(!conn_params->local_connection) {
    rcvd.session_identificator = htobe64(rcvd.session_identificator);
  }

  conn_params->client_address_length =
      (socklen_t) sizeof(conn_params->client_address);
  ssize_t n = sendto(conn_params->socket_fd,
                     &rcvd,
                     sizeof(rcvd),
                     0,
                     (struct sockaddr*) &conn_params->client_address,
                         conn_params->client_address_length);

  if (n < 0) {
    error("noticed not sending RCVD packet"
          "(error in communication, closing connection)");
    conn_params->error_without_message = true;
  }
}