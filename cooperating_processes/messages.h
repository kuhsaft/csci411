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

union message_data {
  double double_val;
  long long_val;

  explicit message_data(double val) : double_val(val) {}
  explicit message_data(long val) : long_val(val) {}
};

typedef struct message {
  MessageType type;
  message_data data;

  message() : type(UNKNOWN), data(0L) {}
  message(MessageType type, double data) : type(type), data(data) {}
  message(MessageType type, long data) : type(type), data(data) {}
} message;

std::ostream &operator<<(std::ostream &out, const message &in) {
  switch (in.type) {
    case SYN:out << "SYN: " << in.data.long_val;
      break;
    case SYN_ACK:out << "SYN_ACK: " << in.data.long_val;
      break;
    case ACK:out << "ACK: " << in.data.long_val;
      break;
    case TEMPERATURE:out << "Temperature: " << in.data.double_val;
      break;
    case DONE: out << "Done";
      return out;
    case UNKNOWN: out << "Unknown";
      return out;
  }

  return out;
}

#endif //CSCI411_MESSAGES_H
