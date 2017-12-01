#include "geocalfitter.h"

#include "util/coordinateutil.h"
#include "infra/referencestar.h"
#include "infra/source.h"

GeoCalFitter::GeoCalFitter(CameraModelBase *cam, Eigen::Quaterniond *q_sez_cam, std::vector<std::pair<Source, ReferenceStar> > *xms, const double &gmst, const double &lon, const double &lat) :
     LevenbergMarquardtSolver(cam->getNumParameters(), xms->size()*2), cam(cam), q_sez_cam(q_sez_cam), xms(xms), gmst(gmst), lon(lon), lat(lat) {

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
        unsigned int ji = (2 * idx + 1) * (N + 1);
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
    cam->getParameters(params);
}

void GeoCalFitter::getModel(const double *params, double *model) {

    // The model consists of the (i,j) coordinates of the reference stars

    // Read out the current quaternion elements from the parameters
    Eigen::Quaterniond q_sez_cam(params[0], params[1], params[2], params[3]);

    // Set the parameters of the camera (advance pointer past the first four elements, which
    // contain the elements of the orientation quaternion)
    cam->setParameters(params + 4*sizeof(double));

    // Rotation matrices
    Matrix3d r_bcrf_ecef = CoordinateUtil::getBcrfToEcefRot(gmst);
    Matrix3d r_ecef_sez  = CoordinateUtil::getEcefToSezRot(lon, lat);
    Matrix3d r_sez_cam = q_sez_cam.toRotationMatrix();

    // Full transformation BCRF->CAM
    Matrix3d r_bcrf_cam = r_sez_cam * r_ecef_sez * r_bcrf_ecef;

    long idx = 0l;
    for(std::pair<Source, ReferenceStar> xm : *xms) {
        ReferenceStar &star = xm.second;
        CoordinateUtil::projectReferenceStar(star, r_bcrf_cam, *cam);
        model[idx++] = star.i;
        model[idx++] = star.j;
    }
}

void GeoCalFitter::getJacobian(const double * params, double * jac) {

    fprintf(stderr, "Using GeoCalFitter getJacobian method\n");

    // Read out the current quaternion elements from the parameters
    Eigen::Quaterniond q_sez_cam(params[0], params[1], params[2], params[3]);

    // Set the parameters of the camera (advance pointer past the first four elements, which
    // contain the elements of the orientation quaternion)
    cam->setParameters(params + 4*sizeof(double));

    // Rotation matrices
    Matrix3d r_bcrf_ecef = CoordinateUtil::getBcrfToEcefRot(gmst);
    Matrix3d r_ecef_sez  = CoordinateUtil::getEcefToSezRot(lon, lat);
    Matrix3d r_sez_cam = q_sez_cam.toRotationMatrix();

    // Full transformation BCRF->SEZ
    Matrix3d r_bcrf_sez = r_ecef_sez * r_bcrf_ecef;
    // Full transformation BCRF->CAM
    Matrix3d r_bcrf_cam = r_sez_cam * r_ecef_sez * r_bcrf_ecef;

    // Array jac has size [N * M]; there are two rows for every cross-match and one
    // column for each of the quaternion elements and intrinsic camera parameters.

    long idx = 0l;
    for(std::pair<Source, ReferenceStar> xm : *xms) {
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
        cam->getExtrinsicPartialDerivatives(extrinsic, r_sez, r_sez_cam);

        // Get the partial derivatives of the reference star (i,j) coordinates with respect to the
        // parameters of the camera projective optics.
        double intrinsic[2*cam->getNumParameters()];
        cam->getIntrinsicPartialDerivatives(intrinsic, r_cam);

        // Load these into the jacobian array
        jac[2*idx*M + 0] = extrinsic[0];
        jac[2*idx*M + 1] = extrinsic[1];
        jac[2*idx*M + 2] = extrinsic[2];
        jac[2*idx*M + 3] = extrinsic[3];

        for(unsigned int i=0; i<cam->getNumParameters(); i++) {
            jac[2*idx*M + 3 + i] = intrinsic[i];
        }

        jac[(2*idx + 1)*M + 0] = extrinsic[4];
        jac[(2*idx + 1)*M + 1] = extrinsic[5];
        jac[(2*idx + 1)*M + 2] = extrinsic[6];
        jac[(2*idx + 1)*M + 3] = extrinsic[7];

        for(unsigned int i=0; i<cam->getNumParameters(); i++) {
            jac[(2*idx + 1)*M + 3 + i] = intrinsic[cam->getNumParameters() + i];
        }
    }

}

void GeoCalFitter::finiteDifferencesStepSizePerParam(double * steps) {
    // Don't implement this: the camera model provides the jacobian, plus we don't know what
    // type of camera model we're dealing with so don't know the appropriate step sizes to choose.
}

void GeoCalFitter::fit(unsigned int maxIterations, bool verbose) {

    // Compute the initial model
    getModel(params, model);

    // Covariance weighted chi-square for current parameter set
    double chi2_initial = getChi2();

    if(verbose) {
        fprintf(stderr, "LMA: %d data and %d parameters\n", N, M);
        fprintf(stderr, "LMA: Initial chi2 = %3.3f\n", chi2_initial);
    }

    // Get suitable starting value for damping parameter, from 10^{-3}
    // times the average of the diagonal elements of JTWJ:

    double jac[N*M];
    getJacobian(params, jac);
    // Load the Jacobian elements into an Eigen Matrix for linear algebra operations
    Map<Matrix<double, Dynamic, Dynamic, RowMajor>> J(jac, N, M);

    // Compute W*J, where W is the inverse of the covariance matrix
    MatrixXd WJ(N, M);
    if(covarianceIsDiagonal) {
        // Manually divide each row of J by the inverse of the corresponding variance term
        for(unsigned int n=0; n<N; n++) {
            WJ.row(n) = J.row(n) / covariance[n];
        }
    }
    else {
        Map<Matrix<double, Dynamic, Dynamic, RowMajor>> W(covariance, N, N);
        WJ = W.colPivHouseholderQr().solve(J);
    }
    MatrixXd JTWJ = J.transpose() * WJ;

    double L = JTWJ.trace()/(M*1000.0);

    double lambda[] = {L, L*maxDamping};

    unsigned int nIterations = 0;

    while(!iteration(lambda, verbose) && nIterations<maxIterations) {

        // TODO: renormalise the quaternion elements
        Eigen::Quaterniond q_sez_cam(params[0], params[1], params[2], params[3]);
        q_sez_cam.normalize();
        params[0] = q_sez_cam.w();
        params[1] = q_sez_cam.x();
        params[2] = q_sez_cam.y();
        params[3] = q_sez_cam.z();

        if(verbose) {
            fprintf(stderr, "LMA: Iteration %d complete, residual = %3.3f\n", nIterations, getChi2());
        }
        nIterations++;
    }

    if(verbose) {
        // Chi-square on exit
        double chi2_final = getChi2();
        fprintf(stderr, "LMA: Number of iterations = %d\n", nIterations);
        fprintf(stderr, "LMA: Final chi2 = %3.3f\n", chi2_final);
        fprintf(stderr, "LMA: Reduced chi2 = %3.3f\n", getReducedChi2());
        fprintf(stderr, "LMA: Reduction factor = %3.3f\n", chi2_initial/chi2_final);
    }

    return;
}
