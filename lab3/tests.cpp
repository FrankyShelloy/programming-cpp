#include "flat_hash_map.hpp"
#include <gtest/gtest.h>
#include <string>
#include <vector>

class FlatHashMapTest : public ::testing::Test {
protected:
    flat_hash_map<int, std::string> map;
    flat_hash_map<std::string, int> str_map;
};

TEST_F(FlatHashMapTest, DefaultConstructor) {
    flat_hash_map<int, int> m;
    EXPECT_TRUE(m.empty());
    EXPECT_EQ(m.size(), 0);
}

TEST_F(FlatHashMapTest, BucketCountConstructor) {
    flat_hash_map<int, int> m(100);
    EXPECT_TRUE(m.empty());
    EXPECT_GE(m.capacity(), 100);
}

TEST_F(FlatHashMapTest, InitializerListConstructor) {
    flat_hash_map<int, std::string> m = {{1, "one"}, {2, "two"}, {3, "three"}};
    EXPECT_EQ(m.size(), 3);
    EXPECT_EQ(m[1], "one");
}

TEST_F(FlatHashMapTest, CopyConstructor) {
    map[1] = "one";
    map[2] = "two";
    flat_hash_map<int, std::string> copy(map);
    EXPECT_EQ(copy.size(), 2);
    EXPECT_EQ(copy[1], "one");
}

TEST_F(FlatHashMapTest, MoveConstructor) {
    map[1] = "one";
    flat_hash_map<int, std::string> moved(std::move(map));
    EXPECT_EQ(moved[1], "one");
    EXPECT_EQ(map.size(), 0);
}

TEST_F(FlatHashMapTest, CopyAssignment) {
    map[1] = "one";
    flat_hash_map<int, std::string> copy;
    copy = map;
    EXPECT_EQ(copy[1], "one");
}

TEST_F(FlatHashMapTest, MoveAssignment) {
    map[1] = "one";
    flat_hash_map<int, std::string> moved;
    moved = std::move(map);
    EXPECT_EQ(moved[1], "one");
}

TEST_F(FlatHashMapTest, Swap) {
    map[1] = "one";
    flat_hash_map<int, std::string> other;
    other[2] = "two";
    map.swap(other);
    EXPECT_EQ(map[2], "two");
    EXPECT_EQ(other[1], "one");
}

TEST_F(FlatHashMapTest, Iterator) {
    map = {{1, "one"}, {2, "two"}, {3, "three"}};
    int count = 0;
    for (auto it = map.begin(); it != map.end(); ++it) {
        count++;
    }
    EXPECT_EQ(count, 3);
}

TEST_F(FlatHashMapTest, RangeBasedFor) {
    map = {{1, "one"}, {2, "two"}};
    int count = 0;
    for (const auto& pair : map) {
        count++;
    }
    EXPECT_EQ(count, 2);
}

TEST_F(FlatHashMapTest, Empty) {
    EXPECT_TRUE(map.empty());
    map[1] = "one";
    EXPECT_FALSE(map.empty());
}

TEST_F(FlatHashMapTest, Size) {
    EXPECT_EQ(map.size(), 0);
    map[1] = "one";
    EXPECT_EQ(map.size(), 1);
}

TEST_F(FlatHashMapTest, Clear) {
    map = {{1, "one"}, {2, "two"}};
    map.clear();
    EXPECT_TRUE(map.empty());
}

TEST_F(FlatHashMapTest, InsertLvalue) {
    std::pair<const int, std::string> value(1, "one");
    auto result = map.insert(value);
    EXPECT_TRUE(result.second);
    EXPECT_EQ(map.size(), 1);
}

TEST_F(FlatHashMapTest, InsertDuplicate) {
    map.insert({1, "one"});
    auto result = map.insert({1, "uno"});
    EXPECT_FALSE(result.second);
    EXPECT_EQ(map[1], "one");
}

TEST_F(FlatHashMapTest, Emplace) {
    auto result = map.emplace(1, "one");
    EXPECT_TRUE(result.second);
    EXPECT_EQ(map[1], "one");
}

TEST_F(FlatHashMapTest, EraseByKey) {
    map = {{1, "one"}, {2, "two"}};
    size_t erased = map.erase(2);
    EXPECT_EQ(erased, 1);
    EXPECT_EQ(map.size(), 1);
}

TEST_F(FlatHashMapTest, EraseByIterator) {
    map = {{1, "one"}, {2, "two"}};
    auto it = map.find(2);
    map.erase(it);
    EXPECT_FALSE(map.contains(2));
}

TEST_F(FlatHashMapTest, Reserve) {
    map.reserve(100);
    EXPECT_GE(map.capacity(), 100);
}

TEST_F(FlatHashMapTest, LoadFactor) {
    map.reserve(100);
    for (int i = 0; i < 50; ++i) {
        map[i] = std::to_string(i);
    }
    EXPECT_GT(map.load_factor(), 0.0f);
}

TEST_F(FlatHashMapTest, SubscriptOperator) {
    map[1] = "one";
    EXPECT_EQ(map[1], "one");
}

TEST_F(FlatHashMapTest, AtValid) {
    map[1] = "one";
    EXPECT_EQ(map.at(1), "one");
}

TEST_F(FlatHashMapTest, AtInvalid) {
    EXPECT_THROW(map.at(99), std::out_of_range);
}

TEST_F(FlatHashMapTest, Find) {
    map[1] = "one";
    auto it = map.find(1);
    ASSERT_NE(it, map.end());
    EXPECT_EQ(it->second, "one");
}

TEST_F(FlatHashMapTest, Contains) {
    map[1] = "one";
    EXPECT_TRUE(map.contains(1));
    EXPECT_FALSE(map.contains(99));
}

TEST_F(FlatHashMapTest, Count) {
    map[1] = "one";
    EXPECT_EQ(map.count(1), 1);
    EXPECT_EQ(map.count(99), 0);
}

TEST_F(FlatHashMapTest, LargeInsertion) {
    for (int i = 0; i < 1000; ++i) {
        map[i] = std::to_string(i);
    }
    EXPECT_EQ(map.size(), 1000);
}

TEST_F(FlatHashMapTest, RangeConstructor) {
    std::vector<std::pair<int, std::string>> vec = {{1, "one"}, {2, "two"}, {3, "three"}};
    flat_hash_map<int, std::string> m(vec.begin(), vec.end());
    EXPECT_EQ(m.size(), 3);
    EXPECT_EQ(m[2], "two");
}

TEST_F(FlatHashMapTest, RangeInsert) {
    std::vector<std::pair<int, std::string>> vec = {{1, "one"}, {2, "two"}};
    map.insert(vec.begin(), vec.end());
    EXPECT_EQ(map.size(), 2);
    EXPECT_EQ(map[1], "one");
}

TEST_F(FlatHashMapTest, SubscriptOperatorRvalue) {
    int key = 42;
    map[std::move(key)] = "value";
    EXPECT_EQ(map[42], "value");
}

TEST_F(FlatHashMapTest, AtConstVersion) {
    map[1] = "one";
    const auto& const_map = map;
    EXPECT_EQ(const_map.at(1), "one");
}

TEST_F(FlatHashMapTest, FindConstVersion) {
    map[1] = "one";
    const auto& const_map = map;
    auto it = const_map.find(1);
    EXPECT_NE(it, const_map.end());
    EXPECT_EQ(it->second, "one");
}

TEST_F(FlatHashMapTest, ConstIterators) {
    map = {{1, "one"}, {2, "two"}};
    const auto& const_map = map;
    int count = 0;
    for (auto it = const_map.cbegin(); it != const_map.cend(); ++it) {
        count++;
    }
    EXPECT_EQ(count, 2);
}

TEST_F(FlatHashMapTest, AssignmentOperatorInitList) {
    map = {{1, "one"}, {2, "two"}, {3, "three"}};
    EXPECT_EQ(map.size(), 3);
    EXPECT_EQ(map[2], "two");
}

TEST_F(FlatHashMapTest, EraseIteratorReturnValue) {
    map = {{1, "one"}, {2, "two"}, {3, "three"}};
    auto it = map.find(1);
    auto next_it = map.erase(it);
    EXPECT_NE(next_it, map.end());
}

TEST_F(FlatHashMapTest, MaxLoadFactor) {
    EXPECT_EQ(map.max_load_factor(), 0.75f);
    map.max_load_factor(0.5f);
    EXPECT_EQ(map.max_load_factor(), 0.5f);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
