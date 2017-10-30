#ifndef METEORIMAGELOCATIONMEASUREMENT_H
#define METEORIMAGELOCATIONMEASUREMENT_H

#include <vector>
#include <fstream>

// include headers that implement a archive in XML format
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/serialization/vector.hpp>

/**
 * @brief This class encapsulates the results of several localisation algorithms applied
 * to a single image of a meteor from a video sequence.
 *
 * The precise sub-pixel location of the meteor centroid in the image is determined by applying several
 * algorithms of increasing sophistication. The centroid location in each image is the starting
 * point for the trajectory reconstruction.
 */
class MeteorImageLocationMeasurement
{
public:

    MeteorImageLocationMeasurement();

    /**
     * @brief Copy constructor for the MeteorImageLocationMeasurement.
     * @param copyme
     *  The MeteorImageLocationMeasurement to copy.
     */
    MeteorImageLocationMeasurement(const MeteorImageLocationMeasurement & copyme);

    /**
     * @brief Copy assignment operator for the MeteorImageLocationMeasurement.
     * @param copyassign
     *  The MeteorImageLocationMeasurement to copy.
     * @return
     *  A reference to this (newly copied) MeteorImageLocationMeasurement.
     */
    MeteorImageLocationMeasurement& operator=(const MeteorImageLocationMeasurement& copyassign);

    /**
     * @brief Function used to aid sorting of a vector into ascending order of capture time
     * using std::sort(vec.begin(), vec.end());
     *
     * @param loc
     *  The MeteorImageLocationMeasurement to compare to this one.
     * @return
     *  True if this MeteorImageLocationMeasurement was captured at an earlier time than the given one.
     */
    bool operator < (const MeteorImageLocationMeasurement& loc) const;

    /**
     * @brief Records the epoch time in microseconds of the location measurement, which is the same as the
     * epoch time of the image from which the location measurement was derived.
     */
    long long epochTimeUs;

    /**
     * @brief Indices of the pixels with a significant positive change between this image and the previous one.
     */
    std::vector<unsigned int> changedPixelsPositive;

    /**
     * @brief Indices of the pixels with a significant negative change between this image and the previous one.
     */
    std::vector<unsigned int> changedPixelsNegative;

    /**
     * @brief Positions of the edges of the bounding box that encloses the set of changed pixels for this image,
     * with respect to the earlier image. Outlier filtering is used so the bounding box may not contain
     * all of the changed pixels, but should provide a better localisation of the event trigger.
     */
    bool coarse_localisation_success;
    unsigned int bb_xmin;
    unsigned int bb_xmax;
    unsigned int bb_ymin;
    unsigned int bb_ymax;

    /**
     * @brief Coordinates of the centre of flux of the object.
     */
    double x_flux_centroid;
    double y_flux_centroid;

private:
    friend class boost::serialization::access;
    // When the class Archive corresponds to an output archive, the
    // & operator is defined similar to <<.  Likewise, when the class Archive
    // is a type of input archive the & operator is defined similar to >>.
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        // The use of the BOOST_SERIALIZATION_NVP macro is necessary for XML archives.
        ar & BOOST_SERIALIZATION_NVP(epochTimeUs);
        ar & BOOST_SERIALIZATION_NVP(changedPixelsPositive);
        ar & BOOST_SERIALIZATION_NVP(changedPixelsNegative);
        ar & BOOST_SERIALIZATION_NVP(coarse_localisation_success);
        ar & BOOST_SERIALIZATION_NVP(bb_xmin);
        ar & BOOST_SERIALIZATION_NVP(bb_xmax);
        ar & BOOST_SERIALIZATION_NVP(bb_ymin);
        ar & BOOST_SERIALIZATION_NVP(bb_ymax);
        ar & BOOST_SERIALIZATION_NVP(x_flux_centroid);
        ar & BOOST_SERIALIZATION_NVP(y_flux_centroid);
    }

};

#endif // METEORIMAGELOCATIONMEASUREMENT_H
