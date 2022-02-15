#pragma once
#include "control-block.h"
#include <cstddef>
#include <cstdlib>
#include <memory>
#include <type_traits>
#include <utility>


template <typename T>
class weak_ptr;

template<typename T, typename... Args>
shared_ptr<T> make_shared(Args&&... args);


template <typename T>
class shared_ptr {
public:
  shared_ptr() noexcept = default;

  shared_ptr(std::nullptr_t) noexcept {}

  template <typename Q, typename D = std::default_delete<Q>, typename = std::enable_if_t<std::is_convertible_v<Q*, T*>>>
  shared_ptr(Q* ptr, D&& deleter = D()) try
      : block(new regular_control_block<Q, D>(ptr, std::forward<D>(deleter))),
        ptr(static_cast<T*>(ptr)) {
    block->inc_strong_counter();
  } catch (...) {
    deleter(ptr);
    throw;
  }

  template <typename Q>
  shared_ptr(const shared_ptr<Q>& other, T* ptr) noexcept : block(other.block), ptr(ptr) {
    if (block) {
      block->inc_strong_counter();
    }
  }

  shared_ptr(const shared_ptr& other) noexcept : block(other.block), ptr(other.ptr) {
    if (block) {
      block->inc_strong_counter();
    }
  }

  template <typename Q, typename = std::enable_if_t<std::is_convertible_v<Q*, T*>>>
  shared_ptr(const shared_ptr<Q>& other) noexcept: block(other.block), ptr(static_cast<T*>(other.ptr)) {
    if (block) {
      block->inc_strong_counter();
    }
  }

  shared_ptr(shared_ptr&& other) noexcept : block(other.block), ptr(other.ptr) {
    other.block = nullptr;
    other.ptr = nullptr;
  }

  template <typename Q, typename = std::enable_if_t<std::is_convertible_v<Q*, T*>>>
  shared_ptr(shared_ptr<Q>&& other) noexcept : block(other.block), ptr(static_cast<T*>(other.ptr)) {
    other.block = nullptr;
    other.ptr = nullptr;
  }

  ~shared_ptr() {
    if (block) {
      block->dec_strong_counter();
    }
  }


  shared_ptr& operator=(const shared_ptr& other) noexcept {
    if (this != &other) {
      shared_ptr(other).swap(*this);
    }
    return *this;
  }

  template <typename Q, typename = std::enable_if_t<std::is_convertible_v<Q*, T*>>>
  shared_ptr& operator=(const shared_ptr<Q>& other) noexcept {
    shared_ptr(other).swap(*this);
    return *this;
  }

  shared_ptr& operator=(shared_ptr&& other) noexcept {
    if (this != &other) {
      shared_ptr(std::move(other)).swap(*this);
    }
    return *this;
  }

  template <typename Q, typename = std::enable_if_t<std::is_convertible_v<Q*, T*>>>
  shared_ptr& operator=(shared_ptr<Q>&& other) noexcept {
    shared_ptr(std::move(other)).swap(*this);
    return *this;
  }


  T* get() const noexcept {
    return ptr;
  }

  operator bool() const noexcept {
    return get() != nullptr;
  }

  T& operator*() const noexcept {
    return *get();
  }

  T* operator->() const noexcept {
    return get();
  }


  size_t use_count() const noexcept {
    return block ? block->use_count() : 0;
  }

  void reset() noexcept {
    shared_ptr().swap(*this);
  }

  template <typename Q, typename D = std::default_delete<Q>, typename = std::enable_if_t<std::is_convertible_v<Q*, T*>>>
  void reset(Q* new_ptr, D&& new_deleter = D()) {
    shared_ptr(new_ptr, std::forward<D>(new_deleter)).swap(*this);
  }


  friend bool operator==(const shared_ptr& lhs, const shared_ptr& rhs) {
    return lhs.ptr == rhs.ptr;
  }

  friend bool operator!=(const shared_ptr& lhs, const shared_ptr& rhs) {
    return !(lhs == rhs);
  }

  template <typename Q>
  friend class shared_ptr;

  template <typename Q>
  friend class weak_ptr;

  template <typename Q, typename... Args>
  friend shared_ptr<Q> make_shared(Args&&... args);

private:
  control_block* block{nullptr};
  T* ptr{nullptr};

  void swap(shared_ptr &other) noexcept {
    std::swap(block, other.block);
    std::swap(ptr, other.ptr);
  }

  shared_ptr(control_block* block, T* ptr) : block(block), ptr(ptr) {
    block->inc_strong_counter();
  }

  shared_ptr(inplace_control_block<T>* block) : block(block), ptr(block->get_ptr()) {
    block->inc_strong_counter();
  }
};


template <typename T>
class weak_ptr {
public:
  weak_ptr() noexcept = default;

  weak_ptr(const weak_ptr& other) noexcept : block(other.block), ptr(other.ptr) {
    if (block) {
      block->inc_weak_counter();
    }
  }

  template <typename Q, typename = std::enable_if_t<std::is_convertible_v<Q*, T*>>>
  weak_ptr(const weak_ptr<Q>& other) noexcept : block(other.block), ptr(static_cast<T*>(other.ptr)) {
    if (block) {
      block->inc_weak_counter();
    }
  }

  template <typename Q, typename = std::enable_if_t<std::is_convertible_v<Q*, T*>>>
  weak_ptr(const shared_ptr<Q>& other) noexcept : block(other.block), ptr(static_cast<T*>(other.ptr)) {
    if (block) {
      block->inc_weak_counter();
    }
  }

  weak_ptr(weak_ptr&& other) noexcept : block(other.block), ptr(other.ptr) {
    other.block = nullptr;
    other.ptr = nullptr;
  }

  template <typename Q, typename = std::enable_if_t<std::is_convertible_v<Q*, T*>>>
  weak_ptr(weak_ptr<Q>&& other) noexcept : block(other.block), ptr(static_cast<T*>(other.ptr)) {
    other.block = nullptr;
    other.ptr = nullptr;
  }

  ~weak_ptr() {
    if (block) {
      block->dec_weak_counter();
    }
  }


  weak_ptr& operator=(const weak_ptr& other) noexcept {
    if (this != &other) {
      weak_ptr(other).swap(*this);
    }
    return *this;
  }

  template <typename Q, typename = std::enable_if_t<std::is_convertible_v<Q*, T*>>>
  weak_ptr& operator=(const weak_ptr<Q>& other) noexcept {
    weak_ptr(other).swap(*this);
    return *this;
  }

  template <typename Q, typename = std::enable_if_t<std::is_convertible_v<Q*, T*>>>
  weak_ptr& operator=(const shared_ptr<Q>& other) noexcept {
    weak_ptr(other).swap(*this);
    return *this;
  }

  weak_ptr& operator=(weak_ptr&& other) noexcept {
    if (this != &other) {
      weak_ptr(std::move(other)).swap(*this);
    }
    return *this;
  }

  template <typename Q, typename = std::enable_if_t<std::is_convertible_v<Q*, T*>>>
  weak_ptr& operator=(weak_ptr<Q>&& other) noexcept {
    weak_ptr(std::move(other)).swap(*this);
    return *this;
  }


  shared_ptr<T> lock() const noexcept {
    if (block) {
      return block->use_count() == 0 ? shared_ptr<T>() : shared_ptr<T>(block, ptr);
    }
    return shared_ptr<T>();
  }

private:
  control_block* block{nullptr};
  T* ptr{nullptr};

  void swap(weak_ptr &other) noexcept {
    std::swap(block, other.block);
    std::swap(ptr, other.ptr);
  }
};

template <typename T, typename... Args>
shared_ptr<T> make_shared(Args&&... args) {
  return shared_ptr<T>(new inplace_control_block<T>(std::forward<Args>(args)...));
}
