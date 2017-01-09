/*
 * Name:
 *   mat4x4.h
 *
 * Purpose:
 *   Represents a 4-by-4 matrix.
 *
 * Description:
 *   Used to represent GL model view matrix.
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

#ifndef MAT4X4_H
#define	MAT4X4_H

#include <iostream>

#include "../math/mat3x3.h"
#include "../math/vec4.h"

template < typename T >
class Mat4x4
{
    
private:
    // Matrix elements, in row-major order
    T els[16];
    
public:
    
    Mat4x4()
    {
        // Initialise all elements to zero
        T * fptr = els;
        *(fptr++) = 0; *(fptr++) = 0; *(fptr++) = 0; *(fptr++) = 0;
        *(fptr++) = 0; *(fptr++) = 0; *(fptr++) = 0; *(fptr++) = 0;
        *(fptr++) = 0; *(fptr++) = 0; *(fptr++) = 0; *(fptr++) = 0;
        *(fptr++) = 0; *(fptr++) = 0; *(fptr++) = 0; *(fptr++) = 0;
    }
    
    Mat4x4(const T &x00, const T &x01, const T &x02, const T &x03,
           const T &x10, const T &x11, const T &x12, const T &x13,
           const T &x20, const T &x21, const T &x22, const T &x23,
           const T &x30, const T &x31, const T &x32, const T &x33)
    {
        // Initialise element to specified values
        T * fptr = els;
        *(fptr++) = x00; *(fptr++) = x01; *(fptr++) = x02; *(fptr++) = x03;
        *(fptr++) = x10; *(fptr++) = x11; *(fptr++) = x12; *(fptr++) = x13;
        *(fptr++) = x20; *(fptr++) = x21; *(fptr++) = x22; *(fptr++) = x23;
        *(fptr++) = x30; *(fptr++) = x31; *(fptr++) = x32; *(fptr++) = x33;
    }
    
    // Copy constructor
    Mat4x4(const Mat4x4 &copy)
    {
        for(unsigned int i=0; i<16; i++)
            els[i] = copy.els[i];
    }
    
    
    
    T get(const int &r, const int &c) const { return els[r*4 + c];}
    
    T * getPtr(){ return els;}
    
    /**
     * Extract upper left 3x3 matrix, usually interpreted as a rotation matrix.
     * @return 
     */
    Mat3x3<T> getR()
    {
        return Mat3x3<T>(els[0],els[1],els[2],
                         els[4],els[5],els[6],
                         els[8],els[9],els[10]);
    }
    
    /**
     * Right multiply this matrix with another matrix.
     * @param B
     * @return 
     */
    Mat4x4<T> operator*(const Mat4x4<T> &B) const
    {
        return Mat4x4<T>(
             els[0]*B.els[0] + els[1]*B.els[4] + els[2]*B.els[8]  + els[3]*B.els[12],
             els[0]*B.els[1] + els[1]*B.els[5] + els[2]*B.els[9]  + els[3]*B.els[13],
             els[0]*B.els[2] + els[1]*B.els[6] + els[2]*B.els[10] + els[3]*B.els[14],
             els[0]*B.els[3] + els[1]*B.els[7] + els[2]*B.els[11] + els[3]*B.els[15],
             
             els[4]*B.els[0] + els[5]*B.els[4] + els[6]*B.els[8]  + els[7]*B.els[12],
             els[4]*B.els[1] + els[5]*B.els[5] + els[6]*B.els[9]  + els[7]*B.els[13],
             els[4]*B.els[2] + els[5]*B.els[6] + els[6]*B.els[10] + els[7]*B.els[14],
             els[4]*B.els[3] + els[5]*B.els[7] + els[6]*B.els[11] + els[7]*B.els[15],
             
             els[8]*B.els[0] + els[9]*B.els[4] + els[10]*B.els[8]  + els[11]*B.els[12],
             els[8]*B.els[1] + els[9]*B.els[5] + els[10]*B.els[9]  + els[11]*B.els[13],
             els[8]*B.els[2] + els[9]*B.els[6] + els[10]*B.els[10] + els[11]*B.els[14],
             els[8]*B.els[3] + els[9]*B.els[7] + els[10]*B.els[11] + els[11]*B.els[15],
                
             els[12]*B.els[0] + els[13]*B.els[4] + els[14]*B.els[8]  + els[15]*B.els[12],
             els[12]*B.els[1] + els[13]*B.els[5] + els[14]*B.els[9]  + els[15]*B.els[13],
             els[12]*B.els[2] + els[13]*B.els[6] + els[14]*B.els[10] + els[15]*B.els[14],
             els[12]*B.els[3] + els[13]*B.els[7] + els[14]*B.els[11] + els[15]*B.els[15]);
    }

    /**
     * Right multiply this matrix in place with another matrix.
     * @param B
     * @return 
     */
    void operator*=(const Mat4x4<T> &B)
    {
        T x00 = els[0]*B.els[0] + els[1]*B.els[4] + els[2]*B.els[8]  + els[3]*B.els[12];
        T x01 = els[0]*B.els[1] + els[1]*B.els[5] + els[2]*B.els[9]  + els[3]*B.els[13];
        T x02 = els[0]*B.els[2] + els[1]*B.els[6] + els[2]*B.els[10] + els[3]*B.els[14];
        T x03 = els[0]*B.els[3] + els[1]*B.els[7] + els[2]*B.els[11] + els[3]*B.els[15];
        
        T x10 = els[4]*B.els[0] + els[5]*B.els[4] + els[6]*B.els[8]  + els[7]*B.els[12];
        T x11 = els[4]*B.els[1] + els[5]*B.els[5] + els[6]*B.els[9]  + els[7]*B.els[13];
        T x12 = els[4]*B.els[2] + els[5]*B.els[6] + els[6]*B.els[10] + els[7]*B.els[14];
        T x13 = els[4]*B.els[3] + els[5]*B.els[7] + els[6]*B.els[11] + els[7]*B.els[15];
        
        T x20 = els[8]*B.els[0] + els[9]*B.els[4] + els[10]*B.els[8]  + els[11]*B.els[12];
        T x21 = els[8]*B.els[1] + els[9]*B.els[5] + els[10]*B.els[9]  + els[11]*B.els[13];
        T x22 = els[8]*B.els[2] + els[9]*B.els[6] + els[10]*B.els[10] + els[11]*B.els[14];
        T x23 = els[8]*B.els[3] + els[9]*B.els[7] + els[10]*B.els[11] + els[11]*B.els[15];
        
        T x30 = els[12]*B.els[0] + els[13]*B.els[4] + els[14]*B.els[8]  + els[15]*B.els[12];
        T x31 = els[12]*B.els[1] + els[13]*B.els[5] + els[14]*B.els[9]  + els[15]*B.els[13];
        T x32 = els[12]*B.els[2] + els[13]*B.els[6] + els[14]*B.els[10] + els[15]*B.els[14];
        T x33 = els[12]*B.els[3] + els[13]*B.els[7] + els[14]*B.els[11] + els[15]*B.els[15];
        
        T * fptr = els;
        
        *(fptr++) = x00; *(fptr++) = x01; *(fptr++) = x02; *(fptr++) = x03;
        *(fptr++) = x10; *(fptr++) = x11; *(fptr++) = x12; *(fptr++) = x13;
        *(fptr++) = x20; *(fptr++) = x21; *(fptr++) = x22; *(fptr++) = x23;
        *(fptr++) = x30; *(fptr++) = x31; *(fptr++) = x32; *(fptr++) = x33;
    }
    
    Mat4x4& transposeInPlace()
    {
        // Row-major matrix has this layout:
        //
        // els[0]  els[1]  els[2]  els[3]
        // els[4]  els[5]  els[6]  els[7]
        // els[8]  els[9]  els[10] els[11]
        // els[12] els[13] els[14] els[15]
        //
        // We rearrange it into this format:
        //
        // els[0]  els[4]  els[8]   els[12]
        // els[1]  els[5]  els[9]   els[13]
        // els[2]  els[6]  els[10]  els[14]
        // els[3]  els[7]  els[11]  els[15]
        
        T temp;
        temp = els[1];  els[1]  = els[4];  els[4]  = temp;  // Swap 1 & 4
        temp = els[2];  els[2]  = els[8];  els[8]  = temp;  // Swap 2 & 8
        temp = els[3];  els[3]  = els[12]; els[12] = temp;  // Swap 3 & 12
        temp = els[6];  els[6]  = els[9];  els[9]  = temp;  // Swap 6 & 9
        temp = els[7];  els[7]  = els[13]; els[13] = temp;  // Swap 7 & 13
        temp = els[11]; els[11] = els[14]; els[14] = temp;  // Swap 11 & 14
        
        return *this;
    }
    
    
    Mat4x4 transpose()
    {
        return (Mat4x4<T>(*this)).transposeInPlace();
    }
    
    
    /**
     * Get the matrix that performs the inverse transformation on 4-vectors.
     * e.g. we can transform the coordinates of vertices from the external frame
     * to the eye frame like so:
     * 
     * position_eye = glVMatrix * position_external
     * 
     * and transform back the way using this inverse matrix:
     * 
     * position_external = glVMatrix.inverse() * position_eye
     * 
     * @param B
     * @return 
     */
    Mat4x4 inverse()
    {
        // Extract rotation part of matrix
        Mat3x3<T> Rt = getR().transpose();
        
        // Extract translation part of matrix
        Vec3<T> t(els[3], els[7], els[11]);
        
        // Inverse translation, negated
        Vec3<T> RT = Rt * t;
        
        // Build Mat4x4
        T * elsR = Rt.getPtr();
        
        return Mat4x4<T>(elsR[0], elsR[1], elsR[2], -RT.x,
                         elsR[3], elsR[4], elsR[5], -RT.y,
                         elsR[6], elsR[7], elsR[8], -RT.z,
                             0.0,     0.0,     0.0,   1.0);
        
        
        
    }
    
    
    /**
     * Right multiply this matrix with a 4-vector.
     * @param B
     * @return
     */
    Vec4<T> operator*(const Vec4<T> &B) const
    {
        return Vec4<T>( els[0]*B.x  + els[1]*B.y  + els[2]*B.z  + els[3]*B.w,
                        els[4]*B.x  + els[5]*B.y  + els[6]*B.z  + els[7]*B.w,
                        els[8]*B.x  + els[9]*B.y  + els[10]*B.z + els[11]*B.w,
                        els[12]*B.x + els[13]*B.y + els[14]*B.z + els[15]*B.w);
    }
    
    /**
     * Right multiply this matrix with a 3-vector. The missing 4th component
     * is implicitly assigned a value of 1.0.
     * @param B
     * @return
     */
    Vec4<T> operator*(const Vec3<T> &B) const
    {
        return Vec4<T>( els[0]*B.x  + els[1]*B.y  + els[2]*B.z  + els[3],
                        els[4]*B.x  + els[5]*B.y  + els[6]*B.z  + els[7],
                        els[8]*B.x  + els[9]*B.y  + els[10]*B.z + els[11],
                        els[12]*B.x + els[13]*B.y + els[14]*B.z + els[15]);
    }
    
    
    
    
    
    friend std::ostream& operator<<(std::ostream& os, const Mat4x4& mat)
    {
        os << mat.els[0]  << ' ' << mat.els[1]  << ' ' << mat.els[2]  << ' ' << mat.els[3]  << ' ' << '\n'
           << mat.els[4]  << ' ' << mat.els[5]  << ' ' << mat.els[6]  << ' ' << mat.els[7]  << ' ' << '\n'
           << mat.els[8]  << ' ' << mat.els[9]  << ' ' << mat.els[10] << ' ' << mat.els[11] << ' ' << '\n'
           << mat.els[12] << ' ' << mat.els[13] << ' ' << mat.els[14] << ' ' << mat.els[15] << ' ' << '\n';
        
        // Return a reference to the original ostream so that we can chain
        // stream insertion operations.
        return os;
    }
    
};



#endif	/* MAT3X3_H */
