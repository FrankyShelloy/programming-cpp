#include "PriorityQueue.h"
#include <gtest/gtest.h>
#include <climits>
#include <vector>

// Тест: конструктор по умолчанию создаёт max-heap и изначально пустую очередь.
TEST(PriorityQueueTest, DefaultConstructorIsMaxHeap) {
    PriorityQueue pq;
    EXPECT_TRUE(pq.is_max_heap());
    EXPECT_TRUE(pq.empty());
}

// Тест: конструктор с булевым флагом позволяет явно задать тип кучи — max-heap (true) или min-heap (false).
TEST(PriorityQueueTest, ConstructorWithFlag) {
    PriorityQueue max_pq(true);
    PriorityQueue min_pq(false);
    EXPECT_TRUE(max_pq.is_max_heap());
    EXPECT_FALSE(min_pq.is_max_heap());
}

// Тест: конструктор, принимающий вектор и флаг, корректно строит кучу из исходных данных.
// Для max-heap вершина должна содержать максимальный элемент.
TEST(PriorityQueueTest, ConstructorFromVector) {
    std::vector<int> data = {1, 3, 2, 5, 4};
    PriorityQueue pq(data, true);
    EXPECT_EQ(pq.size(), 5);
    EXPECT_EQ(pq.top(), 5);
}

// Тест: метод push добавляет элементы в max-heap, а top возвращает наибольший без извлечения.
TEST(PriorityQueueTest, PushAndTopMaxHeap) {
    PriorityQueue pq(true);
    pq.push(10);
    pq.push(30);
    pq.push(20);
    EXPECT_EQ(pq.top(), 30);
}

// Тест: в min-heap метод top возвращает наименьший элемент после нескольких вставок.
TEST(PriorityQueueTest, PushAndTopMinHeap) {
    PriorityQueue pq(false);
    pq.push(10);
    pq.push(30);
    pq.push(5);
    EXPECT_EQ(pq.top(), 5);
}

// Тест: последовательные вызовы pop в max-heap извлекают элементы в порядке убывания.
TEST(PriorityQueueTest, PopOrderMaxHeap) {
    PriorityQueue pq(true);
    pq.push(1);
    pq.push(3);
    pq.push(2);
    pq.pop();  // удаляет 3
    EXPECT_EQ(pq.top(), 2);
    pq.pop();  // удаляет 2
    EXPECT_EQ(pq.top(), 1);
    pq.pop();  // удаляет 1
    EXPECT_TRUE(pq.empty());
}

// Тест: вызов top() на пустой очереди должен выбрасывать исключение std::runtime_error.
TEST(PriorityQueueTest, TopOnEmptyThrows) {
    PriorityQueue pq;
    EXPECT_THROW(pq.top(), std::runtime_error);
}

// Тест: вызов pop() на пустой очереди также должен выбрасывать исключение std::runtime_error.
TEST(PriorityQueueTest, PopOnEmptyThrows) {
    PriorityQueue pq;
    EXPECT_THROW(pq.pop(), std::runtime_error);
}

// Тест: методы empty() и size() корректно отражают текущее состояние очереди.
TEST(PriorityQueueTest, EmptyAndSize) {
    PriorityQueue pq;
    EXPECT_TRUE(pq.empty());
    EXPECT_EQ(pq.size(), 0);

    pq.push(42);
    EXPECT_FALSE(pq.empty());
    EXPECT_EQ(pq.size(), 1);
}

// Тест: метод clear() полностью очищает очередь, делая её пустой.
TEST(PriorityQueueTest, Clear) {
    PriorityQueue pq;
    pq.push(1);
    pq.push(2);
    pq.clear();
    EXPECT_TRUE(pq.empty());
}

// Тест: метод reserve() предварительно резервирует память, не нарушая логику работы очереди.
TEST(PriorityQueueTest, Reserve) {
    PriorityQueue pq;
    pq.reserve(100);
    pq.push(10);
    EXPECT_EQ(pq.top(), 10);
}

// Тест: оператор == считает две max-heap эквивалентными, если они содержат одинаковые элементы,
// независимо от порядка вставки.
TEST(PriorityQueueTest, EqualitySameContentMaxHeap) {
    PriorityQueue a(true), b(true);
    a.push(1); a.push(2); a.push(3);
    b.push(3); b.push(1); b.push(2);
    EXPECT_TRUE(a == b);
}

// Тест: оператор == возвращает false, если содержимое очередей различается.
TEST(PriorityQueueTest, EqualityDifferentContent) {
    PriorityQueue a(true), b(true);
    a.push(1); a.push(2);
    b.push(1); b.push(3);
    EXPECT_FALSE(a == b);
}

// Тест: очереди с разными режимами (max-heap vs min-heap) считаются неравными,
// даже если содержат одни и те же элементы.
TEST(PriorityQueueTest, EqualityDifferentModes) {
    PriorityQueue max_pq(true);
    PriorityQueue min_pq(false);
    max_pq.push(1); max_pq.push(2);
    min_pq.push(1); min_pq.push(2);
    EXPECT_FALSE(max_pq == min_pq);
}

// Тест: оператор != корректно работает как логическое отрицание оператора ==.
TEST(PriorityQueueTest, InequalityOperator) {
    PriorityQueue a(true), b(true);
    a.push(1);
    b.push(2);
    EXPECT_TRUE(a != b);
}

// Тест: конструктор копирования создаёт независимую, но эквивалентную копию очереди.
TEST(PriorityQueueTest, CopyConstructor) {
    PriorityQueue original(true);
    original.push(10);
    original.push(20);

    PriorityQueue copy = original;
    EXPECT_TRUE(copy == original);
    EXPECT_EQ(copy.top(), 20);
}

// Тест: конструктор перемещения передаёт ресурсы исходной очереди новой, оставляя исходную в валидном, но неопределённом состоянии.
TEST(PriorityQueueTest, MoveConstructor) {
    PriorityQueue original(true);
    original.push(100);
    original.push(200);

    PriorityQueue moved = std::move(original);
    EXPECT_EQ(moved.top(), 200);
}

// Тест: оператор присваивания копированием корректно копирует содержимое и режим кучи.
TEST(PriorityQueueTest, CopyAssignment) {
    PriorityQueue a(true);
    a.push(5);
    PriorityQueue b(false);
    b = a;
    EXPECT_TRUE(b.is_max_heap());
    EXPECT_EQ(b.top(), 5);
}

// Тест: оператор присваивания перемещением передаёт данные и режим от одной очереди к другой.
TEST(PriorityQueueTest, MoveAssignment) {
    PriorityQueue a(true);
    a.push(999);
    PriorityQueue b(false);
    b = std::move(a);
    EXPECT_EQ(b.top(), 999);
    EXPECT_TRUE(b.is_max_heap());
}

// Тест: перегрузка оператора << позволяет выводить очередь в поток;
// проверяем, что вывод не пуст и не равен строке пустого списка.
TEST(PriorityQueueTest, StreamOutput) {
    PriorityQueue pq(true);
    pq.push(1);
    pq.push(2);
    std::ostringstream oss;
    oss << pq;
    std::string output = oss.str();
    EXPECT_FALSE(output.empty());
    EXPECT_NE(output, "[]");
}

// Тест: очередь корректно обрабатывает граничные значения типа int INT_MIN и INT_MAX
TEST(PriorityQueueTest, HandlesIntMinMax) {
    PriorityQueue max_pq(true);
    max_pq.push(INT_MAX);
    max_pq.push(INT_MIN);
    max_pq.push(0);

    EXPECT_EQ(max_pq.top(), INT_MAX);
    max_pq.pop();
    EXPECT_EQ(max_pq.top(), 0);
    max_pq.pop();
    EXPECT_EQ(max_pq.top(), INT_MIN);

    PriorityQueue min_pq(false);
    min_pq.push(INT_MAX);
    min_pq.push(INT_MIN);
    min_pq.push(0);

    EXPECT_EQ(min_pq.top(), INT_MIN);
    min_pq.pop();
    EXPECT_EQ(min_pq.top(), 0);
    min_pq.pop();
    EXPECT_EQ(min_pq.top(), INT_MAX);
}