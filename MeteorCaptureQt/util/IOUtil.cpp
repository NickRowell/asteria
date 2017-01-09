/*
 * IOUtil.cpp
 *
 *  Created on: 26 Dec 2016
 *      Author: nrowell
 */

#include "IOUtil.h"

IOUtil::IOUtil() {
	// TODO Auto-generated constructor stub

}

IOUtil::~IOUtil() {
	// TODO Auto-generated destructor stub
}

/**
 * Converts an int to a string
 * \param i Integer to convert to a string
 * \return	String containing the converted integer
 */
string IOUtil::intToString(int i){
    ostringstream oss;
    oss << i;
    string result = oss.str();
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
int IOUtil::xioctl (int fd, int request, void *arg) {
	int r;

	do {
		r = ioctl(fd, request, arg);
	}
	while (-1 == r && EINTR == errno);

	return r;
}
