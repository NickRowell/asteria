#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include "infra/imageuc.h"

#include <vector>
#include <memory>               // shared_ptr

#include <QDebug>

/**
 * @brief The RingBuffer class
 * http://www.cplusplus.com/forum/beginner/176072/
 */
template<class T> class RingBuffer
{

public:
    RingBuffer(std::size_t cap): buffer(cap) {

    }

    bool empty() const {
        return sz == 0;
    }

    bool full() const {
        return sz == buffer.size();
    }

    std::size_t size() const {
        return sz;
    }

    void push(T str) {

        // TODO: can do this increment more intelligently like for 'first'
        ++last;
        if( last >= buffer.size() ) {
            // Wrap around
            last = 0 ;
        }
        buffer[last] = str;

        if(full()) {
            first = (first+1) %  buffer.size();
        }
        else {
            ++sz;
        }
    }

    void clear() {
        // NOTE: don't clear the underlying vector otherwise
        // it will remove all the ring buffer elements.
        first = 0;
        last = -1;
        sz = 0;
    }

    T front() {
        if(empty()) {
            return T();
        }
        return buffer[first];
    }

    T back() {
        if(empty()) {
            return T();
        }
        return buffer[last];
    }

    T& operator[] ( std::size_t pos ) {
        auto p = ( first + pos ) % buffer.size() ;
        return buffer[p];
    }

    std::vector<T> unroll() {

        std::vector<T> unrolled;

        if( first < last ) {
            for( std::size_t i = first ; i < last ; ++i ) {
                unrolled.push_back(buffer[i]);
            }
        }
        else
        {
            for( std::size_t i = first ; i < buffer.size() ; ++i ) {
                unrolled.push_back(buffer[i]);
            }
            for( std::size_t i = 0 ; i < last ; ++i ) {
                unrolled.push_back(buffer[i]);
            }
        }
        return unrolled;
    }

private:

    // The ring buffer data packaged in a vector
    std::vector<T> buffer;

    // Index of the first element in the ring
    std::size_t first = 0;

    // Stores the index of the last (most recent) element to be added
    std::size_t last = -1;

    // Number of elements in the ring buffer currently
    std::size_t sz = 0;

};

#endif // RINGBUFFER_H
