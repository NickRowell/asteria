#ifndef POLYNOMIALFITTER_H
#define POLYNOMIALFITTER_H

#include "math/levenbergmarquardtsolver.h"

/**
 * @brief The PolynomialFitter class
 *
 * Used to fit a polynomial of arbitrary order to a set of points. The polynomial is defined in terms of
 * the parameters p[M] as:
 * model = p[0] + p[1]*x + p[2]*x*x + p[3]*x*x*x ...
 *
 */
class PolynomialFitter : public LevenbergMarquardtSolver
{
public:
    PolynomialFitter(std::vector<double> &xs, std::vector<double> &ys, unsigned int order);

    std::vector<double> xs;
    std::vector<double> ys;

    void getModel(const double * params, double * model);

    // One or the other of these should be implemented:
    void getJacobian(const double * params, double * jac);
    void finiteDifferencesStepSizePerParam(double *steps);

};

#endif // POLYNOMIALFITTER_H
