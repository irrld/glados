//
// Created by irrl on 8/26/24.
//

#include "thread.h"
#include "malloc.h"
#include "string.h"

uint64_t thread_counter_ = 0;

typedef struct entry {
  thread_t* thread;
  struct entry* previous;
  struct entry* next;
} entry_t;
entry_t* thread_head_ = NULL;

void* allocate_stack() {
  return malloc(THREAD_STACK_SIZE) + THREAD_STACK_SIZE - 1;
}

void delete_stack(void* ptr) {
  free(ptr);
}

void on_thread_delete(thread_t* thread) {
  delete_stack(thread->stack);
}

void on_thread_create(thread_t* thread) {
  thread->stack = allocate_stack();
  memset(&thread->state, NULL, sizeof(cpu_state_t));
  thread->state.rip = (uint64_t) thread->entry_point;
  thread->state.rsp = (uint64_t) thread->stack;
  thread->state.rbp = thread->state.rsp;
  thread->state.rflags = 0x246; // Enable interrupts
  thread->state.cs = 0x08; // Code segment selector for 64 bit
  thread->state.ss = 0x10; // Stack segment selector for 64 bit
}

thread_t* create_thread(void (*entry_point)(void)) {
  thread_t* thread = malloc(sizeof(thread_t));
  thread->entry_point = entry_point;
  thread->id = ++thread_counter_;
  entry_t* entry = malloc(sizeof(entry_t));
  entry->thread = thread;
  entry->previous = thread_head_; // Head is the previous node now
  entry->next = NULL;
  thread_head_ = entry; // Set the head to the new entry
  on_thread_create(thread);
  return thread;
}

thread_t* current_thread_ = NULL;

void switch_to_thread(cpu_state_t* current, thread_t* thread) {
  memcpy(current, &thread->state, sizeof(cpu_state_t));
  current_thread_ = thread;
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
    free(node->thread);
    if (node->previous) { // Has nodes before this
      // Remove this node from the chain
      node->previous->next = node->next;
      free(node);
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

void lookup_switch_thread(entry_t* node) {
  if (!node) {
    return;
  }
  if (node->thread == current_thread_) {
    lookup_switch_thread(node->previous);
    return;
  }
  cpu_state_t* saved_state = get_saved_cpu_state();
  switch_to_thread(saved_state, node->thread);
}

void handle_threads() {
  lookup_switch_thread(thread_head_);
}


