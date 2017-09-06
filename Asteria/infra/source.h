#ifndef SOURCE_H
#define SOURCE_H

#include <vector>

/**
 * @brief The Source class
 * Represents a source extracted from an image.
 */
class Source
{
public:
    Source();

    /**
     * @brief pixels
     * Contains the indices of the pixels assigned to this source
     */
    std::vector<unsigned int> pixels;

};

#endif // SOURCE_H
