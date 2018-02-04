//
// Created by Peter on 2/4/2018.
//

#ifndef CSCI411_PRODUCERCONSUMER_H
#define CSCI411_PRODUCERCONSUMER_H

#include <iostream>
#include <chrono>
#include <thread>
#include <ctime>
#include <random>
#include <sstream>
#include "RingBuffer.h"

template<typename T>
class ProducerConsumer {
 private:
  std::reference_wrapper<RingBuffer<T>> ringBuffer;
  const size_t NUM_PRODUCER_THREADS, NUM_CONSUMER_THREADS;

  /// Returns a static thread local RNG
  static thread_local std::mt19937 rng() {
    return std::mt19937(
        clock() + std::hash<std::thread::id>()(std::this_thread::get_id())
    );
  }

  /// Sleeps thread for 250-500 ms
  void sleep_random() {
    // Random number generation
    std::mt19937 generator = rng();
    static thread_local std::uniform_int_distribution<int> sleep_dist(250, 500);

    std::this_thread::sleep_for(std::chrono::milliseconds(sleep_dist(generator)));
  }

  /// Inserts items into RingBuffer
  ///
  /// \param producer_id the id of the producer
  void producer(size_t producer_id) {
    // Random number generation
    std::mt19937 generator = rng();
    std::uniform_int_distribution<T> item_dist(
        std::numeric_limits<T>::min(),
        std::numeric_limits<T>::max()
    );

    // Buffer item
    T item;

    // Output text


    while (true) {
      // Sleep
      sleep_random();

      // Generate a random number
      item = item_dist(generator);

      // Try to insert item
      try {
        ringBuffer.get().enqueue_sync(item);

        // Print logging
        std::stringstream message;
        message << "Producer #" << producer_id
                << ": Produced item " << item << std::endl;
        std::cout << message.str();
      } catch (std::exception &e) {
        std::stringstream message;
        message << "Producer #" << producer_id
                << ": Error producing value " << item << std::endl;
        std::cerr << message.str();
      }
    }
  }

  /// Removes items from RingBuffer
  ///
  /// \param consumer_id the id of the consumer
  void consumer(size_t consumer_id) {
    // Buffer item
    T item;

    while (true) {
      // Sleep
      sleep_random();

      // Try to consume item
      try {
        item = ringBuffer.get().dequeue_sync();

        // Print logging
        std::stringstream message;
        message << "Consumer #" << consumer_id
                << ": Consumed item " << item << std::endl;
        std::cout << message.str();
      } catch (std::exception &e) {
        std::stringstream message;
        message << "Consumer #" << consumer_id
                << ": Error trying to consume item" << std::endl;
        std::cout << message.str();
      }
    }
  }

 public:
  /// Creates a producer-consumer system
  ///
  /// \param ringBuffer the shared ring buffer
  /// \param numProducerThreads the number of producers
  /// \param numConsumerThreads the number of consumers
  ProducerConsumer(
      std::reference_wrapper<RingBuffer<T>> ringBuffer,
      size_t numProducerThreads,
      size_t numConsumerThreads
  ) : ringBuffer(ringBuffer),
      NUM_PRODUCER_THREADS(numProducerThreads),
      NUM_CONSUMER_THREADS(numConsumerThreads) {};

  /// Starts the producer consumer system
  void start() {
    // Create the producers
    for (size_t i = 0; i < NUM_PRODUCER_THREADS; ++i) {
      try {
        std::thread(&ProducerConsumer<T>::producer, this, i).detach();
      } catch (std::system_error &e) {
        std::cerr << "Error: Could not create producer #" << i << std::endl;
      }
    }

    // Create the consumers
    for (size_t i = 0; i < NUM_CONSUMER_THREADS; ++i) {
      try {
        std::thread(&ProducerConsumer<T>::consumer, this, i).detach();
      } catch (std::system_error &e) {
        std::cerr << "Error: Could not create producer #" << i << std::endl;
      }
    }
  }
};

#endif //CSCI411_PRODUCERCONSUMER_H
