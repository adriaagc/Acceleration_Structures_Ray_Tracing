#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "shape.h"

// Triangulo definido por tres vertices en coordenadas de MUNDO.
// Interseccion rayo-triangulo: Moller-Trumbore.
//
// Cada triangulo es un Shape mas de la escena (scene.objectsList), asi que
// Utils::getClosestIntersection() / Utils::hasIntersection() funcionan sin cambios.
class Triangle : public Shape
{
public:
    Triangle() = delete;
    Triangle(const Vector3D &v0_, const Vector3D &v1_, const Vector3D &v2_,
             Material *material_);

    bool rayIntersect(const Ray &ray, Intersection &its) const override;
    bool rayIntersectP(const Ray &ray) const override;

    // Normal geometrica (unitaria) en coordenadas de mundo
    Vector3D getNormalWorld() const { return normal; }

    // Utiles para construir mas adelante la AABB / BVH
    const Vector3D &getV0() const { return v0; }
    const Vector3D &getV1() const { return v1; }
    const Vector3D &getV2() const { return v2; }

private:
    Vector3D v0, v1, v2;
    Vector3D edge1, edge2;   // v1 - v0, v2 - v0 (precalculados)
    Vector3D normal;         // normal geometrica normalizada
};

#endif // TRIANGLE_H
