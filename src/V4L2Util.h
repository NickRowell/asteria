/*
 * V4L2Util.h
 *
 *  Created on: 26 Dec 2016
 *      Author: nrowell
 */

#ifndef V4L2UTIL_H_
#define V4L2UTIL_H_


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

#include "IOUtil.h"

#include <linux/videodev2.h>

// Mean I can use vector rather tha std::vector etc.
using namespace std;

class V4L2Util {

public:

	V4L2Util();

	virtual ~V4L2Util();

	vector< pair< int, string > > getCamerasList();

private:


};

#endif /* V4L2UTIL_H_ */
