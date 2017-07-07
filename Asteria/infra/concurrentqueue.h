#ifndef CONCURRENTQUEUE_H
#define CONCURRENTQUEUE_H

#include <mutex>
#include <queue>

/**
 * Simple thread-safe queue implementation, for use in producer-consumer scenarios.
 * See e.g. https://www.justsoftwaresolutions.co.uk/threading/implementing-a-thread-safe-queue-using-condition-variables.html
 */
template<typename Data>
class ConcurrentQueue
{

private:
    std::queue<Data> the_queue;
    mutable std::mutex the_mutex;

public:

    void push(const Data& data) {
        std::lock_guard<std::mutex> lock(the_mutex);
        the_queue.push(data);
    }

    bool pop(Data & data) {
        std::lock_guard<std::mutex> lock(the_mutex);
        if(the_queue.empty()) {
            return false;
        }
        data = the_queue.front();
        the_queue.pop();
        return true;
    }

};

#endif // CONCURRENTQUEUE_H
