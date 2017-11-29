#include "geocalfitter.h"

GeoCalFitter::GeoCalFitter(CameraModelBase *cam, Eigen::Quaterniond *q_sez_cam, std::vector<std::pair<Source, ReferenceStar> > *xms, const double &gmst, const double &lon, const double &lat) :
     LevenbergMarquardtSolver(cam->getNumParameters(), xms->size()*2), cam(cam), q_sez_cam(q_sez_cam), xms(xms), gmst(gmst), lon(lon), lat(lat) {

}

void GeoCalFitter::getModel(const double *params, double *model) {

}

void GeoCalFitter::getJacobian(const double * params, double * jac) {

//    double amp = params[0];
//    double freq = params[1];
//    double phase = params[2];

//    for(unsigned int n=0; n<N; n++) {
//        double x = xs[n];

//        // Partial derivative with respect to amplitude
//        jac[3*n + 0] = std::cos(x * freq + phase);
//        // Partial derivative with respect to frequency
//        jac[3*n + 1] = -amp * x * std::sin(x * freq + phase);
//        // Partial derivative with respect to phase
//        jac[3*n + 2] = -amp * std::sin(x * freq + phase);
//    }
}

// Don't implement this: the camera model provides the jacobian, plus we don't know what
// type of camera model we're dealing with so don't know the appropriate step sizes to choose.
void GeoCalFitter::finiteDifferencesStepSizePerParam(double * steps) {
    // Step in amplitude
    steps[0] = 0.01;
    // Step in frequency
    steps[1] = 0.0001;
    // Step in phase
    steps[2] = 0.0001;
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
