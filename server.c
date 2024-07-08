#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <endian.h>
#include "common.h"
#include "protconst.h"
#include "tcp_server_helper.h"
#include "udp_server_helper.h"
#include "server_structures.h"
#include "packets.h"
#include "size_const.h"
#include <stdint.h>

void tcp_serv_data_transfer(server_conn_params* conn_params,
                            server_rcvd_params* rcvd_params,
                            char* byte_sequence) {
  ssize_t n;
  DATA_HEADER data_header;

  n = readn(conn_params->client_socket_fd, &data_header, sizeof(data_header));
  if (n < 0) {
    if(errno == EAGAIN) {
      error("noticed not reading DATA packet from socket on time"
            "(error in communication, closing connection)");
      conn_params->error_without_message = true;
    } else {
      error("noticed not reading DATA packet from socket"
            "(error in communication, closing connection)");
      conn_params->error_without_message = true;
    }
    return;
  } else if (n < (ssize_t) sizeof(data_header)) {
    error("noticed not getting all necessary data in the packet"
          "(error in communication, closing connection)");
    conn_params->error_without_message = true;
    return;
  } else {
    rcvd_params->packet_number = data_header.packet_number;
    rcvd_params->net_session_identificator = data_header.session_identificator;
  }

  if(!conn_params->local_connection) {
    rcvd_params->packet_number = be64toh(rcvd_params->packet_number );
    rcvd_params->net_session_identificator =
        be64toh(rcvd_params->net_session_identificator);
    data_header.bytes_in_packet = ntohl(data_header.bytes_in_packet);
  }

  if (data_header.packet_identificator != 4) {
    error("noticed wrong packet identificator"
          "(error in communication, closing connection)");
    conn_params->error_with_rjt = true;
    return;
  } else if (rcvd_params->net_session_identificator !=
      conn_params->net_session_identificator) {
    error("noticed wrong session identificator"
          "(error in communication, closing connection)");
    conn_params->error_with_rjt = true;
    return;
  } else if (rcvd_params->packet_number !=
      conn_params->awaited_packet_number) {
    error("noticed wrong packet number"
          "(error in communication, closing connection)");
    conn_params->error_with_rjt = true;
    return;
  } else if (data_header.bytes_in_packet > 64000) {
    error("noticed wrong packet size, greater than the limit of 64k bytes"
          "(error in communication, closing connection)");
    conn_params->error_with_rjt = true;
    return;
  }

  n = readn(conn_params->client_socket_fd,
            byte_sequence,
            data_header.bytes_in_packet);
  if (n < 0) {
    if(errno == EAGAIN) {
      error("noticed not reading DATA packet from socket on time"
            "(error in communication, closing connection)");
      conn_params->error_without_message = true;
    } else {
      error("noticed not reading DATA packet from socket"
            "(error in communication, closing connection)");
      conn_params->error_without_message = true;
    }
    return;
  } else if (n < (ssize_t) data_header.bytes_in_packet) {
    error("noticed not getting all necessary data in the packet"
          "(error in communication, closing connection)");
    conn_params->error_without_message = true;
    return;
  }

  conn_params->awaited_packet_number++;
  conn_params->awaited_number_of_bytes -=
      (uint64_t) data_header.bytes_in_packet;

  printf("%.*s", data_header.bytes_in_packet, byte_sequence);
  fflush(stdout);

  if (conn_params->awaited_number_of_bytes == 0) {
    tcp_send_RCVD(conn_params);
  }
}

void tcp_serv_conn_establishment(server_conn_params* conn_params) {
  conn_params->net_session_identificator = 0;
  conn_params->client_protocol_identificator = 0;
  conn_params->awaited_packet_number = 0;
  conn_params->awaited_number_of_bytes = 0;

  ssize_t n;
  CONN conn;

  n = readn(conn_params->client_socket_fd, &conn, sizeof(conn));
  if (n < 0) {
    error("noticed not reading CONN packet from socket"
          "(error in communication, closing connection)");
    conn_params->error_without_message = true;
    return;
  } else if (n != (ssize_t) sizeof(conn)) {
    error("noticed not getting all necessary data in the packet"
          "(error in communication, closing connection)");
    conn_params->error_without_message = true;
    return;
  }

  if(!conn_params->local_connection) {
    conn.session_identificator = be64toh(conn.session_identificator);
    conn.message_length = be64toh(conn.message_length);
  }

  if (conn.packet_identificator != 1) {
    error("noticed wrong packet identificator"
          "(error in communication, closing connection)");
    conn_params->error_without_message = true;
    return;
  } else if(conn.protocol_identificator != 1) {
    error("noticed wrong protocol identificator"
          "(error in communication, closing connection)");
    conn_params->error_without_message = true;
    return;
  }

  conn_params->net_session_identificator = conn.session_identificator;
  conn_params->client_protocol_identificator = conn.protocol_identificator;
  conn_params->awaited_packet_number = 0;
  conn_params->awaited_number_of_bytes = conn.message_length;

  set_timeout(conn_params->client_socket_fd, MAX_WAIT);

  tcp_send_CONACC(conn_params);
}

void tcp_server(server_conn_params* conn_params) {
  if (listen(conn_params->socket_fd, TCP_SERVER_QUEUE_LENGTH) < 0) {
    error("unable to switch socket to listening"
          "(critical error in communication, closing server)");
    return;
  }

  static char byte_sequence[MAX_DATA_SIZE];
  server_rcvd_params rcvd_params;

  do {
    conn_params->error_with_rjt = false;
    conn_params->error_without_message = false;
    conn_params->client_socket_fd =
        accept(conn_params->socket_fd,
               (struct sockaddr *) &(conn_params->client_address),
                   &((socklen_t){sizeof(conn_params->client_address)}));
    if (conn_params->client_socket_fd < 0) {
      error("unable to accept connection"
            "(critical error in communication, closing server)");
      break;
    }

    if(is_ip_local(&conn_params->client_address)) {
      conn_params->local_connection = true;
    } else {
      conn_params->local_connection = false;
    }
    tcp_serv_conn_establishment(conn_params);

    while(conn_params->awaited_number_of_bytes > 0 &&
          !conn_params->error_without_message &&
          !conn_params->error_with_rjt) {
      tcp_serv_data_transfer(conn_params, &rcvd_params, byte_sequence);
    }

    if(conn_params->error_with_rjt) {
      tcp_send_RJT(conn_params, &rcvd_params);
    }

    close(conn_params->client_socket_fd);

  } while(1);

}

void udp_serv_data_transfer(server_conn_params* conn_params,
                            server_rcvd_params* rcvd_params,
                            char* byte_sequence) {
  if(conn_params->error_without_message) return;

  ssize_t n;
  DATA_HEADER* data_header = (DATA_HEADER*) byte_sequence;

  rcvd_params->client_address_length =
      (socklen_t) sizeof(rcvd_params->client_address);
  n = recvfrom(conn_params->socket_fd,
               byte_sequence,
               MAX_DATA_PACKET_SIZE,
               0,
               (struct sockaddr*) &rcvd_params->client_address,
                   (socklen_t*) &rcvd_params->client_address_length);
  if (n < 0) {
    if (errno == EAGAIN) {
      if (conn_params->client_protocol_identificator == 3) {
        conn_params->retransmissions_done++;
      } else {
        error("noticed not reading DATA packet from socket on time"
              "(error in communication, closing connection)");
        conn_params->error_without_message = true;
      }
    } else {
      error("noticed not reading DATA packet from socket"
            "(error in communication, closing connection)");
      conn_params->error_without_message = true;
    }
    return;
  } else if (n < (ssize_t) sizeof(*data_header) &&
             (conn_params->client_protocol_identificator != 3
             || n != (ssize_t) 2*(sizeof(uint8_t) + sizeof(uint64_t)))) {
    error("noticed not getting all necessary data in the packet"
          "(error in communication, closing connection)");
    conn_params->error_without_message = true;
    return;
  } else {
    rcvd_params->packet_number = data_header->packet_number;
    rcvd_params->net_session_identificator =
        data_header->session_identificator;
  }

  if(is_ip_local(&rcvd_params->client_address)) {
    rcvd_params->local_connection = true;
  } else {
    rcvd_params->local_connection = false;
  }

  if(!rcvd_params->local_connection) {
    rcvd_params->packet_number  = be64toh(rcvd_params->packet_number );
    rcvd_params->net_session_identificator =
        be64toh(rcvd_params->net_session_identificator);
    data_header->bytes_in_packet = ntohl(data_header->bytes_in_packet);
  }

  if (addr_equal(&rcvd_params->client_address, &conn_params->client_address)) {

    if (data_header->packet_identificator != 4) {
      if (conn_params->client_protocol_identificator != 3 ||
          data_header->packet_identificator != 1) {
        error("noticed wrong packet identificator"
              "(error in communication, closing connection)");
        conn_params->error_with_rjt = true;
      }
      return;
    } else if (rcvd_params->net_session_identificator !=
        conn_params->net_session_identificator) {
      error("noticed wrong session identificator"
            "(error in communication, closing connection)");
      conn_params->error_with_rjt = true;
      return;
    } else if (rcvd_params->packet_number !=
        conn_params->awaited_packet_number) {
      if (conn_params->client_protocol_identificator != 3 ||
          data_header->packet_number > conn_params->awaited_packet_number) {
        error("noticed wrong packet number"
              "(error in communication, closing connection)");
        conn_params->error_with_rjt = true;
      }
      return;
    } else if (data_header->bytes_in_packet > 64000) {
      error("noticed wrong packet size, greater than the limit of 64k bytes"
            "(error in communication, closing connection)");
      conn_params->error_with_rjt = true;
      return;
    }

  } else {
    if (data_header->packet_identificator == 1) {
      conn_params->error_with_conrjt = true;
    } else if (data_header->packet_identificator == 4) {
      conn_params->error_with_rjt = true;
    } else {
      conn_params->error_without_message = true;
    }
    return;
  }

  printf("%.*s",
         data_header->bytes_in_packet,
         byte_sequence + sizeof(*data_header));
  fflush(stdout);

  conn_params->retransmissions_done = 0;
  conn_params->awaited_packet_number++;
  conn_params->awaited_number_of_bytes -= data_header->bytes_in_packet;

  if(conn_params->client_protocol_identificator == 3) {
    udp_send_ACC(conn_params);
  }

  if (conn_params->awaited_number_of_bytes == 0 &&
      !conn_params->error_without_message) {
    udp_send_RCVD(conn_params);
  }
}

void udp_serv_conn_establishment(server_conn_params* conn_params) {
  conn_params->net_session_identificator = 0;
  conn_params->client_protocol_identificator = 0;
  conn_params->awaited_packet_number = 0;
  conn_params->awaited_number_of_bytes = 0;

  ssize_t n; //do czytania
  CONN conn;

  conn_params->client_address_length =
      (socklen_t) sizeof(conn_params->client_address);
  n = recvfrom(conn_params->socket_fd,
               &conn,
               sizeof(conn),
               0,
               (struct sockaddr *) &conn_params->client_address,
                   (socklen_t *) &conn_params->client_address_length);
  if (n < 0) {
    error("noticed not reading CONN packet from socket"
          "(error in communication, closing connection)");
    conn_params->error_without_message = true;
    return;
  } else if (n != (ssize_t) sizeof(conn)) {
    error("noticed not getting all necessary data in the packet"
          "(error in communication, closing connection)");
    conn_params->error_without_message = true;
    return;
  }

  if(is_ip_local(&conn_params->client_address)) {
    conn_params->local_connection = true;
  } else {
    conn_params->local_connection = false;
  }

  if(!conn_params->local_connection) {
    conn.session_identificator = be64toh(conn.session_identificator);
    conn.message_length = be64toh(conn.message_length);
  }

  if (conn.packet_identificator != 1) {
    error("noticed wrong packet identificator"
          "(error in communication, closing connection)");
    conn_params->error_without_message = true;
    return;
  } else if (conn.protocol_identificator != 2 &&
             conn.protocol_identificator != 3) {
    error("noticed wrong protocol identificator"
          "(error in communication, closing connection)");
    conn_params->error_without_message = true;
    return;
  }

  conn_params->net_session_identificator = conn.session_identificator;
  conn_params->client_protocol_identificator = conn.protocol_identificator;
  conn_params->awaited_packet_number = 0;
  conn_params->awaited_number_of_bytes = conn.message_length;

  conn_params->retransmissions_done = 0;
  set_timeout(conn_params->client_socket_fd, MAX_WAIT);

  udp_send_CONACC(conn_params);
}
void udp_server(server_conn_params* conn_params) {
  static char byte_sequence[MAX_DATA_PACKET_SIZE];
  server_rcvd_params rcvd_params;

  do {
    conn_params->error_with_rjt = false;
    conn_params->error_with_conrjt = false;
    conn_params->error_without_message = false;
    set_timeout(conn_params->client_socket_fd, 0);

    udp_serv_conn_establishment(conn_params);

    while (conn_params->awaited_number_of_bytes > 0 &&
           !conn_params->error_without_message &&
           !conn_params->error_with_rjt &&
           !conn_params->error_with_conrjt) {
      if(conn_params->retransmissions_done &&
         conn_params->awaited_packet_number == 0) {
        udp_send_CONACC(conn_params);
      } else if (conn_params->retransmissions_done) {
        udp_send_ACC(conn_params);
      }

      udp_serv_data_transfer(conn_params, &rcvd_params, byte_sequence);

      if(conn_params->retransmissions_done > MAX_RETRANSMITS) {
        conn_params->error_without_message = true;
      }
    }

    if(conn_params->error_with_rjt) {
      udp_send_RJT(conn_params, &rcvd_params);
    } else if (conn_params->error_with_conrjt) {
      udp_send_CONRJT(conn_params, &rcvd_params);
    }
  } while(1);
}

int main(int argc, char* argv[]) {

  if (argc != 3) {
    error("Usage: %s <type of protocol> <port>", argv[0]);
    return -1;
  }

  char* protocol = argv[1];
  uint16_t port = read_port(argv[2]);

  server_conn_params conn_params;

  if(strcmp(protocol, "tcp") == 0) {
    conn_params.socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (conn_params.socket_fd < 0) {
      error("unable to create a socket"
            "(critical error in communication, closing server)");
      return -1;
    }
    conn_params.server_protocol_identificator = 1;

  } else if(strcmp(protocol, "udp") == 0) {
    conn_params.socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (conn_params.socket_fd < 0) {
      error("unable to create a socket"
            "(critical error in communication, closing server)");
      return -1;
    }
    conn_params.server_protocol_identificator = 2;

  } else {
    error("unable to match the pattern for the type of protocol"
          "(critical error in communication, closing server)");
    return -1;
  }

  struct sockaddr_in server_address;
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = htonl(INADDR_ANY);
  server_address.sin_port = htons(port);


  if (bind(conn_params.socket_fd,
           (struct sockaddr *) &server_address,
               (socklen_t) sizeof(server_address)) < 0) {
    error("unable to bind server address to socket fd"
          "(critical error in communication, closing server)");
    return -1;
  }

  if(conn_params.server_protocol_identificator == 1) tcp_server(&conn_params);
  else udp_server(&conn_params);

  close(conn_params.socket_fd);
}

