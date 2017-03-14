#ifndef JPGUTIL_H
#define JPGUTIL_H

#include <stdio.h>
extern "C" {
    #include <jpeglib.h>
}



class JpgUtil
{
public:
    JpgUtil();

    static void convertJpeg(unsigned char * buffer, const unsigned long insize, char * decodedImage);

    static void convertYuyv422(unsigned char * buffer, const unsigned long insize, char * decodedImage);
};

#endif // JPGUTIL_H