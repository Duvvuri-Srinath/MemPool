#include <iostream>
#include <vector>
#include <atomic>
#include <cstdlib>
#include <stdexcept>
#include <utility>
#include <new>

// High Performance Memory Pool Class
class MemoryPool {
private:
    struct Node {
        Node* next;
    };

    std::atomic<Node*> freeList;         // Lock-free free list head
    std::vector<void*> allocatedChunks;  // Holds all allocated chunks for later cleanup
    size_t blockSize;                    // Size of each block (should be >= sizeof(Node))
    size_t blockCount;                   // Number of blocks per chunk

public:
    // Constructor: pre-allocates one chunk immediately.
    MemoryPool(size_t blockSize, size_t blockCount)
        : blockSize(blockSize), blockCount(blockCount), freeList(nullptr) {
        // Ensure blockSize is large enough to store a Node pointer.
        if (blockSize < sizeof(Node))
            blockSize = sizeof(Node);
        allocateChunk();
    }

    // Destructor: free all allocated memory chunks.
    ~MemoryPool() {
        for (void* chunk : allocatedChunks) {
            std::free(chunk);
        }
    }

    // Allocate a block from the pool
    void* allocate() {
        Node* head = freeList.load(std::memory_order_acquire);
        while (head) {
            // Try to pop the head from the free list
            if (freeList.compare_exchange_weak(head, head->next,
                                               std::memory_order_acquire,
                                               std::memory_order_relaxed)) {
                return static_cast<void*>(head);
            }
        }
        // No free block available; allocate a new chunk and try again.
        allocateChunk();
        return allocate();
    }

    // Deallocate a block and push it back onto the free list
    void deallocate(void* ptr) {
        Node* node = static_cast<Node*>(ptr);
        Node* head = freeList.load(std::memory_order_acquire);
        do {
            node->next = head;
        } while (!freeList.compare_exchange_weak(head, node,
                                                 std::memory_order_release,
                                                 std::memory_order_relaxed));
    }

    // Delete copy constructor and copy assignment operator
    MemoryPool(const MemoryPool&) = delete;
    MemoryPool& operator=(const MemoryPool&) = delete;

    // Move constructor: transfers ownership of resources.
    MemoryPool(MemoryPool&& other) noexcept
        : freeList(other.freeList.load(std::memory_order_relaxed)),
          allocatedChunks(std::move(other.allocatedChunks)),
          blockSize(other.blockSize),
          blockCount(other.blockCount) {
        other.freeList.store(nullptr, std::memory_order_relaxed);
        other.blockSize = 0;
        other.blockCount = 0;
    }

    // Move assignment operator: transfers ownership and cleans up existing resources.
    MemoryPool& operator=(MemoryPool&& other) noexcept {
        if (this != &other) {
            // Free current resources.
            for (void* chunk : allocatedChunks) {
                std::free(chunk);
            }
            allocatedChunks = std::move(other.allocatedChunks);
            freeList.store(other.freeList.load(std::memory_order_relaxed), std::memory_order_relaxed);
            blockSize = other.blockSize;
            blockCount = other.blockCount;

            other.freeList.store(nullptr, std::memory_order_relaxed);
            other.blockSize = 0;
            other.blockCount = 0;
        }
        return *this;
    }

private:
    // Allocates a new chunk of memory and populates the free list with blocks.
    void allocateChunk() {
        size_t chunkSize = blockSize * blockCount;
        void* chunk = std::malloc(chunkSize);
        if (!chunk) {
            throw std::bad_alloc();
        }
        allocatedChunks.push_back(chunk);
        // Break the chunk into blocks and add them to the free list.
        char* block = static_cast<char*>(chunk);
        for (size_t i = 0; i < blockCount; ++i) {
            deallocate(block + i * blockSize);
        }
    }
};

// Test class that uses MemoryPool for custom allocation.
class Test {
public:
    int x;
    int y;

    Test(int a, int b) : x(a), y(b) {}

    void print() const {
        std::cout << "Test(" << x << ", " << y << ")\n";
    }

    // Overload new and delete operators to use the MemoryPool.
    static void* operator new(size_t size);
    static void operator delete(void* ptr);

private:
    // A static MemoryPool instance for Test objects.
    // Pre-allocates space for 10 Test objects.
    static MemoryPool pool;
};

// Define and initialize the static MemoryPool for Test objects.
MemoryPool Test::pool(sizeof(Test), 10);

void* Test::operator new(size_t size) {
    return pool.allocate();
}

void Test::operator delete(void* ptr) {
    pool.deallocate(ptr);
}

// Example usage of the custom memory pool.
int main() {
    // Create Test objects using the custom memory pool.
    Test* t1 = new Test(1, 2);
    Test* t2 = new Test(3, 4);

    t1->print();
    t2->print();

    // Delete objects, returning memory back to the pool.
    delete t1;
    delete t2;

    std::cout << "Custom Memory Pool test completed.\n";
    return 0;
}
