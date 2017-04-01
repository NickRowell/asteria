#include "parameterdir.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>
#include <sstream>
#include <QDebug>

ParameterDir::ParameterDir(const string key, const string title, const string units, string * const data, const bool &requireWritePermission, const bool &requireExists, const bool &createIfNotExists)
    : ConfigParameter(key, title, units), data(data), requireWritePermission(requireWritePermission), requireExists(requireExists), createIfNotExists(createIfNotExists)
{

}

void ParameterDir::validate(const string path) {

//    qInfo() << "validate " << title.c_str();

    value = path;

    // First, check if directory exists
    struct stat info;

    if( stat( path.c_str(), &info ) != 0 ) {
        // Directory does not exist
        if(requireExists) {
            std::ostringstream strs;
            strs << "Directory \"" << path << "\" does not exist!";
            message = strs.str();
            isValid = INVALID;
            return;
        }
        else {
            // Directory does not exist and we'll attempt to create it

            // Extract the path of the parent directory
            string parent = path.substr(0, path.find_last_of("/"));
            struct stat parentinfo;
            // Check if parent exists and we can write to it
            if(stat( parent.c_str(), &parentinfo ) != 0 ) {
                // Parent directory doesn't exist! Can't do anything
                std::ostringstream strs;
                strs << "Cannot create directory \"" << path << "\" in non-existent parent directory \"" << parent << "\"";
                message = strs.str();
                isValid = INVALID;
                return;
            }
            else if( parentinfo.st_mode & S_IFDIR )  {
                // Parent exists and is a directory. Check if we can create the child, and create it if so.
                if (access(parent.c_str(), R_OK | W_OK) == 0) {
                    // Create the folder!
                    // https://techoverflow.net/2013/04/05/how-to-use-mkdir-from-sysstat-h/
                    // User gets read, write & execute permission; group members and other get nothing.
                    int status = mkdir(path.c_str(), S_IRWXU);
                    if(status == -1) {
                        std::ostringstream strs;
                        strs << "Could not create directory " << path;
                        message = strs.str();
                        isValid = INVALID;
                        return;
                    }
                }
                else {
                    // Don't have write access on parent folder
                    std::ostringstream strs;
                    strs << "Cannot create directory \"" << path << "\" in write protected parent directory \"" << parent << "\"";
                    message = strs.str();
                    isValid = INVALID;
                    return;
                }
            }
            else {
                // Parent exists and is not a directory
                std::ostringstream strs;
                strs << "Pathname \"" << path << "\" is invalid";
                message = strs.str();
                isValid = INVALID;
                return;
            }
        }
    }
    else if( info.st_mode & S_IFDIR )  {
        // Exists and is a directory. Check that we have read/write access.
        if (access(path.c_str(), R_OK | W_OK) != 0) {
            // Don't have write access on parent folder
            std::ostringstream strs;
            strs << "No read/write access to \"" << path << "\"";
            message = strs.str();
            isValid = INVALID;
            return;
        }
    }
    else {
        // Exists and is not a directory
        std::ostringstream strs;
        strs << "Pathname \"" << path << "\" is not a directory!";
        message = strs.str();
        isValid = INVALID;
        return;
    }

    // If we've reached this point then the directory does exist and we have the necessary
    // permissions, even if we had to create it ourselves.
    message = "";
    (*data) = path;
    isValid = VALID;
    return;
}
