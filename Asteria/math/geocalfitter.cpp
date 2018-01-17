#include "geocalfitter.h"

#include "util/coordinateutil.h"
#include "infra/referencestar.h"
#include "infra/source.h"

GeoCalFitter::GeoCalFitter(CameraModelBase *cam, Eigen::Quaterniond *q_sez_cam, std::vector<std::pair<Source, ReferenceStar> > *xms, const double &gmst, const double &lon, const double &lat) :
     LevenbergMarquardtSolver(cam->getNumParameters() + 4, xms->size()*2), cam(cam), q_sez_cam(q_sez_cam), xms(xms), gmst(gmst), lon(lon), lat(lat) {

    // The data consists of the (i,j) coordinates of the extracted sources
    double data[N];
    double covar[N*N] = {0.0};
    // Index of the Source
    long idx = 0l;
    for(std::pair<Source, ReferenceStar> xm : *xms) {
        Source & source = xm.first;
        data[2*idx+0] = source.i;
        data[2*idx+1] = source.j;

        // Indices of the row-packed covariance matrix elements
        unsigned int ii = (2 * idx) * (N + 1);
        unsigned int ij = ii + 1;
        unsigned int ji = ii + N;
        unsigned int jj = ji + 1;

        covar[ii] = source.c_ii;
        covar[ij] = source.c_ij;
        covar[ji] = source.c_ij;
        covar[jj] = source.c_jj;

        idx++;
    }
    setData(data);
    setCovariance(covar);

    // Set the intial guess parameters
    double initial_params[M];
    initial_params[0] = q_sez_cam->w();
    initial_params[1] = q_sez_cam->x();
    initial_params[2] = q_sez_cam->y();
    initial_params[3] = q_sez_cam->z();
    cam->getParameters(&initial_params[4]);
    this->setParameters(initial_params);
}

void GeoCalFitter::getModel(double *model) {

    // The model consists of the (i,j) coordinates of the reference stars

    // Read out the current quaternion elements from the parameters
    q_sez_cam->w() = params[0];
    q_sez_cam->x() = params[1];
    q_sez_cam->y() = params[2];
    q_sez_cam->z() = params[3];

    // Set the parameters of the camera (advance pointer past the first four elements, which
    // contain the elements of the orientation quaternion)
    cam->setParameters(&params[4]);

    // Rotation matrices
    Matrix3d r_bcrf_ecef = CoordinateUtil::getBcrfToEcefRot(gmst);
    Matrix3d r_ecef_sez  = CoordinateUtil::getEcefToSezRot(lon, lat);
    Matrix3d r_sez_cam = q_sez_cam->toRotationMatrix();

    // Full transformation BCRF->CAM
    Matrix3d r_bcrf_cam = r_sez_cam * r_ecef_sez * r_bcrf_ecef;

    long idx = 0l;
    for(std::pair<Source, ReferenceStar> &xm : *xms) {
        ReferenceStar &star = xm.second;
        CoordinateUtil::projectReferenceStar(star, r_bcrf_cam, *cam);
        model[idx++] = star.i;
        model[idx++] = star.j;
    }
}

void GeoCalFitter::postParameterUpdateCallback() {
    // Normalise the quaternion elements
    double norm = std::sqrt(params[0]*params[0] + params[1]*params[1] + params[2]*params[2] + params[3]*params[3]);
    params[0] /= norm;
    params[1] /= norm;
    params[2] /= norm;
    params[3] /= norm;
}

void GeoCalFitter::getJacobian(double * jac) {

    // Read out the current quaternion elements from the parameters
    q_sez_cam->w() = params[0];
    q_sez_cam->x() = params[1];
    q_sez_cam->y() = params[2];
    q_sez_cam->z() = params[3];

    // Set the parameters of the camera (advance pointer past the first four elements, which
    // contain the elements of the orientation quaternion)
    cam->setParameters(&params[4]);

    // Rotation matrices
    Matrix3d r_bcrf_ecef = CoordinateUtil::getBcrfToEcefRot(gmst);
    Matrix3d r_ecef_sez  = CoordinateUtil::getEcefToSezRot(lon, lat);
    Matrix3d r_sez_cam = q_sez_cam->toRotationMatrix();

    // Full transformation BCRF->SEZ
    Matrix3d r_bcrf_sez = r_ecef_sez * r_bcrf_ecef;
    // Full transformation BCRF->CAM
    Matrix3d r_bcrf_cam = r_sez_cam * r_ecef_sez * r_bcrf_ecef;

    // Array jac has size [N * M]; there are two rows for every cross-match and one
    // column for each of the quaternion elements and intrinsic camera parameters.

    long idx = 0l;
    for(std::pair<Source, ReferenceStar> &xm : *xms) {
        ReferenceStar &star = xm.second;

        // Unit vector towards star in BCRF frame:
        Vector3d r_bcrf;
        CoordinateUtil::sphericalToCartesian(r_bcrf, 1.0, star.ra, star.dec);

        // Transform to SEZ frame:
        Eigen::Vector3d r_sez = r_bcrf_sez * r_bcrf;

        // Transform to CAM frame:
        Eigen::Vector3d r_cam = r_bcrf_cam * r_bcrf;

        // Fill row idx of the jacobian: partial derivatives of the i coordinate of reference star idx
        // with respect to the extrinsic and intrinsic parameters of the camera.

        // Get the partial derivatives of the reference star (i,j) coordinates with respect to the
        // four components of the orientation quaternion.
        double extrinsic[8];
        cam->getExtrinsicPartialDerivatives(extrinsic, r_sez, *q_sez_cam);

        // Get the partial derivatives of the reference star (i,j) coordinates with respect to the
        // parameters of the camera projective optics.
        double intrinsic[2*cam->getNumParameters()];
        cam->getIntrinsicPartialDerivatives(intrinsic, r_cam);

        // Load these into the jacobian array

        // di/dq[0,1,2,3]
        jac[2*idx*M + 0] = extrinsic[0];
        jac[2*idx*M + 1] = extrinsic[2];
        jac[2*idx*M + 2] = extrinsic[4];
        jac[2*idx*M + 3] = extrinsic[6];

        // di/dcam[0,1,2, ...]
        for(unsigned int i=0; i<cam->getNumParameters(); i++) {
            jac[2*idx*M + 4 + i] = intrinsic[2*i];
        }

        // dj/dq[0,1,2,3]
        jac[(2*idx + 1)*M + 0] = extrinsic[1];
        jac[(2*idx + 1)*M + 1] = extrinsic[3];
        jac[(2*idx + 1)*M + 2] = extrinsic[5];
        jac[(2*idx + 1)*M + 3] = extrinsic[7];

        // dj/dcam[0,1,2, ...]
        for(unsigned int i=0; i<cam->getNumParameters(); i++) {
            jac[(2*idx + 1)*M + 4 + i] = intrinsic[2*i + 1];
        }

        idx++;
    }

}

//void GeoCalFitter::finiteDifferencesStepSizePerParam(double * steps) {
//    // Don't implement this: the camera model provides the jacobian, plus we don't know what
//    // type of camera model we're dealing with so don't know the appropriate step sizes to choose.

//    // Quaternion elements
//    steps[0] = 0.000001;
//    steps[1] = 0.000001;
//    steps[2] = 0.000001;
//    steps[3] = 0.000001;
//    // Camera intrinsics
//    steps[4] = 1.0;
//    steps[5] = 1.0;
//    steps[6] = 0.1;
//    steps[7] = 0.1;
//}
