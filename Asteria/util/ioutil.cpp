#include "util/ioutil.h"

#include <sstream>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fstream>
#include <sys/stat.h>
#include <iostream>

/**
 * @brief Tokenise the string
 * @param s The string
 * @param delim The token delimiter
 * @return Vector of string tokens
 */
std::vector<std::string> IoUtil::split(const std::string &s, const char &delim) {

    std::vector<std::string> tokens;
    std::stringstream   mySstream(s);
    std::string         temp;

    while( std::getline( mySstream, temp, delim ) ) {
        tokens.push_back( temp );
    }

    return tokens;
}

/**
 * Converts an int to a string
 * \param i Integer to convert to a string
 * \return	String containing the converted integer
 */
std::string IoUtil::intToString(int i){
    std::ostringstream oss;
    oss << i;
    std::string result = oss.str();
    return result;
}

/**
 * Wraps up the ioctl function (which manipulates the underlying device
 * parameters of special files). This wrapper calls the function until it
 * returns successfully, in case of interrupted system calls.
 *
 * \param fd Open file descriptor pointing to the device.
 * \param request The request to execute on the underlying device identified by the file descriptor.
 * \param arg Untyped pointer to memory, where the results of the request are written.
 * \return Error flag: 0 for success, -1 indicates error.
 */
int IoUtil::xioctl (int fd, int request, void *arg) {
	int r;

	do {
		r = ioctl(fd, request, arg);
	}
	while (-1 == r && EINTR == errno);

	return r;
}
