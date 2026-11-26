#include "triangle.h"
#include "../core/stats.h"

#include <cmath>

namespace
{
    // Umbral para considerar el rayo paralelo al triangulo (determinante ~ 0).
    // OJO: el determinante escala con el area del triangulo
    // (det ~ 2 * area * cos(theta) * |d|), asi que en mallas densas es muy pequeno.
    // Con el Epsilon "global" del framework se descartarian triangulos validos,
    // por eso usamos un umbral propio mucho mas bajo.
    const double kDetEpsilon = 1e-12;
}

Triangle::Triangle(const Vector3D &v0_, const Vector3D &v1_, const Vector3D &v2_,
                   Material *material_)
    : Shape(Matrix4x4(), material_),
      v0(v0_), v1(v1_), v2(v2_),
      edge1(v1_ - v0_), edge2(v2_ - v0_),
      normal(cross(v1_ - v0_, v2_ - v0_).normalized())
{ }

bool Triangle::getBoundingBox(AABB& box) const
{
    box = AABB();
    box.expand(v0);
    box.expand(v1);
    box.expand(v2);
    return true;
}

bool Triangle::rayIntersect(const Ray &ray, Intersection &its) const
{
    STAT_INC(closestHitTests);   // <-- cuenta un test de triangulo (closest-hit)

    // ---------------- Moller-Trumbore ----------------
    const Vector3D pvec = cross(ray.d, edge2);
    const double det = dot(edge1, pvec);

    // Rayo paralelo al plano del triangulo
    if (std::abs(det) < kDetEpsilon)
        return false;

    const double invDet = 1.0 / det;
    const Vector3D tvec = ray.o - v0;

    // Coordenada baricentrica u
    const double u = dot(tvec, pvec) * invDet;
    if (u < 0.0 || u > 1.0)
        return false;

    // Coordenada baricentrica v
    const Vector3D qvec = cross(tvec, edge1);
    const double v = dot(ray.d, qvec) * invDet;
    if (v < 0.0 || u + v > 1.0)
        return false;

    // Distancia a lo largo del rayo
    const double tHit = dot(edge2, qvec) * invDet;
    if (tHit < ray.minT || tHit > ray.maxT)
        return false;

    // ---------------- Hit valido ----------------
    // Actualizamos maxT para que los siguientes shapes descarten hits mas lejanos
    // (el mismo mecanismo que usan Sphere, Square e InfinitePlan).
    ray.maxT = tHit;

    its.itsPoint = ray.o + ray.d * tHit;

    // Normal orientada hacia el lado desde el que llega el rayo. Asi las mallas
    // con winding inconsistente no salen negras con materiales difusos.
    // (Si usas un material Transmissive sobre la malla y necesitas distinguir
    //  entrar/salir, quita este flip y usa siempre 'normal'.)
    its.normal = (dot(normal, ray.d) < 0.0) ? normal : normal * (-1.0);

    its.shape = this;

    return true;
}

bool Triangle::rayIntersectP(const Ray &ray) const
{
    STAT_INC(shadowTests);       // <-- cuenta un test de triangulo (shadow ray)

    const Vector3D pvec = cross(ray.d, edge2);
    const double det = dot(edge1, pvec);

    if (std::abs(det) < kDetEpsilon)
        return false;

    const double invDet = 1.0 / det;
    const Vector3D tvec = ray.o - v0;

    const double u = dot(tvec, pvec) * invDet;
    if (u < 0.0 || u > 1.0)
        return false;

    const Vector3D qvec = cross(tvec, edge1);
    const double v = dot(ray.d, qvec) * invDet;
    if (v < 0.0 || u + v > 1.0)
        return false;

    const double tHit = dot(edge2, qvec) * invDet;
    if (tHit < ray.minT || tHit > ray.maxT)
        return false;

    // Para un shadow ray basta saber que hay oclusion.
    // (No tocamos maxT: no hace falta y evita efectos laterales.)
    return true;
}
