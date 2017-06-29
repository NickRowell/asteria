#ifndef V4L2UTIL_H
#define V4L2UTIL_H

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

using namespace std;

class V4L2Util {

public:
	V4L2Util();

    static vector< pair< string, string > > getAllV4LCameras();

    static vector< pair< string, string > > getSupportedV4LCameras(const unsigned int * supportedFmts, const unsigned int supportedFmtsN);

    /**
     * Examine the pixel formats provided by the camera, and get the subset of formats supported by the application, in order of
     * preference. Clients can pick the preferred format from the first element in the vector, or detect if no supported format
     * exists if the vector is empty.
     * @brief getSupportedPixelFormats
     * @param fd
     * @param supportedFmts
     * @param supportedFmtsN
     * @return
     */
    static std::vector< v4l2_fmtdesc > getSupportedPixelFormats(int & fd, const unsigned int * supportedFmts, const unsigned int supportedFmtsN);

    static string getFourCC(__u32 format);

    static void openCamera(string &path, int *&fd, unsigned int &format);

    static string getCameraName(int & fd);

    static string getV4l2FieldNameFromIndex(const unsigned int &field);



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
