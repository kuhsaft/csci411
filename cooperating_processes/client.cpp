#include <cstdlib>
#include <iostream>
#include <fcntl.h>
#include <mqueue.h>
#include <sstream>
#include <unistd.h>
#include "messages.h"

const size_t num_clients = 4;
mqd_t qd_client_send, qd_client_recv;

const long client_id = getpid();

void at_exit(int signal = 0) {
  mq_close(qd_client_send);
  mq_close(qd_client_recv);
}

void print_error(const std::string &message) {
  std::cerr << "Client " << client_id << ": " << message << std::endl;
}

void print_message(const std::string &message) {
  std::cout << "Client " << client_id << ": " << message << std::endl;
}

int main() {
  signal(SIGINT, &at_exit);

  print_message("Started!");

  mq_attr attr = {};
  attr.mq_flags = 0;
  attr.mq_maxmsg = max_messages;
  attr.mq_msgsize = max_msg_size;
  attr.mq_curmsgs = 0;

  double central_temperature;
  double current_temperature;

  print_message("Connecting to server");

  // Create recv message queue
  std::stringstream client_recv_name_ss;
  client_recv_name_ss << client_server_queue_name << client_id;
  std::string client_recv_name = client_recv_name_ss.str();
  if ((qd_client_recv = mq_open(client_recv_name.c_str(), O_RDONLY | O_CREAT, queue_permissions, &attr)) == -1) {
    print_error("Could not create receive queue");
    at_exit();
    exit(1);
  }

  // Connect to server
  mqd_t qd_server;
  if ((qd_server = mq_open(server_queue_name, O_WRONLY)) == -1) {
    print_error("Could not connect to server");
    at_exit();
    exit(1);
  }

  // Send SYN
  message syn_msg(SYN, client_id);
  if (mq_send(qd_server, (const char *) &syn_msg, sizeof(syn_msg), 0) == -1) {
    print_error("Could not send SYN to server");
    at_exit();
    exit(1);
  }

  // Receive SYN-ACK
  std::stringstream client_queue_name;
  message syn_ack_msg;
  if (mq_receive(qd_client_recv, (char *) &syn_ack_msg, sizeof(syn_ack_msg), nullptr) == -1) {
    print_error("Could not receive SYN-ACK");
    at_exit();
    exit(1);
  }

  // Connect to client send
  std::stringstream client_send_name_ss;
  client_recv_name_ss << server_client_queue_name << client_id;
  std::string client_send_name = client_send_name_ss.str();
  if ((qd_client_send = mq_open(client_send_name.c_str(), O_WRONLY)) == 1) {
    print_error("Could not connect to client-to-server queue");
    at_exit();
    exit(1);
  }

  // Send ACK
  message ack_msg(ACK, client_id);
  if (mq_send(qd_server, (const char *) &ack_msg, sizeof(ack_msg), 0) == -1) {
    print_error("Could not send ACK to server");
    at_exit();
    exit(1);
  }

  at_exit();
}
