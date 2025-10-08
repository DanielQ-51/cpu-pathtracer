/*
Contains the class definitions for the BSDF and Integrator functions.

*/

#pragma once

#include <vector>
#include <cmath>
#include <random>
#include "object.h"

class SimpleSampler 
{
    public:
    SimpleSampler(unsigned int seed = 12345) : rng(seed), dist(0.0f, 1.0f) {}
    
    float get1D() {return dist(rng);}

    std::pair<float, float> get2D() {return {dist(rng), dist(rng)};}

    private:
    std::mt19937 rng;  // Mersenne Twister pseudo-random engine
    std::uniform_real_distribution<float> dist;
};

struct Intersection
{
    Point point;
    Vec3 normal;
    Color baseColor;
    Ray ray;
    Triangle hitTri;
    int name;
    bool valid;
    bool backface;

    Intersection(Point p, Vec3 n, Color c);
    Intersection() {valid = false;};
};

class BSDF
{
    public:

    virtual Color f(const Vec3& wi, const Vec3& wo, const Color& color) { return Vec3(0,0,0);}

    virtual Color sample_f(const Vec3& wi, Vec3& wo, double& pdf, const Color& color, SimpleSampler& sample) {return Vec3(0,0,0);}

    virtual double pdf(const Vec3& wi, const Vec3& wo) {return 0.0;}
};

class simpleDiffuseBSDF : public BSDF
{
    Color f(const Vec3& wi, const Vec3& wo, const Color& color) override;
    Color sample_f(const Vec3& wi, Vec3& wo, double& pdf, const Color& color, SimpleSampler& sample) override;
    double pdf(const Vec3& wi, const Vec3& wo) override;
};

class phongBSDF : public BSDF
{
    public:

    int phongExponent;
    
    Color f(const Vec3& wi, const Vec3& wo, const Color& color);
    Color sample_f(const Vec3& wi, Vec3& wo, double& pdf, const Color& color, SimpleSampler& sample);
    double pdf(const Vec3& wi, const Vec3& wo);
};

class mirrorBSDF : public BSDF
{
    public:
    
    Color f(const Vec3& wi, const Vec3& wo, const Color& color);
    Color sample_f(const Vec3& wi, Vec3& wo, double& pdf, const Color& color, SimpleSampler& sample);
    double pdf(const Vec3& wi, const Vec3& wo);
};

Color nextEventEstimation(const Vec3& wo, const std::vector<Triangle>& objects, const std::vector<Triangle>& lights, SimpleSampler& sample, BSDF& reflector, Intersection& intersect);


class MISIntegrator 
{
    public:

    int maxDepth;

    Color Li(const std::vector<Triangle>& objects, const std::vector<Triangle>& lights, Ray r, SimpleSampler& sample);
};
