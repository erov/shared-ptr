#pragma once
#include <cstddef>
#include <cstdlib>
#include <memory>


template <typename T>
class shared_ptr;


class control_block {
public:
  control_block() = default;
  virtual ~control_block() = default;

  void inc_strong_counter();
  void dec_strong_counter();
  void inc_weak_counter();
  void dec_weak_counter();
  size_t use_count();

private:
  size_t strong_counter{0};
  size_t weak_counter{0};

  virtual void delete_object() = 0;
};

template <typename T, typename D>
class regular_control_block : public control_block {
public:
  regular_control_block(T* ptr, D deleter) : ptr(ptr), deleter(std::move(deleter)) {}
  ~regular_control_block() = default;

private:
  T* ptr;
  [[no_unique_address]] D deleter;

  void delete_object() override {
    deleter(ptr);
  }
};


template <typename T>
class inplace_control_block : public control_block {
public:
  template <typename... Args>
  inplace_control_block(Args&&... args) {
    new (&data) T(std::forward<Args>(args)...);
  }
  ~inplace_control_block() = default;

  friend class shared_ptr<T>;

private:
  std::aligned_storage_t<sizeof(T), alignof(T)> data;

  void delete_object() override {
    get_ptr()->~T();
  }

  T* get_ptr() {
    return reinterpret_cast<T*>(&data);
  }
};

