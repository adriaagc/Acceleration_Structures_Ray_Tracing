#include "whittedintegrator.h"
#include "../core/utils.h"
#include "../materials/phong.h"

WhittedIntegrator::WhittedIntegrator() :
    hitColor(Vector3D(1, 0, 0))
{
}

WhittedIntegrator::WhittedIntegrator(Vector3D hitColor_, Vector3D bgColor_) :
    Shader(bgColor_), hitColor(hitColor_)
{
}

Vector3D WhittedIntegrator::computeColor(const Ray& r, const std::vector<Shape*>& objList, const std::vector<LightSource*>& lsList) const
{
    Intersection its;
    if (Utils::getClosestIntersection(r, objList, its)) {

        if (r.depth >= 5) { return Vector3D(0.0); } // avoid infinite loops when 2 mirrors are facing each other

        Vector3D lo = Vector3D(0.0);
        // 4.5.3 Perfect Specular Reflection
        if (its.shape->getMaterial().hasSpecular()) {
            Vector3D wr = Utils::computeReflectionDirection(-r.d, its.normal); // specular surface --> angle of incidence of r (the ray that is reflected towards the camera) = angle of reflection of the ray that hits the mirror-like surface
            Ray reflectionRay = Ray(its.itsPoint, wr, r.depth+1);
            lo += computeColor(reflectionRay, objList, lsList);
        }
        // 4.5.2 Direct Illumination
        else if (its.shape->getMaterial().hasDiffuseOrGlossy()) {
            //lo = Vector3D(0.0); // Whithout any constant light
            lo = Vector3D(0.1) * its.shape->getMaterial().getDiffuseReflectance(); // With a constant light
            Vector3D n = its.normal;
            Vector3D x = its.itsPoint;                  // intersection point
            for (auto s : lsList) {
                double d = (x - s->sampleLightPosition()).length(); // distance intersection point to light source
                Vector3D li = s->getIntensity();
                Vector3D wi = -(x - s->sampleLightPosition()).normalized(); // vector intersection point to light source
                Vector3D fr = its.shape->getMaterial().getReflectance(n, -r.d.normalized(), wi.normalized());
                Ray lightToObject = Ray(s->sampleLightPosition(), -wi);
                lightToObject.maxT = d - 1.0E-4; // the ray must finish before reaching x.
                int visibility = int(!Utils::hasIntersection(lightToObject, objList)); // if it has intersected then visibility is 0.
                lo += li * fr * dot(wi, n) * visibility; // / (d * d);    // divide by d^2 because light intensity fades as we move away from the point light source.
            }
        }
        // 4.5.4 Perfect Specular Refraction
        else if (its.shape->getMaterial().hasTransmission()) {
            double mu_t = its.shape->getMaterial().getIndexOfRefraction(); //value to test if its total reflection
            Vector3D wo = -r.d; //vector from the surface to the camara
            Vector3D nx = its.normal;
            double mu = mu_t;

            if (dot(nx, wo) < 0) { // we look if the ray is inside the object and change mu to the oposite. if negative we are inside
                nx = -nx; //as we are inside we flip the normal
                mu = 1.0 / mu_t; //now the n1 and n2 have switched order so mu=nglass/nair
            }

            //compute the radicand: radicand=1-mu_t^2 (1-(nx · wo)^2)
            double cos = dot(nx, wo);
            double radicand = 1.0 - mu * mu * (1.0 - cos * cos);

            if (radicand >= 0.0) { //if radicand is positive there is refraction
                Vector3D wt = -mu * wo + nx * (mu * cos - std::sqrt(radicand)); //formula from notes
                Ray refracRay = Ray(its.itsPoint, wt, r.depth + 1);
                lo = computeColor(refracRay, objList, lsList);
            }
            else { //else there is total internal reflection
                Vector3D wr = Utils::computeReflectionDirection(wo, nx);
                Ray reflRay = Ray(its.itsPoint, wr, r.depth + 1);
                lo = computeColor(reflRay, objList, lsList);
            }
        }
        return lo;
    }
    return bgColor; // If it does not hit anything
}
