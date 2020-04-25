#ifndef  OBJLOADER_INC
#define  OBJLOADER_INC

// #include <iostream>
#include <vector>

//GML library
#include <glm/glm.hpp>

/**
 * On souhaite que loadOBJ lise le fichier � path �, �crive les donn�es dans out_vertices/out_uvs/out_normals et
 * retourne false si quelque chose s�est mal pass�. *std::vector* est la fa�on de d�clarer un tableau de glm::vec3 en C++
 * dont la taille peut �tre modifi�e � volont� : cela n'a rien � voir avec un vecteur math�matique.
 * Sinc�rement, ce n'est qu'un tableau. Finalement, le & signifie que la fonction sera capable de modifier les std::vector.
 */
bool loadOBJ(
    const char* path,
    std::vector<glm::vec3> & out_vertices,
    std::vector<glm::vec2> & out_uvs,
    std::vector<glm::vec3> & out_normals
);

#endif