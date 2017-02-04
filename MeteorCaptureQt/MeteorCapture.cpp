#include <sys/mman.h>

#include "util/V4L2Util.h"

using namespace std;

int main2() {

//	V4L2Util v4l2util;
//	vector< pair<int,string> > cams = v4l2util.getCamerasList();
//	for (unsigned i=0; i < cams.size(); i++) {
//	    cout << "Cam " << cams[i].first << ": " << cams[i].second << endl;
//	}


	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	//                                                       //
	//      Open a file descriptor on the camera device      //
	//                                                       //
	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++//

	string devicePathStr = "/dev/video0";

	// Open the device
	int fd = open(devicePathStr.c_str(), O_RDWR);
	if(fd == -1) {
		perror("Can't open device");
		close(fd);
		exit(1);
	}

	V4L2Util::printUserControls(fd);

	// Get the time difference between time of day and the frame timestamp. This needs
	// to be recomputed whenever the computer hibernates.
	long epochTimeDiffUs = V4L2Util::getEpochTimeShift();

	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	//                                                       //
	//           Query the device capabilities               //
	//                                                       //
	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++//

	struct v4l2_capability cap;
	if(ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0){
	    perror("VIDIOC_QUERYCAP");
		close(fd);
	    exit(1);
	}

	if(!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)){
	    fprintf(stderr, "The device does not handle single-planar video capture.\n");
		close(fd);
	    exit(1);
	}

	if(!(cap.capabilities & V4L2_CAP_STREAMING)){
	    fprintf(stderr, "The device does not handle streaming.\n");
		close(fd);
	    exit(1);
	}




	// Image is now written to the buffer and ready to retrieve




	close(fd);

}
