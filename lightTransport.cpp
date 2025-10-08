/*
Contains the implementations for the BSDF and Integrator functions. It also has utility functions, such as 
the triangle intersection code.

*/



#include <vector>
#include <cmath>
#include <random>
#include "object.h"
#include "lightTransport.h"

const double PI = 3.14159265358979323846;

Intersection::Intersection(Point p, Vec3 n, Color c) : point{p}, normal{n}, baseColor{c} {}

std::pair<double, Vec3> triangleIntersect(const Triangle& tri, const Ray& r)
{
    Vec3 e1 = tri.b->pt-tri.a->pt;
    Vec3 e2 = tri.c->pt-tri.a->pt;

    Vec3 h = cross(r.direction(), e2);
    double a = dot(h, e1);

    if (a < 0.00001)
        return {-1.0, Vec3(0,0,0)};
    double f = 1.0/a;

    Vec3 s = r.origin()-tri.a->pt;
    double u = f * dot(s, h);
    Vec3 q = cross(s, e1);
    double v = f * dot(r.direction(), q);
    double t = f * dot(e2, q);


    if (((u >= 0) && (v >= 0) && (u + v <= 1)) && t > 0.00001)
    {
        //std::cout << "intersected!" << std::endl;
        return {t, Vec3(u, v, 1.0-u-v)};
    }
    else
        return {-1.0, Vec3(0,0,0)};
}

Intersection sceneIntersection(const std::vector<Triangle>& tris, const Ray& r, double max_t = 99999999.0)
{
    double minT = std::numeric_limits<double>::max();
    Intersection closest = Intersection();

    for (const Triangle& tri : tris)
    {
        auto [t, P] = triangleIntersect(tri, r);
        if (t != -1.0)
        {
            if (t < minT && t < max_t)
            {
    
                minT = t;

                closest.point = r.pointAt(t);
                closest.normal = tri.a->n; // replace with averaged normal
                closest.baseColor = tri.a->c * P[0] + tri.b->c * P[1] + tri.c->c * P[2];
                closest.ray = r;
                closest.hitTri = tri;
                closest.valid = true;
                closest.backface = false;

            }
        }
    }

    return closest;
    
}

Color phongBSDF::f(const Vec3& wi, const Vec3& wo, const Color& color) 
{
    if (wi[2] <= 0 || wo[2] <= 0) 
        return Vec3();

    Vec3 wr = (Vec3(0,0,1) * (2.0 * dot(wi, Vec3(0,0,1))))-wi;
    double cosAlpha = std::max(0.0, dot(wo, wr));
    return color * ((phongExponent + 2) / (2 * PI)) * std::pow(cosAlpha, phongExponent) * wo.z();
}

Color phongBSDF::sample_f(const Vec3& wi, Vec3& wo, double& pdf, const Color& color, SimpleSampler& sample) 
{
    Vec3 wr =  (Vec3(0,0,1) * (2.0 * dot(wi, Vec3(0,0,1))))- wi;

    auto [u1, u2] = sample.get2D();
    
    double theta = std::acos(std::pow(u1, 1.0 / (phongExponent + 2)));
    double phi = 2 * PI * u2;
    double x = std::sin(theta) * std::cos(phi);
    double y = std::sin(theta) * std::sin(phi);
    double z = std::cos(theta);

    Vec3 t, b;
    if (std::fabs(wr[2]) < 0.999) 
        t = unit(cross(Vec3(0,0,1), wr));
    else 
        t = Vec3(1,0,0);
    b = cross(wr, t);
    wo = unit(t*x + b*y + wr*z);

    pdf = phongBSDF::pdf(wi, wo);
    return f(wi, wo, color);
}

double phongBSDF::pdf(const Vec3& wi, const Vec3& wo)
{
    if (wi.z() <= 0 || wo.z() <= 0)
        return 0.0;

    Vec3 wr = (Vec3(0,0,1) * (2.0 * dot(wi, Vec3(0,0,1))))-wi;
    wr = unit(wr);

    return ((phongExponent + 2) / (2 * PI)) * std::pow(std::max(0.0, dot(wo, wr)), phongExponent);
}

Color simpleDiffuseBSDF::f(const Vec3& wi, const Vec3& wo, const Color& color) 
{
    return color/PI;
}

Color simpleDiffuseBSDF::sample_f(const Vec3& wi, Vec3& wo, double& pdf, const Color& color, SimpleSampler& sample) 
{
    auto [u1, u2] = sample.get2D();
    //Vec3 wr = (Vec3(0,0,1) * (2.0 * dot(wi, Vec3(0,0,1))))-wi;
    double theta = std::acos(std::sqrt(u1));
    double phi = 2*PI*u2;

    double x = std::sin(theta) * std::cos(phi);
    double y = std::sin(theta) * std::sin(phi);
    double z = std::cos(theta);

    wo = Vec3(x, y, z);
    pdf = simpleDiffuseBSDF::pdf(wi, wo);

    return f(wi, wo, color);
}

double simpleDiffuseBSDF::pdf(const Vec3& wi, const Vec3& wo) 
{
    if (wo.z() <= 0.0) return 0.0;
    return wo.z() / PI;
    
}

Color mirrorBSDF::f(const Vec3& wi, const Vec3& wo, const Color& color) {return color;}

Color mirrorBSDF::sample_f(const Vec3& wi, Vec3& wo, double& pdf, const Color& color, SimpleSampler& sample) 
{
    wo =  (Vec3(0,0,1) * (2.0 * dot(wi, Vec3(0,0,1))))- wi;
    pdf = mirrorBSDF::pdf(wi, wo);
    return f(wi, wo, color);
}

double mirrorBSDF::pdf(const Vec3& wi, const Vec3& wo) {return 1.0;}

Color nextEventEstimation( const Vec3& wo, const std::vector<Triangle>& objects, const std::vector<Triangle>& lights, SimpleSampler& sample, BSDF& reflector, Intersection& intersect, double& light_pdf)
{
    Color contribution = Color(0,0,0);
    int index = static_cast<int>(sample.get1D() * lights.size());
    Triangle l = lights.at(index); 
    double u = sqrt(sample.get1D());
    double v = sample.get1D();

    Point p = (1 - u) * l.a->pt + u * (1 - v) * l.b->pt + u * v * l.c->pt;
    Vec3 n = intersect.hitTri.a->n;

    Vec3 surfaceToLight = p-intersect.point;
    
    
    Vec3 wi = unit(surfaceToLight);
    Ray r = Ray(wi, intersect.point + n * 0.0001);
    
    auto [t, _] = triangleIntersect(l, r);
    
    Intersection lightIntersect = sceneIntersection(objects, r, t*0.99999);

    if (!lightIntersect.valid && t != -1.0)
    {
        double distanceSQR = surfaceToLight.lengthSquared();
        Vec3 lightNormal = l.a->n;

        double G = dot(lightNormal, -wi) * dot (n, wi)/distanceSQR;
        double area = 0.5 * cross(l.b->pt - l.a->pt, l.c->pt - l.a->pt).length();
        
        
        light_pdf = distanceSQR/(lights.size()*dot(lightNormal, -wi) * area);
        Color Le = l.emission;
        Color f_val = reflector.f(wi, wo, intersect.baseColor);

        contribution = f_val * Le * G /light_pdf;
    }
    return contribution;
}

// const std::vector<Triangle>& objects - stores all scene triangles
// const std::vector<Triangle>& lights - stores all emissive triangles
// Ray r - initial ray out of camera
// SimpleSampler& sample - the random sampler
Color MISIntegrator::Li(const std::vector<Triangle>& objects, const std::vector<Triangle>& lights, Ray r, SimpleSampler& sample)
{
    Vec3 Li = Vec3();  
    Vec3 beta = Vec3(1.0,1.0,1.0);  

    Intersection intersectPt;
    Vec3 wi_local, wo_local, wo_world;

    for (int depth = 0; depth < maxDepth; depth++)
    {
        intersectPt = sceneIntersection(objects,r);
        
        if (!intersectPt.valid)
        {
            break;
        }
        BSDF* reflector = intersectPt.hitTri.material;
        
        toLocal(-r.direction(), unit(intersectPt.normal) , wi_local);

        double light_pdf = 0;
        Color nee = nextEventEstimation(wi_local, objects, lights, sample, *reflector, intersectPt, light_pdf);
        
        wo_local = Vec3(0,0,0);

        double pdf_val;
        Vec3 f_val = reflector->sample_f(wi_local, wo_local, pdf_val, intersectPt.baseColor, sample);
        if (pdf_val <= 0) 
            break;

        // MIS STUFF - power heuristic?
        double neeWeight = light_pdf * light_pdf / (light_pdf * light_pdf + pdf_val * pdf_val);
        double bsdfWeight = pdf_val * pdf_val / (light_pdf * light_pdf + pdf_val * pdf_val);

        toWorld(unit(intersectPt.normal), wo_local, wo_world);
        r = Ray(wo_world, intersectPt.point + intersectPt.normal*0.0001);

        Li += beta * nee * neeWeight;
        beta *= (f_val * fabs(wo_local.z()) / pdf_val);
        Li += beta * intersectPt.hitTri.emission * bsdfWeight;
        

    }
    return Li;
}
