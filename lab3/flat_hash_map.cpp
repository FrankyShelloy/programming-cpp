#include <algorithm>
#include <emmintrin.h>
#include <stdexcept>

#include "flat_hash_map.hpp"

template <class Key, class T, class Hash, class KeyEqual>
typename flat_hash_map<Key, T, Hash, KeyEqual>::value_type*
flat_hash_map<Key, T, Hash, KeyEqual>::Slot::ptr() {
  return reinterpret_cast<value_type*>(storage);
}

template <class Key, class T, class Hash, class KeyEqual>
const typename flat_hash_map<Key, T, Hash, KeyEqual>::value_type*
flat_hash_map<Key, T, Hash, KeyEqual>::Slot::ptr() const {
  return reinterpret_cast<const value_type*>(storage);
}

template <class Key, class T, class Hash, class KeyEqual>
typename flat_hash_map<Key, T, Hash, KeyEqual>::size_type
flat_hash_map<Key, T, Hash, KeyEqual>::CalculateCapacity(size_type requested) {
  if (requested == 0) return 16;
  
  size_type capacity = 16;
  while (capacity < requested) {
    capacity *= 2;
  }
  return capacity;
}

template <class Key, class T, class Hash, class KeyEqual>
uint8_t flat_hash_map<Key, T, Hash, KeyEqual>::H2(size_t hash_value) {
  uint8_t h = static_cast<uint8_t>(hash_value >> 7);
  if (h >= kDeleted) {
    h = kDeleted - 1;
  }
  return h;
}

template <class Key, class T, class Hash, class KeyEqual>
typename flat_hash_map<Key, T, Hash, KeyEqual>::size_type
flat_hash_map<Key, T, Hash, KeyEqual>::FindSlot(const Key& key,
                                                 size_t hash_value) const {
  if (capacity_ == 0) return 0;

  const uint8_t h2_value = H2(hash_value);
  size_type index = hash_value & (capacity_ - 1);
  const size_type start_index = index;

  while (true) {
    const size_type group_start = index & ~15;
    
    __m128i group = _mm_loadu_si128(
        reinterpret_cast<const __m128i*>(metadata_ + group_start));

    __m128i target = _mm_set1_epi8(h2_value);
    __m128i cmp_match = _mm_cmpeq_epi8(group, target);
    
    __m128i empty_vec = _mm_set1_epi8(kEmpty);
    __m128i cmp_empty = _mm_cmpeq_epi8(group, empty_vec);

    int match_mask = _mm_movemask_epi8(cmp_match);
    int empty_mask = _mm_movemask_epi8(cmp_empty);

    while (match_mask != 0) {
      int bit_pos = __builtin_ctz(match_mask);
      size_type slot_index = (group_start + bit_pos) & (capacity_ - 1);

      if (metadata_[slot_index] == h2_value &&
          equal_(slots_[slot_index].ptr()->first, key)) {
        return slot_index;
      }

      match_mask &= (match_mask - 1);
    }

    if (empty_mask != 0) {
      return capacity_;
    }

    index = (group_start + 16) & (capacity_ - 1);

    if (index == (start_index & ~15)) {
      break;
    }
  }

  return capacity_;
}

template <class Key, class T, class Hash, class KeyEqual>
typename flat_hash_map<Key, T, Hash, KeyEqual>::size_type
flat_hash_map<Key, T, Hash, KeyEqual>::FindInsertSlot(
    const Key& key, size_t hash_value) const {
  if (capacity_ == 0) return 0;

  const uint8_t h2_value = H2(hash_value);
  size_type index = hash_value & (capacity_ - 1);
  size_type first_deleted = capacity_;
  const size_type start_index = index;

  while (true) {
    const size_type group_start = index & ~15;
    
    __m128i group = _mm_loadu_si128(
        reinterpret_cast<const __m128i*>(metadata_ + group_start));

    __m128i target = _mm_set1_epi8(h2_value);
    __m128i cmp_match = _mm_cmpeq_epi8(group, target);
    
    __m128i empty_vec = _mm_set1_epi8(kEmpty);
    __m128i cmp_empty = _mm_cmpeq_epi8(group, empty_vec);
    
    __m128i deleted_vec = _mm_set1_epi8(kDeleted);
    __m128i cmp_deleted = _mm_cmpeq_epi8(group, deleted_vec);

    int match_mask = _mm_movemask_epi8(cmp_match);
    int empty_mask = _mm_movemask_epi8(cmp_empty);
    int deleted_mask = _mm_movemask_epi8(cmp_deleted);

    while (match_mask != 0) {
      int bit_pos = __builtin_ctz(match_mask);
      size_type slot_index = (group_start + bit_pos) & (capacity_ - 1);

      if (metadata_[slot_index] == h2_value &&
          equal_(slots_[slot_index].ptr()->first, key)) {
        return slot_index;
      }

      match_mask &= (match_mask - 1);
    }

    if (first_deleted == capacity_ && deleted_mask != 0) {
      int bit_pos = __builtin_ctz(deleted_mask);
      first_deleted = (group_start + bit_pos) & (capacity_ - 1);
    }

    if (empty_mask != 0) {
      if (first_deleted != capacity_) {
        return first_deleted;
      }
      int bit_pos = __builtin_ctz(empty_mask);
      return (group_start + bit_pos) & (capacity_ - 1);
    }

    index = (group_start + 16) & (capacity_ - 1);

    if (index == (start_index & ~15)) {
      if (first_deleted != capacity_) {
        return first_deleted;
      }
      break;
    }
  }

  return capacity_;
}

template <class Key, class T, class Hash, class KeyEqual>
void flat_hash_map<Key, T, Hash, KeyEqual>::Allocate(size_type new_capacity) {
  slots_ = static_cast<Slot*>(::operator new(new_capacity * sizeof(Slot)));
  metadata_ = new uint8_t[new_capacity];
  std::fill(metadata_, metadata_ + new_capacity, kEmpty);
  capacity_ = new_capacity;
}

template <class Key, class T, class Hash, class KeyEqual>
void flat_hash_map<Key, T, Hash, KeyEqual>::Deallocate() {
  if (slots_) {
    for (size_type i = 0; i < capacity_; ++i) {
      if (metadata_[i] < kDeleted) {
        slots_[i].ptr()->~value_type();
      }
    }
    ::operator delete(slots_);
    delete[] metadata_;
  }
}

template <class Key, class T, class Hash, class KeyEqual>
void flat_hash_map<Key, T, Hash, KeyEqual>::RehashImpl(
    size_type new_capacity) {
  Slot* old_slots = slots_;
  uint8_t* old_metadata = metadata_;
  size_type old_capacity = capacity_;

  Allocate(new_capacity);
  size_ = 0;

  for (size_type i = 0; i < old_capacity; ++i) {
    if (old_metadata[i] < kDeleted) {
      value_type* old_val = old_slots[i].ptr();
      Key key_copy = std::move(const_cast<Key&>(old_val->first));
      mapped_type val_copy = std::move(old_val->second);
      old_val->~value_type();
      InsertImpl(std::move(key_copy), std::move(val_copy));
    }
  }

  ::operator delete(old_slots);
  delete[] old_metadata;
}

template <class Key, class T, class Hash, class KeyEqual>
std::pair<typename flat_hash_map<Key, T, Hash, KeyEqual>::iterator, bool>
flat_hash_map<Key, T, Hash, KeyEqual>::InsertImpl(const Key& key,
                                                   const mapped_type& value) {
  if (capacity_ == 0) {
    Allocate(CalculateCapacity(16));
  }

  if (static_cast<float>(size_ + 1) > capacity_ * max_load_factor_) {
    RehashImpl(capacity_ * 2);
  }

  size_t hash_value = hash_(key);
  size_type slot = FindInsertSlot(key, hash_value);

  if (slot < capacity_ && metadata_[slot] < kDeleted) {
    return {iterator(this, slot), false};
  }

  if (slot == capacity_) {
    RehashImpl(capacity_ * 2);
    hash_value = hash_(key);
    slot = FindInsertSlot(key, hash_value);
  }

  new (slots_[slot].ptr()) value_type(key, value);
  metadata_[slot] = H2(hash_value);
  ++size_;

  return {iterator(this, slot), true};
}

template <class Key, class T, class Hash, class KeyEqual>
std::pair<typename flat_hash_map<Key, T, Hash, KeyEqual>::iterator, bool>
flat_hash_map<Key, T, Hash, KeyEqual>::InsertImpl(Key&& key,
                                                   mapped_type&& value) {
  if (capacity_ == 0) {
    Allocate(CalculateCapacity(16));
  }

  if (static_cast<float>(size_ + 1) > capacity_ * max_load_factor_) {
    RehashImpl(capacity_ * 2);
  }

  size_t hash_value = hash_(key);
  size_type slot = FindInsertSlot(key, hash_value);

  if (slot < capacity_ && metadata_[slot] < kDeleted) {
    return {iterator(this, slot), false};
  }

  if (slot == capacity_) {
    RehashImpl(capacity_ * 2);
    hash_value = hash_(key);
    slot = FindInsertSlot(key, hash_value);
  }

  new (slots_[slot].ptr()) value_type(std::move(key), std::move(value));
  metadata_[slot] = H2(hash_value);
  ++size_;

  return {iterator(this, slot), true};
}

template <class Key, class T, class Hash, class KeyEqual>
flat_hash_map<Key, T, Hash, KeyEqual>::iterator::iterator(flat_hash_map* map,
                                                           size_type index)
    : map_(map), index_(index) {}

template <class Key, class T, class Hash, class KeyEqual>
flat_hash_map<Key, T, Hash, KeyEqual>::iterator::iterator()
    : map_(nullptr), index_(0) {}

template <class Key, class T, class Hash, class KeyEqual>
typename flat_hash_map<Key, T, Hash, KeyEqual>::iterator::reference
flat_hash_map<Key, T, Hash, KeyEqual>::iterator::operator*() const {
  return *map_->slots_[index_].ptr();
}

template <class Key, class T, class Hash, class KeyEqual>
typename flat_hash_map<Key, T, Hash, KeyEqual>::iterator::pointer
flat_hash_map<Key, T, Hash, KeyEqual>::iterator::operator->() const {
  return map_->slots_[index_].ptr();
}

template <class Key, class T, class Hash, class KeyEqual>
typename flat_hash_map<Key, T, Hash, KeyEqual>::iterator&
flat_hash_map<Key, T, Hash, KeyEqual>::iterator::operator++() {
  ++index_;
  while (index_ < map_->capacity_ &&
         map_->metadata_[index_] >= flat_hash_map::kDeleted) {
    ++index_;
  }
  return *this;
}

template <class Key, class T, class Hash, class KeyEqual>
typename flat_hash_map<Key, T, Hash, KeyEqual>::iterator
flat_hash_map<Key, T, Hash, KeyEqual>::iterator::operator++(int) {
  iterator tmp = *this;
  ++(*this);
  return tmp;
}

template <class Key, class T, class Hash, class KeyEqual>
bool flat_hash_map<Key, T, Hash, KeyEqual>::iterator::operator==(
    const iterator& other) const {
  return map_ == other.map_ && index_ == other.index_;
}

template <class Key, class T, class Hash, class KeyEqual>
bool flat_hash_map<Key, T, Hash, KeyEqual>::iterator::operator!=(
    const iterator& other) const {
  return !(*this == other);
}

template <class Key, class T, class Hash, class KeyEqual>
flat_hash_map<Key, T, Hash, KeyEqual>::const_iterator::const_iterator(
    const flat_hash_map* map, size_type index)
    : map_(map), index_(index) {}

template <class Key, class T, class Hash, class KeyEqual>
flat_hash_map<Key, T, Hash, KeyEqual>::const_iterator::const_iterator()
    : map_(nullptr), index_(0) {}

template <class Key, class T, class Hash, class KeyEqual>
flat_hash_map<Key, T, Hash, KeyEqual>::const_iterator::const_iterator(
    const iterator& it)
    : map_(it.map_), index_(it.index_) {}

template <class Key, class T, class Hash, class KeyEqual>
typename flat_hash_map<Key, T, Hash, KeyEqual>::const_iterator::reference
flat_hash_map<Key, T, Hash, KeyEqual>::const_iterator::operator*() const {
  return *map_->slots_[index_].ptr();
}

template <class Key, class T, class Hash, class KeyEqual>
typename flat_hash_map<Key, T, Hash, KeyEqual>::const_iterator::pointer
flat_hash_map<Key, T, Hash, KeyEqual>::const_iterator::operator->() const {
  return map_->slots_[index_].ptr();
}

template <class Key, class T, class Hash, class KeyEqual>
typename flat_hash_map<Key, T, Hash, KeyEqual>::const_iterator&
flat_hash_map<Key, T, Hash, KeyEqual>::const_iterator::operator++() {
  ++index_;
  while (index_ < map_->capacity_ &&
         map_->metadata_[index_] >= flat_hash_map::kDeleted) {
    ++index_;
  }
  return *this;
}

template <class Key, class T, class Hash, class KeyEqual>
typename flat_hash_map<Key, T, Hash, KeyEqual>::const_iterator
flat_hash_map<Key, T, Hash, KeyEqual>::const_iterator::operator++(int) {
  const_iterator tmp = *this;
  ++(*this);
  return tmp;
}

template <class Key, class T, class Hash, class KeyEqual>
bool flat_hash_map<Key, T, Hash, KeyEqual>::const_iterator::operator==(
    const const_iterator& other) const {
  return map_ == other.map_ && index_ == other.index_;
}

template <class Key, class T, class Hash, class KeyEqual>
bool flat_hash_map<Key, T, Hash, KeyEqual>::const_iterator::operator!=(
    const const_iterator& other) const {
  return !(*this == other);
}

template <class Key, class T, class Hash, class KeyEqual>
flat_hash_map<Key, T, Hash, KeyEqual>::flat_hash_map()
    : slots_(nullptr),
      metadata_(nullptr),
      capacity_(0),
      size_(0),
      max_load_factor_(0.75f),
      hash_(),
      equal_() {}

template <class Key, class T, class Hash, class KeyEqual>
flat_hash_map<Key, T, Hash, KeyEqual>::flat_hash_map(size_type bucket_count,
                                                      const Hash& hash,
                                                      const KeyEqual& equal)
    : slots_(nullptr),
      metadata_(nullptr),
      capacity_(0),
      size_(0),
      max_load_factor_(0.75f),
      hash_(hash),
      equal_(equal) {
  if (bucket_count > 0) {
    Allocate(CalculateCapacity(bucket_count));
  }
}

template <class Key, class T, class Hash, class KeyEqual>
template <class InputIt>
flat_hash_map<Key, T, Hash, KeyEqual>::flat_hash_map(InputIt first,
                                                      InputIt last,
                                                      size_type bucket_count,
                                                      const Hash& hash,
                                                      const KeyEqual& equal)
    : flat_hash_map(bucket_count, hash, equal) {
  insert(first, last);
}

template <class Key, class T, class Hash, class KeyEqual>
flat_hash_map<Key, T, Hash, KeyEqual>::flat_hash_map(
    std::initializer_list<value_type> init, size_type bucket_count,
    const Hash& hash, const KeyEqual& equal)
    : flat_hash_map(bucket_count, hash, equal) {
  insert(init);
}

template <class Key, class T, class Hash, class KeyEqual>
flat_hash_map<Key, T, Hash, KeyEqual>::flat_hash_map(
    const flat_hash_map& other)
    : slots_(nullptr),
      metadata_(nullptr),
      capacity_(0),
      size_(0),
      max_load_factor_(other.max_load_factor_),
      hash_(other.hash_),
      equal_(other.equal_) {
  if (other.capacity_ > 0) {
    Allocate(other.capacity_);
    for (size_type i = 0; i < other.capacity_; ++i) {
      metadata_[i] = other.metadata_[i];
      if (metadata_[i] < kDeleted) {
        new (slots_[i].ptr()) value_type(*other.slots_[i].ptr());
        ++size_;
      }
    }
  }
}

template <class Key, class T, class Hash, class KeyEqual>
flat_hash_map<Key, T, Hash, KeyEqual>::flat_hash_map(
    flat_hash_map&& other) noexcept
    : slots_(other.slots_),
      metadata_(other.metadata_),
      capacity_(other.capacity_),
      size_(other.size_),
      max_load_factor_(other.max_load_factor_),
      hash_(std::move(other.hash_)),
      equal_(std::move(other.equal_)) {
  other.slots_ = nullptr;
  other.metadata_ = nullptr;
  other.capacity_ = 0;
  other.size_ = 0;
}

template <class Key, class T, class Hash, class KeyEqual>
flat_hash_map<Key, T, Hash, KeyEqual>::~flat_hash_map() {
  Deallocate();
}

template <class Key, class T, class Hash, class KeyEqual>
flat_hash_map<Key, T, Hash, KeyEqual>&
flat_hash_map<Key, T, Hash, KeyEqual>::operator=(const flat_hash_map& other) {
  if (this != &other) {
    flat_hash_map tmp(other);
    swap(tmp);
  }
  return *this;
}

template <class Key, class T, class Hash, class KeyEqual>
flat_hash_map<Key, T, Hash, KeyEqual>&
flat_hash_map<Key, T, Hash, KeyEqual>::operator=(
    flat_hash_map&& other) noexcept {
  if (this != &other) {
    Deallocate();
    slots_ = other.slots_;
    metadata_ = other.metadata_;
    capacity_ = other.capacity_;
    size_ = other.size_;
    max_load_factor_ = other.max_load_factor_;
    hash_ = std::move(other.hash_);
    equal_ = std::move(other.equal_);

    other.slots_ = nullptr;
    other.metadata_ = nullptr;
    other.capacity_ = 0;
    other.size_ = 0;
  }
  return *this;
}

template <class Key, class T, class Hash, class KeyEqual>
flat_hash_map<Key, T, Hash, KeyEqual>&
flat_hash_map<Key, T, Hash, KeyEqual>::operator=(
    std::initializer_list<value_type> init) {
  clear();
  insert(init);
  return *this;
}

template <class Key, class T, class Hash, class KeyEqual>
void flat_hash_map<Key, T, Hash, KeyEqual>::swap(
    flat_hash_map& other) noexcept {
  std::swap(slots_, other.slots_);
  std::swap(metadata_, other.metadata_);
  std::swap(capacity_, other.capacity_);
  std::swap(size_, other.size_);
  std::swap(max_load_factor_, other.max_load_factor_);
  std::swap(hash_, other.hash_);
  std::swap(equal_, other.equal_);
}

template <class Key, class T, class Hash, class KeyEqual>
typename flat_hash_map<Key, T, Hash, KeyEqual>::iterator
flat_hash_map<Key, T, Hash, KeyEqual>::begin() noexcept {
  size_type i = 0;
  while (i < capacity_ && metadata_[i] >= kDeleted) {
    ++i;
  }
  return iterator(this, i);
}

template <class Key, class T, class Hash, class KeyEqual>
typename flat_hash_map<Key, T, Hash, KeyEqual>::const_iterator
flat_hash_map<Key, T, Hash, KeyEqual>::begin() const noexcept {
  size_type i = 0;
  while (i < capacity_ && metadata_[i] >= kDeleted) {
    ++i;
  }
  return const_iterator(this, i);
}

template <class Key, class T, class Hash, class KeyEqual>
typename flat_hash_map<Key, T, Hash, KeyEqual>::const_iterator
flat_hash_map<Key, T, Hash, KeyEqual>::cbegin() const noexcept {
  return begin();
}

template <class Key, class T, class Hash, class KeyEqual>
typename flat_hash_map<Key, T, Hash, KeyEqual>::iterator
flat_hash_map<Key, T, Hash, KeyEqual>::end() noexcept {
  return iterator(this, capacity_);
}

template <class Key, class T, class Hash, class KeyEqual>
typename flat_hash_map<Key, T, Hash, KeyEqual>::const_iterator
flat_hash_map<Key, T, Hash, KeyEqual>::end() const noexcept {
  return const_iterator(this, capacity_);
}

template <class Key, class T, class Hash, class KeyEqual>
typename flat_hash_map<Key, T, Hash, KeyEqual>::const_iterator
flat_hash_map<Key, T, Hash, KeyEqual>::cend() const noexcept {
  return end();
}

template <class Key, class T, class Hash, class KeyEqual>
bool flat_hash_map<Key, T, Hash, KeyEqual>::empty() const noexcept {
  return size_ == 0;
}

template <class Key, class T, class Hash, class KeyEqual>
typename flat_hash_map<Key, T, Hash, KeyEqual>::size_type
flat_hash_map<Key, T, Hash, KeyEqual>::size() const noexcept {
  return size_;
}

template <class Key, class T, class Hash, class KeyEqual>
void flat_hash_map<Key, T, Hash, KeyEqual>::clear() noexcept {
  for (size_type i = 0; i < capacity_; ++i) {
    if (metadata_[i] < kDeleted) {
      slots_[i].ptr()->~value_type();
      metadata_[i] = kEmpty;
    }
  }
  size_ = 0;
}

template <class Key, class T, class Hash, class KeyEqual>
std::pair<typename flat_hash_map<Key, T, Hash, KeyEqual>::iterator, bool>
flat_hash_map<Key, T, Hash, KeyEqual>::insert(const value_type& value) {
  return InsertImpl(value.first, value.second);
}

template <class Key, class T, class Hash, class KeyEqual>
std::pair<typename flat_hash_map<Key, T, Hash, KeyEqual>::iterator, bool>
flat_hash_map<Key, T, Hash, KeyEqual>::insert(value_type&& value) {
  Key key_copy = std::move(const_cast<Key&>(value.first));
  return InsertImpl(std::move(key_copy), std::move(value.second));
}

template <class Key, class T, class Hash, class KeyEqual>
template <class InputIt>
void flat_hash_map<Key, T, Hash, KeyEqual>::insert(InputIt first,
                                                    InputIt last) {
  for (auto it = first; it != last; ++it) {
    insert(*it);
  }
}

template <class Key, class T, class Hash, class KeyEqual>
void flat_hash_map<Key, T, Hash, KeyEqual>::insert(
    std::initializer_list<value_type> init) {
  insert(init.begin(), init.end());
}

template <class Key, class T, class Hash, class KeyEqual>
template <class... Args>
std::pair<typename flat_hash_map<Key, T, Hash, KeyEqual>::iterator, bool>
flat_hash_map<Key, T, Hash, KeyEqual>::emplace(Args&&... args) {
  value_type temp(std::forward<Args>(args)...);
  return insert(std::move(temp));
}

template <class Key, class T, class Hash, class KeyEqual>
typename flat_hash_map<Key, T, Hash, KeyEqual>::size_type
flat_hash_map<Key, T, Hash, KeyEqual>::erase(const key_type& key) {
  if (capacity_ == 0) return 0;

  size_t hash_value = hash_(key);
  size_type slot = FindSlot(key, hash_value);

  if (slot == capacity_) {
    return 0;
  }

  slots_[slot].ptr()->~value_type();
  metadata_[slot] = kDeleted;
  --size_;
  return 1;
}

template <class Key, class T, class Hash, class KeyEqual>
typename flat_hash_map<Key, T, Hash, KeyEqual>::iterator
flat_hash_map<Key, T, Hash, KeyEqual>::erase(iterator pos) {
  if (pos.index_ < capacity_ && metadata_[pos.index_] < kDeleted) {
    slots_[pos.index_].ptr()->~value_type();
    metadata_[pos.index_] = kDeleted;
    --size_;
  }
  ++pos;
  return pos;
}

template <class Key, class T, class Hash, class KeyEqual>
void flat_hash_map<Key, T, Hash, KeyEqual>::reserve(size_type new_capacity) {
  if (new_capacity > capacity_) {
    RehashImpl(CalculateCapacity(new_capacity));
  }
}

template <class Key, class T, class Hash, class KeyEqual>
typename flat_hash_map<Key, T, Hash, KeyEqual>::size_type
flat_hash_map<Key, T, Hash, KeyEqual>::capacity() const noexcept {
  return capacity_;
}

template <class Key, class T, class Hash, class KeyEqual>
float flat_hash_map<Key, T, Hash, KeyEqual>::load_factor() const noexcept {
  return capacity_ == 0 ? 0.0f : static_cast<float>(size_) / capacity_;
}

template <class Key, class T, class Hash, class KeyEqual>
void flat_hash_map<Key, T, Hash, KeyEqual>::max_load_factor(float ml) {
  max_load_factor_ = ml;
}

template <class Key, class T, class Hash, class KeyEqual>
float flat_hash_map<Key, T, Hash, KeyEqual>::max_load_factor() const noexcept {
  return max_load_factor_;
}

template <class Key, class T, class Hash, class KeyEqual>
typename flat_hash_map<Key, T, Hash, KeyEqual>::mapped_type&
flat_hash_map<Key, T, Hash, KeyEqual>::operator[](const key_type& key) {
  if (capacity_ == 0) {
    Allocate(CalculateCapacity(16));
  }

  size_t hash_value = hash_(key);
  size_type slot = FindInsertSlot(key, hash_value);

  if (slot < capacity_ && metadata_[slot] < kDeleted) {
    return slots_[slot].ptr()->second;
  }

  auto result = InsertImpl(key, mapped_type{});
  return result.first->second;
}

template <class Key, class T, class Hash, class KeyEqual>
typename flat_hash_map<Key, T, Hash, KeyEqual>::mapped_type&
flat_hash_map<Key, T, Hash, KeyEqual>::operator[](key_type&& key) {
  if (capacity_ == 0) {
    Allocate(CalculateCapacity(16));
  }

  size_t hash_value = hash_(key);
  size_type slot = FindInsertSlot(key, hash_value);

  if (slot < capacity_ && metadata_[slot] < kDeleted) {
    return slots_[slot].ptr()->second;
  }

  auto result = InsertImpl(std::move(key), mapped_type{});
  return result.first->second;
}

template <class Key, class T, class Hash, class KeyEqual>
typename flat_hash_map<Key, T, Hash, KeyEqual>::mapped_type&
flat_hash_map<Key, T, Hash, KeyEqual>::at(const key_type& key) {
  if (capacity_ == 0) {
    throw std::out_of_range("key not found");
  }

  size_t hash_value = hash_(key);
  size_type slot = FindSlot(key, hash_value);

  if (slot == capacity_) {
    throw std::out_of_range("key not found");
  }

  return slots_[slot].ptr()->second;
}

template <class Key, class T, class Hash, class KeyEqual>
const typename flat_hash_map<Key, T, Hash, KeyEqual>::mapped_type&
flat_hash_map<Key, T, Hash, KeyEqual>::at(const key_type& key) const {
  if (capacity_ == 0) {
    throw std::out_of_range("key not found");
  }

  size_t hash_value = hash_(key);
  size_type slot = FindSlot(key, hash_value);

  if (slot == capacity_) {
    throw std::out_of_range("key not found");
  }

  return slots_[slot].ptr()->second;
}

template <class Key, class T, class Hash, class KeyEqual>
typename flat_hash_map<Key, T, Hash, KeyEqual>::iterator
flat_hash_map<Key, T, Hash, KeyEqual>::find(const key_type& key) {
  if (capacity_ == 0) {
    return end();
  }

  size_t hash_value = hash_(key);
  size_type slot = FindSlot(key, hash_value);

  if (slot == capacity_) {
    return end();
  }

  return iterator(this, slot);
}

template <class Key, class T, class Hash, class KeyEqual>
typename flat_hash_map<Key, T, Hash, KeyEqual>::const_iterator
flat_hash_map<Key, T, Hash, KeyEqual>::find(const key_type& key) const {
  if (capacity_ == 0) {
    return end();
  }

  size_t hash_value = hash_(key);
  size_type slot = FindSlot(key, hash_value);

  if (slot == capacity_) {
    return end();
  }

  return const_iterator(this, slot);
}

template <class Key, class T, class Hash, class KeyEqual>
bool flat_hash_map<Key, T, Hash, KeyEqual>::contains(
    const key_type& key) const {
  return find(key) != end();
}

template <class Key, class T, class Hash, class KeyEqual>
typename flat_hash_map<Key, T, Hash, KeyEqual>::size_type
flat_hash_map<Key, T, Hash, KeyEqual>::count(const key_type& key) const {
  return contains(key) ? 1 : 0;
}
