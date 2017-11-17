#ifndef MATHUTIL_H
#define MATHUTIL_H

#include <vector>
#include <cmath>

class MathUtil
{
public:
    MathUtil();

    /**
     * @brief Get the median of the values stored in the vector.
     * @param values
     *  The values. On exit, this will be sorted into ascending order.
     * @return
     *  The median of the values stored in the vector.
     */
    static unsigned char getMedian(std::vector<unsigned char> &values);

    /**
     * @brief Get the median of the values stored in the vector.
     * @param values
     *  The values. On exit, this will be sorted into ascending order.
     * @return
     *  The median of the values stored in the vector.
     */
    static double getMedian(std::vector<double> &values);

    static void getMedianMad(std::vector<unsigned int> values, int &median, int &mad);

   /**
    * @brief Computes the 'trimmed mean' and 'trimmed standard deviation' values of the vector of doubles, i.e. the mean and
    * sample standard deviation after removing a specified fraction of the highest and lowest values.
    * @param values
    *   The vector of values to process.
    * @param mean
    *   On exit, contains the trimmed mean value.
    * @param std
    *   On exit, contains the trimmed sample standard deviation value.
    * @param percentile
    *   The fraction of the lowest and highest values to remove; must lie in the [0.0:0.5] range, values around
    * 0.05 are probably ideal (5% outliers at each end of range).
    */
   static void getTrimmedMeanStd(std::vector<double> values, double &mean, double &std, const double percentile);

   static double toDegrees(const double &rad);
   static double toRadians(const double &deg);
};

#endif // MATHUTIL_H
