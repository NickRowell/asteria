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
    RingBuffer(std::size_t cap);

    bool empty() const;

    bool full() const;

    void push(std::shared_ptr<Image>  str );

    void clear();

    std::shared_ptr<Image> back();

    std::shared_ptr<Image>& operator[] ( std::size_t pos );

    std::vector<std::shared_ptr<Image>> unroll();

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
