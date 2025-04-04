
#include <queue>
#include <iostream>
#include <list>
#include <cassert>
#include <stdexcept>
using namespace std;
#include "generic_cache.hpp"


template<typename key, typename value>
class FIFOCache: public CacheI<key, value>
{
    public:
    typedef typename std::pair<key, value> key_value_pair_t;


        FIFOCache(size_t size){
            this->max_size = size;
            this->fifo_queue = std::deque<key_value_pair_t>(size);
        }

     
        bool insert(key k, value v) {
            if(fifo_queue.size()==this->max_size){
                fifo_queue.pop_back();
            }
           fifo_queue.push_front(std::make_pair (k,v));
           return true;
        }   

        bool remove(key k){
            for (auto it = fifo_queue.begin(); it!=fifo_queue.end(); ++it){
                if(it->first == k){
                    fifo_queue.erase(it);
                    return true;
                }
            }
            return false;
        }
        value get(key k){
            for (auto it = fifo_queue.begin(); it!=fifo_queue.end(); ++it){
                if(it->first == k){
                    return it->second;
                }
            }
            throw std::range_error("Not found");
        }
        

        void clear(){
        }




    private: 
        std::deque<key_value_pair_t> fifo_queue;
        size_t max_size;
};



int main (){
    FIFOCache<int, string> obj = FIFOCache<int, string>((size_t)3);
        string test = "TEST"; 
        string test2 = "TEST2"; 

    obj.insert( 1,test);
    assert(obj.get(1) == test);
    obj.insert( 2,test);
    obj.insert( 3,test);
    obj.insert( 4,test2);
    try{    auto a = obj.get(1);
    }catch (const std::range_error& e) {
        assert(true);
    }

    assert( obj.get(4) == test2);

    return 0;
    }