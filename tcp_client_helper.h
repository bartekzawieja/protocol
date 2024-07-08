#ifndef TCP_CLIENT_HELPER_H
#define TCP_CLIENT_HELPER_H

#include "client_structures.h"

void tcp_send_CONN(client_conn_params* conn_params);
void tcp_send_DATA(client_conn_params* conn_params);


#endif // TCP_CLIENT_HELPER_H