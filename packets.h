#ifndef PACKETS_H
#define PACKETS_H
#include <stdint.h>

typedef struct __attribute__((__packed__)) {
  uint8_t packet_identificator;
  uint64_t session_identificator;
  uint8_t protocol_identificator;
  uint64_t message_length;
} CONN;

typedef struct __attribute__((__packed__)) {
  uint8_t packet_identificator;
  uint64_t session_identificator;
} CONACC_CONRJT_RCVD;

typedef struct __attribute__((__packed__)) {
  uint8_t packet_identificator;
  uint64_t session_identificator;
  uint64_t packet_number;
  uint32_t bytes_in_packet;
} DATA_HEADER;

typedef struct __attribute__((__packed__)) {
  uint8_t packet_identificator;
  uint64_t session_identificator;
  uint64_t packet_number;
} ACC_RJT;

#endif // PACKETS_H