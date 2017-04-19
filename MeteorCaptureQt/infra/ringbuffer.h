#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include "infra/image.h"

#include <vector>
#include <memory>               // shared_ptr

#include <QDebug>

/**
 * @brief The RingBuffer class
 * http://www.cplusplus.com/forum/beginner/176072/
 */
class RingBuffer
{

public:
    RingBuffer(std::size_t cap) : buffer(cap) {}

    bool empty() const {
        return sz == 0;
    }

    bool full() const {
        return sz == buffer.size();
    }

    void push(std::shared_ptr<Image>  str )
    {
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
        buffer.clear();
        first = 0;
        last = -1;
        sz = 0;
    }

    std::shared_ptr<Image> back() {
        if(empty()) {
            return std::shared_ptr<Image>();
        }
        return buffer[last];
    }

    std::shared_ptr<Image>& operator[] ( std::size_t pos )
    {
        auto p = ( first + pos ) % buffer.size() ;
        return buffer[p];
    }

    std::vector<std::shared_ptr<Image>> unroll() {

        std::vector<std::shared_ptr<Image>> unrolled;

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

//    std::ostream& print( std::ostream& stm = std::cout ) const
//    {
//        if( first < last )
//            for( std::size_t i = first ; i < last ; ++i ) std::cout << buffer[i] << ' ' ;
//        else
//        {
//            for( std::size_t i = first ; i < buffer.size() ; ++i ) std::cout << buffer[i] << ' ' ;
//            for( std::size_t i = 0 ; i < last ; ++i ) std::cout << buffer[i] << ' ' ;
//        }
//        return stm ;
//    }

private:

    // The ring buffer data packaged in a vector
    std::vector<std::shared_ptr<Image>> buffer;

    // Index of the first element in the ring
    std::size_t first = 0;

    // Stores the index of the last (most recent) element to be added
    std::size_t last = -1;

    // Number of elements in the ring buffer currently
    std::size_t sz = 0;

};

#endif // RINGBUFFER_H
