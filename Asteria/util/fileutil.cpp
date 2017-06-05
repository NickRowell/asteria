
#include "fileutil.h"

#include <iostream>
#include <stdio.h>
#include <string.h>   /* strdup */
#include <fcntl.h>  /* open */
#include <unistd.h> /* lseek */
#include <math.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <QDebug>

FileUtil::FileUtil()
{

}


bool FileUtil::deleteFilePath(std::string path) {

    // Is the path a regular file? If so, delete and return.

    // First, check if path exists. NOTE: we MUST use lstat(...) rather than stat(...),
    // otherwise this will follow any symlinks that have crept in and delete everything
    // there, possibly with disastrous consequences...
    struct stat info;
    if( lstat( path.c_str(), &info ) != 0 ) {
        // File does not exist - nothing to delete!
        return true;
    }

    // What type of object is this?
    if(S_ISCHR(info.st_mode)) {
        // Character device - don't touch it
        return false;
    }

    if(S_ISBLK(info.st_mode)) {
        // Block device - don't touch it
        return false;
    }

    if(S_ISFIFO(info.st_mode)) {
        // FIFO (named pipe) - don't touch it
        return false;
    }

    if(S_ISLNK(info.st_mode)) {
        // Symbolic link - don't touch it
        qInfo() << "Not deleting link " << path.c_str();
        return false;
    }

    if(S_ISSOCK(info.st_mode)) {
        // Socket - don't touch it
        return false;
    }

    if(S_ISREG(info.st_mode)) {
        // Regular file - delete and return.
        return remove( path.c_str() ) == 0;
    }

    if(S_ISDIR(info.st_mode)) {
        // Directory: delete all of the contents then delete it.
        DIR *dir;
        if ((dir = opendir (path.c_str())) == NULL) {
            // Couldn't open the directory!
            return false;
        }

        // Loop over the contents of the directory and delete each item in turn
        struct dirent *item;
        bool dirNowEmpty = true;
        while ((item = readdir (dir)) != NULL) {

            // VITAL that we skip the . and .. directories
            if(strcmp(item->d_name,".") == 0 || strcmp(item->d_name,"..") == 0) {
                continue;
            }

            // Build full path to the item
            std::string subPath = path + "/" + item->d_name;
            // Recursively delete this
            if(!deleteFilePath(subPath)) {
                dirNowEmpty = false;
            }
        }
        closedir (dir);

        // Only delete the directory if we managed to delete all of it's contents.
        if(dirNowEmpty) {
            return remove( path.c_str() ) == 0;
        }
        else {
            return false;
        }
    }

    // Unrecognized type of object! Don't touch it.
    return false;
}

void
FileUtil::save_rgba_image_to_ppm(const char * fname, const GLubyte * image, const unsigned int &w, const unsigned int &h)
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
FileUtil::save_rgb_image_to_ppm(const char * fname, const GLubyte * image, const unsigned int &w, const unsigned int &h)
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
FileUtil::save_grey_image_to_pgm(const char * fname, const GLubyte * image, const unsigned int &w, const unsigned int &h)
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
FileUtil::save_grey_image_to_pgm(const char * fname, const float * image, const unsigned int &w, const unsigned int &h)
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
