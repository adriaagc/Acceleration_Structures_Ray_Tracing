#ifndef HEMISPHERICALDIRECTLINTEGRATOR_H
#define HEMISPHERICALDIRECTLINTEGRATOR_H

#include "shader.h"

class HemisphericalDirectIllumintaionIntegrator : public Shader
{
public:
    HemisphericalDirectIllumintaionIntegrator();
    HemisphericalDirectIllumintaionIntegrator(Vector3D hitColor, Vector3D bgColor_);

    virtual Vector3D computeColor(const Ray& r,
        const std::vector<Shape*>& objList,
        const std::vector<LightSource*>& lsList) const;

    Vector3D hitColor;
    int n_samples;
};

#endif
