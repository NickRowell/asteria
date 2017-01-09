/*
 * Name:
 *   vec4.h
 *
 * Purpose:
 *   Represents 4-vectors.
 *
 * Description:
 *   Manipulation of 4-vector objects is fundamental to graphics.
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

#ifndef VEC4_H
#define	VEC4_H

#include <math.h>       /* sqrt, cos */

template < typename T >
class Vec4
{
    
public:
    
    // Public for easy access.
    T x,y,z,w;
    
    // Constructors.
    Vec4() : x(0), y(0), z(0), w(0) { }
    Vec4(const T & px, const T & py, const T & pz, const T & pw) : x(px), y(py), z(pz), w(pw) { }
    
    // Overload all arithmetic operators
    Vec4<T>  operator*(const T &b) const { return Vec4<T>(x*b, y*b, z*b, w*b);}
    Vec4<T> &operator*=(const T &b){ x*=b; y*=b; z*=b; w*=b; return *this;}
    Vec4<T>  operator/(const T &b) const { return Vec4<T>(x/b, y/b, z/b, w/b);}
    Vec4<T> &operator/=(const T &b){ x/=b; y/=b; z/=b; w/=b; return *this;}
    Vec4<T>  operator-(const Vec4<T> &b) const { return Vec4<T>(x-b.x, y-b.y, z-b.z, w-b.w);}
    Vec4<T> &operator-=(const Vec4<T> &b){ x-=b.x; y-=b.y; z-=b.z; w-=b.w; return *this;}
    Vec4<T>  operator+(const Vec4<T> &b) const { return Vec4<T>(x+b.x, y+b.y, z+b.z, w+b.w);}
    Vec4<T> &operator+=(const Vec4<T> &b){ x+=b.x; y+=b.y; z+=b.z; w+=b.w; return *this;}
    
    /** Dot product */
    T operator*(const Vec4<T> &b) const { return x*b.x + y*b.y + z*b.z + w*b.w;}
    
    /** Cross product does not exist for 4-vectors. */
    
    
    /** Equality assignment operator */
    Vec4<T> &operator=(const Vec4<T> &rhs)
    {
        x = rhs.x;
        y = rhs.y;
        z = rhs.z;
        w = rhs.w;
        return *this;
    }
    
    /**
     * Get magnitude of this vector, squared.
     * @return 
     */
    T norm2() const { return x*x + y*y + z*z + w*w;}
    
    /**
     * Get magnitude of this vector.
     * @return 
     */
    T norm() const { return (T)sqrt(norm2());}
    Vec4<T> normalise() const { return (*this)/(norm());}
    
    friend std::ostream& operator<<(std::ostream& os, const Vec4& vec4)
    {
        os << vec4.x  << ' ' << vec4.y << ' ' << vec4.z  << ' ' << vec4.w  << '\n';
        
        // Return a reference to the original ostream so that we can chain
        // stream insertion operations.
        return os;
    }
    
};

#endif	/* VEC3_H */