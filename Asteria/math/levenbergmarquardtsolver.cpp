#include "levenbergmarquardtsolver.h"

#include <iostream>

LevenbergMarquardtSolver::LevenbergMarquardtSolver(unsigned int M, unsigned int N) : M(M), N(N) {
    data = new double[N];
    model = new double[N];
    params = new double[M];
    covariance = new double[N];
    covarianceIsDiagonal = true;
    // Initialise covariance to identity matrix
    for(unsigned int n=0; n<N; n++) {
        covariance [n] = 1.0;
    }
}

LevenbergMarquardtSolver::~LevenbergMarquardtSolver() {
    delete data;
    delete model;
    delete params;
    delete covariance;
}


void LevenbergMarquardtSolver::setData(const double * data) {
    for(unsigned int n=0; n<N; n++) {
        this->data[n] =  data[n];
    }
}

void LevenbergMarquardtSolver::setCovariance(const double * covariance) {
    covarianceIsDiagonal = false;
    delete this->covariance;
    this->covariance = new double[N*N];
    for(unsigned int idx=0; idx<N*N; idx++) {
        this->covariance[idx] = covariance[idx];
    }
}

void LevenbergMarquardtSolver::setVariance(const double * variance) {
    covarianceIsDiagonal = true;
    delete covariance;
    covariance = new double[N];
    for(unsigned int idx=0; idx<N; idx++) {
        this->covariance[idx] = variance[idx];
    }
}

void LevenbergMarquardtSolver::setParameters(const double *params) {
    for(unsigned int m=0; m<M; m++) {
        this->params[m] = params[m];
    }
}

void LevenbergMarquardtSolver::getParameters(double * params) {
    for(unsigned int m=0; m<M; m++) {
        params[m] = this->params[m];
    }
}

void LevenbergMarquardtSolver::setH(double h)  {
    this->h = h;
}

void LevenbergMarquardtSolver::setExitTolerance(double exitTolerance) {
    this->exitTolerance = exitTolerance;
}

void LevenbergMarquardtSolver::setMaxDamping(double maxDamping) {
    this->maxDamping = maxDamping;
}

void LevenbergMarquardtSolver::finiteDifferencesStepSizePerParam(double * steps) {

    for (unsigned int m=0; m<M; m++) {
        steps[m] = 1.0;
    }
    fprintf(stderr, "If getJacobian(double * jac) is not overridden then "
                    "the finiteDifferencesStepSizePerParam() should be overridden!");
}

/**
 * @brief LevenbergMarquardtSolver::getJacobian
 * Provides a finite-differences approximation to the Jacobian. The user should consider
 * overriding this to provide an analytic Jacobian if that's possible.
 * @param params
 *  Pointer to the start of an array of double with Mx1 elements, containing the current parameters.
 * @param jac
 *  Pointer to the start of an array of doubles with NxM elements; on exit this will be filled
 * with the Jacobian values, packed in row-major order.
 */
void LevenbergMarquardtSolver::getJacobian(double * jac) {

    fprintf(stderr, "Using finite difference Jacobian\n");

    // Get finite step sizes to use for each parameter
    double steps[M];
    finiteDifferencesStepSizePerParam(steps);

    // Cache initial parameters so we can restore them
    double initParam[M];
    std::copy(&params[0], &params[M], initParam);

    // Iterate over each parameter...
    for (unsigned int m=0; m<M; m++) {

        // First advance the parameter
        params[m] += steps[m];
        postParameterUpdateCallback();

        // Get model values for advanced parameter set: f(x+h)
        getModel(model);

        // Store f(x+h) in the Jacobian
        for (unsigned int n = 0; n < N; n++) {
            // Row-major packing
            unsigned int idx = n*M + m;
            jac[idx] =  model[n];
        }

        // Now retard the parameter...
        std::copy(&initParam[0], &initParam[M], params);
        params[m] -= steps[m];
        postParameterUpdateCallback();

        // Get model values for retarded parameter set: f(x-h)
        getModel(model);

        // Build Jacobian by finite difference
        for (unsigned int n = 0; n < N; n++) {
            // Row-major packing
            unsigned int idx = n*M + m;

            // Retrieve f(x+h)...
            double fxph = jac[idx];
            // Retrieve f(x-h)...
            double fxmh = model[n];
            // Compute finite difference (f(x+h) - f(x-h))/2h
            jac[idx] = (fxph - fxmh) / (2.0*steps[m]);
        }

        // Return the parameter to original value
        std::copy(&initParam[0], &initParam[M], params);

        // Reset the model
        getModel(model);
    }
}

void LevenbergMarquardtSolver::postParameterUpdateCallback() {
    // Default implementation does nothing.
}

void LevenbergMarquardtSolver::fit(unsigned int maxIterations, bool verbose) {

    // Compute the initial model
    getModel(model);

    // Covariance weighted chi-square for current parameter set
    double chi2_initial = getChi2();

    if(verbose) {
        fprintf(stderr, "LMA: %d data and %d parameters\n", N, M);
        fprintf(stderr, "LMA: Initial chi2 = %3.3f\n", chi2_initial);
    }

    // Get suitable starting value for damping parameter, from 10^{-3}
    // times the average of the diagonal elements of JTWJ:
    double jac[N*M];
    getJacobian(jac);

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

    double lambda = JTWJ.trace()/(M*1000.0);
    double maxLambda = lambda*maxDamping;

    unsigned int nIterations = 0;

    while(!iteration(lambda, maxLambda, verbose) && nIterations<maxIterations) {

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

bool LevenbergMarquardtSolver::iteration(double &lambda, const double &maxLambda, bool verbose) {

    // Compute model
    getModel(model);

    // Compute chi-square prior to parameter update
    double chi2prev = getChi2();

    // Now get Jacobian matrix for current parameters
    double jac[N*M];
    getJacobian(jac);
    // Load the Jacobian elements into an Eigen Matrix for linear algebra operations
    Map<Matrix<double, Dynamic, Dynamic, RowMajor>> J(jac, N, M);

    // Compute RHS of LM update equation:
    // (J^T*W*J + diag(J^T*W*J))*delta = J^T*W*(residuals)
    MatrixXd WJ(N, M);
    MatrixXd WR(N, 1);

    // Get residuals array
    double residuals[N];
    getResiduals(residuals);

    if(covarianceIsDiagonal) {
        // Manually divide each row of J by the inverse of the corresponding variance term
        for(unsigned int n=0; n<N; n++) {
            WJ.row(n) = J.row(n) / covariance[n];
            WR(n, 0) = residuals[n] / covariance[n];
        }
    }
    else {
        // Load covariance elements into a Matrix
        Map<Matrix<double, Dynamic, Dynamic, RowMajor>> C(covariance, N, N);
        // Load residuals into a Matrix
        Map<Matrix<double, Dynamic, Dynamic, RowMajor>> R(residuals, N, 1);

        ColPivHouseholderQR<Matrix<double, Dynamic, Dynamic, RowMajor>> W = C.colPivHouseholderQr();
        WR = W.solve(R);
        WJ = W.solve(J);
    }

    // Get J^T*W*(residuals)
    MatrixXd RHS = J.transpose() * WR;

    // Get J^T*W*J
    MatrixXd JTWJ = J.transpose() * WJ;

    // Change in chi-square from one iteration to the next
    double rrise = 0;

    // Copy initial parameters so we can restore them if necessary
    double initParam[M];
    std::copy(&params[0], &params[M], initParam);

    // Exit status
    bool done = true;

    // Search for a good step:
    do {
        // Make damping matrix
        MatrixXd L = MatrixXd::Constant(M, M, 0.0);
        // Insert diagonal elements of JTWJ multiplied by damping factor
        for(unsigned int m=0; m<M; m++) {
            // Diagonal element
            L(m, m) = JTWJ(m, m) * lambda;
        }

        // Add this to Grammian
        MatrixXd LHS = JTWJ + L;

        // Compute parameter adjustment vector
        MatrixXd delta = LHS.colPivHouseholderQr().solve(RHS);

        // Adjust parameters...
        for(unsigned int m=0; m<M; m++) {
            params[m] += delta(m, 0);
        }

        postParameterUpdateCallback();

        // Recompute model
        getModel(model);

        // Get new chi-square statistic
        double chi2 = getChi2();

        // if rrise is negative, then current residuals are lower than
        // those found on previous step
        rrise = (chi2-chi2prev)/chi2;

        // Residuals dropped by an amount greater than exit tolerance.
        // Succesful LM iteration. Shrink damping parameter and quit loop.
        if (rrise < -exitTolerance) {
            // Good step! Want more iterations.
            done = false;
            lambda /= boostShrinkFactor;
            break;
        }

        // Exit tolerance exceeded: residuals changed by a very small
        // amount. We appear to be at the minimum, so keep previous
        // parameters and quit loop. Algorithm cannot find a better value.
        else if (fabs(rrise) < exitTolerance) {

            std::copy(&initParam[0], &initParam[M], params);

            // Reset the model
            getModel(model);

            // Cannot improve parameters - no further iterations
            if(verbose) {
                fprintf(stderr, "LMA: Residual threshold exceeded\n");
            }
            break;
        }
        else {

            // Bad step (residuals increased)! Try again with larger damping.
            // Reset parameters to values before previous nudge.
            std::copy(&initParam[0], &initParam[M], params);

            // Reset the model
            getModel(model);

            // Boost damping parameter and try another step.
            lambda *= boostShrinkFactor;
        }

    }
    // Check damping parameter remains within allowed range
    while (lambda<=maxLambda);

    if(lambda>maxLambda && verbose) {
        fprintf(stderr, "LMA: Damping threshold exceeded (%f > %f)\n", lambda, maxLambda);
    }

    return done;
}

double LevenbergMarquardtSolver::getChi2() {

    // Get residuals array
    double residuals[N];
    getResiduals(residuals);

    double chi2 = 0.0;

    if(covarianceIsDiagonal) {
        // Manually divide each row of J by the inverse of the corresponding variance term
        for(unsigned int n=0; n<N; n++) {
            chi2 += (residuals[n] * residuals[n]) / covariance[n];
        }
    }
    else {
        // Load residuals into a Matrix for full covariance weighted chi-square
        Map<Matrix<double, Dynamic, Dynamic, RowMajor>> R(residuals, N, 1);
        // Load covariance array into a Matrix
        Map<Matrix<double, Dynamic, Dynamic, RowMajor>> C(covariance, N, N);

        chi2 = (R.transpose() * C.jacobiSvd(ComputeThinU | ComputeThinV).solve(R))(0, 0);
    }
    return chi2;
}

void LevenbergMarquardtSolver::getResiduals(double * residuals) {
    for(unsigned int n=0; n<N; n++) {
        residuals[n] = data[n] - model[n];
    }
}

double LevenbergMarquardtSolver::getReducedChi2() {
    return getChi2()/getDOF();
}

double LevenbergMarquardtSolver::getDOF() {
    return N - M;
}


MatrixXd LevenbergMarquardtSolver::getParameterCovariance() {

    // Get Jacobian matrix for current parameter set
    double jac[N*M];
    getJacobian(jac);
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

    // This step is thrown in to make results match Gnuplot. Without this scaling, the function
    // gives the same results as the getFourthOrderCovariance() function.
    JTWJ /= getReducedChi2();

    // Invert...
    return JTWJ.inverse();
}

void LevenbergMarquardtSolver::getAsymptoticStandardError(double *errors) {
    // Get covariance matrix for current parameter set
    MatrixXd covariance = getParameterCovariance();

    for(unsigned int p=0; p<M; p++) {
        errors[p] = sqrt(covariance(p, p));
    }
}

MatrixXd LevenbergMarquardtSolver::getParameterCorrelation() {

    MatrixXd covariance = getParameterCovariance();

    double errors[M];
    getAsymptoticStandardError(errors);

    // Outer product of the errors
    MatrixXd standardErrorSquared(M, M);
    for(unsigned int i=0; i<M; i++) {
        for(unsigned int j=0; j<M; j++) {
            standardErrorSquared(i, j) = errors[i] * errors[j];
        }
    }

    MatrixXd correlation(M, M);

    for(unsigned int i=0; i<M; i++) {
        for(unsigned int j=0; j<M; j++) {
            correlation(i, j) = covariance(i, j) / standardErrorSquared(i, j);
        }
    }

    return correlation;
}

MatrixXd LevenbergMarquardtSolver::getFourthOrderCovariance() {

    MatrixXd dpdx = getJacobian_dpdx();

    // Transform the data covariance matrix to the parameter space by:
    // S_p = (dp/dx)^T S_x (dp/dx)

    MatrixXd sx_dpdx(N, M);
    if(covarianceIsDiagonal) {
        // Manually multiply each row of dp/dx by the corresponding variance term
        for(unsigned int n=0; n<N; n++) {
            sx_dpdx.row(n) = dpdx.row(n) * covariance[n];
        }
    }
    else {
        Map<Matrix<double, Dynamic, Dynamic, RowMajor>> sx(covariance, N, N);
        sx_dpdx = sx * dpdx;
    }

    return dpdx.transpose() * sx_dpdx;
}

MatrixXd LevenbergMarquardtSolver::getJacobian_dpdx() {

    // Set up Jacobian matrix: N rows by M columns
    MatrixXd jac(N, M);

    // Iterate over each data point...
    for (unsigned int n=0; n<N; n++) {

        // Get f(x+2h)
        // Nudge data point j: d(j) -> d(j) + 2h
        data[n] += 2*h;

        // Solve for updated parameters
        fit(500, false);

        // -f(x+2h)
        for (unsigned int m = 0; m < M; m++) {
            jac(n, m) = -params[m];
        }

        // Get f(x+h)
        // Nudge data point j: d(j) + 2h -> d(j) + h
        data[n] -= h;
        fit(500, false);

        // +8f(x+h)
        for (unsigned int m = 0; m < M; m++) {
            jac(n, m) += 8.0 * params[m];
        }

        // Get f(x-h)
        // Nudge data point j: d(j) + h -> d(j) - h
        data[n] -= 2*h;
        fit(500, false);

        // -8f(x-h)
        for (unsigned int m = 0; m < M; m++) {
            jac(n, m) -= 8.0 * params[m];
        }

        // Get f(x-2h)
        // Nudge data point j: d(j) - h -> d(j) - 2*h
        data[n] -= h;
        fit(500, false);

        // +f(x-2h)
        for (unsigned int m = 0; m < M; m++) {
            jac(n, m) += params[m];
        }

        // Divide by step size to complete derivative approximation
        for (unsigned int m = 0; m < M; m++) {
            // (-f(x-2h) + 8f(x-h) - 8f(x+h) + f(x+2h)) / 12h
            jac(n, m) /= 12.0*h;
        }

        // Return data to its original value
        // Nudge data point j: d(j) - 2h -> d(j)
        data[n] += 2*h;
    }

    return jac;
}
