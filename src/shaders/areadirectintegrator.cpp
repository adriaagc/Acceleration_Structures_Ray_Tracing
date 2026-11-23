#define _USE_MATH_DEFINES //so we can use M_PI

#include "areadirectintegrator.h"
#include "../core/utils.h"
#include "../materials/phong.h"

AreaDirectIllumintaionIntegrator::AreaDirectIllumintaionIntegrator() :
    hitColor(Vector3D(1, 0, 0)), n_samples(256)
{
}

AreaDirectIllumintaionIntegrator::AreaDirectIllumintaionIntegrator(Vector3D hitColor_, Vector3D bgColor_) :
    Shader(bgColor_), hitColor(hitColor_)
{
}

Vector3D AreaDirectIllumintaionIntegrator::computeColor(const Ray& r, const std::vector<Shape*>& objList, const std::vector<LightSource*>& lsList) const
{
    Intersection its;
    if (Utils::getClosestIntersection(r, objList, its)) {
        
        Vector3D Lo_ind = Vector3D(0.2) * its.shape->getMaterial().getDiffuseReflectance(); // I_a * K_d (constant/ambient term)

        // If the ray hits directly a lightsource:
        if (its.shape->getMaterial().isEmissive()) {
            return Lo_ind + its.shape->getMaterial().getEmissiveRadiance(); // this is the Le(x, wo) term
        }
        
        Vector3D x = its.itsPoint;
        Vector3D nx = its.normal;
        Vector3D wo = -r.d.normalized(); // ray towards camera (outgoing)

        Vector3D Lo = Vector3D(0.0); // outgoing direct light 

        for (auto s : lsList) {

            float lsArea = s->getArea();
            float P_wi = 1 / lsArea; // probability distribution = 1/A
            Vector3D ny = s->getNormal();

            for (int i = 0; i < n_samples; i++) {
                
                Vector3D y = s->sampleLightPosition();   // generates a random point inside the square ls
                Vector3D wi = (y - x).normalized();      // unit vector that goes from x towards a random point from the ls (y)

                Ray ls_ray = Ray(x, wi);                        // ray towards the light source
                Intersection its2;

                Vector3D L_i_dir = Vector3D(0.0);

                if (Utils::getClosestIntersection(ls_ray, objList, its2)) {   // check for occlusion !!

                    if (its2.shape->getMaterial().isEmissive()) {
                        L_i_dir = its2.shape->getMaterial().getEmissiveRadiance();   // Le only contributes if its a ls (direct illumination)
                    }
                    else {
                        // replaces the visibility term:
                        //    if the object the ray hits 1st is not a ls (emissive mat) then it is occluded.
                        continue;
                    }
                }
                else {
                    // if there is no intersection!
                    continue;
                }

                Vector3D G_xy = (std::max(0.0, dot(nx, wi)) * std::max(0.0, dot(-wi, ny))) / (y - x).lengthSq();   // geometric term

                Vector3D fr = its.shape->getMaterial().getReflectance(nx, wo, wi);

                Lo += L_i_dir * fr * G_xy / P_wi;

            }
        }
        Lo = 1.0/n_samples * Lo;
        return Lo + Lo_ind;
    }
    return bgColor; // If it does not hit anything
}
