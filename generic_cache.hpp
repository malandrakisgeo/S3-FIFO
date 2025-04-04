
using namespace std;

template<typename key, typename value>
class CacheI 
{
    public: 
        virtual bool insert(key k, value v) = 0;
        virtual bool remove(key k)= 0;
        virtual value get(key k) = 0;
        virtual void clear()= 0;
};