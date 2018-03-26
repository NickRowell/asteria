#include "optics/cameramodelbase.h"

#include "optics/pinholecamera.h"
#include "optics/pinholecamerawithradialdistortion.h"
#include "optics/pinholecamerawithsipdistortion.h"

const std::vector<CameraModelBase::CameraModelType> CameraModelBase::cameraModelTypes = {PINHOLECAMERA, PINHOLECAMERAWITHRADIALDISTORTION, PINHOLECAMERAWITHSIPDISTORTION};

CameraModelBase::CameraModelBase() : width(0), height(0) {

}

CameraModelBase::CameraModelBase(const unsigned int &width, const unsigned int &height) : width(width), height(height) {

}

CameraModelBase::~CameraModelBase() {

}

CameraModelBase * CameraModelBase::getCameraModelFromEnum(const CameraModelType &type) {
    switch(type) {
    case PINHOLECAMERA: return new PinholeCamera();
    case PINHOLECAMERAWITHRADIALDISTORTION: return new PinholeCameraWithRadialDistortion();
    case PINHOLECAMERAWITHSIPDISTORTION: return new PinholeCameraWithSipDistortion();
    }
}
