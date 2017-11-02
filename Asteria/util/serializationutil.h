#ifndef SERIALIZATIONUTIL_H
#define SERIALIZATIONUTIL_H

#include "infra/source.h"

#include <Eigen/Dense>

//#include <boost/archive/xml_oarchive.hpp>
//#include <boost/archive/xml_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/serialization.hpp>

/**
 * Provides non-intrusive Boost serialization support for library classes.
 */

/**
 * See https://stackoverflow.com/questions/18382457/eigen-and-boostserialize
 */
namespace boost {
    namespace serialization {

        template<class Archive>
        inline void save(Archive & ar, const Eigen::Quaterniond & g, const unsigned int version) {
            double w = g.w();
            double x = g.x();
            double y = g.y();
            double z = g.z();

            ar & BOOST_SERIALIZATION_NVP(w);
            ar & BOOST_SERIALIZATION_NVP(x);
            ar & BOOST_SERIALIZATION_NVP(y);
            ar & BOOST_SERIALIZATION_NVP(z);
        }

        template<class Archive>
        inline void load(Archive & ar, Eigen::Quaterniond & g, const unsigned int version) {
            double w, x, y, z;
            ar & BOOST_SERIALIZATION_NVP(w);
            ar & BOOST_SERIALIZATION_NVP(x);
            ar & BOOST_SERIALIZATION_NVP(y);
            ar & BOOST_SERIALIZATION_NVP(z);

            g.w() = w;
            g.x() = x;
            g.y() = y;
            g.z() = z;
        }

        template<class Archive>
        inline void serialize(Archive & ar, Eigen::Quaterniond & g, const unsigned int version) {
            split_free(ar, g, version);
        }

        template<class Archive>
        void serialize(Archive & ar, Source & s, const unsigned int version)
        {
            // The use of the BOOST_SERIALIZATION_NVP macro is necessary for XML archives.
            ar & BOOST_SERIALIZATION_NVP(s.pixels);
            ar & BOOST_SERIALIZATION_NVP(s.adu);
            ar & BOOST_SERIALIZATION_NVP(s.sigma_adu);
            ar & BOOST_SERIALIZATION_NVP(s.x0);
            ar & BOOST_SERIALIZATION_NVP(s.y0);
            ar & BOOST_SERIALIZATION_NVP(s.c_xx);
            ar & BOOST_SERIALIZATION_NVP(s.c_xy);
            ar & BOOST_SERIALIZATION_NVP(s.c_yy);
            ar & BOOST_SERIALIZATION_NVP(s.l1);
            ar & BOOST_SERIALIZATION_NVP(s.l2);
            ar & BOOST_SERIALIZATION_NVP(s.orientation);
        }

    } // namespace serialization
} // namespace boost



#endif // SERIALIZATIONUTIL_H
