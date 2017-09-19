#include "coordinateutil.h"

CoordinateUtil::CoordinateUtil()
{

}

/**
 * @brief CoordinateUtil::raDecToAzEl
 *
 * Converts celestial Right Ascension and Declination to Azimuth and Elevation.
 *
 * @param ra
 *  The right ascension [degrees]
 * @param dec
 *  The declination [degrees]
 * @param lat
 *  The latitude of the observing site [degrees]
 * @param lst
 *  The Local Sidereal Time [decimal hours]
 * @param az
 *  On exit, contains the azimuth [degrees]
 * @param el
 *  On exit, contains the elevation [degrees]
 */
void CoordinateUtil::raDecToAzEl(const double &ra, const double &dec, const double &lat, const double &lst, double &az, double &el) {

}
