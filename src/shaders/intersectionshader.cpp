#include "intersectionshader.h"
#include "../core/utils.h"

IntersectionShader::IntersectionShader() :
    hitColor(Vector3D(1, 0, 0))
{ }

IntersectionShader::IntersectionShader(Vector3D hitColor_, Vector3D bgColor_) :
    Shader(bgColor_), hitColor(hitColor_)
{ }

Vector3D IntersectionShader::computeColor(const Ray &r, const std::vector<Shape*> &objList, const std::vector<LightSource*> &lsList) const
{
    //Example 1
    if (Utils::hasIntersection(r, objList)) {
        return hitColor;
    }

    //Example 2 
    /*Intersection its;
    if (Utils::getClosestIntersection(r, objList, its)) {
        return Vector3D(0.0, 1.0, 0.0);
    }*/

    return bgColor;
}
