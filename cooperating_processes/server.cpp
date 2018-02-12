#include <csignal>
#include <cmath>
#include <iostream>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <sstream>
#include <unistd.h>
#include <iomanip>
#include "messages.h"

const size_t num_clients = 4;
mqd_t qd_server;
mqd_t qd_client_send[num_clients], qd_client_recv[num_clients];

void quit(int signal = 0) {
  mq_close(qd_server);
  for (mqd_t client_send_mq : qd_client_send) {
    mq_close(client_send_mq);
  }

  if (signal == SIGINT || signal == 0) {
    exit(0);
  } else {
    exit(1);
  }
}

void iterate(double &central_temp, double external_temps[num_clients]) {
  double sum = 0.0;
  for (size_t i = 0; i < num_clients; ++i) {
    message client_message(UNKNOWN, 0.0);
    while (client_message.type != TEMPERATURE) {
      if (mq_receive(qd_client_recv[i], (char *) &client_message, sizeof(client_message), nullptr)
          == -1) {
        std::cerr << "Server: Could not receive temperature from client " << i << std::endl;
        quit();
      }
    }

    sum += client_message.data.double_val;
    external_temps[i] = client_message.data.double_val;
  }

  central_temp = (2 * central_temp + sum) / 6;

  // Send current temperature to clients
  std::stringstream log_msg;
  log_msg << "Server: Sending " << std::setprecision(4) << std::fixed << central_temp;
  std::cout << log_msg.str() << std::endl;

  message central_temp_msg(TEMPERATURE, central_temp);
  for (mqd_t &client_send_mq : qd_client_send) {
    if (mq_send(client_send_mq, (const char *) &central_temp_msg, sizeof(central_temp_msg), 0) == -1) {
      std::cerr << "Server: Could not receive temperature to client" << std::endl;
      quit();
    }
  }
}

bool is_stable(const double external_temps[num_clients]) {
  double val = external_temps[0];
  for (size_t i = 1; i < num_clients; ++i) {
    if (std::abs(external_temps[i] - val) > 0.0001)
      return false;

    val = external_temps[i];
  }

  return true;
}

int main() {
  signal(SIGINT, &quit);

  std::cout << "Server: Started!\n";

  mq_attr attr = {};
  attr.mq_flags = 0;
  attr.mq_maxmsg = max_messages;
  attr.mq_msgsize = sizeof(message);
  attr.mq_curmsgs = 0;

  if ((qd_server = mq_open(server_queue_name, O_RDONLY | O_CREAT, queue_permissions, &attr)) == -1) {
    std::cerr << "Server: Could not create main message queue" << std::endl;
    quit();
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
        quit();
      }
    }

    std::cout << "Server: Client #" << client_number << " connecting with id " << client_message.data.long_val
              << std::endl;

    // Connect to client
    std::stringstream client_send_queue_name;
    client_send_queue_name << server_client_queue_name << client_message.data.long_val;
    if ((qd_client_send[client_number] = mq_open(client_send_queue_name.str().c_str(), O_WRONLY)) == 1) {
      std::cerr << "Server: Not able to open client queue: " << client_send_queue_name.str() << std::endl;
      continue;
    }

    // Create a client recv queue
    std::stringstream client_recv_queue_name;
    client_recv_queue_name << client_server_queue_name << client_message.data.long_val;
    if ((qd_client_recv[client_number] =
             mq_open(client_recv_queue_name.str().c_str(), O_RDONLY | O_CREAT, queue_permissions, &attr)) == -1) {
      std::cerr << "Server: Could not create server to client queue: " << client_recv_queue_name.str() << std::endl;
      quit();
    }

    // Send client SYN-ACK
    message syn_ack_msg(SYN_ACK, client_number);
    if (mq_send(qd_client_send[client_number], (const char *) &syn_ack_msg, sizeof(syn_ack_msg), 0) == -1) {
      std::cerr << "Server: Not able to send SYN-ACK to client: " << client_send_queue_name.str() << std::endl;

      // Close server-to-client queue
      if (mq_close(qd_client_send[client_number]) == -1) {
        std::cerr << "Server: Error closing server-to-client queue: " << client_recv_queue_name.str() << std::endl;
        quit();
      }
      continue;
    }

    while (client_message.type != ACK) {
      // Receive ACK
      if (mq_receive(qd_client_recv[client_number], (char *) &client_message, sizeof(client_message), nullptr)
          == -1) {
        std::cerr << "Server: Could not receive ACK: " << client_recv_queue_name.str() << std::endl;

        // Close server-to-client queue
        if (mq_close(qd_client_send[client_number]) == -1) {
          std::cerr << "Server: Error closing server-to-client queue: " << client_recv_queue_name.str() << std::endl;
          quit();
        }
        continue;
      }
    }

    ++client_number;
  }


  // Send/receive until stable
  iterate(central_temperature, external_temperatures);
  while (!is_stable(external_temperatures)) {
    iterate(central_temperature, external_temperatures);
  }

  // Send DONE
  std::cout << "Server: Sending DONE" << std::endl;

  message done_msg(DONE, 0L);
  for (mqd_t &client_send_mq : qd_client_send) {
    if (mq_send(client_send_mq, (const char *) &done_msg, sizeof(done_msg), 0) == -1) {
      std::cerr << "Server: Could not receive temperature to client" << std::endl;
      quit();
    }
  }

  quit(0);
}
