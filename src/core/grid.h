#ifndef GRID_H
#define GRID_H

#include <vector>
#include <cmath>
#include <algorithm>
#include <limits>

#include "../shapes/shape.h"


class RegularGrid {
public:
    AABB bounds;                                // outer bbox encloising all FINITE shapes in the scene
    int nx = 1, ny = 1, nz = 1;                 // number of voxels the grid is divided into along the X, Y, and Z axes
    Vector3D cellSize;                          // size of a voxel in world units
    std::vector<std::vector<Shape*>> cells;     // 3D grid flattened --> size = nx · ny · nz
    std::vector<Shape*> unboundedShapes;        // InfinitePlan goes here (walls of the Cornell Box)

    void build(const std::vector<Shape*>& shapes, double lambda = 4.0);
    bool getClosestIntersection(const Ray& r, Intersection& its) const;
    bool hasIntersection(const Ray& r) const;
};

#endif // GRID_H