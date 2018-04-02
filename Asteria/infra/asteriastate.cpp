#include "infra/asteriastate.h"
#include "infra/calibrationinventory.h"

// Define global state variables

// Pixel formats supported by the software, in order of preference
// V4L2_PIX_FMT_GREY - Watec camera
// V4L2_PIX_FMT_MJPEG - Many (all?) webcams
const unsigned int AsteriaState::preferredFormats[3] = {V4L2_PIX_FMT_GREY, V4L2_PIX_FMT_YUYV, V4L2_PIX_FMT_MJPEG};
const unsigned int AsteriaState::preferredFormatsN = 3;

AsteriaState::AsteriaState() {
}

AsteriaState::~AsteriaState() {
}
