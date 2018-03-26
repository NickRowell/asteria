#include "fileutil.h"
#include "util/timeutil.h"

#include <fstream>      // std::ofstream
#include <iostream>     // std::cin, std::cout
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <regex>

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

bool FileUtil::fileExists(std::string path) {

    // First, check if path exists. NOTE: we MUST use lstat(...) rather than stat(...),
    // otherwise this will follow any symlinks.
    struct stat info;
    if( lstat( path.c_str(), &info ) != 0 ) {
        // File does not exist
        return false;
    }

    if(S_ISREG(info.st_mode)) {
        // Regular file
        return true;
    }

    // Something else
    return false;
}

std::map<long long, std::string> FileUtil::mapVideoDirectory(std::string rootPath) {

    std::map<long long, std::string> map;

    // This regex usage relies on version 4.9 or later of the GCC
    const std::regex yearRegex("[0-9]{4}");
    const std::regex monthDayRegex("[0-9]{2}");

    DIR *rootDir;
    if ((rootDir = opendir (rootPath.c_str())) == NULL) {
        // Couldn't open root directory; return empty map
        return map;
    }

    // Loop over all the files/directories in the root directory
    struct dirent *yyyy;
    while ((yyyy = readdir (rootDir)) != NULL) {

        // Detect directories with names matching the format YYYY
        if(!(yyyy->d_type == DT_DIR && std::regex_match (yyyy->d_name, yearRegex))) {
            // Not a YYYY directory
            continue;
        }

        // Found a directory called e.g. 2018/. Now search the contents.
        std::string yearPath = rootPath + "/" + yyyy->d_name;

        DIR *yearDir;
        if ((yearDir = opendir (yearPath.c_str())) == NULL) {
            // Couldn't open the year directory; skip it
            continue;
        }

        struct dirent *mm;
        while ((mm = readdir (yearDir)) != NULL) {

            // Detect directories with names matching the format MM
            if(!(mm->d_type == DT_DIR && std::regex_match (mm->d_name, monthDayRegex))) {
                // Not a MM directory
                continue;
            }

            // Found a subdirectory called e.g. 03/. Now search the contents.
            std::string monthPath = yearPath + "/" + mm->d_name;

            DIR *monthDir;
            if ((monthDir = opendir (monthPath.c_str())) == NULL) {
                // Couldn't open the month directory; skip it
                continue;
            }

            struct dirent *dd;
            while ((dd = readdir (monthDir)) != NULL) {
                // Detect directories with names matching the format DD
                if(!(dd->d_type == DT_DIR && std::regex_match (dd->d_name, monthDayRegex))) {
                    // Not a DD directory
                    continue;
                }

                // Found a subdirectory called e.g. 29/. Now search the contents.
                std::string dayPath = monthPath + "/" + dd->d_name;

                DIR *dayDir;
                if ((dayDir = opendir (dayPath.c_str())) == NULL) {
                    // Couldn't open the day directory; skip it
                    continue;
                }
                struct dirent *utc;
                while ((utc = readdir (dayDir)) != NULL) {
                    // Detect directories with names matching the UTC format
                    if(!(utc->d_type == DT_DIR && std::regex_match (utc->d_name, TimeUtil::utcRegex))) {
                        // Not a UTC directory
                        continue;
                    }

                    // Found a directory with a name that is a UTC.
                    std::string clipPath = dayPath + "/" + utc->d_name;

                    // Get the epoch time corresponding to this UTC
                    long long epochTime_us = TimeUtil::utcStringToEpoch(utc->d_name);

                    // Insert it into the map.
                    map.insert ( std::pair<long long, std::string>(epochTime_us, clipPath));
                }
                closedir (dayDir);
            }
            closedir (monthDir);
        }
        closedir (yearDir);
    }
    closedir (rootDir);

    return map;
}
