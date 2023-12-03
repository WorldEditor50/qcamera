#ifndef MEMORYPOOL_HPP
#define MEMORYPOOL_HPP
#include <queue>
#include <mutex>
#include <memory>

class MemoryPool
{
private:
    std::mutex mutex;
    std::queue<unsigned char*> cache;
    constexpr static int max_cache_len = 64;
public:
    constexpr static int img_size = 1000;
public:
    MemoryPool(){}
    ~MemoryPool()
    {
        clear();
    }
    void reserve(std::size_t size)
    {
        for (std::size_t i = 0; i < size; i++) {
            unsigned char* ptr = new unsigned char[img_size];
            cache.push(ptr);
        }
        return;
    }
    void clear()
    {
        std::lock_guard<std::mutex> guard(mutex);
        for (std::size_t i = 0; i < cache.size(); i++) {
            unsigned char* ptr = cache.front();
            delete [] ptr;
            cache.pop();
        }
        return;
    }
    unsigned char* get()
    {
        std::lock_guard<std::mutex> guard(mutex);
        unsigned char* ptr = nullptr;
        if (cache.empty()) {
            ptr = new unsigned char[img_size];
        } else {
            ptr = cache.front();
            cache.pop();
        }
        memset(ptr, 0, img_size);
        return ptr;
    }
    void put(unsigned char* ptr)
    {
        std::lock_guard<std::mutex> guard(mutex);
        if (cache.size() > max_cache_len) {
            delete [] ptr;
        } else {
            cache.push(ptr);
        }
        return;
    }
};

#endif // MEMORYPOOL_HPP
