#include "testutil.h"

#include "math/polynomialfitter.h"
#include "util/coordinateutil.h"
#include "util/mathutil.h"
#include "util/timeutil.h"
#include "infra/imaged.h"

#include <fstream>

#include <Eigen/Dense>

using namespace Eigen;

void TestUtil::testLevenbergMarquardtFitter() {

    // This can be compared to the results of Gnuplot using the script:
    // f(x) = a*x**2 + b*x + c
    // a = 1
    // b = 1
    // c = 1
    // FIT_LIMIT = 1e-6
    // fit f(x) '-' via a, b, c
    // -1.000000	24.037018
    // -0.900000	22.175963
    // -0.800000	20.859003
    // -0.700000	18.599866
    // -0.600000	16.905037
    // -0.500000	15.040288
    // -0.400000	13.548298
    // -0.300000	11.557022
    // -0.200000	10.067366
    // -0.100000	8.478361
    // -0.000000	7.116529
    // 0.100000	5.427864
    // 0.200000	3.413441
    // 0.300000	2.827226
    // 0.400000	1.186261
    // 0.500000	0.128155
    // 0.600000	-1.079268
    // 0.700000	-2.396080
    // 0.800000	-3.436993
    // 0.900000	-4.574391
    // 1.000000	-6.014677
    // e

    std::vector<double> xs;
    std::vector<double> ys;

//     True parameters:
//     a = 2.35;
//     b = -15.3;
//     c = 6.367;
    xs.push_back(-1.000000);	ys.push_back(24.037018);
    xs.push_back(-0.900000);	ys.push_back(22.175963);
    xs.push_back(-0.800000);	ys.push_back(20.859003);
    xs.push_back(-0.700000);	ys.push_back(18.599866);
    xs.push_back(-0.600000);	ys.push_back(16.905037);
    xs.push_back(-0.500000);	ys.push_back(15.040288);
    xs.push_back(-0.400000);	ys.push_back(13.548298);
    xs.push_back(-0.300000);	ys.push_back(11.557022);
    xs.push_back(-0.200000);	ys.push_back(10.067366);
    xs.push_back(-0.100000);	ys.push_back(8.478361);
    xs.push_back(-0.000000);	ys.push_back(7.116529);
    xs.push_back(0.100000);	    ys.push_back(5.427864);
    xs.push_back(0.200000);	    ys.push_back(3.413441);
    xs.push_back(0.300000);	    ys.push_back(2.827226);
    xs.push_back(0.400000);	    ys.push_back(1.186261);
    xs.push_back(0.500000);	    ys.push_back(0.128155);
    xs.push_back(0.600000);	    ys.push_back(-1.079268);
    xs.push_back(0.700000);	    ys.push_back(-2.396080);
    xs.push_back(0.800000);	    ys.push_back(-3.436993);
    xs.push_back(0.900000);	    ys.push_back(-4.574391);
    xs.push_back(1.000000);	    ys.push_back(-6.014677);

    PolynomialFitter polyFit(xs, ys, 3);
    double initialGuessParams[3];
    initialGuessParams[0] = 1.0;
    initialGuessParams[1] = 1.0;
    initialGuessParams[2] = 1.0;
    polyFit.setParameters(initialGuessParams);
    polyFit.fit(500, true);
    double solution[3];
    polyFit.getParameters(solution);
    double errors[3];
    polyFit.getAsymptoticStandardError(errors);
    fprintf(stderr, "A = %f +/- %f\n", solution[2], errors[2]);
    fprintf(stderr, "B = %f +/- %f\n", solution[1], errors[1]);
    fprintf(stderr, "C = %f +/- %f\n", solution[0], errors[0]);
    MatrixXd corr = polyFit.getParameterCorrelation();
    fprintf(stderr, "Parameter correlation:\n");
    fprintf(stderr, "%f\t%f\t%f\n", corr(0,0), corr(0,1), corr(0,2));
    fprintf(stderr, "%f\t%f\t%f\n", corr(1,0), corr(1,1), corr(1,2));
    fprintf(stderr, "%f\t%f\t%f\n", corr(2,0), corr(2,1), corr(2,2));


    // Print the data and model
    double model[xs.size()];
    polyFit.getModel(solution, model);
    double initialModel[xs.size()];
    polyFit.getModel(initialGuessParams, initialModel);
    for(unsigned int i=0; i<xs.size(); i++) {
        fprintf(stderr, "%f\t%f\t%f\t%f\n", xs[i], ys[i], model[i], initialModel[i]);
    }

}

/**
 * @brief Tests the transformation of right ascension and declination to azimuth and elevation.
 */
void TestUtil::testRaDecAzElConversion() {

    // Coordinates of Delta Ursae Majoris, one of the stars in The Plough
    double ra = MathUtil::toRadians(183.8583);
    double dec = MathUtil::toRadians(57.0325);

    // Coordinates of Deneb
//    double ra = MathUtil::toRadians(310.3583);
//    double dec = MathUtil::toRadians(45.2803);

    // The Greenwich Mean Sidereal Time [decimal hours]
    struct timeval epochtime;
    gettimeofday(&epochtime, NULL);
    long long epoch_us =  epochtime.tv_sec * 1000000LL  + (long long) round( epochtime.tv_usec);

    double gmst = TimeUtil::epochToGmst(epoch_us);

    // Longitude & latitude of observing site [approx. Abbeyhill]
    double lon = MathUtil::toRadians(-3.172414);
    double lat = MathUtil::toRadians(55.956325);

    // Convert ra, dec to az, el using single formula:
    double az, el;
    double lst = TimeUtil::gmstToLst(gmst, lon);
    CoordinateUtil::raDecToAzEl(ra, dec, lat, lst, az, el);

    // Convert using full rotations:

    // Rotation matrices
    Matrix3d r_bcrf_ecef = CoordinateUtil::getBcrfToEcefRot(gmst);
    Matrix3d r_ecef_sez  = CoordinateUtil::getEcefToSezRot(lon, lat);
    // Full transformation
    Matrix3d r_bcrf_sez = r_ecef_sez * r_bcrf_ecef;

    // Unit vector towards star in original frame:
    Vector3d r_bcrf;
    CoordinateUtil::sphericalToCartesian(r_bcrf, 1.0, ra, dec);
    // Transform to SEZ frame:
    Vector3d r_sez = r_bcrf_sez * r_bcrf;
    // Get position in spherical coordinates
    double r, theta, phi;
    CoordinateUtil::cartesianToSpherical(r_sez, r, theta, phi);
    // Transform east-of-south angle to east-of-north for conventional azimuth
    CoordinateUtil::eastOfSouthToEastOfNorth(theta);

    fprintf(stderr, "Azimuth / Elevation by single formula    = %8.5f / %8.5f\n", MathUtil::toDegrees(az), MathUtil::toDegrees(el));
    fprintf(stderr, "Azimuth / Elevation by chained rotations = %8.5f / %8.5f\n", MathUtil::toDegrees(theta), MathUtil::toDegrees(phi));
}

/**
 * @brief Tests the functions to write & read Image<double> types to/from files.
 */
void TestUtil::testImagedReadWrite() {

    // Create a dummy image
    unsigned int width = 2u;
    unsigned int height = 2u;
    Imaged testIm(width, height);
    testIm.epochTimeUs = 12345ll;
    testIm.rawImage[0] = 5.4;
    testIm.rawImage[1] = 6.3;
    testIm.rawImage[2] = 3.14159265357989;
    testIm.rawImage[3] = -1.0;

    char filename [100];
    sprintf(filename, "/home/nrowell/Temp/imaged.pfm");

    std::ofstream out(filename);
    out << testIm;
    out.close();

    fprintf(stderr, "%f\t%f\t%f\t%f\n", testIm.rawImage[0], testIm.rawImage[1], testIm.rawImage[2], testIm.rawImage[3]);

    // Read in the same file
    Imaged testIm2 = Imaged();
    std::ifstream ifs(filename);
    ifs >> testIm2;
    ifs.close();

    fprintf(stderr, "Recovered size = %d x %d\n", testIm2.width, testIm2.height);
    fprintf(stderr, "Number of samples = %d\n", testIm2.rawImage.size());

    for(unsigned int i=0; i<testIm2.rawImage.size(); i++) {
        fprintf(stderr, "rawImage[%d] = %f\n", i, testIm2.rawImage[i]);
    }



}
