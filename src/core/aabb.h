#ifndef AABB_H
#define AABB_H

#pragma once
#include "vector3d.h"
#include "ray.h"
#include <algorithm>
#include <limits>

struct AABB {
    Vector3D min;
    Vector3D max;

    AABB()
        : min(std::numeric_limits<double>::infinity()),
        max(-std::numeric_limits<double>::infinity()) {
    }

    AABB(const Vector3D& minPt, const Vector3D& maxPt) : min(minPt), max(maxPt) {}

    void expand(const Vector3D& p) {
        min.x = std::min(min.x, p.x); min.y = std::min(min.y, p.y); min.z = std::min(min.z, p.z);
        max.x = std::max(max.x, p.x); max.y = std::max(max.y, p.y); max.z = std::max(max.z, p.z);
    }

    void expand(const AABB& other) { // useful for building BVH trees
        expand(other.min);
        expand(other.max);
    }

    // Slab Method: returns true if ray hits box within [ray.minT, ray.maxT]
    bool intersect(const Ray& r, double& tEnter, double& tExit) const {
        tEnter = r.minT;
        tExit = r.maxT;

        for (int axis = 0; axis < 3; ++axis) {
            double orig = (axis == 0) ? r.o.x : (axis == 1 ? r.o.y : r.o.z); // obtain current axis value (x, y or z)
            double dir = (axis == 0) ? r.d.x : (axis == 1 ? r.d.y : r.d.z);
            double bMin = (axis == 0) ? min.x : (axis == 1 ? min.y : min.z);
            double bMax = (axis == 0) ? max.x : (axis == 1 ? max.y : max.z);

            if (std::abs(dir) < 1e-12) {
                if (orig < bMin || orig > bMax) return false;
            }
            else {
                double invD = 1.0 / dir;
                double t0 = (bMin - orig) * invD;   // distance at which the ray intersects with the plane (axis = bMin)
                double t1 = (bMax - orig) * invD;   // "" (axis = bMax)
                if (t0 > t1) std::swap(t0, t1);     // ensure t0 is always the closest intersection
                tEnter = std::max(tEnter, t0);      // ray when entering must have intersected all the planes (keep farthest)
                tExit = std::min(tExit, t1);        // ray when leaving only has to intersect one plane (keep nearest)
                if (tEnter > tExit) return false;
            }
        }
        return true;
    }
};

#endif // AABB_H