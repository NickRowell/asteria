#ifndef SERIALIZATIONUTIL_H
#define SERIALIZATIONUTIL_H

#include "infra/source.h"
#include "infra/meteorimagelocationmeasurement.h"

#include <Eigen/Dense>

#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/export.hpp>

BOOST_CLASS_IMPLEMENTATION(std::vector<MeteorImageLocationMeasurement>, boost::serialization::object_serializable)
BOOST_CLASS_IMPLEMENTATION(MeteorImageLocationMeasurement, boost::serialization::object_serializable)

/**
 * Provides non-intrusive Boost serialization support for various classes. A few notes:
 *
 * 1) The use of the BOOST_SERIALIZATION_NVP macro is necessary for XML archives.
 * 2) In the case of classes for which we don't have direct access to the internal fields that
 *    we need to serialize (only via getters/setters), the serialization & deserialization need
 *    to be implemented separately as is done below for the Eigen::Quaterniond type. For further
 *    information, see https://stackoverflow.com/questions/18382457/eigen-and-boostserialize
 *
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

            // This is how we serialize types for which we can only access the internal fields through
            // getters/setters. For further information specific to the Eigen classes, see:
            // https://stackoverflow.com/questions/18382457/eigen-and-boostserialize

            split_free(ar, g, version);
        }

        template<class Archive>
        void serialize(Archive & ar, Source & s, const unsigned int version) {

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

        template<class Archive>
        void serialize(Archive & ar, MeteorImageLocationMeasurement & g, const unsigned int version) {

            ar & BOOST_SERIALIZATION_NVP(g.epochTimeUs);
            ar & BOOST_SERIALIZATION_NVP(g.changedPixelsPositive);
            ar & BOOST_SERIALIZATION_NVP(g.changedPixelsNegative);
            ar & BOOST_SERIALIZATION_NVP(g.coarse_localisation_success);
            ar & BOOST_SERIALIZATION_NVP(g.bb_xmin);
            ar & BOOST_SERIALIZATION_NVP(g.bb_xmax);
            ar & BOOST_SERIALIZATION_NVP(g.bb_ymin);
            ar & BOOST_SERIALIZATION_NVP(g.bb_ymax);
            ar & BOOST_SERIALIZATION_NVP(g.x_flux_centroid);
            ar & BOOST_SERIALIZATION_NVP(g.y_flux_centroid);
        }

//        template<class Archive>
//        void serialize(Archive & ar, CameraModelBase & g, const unsigned int version) {

//            ar & BOOST_SERIALIZATION_NVP(g.width);
//            ar & BOOST_SERIALIZATION_NVP(g.height);
//        }

//        template<class Archive>
//        void serialize(Archive & ar, PinholeCamera & g, const unsigned int version) {

//            ar.template register_type<CameraModelBase>();
//            ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(CameraModelBase);

//            ar & BOOST_SERIALIZATION_NVP(g.fi);
//            ar & BOOST_SERIALIZATION_NVP(g.fj);
//            ar & BOOST_SERIALIZATION_NVP(g.pi);
//            ar & BOOST_SERIALIZATION_NVP(g.pj);
//        }

//        template<class Archive>
//        void serialize(Archive & ar, PinholeCameraWithRadialDistortion & g, const unsigned int version) {

//            ar & BOOST_SERIALIZATION_NVP(g.width);
//            ar & BOOST_SERIALIZATION_NVP(g.height);
//            ar & BOOST_SERIALIZATION_NVP(g.fi);
//            ar & BOOST_SERIALIZATION_NVP(g.fj);
//            ar & BOOST_SERIALIZATION_NVP(g.pi);
//            ar & BOOST_SERIALIZATION_NVP(g.pj);
//            ar & BOOST_SERIALIZATION_NVP(g.K0);
//            ar & BOOST_SERIALIZATION_NVP(g.K1);
//            ar & BOOST_SERIALIZATION_NVP(g.K2);
//            ar & BOOST_SERIALIZATION_NVP(g.K3);
//            ar & BOOST_SERIALIZATION_NVP(g.K4);
//        }







    } // namespace serialization
} // namespace boost



#endif // SERIALIZATIONUTIL_H
