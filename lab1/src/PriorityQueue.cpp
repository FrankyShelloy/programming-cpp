#include "PriorityQueue.h"
#include <algorithm>
#include <stdexcept>
#include <sstream>

PriorityQueue::PriorityQueue() {
  is_max_heap_ = true;
}

PriorityQueue::PriorityQueue(bool max_heap) {
  is_max_heap_ = max_heap;
}

PriorityQueue::PriorityQueue(const std::vector<int>& data, bool max_heap) {
  heap_ = data;
  is_max_heap_ = max_heap;
  if (heap_.size() == 0) {
    return;
  }
  size_t start_index = (heap_.size() - 2) / 2;
  size_t i = start_index;
  while (true) {
    sift_down(i);
    if (i == 0) {
      break;
    }
    i = i - 1;
  }
}

PriorityQueue::PriorityQueue(const PriorityQueue& other) {
  heap_ = other.heap_;
  is_max_heap_ = other.is_max_heap_;
}

PriorityQueue::PriorityQueue(PriorityQueue&& other) noexcept {
  heap_ = std::move(other.heap_);
  is_max_heap_ = other.is_max_heap_;
}

PriorityQueue& PriorityQueue::operator=(const PriorityQueue& other) {
  if (this == &other) {
    return *this;
  }
  heap_ = other.heap_;
  is_max_heap_ = other.is_max_heap_;
  return *this;
}

PriorityQueue& PriorityQueue::operator=(PriorityQueue&& other) noexcept {
  if (this == &other) {
    return *this;
  }
  heap_ = std::move(other.heap_);
  is_max_heap_ = other.is_max_heap_;
  other.is_max_heap_ = true;
  return *this;
}

bool PriorityQueue::compare(int a, int b) const {
  if (is_max_heap_) {
    return a > b;
  } else {
    return a < b;
  }
}

void PriorityQueue::sift_up(size_t index) {
  while (index > 0) {
    size_t parent_index = (index - 1) / 2;
    if (compare(heap_[index], heap_[parent_index])) {
      std::swap(heap_[parent_index], heap_[index]);
      index = parent_index;
    } else {
      break;
    }
  }
}

void PriorityQueue::sift_down(size_t index) {
  size_t n = heap_.size();
  while (true) {
    size_t left = 2 * index + 1;
    size_t right = 2 * index + 2;
    size_t best = index;

    if (left < n) {
      if (compare(heap_[left], heap_[best])) {
        best = left;
      }
    }

    if (right < n) {
      if (compare(heap_[right], heap_[best])) {
        best = right;
      }
    }

    if (best == index) {
      break;
    }

    std::swap(heap_[index], heap_[best]);
    index = best;
  }
}

void PriorityQueue::push(int x) {
  heap_.push_back(x);
  sift_up(heap_.size() - 1);
}

void PriorityQueue::pop() {
  if (heap_.size() == 0) {
    throw std::runtime_error("PriorityQueue is empty");
  }
  std::swap(heap_[0], heap_[heap_.size() - 1]);
  heap_.pop_back();
  if (heap_.size() > 0) {
    sift_down(0);
  }
}

const int& PriorityQueue::top() const {
  if (heap_.size() == 0) {
    throw std::runtime_error("PriorityQueue is empty");
  }
  return heap_[0];
}

bool PriorityQueue::empty() const noexcept {
  return heap_.size() == 0;
}

size_t PriorityQueue::size() const noexcept {
  return heap_.size();
}

void PriorityQueue::clear  () noexcept{
  heap_.clear();
}

void PriorityQueue::reserve(size_t n) {
  heap_.reserve(n);
}

bool PriorityQueue::is_max_heap() const noexcept{
  return is_max_heap_;
}

bool PriorityQueue::operator==(const PriorityQueue& other) const noexcept {
  if (is_max_heap_ != other.is_max_heap_) {
    return false;
  }
  if (heap_.size() != other.heap_.size()) {
    return false;
  }

  PriorityQueue copy1(*this);
  PriorityQueue copy2(other);

  while (copy1.empty() == false) {
    if (copy1.top() != copy2.top()) {
      return false;
    }
    copy1.pop();
    copy2.pop();
  }

  return true;
}

bool PriorityQueue::operator!=(const PriorityQueue& other) const noexcept{
  return !(*this == other);
}

std::ostream& operator<<(std::ostream& os, const PriorityQueue& pq) {
  os << "[";
  for (size_t i = 0; i < pq.heap_.size(); i++) {
    if (i > 0) {
      os << ", ";
    }
    os << pq.heap_[i];
  }
  os << "]";
  return os;
}