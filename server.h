#ifndef SERVER_H
#define SERVER_H

#include "server_structures.h"

void tcp_serv_data_transfer(server_conn_params* conn_params,
                            server_rcvd_params* rcvd_params,
                            char* byte_sequence);
void tcp_serv_conn_establishment(server_conn_params* conn_params);
void tcp_server(server_conn_params* conn_params);

void udp_serv_data_transfer(server_conn_params* conn_params,
                            server_rcvd_params* rcvd_params,
                            char* byte_sequence);
void udp_serv_conn_establishment(server_conn_params* conn_params);
void udp_server(server_conn_params* conn_params);


#endif // SERVER_H
