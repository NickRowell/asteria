#include "ringbuffer.h"


RingBuffer::RingBuffer(std::size_t cap) : buffer(cap) {

}

bool RingBuffer::empty() const {
    return sz == 0;
}

bool RingBuffer::full() const {
    return sz == buffer.size();
}

void RingBuffer::push(std::shared_ptr<Image>  str )
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

void RingBuffer::clear() {
    buffer.clear();
    first = 0;
    last = -1;
    sz = 0;
}

std::shared_ptr<Image> RingBuffer::back() {
    if(empty()) {
        return std::shared_ptr<Image>();
    }
    return buffer[last];
}

std::shared_ptr<Image>& RingBuffer::operator[] ( std::size_t pos )
{
    auto p = ( first + pos ) % buffer.size() ;
    return buffer[p];
}

std::vector<std::shared_ptr<Image>> RingBuffer::unroll() {

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
