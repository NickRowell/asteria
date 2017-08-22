#include "cosinefitter.h"

CosineFitter::CosineFitter(std::vector<double> &xs, std::vector<double> &ys) : LevenbergMarquardtSolver(3, xs.size()), xs(xs), ys(ys) {

    // Set the observations
    setData(&(ys[0]));
    double initialGuessParams[3];
    initialGuessParams[0] = 1.0;
    initialGuessParams[1] = 1.0;
    initialGuessParams[2] = 1.0;

    setInitialGuessParameters(initialGuessParams);
}


void CosineFitter::getModel(const double * params, double * model) {

    double amp = params[0];
    double freq = params[1];
    double phase = params[2];

    for(unsigned int i=0; i<N; i++) {
        double x = xs[i];
        model[i] = amp * std::cos(x * freq + phase);
    }
}

void CosineFitter::getJacobian(const double * params, double * jac) {

    double amp = params[0];
    double freq = params[1];
    double phase = params[2];

    for(unsigned int i=0; i<N; i++) {
        double x = xs[i];

        // Partial derivative with respect to amplitude
        jac[3*i + 0] = std::cos(x * freq + phase);
        // Partial derivative with respect to frequency
        jac[3*i + 1] = -amp * x * std::sin(x * freq + phase);
        // Partial derivative with respect to phase
        jac[3*i + 2] = -amp * std::sin(x * freq + phase);
    }
}

void CosineFitter::finiteDifferencesStepSizePerParam(double * steps) {
    // Step in amplitude
    steps[0] = 0.01;
    // Step in frequency
    steps[1] = 0.0001;
    // Step in phase
    steps[2] = 0.0001;
}
