//
// Created by irrl on 9/1/24.
//

#include "glados/mutex.h"
#include "glados/kmalloc.h"
#include "glados/thread.h"

void wait_queue_init(wait_queue_t* queue) {
  queue->head = NULL;
  queue->tail = NULL;
}

void wait_queue_add(wait_queue_t* queue, thread_t* thread) {
  wait_queue_node_t* node = kmalloc(sizeof(wait_queue_node_t));
  node->thread = thread;
  node->next = NULL;

  if (queue->tail) {
    queue->tail->next = node;   // Append to the end of the list
  } else {
    queue->head = node;         // The queue is empty
  }
  queue->tail = node;             // Update the tail
}

thread_t* wait_queue_remove(wait_queue_t *queue) {
  if (!queue->head) {
    return NULL;  // The queue is empty
  }

  wait_queue_node_t* node = queue->head;
  thread_t* thread = node->thread;

  queue->head = node->next;        // Move to the next node
  if (!queue->head) {
    queue->tail = NULL;          // Queue is now empty
  }

  kfree(node);
  return thread;
}

bool wait_queue_is_empty(wait_queue_t* queue) {
  return (queue->head == NULL);
}

void mutex_lock(mutex_t* mutex) {
  disable_interrupts();
  if (mutex->locked) {
    if (mutex->owner == current_thread()) {
      goto finish; // Already locked by this thread, skip
    }
    // Mutex is already locked, add current thread to wait queue and block it
    wait_queue_add(&mutex->waiters, current_thread());
    block_current_thread();  // Kernel function to put the current thread to sleep
  } else {
    // Mutex is not locked, acquire the lock
    mutex->locked = true;
    mutex->owner = current_thread();
  }
finish:
  enable_interrupts();
}

void mutex_unlock(mutex_t* mutex) {
  disable_interrupts();
  if (mutex->owner != current_thread()) {
    goto finish;
  }
  if (wait_queue_is_empty(&mutex->waiters)) {
    // No threads are waiting, simply release the lock
    mutex->locked = false;
    mutex->owner = NULL;
  } else {
    // Wake up the next waiting thread
    thread_t* next_thread = wait_queue_remove(&mutex->waiters);
    mutex->owner = next_thread;
    wake_up_thread(next_thread);
  }
finish:
  enable_interrupts();
}