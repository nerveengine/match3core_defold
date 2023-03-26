#include <queue>
#include <mutex>

template <typename T>
class WaitableQueue
{
public:

    // if need clear - call invoke with empty body CB
    template<typename CB>
    void invoke(CB callback)
    {
        std::lock_guard<std::mutex> lock(mutex);
        while (!queue.empty())
        {
            T item = queue.front();
            queue.pop();
            callback(item);
        }
    }

    void push(const T& item)
    {
        std::lock_guard<std::mutex> mlock(mutex);
        queue.push(item);
    }

    void push(T&& item)
    {
        std::lock_guard<std::mutex> lock(mutex);
        queue.push(std::move(item));
    }
    
    private:
    std::queue<T> queue;
    std::mutex mutex;
};