#ifndef  FUNCTIONS_INC
#define  FUNCTIONS_INC

//GML libraries
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "structures.h"

// objects 3D
#include "Geometry.h"

//needed for texture mapping
#include <SDL2/SDL_image.h>

#define INDICE_TO_PTR(x) ((void*)(x))


//La m�thode generate() permet d'instancier le buffer associ� � une figure et le r�cup�rer
GLuint generate(Geometry g);

//La m�thode createBuffer() permet d'instancier n'importe quel buffer
GLuint createBuffer(GLuint buffer, const float* vertices, const float* normals, int nbVertices);

//getMatrix() permet d'effectuer une translation de tx en x, ty en y, tz en z et effectuer une rotation de angle radians autours de l'axe dont la valeur vaut 1
glm::mat4 getMatrix(float tx, float ty, float tz, float angle, int x, int y, int z);

//scaleMatrix est utile pour scaler l'objet ind�pendamment des translations et rotations. Permet d'�viter le rescale de tous les objets li�s � l'objet que l'on souhaite rescale
glm::mat4 scaleMatrix(float sx, float sy, float sz);

//draw permet de dessiner la figure
void draw(GLuint buffer, uint32_t figVertices, Shader* shader, glm::mat4 mvp, Material material, Light light, GLuint textureID);

//affiche (dans le terminal) les touches utiles et leurs fonctions
void displayCommands();

//retourne une texture � partir du chemin donn�
GLuint createTexture(const char* path);

#endif