#include "control-block.h"

void control_block::inc_strong_counter() {
  ++strong_counter;
  inc_weak_counter();
}

void control_block::dec_strong_counter() {
  if (--strong_counter == 0) {
    delete_object();
  }
  dec_weak_counter();
}

void control_block::inc_weak_counter() {
  ++weak_counter;
}

void control_block::dec_weak_counter() {
  if (--weak_counter == 0) {
    delete this;
  }
}

size_t control_block::use_count() {
  return strong_counter;
}
