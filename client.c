#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <time.h>
#include <endian.h>
#include "common.h"
#include "protconst.h"
#include "packets.h"
#include "size_const.h"
#include "client_structures.h"
#include "tcp_client_helper.h"
#include "udp_client_helper.h"
#include "client_buffering_helper.h"

void tcp_client_data_transfer(client_conn_params* conn_params) {
  tcp_send_DATA(conn_params);
  free_head_node(conn_params);

  if (conn_params->buffer_list == NULL) {
    ssize_t n;
    ACC_RJT rjt_rcvd;

    n = readn(conn_params->socket_fd, &rjt_rcvd, sizeof(rjt_rcvd));
    if (n < 0) {
      if(errno == EAGAIN) {
        error("noticed not reading RCVD packet from socket on time"
              "(error in communication, closing connection)");
        conn_params->error = true;
      } else {
        error("noticed not reading RCVD packet from socket"
              "(error in communication, closing connection)");
        conn_params->error = true;
      }
      return;
    } else if (n != (ssize_t) sizeof(uint8_t) + sizeof(uint64_t) &&
               n != (ssize_t) sizeof(rjt_rcvd)) {
      error("noticed not getting all necessary data in the packet"
            "(error in communication, closing connection)");
      conn_params->error = true;
      return;
    }

    if(!conn_params->local_connection) {
      rjt_rcvd.session_identificator = be64toh(rjt_rcvd.session_identificator);
      rjt_rcvd.packet_number = be64toh(rjt_rcvd.packet_number);
    }

    if (rjt_rcvd.packet_identificator != 6 &&
        rjt_rcvd.packet_identificator != 7) {
      error("noticed wrong packet identificator"
            "(error in communication, closing connection)");
      conn_params->error = true;
      return;
    }

    if (rjt_rcvd.session_identificator != conn_params->net_session_identificator) {
      error("noticed wrong session identificator"
            "(error in communication, closing connection)");
      conn_params->error = true;
      return;
    }

    if(rjt_rcvd.packet_identificator == 6) {
      if(rjt_rcvd.packet_number > conn_params->last_packet_number) {
        error("noticed wrong RJT packet number"
              "(error in communication, closing connection)");
      } else {
        error("got RJT packet from server (closing connection)");
      }
      conn_params->error = true;
      return;
    }
  }
}

void tcp_client_conn_establishment(client_conn_params* conn_params) {
  if (connect(conn_params->socket_fd,
              (struct sockaddr *) &conn_params->server_address,
                  (socklen_t) sizeof(conn_params->server_address)) < 0) {
    error("unable to connect to the server"
          "(error in communication, closing client)");
    conn_params->error = true;
    return;
  }

  tcp_send_CONN(conn_params);
  if(conn_params->error) return;

  ssize_t n;
  CONACC_CONRJT_RCVD connacc;

  n = readn(conn_params->socket_fd, &connacc, sizeof(connacc));
  if (n < 0) {
    if(errno == EAGAIN) {
      error("noticed not reading CONACC packet from socket on time"
            "(error in communication, closing connection)");
      conn_params->error = true;
    } else {
      error("noticed not reading CONACC packet from socket"
            "(error in communication, closing connection)");
      conn_params->error = true;
    }
    return;
  } else if (n != (ssize_t) sizeof(connacc)) {
    error("noticed not getting all necessary data in the packet"
          "(error in communication, closing connection)");
    conn_params->error = true;
    return;
  }

  if(!conn_params->local_connection) {
    connacc.session_identificator = be64toh(connacc.session_identificator);
  }

  if (connacc.packet_identificator != 2) {
    error("noticed wrong packet identificator"
          "(error in communication, closing connection)");
    conn_params->error = true;
    return;
  } else if (connacc.session_identificator
             != conn_params->net_session_identificator) {
    error("noticed wrong session identificator"
          "(error in communication, closing connection)");
    conn_params->error = true;
    return;
  }
}

void tcp_client(client_conn_params* conn_params) {
  tcp_client_conn_establishment(conn_params);
  while(conn_params->buffer_list != NULL && !conn_params->error) {
    tcp_client_data_transfer(conn_params);
  }
}

void udp_client_data_transfer(client_conn_params* conn_params,
                              client_rcvd_params* rcvd_params,
                              char* data_packet) {
  ssize_t n;

  if(conn_params->buffer_list != NULL) {

    udp_send_DATA(conn_params, data_packet);
    if(conn_params->error) {
      return;
    }

    if (conn_params->protocol_identificator == 3) {
      ACC_RJT conacc_acc_rjt;

      rcvd_params->server_address_length =
          (socklen_t) sizeof(rcvd_params->server_address);
      n = recvfrom(conn_params->socket_fd,
                   &conacc_acc_rjt,
                   sizeof(conacc_acc_rjt),
                   0,
                   (struct sockaddr*) &rcvd_params->server_address,
                       (socklen_t*) &rcvd_params->server_address_length);
      if (n < 0) {
        if(errno == EAGAIN) {
          conn_params->retransmissions_done++;
        } else {
          error("noticed not reading ACC packet from socket"
                "(error in communication, closing connection)");
          conn_params->error = true;
        }
        return;
      } else if (n != (ssize_t) sizeof(uint8_t) + sizeof(uint64_t) &&
                 n != (ssize_t) sizeof(conacc_acc_rjt)) {
        error("noticed not getting all necessary data in the packet"
              "(error in communication, closing connection)");
        conn_params->error = true;
        return;
      }

      if(is_ip_local(&rcvd_params->server_address)) {
        rcvd_params->local_connection = true;
      } else {
        rcvd_params->local_connection = false;
      }

      if(!rcvd_params->local_connection) {
        conacc_acc_rjt.session_identificator =
            be64toh(conacc_acc_rjt.session_identificator);
        conacc_acc_rjt.packet_number =
            be64toh(conacc_acc_rjt.packet_number);
      }

      if (addr_equal(&rcvd_params->server_address,
                     &conn_params->server_address)) {

        if (conacc_acc_rjt.packet_identificator != 2 &&
            conacc_acc_rjt.packet_identificator != 5 &&
            conacc_acc_rjt.packet_identificator != 6) {
          error("noticed wrong packet identificator"
                "(error in communication, closing connection)");
          conn_params->error = true;
          return;
        }

        if (conacc_acc_rjt.session_identificator !=
               conn_params->net_session_identificator) {
          error("noticed wrong session identificator"
                "(error in communication, closing connection)");
          conn_params->error = true;
          return;
        }

        if(conacc_acc_rjt.packet_identificator == 2) {
          return;
        } else if (conacc_acc_rjt.packet_identificator == 5) {
          if(conacc_acc_rjt.packet_number > (conn_params->buffer_list)->index) {
            error("noticed wrong ACC packet number"
                  "(error in communication, closing connection)");
            conn_params->error = true;
            return;
          } else if (conacc_acc_rjt.packet_number <
                     (conn_params->buffer_list)->index) {
            return;
          }
        } else {
          if(conacc_acc_rjt.packet_number > conn_params->last_packet_number) {
            error("noticed wrong RJT packet number"
                  "(error in communication, closing connection)");
          } else {
            error("got RJT packet (closing connection)");
          }
          conn_params->error = true;
          return;
        }

      } else {
        error("noticed wrong address"
              "(error in communication, closing connection)");
        conn_params->error = true;
        return;
      }
      conn_params->retransmissions_done = 0;
    }
    free_head_node(conn_params);
  }

  if (conn_params->buffer_list == NULL) {
    ACC_RJT acc_rjt_rcvd;

    rcvd_params->server_address_length =
        (socklen_t) sizeof(rcvd_params->server_address);
    n = recvfrom(conn_params->socket_fd,
                 &acc_rjt_rcvd,
                 sizeof(acc_rjt_rcvd),
                 0,
                 (struct sockaddr*) &rcvd_params->server_address,
                     (socklen_t*) &rcvd_params->server_address_length);
    if (n < 0) {
      if(errno == EAGAIN) {
        error("noticed not reading RCVD packet from socket on time"
              "(error in communication, closing connection)");
        conn_params->error = true;
      } else {
        error("noticed not reading RCVD packet from socket"
              "(error in communication, closing connection)");
        conn_params->error = true;
      }
      return;
    } else if (n != (ssize_t) sizeof(uint8_t) + sizeof(uint64_t) &&
               n != (ssize_t) sizeof(acc_rjt_rcvd)) {
      error("noticed not getting all necessary data in the packet"
            "(error in communication, closing connection)");
      conn_params->error = true;
      return;
    }

    if(is_ip_local(&rcvd_params->server_address)) {
      rcvd_params->local_connection = true;
    } else {
      rcvd_params->local_connection = false;
    }

    if(!rcvd_params->local_connection) {
      acc_rjt_rcvd.session_identificator =
          be64toh(acc_rjt_rcvd.session_identificator);
      acc_rjt_rcvd.packet_number = be64toh(acc_rjt_rcvd.packet_number);
    }

    if (addr_equal(&rcvd_params->server_address, &conn_params->server_address)) {

      if (acc_rjt_rcvd.packet_identificator != 7 &&
      acc_rjt_rcvd.packet_identificator != 6 &&
      (conn_params->protocol_identificator != 3 ||
      acc_rjt_rcvd.packet_identificator != 5)) {
        error("noticed wrong packet identificator"
              "(error in communication, closing connection)");
        conn_params->error = true;
        return;
      }

      if (acc_rjt_rcvd.session_identificator !=
          conn_params->net_session_identificator) {
        error("noticed wrong session identificator"
              "(error in communication, closing connection)");
        conn_params->error = true;
        return;
      }

      if(acc_rjt_rcvd.packet_identificator == 5) {
        if(acc_rjt_rcvd.packet_number > conn_params->last_packet_number) {
          error("noticed wrong ACC packet number"
                "(error in communication, closing connection)");
          conn_params->error = true;
        }
        return;
      } else if (acc_rjt_rcvd.packet_identificator == 6) {
        if(acc_rjt_rcvd.packet_number > conn_params->last_packet_number) {
          error("noticed wrong RJT packet number"
                "(error in communication, closing connection)");
        } else {
          error("got RJT packet from server"
                "(closing connection)");
        }
        conn_params->error = true;
        return;
      } else {
        conn_params->finished = true;
        return;
      }

    } else {
      error("noticed wrong address"
            "(error in communication, closing connection)");
      conn_params->error = true;
      return;
    }
  }
}

void udp_client_conn_establishment(client_conn_params* conn_params,
                                   client_rcvd_params* rcvd_params) {
  if(conn_params->protocol_identificator == 2) {
    udp_send_CONN(conn_params);
  } else {
    udpr_send_CONN(conn_params);
  }

  if(conn_params->error) {
    return;
  }

  ssize_t n;
  CONACC_CONRJT_RCVD conacc_conrjt;

  rcvd_params->server_address_length =
      (socklen_t) sizeof(rcvd_params->server_address);
  n = recvfrom(conn_params->socket_fd,
               &conacc_conrjt,
               sizeof(conacc_conrjt),
               0,
               (struct sockaddr*) &(rcvd_params->server_address),
                   (socklen_t*) &(rcvd_params->server_address_length));
  if (n < 0) {
    if(errno == EAGAIN) {
      if (conn_params->protocol_identificator == 3) {
        conn_params->retransmissions_done++;
      } else {
        error("noticed not reading CONACC packet from socket on time"
              "(error in communication, closing connection)");
        conn_params->error = true;
      }
    } else {
      error("noticed not reading CONACC packet from socket"
            "(error in communication, closing connection)");
      conn_params->error = true;
    }
    return;
  } else if (n != (ssize_t) sizeof(conacc_conrjt)) {
    error("noticed not getting all necessary data in the packet"
          "(error in communication, closing connection)");
    conn_params->error = true;
    return;
  }

  if(is_ip_local(&rcvd_params->server_address)) {
    rcvd_params->local_connection = true;
  } else {
    rcvd_params->local_connection = false;
  }

  if(!rcvd_params->local_connection) {
    conacc_conrjt.session_identificator =
        be64toh(conacc_conrjt.session_identificator);
  }

  if (addr_equal(&rcvd_params->server_address, &conn_params->server_address)) {

    if (conacc_conrjt.packet_identificator != 2 &&
        conacc_conrjt.packet_identificator != 3) {
      error("noticed wrong packet identificator"
            "(error in communication, closing connection)");
      conn_params->error = true;
      return;
    } else if (conacc_conrjt.session_identificator !=
        conn_params->net_session_identificator) {
      error("noticed wrong session identificator"
            "(error in communication, closing connection)");
      conn_params->error = true;
      return;
    } else if(conacc_conrjt.packet_identificator == 3) {
      error("got CONRJT package from server"
            "(closing connection)");
      conn_params->error = true;
      return;
    }

  } else {
    error("noticed wrong address"
          "(error in communication, closing connection)");
    conn_params->error = true;
    return;
  }
  conn_params->established = true;
  conn_params->retransmissions_done = 0;
}

void udp_client(client_conn_params* conn_params) {
  client_rcvd_params rcvd_params;
  static char data_packet[MAX_DATA_PACKET_SIZE];

  conn_params->established = false;
  conn_params->finished = false;
  conn_params->retransmissions_done = 0;

  while (!conn_params->established && !conn_params->error) {
    udp_client_conn_establishment(conn_params, &rcvd_params);
    if(conn_params->retransmissions_done > MAX_RETRANSMITS) {
      conn_params->error = true;
    }
  }

  while(!conn_params->finished && !conn_params->error) {
    udp_client_data_transfer(conn_params, &rcvd_params, data_packet);
    if(conn_params->retransmissions_done > MAX_RETRANSMITS) {
      conn_params->error = true;
    }
  }
}

int main(int argc, char* argv[]) {
  if (argc != 4) {
    error("usage: %s <type of protocol> <host> <port>", argv[0]);
    return -1;
  }

  srand((unsigned int) time(NULL));
  char* protocol = argv[1];
  const char* host = argv[2];
  uint16_t port = read_port(argv[3]);

  client_conn_params conn_params;

  if(strcmp(protocol, "tcp") == 0) {

    conn_params.socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (conn_params.socket_fd < 0) {
      error("unable to create a socket"
            "(error in communication, closing client)");
      return -1;
    }
    conn_params.protocol_identificator = 1;

  } else if(strcmp(protocol, "udp") == 0) {

    conn_params.socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (conn_params.socket_fd < 0) {
      error("unable to create a socket"
            "(error in communication, closing client)");
      return -1;
    }
    conn_params.protocol_identificator = 2;

  } else if (strcmp(protocol, "udpr") == 0) {

    conn_params.socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (conn_params.socket_fd < 0) {
      error("unable to create a socket"
            "(error in communication, closing client)");
      return -1;
    }
    conn_params.protocol_identificator = 3;

  } else {
    error("unable to match the pattern for the type of protocol"
          "(error in communication, closing client)");
    return -1;
  }

  conn_params.server_address = get_server_address(host, port);
  conn_params.server_address_length =
      (socklen_t) sizeof(conn_params.server_address);
  conn_params.net_session_identificator = random_uint64();
  if(strcmp(host, "127.0.0.1") == 0) {
    conn_params.local_connection = true;
  } else {
    conn_params.local_connection = false;
  }

  set_timeout(conn_params.socket_fd, MAX_WAIT);
  conn_params.error = false;

  if (read_input(&conn_params) == -1) {
    error("unable to allocate buffer for the message"
          "(error in communication, closing client)");
    close(conn_params.socket_fd);
    free_list(&conn_params);
    return -1;
  }

  if(conn_params.protocol_identificator == 1) tcp_client(&conn_params);
  else udp_client(&conn_params);

  int finish_value = 0;
  if(conn_params.error) finish_value = -1;

  close(conn_params.socket_fd);
  free_list(&conn_params);

  return finish_value;
}
