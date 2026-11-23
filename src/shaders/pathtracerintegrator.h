#ifndef PATHTRACERINTEGRATOR_H
#define PATHTRACERINTEGRATOR_H

#include "shader.h"

class PurePathTracerIntegrator : public Shader
{
public:
    PurePathTracerIntegrator();
    PurePathTracerIntegrator(Vector3D hitColor, Vector3D bgColor_);

    Vector3D ComputeRadiance(const Ray& r,
        const std::vector<Shape*>& objList, 
        const std::vector<LightSource*>& lsList) const;

    virtual Vector3D computeColor(const Ray& r,
        const std::vector<Shape*>& objList,
        const std::vector<LightSource*>& lsList) const;

    Vector3D hitColor;
    int max_depth;
    int spp;
};

#endif
