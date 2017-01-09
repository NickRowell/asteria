/**
 * Main application entry point.
 */


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


	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	//                                                       //
	//        Set the image format for the camera            //
	//                                                       //
	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++//

	struct v4l2_format format;
	format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
//	format.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
	format.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
	format.fmt.pix.width = 640;
	format.fmt.pix.height = 480;

	if(ioctl(fd, VIDIOC_S_FMT, &format) < 0) {
		perror("VIDIOC_S_FMT");
		close(fd);
		exit(1);
	}

	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	//                                                       //
	//     Inform device about buffers & streaming mode      //
	//                                                       //
	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++//

	// Inform device about buffers to use
	struct v4l2_requestbuffers bufrequest;
	bufrequest.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	bufrequest.memory = V4L2_MEMORY_MMAP;
	bufrequest.count = 32;

//	fprintf(stdout, "Requested buffers: %d\n", bufrequest.count);

	if(ioctl(fd, VIDIOC_REQBUFS, &bufrequest) < 0){
	    perror("VIDIOC_REQBUFS");
		close(fd);
	    exit(1);
	}

//	fprintf(stdout, "Provided buffers: %d\n", bufrequest.count);

	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	//                                                       //
	//  Determine memory requirements and allocate buffers   //
	//                                                       //
	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++//

	// Here, the device informs us how much memory is required for the buffers
	// given the image format, frame dimensions and number of buffers.
	struct v4l2_buffer bufferinfo;
	memset(&bufferinfo, 0, sizeof(bufferinfo));

	bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	bufferinfo.memory = V4L2_MEMORY_MMAP;

//	bufferinfo.flags = bufferinfo.flags | V4L2_BUF_FLAG_TIMECODE;

	void * buffer_start[bufrequest.count];

	for(unsigned int b = 0; b < bufrequest.count; b++) {
		bufferinfo.index = b;

		if(ioctl(fd, VIDIOC_QUERYBUF, &bufferinfo) < 0){
			perror("VIDIOC_QUERYBUF");
			exit(1);
		}

		fprintf(stdout, "bufferinfo.length = %d\nbufferinfo.m.offset = %d\n", bufferinfo.length, bufferinfo.m.offset);

		// bufferinfo.length: number of bytes of memory required for the buffer
		// bufferinfo.m.offset: offset from the start of the device memory for this buffer
		buffer_start[b] = mmap(NULL, bufferinfo.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, bufferinfo.m.offset);

		if(buffer_start[b] == MAP_FAILED){
		    perror("mmap");
		    exit(1);
		}

		memset(buffer_start[b], 0, bufferinfo.length);
	}




	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	//                                                       //
	//                 Activate streaming                    //
	//                                                       //
	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++//

	int type = bufferinfo.type;
	if(ioctl(fd, VIDIOC_STREAMON, &type) < 0){
	    perror("VIDIOC_STREAMON");
	    exit(1);
	}




	/* Here is where you typically start two loops:
	 * - One which runs for as long as you want to
	 *   capture frames (shoot the video).
	 * - One which iterates over your buffers everytime. */


	// Write frames into circular buffer in device memory
	for(unsigned long i = 0; i < 320; i++) {

		// Index into circular buffer
		unsigned int j = i % bufrequest.count;

		cout << "buffer index = " << j << endl;

		bufferinfo.index = j;

		// Put the buffer in the incoming queue.
		if(ioctl(fd, VIDIOC_QBUF, &bufferinfo) < 0){
			perror("VIDIOC_QBUF");
			exit(1);
		}

		// The buffer's waiting in the outgoing queue.
		bufferinfo.flags = V4L2_BUF_FLAG_TIMECODE;
		if(ioctl(fd, VIDIOC_DQBUF, &bufferinfo) < 0){
			perror("VIDIOC_QBUF");
			exit(1);
		}

		// The image is ready to be read; it is stored in the buffer with index j,
		// which is mapped into application address space at buffer_start[j]

		// Retrieve the timestamp and frame index for this image
		bufferinfo.timestamp;  // A struct timeval
		bufferinfo.timecode;   // A struct v4l2_timecode
		bufferinfo.sequence;   // __u32 recording the sequence count of this frame

		time_t timer;
		tm * gmt = gmtime (&timer);

		cout << "Sequence  = " << bufferinfo.sequence << endl;
		cout << "Timestamp = " << bufferinfo.timestamp.tv_sec << " [s] " << bufferinfo.timestamp.tv_usec << " [usec]" << endl;
//		cout << "Timecode: " << endl;
//		cout << "  frames  = " << bufferinfo.timecode.frames << endl;
//		cout << "  hours   = " << bufferinfo.timecode.hours << endl;
//		cout << "  minutes = " << bufferinfo.timecode.minutes << endl;
//		cout << "  seconds = " << bufferinfo.timecode.seconds << endl;

		// Microseconds of the current time of day
		long temp_ms = 1000000 * bufferinfo.timestamp.tv_sec + (long) round(  bufferinfo.timestamp.tv_usec);
		// Microseconds after 1970-01-01T00:00:00Z
		long epochTimeStamp_us = temp_ms +  epochTimeDiffUs;

		long US_PER_DAY = 24l * 60l * 60l * 1000000l;
		long days = epochTimeStamp_us / US_PER_DAY;
		long years = days / 365;
		long daysPastTheYear = days % years;

		printf( "The frame's timestamp in epoch ms is: %ld \n", epochTimeStamp_us);
		printf( "Years/Days =  %ld / %ld \n", years, daysPastTheYear);

//		// Write the image data out to a JPEG file
//		int jpgfile;
//		char filename [100];
//		sprintf(filename, "/home/nrowell/Temp/myimage_%lu_%u.jpeg", i, j);
//		if((jpgfile = open(filename, O_WRONLY | O_CREAT, 0660)) < 0){
//		    perror("open");
//		    exit(1);
//		}
//		write(jpgfile, buffer_start[j], bufferinfo.length);
//		close(jpgfile);
	}

	// Image is now written to the buffer and ready to retrieve




	// Deactivate streaming
	if(ioctl(fd, VIDIOC_STREAMOFF, &type) < 0){
	    perror("VIDIOC_STREAMOFF");
	    exit(1);
	}
	close(fd);

}
