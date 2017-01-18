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
 * Get the time difference in microseconds between the clock time (records time since bootup)
 * and the epoch time (i.e. current time of day). This is useful for referencing the recorded time of video frames
 * to UTC. Note that this must be recomputed whenever the computer hibernates.
 *
 * See http://stackoverflow.com/questions/10266451/where-does-v4l2-buffer-timestamp-value-starts-counting
 *
 * \return The time difference in microseconds between the clock time (records time since bootup)
 * and the current time of day.
 */
long V4L2Util::getEpochTimeShift() {

	// Records time of day, to microsecond accuracy
	struct timeval epochtime;

	// Records time since bootup, to nanosecond accuracy
	struct timespec  vsTime;

	gettimeofday(&epochtime, NULL);
	clock_gettime(CLOCK_MONOTONIC, &vsTime);

	long uptime_us = vsTime.tv_sec* 1000000 + (long)  round( vsTime.tv_nsec/ 1000.0);
	long epoch_us =  epochtime.tv_sec * 1000000  + (long) round( epochtime.tv_usec);
	return epoch_us - uptime_us;
}


void V4L2Util::printUserControls(int & fd) {

	struct v4l2_queryctrl queryctrl;
	struct v4l2_querymenu querymenu;

//	static void enumerate_menu(__u32 id)
//	{
//	    printf("  Menu items:\\n");
//
//	    memset(&querymenu, 0, sizeof(querymenu));
//	    querymenu.id = id;
//
//	    for (querymenu.index = queryctrl.minimum;
//	         querymenu.index <= queryctrl.maximum;
//	         querymenu.index++) {
//	        if (0 == ioctl(fd, VIDIOC_QUERYMENU, &querymenu)) {
//	            printf("  %s\\n", querymenu.name);
//	        }
//	    }
//	}

	memset(&queryctrl, 0, sizeof(queryctrl));

	queryctrl.id = V4L2_CTRL_FLAG_NEXT_CTRL;

	while (0 == ioctl(fd, VIDIOC_QUERYCTRL, &queryctrl)) {

	    if (!(queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)) {

	        printf("Control %s\n", queryctrl.name);

	        if (queryctrl.type == V4L2_CTRL_TYPE_MENU) {

	        	printf("  Menu items:\n");

				memset(&querymenu, 0, sizeof(querymenu));
				querymenu.id = queryctrl.id;

				for (querymenu.index = queryctrl.minimum; querymenu.index <= queryctrl.maximum; querymenu.index++) {
					if (0 == ioctl(fd, VIDIOC_QUERYMENU, &querymenu)) {
						printf("  %s\n", querymenu.name);
					}
				}

	        }
	    }

	    queryctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
	}
	if (errno != EINVAL) {
	    perror("VIDIOC_QUERYCTRL");
	    exit(EXIT_FAILURE);
	}
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

        	// File descriptor (will be directed to the camera device)
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

                    getInfos(fd);
                    double emin, emax;

//                    getExposureBounds(fd, emin, emax);

//                    printf ("Exposure time  : %f [%f:%f]\n", getExposureTime(fd), emin, emax);

                    whatTypesOfStreamingDoesDeviceSupport(fd);

                    whatPixelFormatsAreAvailable(fd);

//                    createDevice(deviceNumber, fd);

                    // Set the image format for the camera
                    struct v4l2_format format;
                    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                    format.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
                    format.fmt.pix.width = 640;
                    format.fmt.pix.height = 480;

                    if(ioctl(fd, VIDIOC_S_FMT, &format) < 0) {
                        perror("VIDIOC_S_FMT");
                        exit(1);
                    }

                    cout << "Set image format" << endl;

                    unsigned int buffer_size = format.fmt.pix.sizeimage;

                    // Set up for reading single image from camera
                    struct buffer
                    {
                        void   *start;
                        size_t  length;
                    };

                    struct buffer * buffers = NULL;
                    buffers = (buffer*)(calloc(1, sizeof(*buffers)));

					if (!buffers)
					{
						fprintf(stderr, "Out of memory\n");
						exit(EXIT_FAILURE);
					}

					buffers[0].length = buffer_size;
					buffers[0].start = malloc(buffer_size);

					if (!buffers[0].start)
					{
						fprintf(stderr, "Out of memory\n");
						exit(EXIT_FAILURE);
					}



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


void V4L2Util::whatPixelFormatsAreAvailable(int & fd) {

	struct v4l2_fmtdesc vid_fmtdesc;
	memset(&vid_fmtdesc, 0, sizeof(vid_fmtdesc));
	vid_fmtdesc.index = 0;

	/* Conversion between enumerated type & english */
	char *buf_types[] = {"VIDEO_CAPTURE","VIDEO_OUTPUT", "VIDEO_OVERLAY"};
	char *flags[] = {"uncompressed", "compressed"};
	fprintf(stdout, "\nDiscovering supported video formats:\n");

	/* For each of the supported v4l2_buf_type buffer types */
	for (vid_fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; vid_fmtdesc.type < V4L2_BUF_TYPE_VIDEO_OVERLAY; vid_fmtdesc.type++)
	{
		/* Send the VIDIOC_ENUM_FM ioctl and print the results */
		while( ioctl(fd, VIDIOC_ENUM_FMT, &vid_fmtdesc ) == 0 )
		{
			/* We got a video format/codec back */
			fprintf(stdout,"VIDIOC_ENUM_FMT(%d, %s)\n", vid_fmtdesc.index, buf_types[vid_fmtdesc.type-1]);
			fprintf(stdout, "  index        :%d\n", vid_fmtdesc.index);
			fprintf(stdout, "  type         :%s\n", buf_types[vid_fmtdesc.type-1]);
			fprintf(stdout, "  flags        :%s\n", flags[vid_fmtdesc.flags]);
			fprintf(stdout, "  description  :%s\n", vid_fmtdesc.description);

			/* Convert the pixelformat attributes from FourCC into 'human readable' format */
			fprintf(stdout, "  pixelformat  :%c%c%c%c\n",
							vid_fmtdesc.pixelformat & 0xFF, (vid_fmtdesc.pixelformat >> 8) & 0xFF,
							(vid_fmtdesc.pixelformat >> 16) & 0xFF, (vid_fmtdesc.pixelformat >> 24) & 0xFF);

			/* Increment the index */
			vid_fmtdesc.index++;
		}
	}
}


void V4L2Util::whatTypesOfStreamingDoesDeviceSupport(int & fd) {

	struct v4l2_requestbuffers req;

	memset(&req, 0, sizeof(req));

	req.count = 4;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;

	if (-1 == ioctl(fd, VIDIOC_REQBUFS, &req))
	{
		fprintf(stdout, "Camera does not support memory mapping\n");
	}
	else
	{
		fprintf(stdout, "Camera supports memory mapping\n");
	}

	req.count  = 4;
	req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_USERPTR;

	if (-1 == ioctl(fd, VIDIOC_REQBUFS, &req))
	{
		fprintf(stdout, "Camera does not support user pointer i/o\n");
	}
	else
	{
		fprintf(stdout, "Camera supports user pointer i/o\n");
	}

	return;
}


/**
 * Retrieves and prints information about the camera, image format, video drivers etc.
 * \param fd Open file descriptor pointing to the device.
 * \return Status flag: false indicates error.
 */
bool V4L2Util::getInfos(int & fd) {

    struct v4l2_capability caps = {};

    IOUtil ioutil = IOUtil();

    // http://linuxtv.org/downloads/v4l-dvb-apis/vidioc-querycap.html

    if (-1 == ioutil.xioctl(fd, VIDIOC_QUERYCAP, &caps)) {
        perror("Querying Capabilities");
        return false;
    }

    cout << "Driver name     : " << caps.driver << endl;
    cout << "Device name     : " << caps.card << endl;
    cout << "Device location : " << caps.bus_info << endl;
    printf ("Driver version  : %u.%u.%u\n",(caps.version >> 16) & 0xFF, (caps.version >> 8) & 0xFF, caps.version & 0xFF);
    cout << "Capabilities    : " << endl;
    cout << "V4L2_CAP_VIDEO_CAPTURE			" << !(caps.capabilities & V4L2_CAP_VIDEO_CAPTURE) << endl;
    cout << "V4L2_CAP_VIDEO_OUTPUT			" << !(caps.capabilities & V4L2_CAP_VIDEO_OUTPUT) << endl;
    cout << "V4L2_CAP_VIDEO_OVERLAY			" << !(caps.capabilities & V4L2_CAP_VIDEO_OVERLAY) << endl;
    cout << "V4L2_CAP_VBI_CAPTURE			" << !(caps.capabilities & V4L2_CAP_VBI_CAPTURE) << endl;
    cout << "V4L2_CAP_VBI_OUTPUT			" << !(caps.capabilities & V4L2_CAP_VBI_OUTPUT) << endl;
    cout << "V4L2_CAP_SLICED_VBI_CAPTURE	" << !(caps.capabilities & V4L2_CAP_SLICED_VBI_CAPTURE) << endl;
    cout << "V4L2_CAP_SLICED_VBI_OUTPUT		" << !(caps.capabilities & V4L2_CAP_SLICED_VBI_OUTPUT) << endl;
    cout << "V4L2_CAP_RDS_CAPTURE			" << !(caps.capabilities & V4L2_CAP_RDS_CAPTURE) << endl;
    cout << "V4L2_CAP_VIDEO_OUTPUT_OVERLAY	" << !(caps.capabilities & V4L2_CAP_VIDEO_OUTPUT_OVERLAY) << endl;
    cout << "V4L2_CAP_HW_FREQ_SEEK			" << !(caps.capabilities & V4L2_CAP_HW_FREQ_SEEK) << endl;
    cout << "V4L2_CAP_RDS_OUTPUT			" << !(caps.capabilities & V4L2_CAP_RDS_OUTPUT) << endl;
    cout << "V4L2_CAP_VIDEO_CAPTURE_MPLANE	" << !(caps.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE) << endl;
    cout << "V4L2_CAP_VIDEO_OUTPUT_MPLANE	" << !(caps.capabilities & V4L2_CAP_VIDEO_OUTPUT_MPLANE) << endl;
    cout << "V4L2_CAP_VIDEO_M2M_MPLANE		" << !(caps.capabilities & V4L2_CAP_VIDEO_M2M_MPLANE) << endl;
    cout << "V4L2_CAP_VIDEO_M2M				" << !(caps.capabilities & V4L2_CAP_VIDEO_M2M) << endl;
    cout << "V4L2_CAP_TUNER					" << !(caps.capabilities & V4L2_CAP_TUNER) << endl;
    cout << "V4L2_CAP_AUDIO					" << !(caps.capabilities & V4L2_CAP_AUDIO) << endl;
    cout << "V4L2_CAP_RADIO					" << !(caps.capabilities & V4L2_CAP_RADIO) << endl;
    cout << "V4L2_CAP_MODULATOR				" << !(caps.capabilities & V4L2_CAP_MODULATOR) << endl;
    cout << "V4L2_CAP_READWRITE				" << !(caps.capabilities & V4L2_CAP_READWRITE) << endl;
    cout << "V4L2_CAP_ASYNCIO				" << !(caps.capabilities & V4L2_CAP_ASYNCIO) << endl;
    cout << "V4L2_CAP_STREAMING				" << !(caps.capabilities & V4L2_CAP_STREAMING) << endl;

    struct v4l2_cropcap cropcap;
    memset(&cropcap, 0, sizeof(cropcap));
    cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;


    if (-1 == ioutil.xioctl (fd, VIDIOC_CROPCAP, &cropcap)) {
        perror("Querying Cropping Capabilities");
        return false;
    }

    printf( "Camera Cropping :\n"
            "  Bounds  : %dx%d+%d+%d\n"
            "  Default : %dx%d+%d+%d\n"
            "  Aspect  : %d/%d\n",
            cropcap.bounds.width, cropcap.bounds.height, cropcap.bounds.left, cropcap.bounds.top,
            cropcap.defrect.width, cropcap.defrect.height, cropcap.defrect.left, cropcap.defrect.top,
            cropcap.pixelaspect.numerator, cropcap.pixelaspect.denominator);

    int support_grbg10 = 0;

    struct v4l2_fmtdesc fmtdesc = {0};
    fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    char fourcc[5] = {0};
    char c, e;
    printf( "  FORMAT    : CE Desc\n");

    while (0 == ioutil.xioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc)) {

        strncpy(fourcc, (char *)&fmtdesc.pixelformat, 4);
        if (fmtdesc.pixelformat == V4L2_PIX_FMT_SGRBG10)
            support_grbg10 = 1;
        c = fmtdesc.flags & 1? 'C' : ' ';
        e = fmtdesc.flags & 2? 'E' : ' ';
        printf("  %s : %c%c %s\n", fourcc, c, e, fmtdesc.description);
        fmtdesc.index++;
    }

    /*struct v4l2_format fmt = {0};
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = 640;
    fmt.fmt.pix.height = 480;
    //fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_BGR24;
    //fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_GREY;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
    fmt.fmt.pix.field = V4L2_FIELD_NONE;

    if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt)) {
        perror("Setting Pixel Format");
        return false;
    }

    strncpy(fourcc, (char *)&fmt.fmt.pix.pixelformat, 4);
    printf( "Selected mode   :\n"
            "  Width  : %d\n"
            "  Height : %d\n"
            "  PixFmt : %s\n"
            "  Field  : %d\n",
            fmt.fmt.pix.width,
            fmt.fmt.pix.height,
            fourcc,
            fmt.fmt.pix.field);*/

//    double eMin, eMax; int gMin, gMax;
//    getExposureBounds(eMin, eMax);
//    cout << "Min exposure    : " << eMin << endl;
//    cout << "Max exposure    : " << eMax << endl;

//    getGainBounds(gMin, gMax);
//    cout << "Min gain        : " << gMin << endl;
//    cout << "Max gain        : " << gMax << endl;

    return true;
};



void V4L2Util::getExposureBounds(int & fd, double &eMin, double &eMax) {

    IOUtil ioutil = IOUtil();

    struct v4l2_queryctrl queryctrl;
    memset(&queryctrl, 0, sizeof(queryctrl));
    queryctrl.id = V4L2_CID_EXPOSURE_ABSOLUTE;

    if (-1 == ioctl(fd, VIDIOC_QUERYCTRL, &queryctrl)) {

        if (errno != EINVAL) {

            perror("VIDIOC_QUERYCTRL");
            exit(EXIT_FAILURE);

        }
        else {

            printf(">> V4L2_CID_EXPOSURE_ABSOLUTE is not supported\n");
            eMin = -1;
            eMax = -1;

        }

    }
    else if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED) {

        printf(">> V4L2_CID_EXPOSURE_ABSOLUTE is not supported\n");
        eMin = -1;
        eMax = -1;

    }
    else {

        /*cout << "Name    : " << queryctrl.name << endl;
        cout << "Min     : " << queryctrl.minimum << endl;
        cout << "Max     : " << queryctrl.maximum << endl;
        cout << "Step    : " << queryctrl.step << endl;
        cout << "Default : " << queryctrl.default_value << endl;
        cout << "Flags   : " << queryctrl.flags << endl;*/

        eMin = queryctrl.minimum;
        eMax = queryctrl.maximum;
    }

}

bool V4L2Util::createDevice(int id, int &fd) {

    IOUtil ioutil = IOUtil();

    string deviceNameStr = "/dev/video" + ioutil.intToString(id);

    const char* mDeviceName = deviceNameStr.c_str();

    struct stat st;

    // Check if device exists
    if (-1 == stat(mDeviceName, &st)) {
        fprintf(stderr, "Cannot identify '%s': %d, %s\n", mDeviceName, errno, strerror(errno));
        return false;
    }

    // Check if device is a 'character device', which is what graphics cards are.
    if (!S_ISCHR(st.st_mode)) {
        fprintf(stderr, "%s is no device\n", mDeviceName);
        return false;
    }

    fd = open(mDeviceName, O_RDWR /* required */ | O_NONBLOCK, 0);

    if (-1 == fd) {
        fprintf(stderr, "Cannot open '%s': %d, %s\n", mDeviceName, errno, strerror(errno));
        return false;
    }

    struct v4l2_format mFormat;

    memset(&mFormat, 0, sizeof(mFormat));
    mFormat.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    // Preserve original settings as set by v4l2-ctl for example
    if (-1 == ioutil.xioctl(fd, VIDIOC_G_FMT, &mFormat)){
    	fprintf(stdout, "Failed to create device\n");
        return false;
    }
    fprintf(stdout, "Created device\n");
    return true;

}

double V4L2Util::getExposureTime(int & fd) {

    struct v4l2_control control;
    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_EXPOSURE_ABSOLUTE;

    if(0 == ioctl(fd, VIDIOC_G_CTRL, &control)) {

        return control.value * 100;

    // Ignore if V4L2_CID_CONTRAST is unsupported
    } else if (errno != EINVAL) {

        perror("VIDIOC_G_CTRL");

    }

    return 0;
}

bool V4L2Util::setExposureTime(int & fd, double val){

//    if(expMax > 0 && expMin > 0 && val >= expMin && val <= expMax) {

        // ************************ DISABLE AUTO EXPOSURE *****************************

    	struct v4l2_queryctrl queryctrl1;
        struct v4l2_control control1;
        memset(&queryctrl1, 0, sizeof(queryctrl1));
        queryctrl1.id = V4L2_CID_EXPOSURE_AUTO;

        if(-1 == ioctl(fd, VIDIOC_QUERYCTRL, &queryctrl1)) {

            if(errno != EINVAL) {

                perror("VIDIOC_QUERYCTRL");
                return false;

            }else {

                printf(">> V4L2_CID_EXPOSURE_AUTO is not supported\n");

            }

        }else if (queryctrl1.flags & V4L2_CTRL_FLAG_DISABLED) {

            printf(">> V4L2_CID_EXPOSURE_AUTO is not supported\n");

        }else {

            memset(&control1, 0, sizeof (control1));
            control1.id = V4L2_CID_EXPOSURE_AUTO;
            control1.value = V4L2_EXPOSURE_MANUAL;

            if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control1)) {
                perror("VIDIOC_S_CTRL");
                return false;
            }

            cout << ">> Manual exposure setted." << endl;

        }

        // ************************ SET AUTO EXPOSURE *****************************

        struct v4l2_queryctrl queryctrl;
        struct v4l2_control control;
        memset(&queryctrl, 0, sizeof(queryctrl));
        queryctrl.id = V4L2_CID_EXPOSURE_ABSOLUTE;

        if(-1 == ioctl(fd, VIDIOC_QUERYCTRL, &queryctrl)) {

            if(errno != EINVAL) {

                perror("VIDIOC_QUERYCTRL");
                return false;

            }else {

                printf(">> V4L2_CID_EXPOSURE_ABSOLUTE is not supported\n");

            }

        }else if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED) {

            printf(">> V4L2_CID_EXPOSURE_ABSOLUTE is not supported\n");

        }else {

            memset(&control, 0, sizeof (control));
            control.id = V4L2_CID_EXPOSURE_ABSOLUTE;

            /*
            V4L2_CID_EXPOSURE_ABSOLUTE integer
            Determines the exposure time of the camera sensor.
            The exposure time is limited by the frame interval.
            Drivers should interpret the values as 100 µs units, w
            here the value 1 stands for 1/10000th of a second, 10000
            for 1 second and 100000 for 10 seconds.
            */

            control.value = val/100;
//            exp = val;
            printf(">> V4L2_CID_EXPOSURE_ABSOLUTE setted to %f (%f with V4L2)\n", val, val/100);

            if (-1 == ioctl(fd, VIDIOC_S_CTRL, &control)) {
                perror("VIDIOC_S_CTRL");
                return false;
            }

        }

        return true;

//    }else {
//
//        if(expMin == -1 && expMax == -1) {
//
//            cout << "Exposure time not supported." << endl;
//            return true;
//
//        }

//        cout << "> Exposure value (" << val << ") is not in range [ " << expMin << " - " << expMax << " ]" << endl;

//    }

//    return false;
}




