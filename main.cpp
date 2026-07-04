#include <iostream>
#include <chrono>
#include <vector>
#include "MemoryPool.h"

// A dummy object to represent a financial order
struct Order {
    int id;
    double price;
    int quantity;
    bool is_buy;

    // A simple constructor
    Order(int i, double p, int q, bool b) 
        : id(i), price(p), quantity(q), is_buy(b) {}
};

int main() {
    const size_t NUM_ALLOCATIONS = 1000000;
    std::cout << "Starting benchmark for " << NUM_ALLOCATIONS << " allocations...\n\n";

    // -------------------------------------------------------------
    // Test 1: Standard 'new' and 'delete'
    // -------------------------------------------------------------
    std::vector<Order*> standard_orders(NUM_ALLOCATIONS);
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Allocate
    for (size_t i = 0; i < NUM_ALLOCATIONS; ++i) {
        standard_orders[i] = new Order(i, 100.50, 100, true);
    }
    
    // Deallocate
    for (size_t i = 0; i < NUM_ALLOCATIONS; ++i) {
        delete standard_orders[i];
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> standard_duration = end_time - start_time;
    
    std::cout << "Standard C++ (new/delete): " << standard_duration.count() << " ms\n";


    // -------------------------------------------------------------
    // Test 2: Custom Memory Pool
    // -------------------------------------------------------------
    // We instantiate our pool to hold at least NUM_ALLOCATIONS
    MemoryPool<Order, NUM_ALLOCATIONS> order_pool;
    std::vector<Order*> pool_orders(NUM_ALLOCATIONS);

    start_time = std::chrono::high_resolution_clock::now();
    
    // Allocate
    for (size_t i = 0; i < NUM_ALLOCATIONS; ++i) {
        pool_orders[i] = order_pool.allocate(i, 100.50, 100, true);
    }
    
    // Deallocate
    for (size_t i = 0; i < NUM_ALLOCATIONS; ++i) {
        order_pool.deallocate(pool_orders[i]);
    }
    
    end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> pool_duration = end_time - start_time;
    
    std::cout << "Custom Memory Pool:        " << pool_duration.count() << " ms\n";
    
    // Calculate the speedup
    std::cout << "\nPerformance Gain: " << (standard_duration.count() / pool_duration.count()) << "x faster!\n";

    return 0;
}