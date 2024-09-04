//
// Created by irrl on 9/1/24.
//

#ifndef GLADOS_MUTEX_H
#define GLADOS_MUTEX_H

#include "stddef.h"

typedef struct wait_queue_node {
  struct thread* thread;                  // Pointer to the waiting thread
  struct wait_queue_node* next;           // Pointer to the next node in the queue
} wait_queue_node_t;

typedef struct wait_queue {
  wait_queue_node_t* head;                // Head of the linked list
  wait_queue_node_t* tail;                // Tail of the linked list
} wait_queue_t;

typedef struct mutex {
  bool locked;                  // Lock state (0 = unlocked, 1 = locked)
  struct thread* owner;        // Pointer to the thread that currently holds the lock
  wait_queue_t waiters;  // Queue of threads waiting for the lock
} mutex_t;

void mutex_init();
void mutex_deinit();

void mutex_lock(mutex_t* mutex);
void mutex_unlock(mutex_t* mutex);

#endif  //GLADOS_MUTEX_H
