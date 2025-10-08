/*
Contains the class data for the basic Vector, Ray, Vertex, Triangle classes, with the operator overloads.

*/

#pragma once

#include <vector>
#include <iostream>
#include "image.h"

class BSDF;
struct Intersection;


class Vec3 {

private:
    

public:
    double coord[3];
    double x() const {return coord[0];}
    double y() const {return coord[1];}
    double z() const {return coord[2];}

    Vec3();
    Vec3(double x, double y, double z);
    //Vec3(const Vec3 u,const Vec3 v);
    ~Vec3();

    double length() const;
    double lengthSquared() const;

    Vec3 operator-() const;

    double operator[](int i) const;
    double operator[](int i);
    
    Vec3& operator+=(const Vec3& v);
    Vec3& operator-=(const Vec3& v);
    Vec3& operator*=(double r);
    Vec3& operator*=(const Vec3& v);
    Vec3& operator/=(double r);
};

using Color = Vec3;
using Point = Vec3;

class Ray
{
    private:
    Vec3 dir;
    Point orig;
    public:

    Ray();
    ~Ray();
    Ray(const Vec3& dir, const Point& orig);

    Point pointAt(double t) const;

    Vec3 direction() const{return dir;}
    Point origin() const{return orig;}

};

struct Vertex {
    Point pt;
    Color c;
    Vec3 n;

    Vertex();
    Vertex(Point point, Color color, Vec3 norm);
    ~Vertex();
};

struct Triangle {
    Vertex* a; 
    Vertex* b;
    Vertex* c;
    Color emission;
    BSDF* material;

    // for use in flat shading
    Vec3 surfaceNormal;

    //Triangle();
    Triangle() 
        : a(nullptr), b(nullptr), c(nullptr), emission(), surfaceNormal(), material(nullptr){}
    //~Triangle();
    Triangle(Vertex* v1, Vertex* v2, Vertex* v3);
    Triangle(Vertex* v1, Vertex* v2, Vertex* v3, Color e);
    Triangle(Vertex* v1, Vertex* v2, Vertex* v3, Color e, BSDF* m);
    
};

std::ostream& operator<< (std::ostream& out, const Vec3& v );

Vec3 operator+(const Vec3& v1, const Vec3& v2);
Vec3 operator-(const Vec3& v1, const Vec3& v2);
Vec3 operator*(const Vec3& v1, double r);
Vec3 operator*(double r, const Vec3& v);
Vec3 operator*(const Vec3& a, const Vec3& b);
Vec3 operator/(const Vec3& v1, double r);

double dot(const Vec3& v1, const Vec3& v2);
Vec3 cross(const Vec3& v1, const Vec3& v2);

Vec3 unit(const Vec3& v);
void toLocal(const Vec3 &wi_world, const Vec3 &normal, Vec3& wi_local);
void toWorld(const Vec3 &normal, const Vec3 &wo_local, Vec3& wo_world);

Vec3 barycentricCoordinate(const Triangle& t, const Point& i);


/*class Objects {

public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices; // contiguous 3 makes one triangle

    Objects();
    ~Objects();


};*/
