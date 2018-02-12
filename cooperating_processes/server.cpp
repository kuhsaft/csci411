#include <iostream>
#include <fcntl.h>
#include <mqueue.h>
#include <sstream>
#include "messages.h"

const size_t num_clients = 4;
mqd_t qd_server;
mqd_t qd_client_send[num_clients], qd_client_recv[num_clients];

void at_exit(int signal = 0) {
  mq_close(qd_server);
  for (mqd_t client_send_mq : qd_client_send) {
    mq_close(client_send_mq);
  }
}

void iterate() {
  message client_message;
  for (mqd_t client_recv_mq : qd_client_recv) {
    if (mq_receive(client_recv_mq, (char *) &client_message, sizeof(client_message), nullptr)
        == -1) {
      std::cerr << "Server: Could not receive temperature from client " << client_recv_mq << std::endl;
      at_exit();
      exit(1);
    }
  }
}

int main() {
  signal(SIGINT, &at_exit);

  std::cout << "Server: Started!\n";

  mq_attr attr = {};
  attr.mq_flags = 0;
  attr.mq_maxmsg = max_messages;
  attr.mq_msgsize = max_msg_size;
  attr.mq_curmsgs = 0;

  if ((qd_server = mq_open(server_queue_name, O_RDONLY | O_CREAT, queue_permissions, &attr)) == -1) {
    std::cerr << "Server: Could not create main message queue" << std::endl;
    at_exit();
    exit(1);
  }

  double central_temperature;
  double external_temperatures[num_clients];

  long client_number = 0;

  // Connect to four clients
  while (client_number < num_clients) {
    message client_message;
    while (client_message.type != SYN) {
      if (mq_receive(qd_server, (char *) &client_message, sizeof(client_message), nullptr) == -1) {
        std::cerr << "Server: Could not receive SYN" << std::endl;
        at_exit();
        exit(1);
      }
    }

    std::cout << "Server: Client #" << client_number << " connecting with id " << client_message.data << std::endl;

    // Connect to client
    std::stringstream client_queue_name;
    client_queue_name << client_server_queue_name << client_message.data;
    if ((qd_client_recv[client_number] = mq_open(client_queue_name.str().c_str(), O_WRONLY)) == 1) {
      std::cerr << "Server: Not able to open client queue: " << client_queue_name.str() << std::endl;
      continue;
    }

    // Create a client send queue
    std::stringstream client_send_queue_name;
    client_queue_name << server_client_queue_name << client_message.data;
    if ((qd_client_send[client_number] =
             mq_open(client_send_queue_name.str().c_str(), O_RDONLY | O_CREAT, queue_permissions, &attr)) == -1) {
      std::cerr << "Server: Could not create server to client queue: " << client_send_queue_name.str() << std::endl;
      at_exit();
      exit(1);
    }

    // Send client SYN-ACK
    message syn_ack_msg(SYN_ACK, client_message.data);
    if (mq_send(qd_client_recv[client_number], (const char *) &syn_ack_msg, sizeof(syn_ack_msg), 0) == -1) {
      std::cerr << "Server: Not able to send SYN-ACK to client: " << client_queue_name.str() << std::endl;

      // Close server-to-client queue
      if (mq_close(qd_client_send[client_number]) == -1) {
        std::cerr << "Server: Error closing server-to-client queue: " << client_send_queue_name.str() << std::endl;
        at_exit();
        exit(1);
      }
      continue;
    }

    // Receive ACK
    while (client_message.type != ACK) {
      if (mq_receive(qd_client_recv[client_number], (char *) &client_message, sizeof(client_message), nullptr)
          == -1) {
        std::cerr << "Server: Could not receive ACK: " << client_send_queue_name.str() << std::endl;

        // Close server-to-client queue
        if (mq_close(qd_client_send[client_number]) == -1) {
          std::cerr << "Server: Error closing server-to-client queue: " << client_send_queue_name.str() << std::endl;
          at_exit();
          exit(1);
        }
        continue;
      }
    }

    ++client_number;
  }

  iterate();

  at_exit();
}