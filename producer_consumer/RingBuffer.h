//
// Created by Peter on 2/1/2018.
//

#ifndef CSCI411_RINGBUFFER_H
#define CSCI411_RINGBUFFER_H

#include <cstdlib>
#include <thread>
#include <mutex>
#include <pthread.h>
#include <semaphore.h>

template<typename T>
class RingBuffer {
 private:
  const unsigned int BUFFER_SIZE;
  T *buffer;

  int insertIdx = 0, removeIdx = 0;
  std::mutex buffer_mutex;
  sem_t emptySlots, fullSlots;

  /// Initializes the buffer and semaphores
  void initialize() {
    this->buffer = new T[BUFFER_SIZE];
    sem_init(&fullSlots, 0, 0);
    sem_init(&emptySlots, 0, BUFFER_SIZE);
  }

 public:
  /// Creates a new RingBuffer of size 10
  RingBuffer() : BUFFER_SIZE(10) {
    initialize();
  };

  /// Creates a new RingBuffer with the specified size
  explicit RingBuffer(unsigned int bufferSize) : BUFFER_SIZE(bufferSize) {
    initialize();
  };

  ~RingBuffer() {
    delete[] buffer;
  }

  /// Inserts an item into the ring buffer.
  /// Blocks the current thread until there is room in the buffer.
  void enqueue_sync(T item) {
    // Wait if there is no space left
    sem_wait(&emptySlots);

    // Insert the item
    buffer_mutex.lock();
    buffer[insertIdx] = item;
    insertIdx++;
    if (insertIdx == BUFFER_SIZE) insertIdx = 0;
    buffer_mutex.unlock();

    // Increment the count of the number of items
    sem_post(&fullSlots);
  };

  /// Removes an item from the ring buffer.
  /// Blocks the current thread until there is an item in the buffer.
  ///
  /// \return the earliest item in the buffer
  T dequeue_sync() {
    // Wait if there are no items in the buffer
    sem_wait(&fullSlots);

    // Get the item
    buffer_mutex.lock();
    T result = buffer[removeIdx];
    removeIdx++;
    if (removeIdx == BUFFER_SIZE) removeIdx = 0;
    buffer_mutex.unlock();

    // Increment the number of empty slots
    sem_post(&emptySlots);

    return result;
  };
};

#endif //CSCI411_RINGBUFFER_H
