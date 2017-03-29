#ifndef VECTORCHAR_H
#define VECTORCHAR_H

#include <vector>

/**
 * Using this typedef to represent a vector<char> allows us to pass instances by reference
 * to Qt containers. This is very useful when using a vector<char> to store an image because
 * it means the image can be passed around by reference using Qt's signal/slot mechanism
 * without implicitly copying all the image data to a new vector<char>.
 * @brief VectorChar
 */

typedef std::vector<char> VectorChar;
Q_DECLARE_METATYPE(VectorChar)

#endif // VECTORCHAR_H
