/*
 * Name:
 *   quat.h
 *
 * Purpose:
 *   Represents unit quaternions.
 *
 * Description:
 *   Unit quaternions are used to parameterise 3D orientation.
 * 
 * Notes:
 *   
 * Language:
 *   ANSI C++
 *
 * Author:
 *   Nick Rowell (nickrowell@computing.dundee.ac.uk)
 *   {add_authors_here}
 *
 * History:
 *   {add_changes_here}
 *
 * Future work:
 *   {add_suggestions_here}
 *
 * Bugs:
 *   {add_new_bugs_here}
 */

#ifndef QUAT_H
#define	QUAT_H

#include <math.h>       /* sqrt */
#include "../math/mat3x3.h"
#include "../math/mat4x4.h"
#include "../math/vec3.h"

template <typename T>
class Quat
{
    
public:
    
    /** Quaternion elements; real part and imaginary vector */
    T re;          // q0
    Vec3<T>  im;   // q1 q2 q3
    
    /**
     * Main constructor taking quaternion elements as argument.
     * @param Q0   Real component
     * @param Q1   Imaginary i component
     * @param Q2   Imaginary j component
     * @param Q3   Imaginary k component
     */
    Quat(const T & pre=0, const T & pq1=0, const T & pq2=0, const T & pq3=0) : 
                re(pre), im(pq1,pq2,pq3) {}
    
    /**
     * Constructor setting real and imaginary parts explicitly.
     * @param real
     * @param imag
     */
    Quat(const T & pre, const Vec3<T> & pim) : re(pre), im(pim) {}
    
    
    /**
     * Axis-Angle constructor.
     * @param axis      Rotation axis [unit vector]
     * @param angle     Rotation angle [radians]
     */
    Quat(const Vec3<T> & axis, const T & angle)
    {
        re = cos(angle/2.0);
        im = axis*sin(angle/2.0);
    };
    
    
    
    /**
     * Get quaternion inverse, i.e. quaternion corresponding to reverse rotation
     * @return   (q0,-q1,-q2,-q3)
     */
    Quat<T> inverse() const
    {
        return Quat<T>(re, im*(-1.0));
    };
    
    /**
     * Rotate a vector using a quaternion
     * @param v  Vector to rotate
     * @return   Rotated vector
     */
    Vec3<T> rotate(const Vec3<T> & v) const
    {

        // Represent vector as a quaternion with zero real component
        Quat<T> V(0,v);

        // Now do rotation operation.
        Quat<T> V_prime = (*this)*V*inverse();

        // Convert rotated quaternion back to vector representation by 
        // discarding real component
        return V_prime.im;
    }
    
    
    
    /**
     * Non-static quaternion multiplication
     * @param A   Second quaternion
     * @return    C = this * A
     */
    Quat<T> operator*(const Quat<T> & A) const
    {
        Quat<T> mult(re*A.re - im*(A.im), A.im*(re) + im*(A.re) + (im^(A.im)));

        return mult;
    };
    
    /** Equality assignment operator. */
    Quat<T> & operator=(const Quat<T> & A)
    {
        re = A.re;
        im = A.im;
        return *this;
    };
    
    
    /**
     * Get quaternion magnitude. Normal usage should not change the 
     * normalisation of quaternions, but accumulation of floating point errors
     * over many operations may result in non-unity magnitude.
     * This can be checked intermittently if desired with this method.
     * @return sqrt(q0*q0 + q1*q1 + q2*q2 + q3*q3)
     */
    T norm() const { return (T)sqrt(re*re + im.norm2());};


    /** Normalise the Quaternion in place. */
    void normalise()
    {

        // Get current normalisation
        T qnorm = norm();

        re /= qnorm;
        im /= qnorm;
    };
    

    /**
     * Convert a normalised quaternion to an orthonormal rotation matrix.
     * 
     * @return
     */
    template < typename U >
    Mat3x3<U> toMat3x3() const
    {

        return Mat3x3<U>(1 - 2*im.y*im.y - 2*im.z*im.z,
                         2*im.x*im.y - 2*im.z*re,
                         2*im.x*im.z + 2*im.y*re,
                         2*im.x*im.y + 2*im.z*re,
                         1 - 2*im.x*im.x - 2*im.z*im.z,
                         2*im.y*im.z - 2*im.x*re,
                         2*im.x*im.z - 2*im.y*re,
                         2*im.y*im.z + 2*im.x*re,
                         1 - 2*im.x*im.x - 2*im.y*im.y);
    }
    
    /**
     * Convert to a 4x4 matrix that can be used to apply a rotation to a
     * model matrix.
     * @return 
     */
    template < typename U >
    Mat4x4<U> toMat4x4() const
    {

        return Mat4x4<U>(1 - 2*im.y*im.y - 2*im.z*im.z,
                         2*im.x*im.y - 2*im.z*re,
                         2*im.x*im.z + 2*im.y*re,
                         0,
                         2*im.x*im.y + 2*im.z*re,
                         1 - 2*im.x*im.x - 2*im.z*im.z,
                         2*im.y*im.z - 2*im.x*re,
                         0,
                         2*im.x*im.z - 2*im.y*re,
                         2*im.y*im.z + 2*im.x*re,
                         1 - 2*im.x*im.x - 2*im.y*im.y,
                         0,
                         0,
                         0,
                         0,
                         1);
    }
    
    /** Check if this is a unit quaternion. */
    bool isUnitQuaternion() const
    {
        
        /** Magnitude threshold that defines unit Quaternions. */
        float UNIT_THRESHOLD = 1E-9;
        
        return std::abs(1.0 - norm()) < UNIT_THRESHOLD ;
    }
    
};

#endif	/* QUAT_H */

