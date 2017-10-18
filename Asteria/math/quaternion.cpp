#include "quaternion.h"

/**
 * NOTE: Not actually used in the project, since I replaced with the Eigen library Quaternion class.
 */

/**
 * @brief Constructor for the identity Quaternion.
 */
Quaternion::Quaternion() : q0(1.0), q1(0.0), q2(0.0), q3(0.0) {

}

/**
 * @brief Copy constructor for the Quaternion.
 * @param copyme
 *  The Quaternion to copy.
 */
Quaternion::Quaternion(const Quaternion &copyme) : q0(copyme.q0), q1(copyme.q1), q2(copyme.q2), q3(copyme.q3) {

}

/**
 * @brief Constructor for the Quaternion setting each of the elements individually.
 *
 * @param q0
 *  The first quaternion element, corresponding to the real part.
 * @param q1
 *  The second quaternion element, corresponding to the quaternion unit i.
 * @param q2
 *  The third quaternion element, corresponding to the quaternion unit j.
 * @param q3
 *  The fourth quaternion element, corresponding to the quaternion unit k.
 */
Quaternion::Quaternion(const double &q0, const double &q1, const double &q2, const double &q3) : q0(q0), q1(q1), q2(q2), q3(q3) {

}

/**
 * @brief Construct a unit Quaternion from the axis-angle representation of a spatial rotation in three dimensions.
 *
 * @param axis
 *  Rotation axis [unit vector]
 * @param angle
 *  Rotation angle [radians]
 */
Quaternion::Quaternion(const Eigen::Vector3d &axis, const double &angle) {
    q0 = std::cos(angle/2.0);
    double sinAng2 = std::sin(angle/2.0);
    q1 = axis[0] * sinAng2;
    q2 = axis[1] * sinAng2;
    q3 = axis[2] * sinAng2;
}

/**
 * @brief Quaternion multiplication.
 * @param b
 *  The Quaternion to multiply with.
 * @return
 *  The result of multiplying this Quaternion by the given one.
 */
Quaternion Quaternion::operator*(const Quaternion & b) const {
    double t0 = b.q0*q0 - b.q1*q1 - b.q2*q2 - b.q3*q3;
    double t1 = b.q0*q1 + b.q1*q0 - b.q2*q3 + b.q3*q2;
    double t2 = b.q0*q2 + b.q1*q3 + b.q2*q0 - b.q3*q1;
    double t3 = b.q0*q3 - b.q1*q2 + b.q2*q1 + b.q3*q0;
    return Quaternion(t0, t1, t2, t3);
}

/**
 * @brief In-place Quaternion multiplication.
 * @param b
 *  The Quaternion to multiply with.
 * @return
 *  The result of multiplying this Quaternion by the given one.
 */
Quaternion &Quaternion::operator*=(const Quaternion & b) {
    // Saves repeating the quaternion multiplication algebra
    Quaternion t = (*this)*b;
    q0 = t.q0;
    q1 = t.q1;
    q2 = t.q2;
    q3 = t.q3;
    return *this;
}

/**
 * @brief Scalar multiplication.
 * @param b
 *  The double value to multiply this Quaternion by.
 * @return
 *  The result of multiplying this Quaternion by the double.
 */
Quaternion Quaternion::operator*(const double & b) const {
    return Quaternion(q0*b, q1*b, q2*b, q3*b);
}

/**
 * @brief In-place scalar multiplication.
 * @param b
 *  The double value to multiply this Quaternion by.
 * @return
 *  The result of multiplying this Quaternion by the double.
 */
Quaternion &Quaternion::operator*=(const double & b) {
    q0*=b;
    q1*=b;
    q2*=b;
    q3*=b;
    return *this;
}

/**
 * @brief The equality assignment operator.
 * @param A
 *  The Quaternion to set this one equal to.
 * @return
 *  A reference to this Quaternion.
 */
Quaternion & Quaternion::operator=(const Quaternion & b) {
    this->q0 = b.q0;
    this->q1 = b.q1;
    this->q2 = b.q2;
    this->q3 = b.q3;
    return *this;
}

/**
 * @brief In-place Quaternion normalisation.
 * @return
 *  A reference to this Quaternion, following normalisation.
 */
Quaternion & Quaternion::normalise() {
    (*this) *= (1.0 / norm());
    return *this;
}

/**
 * @brief Compute the Quaternion magnitude.
 * @return
 *  The Quaternion magnitude.
 */
double Quaternion::norm() const {
    return std::sqrt(norm2());
}

/**
 * @brief Compute the Quaternion magnitude squared.
 * @return
 *  The Quaternion magnitude squared.
 */
double Quaternion::norm2() const {
    return (q0*q0 + q1*q1 + q2*q2 + q3*q3);
}

/**
 * @brief Compute the inverse Quaternion.
 *
 * @return
 *  The inverse Quaternion.
 */
Quaternion Quaternion::inverse() const {
    return (Quaternion(q0, -q1, -q2, -q3) * (1.0/norm2()));
}

/**
 * @brief Checks if this represents a unit Quaternion.
 * @return
 *  True if the Quaternion has a length of 1.0
 */
bool Quaternion::isUnitQuaternion() const {
    double UNIT_THRESHOLD = 1E-9;

    return std::abs(1.0 - norm()) < UNIT_THRESHOLD;
}

/**
 * @brief Rotates the given vector by the quaternion.
 *
 * @param v
 *  The vector to rotate
 * @return
 *  The rotated vector
 */
Eigen::Vector3d Quaternion::rotate(const Eigen::Vector3d & v) const {

    // Represent vector as a quaternion with zero real component
    Quaternion qv(0, v[0], v[1], v[2]);

    // Rotation by quaternion multiplication
    Quaternion qv_rot = (*this)*qv*inverse();

    // Discard real component to get the rotated vector
    Eigen::Vector3d v_rot(qv_rot.q1, qv_rot.q2, qv_rot.q3);
    return v_rot;
}

/**
 * @brief Converts a unit quaternion to the equivalent orthonormal rotation
 * matrix.
 *
 * @return
 *  An orthonormal rotation matrix representing the spatial rotation defined
 * by the unit quaternion.
 */
Eigen::Matrix3d Quaternion::toMat() const {

    Eigen::Matrix3d r;

    r << 1 - 2*q2*q2 - 2*q3*q3,     2*q1*q2 - 2*q3*q0,     2*q1*q3 + 2*q2*q0,
             2*q2*q1 + 2*q3*q0, 1 - 2*q1*q1 - 2*q3*q3,     2*q2*q3 - 2*q1*q0,
             2*q1*q3 - 2*q2*q0,     2*q1*q0 + 2*q3*q2, 1 - 2*q1*q1 - 2*q2*q2;

    return r;
}



