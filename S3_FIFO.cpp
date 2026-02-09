#ifndef S3_FIFO_HPP
#define S3_FIFO_HPP
#include "FIFO.hpp"
#endif

template < typename key, typename value >
class S3_FIFO_Cache: public CacheI < key, value > {
    public: typedef typename std::pair < key,
        value_wrapper < value >> key_value_pair_t;

        S3_FIFO_Cache() {}

        S3_FIFO_Cache(const size_t size, value fallback): max_size(size),
         fallback_value(fallback),
         small_queue(size * 10 / 100, fallback),
         main_queue(size * 90 / 100, fallback),
         ghost_queue(size, fallback) {
        }

        ~S3_FIFO_Cache() {}

        bool insert(const key k, value val) {
            /*
             *  evict while full.
             *
             *  If k is in G, then insert k to head of M (and remove from G?)
             *  Else insert k to head of S.
             *
             */
            std::unique_lock < std::mutex > lock(mutex);

            evict(); //only evicts if necessary

            if (this->ghost_queue.get(k) != fallback_value) {

                this->main_queue.insert(k, val);
                this->ghost_queue.remove(k); //This is not explicitly mentioned in the paper.
            } else {
                this->small_queue.insert(k, val);
            }

            ++current_size;

            return true;
        }

        bool remove(const key &k) {
            std::unique_lock < std::mutex > lock(mutex);
            this->ghost_queue.remove(k);
            bool removed = this->small_queue.remove(k) || this->main_queue.remove(k);

            if (removed) {
                --current_size;
            }

            return removed;
        }

        value get(const key &k) {
            std::unique_lock <std::mutex> lock(mutex); 
            value res = this->small_queue.get(k);
            if (res == fallback_value) {
                res = this->main_queue.get(k);
            }
            if (res == fallback_value) {
                res = this->ghost_queue.get(k);
                if (res != fallback_value) {
                    //lock.unlock(); 
                   // insert(k, res); 
                    evict(); //evict if needed
                    this->main_queue.insert(k, res);
                    this->ghost_queue.remove(k); 
                    ++current_size;
                    return res;
                }
            }

            return res;
        }

        int key_contained_in(const key &k) { //Used for testing
            std::unique_lock < std::mutex > lock(mutex);

            int sum = 0;
            if (this->small_queue.get(k) != fallback_value) {
                sum += 1;
            }

            if (this->main_queue.get(k) != fallback_value) {
                sum += 3;
            }

            if (this->ghost_queue.get(k) != fallback_value) {
                sum += 5;
            }

            return sum;
        }

        void evict(){
            /*
             *  If S.size >= 10% * cache size, then evictS(). Else evictM()
             */

            if (this->small_queue.get_current_size() >= (max_size * 10 / 100)) {
                evictS();
            }
            if (this->main_queue.get_current_size() >= (max_size * 90 / 100)) {
                evictM();
            }
        }

        private: size_t max_size;
        size_t current_size = 0;
        fifo_cache_wrapped <key,value> small_queue;
        fifo_cache_wrapped <key, value> main_queue;
        fifo_cache_wrapped <key,value> ghost_queue;

        mutable std::mutex mutex;
        value fallback_value;

        void evictS(){
            /*
             *                1. Let t the tail of S.
             *                    if t.freq > 1, insert t to M (and evictM if M full)
             *                    else insert t to G, and remove from S
             */

            bool evicted = false;
            while (!evicted && this->small_queue.get_current_size() > 0) {
                key_value_pair_t t = this->small_queue.get_tail();

                if (t.second.get_hits() > 1) {
                    this->main_queue.insert(t.first, t.second.get_value()); //or: fallback-v
                    if (this->main_queue.get_current_size() >= (max_size * 90 / 100)) {
                        evictM();
                    }

                } else {
                    this->ghost_queue.insert(t.first, t.second.get_value()); //or: fallback-v
                    evicted = true;
                }
                this->small_queue.remove(t.first);
                --current_size;
            }
        }

        void evictM() {
            /*
             *  let t tail of M.
             *
             *  if t.freq > 0 then
             *      Insert t to head of M
             *      --t.freq
             *  else
             *      remove t from M
             */

            bool evicted = false;
            while (!evicted && this->main_queue.get_current_size() > 0) {
                key_value_pair_t t = this->main_queue.get_tail();
                if (t.second.get_hits() > 0) {
                    this->main_queue.move_to_head(t.first); //decreases the frequency too
                } else {
                    this->main_queue.remove(t.first);
                    --current_size;
                    evicted = true;
                }
            }
        }

};
