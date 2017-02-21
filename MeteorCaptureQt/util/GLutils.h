/* 
 * File:   GLutils.h
 * Author: nickrowell
 *
 * Created on 31 January 2014, 16:50
 */

#ifndef GLUTILS_H
#define	GLUTILS_H

// GL stuff - must be included in the right order!
//#include <GL/glew.h>
#include <GL/glu.h>
#include <GL/gl.h>
//#include <GL/glut.h>

#include <fstream>      // std::ofstream
#include <iostream>     // std::cin, std::cout
#include <sstream>

//#include "vec3.h"


class GLutils
{
    
public:
    
    // Vertex attribute indices used in my shaders
    static const GLuint PositionAttributeIndex = 0;
    static const GLuint TexCoordAttributeIndex = 1;
    static const GLuint NormalAttributeIndex   = 2;
    static const GLuint ColourAttributeIndex   = 3;
    
    // Default constructor
    GLutils();

    // Utility functions used to draw certain types of object in the
    // current rendering context.
    //void sphere(const Vec3<float> &, const Vec3<float> &, const float &, const unsigned int &);
    //void chessBoard(const unsigned int &, const unsigned int &, const float &);

    // Functions to load and render an ICQ file
    //bool loadICQ(const char * fname, unsigned int &q, Vec3<float> ** &verts, Vec3<float> ** &norms);
    //unsigned int getVertexNumber(unsigned int q, unsigned int i, unsigned int j, unsigned int k);
    //void icq(unsigned int &q, Vec3<float> ** &verts, Vec3<float> ** &norms, const float & albedo);

    /**
     * This function writes the contents of the array to disk as a PPM-type image.
     * It applies a linear transformation (scale & shift) to the values in the array
     * in order to keep the grey levels within an 8-bit range.
     * @param fname Filename (full path)
     * @param image Pointer to start of array containing data
     * @param w     Image width
     * @param h     Image height
     */
    void
    save_rgba_image_to_ppm(const char * fname, const GLubyte * image, const unsigned int &w, const unsigned int &h);

    void
    save_rgb_image_to_ppm(const char * fname, const GLubyte * image, const unsigned int &w, const unsigned int &h);

    void
    save_grey_image_to_pgm(const char * fname, const GLubyte * image, const unsigned int &w, const unsigned int &h);

    void
    save_grey_image_to_pgm(const char * fname, const float * image, const unsigned int &w, const unsigned int &h);

    /**
     * Reads a raw PPM image from a file and returns a pointer to the start of the
     * image data.
     * @param fname         String containing path to PPM file
     * @param width         The image width is read from the file and stored here.
     * @param height        The image height is read from the file and stored here.
     * @return 
     */
    template < typename T >
    T *
    load_ppm_from_file(const char * fname, unsigned int &width, unsigned int &height)
    {

        std::ifstream in(fname);

        // Check file OK for reading
        if (!in)
        {
            std::cerr
                    << "Error: unable to open \""
                    << fname
                    << "\" for reading.\n";
        }

        // Read entire file into a string
        std::string message;
        while (in)
        {
          message.push_back(in.get());
        }
        in.close();

        // Get number of chars in string
        int N = message.size();

        // Get pointer to start of array containing chars - must be unsigned!
        const char * ptr = message.data();

        // Check magic number (first two chars in file)
        if(*(ptr++) != 'P' || *(ptr++) != '6') 
        {
            std::cout << "Failed to read image as PPM, magic number wrong.\n" << std::endl;
            return NULL;
        }

        /* Eat ASCII characters until numerals 0-9 are found */
        while (*(ptr) < 48 || *(ptr) > 57) { ptr++;}

        /* Read image width */

        /* Numerals 0 - 9 are codes 48 - 57 in ASCII */
        width = 0;
        while( *(ptr) > 47 && *(ptr) < 58)
        {
            /* Another digit found. Raise current width by one power of ten */
            width *= 10;
            /* Add the new digit at the lowest order. */
            width += (*(ptr) - 48);
            /* Advance pointer */
            ptr++;
        }

        /* Eat ASCII characters until numerals 0-9 are found */
        while (*(ptr) < 48 || *(ptr) > 57) { ptr++;}

        /* Same again for image height */
        height = 0;
        while( *(ptr) > 47 && *(ptr) < 58)
        {
            height *= 10;
            height += (*(ptr) - 48);
            ptr++;
        }

        /* Eat ASCII characters until numerals 0-9 are found */
        while (*(ptr) < 48 || *(ptr) > 57) { ptr++;}

        /* Read maximum pixel value */
        unsigned int maxpix = 0;

        while( *(ptr) > 47 && *(ptr) < 58)
        {
            maxpix *= 10;
            maxpix += (*(ptr) - 48);
            ptr++;
        }   

        /* Skip one white space character to align with start of pixel data */
        ptr++;

        // Allocate dynamic memory to store pixel data
        T * image = new T[3*width*height];

        for(unsigned int i=0; i<3*width*height; i++)
        {
            // Must first cast to an unsigned char, otherwise numbers in the upper
            // half of the 8-bit range are shifted to negative values, which then
            // get shifted to very large positive values if we cast to an unsigned
            // type afterwards.
            image[i] = (T)(unsigned char)*(ptr++);
        }

        return image;
    }

    /**
     * Reads a raw PGM image from a file and returns a pointer to the start of the
     * image data.
     * @param fname         String containing path to PGM file
     * @param width         The image width is read from the file and stored here.
     * @param height        The image height is read from the file and stored here.
     * @return 
     */
    template < typename T >
    T *
    load_pgm_from_file(const char * fname, unsigned int &width, unsigned int &height)
    {

        std::ifstream in(fname);

        // Check file OK for reading
        if (!in)
        {
            std::cerr
                    << "Error: unable to open \""
                    << fname
                    << "\" for reading.\n";
        }

        // Read entire file into a string
        std::string message;
        while (in)
        {
          message.push_back(in.get());
        }
        in.close();

        // Get number of chars in string
        int N = message.size();

        // Get pointer to start of array containing chars - must be unsigned!
        const char * ptr = message.data();

        // Check magic number (first two chars in file)
        if(*(ptr++) != 'P' || *(ptr++) != '5') 
        {
            std::cout << "Failed to read image as PGM, magic number wrong.\n" << std::endl;
            return NULL;
        }

        /* Eat ASCII characters until numerals 0-9 are found, ignoring comment lines
         * which are indicated by a hash sign: everything between a hash sign and the end
         * of the line that it occurred in is ignored.
         */
        bool ISCOMMENT=false;
        while (*(ptr) < 48 || *(ptr) > 57 || ISCOMMENT)
        { 
            if(*(ptr)=='#') ISCOMMENT = true;     // Activate comment flag when # found.
            if(*(ptr)=='\n') ISCOMMENT = false;   // Deactivate on end of line
            if(*(ptr)=='\r') ISCOMMENT = false;   // or carriage return.
            ptr++;                                // Onto next character.
        }

        /* Read image width */

        /* Numerals 0 - 9 are codes 48 - 57 in ASCII */
        width = 0;
        while( *(ptr) > 47 && *(ptr) < 58)
        {
            /* Another digit found. Raise current width by one power of ten */
            width *= 10;
            /* Add the new digit at the lowest order. */
            width += (*(ptr) - 48);
            /* Advance pointer */
            ptr++;
        }

        /* Eat ASCII characters until numerals 0-9 are found */
        while (*(ptr) < 48 || *(ptr) > 57 || ISCOMMENT)
        { 
            if(*(ptr)=='#') ISCOMMENT = true;     // Activate comment flag when # found.
            if(*(ptr)=='\n') ISCOMMENT = false;   // Deactivate on end of line
            if(*(ptr)=='\r') ISCOMMENT = false;   // or carriage return.
            ptr++;                                // Onto next character.
        }

        /* Same again for image height */
        height = 0;
        while( *(ptr) > 47 && *(ptr) < 58)
        {
            height *= 10;
            height += (*(ptr) - 48);
            ptr++;
        }

        /* Eat ASCII characters until numerals 0-9 are found */
        while (*(ptr) < 48 || *(ptr) > 57 || ISCOMMENT)
        { 
            if(*(ptr)=='#') ISCOMMENT = true;     // Activate comment flag when # found.
            if(*(ptr)=='\n') ISCOMMENT = false;   // Deactivate on end of line
            if(*(ptr)=='\r') ISCOMMENT = false;   // or carriage return.
            ptr++;                                // Onto next character.
        }

        /* Read maximum pixel value */
        unsigned int maxpix = 0;

        while( *(ptr) > 47 && *(ptr) < 58)
        {
            maxpix *= 10;
            maxpix += (*(ptr) - 48);
            ptr++;
        }   

        /* Skip one white space character to align with start of pixel data */
        ptr++;

        // Allocate dynamic memory to store pixel data
        T * image = new T[width*height];

        for(unsigned int i=0; i<width*height; i++)
        {
            // Must first cast to an unsigned char, otherwise numbers in the upper
            // half of the 8-bit range are shifted to negative values, which then
            // get shifted to very large positive values if we cast to an unsigned
            // type afterwards.
            image[i] = (T)(unsigned char)*(ptr++);
        }

        return image;
    }
    
};

#endif	/* GLUTILS_H */

