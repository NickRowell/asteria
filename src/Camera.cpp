/*
 * Camera.cpp
 *
 *  Created on: 16 Dec 2016
 *      Author: nrowell
 */

#include "Camera.h"

Camera::Camera() {
	// TODO Auto-generated constructor stub

}

Camera::~Camera() {
	// TODO Auto-generated destructor stub
}

vector< pair<int,string> > Camera::getCamerasList() {

    vector< pair<int,string> > camerasList;

    bool loop = true;
    bool res = true;
    int deviceNumber = 0;

    do {

        string devicePathStr = "/dev/video" + intToString(deviceNumber);

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

                if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &caps)) {
                    cout << "Fail Querying Capabilities." << endl;
                    perror("Querying Capabilities");
                    res = false;
                }
                else {

                    pair<int,string> c;
                    c.first = deviceNumber;
                    std::string s( reinterpret_cast< char const* >(caps.card) );
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
