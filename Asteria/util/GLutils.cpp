
#include "GLutils.h"

#include <iostream>
#include <stdio.h>
#include <string.h>   /* strdup */
#include <fcntl.h>  /* open */
#include <unistd.h> /* lseek */
#include <math.h>

GLutils::GLutils(){}


void
GLutils::save_rgba_image_to_ppm(const char * fname, const GLubyte * image, const unsigned int &w, const unsigned int &h)
{
    std::ofstream out(fname);
    
    // For raw PPMs:
    out << "P6\n" << w << " " << h << " 255\n";
    for(unsigned int j=0; j<h; j++)
    {
        for(unsigned int i=0; i<w; i++)
        {
            // Pixel number
            unsigned int index = 4 * (j*w + i);
            
            char r = (char)(image[index+0]);
            char g = (char)(image[index+1]);
            char b = (char)(image[index+2]);
            char a = (char)(image[index+3]);

            out << r << g << b;
            
        }
    }
    
    out.close();
    
}

void
GLutils::save_rgb_image_to_ppm(const char * fname, const GLubyte * image, const unsigned int &w, const unsigned int &h)
{
    std::ofstream out(fname);
    
    // For raw PPMs:
    out << "P6\n" << w << " " << h << " 255\n";
    for(unsigned int j=0; j<h; j++)
    {
        for(unsigned int i=0; i<w; i++)
        {
            // Pixel number
            unsigned int index = 3 * (j*w + i);
            
            char r = (char)(image[index+0]);
            char g = (char)(image[index+1]);
            char b = (char)(image[index+2]);

            out << r << g << b;
        }
    }
    
    out.close();
    
}

void
GLutils::save_grey_image_to_pgm(const char * fname, const GLubyte * image, const unsigned int &w, const unsigned int &h)
{
    std::ofstream out(fname);
    
    // For raw PGMs:
    out << "P5\n" << w << " " << h << " 255\n";
    for(unsigned int j=0; j<h; j++)
    {
        for(unsigned int i=0; i<w; i++)
        {
            // Lookup grey level
            char grey = (char)(image[j*w + i]);
            out << grey;
        }
    }
    out.close();
}


void
GLutils::save_grey_image_to_pgm(const char * fname, const float * image, const unsigned int &w, const unsigned int &h)
{
    std::ofstream out(fname);
    
    // For raw PGMs:
    out << "P5\n" << w << " " << h << " 255\n";
    for(unsigned int j=0; j<h; j++)
    {
        for(unsigned int i=0; i<w; i++)
        {
            // Lookup grey level
            float grey = (image[j*w+i]*1000000000);
            // Saturate, don't wraparound
            if(grey > 255) grey = 255;
            
            out << (unsigned char)grey;
        }
    }
    out.close();
}
