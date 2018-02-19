#include "polynomialfitter.h"

PolynomialFitter::PolynomialFitter(std::vector<double> &xs, std::vector<double> &ys, unsigned int order) : LevenbergMarquardtSolver(order, xs.size()), xs(xs), ys(ys) {

    // Set the observations
    setData(&(ys[0]));
    double initialGuessParams[order];
    for(unsigned int p=0; p<order; p++) {
        initialGuessParams[p] = 1.0;
    }
    setParameters(initialGuessParams);
}


void PolynomialFitter::getModel(double * model) {
    for(unsigned int n=0; n<N; n++) {
        double x = xs[n];
        double tmp = 1.0;
        model[n] = 0.0;
        for(unsigned int m=0; m<M; m++) {
            model[n] += params[m] * tmp;
            tmp *= x;
        }
    }
}

void PolynomialFitter::getJacobian(double * jac) {
    for(unsigned int n=0; n<N; n++) {
        double x = xs[n];
        double tmp = 1.0;
        for(unsigned int m=0; m<M; m++) {
            jac[M*n + m] = tmp;
            tmp *= x;
        }
    }
}

void PolynomialFitter::finiteDifferencesStepSizePerParam(double * steps) {
    for(unsigned int m=0; m<M; m++) {
        steps[m] = 0.1;
    }
}
