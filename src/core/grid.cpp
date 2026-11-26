#include "grid.h"

void RegularGrid::build(const std::vector<Shape*>& shapes, double lambda) {
    std::vector<Shape*> boundedShapes;
    bounds = AABB();

    // 1. Separate finite vs infinite shapes and compute scene AABB
    for (Shape* s : shapes) {
        AABB box;
        if (s->getBoundingBox(box)) {
            bounds.expand(box);
            boundedShapes.push_back(s);
        }
        else {
            unboundedShapes.push_back(s);
        }
    }

    if (boundedShapes.empty()) return;

    // Add tiny padding to avoid floating-point boundary issues
    // e.g. scene contains only one flat Square lying on plane z = 5
    // then bounds.min.z = bounds.max.z = 5, making the box's depth 0.
    Vector3D eps(1e-4, 1e-4, 1e-4);
    bounds.min = bounds.min - eps; // prevents cellSize = 0 (then we divide by cellSize)
    bounds.max = bounds.max + eps;

    // 2. Choose grid resolution
    Vector3D extent = bounds.max - bounds.min;      // (dx, dy, dz) = (width, height, depth)
    double volume = extent.x * extent.y * extent.z;
    double factor = std::cbrt((lambda * boundedShapes.size()) / std::max(volume, 1e-9)); // k = 1/s value (s: size of the cubes)
    // A. Scale cell count with object count: If a scene has N shapes, we want the total number of cells to be roughly lambda*N.
    // B. Keep every voxel shaped like a cube: If X = 2*Y, you want twice as many cells along X as along Y.

    // every axis must have at least 1 cell (otw 0 volume, 2D grid) and set a maximum to protect the RAM
    nx = std::clamp((int)std::floor(extent.x * factor), 1, 128); // nx = dx/s = dx*k
    ny = std::clamp((int)std::floor(extent.y * factor), 1, 128);
    nz = std::clamp((int)std::floor(extent.z * factor), 1, 128);

    cellSize = Vector3D(extent.x / nx, extent.y / ny, extent.z / nz);
    cells.assign(nx * ny * nz, {}); // initializes every single slot to an empty std::vector<Shape*>

    // 3. Insert each finite shape into all overlapping voxels
    for (Shape* s : boundedShapes) {
        AABB box;
        s->getBoundingBox(box);

        // compute the cell indices that the bbox of the shape occupies:
        int minX = std::clamp((int)((box.min.x - bounds.min.x) / cellSize.x), 0, nx - 1);
        int minY = std::clamp((int)((box.min.y - bounds.min.y) / cellSize.y), 0, ny - 1);
        int minZ = std::clamp((int)((box.min.z - bounds.min.z) / cellSize.z), 0, nz - 1);

        int maxX = std::clamp((int)((box.max.x - bounds.min.x) / cellSize.x), 0, nx - 1);
        int maxY = std::clamp((int)((box.max.y - bounds.min.y) / cellSize.y), 0, ny - 1);
        int maxZ = std::clamp((int)((box.max.z - bounds.min.z) / cellSize.z), 0, nz - 1);

        // walk through every single voxel occupied by the shape:
        for (int z = minZ; z <= maxZ; ++z)
            for (int y = minY; y <= maxY; ++y)
                for (int x = minX; x <= maxX; ++x)
                    cells[x + y * nx + z * nx * ny].push_back(s); // because we flattened cells
                    // z*(nx*ny): skip z full slices (each has nx*ny cells)
                    // y*nx: skip y rows
                    // x: move x cells along the correct row
    }
}

bool RegularGrid::getClosestIntersection(const Ray& r, Intersection& its) const {
    bool hit = false;

    // 1. Always test unbounded shapes (e.g., InfinitePlan walls)
    for (Shape* s : unboundedShapes) {
        if (s->rayIntersect(r, its)) {
            hit = true; // rayIntersect already tightened r.maxT
        }
    }

    // 2. Test ray against the grid's outer AABB
    double tEnter, tExit;
    if (cells.empty() || !bounds.intersect(r, tEnter, tExit))
        return hit;

    // Starting point inside the grid
    double tStart = std::max(tEnter, r.minT);
    Vector3D startPt = r.o + r.d * tStart;

    int ix = std::clamp((int)((startPt.x - bounds.min.x) / cellSize.x), 0, nx - 1);
    int iy = std::clamp((int)((startPt.y - bounds.min.y) / cellSize.y), 0, ny - 1);
    int iz = std::clamp((int)((startPt.z - bounds.min.z) / cellSize.z), 0, nz - 1);

    int stepX = (r.d.x >= 0) ? 1 : -1;
    int stepY = (r.d.y >= 0) ? 1 : -1;
    int stepZ = (r.d.z >= 0) ? 1 : -1;

    auto initAxis = [&](double dir, double orig, double bMin, double cSize, int idx, int step,
        double& tMax, double& tDelta) {
            if (std::abs(dir) < 1e-12) {
                tMax = std::numeric_limits<double>::infinity();
                tDelta = std::numeric_limits<double>::infinity();
            }
            else {
                double nextBoundary = bMin + (idx + (step > 0 ? 1 : 0)) * cSize;
                tMax = (nextBoundary - orig) / dir;
                tDelta = std::abs(cSize / dir);
            }
        };

    double tMaxX, tMaxY, tMaxZ;
    double tDeltaX, tDeltaY, tDeltaZ;
    initAxis(r.d.x, r.o.x, bounds.min.x, cellSize.x, ix, stepX, tMaxX, tDeltaX);
    initAxis(r.d.y, r.o.y, bounds.min.y, cellSize.y, iy, stepY, tMaxY, tDeltaY);
    initAxis(r.d.z, r.o.z, bounds.min.z, cellSize.z, iz, stepZ, tMaxZ, tDeltaZ);

    // 3. Step through voxels along the ray (3D DDA)
    while (ix >= 0 && ix < nx && iy >= 0 && iy < ny && iz >= 0 && iz < nz) {
        const auto& cellShapes = cells[ix + iy * nx + iz * nx * ny];
        for (Shape* s : cellShapes) {
            if (s->rayIntersect(r, its)) {
                hit = true; // rayIntersect already updated r.maxT
            }
        }

        double tNext = std::min({ tMaxX, tMaxY, tMaxZ });

        // Early exit: closest hit is inside (or before) this voxel
        if (hit && r.maxT <= tNext)
            return true;

        if (tNext > tExit)
            break;

        // Advance to whichever voxel boundary the ray crosses first
        if (tMaxX < tMaxY) {
            if (tMaxX < tMaxZ) { ix += stepX; tMaxX += tDeltaX; }
            else { iz += stepZ; tMaxZ += tDeltaZ; }
        }
        else {
            if (tMaxY < tMaxZ) { iy += stepY; tMaxY += tDeltaY; }
            else { iz += stepZ; tMaxZ += tDeltaZ; }
        }
    }

    return hit;
}

bool RegularGrid::hasIntersection(const Ray& r) const {
    // 1. Test unbounded shapes first (any hit means occluded)
    for (Shape* s : unboundedShapes) {
        if (s->rayIntersectP(r))
            return true;
    }

    // 2. Test ray against the grid's outer AABB
    double tEnter, tExit;
    if (cells.empty() || !bounds.intersect(r, tEnter, tExit))
        return false;

    double tStart = std::max(tEnter, r.minT);
    Vector3D startPt = r.o + r.d * tStart;

    int ix = std::clamp((int)((startPt.x - bounds.min.x) / cellSize.x), 0, nx - 1);
    int iy = std::clamp((int)((startPt.y - bounds.min.y) / cellSize.y), 0, ny - 1);
    int iz = std::clamp((int)((startPt.z - bounds.min.z) / cellSize.z), 0, nz - 1);

    int stepX = (r.d.x >= 0) ? 1 : -1;
    int stepY = (r.d.y >= 0) ? 1 : -1;
    int stepZ = (r.d.z >= 0) ? 1 : -1;

    auto initAxis = [&](double dir, double orig, double bMin, double cSize, int idx, int step,
        double& tMax, double& tDelta) {
            if (std::abs(dir) < 1e-12) {
                tMax = std::numeric_limits<double>::infinity();
                tDelta = std::numeric_limits<double>::infinity();
            }
            else {
                double nextBoundary = bMin + (idx + (step > 0 ? 1 : 0)) * cSize;
                tMax = (nextBoundary - orig) / dir;
                tDelta = std::abs(cSize / dir);
            }
        };

    double tMaxX, tMaxY, tMaxZ;
    double tDeltaX, tDeltaY, tDeltaZ;
    initAxis(r.d.x, r.o.x, bounds.min.x, cellSize.x, ix, stepX, tMaxX, tDeltaX);
    initAxis(r.d.y, r.o.y, bounds.min.y, cellSize.y, iy, stepY, tMaxY, tDeltaY);
    initAxis(r.d.z, r.o.z, bounds.min.z, cellSize.z, iz, stepZ, tMaxZ, tDeltaZ);

    // 3. Step through voxels — return immediately on the FIRST blocker
    while (ix >= 0 && ix < nx && iy >= 0 && iy < ny && iz >= 0 && iz < nz) {
        const auto& cellShapes = cells[ix + iy * nx + iz * nx * ny];
        for (Shape* s : cellShapes) {
            if (s->rayIntersectP(r))
                return true;
        }

        double tNext = std::min({ tMaxX, tMaxY, tMaxZ });
        if (tNext > tExit || tNext > r.maxT)
            break;

        if (tMaxX < tMaxY) {
            if (tMaxX < tMaxZ) { ix += stepX; tMaxX += tDeltaX; }
            else { iz += stepZ; tMaxZ += tDeltaZ; }
        }
        else {
            if (tMaxY < tMaxZ) { iy += stepY; tMaxY += tDeltaY; }
            else { iz += stepZ; tMaxZ += tDeltaZ; }
        }
    }

    return false;
}