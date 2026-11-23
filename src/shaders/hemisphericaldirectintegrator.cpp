#define _USE_MATH_DEFINES //so we can use M_PI

#include "hemisphericaldirectintegrator.h"
#include "../core/utils.h"
#include "../core/hemisphericalsampler.h"
#include "../materials/phong.h"

HemisphericalDirectIllumintaionIntegrator::HemisphericalDirectIllumintaionIntegrator() :
    hitColor(Vector3D(1, 0, 0)), n_samples(256)
{
}

HemisphericalDirectIllumintaionIntegrator::HemisphericalDirectIllumintaionIntegrator(Vector3D hitColor_, Vector3D bgColor_) :
    Shader(bgColor_), hitColor(hitColor_)
{
}

Vector3D HemisphericalDirectIllumintaionIntegrator::computeColor(const Ray& r, const std::vector<Shape*>& objList, const std::vector<LightSource*>& lsList) const
{
    Intersection its;
    if (Utils::getClosestIntersection(r, objList, its)) {
        Vector3D x = its.itsPoint;
        Vector3D n = its.normal;
        Vector3D wo = -r.d.normalized(); // ray towards camera (outgoing)
        
        Vector3D Lo_ind = Vector3D(0.2) * its.shape->getMaterial().getDiffuseReflectance(); // I_a * K_d

        // If the ray hits directly a lightsource:
        if (its.shape->getMaterial().isEmissive()) {
            return Lo_ind + its.shape->getMaterial().getEmissiveRadiance(); // this is the Le(x, wo) term
        }

        HemisphericalSampler sampler;

        float P_wi = 1 / (2 * M_PI); // uniform probability distribution

        Vector3D Lo = Vector3D(0.0); // outgoing direct light 

        for (int i = 0; i < n_samples; i++) {
            Vector3D wi = sampler.getSample(n); // the returned vector is already normalized!!!

            Ray hemisphere_ray = Ray(x, wi);
            Intersection its2;
            
            Vector3D L_i_dir = Vector3D(0.0);

            if (Utils::getClosestIntersection(hemisphere_ray, objList, its2)) {
                if (its2.shape->getMaterial().isEmissive()) {
                    L_i_dir = its2.shape->getMaterial().getEmissiveRadiance();
                }
            }

            Vector3D fr = its.shape->getMaterial().getReflectance(n, wo, wi);

            Lo += L_i_dir * fr * (dot(wi, n)) / P_wi;

        }
        Lo = 1.0/n_samples * Lo;
        return Lo + Lo_ind;
    }
    return bgColor; // If it does not hit anything
}
