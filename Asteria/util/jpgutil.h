#ifndef JPGUTIL_H
#define JPGUTIL_H

#include <vector>
#include <stdio.h>
extern "C" {
    #include <jpeglib.h>
}


class JpgUtil {

public:
    JpgUtil();

    /**
     * @brief Reads an image in JPEG format and converts it to an array of 8-bit greyscale pixels.
     * @param buffer
     *  Pointer to the start of the memory buffer containing the JPEG image data.
     * @param insize
     *  Length of the JPEG image data [bytes].
     * @param decodedImage
     *  Vector to which the image data will be written as 8-bit greyscale pixel values.
     */
    static void readJpeg(unsigned char *buffer, const unsigned long insize, std::vector<unsigned char> &decodedImage);

    /**
     * @brief Writes an image from an array of 8-bit greyscale pixels to a JPEG file.
     * @param image
     *  Vector containing the pixels.
     * @param width
     *  The image width [pixels].
     * @param height
     *  The image height [pixels].
     * @param filename
     *  The path to the JPEG file.
     */
    static void writeJpeg(std::vector<unsigned char> &image, const unsigned int width, const unsigned int height, char *filename);


    static void convertYuyv422(unsigned char * buffer, const unsigned long insize, std::vector<unsigned char> &decodedImage);
};

#endif // JPGUTIL_H
