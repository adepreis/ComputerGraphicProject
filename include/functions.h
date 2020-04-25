#ifndef  FUNCTIONS_INC
#define  FUNCTIONS_INC

//GML libraries
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "structures.h"

// objects 3D
#include "Geometry.h"

#define INDICE_TO_PTR(x) ((void*)(x))


//La méthode generate() permet d'instancier le buffer associé à une figure et le récupérer
GLuint generate(Geometry g);

//getMatrix() permet d'effectuer une translation de tx en x, ty en y, tz en z et effectuer une rotation de angle radians autours de l'axe dont la valeur vaut 1
glm::mat4 getMatrix(float tx, float ty, float tz, float angle, int x, int y, int z);

//scaleMatrix est utile pour scaler l'objet indépendamment des translations et rotations. Permet d'éviter le rescale de tous les objets liés à l'objet que l'on souhaite rescale
glm::mat4 scaleMatrix(float sx, float sy, float sz);

//draw permet de dessiner la figure
void draw(GLuint buffer, Geometry g, Shader* shader, glm::mat4 mvp, Material material, Light light, GLuint textureID);

//affiche (dans le terminal) les touches utiles et leurs fonctions
void displayCommands();

#endif