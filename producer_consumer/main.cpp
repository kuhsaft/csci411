/*
 * Peter Nguyen
 * CSCI 411 - Producer Consumer
 *
 * Compile with `-std=c++11`
 */

#include <iostream>
#include <chrono>

#include "RingBuffer.h"
#include "ProducerConsumer.h"

void print_usage() {
  std::cout << "Usage: producer_consumer [SECONDS] [NUM_PRODUCERS] [NUM_CONSUMERS]\n"
            << "    SECONDS       - The number of seconds to run\n"
            << "    NUM_PRODUCERS - The number of producers\n"
            << "    NUM_CONSUMERS - The number of consumers\n";
}

int main(int argc, char *argv[]) {
  int sleep_seconds = 2,
      num_producer_threads = 4,
      num_consumer_threads = 4;

  if (argc > 4) {
    std::cerr << "Error: Too many arguments!\n\n";
    print_usage();
    return 1;
  }

  // Parse arguments
  try {
    switch (argc) {
      case 4:num_consumer_threads = std::stoi(argv[3]);
      case 3:num_producer_threads = std::stoi(argv[2]);
      case 2:sleep_seconds = std::stoi(argv[1]);
      default:break;
    }

    // Arguments must be positive
    if (sleep_seconds < 0 || num_consumer_threads < 0 || num_producer_threads < 0)
      throw std::exception();
  } catch (std::exception &e) {
    std::cerr << "Error: Invalid arguments!\n\n";
    print_usage();
    return 1;
  }

  // Create a new ring buffer
  RingBuffer<short> ringBuffer(10);

  // Create a producer consumer system
  ProducerConsumer<short> producerConsumer(
      std::ref(ringBuffer),
      static_cast<size_t>(num_producer_threads),
      static_cast<size_t>(num_consumer_threads)
  );

  // Start the system
  producerConsumer.start();

  // Sleep the main thread
  std::chrono::seconds sleep_duration(sleep_seconds);
  std::this_thread::sleep_for(sleep_duration);

  return 0;
}
