#include "levenbergmarquardtsolver.h"

LevenbergMarquardtSolver::LevenbergMarquardtSolver(unsigned int M, unsigned int N) : M(M), N(N) {
    this->data = new Matrix<double, Dynamic, 1>(N, 1);
    this->params = new Matrix<double, Dynamic, 1>(M, 1);
    this->dataCovariance = new Matrix<double, Dynamic, Dynamic>(N, N);
    // Initialise dataCovariance to identity matrix
    this->dataCovariance->setIdentity();
}

LevenbergMarquardtSolver::~LevenbergMarquardtSolver() {
    delete data;
    delete params;
    delete dataCovariance;
}

void LevenbergMarquardtSolver::setH(double h)  {
    this->h   = h;
}

void LevenbergMarquardtSolver::setExitTolerance(double exitTolerance) {
    this->exitTolerance = exitTolerance;
}

void LevenbergMarquardtSolver::setMaxDamping(double maxDamping) {
    this->maxDamping = maxDamping;
}

void LevenbergMarquardtSolver::setData(double * data) {
    for(unsigned int row=0; row<N; row++) {
        this->data->operator()(row, 0) =  data[row];
    }
}

void LevenbergMarquardtSolver::setInitialGuessParameters(const double *params) {
    for(unsigned int row=0; row<M; row++) {
        this->params->operator()(row, 0) = params[row];
    }
}

void LevenbergMarquardtSolver::setCovariance(double * dataCovariance) {
    for(unsigned int row=0; row<N; row++) {
        for(unsigned int col=0; col<N; col++) {
            // Row-major packing
            unsigned int idx = row*N + col;
            // Column-major packing
//            unsigned int idx = col*N + row;
            this->dataCovariance->operator()(row, col) = dataCovariance[idx];
        }
    }
}

void LevenbergMarquardtSolver::getParametersSolution(double * params) {
    for(unsigned int row=0; row<M; row++) {
        params[row] = this->params->operator()(row, 0);
    }
}

void LevenbergMarquardtSolver::finiteDifferencesStepSizePerParam(double * steps) {

    for (unsigned int p=0; p<M; p++) {
        steps[p] = 1.0;
    }
    fprintf(stderr, "If getJacobian(const double *params, double * jac) is not overridden then "
                    "the finiteDifferencesStepSizePerParam() must be overridden!");
}

/**
 * @brief LevenbergMarquardtSolver::getJacobian
 * Provides a finite-differences approximation to the Jacobian. The user should consider
 * overriding this to provide an analytic Jacobian if that's possible.
 * @param params
 * @param jac
 */
void LevenbergMarquardtSolver::getJacobian(const double *params, double * jac) {

    // Set up Jacobian matrix: N rows by M columns
    MatrixXd jacobian(N, M);

    // Get finite step sizes to use for each parameter
    double steps[M];
    finiteDifferencesStepSizePerParam(steps);

    // Iterate over each parameter...
    for (unsigned int p=0; p<M; p++) {

        // First advance the parameter
        this->params->operator()(p, 0) += steps[p];

        // Get model values for advanced parameter set: f(x+h)
        MatrixXd model = getModel();

        // Store f(x+h) in the Jacobian
        for (unsigned int d = 0; d < N; d++) {
            jacobian(d, p) =  model(d,0);
        }

        // Now retard the parameter...
        this->params->operator()(p, 0) -= 2.0*steps[p];

        // Get model values for retarded parameter set: f(x-h)
        model = getModel();

        // Build Jacobian by finite difference
        for (unsigned int d = 0; d < N; d++) {
            // Retrieve f(x+h)...
            double fxph = jacobian(d, p);
            // Retrieve f(x-h)...
            double fxmh = model(d, 0);
            // Compute finite difference (f(x+h) - f(x-h))/2h
            jacobian(d, p) = (fxph - fxmh) / (2.0*steps[p]);
        }

        // Return the params to original value
        this->params->operator()(p, 0) += steps[p];
    }

    // Read out the values into the array
    for(unsigned int row=0; row<M; row++) {
        for(unsigned int col=0; col<N; col++) {
            // Row-major packing
            unsigned int idx = row*N + col;
            // Column-major packing
//            unsigned int idx = col*N + row;
            jac[idx] = jacobian(row, col);
        }
    }
}

void LevenbergMarquardtSolver::fit(unsigned int maxIterations, bool verbose) {

    // Covariance weighted chi-square for current parameter set
    double chi2_initial = getChi2();

    if(verbose) {
        fprintf(stderr, "LMA: %d data and %d parameters\n", N, M);
        fprintf(stderr, "LMA: Initial chi2 = %3.3f\n", chi2_initial);
    }

    // get suitable starting value for damping parameter, from 10^{-3}
    // times the average of the diagonal elements of JTWJ:

    MatrixXd J = getJacobian();

    fprintf(stderr, "J has %d rows and %d columns\n", J.rows(), J.cols());
    fprintf(stderr, "J.transpose() has %d rows and %d columns\n", J.transpose().rows(), J.transpose().cols());
    fprintf(stderr, "dataCovariance has %d rows and %d columns\n", dataCovariance->rows(), dataCovariance->cols());

    ColPivHouseholderQR<MatrixXd> qr = dataCovariance->colPivHouseholderQr();

    fprintf(stderr, "qr has %d rows and %d columns\n", qr.rows(), qr.cols());

    MatrixXd CJ = qr.solve(J);

    fprintf(stderr, "CJ has %d rows and %d columns\n", CJ.rows(), CJ.cols());

    MatrixXd JTWJ = J.transpose() * (dataCovariance->colPivHouseholderQr().solve(J));
    double L=0;
    for(unsigned int i=0; i<M; i++) {
        L += JTWJ(i, i);
    }
    L /= (M*1000.0);

    double lambda[] = {L, 10, L*maxDamping, exitTolerance};

    int nIterations = 0;

    while(!iteration(lambda, verbose) && nIterations<maxIterations) {

        // Un-comment the following for very verbose behaviour
        fprintf(stderr, "LMA: Iteration %d complete, residual = %3.3f\n", nIterations, getChi2());
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

bool LevenbergMarquardtSolver::iteration(double * lambda, bool verbose) {

    /**
     * Entries of array lambda are as follows:
     * [0] -    Value of the damping parameter. This will be overwritten
     *          by the method to provide the initial value for the next iteration
     * [1] -    Boost/shrink factor on unsuccessful/successful steps
     * [2] -    Max damping
     * [3] -    Exit tolerance
     */

    // chi-square prior to parameter update
    double chi2prev = getChi2();

    // Now get Jacobian matrix for current parameters
    MatrixXd J = getJacobian();

    // Compute RHS of LM update equation:
    // (J^T*W*J + diag(J^T*W*J))*delta = J^T*W*(residuals)
    // Note use of solve() method to avoid having to invert covariance
    // matrix to get W*(residuals).
    MatrixXd RHS = J.transpose() * (dataCovariance->colPivHouseholderQr().solve(getResiduals()));

    // Get J^T*W*J
    MatrixXd JTWJ = J.transpose() * (dataCovariance->colPivHouseholderQr().solve(J));

    // Change in chi-square from one iteration to the next
    double rrise = 0;

    // Exit status
    bool done = true;

    // Search for a good step:
    do {

        // Make damping matrix
        MatrixXd L(M,M);

        for (unsigned int i=0; i<M; i++) {
            L(i, i) =  lambda[0];
        }

        // Add this to Grammian
        MatrixXd LHS = JTWJ + L;

        MatrixXd delta(M, 1);

        delta = LHS.colPivHouseholderQr().solve(RHS);

        // TODO: what happens if LHS is singular?
//        fprintf(stderr, "LMA: Singular update matrix on exit");

        // Adjust parameters...
        (*params) += delta;

        // Get new chi-square statistic
        double chi2 = getChi2();

        // if rrise is negative, then current residuals are lower than
        // those found on previous step
        rrise = (chi2-chi2prev)/chi2;

        // Residuals dropped by an amount greater than exit tolerance.
        // Succesful LM iteration. Shrink damping parameter and quit loop.
        if (rrise < -lambda[3]) {

            //if(verbose)
            //    System.out.println("LMA: Good step. New chi2 = "+chi2);
            // Good step! Want more iterations.
            done = false;
            lambda[0] /= lambda[1];

            break;
        }

        // Exit tolerance exceeded: residuals changed by a very small
        // amount. We appear to be at the minimum, so keep previous
        // parameters and quit loop. Algorithm cannot find a better value.
        else if (fabs(rrise) < lambda[3]) {

            (*params) -= delta;

            // Cannot improve parameters - no further iterations
            if(verbose) {
                fprintf(stderr, "LMA: Residual threshold exceeded.");
            }
            break;

        }
        else {

            // Bad step (residuals increased)! Try again with larger damping.
            // Reset parameters to values before previous nudge.
            (*params) -= delta;

            // Boost damping parameter and try another step.
            lambda[0] *= lambda[1];
        }

    }
    // Check damping parameter remains within allowed range
    while (lambda[0]<=lambda[2]);

    if(lambda[0]>lambda[2] && verbose) {
        fprintf(stderr, "LMA: Damping threshold exceeded\n");
    }

    return done;
}

Matrix<double, Dynamic, 1> LevenbergMarquardtSolver::getModel() {
    double params[M];
    getParametersSolution(params);
    double modelArray[N];

    getModel(params, modelArray);

    MatrixXd model(N, 1);

    for(unsigned int row=0; row<N; row++) {
        model(row, 0) = modelArray[row];
    }

    return model;
}

MatrixXd LevenbergMarquardtSolver::getJacobian() {

    double params[M];
    getParametersSolution(params);
    double jac[N*M];

    // Get default or user-defined Jacobian
    getJacobian(params, jac);

    MatrixXd jacobian(N, M);

    for(unsigned int row=0; row<N; row++) {
        for(unsigned int col=0; col<M; col++) {
            // Row-major packing
            unsigned int idx = row*M + col;
            // Column-major packing
//            unsigned int idx = col*N + row;
            jacobian(row, col) = jac[idx];
        }
    }

    return jacobian;
}

double LevenbergMarquardtSolver::getChi2(){

    // Get residuals vector
    MatrixXd r = getResiduals();

    // Covariance weighted chi-square. Note use of solve() method
    // to avoid having to invert covariance matrix to get weight matrix.
    return (r.transpose() * dataCovariance->colPivHouseholderQr().solve(r))(0,0);
}

Matrix<double, Dynamic, 1> * LevenbergMarquardtSolver::getData() {
    return data;
}

MatrixXd LevenbergMarquardtSolver::getResiduals(){
    return (*getData()) - getModel();
}

double LevenbergMarquardtSolver::getReducedChi2(){
    return getChi2()/getDOF();
}

double LevenbergMarquardtSolver::getDOF(){
    return N - M;
}

MatrixXd LevenbergMarquardtSolver::getFourthOrderCovariance(){

    MatrixXd dpdx = getJacobian_dpdx();

    // First order propagation.
    return dpdx.transpose() * (*dataCovariance) * dpdx;
}

MatrixXd LevenbergMarquardtSolver::getParameterCovariance(){

    // Get Jacobian matrix for current parameter set
    MatrixXd J = getJacobian();

    // Get inverse of covariance matrix
    // TODO: shouldn't take the inverse here - use dataCovariance.solve(J) further on...
    MatrixXd W = dataCovariance->inverse();

    // Weighted sum of squared residuals
    double WSSR = getChi2();

    // Degrees of freedom
    double dof = getDOF();

    //System.out.println("RMS of residuals = "+Math.sqrt(WSSR/dof));

    // Weight terms in matrix W.
    W *= (dof / WSSR);

    // Get J^T*W*J using new weight matrix
    MatrixXd JTWJ = J.transpose() * W * J;

    // Invert...
    return JTWJ.inverse();
}

MatrixXd LevenbergMarquardtSolver::getAsymptoticStandardError(){

    // Get covariance matrix for current parameter set
    MatrixXd covariance = getParameterCovariance();

    MatrixXd standardError(M,1);

    for(unsigned int p=0; p<M; p++) {
        standardError(p, 0) = sqrt(covariance(p, p));
    }

    return standardError;
}

MatrixXd LevenbergMarquardtSolver::getParameterCorrelation() {

    MatrixXd covariance = getParameterCovariance();

    MatrixXd standardError = getAsymptoticStandardError();

    MatrixXd standardErrorSquared = standardError * standardError.transpose();

    MatrixXd correlation(M,M);

    for(unsigned int i=0; i<M; i++) {
        for(unsigned int j=0; j<M; j++) {
            correlation(i, j) = covariance(i, j) / standardErrorSquared(i, j);
        }
    }

    return correlation;
}

MatrixXd LevenbergMarquardtSolver::getJacobian_dpdx() {

    // Set up Jacobian matrix: N rows by M columns
    MatrixXd jac(N, M);

    // Iterate over each data point...
    for (unsigned int j=0; j<N; j++) {

        // Get f(x+2h)
        // Nudge data point j: d(j) -> d(j) + 2h
        (*data)(j,0) += 2*h;

        // Solve for updated parameters
        fit(500,false);

        // -f(x+2h)
        for (unsigned int i = 0; i < M; i++) {
            jac(j, i) = -(*params)(i,0);
        }

        // Get f(x+h)
        // Nudge data point j: d(j) + 2h -> d(j) + h
        (*data)(j,0) -= h;
        fit(500,false);
        // +8f(x+h)
        for (unsigned int i = 0; i < M; i++) {
            jac(j, i) += 8.0 *(*params)(i,0);
        }

        // Get f(x-h)
        // Nudge data point j: d(j) + h -> d(j) - h
        (*data)(j,0) -= 2*h;
        fit(500,false);
        // -8f(x-h)
        for (unsigned int i = 0; i < M; i++) {
            jac(j, i) -= 8.0 *(*params)(i,0);
        }

        // Get f(x-2h)
        // Nudge data point j: d(j) - h -> d(j) - 2*h
        (*data)(j,0) -= h;
        fit(500,false);
        // +f(x-2h)
        for (unsigned int i = 0; i < M; i++) {
            jac(j, i) += (*params)(i,0);
        }

        // Divide by step size to complete derivative approximation
        for (unsigned int i = 0; i < M; i++) {
            // (-f(x-2h) + 8f(x-h) - 8f(x+h) + f(x+2h)) / 12h
            jac(j, i) /= 12.0*h;
        }

        // Return data to its original value
        // Nudge data point j: d(j) - 2h -> d(j)
        (*data)(j,0) += 2*h;
    }

    return jac;
}
