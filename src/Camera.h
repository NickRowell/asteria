/*
 * Camera.h
 *
 *  Created on: 16 Dec 2016
 *      Author: nrowell
 */

#ifndef CAMERA_H_
#define CAMERA_H_

#include <vector>     // provides vector
#include <string>     // provides string
#include <utility>    // provides pair
#include <fcntl.h>    // Provides open(...)
#include <unistd.h>   // Provides access(...)
#include <stdio.h>    // Provides perror(...)
#include <errno.h>    // provides errno
#include <iostream>
#include <sstream>
#include <list>
#include <sys/ioctl.h>

#include <linux/videodev2.h>

using namespace std;

class Camera {
public:
	Camera();
	virtual ~Camera();

	vector< pair< int, string > > getCamerasList();



	string intToString(int nb){

	    ostringstream oss;
	    oss << nb;
	    string result = oss.str();
	    return result;

	}

	/**
	 * Opens a file descriptor.
	 * \param fh File handle
	 * \param request Request
	 * \param arg Arg
	 */
	int xioctl (int fh, int request, void *arg) {
		int r;

		do
		{
			r = ioctl(fh, request, arg);
		} while (-1 == r && EINTR == errno);

		return r;
	}


};

#endif /* CAMERA_H_ */
