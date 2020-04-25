#ifndef  OBJLOADER_INC
#define  OBJLOADER_INC

// #include <iostream>
#include <vector>

//GML library
#include <glm/glm.hpp>

/**
 * On souhaite que loadOBJ lise le fichier « path », écrive les données dans out_vertices/out_uvs/out_normals et
 * retourne false si quelque chose s’est mal passé. *std::vector* est la façon de déclarer un tableau de glm::vec3 en C++
 * dont la taille peut être modifiée à volonté : cela n'a rien à voir avec un vecteur mathématique.
 * Sincèrement, ce n'est qu'un tableau. Finalement, le & signifie que la fonction sera capable de modifier les std::vector.
 */
bool loadOBJ(
    const char* path,
    std::vector<glm::vec3> & out_vertices,
    std::vector<glm::vec2> & out_uvs,
    std::vector<glm::vec3> & out_normals
);

#endif