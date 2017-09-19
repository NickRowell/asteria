#include "util/v4l2util.h"

#include "util/ioutil.h"

#include "infra/asteriastate.h"

V4L2Util::V4L2Util() {
	// Constructor
}

/**
 * Queries all video devices available under /dev/videoX and returns a vector
 * containing a pair representing each source. The pair contains the device
 * path (e.g. /dev/video1) and a string containing the name of the camera.
 *
 * \return A vector containing a pair representing each video source. The pair
 * contains the device path (e.g. /dev/video1) and a string containing
 * the name of the camera.
 */
vector< pair<string,string> > V4L2Util::getAllV4LCameras() {

    vector< pair<string,string> > camerasList;

    bool loop = true;
    int deviceNumber = 0;

    do {

        string devicePathStr = "/dev/video" + IoUtil::intToString(deviceNumber);

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
            }
            else {

                struct v4l2_capability caps;
                memset(&caps, 0, sizeof(caps));

                if (IoUtil::xioctl(fd, VIDIOC_QUERYCAP, &caps) == -1) {
                    cout << "Fail Querying Capabilities." << endl;
                    perror("Querying Capabilities");
                }
                else {

                    pair<string,string> c;
                    c.first = devicePathStr;
                    string s( reinterpret_cast< char const* >(caps.card) );
                    c.second = s;

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


/**
 *
 */
vector< pair<string,string> > V4L2Util::getSupportedV4LCameras(const unsigned int * supportedFmts, const unsigned int supportedFmtsN) {

    vector< pair<string,string> > allCamerasList = V4L2Util::getAllV4LCameras();

    vector< pair<string,string> > supportedCamerasList;

    for(unsigned int p=0; p<allCamerasList.size(); p++) {

        pair<string,string> camera = allCamerasList[p];

        // Open file desriptor on the camera
        int fd = open(camera.first.c_str(), O_RDWR);

        // Determine if the camera provides at least one of the supported image formats
        if(!getSupportedPixelFormats(fd, supportedFmts, supportedFmtsN).empty()) {

            // The camera does provide one of the supported pixel formats
            supportedCamerasList.push_back(camera);
        }

        // Close file descriptor
        ::close(fd);
    }

    return supportedCamerasList;
}

/**
 * Translate the pixel format integer to four character code
 * @brief V4L2Util::getFourCC
 * @param format
 * @return
 */
string V4L2Util::getFourCC(__u32 format) {
    char buff[10];
    snprintf(buff, sizeof(buff), "%c%c%c%c", format & 0xFF, (format >> 8) & 0xFF, (format >> 16) & 0xFF, (format >> 24) & 0xFF);
    std::string buffAsStdStr = buff;
    return buffAsStdStr;
}

/**
 * Examine the pixel formats provided by the camera, and get the subset of formats supported by the application, in order of
 * preference. Clients can pick the preferred format from the first element in the vector, or detect if no supported format
 * exists if the vector is empty.
 *
 * @brief getSupportedPixelFormats
 * @param fd
 * @param supportedFmts
 * @param supportedFmtsN
 * @return
 */
std::vector< v4l2_fmtdesc > V4L2Util::getSupportedPixelFormats(int & fd, const unsigned int * supportedFmts, const unsigned int supportedFmtsN) {

    // Contains all pixel formats provided by the camera
    std::vector< v4l2_fmtdesc > availableFormats;

    // Contains the subset of formats that are supported by the application, in order of preference
    std::vector< v4l2_fmtdesc > supportedFormats;

    struct v4l2_fmtdesc vid_fmtdesc;
    memset(&vid_fmtdesc, 0, sizeof(vid_fmtdesc));
    vid_fmtdesc.index = 0;
    // Only interested in pixel formats for video capture
    vid_fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    // Get list of all the available pixel formats
    while( ioctl(fd, VIDIOC_ENUM_FMT, &vid_fmtdesc ) == 0 ) {

        // Copy the v4l2_fmtdesc into the vector
        struct v4l2_fmtdesc copy;
        memset(&copy, 0, sizeof(copy));
        std::copy(std::begin(vid_fmtdesc.description), std::end(vid_fmtdesc.description), std::begin(copy.description));
        copy.flags = vid_fmtdesc.flags;
        copy.index = vid_fmtdesc.index;
        copy.pixelformat = vid_fmtdesc.pixelformat;
        std::copy(std::begin(vid_fmtdesc.reserved), std::end(vid_fmtdesc.reserved), std::begin(copy.reserved));
        copy.type = vid_fmtdesc.type;

        availableFormats.push_back(copy);
        // Increment the format descriptor index
        vid_fmtdesc.index++;
    }

    // From the available formats pick the one that is preferred
    for(unsigned int f=0; f < supportedFmtsN; f++) {

        unsigned int preferredFormat = supportedFmts[f];

        // Is the corresponding preferred format provided by the camera?
        for (unsigned int p=0; p < availableFormats.size(); p++) {
            v4l2_fmtdesc format = availableFormats[p];
            if(format.pixelformat == preferredFormat) {
                supportedFormats.push_back(format);
            }
        }
    }

    return supportedFormats;
}

void V4L2Util::openCamera(string &path, int * &fd, unsigned int &format) {

    // Open the camera device and store the file descriptor to the state
    fd = new int(open(path.c_str(), O_RDWR));

    // Get the supported pixel formats, in order of preference
    std::vector<v4l2_fmtdesc> formats = getSupportedPixelFormats(*fd, AsteriaState::preferredFormats, AsteriaState::preferredFormatsN);

    // If there are NO supported formats print an error and exit
    if(formats.empty()) {
        cout << "No supported pixel format provided by camera " << getCameraName(*fd) << flush;
        ::close(*fd);
        exit(1);
    }
    // Otherwise, pick the first entry as the format to use
    v4l2_fmtdesc preferredFormat = formats[0];

    format = preferredFormat.pixelformat;
}


string V4L2Util::getCameraName(int & fd) {

    struct v4l2_capability caps;
    memset(&caps, 0, sizeof(caps));

    if (IoUtil::xioctl(fd, VIDIOC_QUERYCAP, &caps) == -1) {
        perror("Querying Capabilities");
        return "";
    }
    else {
        string s( reinterpret_cast< char const* >(caps.card) );
        return s;
    }
}

string V4L2Util::getV4l2FieldNameFromIndex(const unsigned int &field) {
    switch(field) {
    case V4L2_FIELD_ANY:
        return "V4L2_FIELD_ANY";
    case V4L2_FIELD_NONE:
        return "V4L2_FIELD_NONE";
    case V4L2_FIELD_TOP:
        return "V4L2_FIELD_TOP";
    case V4L2_FIELD_BOTTOM:
        return "V4L2_FIELD_BOTTOM";
    case V4L2_FIELD_SEQ_TB:
        return "V4L2_FIELD_SEQ_TB";
    case V4L2_FIELD_SEQ_BT:
        return "V4L2_FIELD_SEQ_BT";
    case V4L2_FIELD_INTERLACED:
        return "V4L2_FIELD_INTERLACED";
    case V4L2_FIELD_ALTERNATE:
        return "V4L2_FIELD_ALTERNATE";
    case V4L2_FIELD_INTERLACED_TB:
        return "V4L2_FIELD_INTERLACED_TB";
    case V4L2_FIELD_INTERLACED_BT:
        return "V4L2_FIELD_INTERLACED_BT";
    default:
        return "UNDEFINED";
    }
}

void V4L2Util::printUserControls(int & fd) {

    fprintf(stderr, "Configurable controls provided by video driver:\n");

	struct v4l2_queryctrl queryctrl;
	struct v4l2_querymenu querymenu;

	memset(&queryctrl, 0, sizeof(queryctrl));

	queryctrl.id = V4L2_CTRL_FLAG_NEXT_CTRL;

    while (IoUtil::xioctl(fd, VIDIOC_QUERYCTRL, &queryctrl) == 0) {

	    if (!(queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)) {

            fprintf(stderr, " - Control %s\n", queryctrl.name);

	        if (queryctrl.type == V4L2_CTRL_TYPE_MENU) {
                fprintf(stderr, "  -  Menu items:\n");
				memset(&querymenu, 0, sizeof(querymenu));
				querymenu.id = queryctrl.id;

				for (querymenu.index = queryctrl.minimum; querymenu.index <= queryctrl.maximum; querymenu.index++) {
                    if (IoUtil::xioctl(fd, VIDIOC_QUERYMENU, &querymenu) == 0) {
                        fprintf(stderr, "  -  %s\n", querymenu.name);
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

    struct v4l2_capability caps;
    memset(&caps, 0, sizeof(caps));

    // http://linuxtv.org/downloads/v4l-dvb-apis/vidioc-querycap.html

    if (-1 == IoUtil::xioctl(fd, VIDIOC_QUERYCAP, &caps)) {
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


    if (-1 == IoUtil::xioctl (fd, VIDIOC_CROPCAP, &cropcap)) {
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

    struct v4l2_fmtdesc fmtdesc;
    memset(&fmtdesc, 0, sizeof(fmtdesc));

    fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    char fourcc[5] = {0};
    char c, e;
    printf( "  FORMAT    : CE Desc\n");

    while (0 == IoUtil::xioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc)) {

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
}



void V4L2Util::getExposureBounds(int & fd, double &eMin, double &eMax) {

    struct v4l2_queryctrl queryctrl;
    memset(&queryctrl, 0, sizeof(queryctrl));
    queryctrl.id = V4L2_CID_EXPOSURE_ABSOLUTE;

    if (IoUtil::xioctl(fd, VIDIOC_QUERYCTRL, &queryctrl) == -1) {

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


double V4L2Util::getExposureTime(int & fd) {

    struct v4l2_control control;
    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_EXPOSURE_ABSOLUTE;

    if(0 == IoUtil::xioctl(fd, VIDIOC_G_CTRL, &control)) {

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
        memset(&queryctrl1, 0, sizeof(queryctrl1));



        queryctrl1.id = V4L2_CID_EXPOSURE_AUTO;

        if(-1 == IoUtil::xioctl(fd, VIDIOC_QUERYCTRL, &queryctrl1)) {

            if(errno != EINVAL) {

                perror("VIDIOC_QUERYCTRL");
                return false;

            }else {

                printf(">> V4L2_CID_EXPOSURE_AUTO is not supported\n");

            }

        }else if (queryctrl1.flags & V4L2_CTRL_FLAG_DISABLED) {

            printf(">> V4L2_CID_EXPOSURE_AUTO is not supported\n");

        }else {

            struct v4l2_control control1;
            memset(&control1, 0, sizeof(control1));
            control1.id = V4L2_CID_EXPOSURE_AUTO;
            control1.value = V4L2_EXPOSURE_MANUAL;

            if (-1 == IoUtil::xioctl(fd, VIDIOC_S_CTRL, &control1)) {
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

        if(-1 == IoUtil::xioctl(fd, VIDIOC_QUERYCTRL, &queryctrl)) {

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

            if (-1 == IoUtil::xioctl(fd, VIDIOC_S_CTRL, &control)) {
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





