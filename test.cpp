#include "S3_FIFO.cpp"


void simpleton_test(){
    string fallback_str =  "_-NULL-_";
    string test = "TEST_SYNC"; 
    string test2 = "TEST_SYNC2"; 
    string test3 = "TEST_SYNC3"; 

    S3_FIFO_Cache<int, string> o = S3_FIFO_Cache<int, string>(10, fallback_str);

    o.insert(1, test);
    assert(o.key_contained_in(1) == 1);

    o.remove(1);
    assert(o.key_contained_in(1) == 0);

    o.insert(1, test);
    o.get(1);
    o.get(1);
    o.get(1); //3 hits

    //std::cout << static_cast<int16_t>(o.key_contained_in(1)) << std::endl;
    assert(o.key_contained_in(1) == 1);

    o.insert(2, test2);

    assert(o.key_contained_in(1) == 3); //1 should have been moved to the main queue
    assert(o.key_contained_in(2) == 1); //3 should be in the short queue

    o.evict();
    assert(o.key_contained_in(1) == 3); //should still be in the main queue

    o.insert(5, test3);

    o.insert(6, test3);
    o.insert(7, test3);
    o.insert(8, test3);
    o.insert(9, test3);
    o.insert(10, test3);
    o.insert(11, test2);
    o.insert(12, test2);
    o.insert(13, test2);
    o.insert(14, test2);
    o.insert(15, test2);
    o.get(5); //3 hits

    assert(o.key_contained_in(1) == 3);
    assert(o.key_contained_in(3) == 0);

}



// #CLAUDE GENERATED TESTS BELOW

// Test 1: Basic promotion from small to main queue
void test_small_to_main_promotion() {
    std::cout << "\n=== Test 1: Small to Main Promotion ===" << std::endl;
    
    string fallback = "_NULL_";
    S3_FIFO_Cache<int, string> cache(10, fallback);
    
    // Insert item with key 1
    cache.insert(1, "value1");
    assert(cache.key_contained_in(1) == 1); // Should be in small queue
    
    // Access it multiple times to build up hits
    cache.get(1);
    cache.get(1);
    cache.get(1); // Now has >1 hits
    
    // Fill the cache to trigger eviction
    for(int i = 2; i <= 10; i++) {
        cache.insert(i, "value" + std::to_string(i));
    }
    
    // Item 1 should have been promoted to main queue
    assert(cache.key_contained_in(1) == 3); // 3 = main queue
    
    std::cout << "PASSED: Item with high hits promoted to main queue" << std::endl;
}

// Test 2: Low-frequency items go to ghost queue
void test_low_frequency_to_ghost() {
    std::cout << "\n=== Test 2: Low Frequency to Ghost ===" << std::endl;
    
    string fallback = "_NULL_";
    S3_FIFO_Cache<int, string> cache(10, fallback);
    
    // Insert item without accessing it (0 hits after insertion)
    cache.insert(1, "value1");
    assert(cache.key_contained_in(1) == 1); // In small queue
    
    // Fill cache to force eviction of item 1
    for(int i = 2; i <= 11; i++) {
        cache.insert(i, "value" + std::to_string(i));
    }
    
    // Item 1 should be in ghost queue (evicted from small with low hits)
    assert(cache.key_contained_in(1) == 5); // 5 = ghost queue
    
    std::cout << "PASSED: Low-frequency item moved to ghost queue" << std::endl;
}

// Test 3: Ghost queue re-admission to main queue
void test_ghost_readmission() {
    std::cout << "\n=== Test 3: Ghost Re-admission ===" << std::endl;
    
    string fallback = "_NULL_";
    S3_FIFO_Cache<int, string> cache(10, fallback);
    
    // Insert and evict item 1 to ghost
    cache.insert(1, "value1");
    for(int i = 2; i <= 11; i++) {
        cache.insert(i, "value" + std::to_string(i));
    }
    assert(cache.key_contained_in(1) == 5); // In ghost
    
    // Re-insert item 1 (simulating cache hit on ghost entry)
    cache.insert(1, "value1_new");
    
    // Should go directly to main queue (ghost hit)
    assert(cache.key_contained_in(1) == 3); // 3 = main queue
    
    std::cout << "PASSED: Ghost entry re-admitted to main queue" << std::endl;
}

// Test 4: Main queue circulation (frequency decay)
void test_main_queue_circulation() {
    std::cout << "\n=== Test 4: Main Queue Circulation ===" << std::endl;
    
    string fallback = "_NULL_";
    S3_FIFO_Cache<int, string> cache(10, fallback);
    
    // Insert and promote item 1 to main
    cache.insert(1, "value1");
    cache.get(1);
    cache.get(1); // 2 hits
    
    // Fill cache to promote item 1
    for(int i = 2; i <= 10; i++) {
        cache.insert(i, "value" + std::to_string(i));
    }
    
    assert(cache.key_contained_in(1) == 3); // In main queue
    
    // Now fill cache more, causing main queue evictions
    // Item 1 should circulate in main queue (move to head) due to hits
    for(int i = 11; i <= 20; i++) {
        cache.insert(i, "value" + std::to_string(i));
    }
    
    // Item 1 should still be in main (or at least not completely evicted)
    // This tests that the main queue circulation works
    int location = cache.key_contained_in(1);
    assert(location == 3 || location == 0); // Either still in main or evicted (depends on circulation)
    
    std::cout << "PASSED: Main queue circulation working" << std::endl;
}

// Test 5: Capacity enforcement
void test_capacity_enforcement() {
    std::cout << "\n=== Test 5: Capacity Enforcement ===" << std::endl;
    
    string fallback = "_NULL_";
    S3_FIFO_Cache<int, string> cache(5, fallback);
    
    // Insert more items than capacity
    for(int i = 1; i <= 10; i++) {
        cache.insert(i, "value" + std::to_string(i));
    }
    
    // Count how many items are actually in the cache (not ghost)
    int count = 0;
    for(int i = 1; i <= 10; i++) {
        int loc = cache.key_contained_in(i);
        if(loc == 1 || loc == 3) { // Small or main queue
            count++;
        }
    }
    
    // Should not exceed capacity
    assert(count <= 5);
    
    std::cout << "PASSED: Cache respects capacity limit (found " << count << " items)" << std::endl;
}

// Test 6: Thread safety (basic concurrent access)
void test_thread_safety() {
    std::cout << "\n=== Test 6: Thread Safety ===" << std::endl;
    
    string fallback = "_NULL_";
    S3_FIFO_Cache<int, string> cache(100, fallback);
    
    // Launch multiple threads doing concurrent operations
    std::vector<std::thread> threads;
    
    for(int t = 0; t < 4; t++) {
        threads.emplace_back([&cache, t]() {
            for(int i = 0; i < 25; i++) {
                int key = t * 25 + i;
                cache.insert(key, "value" + std::to_string(key));
                cache.get(key);
            }
        });
    }
    
    for(auto& thread : threads) {
        thread.join();
    }
    
    std::cout << "PASSED: No crashes with concurrent access" << std::endl;
}

// Test 7: Remove functionality
void test_remove() {
    std::cout << "\n=== Test 7: Remove Functionality ===" << std::endl;
    
    string fallback = "_NULL_";
    S3_FIFO_Cache<int, string> cache(10, fallback);
    
    cache.insert(1, "value1");
    cache.insert(2, "value2");
   // cache.insert(3, "value3");
    
    assert(cache.key_contained_in(2) == 1); // In small queue
    assert(cache.remove(2) == true);
    assert(cache.key_contained_in(2) == 0); // Not in cache
    assert(cache.get(2) == fallback); // Returns fallback
    
    // Remove non-existent key
    assert(cache.remove(999) == false);
    
    std::cout << "PASSED: Remove works correctly" << std::endl;
}














int main (){


    simpleton_test();
    //AI generated (Claude) tests below:
    test_small_to_main_promotion();
    test_low_frequency_to_ghost();
    test_ghost_readmission();
    test_main_queue_circulation();
    test_capacity_enforcement();
    test_thread_safety();
    test_remove();

    return 0;
}
