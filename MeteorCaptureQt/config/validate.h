#ifndef VALIDATE_H
#define VALIDATE_H

#include "infra/meteorcapturestate.h"

#include <linux/videodev2.h>
#include <sys/ioctl.h>          // IOCTL etc
#include <sstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>

/**
 * Base class for general purpose parameter value checker and validation utility classes.
 */
class ParameterValidator {

public:

    ParameterValidator() {

    }

    /**
     * @brief validates the value of the parameter against criteria defined in the subclass.
     *
     * @param pvalue
     *  A void pointer to the value to be validated
     * @param strs
     *  Error message explaining why validation failed
     * @return
     *  Boolean indicating whether the value passed validation
     */
    virtual bool validate(const void *pvalue, std::ostringstream &strs) const =0;
};

/**
 * Class template that provides implementations of ParameterValidator that verify
 * that a parameter is within specified limits.
 */
template < typename T >
class ValidateWithinLimits : public ParameterValidator {

public:
    ValidateWithinLimits(const T &lower, const T &upper) : lower(lower), upper(upper) {

    }

    T lower;
    T upper;

    bool validate(const void *pvalue, std::ostringstream &strs) const {

        const T * value = static_cast<const T *>(pvalue);

        if(*value <= lower || *value >= upper) {
            strs << "Value (" << *value << ") lies outside allowed range [" << lower << ":" << upper << "]";
            return false;
        }
        return true;
    }
};

class ValidateDirectory : public ParameterValidator {

public:
    ValidateDirectory(const bool &requireWritePermission, const bool &requireExists, const bool &createIfNotExists)
        : requireWritePermission(requireWritePermission), requireExists(requireExists), createIfNotExists(createIfNotExists) {

    }

    bool requireWritePermission;

    bool requireExists;

    bool createIfNotExists;

    bool validate(const void *pvalue, std::ostringstream &strs) const {

        const std::string * tmp = static_cast<const std::string *>(pvalue);

        // Copy path to variable we can modify
        std::string path(*tmp);

        // First, check if directory exists
        struct stat info;

        if( stat( path.c_str(), &info ) != 0 ) {
            // Directory does not exist
            if(requireExists) {
                strs << "Directory \"" << path << "\" does not exist!";
                return false;
            }
            else if (createIfNotExists) {
                // Directory does not exist and we'll attempt to create it

                // Extract the path of the parent directory:
                // 1) Remove any trailing slash from the pathname
                while(path.rbegin() != path.rend() && *path.rbegin() == '/') {
                    path.pop_back();
                }
                // 2) Remove the last path element to leave the parent directory path
                std::string parent = path.substr(0, path.find_last_of("/"));

                struct stat parentinfo;
                // Check if parent exists and we can write to it
                if(stat( parent.c_str(), &parentinfo ) != 0 ) {
                    // Parent directory doesn't exist! Can't do anything
                    strs << "Cannot create directory \"" << path << "\" in non-existent parent directory \"" << parent << "\"";
                    return false;
                }
                else if( parentinfo.st_mode & S_IFDIR )  {
                    // Parent exists and is a directory. Check if we can create the child, and create it if so.
                    if (access(parent.c_str(), R_OK | W_OK) == 0) {
                        // Create the folder!
                        // https://techoverflow.net/2013/04/05/how-to-use-mkdir-from-sysstat-h/
                        // User gets read, write & execute permission; group members and other get nothing.
                        int status = mkdir(path.c_str(), S_IRWXU);
                        if(status == -1) {
                            strs << "Could not create directory " << path;
                            return false;
                        }
                    }
                    else {
                        // Don't have write access on parent folder
                        strs << "Cannot create directory \"" << path << "\" in write protected parent directory \"" << parent << "\"";
                        return false;
                    }
                }
                else {
                    // Parent exists and is not a directory
                    strs << "Pathname \"" << path << "\" is invalid";
                    return false;
                }
            }
            else {
                // Directory doesn't exist, and we're not going to try to create it. Leave it to the user
                // to create the directory if desired.
                strs << "Directory \"" << path << "\" doesn't exist, and we're not going to try and create it programmatically";
                return false;
            }
        }
        else if( info.st_mode & S_IFDIR )  {
            // Exists and is a directory. Insist that we always have read access.
            if (access(path.c_str(), R_OK) != 0) {
                // Don't have read access on parent folder
                strs << "No read access to \"" << path << "\"";
                return false;
            }

            // Additionally require write access if that's specified
            if (access(path.c_str(), W_OK) != 0 && requireWritePermission) {
                // Don't have write access on parent folder
                strs << "No write access to \"" << path << "\"";
                return false;
            }
        }
        else {
            // Exists and is not a directory
            strs << "Pathname \"" << path << "\" is not a directory!";
            return false;
        }

        return true;
    }
};

// Implementation of the ParameterValidator used to validate the image size
class ValidateImageSize : public ParameterValidator {

public:
    ValidateImageSize(MeteorCaptureState * state) : state(state) {

    }

    // State object that contains the connection to the camera
    MeteorCaptureState * state;

    bool validate(const void *pvalue, std::ostringstream &strs) const {

        const unsigned int * value = static_cast<const unsigned int *>(pvalue);

        unsigned int width  = value[0];
        unsigned int height = value[1];

        // Check that the chosen camera & pixel format etc can support the requested image size
        state->format->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        state->format->fmt.pix.pixelformat = state->selectedFormat;
        state->format->fmt.pix.width = width;
        state->format->fmt.pix.height = height;

        if(ioctl(*(this->state->fd), VIDIOC_S_FMT, state->format) < 0) {
            perror("VIDIOC_S_FMT");
            ::close(*(this->state->fd));
            exit(1);
        }

        if(state->format->fmt.pix.width != width || state->format->fmt.pix.height != height ) {
            // Driver couldn't support requested image width
            strs << "Closest support image width and height is \'" << state->format->fmt.pix.width << " " << state->format->fmt.pix.height << "\'";
            return false;
        }

        // Image size is good - set the fields in the state object
        state->width = width;
        state->height = height;

        return true;
    }
};

#endif // VALIDATE_H
