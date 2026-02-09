
#ifndef FIFO_HPP
#define FIFO_HPP
#include <queue>
#include <iostream>
#include <list>
#include <cassert>
#include <stdexcept>
#include <thread>
#include <future>
using namespace std;
#include "generic_cache.hpp"
#endif 

#define DEFAULT_SIZE 100;

template < typename key, typename value >
class fifo_cache_wrapped: public CacheI < key, value > {
    public: typedef typename std::pair < key,
        value_wrapper < value >> key_value_pair_t;
        fifo_cache_wrapped() {
            this -> max_size = DEFAULT_SIZE;
            this -> fifo_queue = std::deque <key_value_pair_t> (); 
        }

        ~fifo_cache_wrapped() {
            evict();
        }

        fifo_cache_wrapped(const size_t size) {
            this -> max_size = size > 0 ? size : DEFAULT_SIZE;
            this -> fifo_queue = std::deque <key_value_pair_t> (); 
        }

        fifo_cache_wrapped(const size_t size, value fallback_val) {
            this -> max_size = size > 0 ? size : DEFAULT_SIZE;
            this -> fifo_queue = std::deque <key_value_pair_t> ();
            this -> fallback_value = fallback_val;
        }

        bool insert(const key k, value val) {
            std::unique_lock <std::mutex> lock(mtx);

            while (current_elements >= this -> max_size) {
                fifo_queue.pop_back();
                --current_elements;
            }

            for (auto it = fifo_queue.begin(); it != fifo_queue.end(); it++) {
                if (it->first == k) {
                    fifo_queue.erase(it);
                    --current_elements;
                    break;
                }
            }

            value_wrapper <value> v = value_wrapper(val);
            fifo_queue.push_front(std::make_pair(k, std::move(v)));
            ++current_elements;
            return true;
        }


        void move_to_head(const key &k) { //AI generated (Claude)
            std::unique_lock <std::mutex> lock(mtx);

            // Find and extract the element
            for (auto it = fifo_queue.begin(); it != fifo_queue.end(); ++it) {
                if (it->first == k) {

                    // Extract the pair
                    key_value_pair_t pair = std::move( * it);
                    fifo_queue.erase(it);
                    --current_elements;

                    // Modify and reinsert at head
                    pair.second.decrease_hits();
                    fifo_queue.push_front(std::move(pair));
                    ++current_elements;
                    return;
                }
            }
        }

        bool remove(const key &k) {
            std::unique_lock <std::mutex> lock(mtx);
            for (auto it = fifo_queue.begin(); it != fifo_queue.end(); it++) {
                if (it->first == k) {
                    fifo_queue.erase(it);
                    --current_elements;

                    return true;
                }
            }
            return false;
        }

        value get(const key &k) {
            std::unique_lock <std::mutex> lock(mtx);
            for (auto it = fifo_queue.begin(); it != fifo_queue.end(); ++it) {
                if (it->first == k) {
                    return it->second.get_value();
                }
            }
            return fallback_value;
        }

        int get_hits(key &k) {
            std::unique_lock <std::mutex> lock(mtx);
            for (auto it = fifo_queue.begin(); it != fifo_queue.end(); ++it) {
                if (it->first == k) {
                    return it->second.get_hits();
                }
            }
            return 0;
        }

        void evict() {
            fifo_queue.clear();
            current_elements = 0;
        }

        int get_current_size() {
            std::unique_lock <std::mutex> lock(mtx);
            return current_elements;
        }

        key_value_pair_t get_tail() {
            std::unique_lock <std::mutex> lock(mtx);

            return this->fifo_queue.back();
        }

        private: std::deque < key_value_pair_t > fifo_queue;
        size_t max_size;
        int current_elements = 0;
        value fallback_value;
        std::mutex mtx;
};

