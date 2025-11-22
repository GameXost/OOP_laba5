#include <iostream>
#include <memory_resource>
#include <algorithm>
#include <iterator>
#include <map>
#include <string>
#include <vector>

using namespace std;

class MapMemoryRes : public pmr::memory_resource {
private:
    map<void*, size_t> _allocate_blocks;
    multimap<size_t, void *> _free_blocks;

protected:
    void *do_allocate(size_t bytes, size_t alignment) override {
        auto range = _free_blocks.equal_range(bytes);
        for (auto i = range.first; i != range.second; i++) {
            void *ptr = i->second;
            if (reinterpret_cast<uintptr_t>(ptr) % alignment == 0) {
                _free_blocks.erase(i);
                cout << "Alloc:reused " << ptr << " size " << bytes << endl;
                return ptr;
            }
        }

        void *ptr = ::operator new(bytes);
        _allocate_blocks[ptr] = bytes;
        cout << "Alloc:new block at " << ptr << " size " << bytes << endl;
        return ptr;
    }

    void do_deallocate(void *p, size_t bytes, size_t alignment) override {
        _free_blocks.insert({bytes, p});
        cout << "Dealloc:returned to pool " << p << " size " << bytes << endl;
    }

    bool do_is_equal(const pmr::memory_resource &other) const noexcept override {
        return this == &other;
    }

public:
    ~MapMemoryRes() {
        cout << "\nDestructor MR:cleaning " << _allocate_blocks.size() << " blocks \n";
        for (const auto& pair : _allocate_blocks) {
            ::operator delete(pair.first);
        }
        _allocate_blocks.clear();
        _free_blocks.clear();
    }
};

template <typename T>
class ForwardList {
private:
    struct Node {
        T value;
        Node *next;
        Node (const T &val, Node *nxt = nullptr) : value(val), next(nxt) {}
    };

    pmr::polymorphic_allocator<Node> _allocator;
    Node *_head = nullptr;

public:
    explicit ForwardList(pmr::memory_resource *memRs = pmr::get_default_resource())
        : _allocator(memRs) {}

    ~ForwardList() {
        clear();
    }

    class Iterator {
    private:
        Node* _current;

    public:
        using iterator_category = forward_iterator_tag;
        using value_type = T;
        using difference_type = ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        Iterator(Node *node) : _current(node) {}

        reference operator*() const { return _current->value; }
        pointer operator->() const { return &_current->value; }

        Iterator &operator++() {
            if (_current) _current = _current->next;
            return *this;
        }

        Iterator operator++(int) {
            Iterator temp = *this;
            ++(*this);
            return temp;
        }

        bool operator==(const Iterator &other) const { return _current == other._current; }
        bool operator!=(const Iterator &other) const { return _current != other._current; }
    };

    Iterator begin() { return Iterator(_head); }
    Iterator end() { return Iterator(nullptr); }

    void push_front(const T &value) {
        Node *newNode = _allocator.allocate(1);
        _allocator.construct(newNode, value, _head);
        _head = newNode;
    }

    void pop_front() {
        if (_head) {
            Node *temp = _head;
            _head = _head->next;

            allocator_traits<decltype(_allocator)>::destroy(_allocator, temp);
            _allocator.deallocate(temp, 1);
        }
    }

    void clear() {
        while (_head) {
            pop_front();
        }
    }
};

