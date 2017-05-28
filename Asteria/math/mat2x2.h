/*
 * Name:
 *   mat2x2.h
 *
 * Purpose:
 *   Represents a 2-by-2 matrix.
 *
 * Description:
 *   Used to represent ray transfer matrices when modelling 
 * lens ghost images.
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

#ifndef MAT2X2_H
#define	MAT2X2_H

#include <iostream>

#include "math/vec2.h"

template < typename T >
class Mat2x2
{
    
private:
    // Matrix elements, in row-major order
    T els[4];
    
public:
    
    Mat2x2()
    {
        // Initialise to identity matrix
        T * fptr = els;
        *(fptr++) = 1; *(fptr++) = 0;
        *(fptr++) = 0; *(fptr++) = 1;
    }
    
    Mat2x2(const T &x00, const T &x01, 
           const T &x10, const T &x11)
    {
        // Initialise element to specified values
        T * fptr = els;
        *(fptr++)=x00; *(fptr++)=x01; 
        *(fptr++)=x10; *(fptr++)=x11;
    }
    
    // Copy constructor
    Mat2x2(const Mat2x2 &copy)
    {
        for(unsigned int i=0; i<4; i++)
            els[i] = copy.els[i];
    }
    
    
    
    T get(const int &r, const int &c) const { return els[r*2 + c];}
    
    T * getPtr(){ return els;}
    
    /** Equality assignment operator */
    Mat2x2<T> &operator=(const Mat2x2<T> &rhs)
    {
        els[0] = rhs.els[0];
        els[1] = rhs.els[1];
        els[2] = rhs.els[2];
        els[3] = rhs.els[3];
        return *this;
    }
    
    /**
     * Right multiply this matrix with another matrix.
     * @param B
     * @return 
     */
    Mat2x2<T> operator*(const Mat2x2<T> &B) const
    {
        return Mat2x2<T>(
                els[0]*B.els[0] + els[1]*B.els[2], els[0]*B.els[1] + els[1]*B.els[3],
                els[2]*B.els[0] + els[3]*B.els[2], els[2]*B.els[1] + els[3]*B.els[3]);
    }
    
    /**
     * Right multiply this matrix with a scalar.
     * @param B Scalar to multiply this matrix by.
     * @return 
     */
    Mat2x2<T> operator*(const T &B) const
    {
        return Mat2x2<T>(els[0]*B, els[1]*B, els[2]*B, els[3]*B);
    }
    
    
    /**
     * Right multiply this matrix with a 2-vector.
     * 
     * rp = mat2x2 * r
     * 
     * @return
     */
    Vec2<T> operator*(const Vec2<T> &r) const
    {
        return Vec2<T>(els[0]*r.x + els[1]*r.y, els[2]*r.x + els[3]*r.y);
    }

    /**
     * Right multiply this matrix in place with another matrix.
     * @param B
     * @return 
     */
    void operator*=(const Mat2x2<T> &B)
    {
        
        T x00 = els[0]*B.els[0] + els[1]*B.els[2];
        T x01 = els[0]*B.els[1] + els[1]*B.els[3];
        T x10 = els[2]*B.els[0] + els[3]*B.els[2];
        T x11 = els[2]*B.els[1] + els[3]*B.els[3];
        
        T * fptr = els;
        *(fptr++)=x00; *(fptr++)=x01;
        *(fptr++)=x10; *(fptr++)=x11;
    }
    
    Mat2x2& transposeInPlace()
    {
        // Row-major matrix has this layout:
        //
        // els[0]  els[1]
        // els[2]  els[3]
        //
        // We rearrange it into this format:
        //
        // els[0]  els[2]
        // els[1]  els[3]
        
        T temp;
        temp = els[1];  els[1]  = els[2];  els[2]  = temp;  // Swap 1 & 2
        
        return *this;
    }
    
    
    Mat2x2 transpose()
    {
        return (Mat2x2<T>(*this)).transposeInPlace();
    }
    
    /**
     * Calculate the inverse of this matrix. For a 2x2 matrix
     * this is quite simple:
     * 
     * 
     * A = [ a b ]
     *     [ c d ]
     * 
     * A^{-1} =  {__1__}  [  d -b ]
     *           {ad-bc}  [ -c  a ]
     *          
     * 
     * 
     * @return 
     */
    Mat2x2 inverse() const
    {
        T det = els[0]*els[3] - els[1]*els[2];
        
        // Check that inverse exists:
        if(det==0)
        {
            std::cerr << "Error: Mat2x2 cannot be inverted!\n";
            return Mat2x2<T>();
        }
        
        Mat2x2<T> inv(els[3], -els[1], -els[2], els[0]);
        
        return inv * (1.0/det);
        
    }
    
    
    /**
     * Create a ray transfer matrix representing translation.
     * @param d Translation distance
     * @return 
     */
    static Mat2x2 getTranslation(const float &d)
    {
        return Mat2x2(1, d, 0, 1);
    }
    
    /**
     * Create a ray transfer matrix representing refraction at
     * a spherical interface between materials of refractive
     * index n1 and n2 respectively.
     * @param R         Radius of curvature
     * @param n1        Refractive index of medium 1
     * @param n2        Refractive index of medium 2
     * @return 
     */
    static Mat2x2 getRefraction(const float &R, const float &n1, const float &n2)
    {
        return Mat2x2(1, 0, (n1-n2)/(n2*R), n1/n2);
    }
    
    /**
     * Create a ray transfer matrix representing reflection at
     * a spherical surface of radius of curvature R.
     * @param R         Radius of curvature
     * @return 
     */
    static Mat2x2 getReflection(const float &R)
    {
        return Mat2x2(1, 0, 2/R, 1);
    }
    
    friend std::ostream& operator<<(std::ostream& os, const Mat2x2& mat)
    {
        os << mat.els[0]  << ' ' << mat.els[1]  << '\n'
           << mat.els[2]  << ' ' << mat.els[3]  << '\n';
        
        // Return a reference to the original ostream so that we can chain
        // stream insertion operations.
        return os;
    }
    
};



#endif	/* MAT3X3_H */
