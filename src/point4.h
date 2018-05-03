#ifndef POINT4_H__
#define POINT4_H__

#include <cassert>
#include <algorithm>
#include <iostream>
#include <vector>
#include <cmath>
#include <float.h>



template< typename T >
class point4
{
public:
    typedef T               type_t;

    point4< T >( T x_ , T y_ , T z_ , T w_) { v[0] = x_; v[1] = y_; v[2] = z_; v[3] = w_; }

    template< class point_t > point4< T >( point_t const & p )
    {
        v[0] = (T)(p[0]);
        v[1] = (T)(p[1]);
        v[2] = (T)(p[2]);
        v[3] = (T)(p[3]);
    }

    point4< T >(){v[0] = 0; v[1] = 0; v[2] = 0; v[3] = 0;}

    inline  T x() const {return v[0];}
    inline  T y() const {return v[1];}
    inline  T z() const {return v[2];}
    inline  T w() const {return v[3];}

    inline  T operator [] (unsigned int c) const
    {
        return v[c];
    }
    inline  T & operator [] (unsigned int c)
    {
        return v[c];
    }

    static point4<T> Zero() { return point4<T>(0,0,0,0); }

    void setZero()
    {
        v[0] = 0;
        v[1] = 0;
        v[2] = 0;
        v[3] = 0;
    }

    // You cannot template over anything here, but maybe you could template over typename T2 for operator += (const point4< T2 > & other)
    void operator += (const point4< T > & other)
    {
        v[0] += other.x();
        v[1] += other.y();
        v[2] += other.z();
        v[3] += other.w();
    }
    void operator -= (const point4< T > & other)
    {
        v[0] -= other.x();
        v[1] -= other.y();
        v[2] -= other.z();
        v[3] -= other.w();
    }

    // This is going to create problems if the compiler needs to resolve umbiguous casts, but it's the cleaner way to do it
    void operator *= (int s)
    {
        v[0] *= s;
        v[1] *= s;
        v[2] *= s;
        v[3] *= s;
    }
    void operator *= (unsigned int s)
    {
        v[0] *= s;
        v[1] *= s;
        v[2] *= s;
        v[3] *= s;
    }
    void operator *= (float s)
    {
        v[0] *= s;
        v[1] *= s;
        v[2] *= s;
        v[3] *= s;
    }
    void operator *= (double s)
    {
        v[0] *= s;
        v[1] *= s;
        v[2] *= s;
        v[3] *= s;
    }
    void operator /= (int s)
    {
        v[0] /= s;
        v[1] /= s;
        v[2] /= s;
        v[3] /= s;
    }
    void operator /= (unsigned int s)
    {
        v[0] /= s;
        v[1] /= s;
        v[2] /= s;
        v[3] /= s;
    }
    void operator /= (float s)
    {
        v[0] /= s;
        v[1] /= s;
        v[2] /= s;
        v[3] /= s;
    }
    void operator /= (double s)
    {
        v[0] /= s;
        v[1] /= s;
        v[2] /= s;
        v[3] /= s;
    }



    T sqrnorm() const
    {
        return v[0]*v[0]+v[1]*v[1]+v[2]*v[2]+v[3]*v[3];
    }
    T norm() const
    {
        return sqrt(sqrnorm());
    }

    void normalize()
    {
        T _n = norm();
        v[0] /= _n;
        v[1] /= _n;
        v[2] /= _n;
        v[3] /= _n;
    }
    void setNorm( T newNorm )
    {
        T _factor = newNorm / norm();
        v[0] *= _factor;
        v[1] *= _factor;
        v[2] *= _factor;
        v[3] *= _factor;
    }

    inline static
    T dot( const point4< T > & p1 , const point4< T > & p2 )
    {
        return p1.x() * p2.x() + p1.y() * p2.y() + p1.z() * p2.z() + p1.w() * p2.w();
    }


    template< class point_t1 , class point_t2 >
    inline static point4< T > min ( const point_t1  & p , const point_t2  & p2 )
    {
        return point4< T >( std::min( (T)(p2[0]) , (T)(p[0]) ),
                std::min( (T)(p2[1]) , (T)(p[1]) ),
                std::min( (T)(p2[2]) , (T)(p[2]) ) ,
                std::min( (T)(p2[3]) , (T)(p[3]) ) );
    }

    template< class point_t1 , class point_t2 >
    inline static point4< T > max ( const point_t1  & p , const point_t2  & p2 )
    {
        return point4< T >( std::max( (T)(p2[0]) , (T)(p[0]) ),
                std::max( (T)(p2[1]) , (T)(p[1]) ),
                std::max( (T)(p2[2]) , (T)(p[2]) ),
                std::max( (T)(p2[3]) , (T)(p[3]) ) );
    }

    inline static point4< T > Rand ( point4< T > const & p , point4< T > const & p2 )
    {
        T rx = p[0] + (p2[0] - p[0]) * (float)(rand()) / (float)( RAND_MAX );
        T ry = p[1] + (p2[1] - p[1]) * (float)(rand()) / (float)( RAND_MAX );
        T rz = p[2] + (p2[2] - p[2]) * (float)(rand()) / (float)( RAND_MAX );
        T rw = p[2] + (p2[2] - p[2]) * (float)(rand()) / (float)( RAND_MAX );
        return point4<T>( rx , ry , rz , rw );
    }
    inline static point4< T > Rand ( double randMagnitude )
    {
        T rx = randMagnitude * (2.0*(float)(rand()) / (float)( RAND_MAX ) - 1.0);
        T ry = randMagnitude * (2.0*(float)(rand()) / (float)( RAND_MAX ) - 1.0);
        T rz = randMagnitude * (2.0*(float)(rand()) / (float)( RAND_MAX ) - 1.0);
        T rw = randMagnitude * (2.0*(float)(rand()) / (float)( RAND_MAX ) - 1.0);
        return point4<T>( rx , ry , rz , rw );
    }


    point4<T> direction() const {
        T n = norm();
        return *this/n;
    }


    bool isnan() const {
        return std::isnan(v[0]) || std::isnan(v[1]) || std::isnan(v[2]) || std::isnan(v[3]);
    }


private:
    T v[4];
};






template< typename T > inline
point4< T > operator + (const point4< T > & p1 , const point4< T > & p2 )
{
    return point4< T >( p1.x() + p2.x() , p1.y() + p2.y() , p1.z() + p2.z(), p1.w() + p2.w() );
}
template< typename T > inline
point4< T > operator - (const point4< T > & p1 , const point4< T > & p2 )
{
    return point4< T >( p1.x() - p2.x() , p1.y() - p2.y() , p1.z() - p2.z(), p1.w() - p2.w() );
}


template< typename T > inline
point4< T > operator - (const point4< T > & p2 )
{
    return point4< T >( - p2.x() , - p2.y() , - p2.z()  , - p2.w() );
}

template< typename T > inline
point4< T > operator * (const point4< T > & p , int s)
{
    return point4< T >( s*p.x() , s*p.y()  , s*p.z()  , s*p.w() );
}
template< typename T > inline
point4< T > operator * (const point4< T > & p , float s)
{
    return point4< T >( s*p.x() , s*p.y() , s*p.z()  , s*p.w()  );
}
template< typename T > inline
point4< T > operator * (const point4< T > & p , double s)
{
    return point4< T >( s*p.x() , s*p.y()  , s*p.z()  , s*p.w() );
}
template< typename T > inline
point4< T > operator * ( int s , const point4< T > & p )
{
    return point4< T >( s*p.x() , s*p.y()  , s*p.z() , s*p.w() );
}
template< typename T > inline
point4< T > operator * ( float s , const point4< T > & p )
{
    return point4< T >( s*p.x() , s*p.y() , s*p.z() , s*p.w()  );
}
template< typename T > inline
point4< T > operator * ( double s , const point4< T > & p )
{
    return point4< T >( s*p.x() , s*p.y()  , s*p.z() , s*p.w() );
}


template< typename T > inline
point4< T > operator / (const point4< T > & p , int s)
{
    return point4< T >( p.x()/s , p.y()/s  , p.z()/s , p.w()/s );
}
template< typename T > inline
point4< T > operator / (const point4< T > & p , float s)
{
    return point4< T >( p.x()/s , p.y()/s  , p.z()/s  , p.w()/s );
}
template< typename T > inline
point4< T > operator / (const point4< T > & p , double s)
{
    return point4< T >( p.x()/s , p.y()/s  , p.z()/s  , p.w()/s );
}


template< typename T > inline
T operator * (const point4< T > & p1 , const point4< T > & p2)
{
    return p1.x() * p2.x() + p1.y() * p2.y() + p1.z() * p2.z() + p1.w() * p2.w();
}





template< typename T > inline std::ostream & operator << (std::ostream & s , point4< T > const & p)
{
    s << p[0] << " \t" << p[1] << " \t" << p[2] << " \t" << p[3];
    return s;
}





typedef point4< float >    point4f;
typedef point4< double >   point4d;
typedef point4< unsigned int >   point4ui;




inline double myAbs( const point4d & f )
{
    return f.norm();
}
inline float myAbs( const point4f & f )
{
    return f.norm();
}





#endif // POINT4_H_
