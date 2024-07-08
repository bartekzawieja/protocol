#ifndef UDP_CLIENT_HELPER_H
#define UDP_CLIENT_HELPER_H

#include "client_structures.h"

void udp_send_CONN(client_conn_params* conn_params);
void udpr_send_CONN(client_conn_params* conn_params);
void udp_send_DATA(client_conn_params* conn_params, char* data_packet);

#endif // UDP_CLIENT_HELPER_H