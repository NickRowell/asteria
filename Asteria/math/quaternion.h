#ifndef QUATERNION_H
#define QUATERNION_H

#include <Eigen/Dense>

/**
 * @brief NOTE: Not actually used in the project, since I replaced with the Eigen library Quaternion class.
 */
class Quaternion {

public:

    Quaternion();
    Quaternion(const Quaternion &copyme);
    Quaternion(const double &q0, const double &q1, const double &q2, const double &q3);
    Quaternion(const Eigen::Vector3d &axis, const double &angle);

    /**
     * @brief The first quaternion element, corresponding to the real part.
     */
    double q0;
    /**
     * @brief The second quaternion element, corresponding to the quaternion unit i.
     */
    double q1;
    /**
     * @brief The third quaternion element, corresponding to the quaternion unit j.
     */
    double q2;
    /**
     * @brief The fourth quaternion element, corresponding to the quaternion unit k.
     */
    double q3;

    Quaternion operator*(const Quaternion & b) const;

    Quaternion &operator*=(const Quaternion & b);

    Quaternion operator*(const double & b) const;

    Quaternion &operator*=(const double & b);

    Quaternion &operator=(const Quaternion & b);

    Quaternion &normalise();

    double norm() const;

    double norm2() const;

    Quaternion inverse() const;

    bool isUnitQuaternion() const;

    Eigen::Vector3d rotate(const Eigen::Vector3d & v) const;

    Eigen::Matrix3d toMat() const;

private:



};

#endif // QUATERNION_H
