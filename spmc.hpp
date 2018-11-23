#pragma once

#include <cstddef>
#include <cstdlib>

#include <atomic>

template <class Type> class spmc256 {
private:
  typedef uint8_t    IndexType;
  typedef char cache_line_pad_t[128]; // Some CPUs could have 128 cache line in LLC

  IndexType               tail; // Not atomic, it is a spmc
  cache_line_pad_t        _pad1;
  std::atomic<IndexType>  head;
  cache_line_pad_t        _pad2;
  Type                    array[256];

public:
  spmc256()
      : tail(0)
      , head(0) {
  }
  virtual ~spmc256() {
  }

  Type *getTailRef() {
    return &array[tail];
  }

  int size() const { // WARNING: NOT ATOMIC. Can give WEIRD RESULTS
    if (tail>head)
      return tail - head;
    else
      return 256 - (head-tail);
  }

  bool enqueue(const Type &item_) {
    // Not thread safe to insert (sp)
    if (full())
      return false;

    array[tail] = item_;
    tail++;

    return true;
  };

  bool full() const { return (tail+1) == head; }
  bool empty() const { return tail == head; }

  bool dequeue(Type &data) {
    for(;;) {
      IndexType head_copy = head.load(std::memory_order_acquire);
      if (head_copy == tail)
        return false;
      data = array[head_copy];
      if (head.compare_exchange_weak(head_copy, (IndexType)(head_copy + 1), std::memory_order_release))
        return true;
    }
  };
};


