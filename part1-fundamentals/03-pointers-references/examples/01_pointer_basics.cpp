#include <iostream>

int main() {
    int x = 42;
    int* p = &x; // p stores the address of x

    std::cout << "x  = " << x << '\n';
    std::cout << "&x = " << &x << '\n';
    std::cout << "p  = " << p << "  (same address)\n";
    std::cout << "*p = " << *p << "  (value at that address)\n";

    *p = 100; // write through the pointer
    std::cout << "after *p = 100, x = " << x << '\n';

    int y = 7;
    p = &y; // re-point
    std::cout << "p now points to y: *p = " << *p << '\n';

    int* none = nullptr;
    if (!none) std::cout << "none is null: never dereference it!\n";

    // const combinations
    const int* ptr_to_const = &x; // can't change x through it
    // *ptr_to_const = 1;          // ERROR
    ptr_to_const = &y;             // OK: re-point

    int* const const_ptr = &x;     // can change x, can't re-point
    *const_ptr = 5;
    // const_ptr = &y;             // ERROR

    std::cout << "x = " << x << ", *ptr_to_const = " << *ptr_to_const << '\n';

    // Pointers have a size too (8 bytes on 64-bit), independent of what they point to
    std::cout << "sizeof(int*) = " << sizeof(int*) << ", sizeof(double*) = " << sizeof(double*) << '\n';

    // Pointer to pointer
    int** pp = &p;
    std::cout << "**pp = " << **pp << '\n';
}
