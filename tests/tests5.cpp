#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <algorithm>

#include "../list.hpp"

struct TestStruct {
    int id;
    std::string data;
    TestStruct(int i, std::string s) : id(i), data(s) {}
    bool operator==(const TestStruct& other) const {
        return id == other.id && data == other.data;
    }
};

// простые операции над простым Т.Д.
TEST(ForwardListTest, BasicPushAndPop) {
    MapMemoryRes mr;
    ForwardList<int> list(&mr);

    // вставка
    list.push_front(10);
    list.push_front(20);
    list.push_front(30);

    auto it = list.begin();
    EXPECT_EQ(*it, 30);
    ++it;
    EXPECT_EQ(*it, 20);
    ++it;
    EXPECT_EQ(*it, 10);

    // улаение
    list.pop_front();
    EXPECT_EQ(*list.begin(), 20);
    list.clear();
    EXPECT_EQ(list.begin(), list.end());
}

// проверка со структурой
TEST(ForwardListTest, ComplexTypes) {
    MapMemoryRes mr;
    ForwardList<TestStruct> list(&mr);

    list.push_front(TestStruct(1, "First"));
    list.push_front(TestStruct(2, "Second"));

    auto it = list.begin();
    EXPECT_EQ(it->id, 2);
    EXPECT_EQ(it->data, "Second");

    ++it;
    EXPECT_EQ(it->id, 1);
    EXPECT_EQ(it->data, "First");
}


// итераторы -_-
TEST(ForwardListTest, IteratorTraits) {
    using Iter = ForwardList<int>::Iterator;

    // проверочка, что итератор считался
    bool is_forward = std::is_same<std::iterator_traits<Iter>::iterator_category,
                                   std::forward_iterator_tag>::value;
    EXPECT_TRUE(is_forward);


    MapMemoryRes mr;
    ForwardList<int> list(&mr);
    list.push_front(1);
    list.push_front(2);
    list.push_front(3);

    auto dist = std::distance(list.begin(), list.end());
    EXPECT_EQ(dist, 3);
}

// тест аллокатора
TEST(MemoryResourceTest, ReusesMemory) {
    MapMemoryRes mr;


    std::pmr::polymorphic_allocator<int> alloc(&mr);

    // память под 1 int
    int* p1 = alloc.allocate(1);

    //  освобождаем в _free_blocks
    alloc.deallocate(p1, 1);

    // выделяем память такого же размера
    int* p2 = alloc.allocate(1);

    // адреса должны совпадать
    EXPECT_EQ(p1, p2) << "Allocator did not reuse the memory block!";

    // еще один блок
    int* p3 = alloc.allocate(1);
    EXPECT_NE(p2, p3) << "Allocator gave the same address for a busy block!";

    // Чистим за собой (хотя деструктор mr сделает это, для теста полезно)
    alloc.deallocate(p2, 1);
    alloc.deallocate(p3, 1);
}

// переиспользование памяти
TEST(ForwardListTest, ContainerReuseIntegration) {
    MapMemoryRes mr;

    ForwardList<int> list(&mr);

    list.push_front(100);
    const int* addr1 = &(*list.begin());

    list.pop_front();

    list.push_front(200);
    const int* addr2 = &(*list.begin());

    // адрес должен совпасть
    EXPECT_EQ(addr1, addr2) << "List node memory was not reused after pop_front + push_front";
}