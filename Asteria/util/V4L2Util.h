#ifndef V4L2UTIL_H_
#define V4L2UTIL_H_

#include "IOUtil.h"

#include <vector>     // provides vector
#include <string>     // provides string
#include <utility>    // provides pair
#include <algorithm>            // std::find(...)

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

#include <linux/videodev2.h>

#include <QString>
#include <QDebug>

// Mean I can use vector rather tha std::vector etc.
using namespace std;

class V4L2Util {

public:

	V4L2Util();

	virtual ~V4L2Util();

    static vector< pair< string, string > > getAllV4LCameras();

    static vector< pair< string, string > > getSupportedV4LCameras(const unsigned int * supportedFmts, const unsigned int supportedFmtsN);

    static __u32 getPreferredPixelFormat(int & fd, const unsigned int * supportedFmts, const unsigned int supportedFmtsN);

    static string getFourCC(__u32 format);




	bool getInfos(int &);

	bool createDevice(int id, int &);

	bool setExposureTime(int & fd, double val);

	double getExposureTime(int & fd);

	void getExposureBounds(int & fd, double &eMin, double &eMax);

	void whatTypesOfStreamingDoesDeviceSupport(int & fd);

	static void printUserControls(int & fd);

private:


};

#endif /* V4L2UTIL_H_ */
