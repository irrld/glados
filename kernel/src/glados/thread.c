//
// Created by irrl on 8/26/24.
//

#include "glados/thread.h"
#include "glados/kmalloc.h"
#include "glados/string.h"

typedef struct entry {
  thread_t* thread;
  struct entry* previous;
  struct entry* next;
} entry_t;

uint64_t thread_counter_ = 0;
thread_t* current_thread_ = NULL;
entry_t* thread_head_ = NULL;

void* allocate_stack() {
  size_t stack_size = 0x800000;
  void* stack_base = kmalloc(stack_size);

  if (stack_base == NULL) {
    kernel_panic("Could not allocate stack for thread");
  }

  // Initialize the stack pointer to the top of the allocated stack
  return stack_base + stack_size - 0x10;
}

void delete_stack(void* ptr) {
  kfree(ptr);
}

void on_thread_delete(thread_t* thread) {
  delete_stack(thread->stack_pointer);
}

void on_thread_create(thread_t* thread) {
  thread->stack_pointer = allocate_stack();
  thread->sleeping = false;
  memset(&thread->state, NULL, sizeof(cpu_state_t));
  thread->state.rip = (uint64_t) thread->entry_point;
  thread->state.rbp = (uint64_t) thread->stack_pointer;
  thread->state.rsp = thread->state.rbp;
  thread->state.rflags = 0x202; // Enable interrupts
  thread->state.cs = 0x08; // Code segment selector for 64 bit
  thread->state.ss = 0x10; // Stack segment selector for 64 bit
}

thread_t* create_thread(void (*entry_point)(void)) {
  thread_t* thread = kmalloc(sizeof(thread_t));
  if (!thread) {
    kernel_panic("Memory allocation error!");
  }
  thread->sleeping = false;
  thread->entry_point = entry_point;
  thread->id = ++thread_counter_;
  entry_t* entry = kmalloc(sizeof(entry_t));
  entry->thread = thread;
  entry->previous = thread_head_; // Head is the previous node now
  entry->next = NULL;
  thread_head_ = entry; // Set the head to the new entry
  on_thread_create(thread);
  return thread;
}

void switch_to_thread(cpu_state_t* current, thread_t* thread) {
  memcpy(current, &thread->state, sizeof(cpu_state_t));
  current_thread_ = thread;
}

void force_switch_thread(thread_t* thread) {
  current_thread_ = thread; // Update the current thread
  load_cpu_state(&thread->state); // Switch to thread
  // This function never returns, it will jump to the target
}

// If there are too many threads, this might fill the stack while finding the
// requested thread. Might replace it later. Same applies to lookup_thread too
void lookup_delete_thread(entry_t* node, uint64_t id) {
  if (!node) {
    return;
  }
  if (node->thread->id == id) {
    // Notify
    on_thread_delete(node->thread);
    kfree(node->thread);
    if (node->previous) { // Has nodes before this
      // Remove this node from the chain
      node->previous->next = node->next;
      kfree(node);
    } else { // Doesn't have nodes before this
      // Make the next node the head
      thread_head_ = node->next;
    }
    // Thread was found and deleted. We can leave now.
    return;
  }
  lookup_delete_thread(node->previous, id);
}

void delete_thread(uint64_t id) {
  lookup_delete_thread(thread_head_, id);
}

thread_t* lookup_thread(entry_t* node, uint64_t id) {
  if (!node) {
    return NULL;
  }
  if (node->thread->id == id) {
    // Thread is found, return it
    return node->thread;
  }
  return lookup_thread(node->next, id);
}

thread_t* get_thread(uint64_t id) {
  return lookup_thread(thread_head_, id);
}

thread_t* current_thread() {
  return current_thread_;
}

thread_t* lookup_thread_to_switch(entry_t* node) {
  if (!node) {
    return NULL;
  }
  if (node->thread == current_thread_ || node->thread->sleeping) {
    return lookup_thread_to_switch(node->previous);
  }
  return node->thread;
}

void block_current_thread() {
  if (!current_thread_) {
    return;
  }
  disable_interrupts();
  current_thread_->sleeping = true;
  save_cpu_state();
  cpu_state_t* saved_state = get_saved_cpu_state();
  memcpy(&current_thread_->state, saved_state, sizeof(cpu_state_t));
  // Set IF to enable interrupts for next time
  current_thread_->state.rflags = current_thread_->state.rflags | 0x200;
  current_thread_->state.rip = (uint64_t) &&resume;
   thread_t* thread = lookup_thread_to_switch(thread_head_);
  if (thread) {
    force_switch_thread(thread);
  } else {
    current_thread_->sleeping = true;
    enable_interrupts();
  }
resume:
  return;
}

void wake_up_thread(thread_t* thread) {
  thread->sleeping = false;
}

static bool lock = false;
void handle_threads() {
  if (lock) {
    kernel_panic("WTF!");
  }
  lock = true;
  thread_t* thread = lookup_thread_to_switch(thread_head_);
  if (thread) {
    cpu_state_t* saved_state = get_saved_cpu_state();
    switch_to_thread(saved_state, thread);
  }
  lock = false;
}
