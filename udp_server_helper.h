#ifndef UDP_SERVER_HELPER_H
#define UDP_SERVER_HELPER_H

#include "server_structures.h"

void udp_send_CONACC(server_conn_params* conn_params);
void udp_send_CONRJT(server_conn_params* conn_params,
                     server_rcvd_params* rcvd_params);
void udp_send_ACC(server_conn_params* conn_params);
void udp_send_RJT(server_conn_params* conn_params,
                  server_rcvd_params* rcvd_params);
void udp_send_RCVD(server_conn_params* conn_params);

#endif // UDP_SERVER_HELPER_H