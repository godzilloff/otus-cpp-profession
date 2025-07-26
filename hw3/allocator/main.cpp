#include <iostream>
#include <map>
#include <cstddef>
#include <new>
#include <cstring>
#include <type_traits>

#include "aalloc.h"
#include "linklist.h"

int factorial(int n) {
    return n <= 1 ? 1 : n * factorial(n - 1);
}

// === main() ===
int main() {
    // ---------------- STD::MAP -----------------------
    std::cout << "*** Part 1 ***" << std::endl;
    std::cout << "=== std::map + std::allocator ===" << std::endl;
    {
        std::map<int, int> standartMap;

        for (int i = 0; i < 10; ++i)         // [0..9] -> factorial(i)
            standartMap[i] = factorial(i);

        for (const auto& [key, value] : standartMap)         // print
            std::cout << key << "! = " << value << std::endl;
    }

    std::cout << "=== std::map + FixedPoolAllocator ===" << std::endl;
    {
        using Allocator = FixedPoolAllocator<std::pair<const int, int>, 10>;
        using Map = std::map<int, int, std::less<int>, Allocator>;

        try {
            Map my_map(Allocator{});

            for (int i = 0; i < 10; ++i)      //[0..9] -> factorial(i)
                my_map[i] = factorial(i);

            for (const auto& [key, value] : my_map)         // Print
                std::cout << key << "! = " << value << std::endl;
        }
        catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return 1;
        }
    }

    // -------------------- LIST ----------------------
    std::cout << "*** Part 2 ***" << std::endl;
    std::cout << "=== Custom list + std::allocator ===" << std::endl;
    {
        SimpleList<int> list1;
        for (int i = 0; i < 10; ++i) {
            list1.push_front(9 - i); // input 0,1,..,9
        }
        list1.print(); // output: 0 1 2 3 4 5 6 7 8 9

        std::cout << "Size: " << list1.size() << ", Empty: " << list1.empty() << std::endl;
    }

    std::cout << std::endl << "=== Custom list + FixedPoolAllocator ===" << std::endl;
    {
        using MyAlloc = FixedPoolAllocator<ListNode<int>, 10>;
        SimpleList<int, MyAlloc> list2(MyAlloc{});

        try {
            for (int i = 0; i < 10; ++i) {
                list2.emplace_front(9-i);
            }
            list2.print(); // print
            std::cout << "Size: " << list2.size() << ", Empty: " << list2.empty() << std::endl;

            // add 11 item — fallback
            std::cout << std::endl << "add 11 item..." << std::endl;
            list2.push_front(42); // fallback на ::new
            
            std::cout << "ok (fallback)" << std::endl;
            list2.print();
            std::cout << "Size: " << list2.size() << ", Empty: " << list2.empty() << std::endl;

        } catch (const std::bad_alloc&) {
            std::cout << "std::bad_alloc!" << std::endl;
        }
    }

    std::cout << "=== Success ===" << std::endl;
    return 0;
}
