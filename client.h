#ifndef UNTITLED9_LIBRARY_H
#define UNTITLED9_LIBRARY_H

#include "client_structures.h"

void tcp_client_data_transfer(client_conn_params* conn_params);
void tcp_client_conn_establishment(client_conn_params* conn_params);
void tcp_client(client_conn_params* conn_params);

void udp_client_data_transfer(client_conn_params* conn_params,
                              client_rcvd_params* rcvd_params,
                              char* data_packet);
void udp_client_conn_establishment(client_conn_params* conn_params,
                                   client_rcvd_params* rcvd_params);
void udp_client(client_conn_params* conn_params);

#endif //UNTITLED9_LIBRARY_H
