#include <cstdlib>
#include <csignal>
#include <iostream>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <unistd.h>
#include <sstream>
#include <functional>
#include <iomanip>
#include "messages.h"

const size_t num_clients = 4;
mqd_t qd_client_send, qd_client_recv;

const long client_id = getpid();

void quit(int signal = 0) {
  mq_close(qd_client_send);
  mq_close(qd_client_recv);

  if (signal == SIGINT || signal == 0) {
    exit(0);
  } else {
    exit(1);
  }
}

void print_error(const std::string &message) {
  std::cerr << "Client " << client_id << ": " << message << std::endl;
}

void print_message(const std::string &message) {
  std::cout << "Client " << client_id << ": " << message << std::endl;
}

double client_number_to_temp(long number) {
  switch (number) {
    case 0: return 100;
    case 1: return 22;
    case 2: return 50;
    case 3: return 40;
    default: return 0;
  }
}

int main() {
  signal(SIGINT, &quit);

  print_message("Started!");

  mq_attr attr = {};
  attr.mq_flags = 0;
  attr.mq_maxmsg = max_messages;
  attr.mq_msgsize = sizeof(message);
  attr.mq_curmsgs = 0;

  double central_temperature;
  double current_temperature;

  print_message("Connecting to server");

  // Create recv message queue
  std::stringstream client_recv_name_ss;
  client_recv_name_ss << server_client_queue_name << client_id;
  std::string client_recv_name = client_recv_name_ss.str();
  if ((qd_client_recv = mq_open(client_recv_name.c_str(), O_RDONLY | O_CREAT, queue_permissions, &attr)) == -1) {
    print_error("Could not create receive queue");
    quit();
  }

  // Connect to server
  mqd_t qd_server;
  if ((qd_server = mq_open(server_queue_name, O_WRONLY)) == -1) {
    print_error("Could not connect to server");
    quit();
  }

  // Send SYN
  message syn_msg(SYN, client_id);
  if (mq_send(qd_server, (const char *) &syn_msg, sizeof(syn_msg), 0) == -1) {
    print_error("Could not send SYN to server");
    quit();
  }

  // Receive SYN-ACK
  std::stringstream client_queue_name;
  message syn_ack_msg;
  while (syn_ack_msg.type != SYN_ACK) {
    if (mq_receive(qd_client_recv, (char *) &syn_ack_msg, sizeof(syn_ack_msg), nullptr) == -1) {
      print_error("Could not receive SYN-ACK");
      quit();
    }
  }

  // Get the client's temperature
  current_temperature = client_number_to_temp(syn_ack_msg.data.long_val);

  // Connect to client send
  std::stringstream client_send_name_ss;
  client_send_name_ss << client_server_queue_name << client_id;
  std::string client_send_name = client_send_name_ss.str();
  if ((qd_client_send = mq_open(client_send_name.c_str(), O_WRONLY)) == 1) {
    print_error("Could not connect to client-to-server queue");
    quit();
  }

  // Send ACK
  message ack_msg(ACK, client_id);
  if (mq_send(qd_client_send, (const char *) &ack_msg, sizeof(ack_msg), 0) == -1) {
    print_error("Could not send ACK to server");
    quit();
  }

  std::function<void()> send_current_temp = [&current_temperature]() {
    std::stringstream log_msg;
    log_msg << "Sending " << std::setprecision(4) << std::fixed << current_temperature;
    print_message(log_msg.str());

    message current_temp_msg(TEMPERATURE, current_temperature);
    if (mq_send(qd_client_send, (const char *) &current_temp_msg, sizeof(current_temp_msg), 0) == -1) {
      print_error("Could not send temperature to server");
      quit();
    }
  };

  send_current_temp();

  message server_message;
  while (server_message.type != DONE) {
    if (mq_receive(qd_client_recv, (char *) &server_message, sizeof(server_message), nullptr) == -1) {
      print_error("Could not receive temperature");
      quit();
    }

    if (server_message.type == TEMPERATURE) {
      central_temperature = server_message.data.double_val;
      current_temperature = (current_temperature * 3 + 2 * central_temperature) / 5;

      // Send current temperature to server
      send_current_temp();
    }
  }

  print_message("Received DONE");

  quit(0);
}
