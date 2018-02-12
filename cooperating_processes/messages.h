//
// Created by Peter on 2/11/2018.
//

#ifndef CSCI411_MESSAGES_H
#define CSCI411_MESSAGES_H

#include <ostream>

const char server_queue_name[] = "/temperature-server";
const char client_server_queue_name[] = "/temperature-client-server-"; // Client-to-server
const char server_client_queue_name[] = "/temperature-server-client-"; // Server-to-client
const int queue_permissions = 0660;
const long max_messages = 10;
const long max_msg_size = 256;
const long msg_buffer_size = max_msg_size + 10;

enum MessageType {
  UNKNOWN,
  SYN,
  SYN_ACK,
  ACK,
  DONE,
  TEMPERATURE
};

typedef struct message {
  MessageType type;
  long data;

  message() : type(UNKNOWN), data(0) {}
  message(MessageType type, long data) : type(type), data(data) {}
} message;

std::ostream &operator<<(std::ostream &out, const message &in) {
  switch (in.type) {
    case SYN:out << "SYN: ";
      break;
    case SYN_ACK:out << "SYN_ACK: ";
      break;
    case ACK:out << "ACK: ";
      break;
    case TEMPERATURE:out << "Temperature: ";
      break;
    case DONE: out << "Done";
      return out;
    case UNKNOWN: out << "Unknown";
      return out;
  }

  out << in.data;
  return out;
}

#endif //CSCI411_MESSAGES_H
