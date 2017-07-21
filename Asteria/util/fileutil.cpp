#include "fileutil.h"

#include <fstream>      // std::ofstream
#include <iostream>     // std::cin, std::cout
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

// TODO: this is only used for strcmp; try using cstring instead BUT be very careful that
// it's the same function because there's a risk of recursively deleting everything in the
// filesystem if it doesn't work in the same way!
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
        fprintf(stderr, "Not deleting character device %s\n", path.c_str());
        return false;
    }

    if(S_ISBLK(info.st_mode)) {
        // Block device - don't touch it
        fprintf(stderr, "Not deleting block device %s\n", path.c_str());
        return false;
    }

    if(S_ISFIFO(info.st_mode)) {
        // FIFO (named pipe) - don't touch it
        fprintf(stderr, "Not deleting FIFO %s\n", path.c_str());
        return false;
    }

    if(S_ISLNK(info.st_mode)) {
        // Symbolic link - don't touch it
        fprintf(stderr, "Not deleting link %s\n", path.c_str());
        return false;
    }

    if(S_ISSOCK(info.st_mode)) {
        // Socket - don't touch it
        fprintf(stderr, "Not deleting socket %s\n", path.c_str());
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

bool FileUtil::createDir(std::string parent, std::string child) {

    std::string newDirPath = parent + "/" + child;
    struct stat info;
    if( stat( newDirPath.c_str(), &info ) != 0 ) {
        // Path does not exist; create it.
        if(mkdir(newDirPath.c_str(), S_IRWXU) == -1) {
            return false;
        }
        else {
            // Successfully created directory
            return true;
        }
    }
    else if( info.st_mode & S_IFDIR )  {
        // Exists and is a directory; take no action
        return true;
    }
    else if( S_ISREG(info.st_mode)) {
        // Exists and is a regular file.
        return false;
    }
    else {
        // Exists and is neither a directory or regular file
        return false;
    }
}

bool FileUtil::createDirs(std::string topLevel, std::vector<std::string> subLevels) {
    // Create each subdirectory in turn
    std::string path = topLevel;
    for(unsigned int d = 0; d<subLevels.size(); d++) {
        if(!FileUtil::createDir(path, subLevels[d])) {
            return false;
        }
        path = path + "/" + subLevels[d];
    }
    return true;
}
