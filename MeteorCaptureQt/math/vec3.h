/*
 * Name:
 *   vec3.h
 *
 * Purpose:
 *   Represents 3-vectors.
 *
 * Description:
 *   Manipulation of 3-vector objects is fundamental to graphics.
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

#ifndef VEC3_H
#define	VEC3_H

#include <math.h>       /* sqrt, cos */

template < typename T >
class Vec3
{
    // Defines tolerance on detection of parallel vectors
    static const constexpr double PARALLEL_THRESHOLD = 0.0001 * (M_PI/180.0);
    
public:
    
    // Public for easy access.
    T x,y,z;
    
    // Constructors.
    Vec3() : x(0), y(0), z(0) { }
    Vec3(const T & px, const T & py, const T & pz) : x(px), y(py), z(pz) { }
    Vec3(const Vec3 &copy) : x(copy.x), y(copy.y), z(copy.z) { }
    
    // Overload all arithmetic operators
    Vec3<T>  operator*(const T &b) const { return Vec3<T>(x*b, y*b, z*b);}
    Vec3<T> &operator*=(const T &b){ x *= b; y *= b; z *= b; return *this;}
    Vec3<T>  operator/(const T &b) const { return Vec3<T>(x/b, y/b, z/b);}
    Vec3<T> &operator/=(const T &b){ x /= b; y /= b; z /= b; return *this;}
    Vec3<T>  operator-(const Vec3<T> &b) const { return Vec3<T>(x-b.x, y-b.y, z-b.z);}
    Vec3<T> &operator-=(const Vec3<T> &b){ x-=b.x; y-=b.y; z-=b.z; return *this;}
    Vec3<T>  operator+(const Vec3<T> &b) const { return Vec3<T>(x+b.x, y+b.y, z+b.z);}
    Vec3<T> &operator+=(const Vec3<T> &b){ x+=b.x; y+=b.y; z+=b.z; return *this;}
    
    /** Dot product */
    T operator*(const Vec3<T> &b) const { return x*b.x + y*b.y + z*b.z;}
    
    /** Cross product */
    Vec3<T> operator^(const Vec3<T> &b) const {
        return Vec3<T>((y * b.z - z * b.y),
                       (z * b.x - x * b.z),
                       (x * b.y - y * b.x));
    }

    /**
     * Equality assignment operator
     */
    Vec3<T> &operator=(const Vec3<T> &rhs) {
        x = rhs.x;
        y = rhs.y;
        z = rhs.z;
        return *this;
    }

    /**
     * Get magnitude of this vector, squared.
     * @return 
     */
    T norm2() const {
        return x*x + y*y + z*z;
    }

    /**
     * Get magnitude of this vector.
     * @return 
     */
    T norm() const {
        return (T)sqrt(norm2());
    }


    Vec3<T> normalise() const {
        return (*this)/norm();
    }
    
    /**
     * Tests if this (non-unit) vector is parallel to the given (non-unit) vector.
     * Returns false if either vector has zero length.
     * @param that
     * @return 
     */
    bool isParallelTo(const Vec3 &that) const {
        // Vector lengths
        T thisNorm = this->norm();
        T thatNorm = that.norm();
        
        // Both vectors must have finite length. Returns false if not.
        if(thisNorm==0 || thatNorm==0)
            return false;
        
        // Use dot product to get cosine of vectors internal angle
        T cosAng = ((*this) * that)/(thisNorm*thatNorm);
        
        // Check if vectors are more parallel than the threshold
        return cosAng > cos(PARALLEL_THRESHOLD);
    }
    
    /**
     * This function computes the surface normal for the triangular facet
     * formed by the three vertices, in a clockwise winding order.
     * @param r0        Vertex 1
     * @param r1        Vertex 2
     * @param r2        Vertex 3
     * @return          Surface normal for triangle formed by the three vectors
     */
    static Vec3<T> getClockwiseSurfaceNormal(const Vec3<T> &r0, const Vec3<T> &r1, const Vec3<T> &r2) {

        // Vector from v0 to v1
        Vec3<T> a(r1-r0);
        // Vector from v0 to v2
        Vec3<T> b(r2-r0);
        
        // Cross product a x b gives normal direction
        Vec3<T> n = a^b;
        
        return n.normalise();
    }
    
    
    friend std::ostream& operator<<(std::ostream& os, const Vec3& vec3) {
        os << vec3.x  << ' ' << vec3.y << ' ' << vec3.z << '\n';
        
        // Return a reference to the original ostream so that we can chain
        // stream insertion operations.
        return os;
    }
    
};

#endif	/* VEC3_H */
