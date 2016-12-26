/*
 * V4L2Util.cpp
 *
 *  Created on: 26 Dec 2016
 *      Author: nrowell
 */

#include "V4L2Util.h"

V4L2Util::V4L2Util() {
	// Constructor

}

V4L2Util::~V4L2Util() {
	// Destructor
}

/**
 * Queries all video devices available under /dev/videoX and returns a vector
 * containing a pair representing each source. The pair contains the device
 * number (i.e. 1 for /dev/video1) and a string containing the name of the video
 * card.
 *
 * \return A vector containing a pair representing each video source. The pair
 * contains the device number (i.e. 1 for /dev/video1) and a string containing
 * the name of the video card.
 */
vector< pair<int,string> > V4L2Util::getCamerasList() {

    vector< pair<int,string> > camerasList;

    bool loop = true;
    bool res = true;
    int deviceNumber = 0;

    IOUtil ioutil = IOUtil();

    do {

        string devicePathStr = "/dev/video" + ioutil.intToString(deviceNumber);

        // http://stackoverflow.com/questions/230062/whats-the-best-way-to-check-if-a-file-exists-in-c-cross-platform

        // access(...) from unistd.h
        if(access(devicePathStr.c_str(), F_OK) != -1 ) {

            // file exists

            // http://stackoverflow.com/questions/4290834/how-to-get-a-list-of-video-capture-devices-web-cameras-on-linux-ubuntu-c

            int fd;

            // open(...) from fcntl.h
            if((fd = open(devicePathStr.c_str(), O_RDONLY)) == -1) {
            	// perror(...) from stdio.h
                perror("Can't open device");
                res = false;
            }
            else {

                struct v4l2_capability caps = {};

                if (-1 == ioutil.xioctl(fd, VIDIOC_QUERYCAP, &caps)) {
                    cout << "Fail Querying Capabilities." << endl;
                    perror("Querying Capabilities");
                    res = false;
                }
                else {

                    pair<int,string> c;
                    c.first = deviceNumber;
                    string s( reinterpret_cast< char const* >(caps.card) );
                    c.second = "NAME[" + s + "] SDK[V4L2]";

                    camerasList.push_back(c);
                }
            }

            close(fd);

            deviceNumber++;

        }
        else {
            loop = false;
        }

    }
    while(loop);

    return camerasList;
}

