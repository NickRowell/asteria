#include "optics/cameramodelbase.h"

CameraModelBase::CameraModelBase() : width(0), height(0) {

}

CameraModelBase::CameraModelBase(const unsigned int &width, const unsigned int &height) : width(width), height(height) {

}

CameraModelBase::~CameraModelBase() {

}
