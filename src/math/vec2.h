/*
 * Name:
 *   vec2.h
 *
 * Purpose:
 *   Represents 2-vectors.
 *
 * Description:
 *   2-vectors are useful for solving ray transfer problems, for example
 * in computing lens ghosts.
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

#ifndef VEC2_H
#define	VEC2_H

#include <math.h>       /* sqrt, cos */

template < typename T >
class Vec2
{
    // Defines tolerance on detection of parallel vectors
    static const double PARALLEL_THRESHOLD = 0.0001 * (M_PI/180.0);
    
public:
    
    // Public for easy access.
    T x,y;
    
    // Constructors.
    Vec2() : x(0), y(0) { }
    Vec2(const T & px, const T & py) : x(px), y(py) { }
    Vec2(const Vec2 &copy) : x(copy.x), y(copy.y) { }
    
    // Overload all arithmetic operators
    Vec2<T>  operator*(const T &b) const { return Vec2<T>(x*b, y*b);}
    Vec2<T> &operator*=(const T &b){ x *= b; y *= b; return *this;}
    Vec2<T>  operator/(const T &b) const { return Vec2<T>(x/b, y/b);}
    Vec2<T> &operator/=(const T &b){ x /= b; y /= b; return *this;}
    Vec2<T>  operator-(const Vec2<T> &b) const { return Vec2<T>(x-b.x, y-b.y);}
    Vec2<T> &operator-=(const Vec2<T> &b){ x-=b.x; y-=b.y; return *this;}
    Vec2<T>  operator+(const Vec2<T> &b) const { return Vec2<T>(x+b.x, y+b.y);}
    Vec2<T> &operator+=(const Vec2<T> &b){ x+=b.x; y+=b.y; return *this;}
    
    /** Dot product */
    T operator*(const Vec2<T> &b) const { return x*b.x + y*b.y;}
    
    
    
    /** Equality assignment operator */
    Vec2<T> &operator=(const Vec2<T> &rhs)
    {
        x = rhs.x;
        y = rhs.y;
        return *this;
    }
    
    /**
     * Get magnitude of this vector, squared.
     * @return 
     */
    T norm2() const { return x*x + y*y;}
    
    /**
     * Get magnitude of this vector.
     * @return 
     */
    T norm() const { return (T)sqrt(norm2());}
    Vec2<T> normalise() const { return (*this)/norm();}
    
    /**
     * Tests if this (non-unit) vector is parallel to the given (non-unit) vector.
     * Returns false if either vector has zero length.
     * @param that
     * @return 
     */
    bool isParallelTo(const Vec2 &that) const
    {
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
    
    
    friend std::ostream& operator<<(std::ostream& os, const Vec2& vec2)
    {
        os << vec2.x  << ' ' << vec2.y << '\n';
        
        // Return a reference to the original ostream so that we can chain
        // stream insertion operations.
        return os;
    }
    
};

#endif	/* VEC2_H */