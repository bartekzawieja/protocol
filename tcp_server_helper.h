#ifndef TCP_SERVER_HELPER_H
#define TCP_SERVER_HELPER_H

#include "server_structures.h"

void tcp_send_CONACC(server_conn_params* conn_params);
void tcp_send_RJT(server_conn_params* conn_params,
                  server_rcvd_params* rcvd_params);
void tcp_send_RCVD(server_conn_params* conn_params);

#endif // SERVER_H