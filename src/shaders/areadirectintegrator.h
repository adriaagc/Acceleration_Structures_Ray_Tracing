#ifndef AREADIRECTLINTEGRATOR_H
#define AREADIRECTLINTEGRATOR_H

#include "shader.h"

class AreaDirectIllumintaionIntegrator : public Shader
{
public:
    AreaDirectIllumintaionIntegrator();
    AreaDirectIllumintaionIntegrator(Vector3D hitColor, Vector3D bgColor_);

    virtual Vector3D computeColor(const Ray& r,
        const std::vector<Shape*>& objList,
        const std::vector<LightSource*>& lsList) const;

    Vector3D hitColor;
    int n_samples;
};

#endif
