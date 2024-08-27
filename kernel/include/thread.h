//
// Created by irrl on 8/26/24.
//

#ifndef GLADOS_THREAD_H
#define GLADOS_THREAD_H

#include "stdint.h"
#include "stdbool.h"
#include "kernel.h"

#define THREAD_STACK_SIZE 0x1000


typedef struct thread {
  uint64_t id;
  cpu_state_t state;
  void (*entry_point)(void);
  void* stack;
} thread_t;

thread_t* create_thread(void (*entry_point)(void));

void delete_thread(uint64_t id);

thread_t* get_thread(uint64_t id);

void handle_threads();
#endif  //GLADOS_THREAD_H
