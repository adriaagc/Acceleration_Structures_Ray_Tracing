#include "depthshader.h"

#include "../core/utils.h"

DepthShader::DepthShader() :
    color(Vector3D(1, 0, 0))
{ }

DepthShader::DepthShader(Vector3D hitColor_, double maxDist_, Vector3D bgColor_) :
    Shader(bgColor_), maxDist(maxDist_), color(hitColor_)
{ }

Vector3D DepthShader::computeColor(const Ray &r, const std::vector<Shape*> &objList, const std::vector<LightSource*> &lsList) const
{
    //TASK 3:
    Intersection its;
    if (Utils::getClosestIntersection(r, objList, its)) {
        //origin point
        Vector3D ray_position = r.o;
        //intersection point
        Vector3D its_pos = its.itsPoint;
        //Distance between this two points
        float hitDist = (its_pos - ray_position).length();
        // green channel only --> nearest objecst will have a color near to "pure" green, while far objects will fade to black.
        return Vector3D(0.0, fmax(1.0 - hitDist / maxDist, 0.0), 0.0); 
    }
    return bgColor;
}
