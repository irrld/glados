//
// Created by irrl on 8/26/24.
//

#ifndef GLADOS_THREAD_H
#define GLADOS_THREAD_H

#include "glados/kernel.h"
#include "glados/stddef.h"

typedef struct kthread {
  uint64_t id;
  cpu_state_t state;
  void (*entry_point)(void);
  void* stack_pointer;
  bool sleeping;
} kthread_t;

kthread_t* create_kthread(void (*entry_point)(void));

void delete_kthread(uint64_t id);

kthread_t* get_kthread(uint64_t id);

kthread_t* current_kthread();

void block_current_kthread();
void wake_up_kthread(kthread_t* thread);

// Will save the cpu state, without the RIP register
extern void save_cpu_state();
extern void load_cpu_state(cpu_state_t* state);// __attribute__((noreturn));
void force_switch_kthread(kthread_t* thread);// __attribute__((noreturn));

void handle_kthreads();
#endif  //GLADOS_THREAD_H
