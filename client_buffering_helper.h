#ifndef CLIENT_BUFFERING_HELPER_H
#define CLIENT_BUFFERING_HELPER_H

#include "client_structures.h"

int  read_input(client_conn_params* conn_params);
void free_list(client_conn_params* conn_params);
void free_head_node(client_conn_params* conn_params);


#endif // CLIENT_BUFFERING_HELPER_H