#ifndef COORDINATEUTIL_H
#define COORDINATEUTIL_H

/**
 * @brief The CoordinateUtil class
 * Utilities for coordinate transformation etc.
 */
class CoordinateUtil
{
public:
    CoordinateUtil();

    static void raDecToAzEl(const double &ra, const double &dec, const double &lat, const double &lst, double &az, double &el);

};

#endif // COORDINATEUTIL_H
