#ifndef FLAT_HASH_MAP_HPP_
#define FLAT_HASH_MAP_HPP_

#include <cstddef>
#include <cstdint>
#include <functional>
#include <initializer_list>
#include <utility>

template <class Key, class T, class Hash = std::hash<Key>,
          class KeyEqual = std::equal_to<Key>>
class flat_hash_map {
 public:
  using key_type = Key;
  using mapped_type = T;
  using value_type = std::pair<const Key, T>;
  using size_type = std::size_t;
  using hasher = Hash;
  using key_equal = KeyEqual;

  class iterator;
  class const_iterator;

  flat_hash_map();

  explicit flat_hash_map(size_type bucket_count, const Hash& hash = Hash(),
                         const KeyEqual& equal = KeyEqual());

  template <class InputIt>
  flat_hash_map(InputIt first, InputIt last, size_type bucket_count = 0,
                const Hash& hash = Hash(), const KeyEqual& equal = KeyEqual());

  flat_hash_map(std::initializer_list<value_type> init,
                size_type bucket_count = 0, const Hash& hash = Hash(),
                const KeyEqual& equal = KeyEqual());

  flat_hash_map(const flat_hash_map& other);
  flat_hash_map(flat_hash_map&& other) noexcept;

  ~flat_hash_map();

  flat_hash_map& operator=(const flat_hash_map& other);
  flat_hash_map& operator=(flat_hash_map&& other) noexcept;
  flat_hash_map& operator=(std::initializer_list<value_type> init);

  void swap(flat_hash_map& other) noexcept;

  iterator begin() noexcept;
  const_iterator begin() const noexcept;
  const_iterator cbegin() const noexcept;

  iterator end() noexcept;
  const_iterator end() const noexcept;
  const_iterator cend() const noexcept;

  bool empty() const noexcept;
  size_type size() const noexcept;
  void clear() noexcept;

  std::pair<iterator, bool> insert(const value_type& value);
  std::pair<iterator, bool> insert(value_type&& value);

  template <class InputIt>
  void insert(InputIt first, InputIt last);

  void insert(std::initializer_list<value_type> init);

  template <class... Args>
  std::pair<iterator, bool> emplace(Args&&... args);

  size_type erase(const key_type& key);
  iterator erase(iterator pos);

  void reserve(size_type new_capacity);
  size_type capacity() const noexcept;
  float load_factor() const noexcept;
  void max_load_factor(float ml);
  float max_load_factor() const noexcept;

  mapped_type& operator[](const key_type& key);
  mapped_type& operator[](key_type&& key);

  mapped_type& at(const key_type& key);
  const mapped_type& at(const key_type& key) const;

  iterator find(const key_type& key);
  const_iterator find(const key_type& key) const;

  bool contains(const key_type& key) const;
  size_type count(const key_type& key) const;

 private:
  static constexpr uint8_t kEmpty = 0xFF;
  static constexpr uint8_t kDeleted = 0xFE;

  struct Slot {
    alignas(value_type) uint8_t storage[sizeof(value_type)];
    value_type* ptr();
    const value_type* ptr() const;
  };

  Slot* slots_;
  uint8_t* metadata_;
  size_type capacity_;
  size_type size_;
  float max_load_factor_;
  Hash hash_;
  KeyEqual equal_;

  static size_type CalculateCapacity(size_type requested);
  static uint8_t H2(size_t hash_value);

  size_type FindSlot(const Key& key, size_t hash_value) const;
  size_type FindInsertSlot(const Key& key, size_t hash_value) const;

  void Allocate(size_type new_capacity);
  void Deallocate();
  void RehashImpl(size_type new_capacity);

  std::pair<iterator, bool> InsertImpl(const Key& key,
                                       const mapped_type& value);
  std::pair<iterator, bool> InsertImpl(Key&& key, mapped_type&& value);
};

template <class Key, class T, class Hash, class KeyEqual>
class flat_hash_map<Key, T, Hash, KeyEqual>::iterator {
  friend class flat_hash_map;
  friend class const_iterator;

  flat_hash_map* map_;
  size_type index_;

  iterator(flat_hash_map* map, size_type index);

 public:
  using difference_type = std::ptrdiff_t;
  using value_type = flat_hash_map::value_type;
  using pointer = value_type*;
  using reference = value_type&;
  using iterator_category = std::forward_iterator_tag;

  iterator();

  reference operator*() const;
  pointer operator->() const;

  iterator& operator++();
  iterator operator++(int);

  bool operator==(const iterator& other) const;
  bool operator!=(const iterator& other) const;
};

template <class Key, class T, class Hash, class KeyEqual>
class flat_hash_map<Key, T, Hash, KeyEqual>::const_iterator {
  friend class flat_hash_map;

  const flat_hash_map* map_;
  size_type index_;

  const_iterator(const flat_hash_map* map, size_type index);

 public:
  using difference_type = std::ptrdiff_t;
  using value_type = flat_hash_map::value_type;
  using pointer = const value_type*;
  using reference = const value_type&;
  using iterator_category = std::forward_iterator_tag;

  const_iterator();
  const_iterator(const iterator& it);

  reference operator*() const;
  pointer operator->() const;

  const_iterator& operator++();
  const_iterator operator++(int);

  bool operator==(const const_iterator& other) const;
  bool operator!=(const const_iterator& other) const;
};

#include "flat_hash_map.cpp"

#endif  // FLAT_HASH_MAP_HPP_
