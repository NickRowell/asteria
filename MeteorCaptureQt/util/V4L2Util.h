/*
 * V4L2Util.h
 *
 *  Created on: 26 Dec 2016
 *      Author: nrowell
 */

#ifndef V4L2UTIL_H_
#define V4L2UTIL_H_


#include <time.h>
#include <math.h>

#include <vector>     // provides vector
#include <string>     // provides string
#include <utility>    // provides pair

// See http://stackoverflow.com/questions/14003466/how-can-i-read-and-write-from-files-using-the-headers-fcntl-h-and-unistd-h
// See https://linux.die.net/man/3/open
#include <fcntl.h>    // Provides open(...)
#include <unistd.h>   // Provides access(...)

#include <cstring>    // strncopy; memset

#include <cstdlib>

#include <stdio.h>    // Provides perror(...)
#include <errno.h>    // provides errno
#include <iostream>
#include <sstream>
#include <list>
#include <sys/ioctl.h>
#include <sys/stat.h>

#include "IOUtil.h"

#include <linux/videodev2.h>

// Mean I can use vector rather tha std::vector etc.
using namespace std;

class V4L2Util {

public:

	V4L2Util();

	virtual ~V4L2Util();

	vector< pair< int, string > > getCamerasList();

	bool getInfos(int &);

	bool createDevice(int id, int &);

	bool setExposureTime(int & fd, double val);

	double getExposureTime(int & fd);

	void getExposureBounds(int & fd, double &eMin, double &eMax);

	void whatTypesOfStreamingDoesDeviceSupport(int & fd);

	void whatPixelFormatsAreAvailable(int & fd);

	static void printUserControls(int & fd);

	static long getEpochTimeShift();

private:


};

#endif /* V4L2UTIL_H_ */
