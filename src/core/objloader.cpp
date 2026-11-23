#include "objloader.h"
#include "utils.h"
#include "../shapes/triangle.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <vector>

namespace
{
    struct Face { size_t a, b, c; };

    // Convierte un token de cara ("12", "12/5", "12//7", "12/5/7") en el indice
    // 0-based del vertice. Soporta indices negativos (relativos al final).
    bool parseVertexIndex(const std::string &token, size_t numVertices, size_t &out)
    {
        try
        {
            const long idx = std::stol(token.substr(0, token.find('/')));
            long result;
            if (idx > 0)      result = idx - 1;
            else if (idx < 0) result = static_cast<long>(numVertices) + idx;
            else              return false;      // en OBJ el indice 0 no existe

            if (result < 0 || static_cast<size_t>(result) >= numVertices)
                return false;

            out = static_cast<size_t>(result);
            return true;
        }
        catch (...)
        {
            return false;
        }
    }
}

size_t loadOBJ(const std::string &path, Material *material, Scene &scene,
               const Vector3D &position, double targetSize, double rotationYDegrees)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        std::cerr << "[loadOBJ] No se puede abrir el fichero: " << path << std::endl;
        return 0;
    }

    // ------------------------------------------------------------------
    // 1) Parseo: solo nos interesan las lineas "v x y z" y "f ..."
    //    (vt, vn, usemtl, o, g, s, etc. se ignoran)
    // ------------------------------------------------------------------
    std::vector<Vector3D> vertices;
    std::vector<Face> faces;

    std::string line;
    while (std::getline(file, line))
    {
        std::istringstream iss(line);
        std::string tag;
        iss >> tag;

        if (tag == "v")
        {
            double x, y, z;
            if (iss >> x >> y >> z)
                vertices.push_back(Vector3D(x, y, z));
        }
        else if (tag == "f")
        {
            std::vector<size_t> poly;
            std::string token;
            while (iss >> token)
            {
                size_t idx;
                if (parseVertexIndex(token, vertices.size(), idx))
                    poly.push_back(idx);
            }

            // Triangulacion en abanico: (0,1,2), (0,2,3), (0,3,4)...
            for (size_t i = 1; i + 1 < poly.size(); i++)
                faces.push_back(Face{ poly[0], poly[i], poly[i + 1] });
        }
    }

    if (vertices.empty() || faces.empty())
    {
        std::cerr << "[loadOBJ] El fichero no contiene geometria valida: " << path << std::endl;
        return 0;
    }

    // ------------------------------------------------------------------
    // 2) Bounding box de la malla original -> centro y factor de escala
    // ------------------------------------------------------------------
    Vector3D bmin(std::numeric_limits<double>::max());
    Vector3D bmax(std::numeric_limits<double>::lowest());
    for (size_t i = 0; i < vertices.size(); i++)
    {
        const Vector3D &p = vertices[i];
        bmin = Vector3D(std::min(bmin.x, p.x), std::min(bmin.y, p.y), std::min(bmin.z, p.z));
        bmax = Vector3D(std::max(bmax.x, p.x), std::max(bmax.y, p.y), std::max(bmax.z, p.z));
    }

    const Vector3D center = (bmin + bmax) * 0.5;
    const Vector3D extent = bmax - bmin;
    const double maxExtent = std::max(extent.x, std::max(extent.y, extent.z));
    const double scale = (maxExtent > 0.0) ? targetSize / maxExtent : 1.0;

    // ------------------------------------------------------------------
    // 3) Transformar los vertices a coordenadas de MUNDO:
    //    centrar -> escalar -> rotar en Y -> trasladar a 'position'
    // ------------------------------------------------------------------
    const double angle = Utils::degreesToRadians(rotationYDegrees);
    const double c = std::cos(angle);
    const double s = std::sin(angle);

    for (size_t i = 0; i < vertices.size(); i++)
    {
        const Vector3D p = (vertices[i] - center) * scale;
        vertices[i] = Vector3D(c * p.x + s * p.z,
                               p.y,
                              -s * p.x + c * p.z) + position;
    }

    // ------------------------------------------------------------------
    // 4) Crear un Triangle por cara y anadirlo a la escena
    // ------------------------------------------------------------------
    size_t added = 0, skipped = 0;
    for (size_t i = 0; i < faces.size(); i++)
    {
        const Vector3D &p0 = vertices[faces[i].a];
        const Vector3D &p1 = vertices[faces[i].b];
        const Vector3D &p2 = vertices[faces[i].c];

        // Descartar triangulos degenerados (area ~ 0): su normal seria NaN
        const Vector3D n = cross(p1 - p0, p2 - p0);
        if (dot(n, n) < 1e-24)
        {
            skipped++;
            continue;
        }

        scene.AddObject(new Triangle(p0, p1, p2, material));
        added++;
    }

    std::cout << "[loadOBJ] " << path << ": " << vertices.size() << " vertices, "
              << added << " triangulos anadidos";
    if (skipped > 0)
        std::cout << " (" << skipped << " degenerados descartados)";
    std::cout << std::endl;

    return added;
}
