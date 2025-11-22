#include <iostream>
#include <string>
#include "list.hpp"

using namespace std;

struct ComplexType {
    int id;
    string name;
    ComplexType(int i, string n) : id(i), name(n) {}
};

int main() {
    MapMemoryRes mr;

    {
        ForwardList<int> list(&mr);
        list.push_front(10);
        list.push_front(20);

        list.pop_front();


        list.push_front(30);

        cout << "List items: ";
        for(auto x : list) {
            cout << x << " ";
        }
        cout << endl;
    }

    {
        ForwardList<ComplexType> list2(&mr);
        list2.push_front(ComplexType(1, "One"));
        list2.push_front(ComplexType(2, "Two"));

        cout << "List items: ";
        for(auto& x : list2) {
            cout << "[" << x.id << ":" << x.name << "] ";
        }
        cout << endl;
    }

    return 0;
}