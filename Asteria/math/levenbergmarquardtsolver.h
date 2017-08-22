#ifndef LEVENBERGMARQUARDTSOLVER_H
#define LEVENBERGMARQUARDTSOLVER_H

/**
 * Note on the usage of virtual functions:
 * virtual keyword indicates a function that MAY be overridden in a derived class.
 * The =0 syntax indicates a 'pure virtual' function that MUST be overridden in derived classes.
 *
 *
 * @brief The LevenbergMarquardtSolver class
 * This class provides a general purpose implementation of the Levenberg-Marquardt
 * algorithm for nonlinear least squares. Applications that want to use this should
 * subclass it and implement/override the following methods:
 *
 * In order to use this class you must extend it and implement the getModel(Matrix params)
 * method and optionally the getJacobian(double[] params) method. If you would like the
 * fitter to use a numerical approximation to the Jacobian instead then override the
 * {@link #useFiniteDifferencesJacobian()} to return true and also override {@link #finiteDifferencesStepSizePerParam()}
 * to provide an appropriate step size for each parameter.
 *
 * Normal usage:
 *
 *	LevenbergMarquardt lma = new LevenbergMarquardt() {
 *
 *		@Override
 *		public double[] getModel(double[] params) {
 *
 *			double[] model = new double[N];
 *
 *			// Implement the parameters -> model function
 *
 *			return model;
 *		}
 *
 *		@Override
 * 		public double[][] getJacobian(double[] params) {
 *
 * 			double[][] jac = new double[N][M];
 *
 *			// Compute the Jacobian elements
 *
 *			return jac;
 *		}
 *	}
 *
 *	lma.setData(electronSamples);
 *	lma.setCovariance(cov);
 *	lma.setInitialGuessParameters(params);
 *	// Perform the optimization
 *	lma.fit(500, true);
 *
 *	// Extract the solution
 *	double[] solution = lma.getParametersSolution();
 *
 *
 */

#include <Eigen/Dense>

using namespace Eigen;

class LevenbergMarquardtSolver
{

public:

    LevenbergMarquardtSolver(unsigned int M, unsigned int N);
    ~LevenbergMarquardtSolver();

    /**
     * Set the absolute step size used in the finite difference Jacobian estimation for the
     * calculation of the rate of change of parameters as a function of the data.
     * @param H
     * 	The finite step size
     */
    void setH(double h);
    /**
     * Set the exit tolerance - if the (absolute value of the) relative change in the chi-square
     * from one iteration to the next is lower than this, then we're at the minimum and the fit
     * is halted.
     * @param exitTolerance
     * 	The exit tolerance to set
     */
    void setExitTolerance(double exitTolerance);

    /**
     * Set the maximum damping factor. If the damping factor becomes larger than this during the fit,
     * then we're stuck and cannot reach a better solution.
     *
     * @param maxDamping
     * 	The max damping factor to set
     */
    void setMaxDamping(double maxDamping);


    /**
     * Set the Nx1 column vector of observed values
     * @param data
     * 	Pointer to an N-element array of observed values
     */
    void setData(double * data);

    /**
     * Set the Mx1 column vector of initial-guess parameters.
     *
     * @param params
     * 	Pointer to an M-element array of initial-guess parameters.
     */
    void setInitialGuessParameters(const double * params);

    /**
     * Set the NxN matrix of covariance of the data points.
     *
     * @param dataCovariance
     * 	NxN matrix of covariance of the data points, packed in a one dimensional array in
     * row-major order.
     */
    void setCovariance(double * dataCovariance);

    /**
     * @brief getParametersSolution
     * @param params
     *  Pointer to an M-element array; on exit this will contain the solution
     */
    void getParametersSolution(double * params);

    /**
     * @brief getModel
     * Get f(X,P): column vector of model values given x points and current
     * parameters set.
     *
     * f(X,P) = [f(x_1, P), f(x_2, P), ... , f(x_N, P)]^T
     *
     * This method MUST be overridden in the derived class.
     * @param params
     *  Pointer to an M-element array containing the parameters of the model
     * @param model
     *  Pointer to an N-element array that on exit will contain the model values
     */
    virtual void getModel(const double * params, double * model) =0;

    /**
     * @brief getJacobian
     * Get the Jacobian matrix -> the matrix of partial derivatives of the
     * model values with respect to the parameters, given the current parameter set.
     *
     * J = [ df(x_0, P)/dp_0  df(x_0, P)/dp_1  df(x_0, p)/dp_2  ...  df(x_0, P)/dp_{M-1} ]
     *     [ df(x_1, P)/dp_0  df(x_1, P)/dp_1  df(x_1, p)/dp_2  ...  df(x_1, P)/dp_{M-1} ]
     *
     *
     * Leading dimension is number of rows, trailing dimension is number of columns, i.e.
     * A[r][c] has r rows and c columns.
     *
     * This function MAY be overridden in the derived class if an analytic Jacobian is possible.
     * A default implementation based on finite differences is provided.
     *
     * @param params
     *  Pointer to an M-element array containing the parameters of the model
     * @param jac
     *  NxM element array that on exit will contain the Jacobian values, packed in a one
     * dimensional array in row-major order.
     */
    virtual void getJacobian(const double * params, double * jac);

    /**
     * @brief finiteDifferencesStepSizePerParam
     * Implementing classes should override this to provide appropriate step sizes per parameter for use
     * in the finite differences Jacobian approximation, if they intend to use that.
     *
     * This function MAY be overridden in the derived class: if an analytic Jacobian is not possible and the
     * getJacobian(double[] params) method is not overridden, then this function should be overridden to
     * provide appropriate step sizes for each parameter in the numerical Jacobian approximation.
     *
     * @param steps
     *  Pointer to an M-element array; on exit this contains appropriate finite-difference step sizes for each
     * parameter.
     */
    virtual void finiteDifferencesStepSizePerParam(double *steps);

    /**
     * @brief fit
     * Perform LM iteration loop until parameters cannot be improved.
     * @param maxIterations  Maximum number of allowed iteration before convergence.
     * @param verbose        Enables verbose logging
     */
    void fit(unsigned int maxIterations, bool verbose);

    /**
     * Chi-square statistic, (x - f(x))^T*C^{-1}*(x - f(x))
     */
    double getChi2();

    /**
     * Reduced Chi-square statistic.
     */
    double getReducedChi2();

    /**
     * Degrees of freedom of fit.
     */
    double getDOF();

    /**
     * This method estimates parameter covariance by propagating data
     * covariance through the system using the following equation:
     *
     * S_p = (dp/dx)^T S_x (dp/dx)
     *
     * It uses a fourth order central difference approximation for the
     * parameter/data Jacobian.
     *
     * Note that this method fails for functions that are significantly
     * non-linear within a STDDEV or two of current solution.
     */
    MatrixXd getFourthOrderCovariance();

    /**
     * Get the covariance matrix for parameters. This method has been tested
     * against Gnuplot 'fit' function and provides the same asymptotic
     * standard error and parameter correlation.
     */
    MatrixXd getParameterCovariance();

    /**
     * Get the asymptotic standard error for the parameters
     */
    MatrixXd getAsymptoticStandardError();

    /**
     * Get the correlation matrix for the parameters
     */
    MatrixXd getParameterCorrelation();

protected:

    /**
     * Number of free parameters.
     */
    unsigned int M;

    /**
     * Number of data points.
     */
    unsigned int N;

private:
    /**
     * Absolute step size used in finite difference Jacobian approximation,
     * for Jacobian of parameter solution with respect to data.
     */
    double h = 1E-2;

    /**
     * Exit tolerance
     */
    double exitTolerance = 1E-32;

    /**
     * Max damping scale factor. This is multiplied by automatically selected
     * starting value of damping parameter, which is 10^{-3}
     * times the average of the diagonal elements of J^T*J
     */
    double maxDamping = 1E32;

    /**
     * Nx1 column vector of observed values
     *
     * Y = [y_0, y_1, y_2, ..., y_{N-1}]^T
     *
     */
    Matrix<double, Dynamic, 1> * data;

    /**
     * NxN covariance matrix for the observed values
     */
    Matrix<double, Dynamic, Dynamic> *  dataCovariance;

    /**
     * Mx1 column vector of parameters
     *
     * P = [p_0, p_1, p_2, ..., p_{M-1}]^T
     *
     */
    Matrix<double, Dynamic, 1> * params;


    /**
     * Get the vector of observed values.
     *
     * @return
     * 	The vector of observed values
     */
    Matrix<double, Dynamic, 1> * getData();

    /**
     * @brief getModel
     * Get the model in Matrix form
     * @return
     *  The model in Matrix form
     */
    Matrix<double, Dynamic, 1> getModel();

    /**
     * @brief getJacobian
     * Gets the Jacobian for the current parameters values.
     * @return
     *  The Jacobian in Matrix form
     */
    MatrixXd getJacobian();

    /**
     * Each call performs one iteration of parameters.
     *
     * @param   lambda  Damping parameters: element 0 is value of lambda, 1 is boost/shrink factor,
     * 					2 is the max permitted damping, 4 is exit tolerance on residual change
     *                  between steps.
     *
     * @return Boolean  States whether another iteration would be appropriate, or
     *                  if change in residuals and/or damping thresholds have
     *                  been reached
     */
    bool iteration(double * lambda, bool verbose);


    /**
     * Get x - f(x)
     */
    MatrixXd getResiduals();

    /**
     * Finite difference Jacobian approximation. This is the derivative of the
     * parameters solution with respect to the data, useful in estimating the
     * parameters covariance with respect to the data.
     *
     * Jacobian is N rows by M columns.
     *
     * This uses fourth-order central difference approximation for the first
     * derivative.
     *
     */
    MatrixXd getJacobian_dpdx();

};

#endif // LEVENBERGMARQUARDTSOLVER_H
