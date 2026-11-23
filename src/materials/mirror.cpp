#define _USE_MATH_DEFINES //so we can use M_PI

#include "mirror.h"

#include <iostream>

#include "../core/utils.h" //for computeReflectionDirection



Mirror::Mirror()
{
}

Mirror::Mirror(Vector3D Kd_, Vector3D Ks_, float alpha_) :
    rho_d(Kd_), Ks(Ks_), alpha(alpha_) {
}


Vector3D Mirror::getReflectance(const Vector3D& n, const Vector3D& wo,
    const Vector3D& wi) const {

    //FILL(...)
    Vector3D wr = Utils::computeReflectionDirection(wi, n);
    Vector3D f_r = (rho_d / M_PI) + Ks * pow(dot(wo, wr), alpha); // std::max(dot(wo, wr), 0.0) ???

    return f_r;
};

double Mirror::getIndexOfRefraction() const
{
    std::cout << "Warning! Calling \"Material::getIndexOfRefraction()\" for a non-transmissive material"
        << std::endl;

    return -1;
}


Vector3D Mirror::getEmissiveRadiance() const
{
    return Vector3D(0.0);
}


Vector3D Mirror::getDiffuseReflectance() const
{
    return rho_d;
}

