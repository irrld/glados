//
// Created by irrl on 8/26/24.
//

#ifndef GLADOS_THREAD_H
#define GLADOS_THREAD_H

#include "kernel.h"
#include "stddef.h"

typedef struct thread {
  uint64_t id;
  cpu_state_t state;
  void (*entry_point)(void);
  void* stack_pointer;
  bool sleeping;
} thread_t;

thread_t* create_thread(void (*entry_point)(void));

void delete_thread(uint64_t id);

thread_t* get_thread(uint64_t id);

thread_t* current_thread();

void block_current_thread();
void wake_up_thread(thread_t* thread);

// Will save the cpu state, without the RIP register
extern void save_cpu_state();
extern void load_cpu_state(cpu_state_t* state);// __attribute__((noreturn));
void force_switch_thread(thread_t* thread);// __attribute__((noreturn));

void handle_threads();
#endif  //GLADOS_THREAD_H
