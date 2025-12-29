#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H

#include <vector>
#include <iostream>
#include <cstddef>

class PriorityQueue {
public:
  using value_type = int;
  using size_type = std::size_t;


  PriorityQueue();
  explicit PriorityQueue(bool max_heap);
  explicit PriorityQueue(const std::vector<value_type>& data, bool max_heap = true);
  ~PriorityQueue() = default;

  PriorityQueue(const PriorityQueue& other);
  PriorityQueue(PriorityQueue&& other) noexcept;

  PriorityQueue& operator=(const PriorityQueue& other);
  PriorityQueue& operator=(PriorityQueue&& other) noexcept;


  void push(value_type x);
  void pop();
  const value_type& top() const;

  bool empty() const noexcept;
  size_type size() const noexcept;
  void clear() noexcept;
  void reserve(size_type n);

  bool is_max_heap() const noexcept;


  bool operator==(const PriorityQueue& rhs) const noexcept;
  bool operator!=(const PriorityQueue& rhs) const noexcept;

  friend std::ostream& operator<<(std::ostream& os, const PriorityQueue& pq);

private:
  std::vector<value_type> heap_;
  bool is_max_heap_;

  void sift_up(size_type index);
  void sift_down(size_type index);
  bool compare(value_type a, value_type b) const;
};

#endif // PRIORITY_QUEUE_H