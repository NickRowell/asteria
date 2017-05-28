/*
 * Name:
 *   mat3x3.h
 *
 * Purpose:
 *   Represents a 3-by-3 matrix.
 *
 * Description:
 *   Used to represent, among other things, the intrinsic camera calibration
 *      matrix and known landmark position covariance matrices. These are
 *      fundamental to the IP algorithm. We mainly need to perform matrix 
 *      multiplications in order to transform and/or project quantities between
 *      coordinate frames and the projective frame of the camera. We also need
 *      to invert the camera matrix so we can perform back-projections, but we
 *      exploit the special structure of the camera matrix to hard-code the 
 *      inversion rather than endowing this class with a dedicated general
 *      purpose inverse function.
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

#ifndef MAT3X3_H
#define	MAT3X3_H

#include "math/vec3.h"

template < typename T >
class Mat3x3
{
    
private:
    // Matrix elements, in row-major order
    T els[9];
    
public:
    
    Mat3x3() {
        // Initialise all elements to zero
        T * fptr = els;
        *(fptr++) = 0; *(fptr++) = 0; *(fptr++) = 0;
        *(fptr++) = 0; *(fptr++) = 0; *(fptr++) = 0;
        *(fptr++) = 0; *(fptr++) = 0; *(fptr++) = 0;
    }
    
    Mat3x3(const T &x00, const T &x01, const T &x02,
           const T &x10, const T &x11, const T &x12,
           const T &x20, const T &x21, const T &x22) {
        // Initialise elements to specified values
        T * fptr = els;
        *(fptr++) = x00; *(fptr++) = x01; *(fptr++) = x02;
        *(fptr++) = x10; *(fptr++) = x11; *(fptr++) = x12;
        *(fptr++) = x20; *(fptr++) = x21; *(fptr++) = x22;
    }
    
    // Copy constructor
    Mat3x3(const Mat3x3 &copy)
    {
        for(unsigned int i=0; i<9; i++)
            els[i] = copy.els[i];
    }
    
    void init(const T &x00, const T &x01, const T &x02,
              const T &x10, const T &x11, const T &x12,
              const T &x20, const T &x21, const T &x22)
    {
        // Initialise elements to specified values
        T * fptr = els;
        *(fptr++) = x00; *(fptr++) = x01; *(fptr++) = x02;
        *(fptr++) = x10; *(fptr++) = x11; *(fptr++) = x12;
        *(fptr++) = x20; *(fptr++) = x21; *(fptr++) = x22;
    }
    
    
    T get(const int &r, const int &c) const {
        return els[r*3 + c];
    }
    
    T * getPtr() {
        return els;
    }

    /**
     * Get transpose of this Mat3x3. This is useful because OpenGL uses a
     * column-packed format for matrices.
     * @return 
     */
    Mat3x3<T> transpose() const
    {
        return Mat3x3<T>(els[0],els[3],els[6],
                         els[1],els[4],els[7],
                         els[2],els[5],els[8]);
    }

    /**
     * Right multiply this matrix with another matrix.
     * @param B
     * @return 
     */
    Mat3x3<T> operator*(const Mat3x3<T> &B) const {
        return Mat3x3<T>(
             els[0]*B.els[0] + els[1]*B.els[3] + els[2]*B.els[6],
             els[0]*B.els[1] + els[1]*B.els[4] + els[2]*B.els[7],
             els[0]*B.els[2] + els[1]*B.els[5] + els[2]*B.els[8],
             els[3]*B.els[0] + els[4]*B.els[3] + els[5]*B.els[6],
             els[3]*B.els[1] + els[4]*B.els[4] + els[5]*B.els[7],
             els[3]*B.els[2] + els[4]*B.els[5] + els[5]*B.els[8],
             els[6]*B.els[0] + els[7]*B.els[3] + els[8]*B.els[6],
             els[6]*B.els[1] + els[7]*B.els[4] + els[8]*B.els[7],
             els[6]*B.els[2] + els[7]*B.els[5] + els[8]*B.els[8]
             );
    }

    /**
     * Right multiply this matrix in place with another matrix.
     * @param B
     * @return 
     */
    void operator*=(const Mat3x3<T> &B)
    {
        T x00 = els[0]*B.els[0] + els[1]*B.els[3] + els[2]*B.els[6];
        T x01 = els[0]*B.els[1] + els[1]*B.els[4] + els[2]*B.els[7];
        T x02 = els[0]*B.els[2] + els[1]*B.els[5] + els[2]*B.els[8];
        T x10 = els[3]*B.els[0] + els[4]*B.els[3] + els[5]*B.els[6];
        T x11 = els[3]*B.els[1] + els[4]*B.els[4] + els[5]*B.els[7];
        T x12 = els[3]*B.els[2] + els[4]*B.els[5] + els[5]*B.els[8];
        T x20 = els[6]*B.els[0] + els[7]*B.els[3] + els[8]*B.els[6];
        T x21 = els[6]*B.els[1] + els[7]*B.els[4] + els[8]*B.els[7];
        T x22 = els[6]*B.els[2] + els[7]*B.els[5] + els[8]*B.els[8];

        T * fptr = els;

        *(fptr++) = x00;
        *(fptr++) = x01;
        *(fptr++) = x02;
        *(fptr++) = x10;
        *(fptr++) = x11;
        *(fptr++) = x12;
        *(fptr++) = x20;
        *(fptr++) = x21;
        *(fptr++) = x22;
    }
    
    
    /**
     * Right multiply this matrix with a 3-vector.
     * @param B
     * @return 
     */
    Vec3<T> operator*(const Vec3<T> &B) const
    {
        return Vec3<T>( els[0]*B.x + els[1]*B.y + els[2]*B.z,
                        els[3]*B.x + els[4]*B.y + els[5]*B.z,
                        els[6]*B.x + els[7]*B.y + els[8]*B.z);
    }
    
    friend std::ostream& operator<<(std::ostream& os, const Mat3x3& mat)
    {
        os << mat.els[0] << ' ' << mat.els[1] << ' ' << mat.els[2] << '\n'
           << mat.els[3] << ' ' << mat.els[4] << ' ' << mat.els[5] << '\n'
           << mat.els[6] << ' ' << mat.els[7] << ' ' << mat.els[8] << '\n';
        
        // Return a reference to the original ostream so that we can chain
        // stream insertion operations.
        return os;
    }
    
    /**
     * Utility function used to create an approximate pinhole camera matrix from
     * the more human-readable parameters of field of view, projection centre
     * and image size.
     * 
     * Note that only the horizontal field of view is specified. This is used to
     * compute the horizontal focal length. The vertical focal length is
     * assigned the same value, which enforces square pixels. Non-square pixels
     * are achieved by having a different focal length along each image axis. 
     * This function does not provide this currently. If the vertical field of
     * view were to be specified as a parameter, we would also need to know the
     * pixel aspect ratio in order to compute the appropriate focal lengths.
     * 
     * Also: the focal length is calculated with the principal point in the
     * image centre. If the specified principal point does no lie in the image
     * centre, then the computed pinhole camera matrix will still be perfectly
     * valid but will not result in images with the specified FOV.
     * 
     * Note that FOV is not a good way to specify the camera parameters, and
     * focal length is more appropriate. This is because if the principal point
     * lies outside of the image area, there are zero, one or two possible 
     * focal length solutions for a given FOV.
     * 
     * 
     * @param fovx      Horizontal FOV [degrees]
     * @param width     Image width [pixels]
     * @param px        Horizontal principal point coordinate [pixels]
     * @param py        Vertical principal point coordinate [pixels]
     * @return
     */
    static 
    Mat3x3<T> 
    getPinholeCameraMatrix(const T & fovx, 
                           const unsigned int & width,
                           const T & px, const T & py)
    {
        // Focal length in horizontal direction [pixels]
        T fx = ((T) (width / 2))  * (1.0 / tan(fovx * (M_PI/180) / 2.0));
        // Enforce square pixels
        T fy = fx;
        // Projection matrix
        return Mat3x3<T>(fx,0,px,0,fy,py,0,0,1);
    }

};

#endif	/* MAT3X3_H */
