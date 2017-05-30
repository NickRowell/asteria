#include "infra/asteriastate.h"

// Define global state variables

// Pixel formats supported by the software, in order of preference
// V4L2_PIX_FMT_GREY - Watec camera
// V4L2_PIX_FMT_MJPEG - Many (all?) webcams
const unsigned int AsteriaState::preferredFormats[3] = {V4L2_PIX_FMT_GREY, V4L2_PIX_FMT_YUYV, V4L2_PIX_FMT_MJPEG};
const unsigned int AsteriaState::preferredFormatsN = 3;

AsteriaState::AsteriaState() {
    bufferinfo = new v4l2_buffer();
    memset(bufferinfo, 0, sizeof(*bufferinfo));

    format = new v4l2_format();
    memset(format, 0, sizeof(*format));

    bufrequest = new v4l2_requestbuffers();
    memset(bufrequest, 0, sizeof(*bufrequest));
}

AsteriaState::~AsteriaState() {
    delete bufferinfo;
    delete format;
    delete bufrequest;
}
