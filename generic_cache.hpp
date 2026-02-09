
#ifndef GENERIC_CACHE_HPP
#define GENERIC_CACHE_HPP
using namespace std;
#endif 

template<typename value>
class value_wrapper{
    public:

    value_wrapper(){
    }

    value_wrapper(value val){
        v = val;
        hits = 0;
    }

    void decrease_hits(){
        --hits;
    }


  value get_value(){
    if(hits<=3){ //Relevant only for S3 fifo!
        hits++;
    }
        return v;
    }

    int get_hits(){
        return hits;
    }

    void update_value(value val){
        v=val;
    }

    private: 
        int hits = 0;
        value v ;

};


template<typename key, typename value>
class CacheI 
{
    public: 
        typedef typename std::pair<key, value_wrapper<value>> key_value_pair_t;
        virtual ~CacheI() = default;
        virtual bool insert(const key k, value v) = 0;
        virtual bool remove(const key &k)= 0;
        virtual value get(const key &k) = 0;
        virtual void evict()= 0;
};

