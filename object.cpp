/*
Contains the class data for the basic Vector, Ray, Vertex, Triangle classes, with the operator overloads.

*/

#include <vector>
#include <cmath>
#include "object.h"

using Color = Vec3;
using Point = Vec3;

Vec3::Vec3() : coord{0,0,0} {}
Vec3::~Vec3() {}

Vec3::Vec3(double x, double y, double z) : coord{x,y,z} {} 

double Vec3::lengthSquared() const
{
    return coord[0]*coord[0] + coord[1]*coord[1] + coord[2]*coord[2];
}

double Vec3::length() const
{
    return std::sqrt(lengthSquared());
}

Vec3 Vec3::operator-() const
{
    return Vec3(-coord[0], -coord[1], -coord[2]);
}

double Vec3::operator[](int i) const
{
    return coord[i];
}

double Vec3::operator[](int i)
{
    return coord[i];
}

Vec3& Vec3::operator+=(const Vec3& v)
{
    coord[0]+=v.x();
    coord[1]+=v.y();
    coord[2]+=v.z();

    return *this;
}

Vec3& Vec3::operator-=(const Vec3& v)
{
    coord[0]-=v.x();
    coord[1]-=v.y();
    coord[2]-=v.z();

    return *this;
}

Vec3& Vec3::operator*=(double r)
{
    coord[0]*=r;
    coord[1]*=r;
    coord[2]*=r;

    return *this;
}

Vec3& Vec3::operator*=(const Vec3& v) 
{
    coord[0] *= v.x(); 
    coord[1] *= v.y(); 
    coord[2] *= v.z();
    return *this;
}

Vec3& Vec3::operator/=(double r)
{
    coord[0]/=r;
    coord[1]/=r;
    coord[2]/=r;

    return *this;
}

Ray::Ray() {}
Ray::~Ray() {}

Ray::Ray(const Vec3& d, const Point& o) : dir{d}, orig{o} {}

Point Ray::pointAt(double t) const
{
    return Point(orig + dir*t);
}

std::ostream& operator<< (std::ostream& out, const Vec3& v )
{
    out << "<" << v.x() << " " << v.y() << " " << v.z() << ">";
    return out; 
}

Vec3 operator+(const Vec3& v1, const Vec3& v2)
{
    return Vec3(v1.x()+v2.x(), v1.y()+v2.y(), v1.z()+v2.z());
}

Vec3 operator-(const Vec3& v1, const Vec3& v2)
{
    return Vec3(v1.x()-v2.x(), v1.y()-v2.y(), v1.z()-v2.z());
}

Vec3 operator*(const Vec3& v, double r)
{
    return Vec3(v.x()*r, v.y()*r, v.z()*r);
}

Vec3 operator*(double r, const Vec3& v)
{
    return v * r;
}

Vec3 operator*(const Vec3& a, const Vec3& b) 
{
    Vec3 result = a;
    result *= b;  // reuse the *= operator
    return result;
}

Vec3 operator/(const Vec3& v, double r)
{
    return Vec3(v.x()/r, v.y()/r, v.z()/r);
}

double dot(const Vec3& v1, const Vec3& v2)
{
    return v1.x()*v2.x() + v1.y()*v2.y() + v1.z()*v2.z();
}

Vec3 cross(const Vec3& v1, const Vec3& v2)
{
    return Vec3(v1.y() * v2.z() - v1.z() * v2.y(),
        v1.z() * v2.x() - v1.x() * v2.z(),
        v1.x() * v2.y() - v1.y() * v2.x());
}

Vec3 unit(const Vec3& v)
{
    return v/v.length();
}

void toLocal(const Vec3 &wi_world, const Vec3 &normal, Vec3& wi_local) 
{
    Vec3 t, b;
    if (std::fabs(normal.x()) > std::fabs(normal.z()))
        t = unit(Vec3(-normal.y(), normal.x(), 0));
    else
        t = unit(Vec3(0, -normal.z(), normal.y()));

    b = cross(normal, t);

    wi_local = Vec3(dot(wi_world, t),
                dot(wi_world, b),
                dot(wi_world, normal));
}

void toWorld(const Vec3 &normal, const Vec3 &wo_local, Vec3& wo_world) 
{
    Vec3 t, b;

    // Build tangent and bitangent
    if (std::fabs(normal.x()) > std::fabs(normal.z()))
        t = unit(Vec3(-normal.y(), normal.x(), 0));
    else
        t = unit(Vec3(0, -normal.z(), normal.y()));

    b = cross(normal, t);

    // Convert local vector to world space
    wo_world = wo_local.x() * t + wo_local.y() * b + wo_local.z() * normal;
}

Vertex::Vertex() {}

Vertex::Vertex(Point point, Color color, Vec3 norm) : pt{point}, c{color}, n{norm} {}

Vertex::~Vertex() {}

Triangle::Triangle(Vertex* v1, Vertex* v2, Vertex* v3) : a{v1}, b{v2}, c{v3}, emission{Color(0,0,0)} {}
Triangle::Triangle(Vertex* v1, Vertex* v2, Vertex* v3, Color e) : a{v1}, b{v2}, c{v3} , emission{e} {}
Triangle::Triangle(Vertex* v1, Vertex* v2, Vertex* v3, Color e, BSDF* m) : a{v1}, b{v2}, c{v3} , emission{e}, material{m}{}

Vec3 barycentricCoordinate(const Triangle& t, const Point& i) 
{
    Point pt1 = t.a->pt;
    Point pt2 = t.b->pt;
    Point pt3 = t.c->pt;
    Vec3 v0 = pt3-pt1;
    Vec3 v1 = pt2-pt1;
    Vec3 v2 = i-pt1;

    double d00 = dot(v0, v0);
    double d01 = dot(v0, v1);
    double d11 = dot(v1, v1);
    double d20 = dot(v2, v0);
    double d21 = dot(v2, v1);

    double denom_bary = d00*d11 - d01*d01;

    double u = (d11*d20 - d01*d21) / denom_bary;
    double v = (d00*d21 - d01*d20) / denom_bary;

    return Vec3(u, v, 1.0-u-v);
}
