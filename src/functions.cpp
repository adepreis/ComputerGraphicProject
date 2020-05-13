#include "functions.h"


//La méthode generate() permet d'instancier le buffer associé à une figure et le récupérer
GLuint generate(Geometry g)
{
	const float* vertices = g.getVertices(); //get the vertices created by the cube.
	const float* normals = g.getNormals(); //Get the normal vectors
	int nbVertices = g.getNbVertices();

	GLuint buffer = NULL;	// initialized at 0 to avoid runtime error due to uninitialized buffer

	return createBuffer(buffer, vertices, normals, nbVertices);
}

//La méthode createBuffer() permet d'instancier n'importe quel buffer
GLuint createBuffer(GLuint buffer, const float* vertices, const float* normals, int nbVertices)
{
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
		glBufferData(GL_ARRAY_BUFFER, (3 + 3) * sizeof(float)*nbVertices, NULL, GL_DYNAMIC_DRAW); // 3 pour les coordonnees , 3 pour la couleur
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * 3 * nbVertices, vertices);
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

GLuint createTexture(const char * path)
{
	//Convert to an RGBA8888 surface
	SDL_Surface* img = IMG_Load(path);
	SDL_Surface* rgbImg = SDL_ConvertSurfaceFormat(img, SDL_PIXELFORMAT_RGBA32, 0);

	uint8_t* imgInverted = (uint8_t*)malloc(sizeof(uint8_t) * 4 * rgbImg->w*rgbImg->h);
	for (uint32_t j = 0; j < rgbImg->h; j++)
	{
		for (uint32_t i = 0; i < rgbImg->w; i++)
		{
			for (uint8_t k = 0; k < 4; k++)
			{
				uint32_t oldID = 4 * (j*rgbImg->w + i) + k;
				uint32_t newID = 4 * (j*rgbImg->w + rgbImg->w - 1 - i) + k;

				imgInverted[newID] = ((uint8_t*)(rgbImg->pixels))[oldID];
			}
		}
	}

	SDL_FreeSurface(img);

	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	{

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Repeat the texture if needed. Texture coordinates go from (0.0, 0.0) (bottom-left),
		//		to (1.0, 1.0) (top-right). If repeat, (2.0, 2.0) is equivalent to (1.0, 1.0)
		// S is "x coordinate" and T "y coordinate"
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// GL_REPEAT can be replaced by GL_MIRRORED_REPEAT,
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);	// GL_CLAMP_TO_EDGE or GL_CLAMP_TO_BORDER

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, rgbImg->w, rgbImg->h, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)imgInverted);

		//Generate mipmap.
		//Optional but recommended
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	glBindTexture(GL_TEXTURE_2D, 0);

	free(imgInverted);
	SDL_FreeSurface(rgbImg);

	return textureID;
}