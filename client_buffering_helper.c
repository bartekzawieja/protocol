#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "client_structures.h"

#define BUFFER_SIZE 64000

static buffer_node* create_node(uint64_t index_numer, uint32_t bytes_filled) {
  buffer_node *newNode = (buffer_node*)malloc(sizeof(buffer_node));
  if (newNode == NULL) {
    return NULL;
  }
  newNode->buffer = (char*)malloc(BUFFER_SIZE);
  if (newNode->buffer == NULL) {
    free(newNode);
    return NULL;
  }
  newNode->index = index_numer;
  newNode->size = bytes_filled;
  newNode->next = NULL;
  return newNode;
}

int read_input(client_conn_params* conn_params) {
  static char tempBuffer[BUFFER_SIZE];

  size_t bytesRead;
  buffer_node* old_node = NULL;
  uint64_t old_index = 0;
  buffer_node* new_node;

  while ((bytesRead =
      fread(tempBuffer, sizeof(char), BUFFER_SIZE, stdin)) > 0) {

    if(old_node == NULL) {
      new_node = create_node(0, (uint32_t) bytesRead);
      if(new_node == NULL) {
        return -1;
      }

      memcpy(new_node->buffer, tempBuffer, bytesRead);

      conn_params->buffer_list = new_node;
      conn_params->buffer_length += bytesRead;

      old_node = new_node;
    } else {
      new_node = create_node(old_index+1, (uint32_t) bytesRead);
      if(new_node == NULL) {
        return -1;
      }

      memcpy(new_node->buffer, tempBuffer, bytesRead);

      conn_params->buffer_length += bytesRead;

      old_node->next = new_node;
      old_node = new_node;
      old_index++;
    }

    if (feof(stdin)) {
      break;
    } else if (ferror(stdin)) {
      perror("Error reading from stdin");
      return -1;
    }
  }

  conn_params->last_packet_number = old_index;
  return 0;
}

void free_list(client_conn_params* conn_params) {
  buffer_node *current = conn_params->buffer_list;
  while (current != NULL) {
    buffer_node *temp = current;
    current = current->next;
    free(temp->buffer);
    free(temp);
  }
}

void free_head_node(client_conn_params* conn_params) {
  buffer_node* old_head = conn_params->buffer_list;
  if(old_head != NULL) {
    conn_params->buffer_list = (conn_params->buffer_list)->next;
    free(old_head);
  }
}