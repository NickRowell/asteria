#ifndef COSINEFITTER_H
#define COSINEFITTER_H

#include "math/levenbergmarquardtsolver.h"

/**
 * @brief The CosineFitter class
 * Fits a cosine function to data points. The parameters of the fit are the amplitude, frequency and phase.
 */
class CosineFitter : public LevenbergMarquardtSolver {
public:
    CosineFitter(std::vector<double> &xs, std::vector<double> &ys);

    std::vector<double> xs;
    std::vector<double> ys;

    void getModel(const double * params, double * model);

    // One or the other of these should be implemented:
    void getJacobian(const double * params, double * jac);
    void finiteDifferencesStepSizePerParam(double *steps);
};

#endif // COSINEFITTER_H
