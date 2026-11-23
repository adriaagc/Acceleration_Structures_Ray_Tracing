#define _USE_MATH_DEFINES //so we can use M_PI

#include "pathtracerintegrator.h"
#include "../core/utils.h"
#include "../materials/phong.h"

#include "../core/hemisphericalsampler.h" // we need to get random light vectors


PurePathTracerIntegrator::PurePathTracerIntegrator() :
    hitColor(Vector3D(1, 0, 0)), max_depth(4), spp(256)
{
}

PurePathTracerIntegrator::PurePathTracerIntegrator(Vector3D hitColor_, Vector3D bgColor_) :
    Shader(bgColor_), hitColor(hitColor_)
{
}

Vector3D PurePathTracerIntegrator::ComputeRadiance(const Ray& r, const std::vector<Shape*>& objList, const std::vector<LightSource*>& lsList) const
{
    Intersection its;
    Vector3D Lo = Vector3D(0.0);
    if (Utils::getClosestIntersection(r, objList, its)) {
        if (its.shape->getMaterial().isEmissive()) {
            return its.shape->getMaterial().getEmissiveRadiance();
        }
        Vector3D n = its.normal;
        Vector3D x = its.itsPoint;
        HemisphericalSampler sampler;
        Vector3D wi = sampler.getSample(n); // the returned vector is already normalized!!!
        Ray hemisphere_ray = Ray(x, wi);
        float pdf = 1.0 / (2 * M_PI);

        if (r.depth < max_depth) {
            Ray newR = Ray(x, wi, r.depth + 1);
            Lo += ComputeRadiance(newR, objList, lsList) * its.shape->getMaterial().getReflectance(n, -r.d, wi) * dot(n, wi) / pdf;
        }
    }
    return Lo;
}

Vector3D PurePathTracerIntegrator::computeColor(const Ray& r, const std::vector<Shape*>& objList, const std::vector<LightSource*>& lsList) const
{
    Vector3D Lo = Vector3D(0.0);

    // Monte Carlo over the pixel (spp)
    for (int i = 0; i < spp; i++) {
        Lo += ComputeRadiance(r, objList, lsList);
    }
    return Lo / float(spp);
}
