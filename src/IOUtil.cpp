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
 * Opens a file descriptor.
 * \param fh File handle
 * \param request Request
 * \param arg Arg
 * \return Status flag: -1 indicates error
 */
int IOUtil::xioctl (int fh, int request, void *arg) {
	int r;

	do {
		r = ioctl(fh, request, arg);
	}
	while (-1 == r && EINTR == errno);

	return r;
}
