# High-Performance Custom Memory Pool

## Overview
This project implements a **high-performance custom memory allocator** in C++ to optimize dynamic memory management and reduce allocation latency. It leverages **lock-free techniques**, **move semantics**, and a **free-list-based allocation strategy** to minimize heap fragmentation and improve memory efficiency in performance-critical applications.

## Features
- **Custom Memory Pool**: Pre-allocates memory chunks to optimize allocation and deallocation.
- **Lock-Free Free List**: Uses `std::atomic` to ensure efficient and thread-safe memory management.
- **Move Semantics**: Utilizes `std::move` and `std::exchange` to efficiently transfer ownership.
- **RAII Compliance**: Automatically manages resources to prevent memory leaks.
- **Custom `new` and `delete` Operators**: Allows objects to use the memory pool instead of heap allocation.

## Technologies Used
- **C++17** (or later)
- **Standard Library (`std::atomic`, `std::vector`, `std::move`)**
- **Lock-Free Programming**
- **Memory Management & RAII**

## Installation & Usage
### Compilation
Ensure you have a C++ compiler that supports C++17 or later (e.g., GCC, Clang, or MSVC).  
Compile the program using:

```sh
g++ -std=c++17 -O2 -pthread MemoryPool.cpp -o memory_pool
```

### Running the Program
After compilation, execute:
``` bash
./memory_pool
```
### Sample
``` bash
Test(1, 2)
Test(3, 4)
Custom Memory Pool test completed.
```

### Code Structure
- MemoryPool Class: Implements a high-performance memory allocator using a lock-free free list.
- Test Class: Demonstrates memory pool integration by overloading new and delete.
- Main Function: Creates and deallocates objects using the custom allocator.

### How it Works
- The MemoryPool class pre-allocates a memory chunk and maintains a lock-free free list.
- When a new memory request is made, it fetches a block from the free list instead of using malloc/new.
- The deallocate() function pushes freed memory back into the free list.
- Objects of the Test class use operator new and operator delete to allocate/deallocate memory from the pool.

### License
This project is released under the MIT License.

### Author
Developed by Srinath Duvvuri.
