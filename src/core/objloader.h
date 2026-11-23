#ifndef OBJLOADER_H
#define OBJLOADER_H

#include <cstddef>
#include <string>

#include "scene.h"
#include "vector3d.h"
#include "../materials/material.h"

// Carga un fichero .obj y anade a la escena UN Triangle por cada cara
// (los poligonos de mas de 3 vertices se triangulan en abanico).
//
//  path           : ruta al .obj
//  material       : material que se asigna a TODOS los triangulos
//                   (usa uno NO emisivo: Scene::AddObject hace dynamic_cast<Square*>
//                    para los emisivos y un Triangle no es un Square)
//  scene          : escena a la que se anaden los triangulos
//  position       : donde queda el CENTRO del bounding box de la malla (mundo)
//  targetSize     : tamano de la dimension mas grande del bounding box (mundo).
//                   Sirve para escalar cualquier modelo a un tamano manejable.
//  rotationYDegrees: giro alrededor del eje Y (respecto al centro de la malla)
//
// Devuelve el numero de triangulos anadidos (0 si ha fallado).
size_t loadOBJ(const std::string &path, Material *material, Scene &scene,
               const Vector3D &position, double targetSize,
               double rotationYDegrees = 0.0);

#endif // OBJLOADER_H
