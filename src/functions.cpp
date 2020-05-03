#include "functions.h"


//La méthode generate() permet d'instancier le buffer associé à une figure et le récupérer
GLuint generate(Geometry g)
{
	const float* data = g.getVertices(); //get the vertices created by the cube.
	const float* normals = g.getNormals(); //Get the normal vectors
	int nbVertices = g.getNbVertices();

	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, (3 + 3) * sizeof(float)*nbVertices, NULL, GL_DYNAMIC_DRAW); // 3 pour les coordonnees , 3 pour la couleur
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * 3 * nbVertices, data);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(float) * 3 * nbVertices, 3 * sizeof(float)*nbVertices, normals);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return buffer;
}

//getMatrix() permet d'effectuer une translation de tx en x, ty en y, tz en z et effectuer une rotation de angle radians autours de l'axe dont la valeur vaut 1
glm::mat4 getMatrix(float tx, float ty, float tz, float angle, int x, int y, int z)
{
	glm::mat4 matrix(1.0f);

	matrix = glm::translate(matrix, glm::vec3(tx, ty, tz));
	matrix = glm::rotate(matrix, angle, glm::vec3(x, y, z));

	return matrix;
}

//scaleMatrix est utile pour scaler l'objet indépendamment des translations et rotations. Permet d'éviter le rescale de tous les objets liés à l'objet que l'on souhaite rescale
glm::mat4 scaleMatrix(float sx, float sy, float sz)
{
	glm::mat4 matrix(1.0f);

	matrix = glm::scale(matrix, glm::vec3(sx, sy, sz));

	return matrix;
}

//draw permet de dessiner la figure
void draw(GLuint buffer, uint32_t figVertices, Shader* shader, glm::mat4 mvp, Material material, Light light, GLuint textureID)
{
	glBindBuffer(GL_ARRAY_BUFFER, buffer);


	//on instancie vPosition, vColor et uMVP pour l'affichage de nos figures

	// Se charge d'envoyer vPosition au vertex :
	GLint vPosition = glGetAttribLocation(shader->getProgramID(), "vPosition");
	glVertexAttribPointer(vPosition, 3, GL_FLOAT, 0, 0, 0);
	//It is here that you ,  select how the Shader reads the VBO. Indeed the 5th parameter is called ,  "stride" : it is the distance in bytes between two values for the same ,  kind or variable. If the values are side-by-side, stride == 0.
	//Here we ,  need to set to 3*sizeof(float) for the first version of the VBO seen in ,  clasc.
	glEnableVertexAttribArray(vPosition);   //Enable "vPosition"

	//Work with vColor
	GLint vColor = glGetAttribLocation(shader->getProgramID(), "vColor");
	// Colors start at 9*sizeof(float) (3*nbVertices*sizeof(float)) for the second ,  version of the VBO. For the first version of the VBO, both the stride ,  and the offset should be 3*sizeof(float) here
	glVertexAttribPointer(vColor, 3, GL_FLOAT, 0, 0, INDICE_TO_PTR(sizeof(float) * 3 * figVertices));
	// Convert an indice to void* : (void*)(x)
	glEnableVertexAttribArray(vColor);  //Enable"vColor"

	// uMVP
	GLint uMvp = glGetUniformLocation(shader->getProgramID(), "uMvp");
	glUniformMatrix4fv(uMvp, 1, GL_FALSE, glm::value_ptr(mvp)); // 1 car une seule matrice 

	// normal
	GLint vNormal = glGetAttribLocation(shader->getProgramID(), "vNormal");
	glVertexAttribPointer(vNormal, 3, GL_FLOAT, 0, 0, INDICE_TO_PTR(sizeof(float) * 3 * figVertices));
	glEnableVertexAttribArray(vNormal); //Enable"vNormal"



	// modelView
	GLint uModelView = glGetUniformLocation(shader->getProgramID(), "uModelView");
	glUniformMatrix4fv(uModelView, 1, GL_FALSE, glm::value_ptr(mvp));

	// uK
	GLint uK = glGetUniformLocation(shader->getProgramID(), "uK");
	glUniform4fv(uK, 1, glm::value_ptr(glm::vec4(material.ka, material.kd, material.ks, material.alpha)));

	// uColor
	GLint uColor = glGetUniformLocation(shader->getProgramID(), "uColor");
	glUniform3fv(uColor, 1, glm::value_ptr(material.color));// glm::vec3(1.0f, 0.0f, 0.0f)));

	// uLightPosition
	GLint uLightPosition = glGetUniformLocation(shader->getProgramID(), "uLightPosition");
	glUniform3fv(uLightPosition, 1, glm::value_ptr(light.pos));

	// uLightCOlor
	GLint uLightColor = glGetUniformLocation(shader->getProgramID(), "uLightColor");
	glUniform3fv(uLightColor, 1, glm::value_ptr(light.color));// glm::vec3(1.0f, 1.0f, 1.0f)));

	// uCamPos
	GLint uCameraPosition = glGetUniformLocation(shader->getProgramID(), "uCameraPosition");
	glUniform3fv(uCameraPosition, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f, 0.0f)));//glm::vec3(0.0f, 0.0f, -1.0f)));



	// vUV
	GLint vUV = glGetAttribLocation(shader->getProgramID(), "vUV");
	glVertexAttribPointer(vUV, 2, GL_FLOAT, 0, 0, INDICE_TO_PTR(3 * sizeof(float)*figVertices));
	glEnableVertexAttribArray(vUV);

	// uTexture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID);
	GLint uTexture = glGetUniformLocation(shader->getProgramID(), "uTexture");
	glUniform1i(uTexture, 0);
	glDrawArrays(GL_TRIANGLES, 0, figVertices);
	glBindTexture(GL_TEXTURE_2D, 0);



	//glDrawArrays(GL_TRIANGLES, 0, figVertices);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void displayCommands()
{
	/*
		To interact with our poject, the keys to use are :
		- `z` : move forward
		- `s` : move backward
		- `q` : move sideways to the left
		- `d` : move sideways to the right
	*/
	printf("COMMANDS :\n");
	printf("========================================================\n");
	printf("Control the arm with your mouse cursor\n");
	printf("`Left Click` on the mouse to turn on/off the flashlight");
}
