//SDL Libraries
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

//OpenGL Libraries
#include <GL/glew.h>
#include <GL/gl.h>

//GML libraries
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/type_ptr.hpp>
//needed for texture mapping
#include <SDL2/SDL_image.h>

#include "Shader.h"

#include "logger.h"

#include "functions.h"
#include "structures.h"
#include "OBJLoader.h"

// objects 3D
#include "Sphere.h"
#include "Cube.h"
#include "Cylinder.h"

//libraries supplémentaires
#include "vector"
#include "math.h"

//On définit une fenêtre carrée pour éviter tout problème de rotation ou scaling.
#define WIDTH     700
#define HEIGHT    700
#define FRAMERATE 60
#define TIME_PER_FRAME_MS  (1.0f/FRAMERATE * 1e3)
#define INDICE_TO_PTR(x) ((void*)(x))


int main(int argc, char *argv[])
{
    ////////////////////////////////////////
    //SDL2 / OpenGL Context initialization : 
    ////////////////////////////////////////
    
    //Initialize SDL2
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0)
    {
        ERROR("The initialization of the SDL failed : %s\n", SDL_GetError());
        return 0;
    }

	//Init the IMG component
	if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG))
	{
		ERROR("Could not load SDL2_image with PNG files\n");
		return EXIT_FAILURE;
	}

    //Create a Window
    SDL_Window* window = SDL_CreateWindow("FPS Flashlight - Projet Info Graphique",		//Titre
                                          SDL_WINDOWPOS_UNDEFINED,               //X Position
                                          SDL_WINDOWPOS_UNDEFINED,               //Y Position
                                          WIDTH, HEIGHT,                         //Resolution
                                          SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN); //Flags (OpenGL + Show)

    //Initialize OpenGL Version (version 3.0)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    //Initialize the OpenGL Context (where OpenGL resources (Graphics card resources) lives)
    SDL_GLContext context = SDL_GL_CreateContext(window);

    //Tells GLEW to initialize the OpenGL function with this version
    glewExperimental = GL_TRUE;
    glewInit();


    //Start using OpenGL to draw something on screen
    glViewport(0, 0, WIDTH, HEIGHT); //Draw on ALL the screen

    //The OpenGL background color (RGBA, each component between 0.0f and 1.0f)
    glClearColor(0.0, 0.0, 0.0, 1.0); //Full Black
	//glClearColor(128.0, 0.0, 128.0, 1.0); //Pink

    glEnable(GL_DEPTH_TEST); //Active the depth test

	//////////////////////////////////////////////////////////////////////////////////////////PARTIE_ELEVE////////////////////////////////////////////////////////////////////////////////////
	

	//Convert to an RGBA8888 surface
	SDL_Surface* img = IMG_Load("../../Images/mercure.png");
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

	//Send data to graphics card
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


    //TODO
	std::vector <Geometry>	listeFigures; //liste de toutes les figures créées
	std::vector <GLuint>	listeBuffer; //liste des buffers associés aux figures
	std::vector <glm::mat4> listeMvp; //liste des matrices associées aux figures

	//ATTENTION: on veillera à ce que tous les objets liés à une même figure soient au même index dans toutes les listes

	//on instancie la matrice de la caméra
	glm::mat4 cameraMatrix(1.0f);

	cameraMatrix = glm::rotate(cameraMatrix, (float)M_PI, glm::vec3(0, 1, 0));		// place la cam derrière le perso
	// cameraMatrix = glm::rotate(cameraMatrix, -0.5f*(float)M_PI, glm::vec3(1, 0, 0));		// place la cam au dessus

	// cameraMatrix = glm::translate(cameraMatrix, glm::vec3(0.f, 0.f, 1.0f));		// tentative de reculer la cam



	// =========================== Flashlight ==============================================
	/////////// Flashlight model loading ///////////////
	std::vector<glm::vec3> flashlight_vertices;
	std::vector<glm::vec2> flashlight_uvs;
	std::vector<glm::vec3> flashlight_normals; // Won't be used at the moment.

	/* This model is not adapted to our parser (but could offer more options (nMap, texture,...) : */
	// bool resTorch = loadOBJ("../../Models/flashlight_model/flashlight.obj", flashlight_vertices, flashlight_uvs, flashlight_normals);

	/* This model is just a test model adapted to our parser : */
	// bool resTorch = loadOBJ("../../Models/cube_test.obj", flashlight_vertices, flashlight_uvs, flashlight_normals);

	/* This one is a adapted model using Blender : */
	bool resTorch = loadOBJ("../../Models/flashLight/Flashlight.obj", flashlight_vertices, flashlight_uvs, flashlight_normals);
	////////////////////////////////////


	GLuint torchBuffer = NULL; 	// generate flashlight buffer

	// !!!!! flashlight_vertices & flashlight_normal NE SONT PAS DES CONST FLOAT * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//torchBuffer = createBuffer(torchBuffer, (const float*)flashlight_vertices, (const float*)flashlight_normals, flashlight_vertices.size());

	glGenBuffers(1, &torchBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, torchBuffer);
		//			glBufferData(GL_ARRAY_BUFFER, (3 + 3) * sizeof(float)*nbBgVertices, NULL, GL_DYNAMIC_DRAW);
		glBufferData(GL_ARRAY_BUFFER, flashlight_vertices.size() * sizeof(glm::vec3), &flashlight_vertices[0], GL_STATIC_DRAW);

		/* Pas défini donc empêche reflet lumière et texture : */
		// glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * 3 * nbBgVertices, bgColor);
		// glBufferSubData(GL_ARRAY_BUFFER, 3 * sizeof(float)*nbBgVertices, 3 * sizeof(float)*nbBgVertices, bgPosition);
	glBindBuffer(GL_ARRAY_BUFFER, 0);



	// =========================== Background (& floor ?) ==============================================

	// TODO : create a dedicated class for the background wall ?? + rename bg in wall ??
	uint32_t nbBgVertices = 3;

	float bgPosition[] = { -1.0f, -1.0f, 1.0f,		// leftDown
							 1.0f, -1.0f, 1.0f,		// rightDown
							-1.0f,  1.0f, 1.0f };	// up

	float bgColor[] = { 1.0f, 0.0f, 0.0f,
						0.0f, 1.0f, 0.0f,
						0.0f, 0.0f, 1.0f };


	GLuint bgBuffer = NULL;	// generate background wall buffer
	bgBuffer = createBuffer(bgBuffer, bgColor, bgPosition, nbBgVertices); // (sizeof(bgColor) / sizeof(bgColor[0]))/3); ???



	// test floor using Immediate Mode
	glColorMaterial(GL_FRONT, GL_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);
	glBegin(GL_TRIANGLES);
		glVertex3f(0.5, 0.0, 0.0);
		glVertex3f(0.0, 0.0, 0.0);
		glVertex3f(0.0, 0.0, 0.5);

		glVertex3f(-0.5, 0.0, 0.0);
		glVertex3f(0.0, 0.0, 0.0);
		glVertex3f(0.0, 0.0, -0.5);
	glEnd();

	// ===================================================================================================


	/*
		Ici, on va créer une par une toutes les figures qui composent notre personnage

		Dans l'ordre :	- on instancie la figure
						- on l'ajoute à la liste des figures
						- on génère son buffer qu'on ajoute à la liste des buffers
						- on créé sa matrice sans prendre en compte le scaling par souci de simplification
						- on ajoute sa matrice à la liste des matrices. Attention à l'ordre des matrices. Chaque matrice doit dépendre de la matrice à sa gauche, la caméra étant le référentiel absolu

		On ne scale qu'une fois tous les objets créés afin de ne pas avoir besoin d'adapter le scale de tous les objets en fonction de celui des objets dont ils dépendent
	
		On créé un premier cylindre qui sera le corps de notre personnage, l'angle de -pi / 2 permet d'orienter le cylindre comme souhaité.
		Attention, par défaut un cylindre fait face à la caméra et ses faces plates sont invisibles.

		On retrouvera un angle par défaut sur les figures représentant les épaules, coudes, cuisses et genoux car ce sont des articulations dans notre modèle

		A l'exception des angles qui sont calculés selon les données du TP, toutes les valeurs ont été trouvées par tatonnements
	*/
	Cylinder body(32);
	listeFigures.push_back(body);
	listeBuffer.push_back(generate(body));
	glm::mat4 bodyMatrix = getMatrix(0, -0.3, 0, -M_PI / 2.f, 1, 0, 0);
	listeMvp.push_back(cameraMatrix * bodyMatrix);

	Sphere head(32, 32);
	listeFigures.push_back(head);
	listeBuffer.push_back(generate(head));
	glm::mat4 headMatrix = getMatrix(0, 0, 0.55, 0.f, 0, 0, 1);
	listeMvp.push_back(cameraMatrix * bodyMatrix * headMatrix);

	Sphere shoulder1(32, 32);
	listeFigures.push_back(shoulder1);
	listeBuffer.push_back(generate(shoulder1));
	// glm::mat4 shoulder1Matrix = getMatrix(-0.32, 0, 0.3, -M_PI / 6.f, 1, 0, 0); // default : bras le long du corps
	glm::mat4 shoulder1Matrix = getMatrix(-0.32, 0, 0.3, 1.f, 1, 0, 0);			   // inclinaison initiale de 57deg (1rad)
	listeMvp.push_back(cameraMatrix * bodyMatrix * shoulder1Matrix);

	Cylinder arm1(32);
	listeFigures.push_back(arm1);
	listeBuffer.push_back(generate(arm1));
	glm::mat4 arm1Matrix = getMatrix(0, 0, -0.2, 0, 1, 0, 0);
	listeMvp.push_back(cameraMatrix * bodyMatrix * shoulder1Matrix * arm1Matrix);

	Sphere elbow1(32, 32);
	listeFigures.push_back(elbow1);
	listeBuffer.push_back(generate(elbow1));
	glm::mat4 elbow1Matrix = getMatrix(0, 0, -0.2, 0.5f, 1, 0, 0);				// légère inclinaison supplémentaire
	listeMvp.push_back(cameraMatrix * bodyMatrix * shoulder1Matrix * arm1Matrix * elbow1Matrix);

	Cylinder forearm1(32);
	listeFigures.push_back(forearm1);
	listeBuffer.push_back(generate(forearm1));
	glm::mat4 forearm1Matrix = getMatrix(0, 0, -0.2, 0, 1, 0, 0);
	listeMvp.push_back(cameraMatrix * bodyMatrix * shoulder1Matrix * arm1Matrix * elbow1Matrix * forearm1Matrix);
	
	Sphere shoulder2(32, 32);
	listeFigures.push_back(shoulder2);
	listeBuffer.push_back(generate(shoulder2));
	glm::mat4 shoulder2Matrix = getMatrix(0.32, 0, 0.3, M_PI / 12.f, 1, 0, 0);
	listeMvp.push_back(cameraMatrix * bodyMatrix * shoulder2Matrix);

	Cylinder arm2(32);
	listeFigures.push_back(arm2);
	listeBuffer.push_back(generate(arm2));
	glm::mat4 arm2Matrix = getMatrix(0, 0, -0.2, 0, 1, 0, 0);
	listeMvp.push_back(cameraMatrix * bodyMatrix * shoulder2Matrix * arm2Matrix);

	Sphere elbow2(32, 32);
	listeFigures.push_back(elbow2);
	listeBuffer.push_back(generate(elbow2));
	glm::mat4 elbow2Matrix = getMatrix(0, 0, -0.2, M_PI/12.f, 1, 0, 0);
	listeMvp.push_back(cameraMatrix * bodyMatrix * shoulder2Matrix * arm2Matrix * elbow2Matrix);

	Cylinder forearm2(32);
	listeFigures.push_back(forearm2);
	listeBuffer.push_back(generate(forearm2));
	glm::mat4 forearm2Matrix = getMatrix(0, 0, -0.2, 0, 1, 0, 0);
	listeMvp.push_back(cameraMatrix * bodyMatrix * shoulder2Matrix * arm2Matrix * elbow2Matrix * forearm2Matrix);

	// listeMvp.push_back(cameraMatrix * bodyMatrix * shoulder2Matrix * arm2Matrix * elbow2Matrix * forearm2Matrix * torchMatrix ????);

	Cylinder thigh1(32);
	listeFigures.push_back(thigh1);
	listeBuffer.push_back(generate(thigh1));
	glm::mat4 thigh1Matrix = getMatrix(-0.15, 0, -0.55, M_PI/9.f, 1, 0, 0);
	listeMvp.push_back(cameraMatrix * bodyMatrix * thigh1Matrix);

	Sphere knee1(32, 32);
	listeFigures.push_back(knee1);
	listeBuffer.push_back(generate(knee1));
	glm::mat4 knee1Matrix = getMatrix(0, 0, -0.2, 0.f, 0, 0, 1);
	listeMvp.push_back(cameraMatrix * bodyMatrix * thigh1Matrix * knee1Matrix);

	Cylinder leg1(32);
	listeFigures.push_back(leg1);
	listeBuffer.push_back(generate(leg1));
	glm::mat4 leg1Matrix = getMatrix(0, 0, -0.2, 0, 1, 0, 0);
	listeMvp.push_back(cameraMatrix * bodyMatrix * thigh1Matrix * knee1Matrix * leg1Matrix);

	Sphere foot1(32, 32);
	listeFigures.push_back(foot1);
	listeBuffer.push_back(generate(foot1));
	glm::mat4 foot1Matrix = getMatrix(0, 0.1, -0.2, 0.f, 0, 0, 1);
	listeMvp.push_back(cameraMatrix * bodyMatrix * thigh1Matrix * knee1Matrix * leg1Matrix * foot1Matrix);

	Cylinder thigh2(32);
	listeFigures.push_back(thigh2);
	listeBuffer.push_back(generate(thigh2));
	glm::mat4 thigh2Matrix = getMatrix(0.15, 0, -0.55, -M_PI/9.f, 1, 0, 0);
	listeMvp.push_back(cameraMatrix * bodyMatrix * thigh2Matrix);

	Sphere knee2(32, 32);
	listeFigures.push_back(knee2);
	listeBuffer.push_back(generate(knee2));
	glm::mat4 knee2Matrix = getMatrix(0, 0, -0.2, -M_PI / 9.f, 1, 0, 0);
	listeMvp.push_back(cameraMatrix * bodyMatrix * thigh2Matrix * knee2Matrix);

	Cylinder leg2(32);
	listeFigures.push_back(leg2);
	listeBuffer.push_back(generate(leg2));
	glm::mat4 leg2Matrix = getMatrix(0, 0, -0.2, 0, 1, 0, 0);
	listeMvp.push_back(cameraMatrix * bodyMatrix * thigh2Matrix * knee2Matrix * leg2Matrix);

	Sphere foot2(32, 32);
	listeFigures.push_back(foot2);
	listeBuffer.push_back(generate(foot2));
	glm::mat4 foot2Matrix = getMatrix(0, 0.1, -0.2, 0.f, 0, 0, 1);
	listeMvp.push_back(cameraMatrix * bodyMatrix * thigh2Matrix * knee2Matrix * leg2Matrix * foot2Matrix);

	//on scale tous les objets

	listeMvp[0] = listeMvp[0] * scaleMatrix(0.5, 0.5, 0.8);
	listeMvp[1] = listeMvp[1] * scaleMatrix(0.3, 0.3, 0.3);
	listeMvp[2] = listeMvp[2] * scaleMatrix(0.2, 0.2, 0.2);
	listeMvp[3] = listeMvp[3] * scaleMatrix(0.1, 0.1, 0.25);
	listeMvp[4] = listeMvp[4] * scaleMatrix(0.2, 0.2, 0.2);
	listeMvp[5] = listeMvp[5] * scaleMatrix(0.1, 0.1, 0.25);

	listeMvp[6] = listeMvp[6] * scaleMatrix(0.2, 0.2, 0.2);
	listeMvp[7] = listeMvp[7] * scaleMatrix(0.1, 0.1, 0.25);
	listeMvp[8] = listeMvp[8] * scaleMatrix(0.2, 0.2, 0.2);
	listeMvp[9] = listeMvp[9] * scaleMatrix(0.1, 0.1, 0.25);
	listeMvp[10] = listeMvp[10] * scaleMatrix(0.15, 0.15, 0.38);
	listeMvp[11] = listeMvp[11] * scaleMatrix(0.2, 0.2, 0.2);
	listeMvp[12] = listeMvp[12] * scaleMatrix(0.15, 0.15, 0.38);
	listeMvp[13] = listeMvp[13] * scaleMatrix(0.2, 0.4, 0.2);
	listeMvp[14] = listeMvp[14] * scaleMatrix(0.15, 0.15, 0.38);
	listeMvp[15] = listeMvp[15] * scaleMatrix(0.2, 0.2, 0.2);
	listeMvp[16] = listeMvp[16] * scaleMatrix(0.15, 0.15, 0.38);
	listeMvp[17] = listeMvp[17] * scaleMatrix(0.2, 0.4, 0.2);


    //From here you can load your OpenGL objects, like VBO, Shaders, etc.
    
	//On charge les fichiers relatifs aux shaders
    FILE* vertFile = fopen("Shaders/color.vert", "r");
    FILE* fragFile = fopen("Shaders/color.frag", "r");
    if (vertFile == NULL || fragFile == NULL) {
		return EXIT_FAILURE;
    }
    Shader* shader = Shader::loadFromFiles(vertFile, fragFile);

    fclose(vertFile);
    fclose(fragFile);

    if (shader == NULL)
    {
		ERROR("Cannot build Shader...\n");
		return EXIT_FAILURE;
    }


	////////////// Lumiere / Material ///////////////////////

	Material material = Material();

	// on se sert des 3 premieres dimensions de cette matrice pour calculer
	// la position de la lumière au bout du bras (de la lampe prochainement)
	glm::mat4 tempoMat = getMatrix(0, 0, -0.2, 0, 1, 0, 0);
	tempoMat = (cameraMatrix * bodyMatrix * shoulder1Matrix * arm1Matrix * elbow1Matrix * forearm1Matrix * tempoMat);

	glm::vec3 lightPos = glm::vec3(tempoMat[0][0], tempoMat[1][1], tempoMat[2][2]);

	glm::vec3 lightColorBase = glm::vec3(255.0f, 211.0f, 1.0f);
	// Light light = Light(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));	// white light
	// Light light = Light(glm::vec3(0.0f, 0.0f, 0.0f), lightColorBase);	//yellow light
	Light light = Light(lightPos, lightColorBase);	//position custom


	//////////////////////////////////////////////////////////////////////////////////////FIN_PARTIE_ELEVE////////////////////////////////////////////////////////////////////////////////////

    bool isOpened = true;

	//Ici, on instancie des variables qui vont servir à l'animation
	int t = 0; //incrémenté à chaque tour de boucle

	int side = 1; //pour changer le sens de rotation ( side * angle ) ; side inclus dans {-1, 1}
	int timer = 60; //temps avant le changement de sens de rotation, en tours de boucle

	int amplitudeArmLR, amplitudeArmUD = 50; //pour limiter rotation du bras en x et en y ; compris dans [0,100]

	displayCommands();

    //Main application loop
    while(isOpened)
    {
        //Time in ms telling us when this frame started. Useful for keeping a fix framerate
        uint32_t timeBegin = SDL_GetTicks();

		//pour changer dir gauche/droite et haut/bas du bras ; inclus dans {-1, 0, 1}
		int armDirectionLR, armDirectionUD = 0;

        //Fetch the SDL events
        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
			// Allume/éteint la lumière au clic (prochainement la lampe torche)
			if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT)) { light.toogle(); }

            switch(event.type)
            {
                case SDL_WINDOWEVENT:
                    switch(event.window.event)
                    {
                        case SDL_WINDOWEVENT_CLOSE:
                            isOpened = false;
							printf("\n\nArret du programme..\n");
                            break;
                        default:
                            break;
                    }
                    break;


				case SDL_KEYDOWN:
					/* Check the SDLKey values and move change the coords */
					switch (event.key.keysym.sym) {
						case SDLK_LEFT:
							cameraMatrix = glm::translate(cameraMatrix, glm::vec3(-0.1f, 0.0f, 0.0f));
							break;
						case SDLK_RIGHT:
							cameraMatrix = glm::translate(cameraMatrix, glm::vec3(0.1f, 0.0f, 0.0f));
							break;
						case SDLK_UP:
							cameraMatrix = glm::translate(cameraMatrix, glm::vec3(0.f, 0.1f, 0.f));
							break;
						case SDLK_DOWN:
							cameraMatrix = glm::translate(cameraMatrix, glm::vec3(0.f, -0.1f, 0.f));
							break;

						case SDLK_q:
							light.pos += glm::vec3(-0.1f, 0.f, 0.f);	// deplace lumière à gauche
							break;
						case SDLK_d:
							light.pos += glm::vec3(0.1f, 0.f, 0.f);		// deplace lumière à droite
							break;
						case SDLK_z:
							light.pos += glm::vec3(0.f, 0.1f, 0.f);		// deplace lumière en haut
							break;
						case SDLK_s:
							light.pos += glm::vec3(0.f, -0.1f, 0.f);	// deplace lumière en bas
							break;


						/* TODO: Adjust to a "global scene zoom" (see glm::perspective or glm::lookAt..) */
						case SDLK_KP_PLUS:
							cameraMatrix = glm::translate(cameraMatrix, glm::vec3(0.f, 0.0f, 0.1f));
							// cameraMatrix = glm::scale(cameraMatrix, glm::vec3(sx, sy, sz));
							break;
						case SDLK_KP_MINUS:
							cameraMatrix = glm::translate(cameraMatrix, glm::vec3(0.f, 0.0f, -0.1f));
							break;

						default:
							break;
					}
					break;

				case SDL_MOUSEMOTION:
					/*if (event.motion.xrel)
					{
						if (event.motion.xrel <= 0)
						{
							armDirectionLR = 1;
							amplitudeArmLR++;
						}
						else {
							armDirectionLR = -1;
							amplitudeArmLR--;
						}
						printf("amplitudeArmLR : %d.\n", amplitudeArmLR);
					}*/
					if (event.motion.yrel <= 0)
					{
						if (amplitudeArmUD < 100) {
							armDirectionUD = 1;
							amplitudeArmUD++;
						}
					}
					else {
						if (amplitudeArmUD > 0) {
							armDirectionUD = -1;
							amplitudeArmUD--;
						}
					}
					//printf("amplitudeArmUD : %d.\n", amplitudeArmUD);
					// printf("x=%d, y=%d.\n", event.motion.x, event.motion.y);
					break;
            }
        }

        //Clear the screen : the depth buffer and the color buffer
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	//////////////////////////////////////////////////////////////////////////////////////////PARTIE_ELEVE////////////////////////////////////////////////////////////////////////////////////
		// A chaque tour de boucle on décrémente le timer
		if (timer > 0)
		{
			timer--;
			if (timer == 0)
			{
				side = -side; //on change le sens de rotation
				timer = 60; //on réinitialise le timer
			}
		}

		//TODO operations on matrix
		// On réinitialise les données de la figure principale (le corps)
		bodyMatrix = getMatrix(0, -0.3, 0, -M_PI / 2.f, 1, 0, 0);



		/*
			Ces deux lignes ne sont pas obligatoires, elles permettent de faire tourner le personnage sur lui-même

			NOTE : Utile plus tard pour le déplacement du personnage ?
		 */
		//glm::mat4 bodyMatrixRot = glm::rotate(headMatrix, (t++) / 60.0f, glm::vec3(0, 0, 1));
		//bodyMatrix = bodyMatrix * bodyMatrixRot;



		// On recalcule toutes les matrices en fonction de la nouvelle position du corps et de la rotation des articulations 
		// Haut du corps
		listeMvp[0] = cameraMatrix * bodyMatrix;
		listeMvp[1] = cameraMatrix * bodyMatrix * headMatrix;

		// BRAS DROIT (CELUI QU'ON DIRIGE)
		//shoulder1Matrix = glm::rotate(shoulder1Matrix, armDirectionUD * (float)M_PI / 120.f, glm::vec3(1, 0, 0));
		// shoulder1Matrix = glm::rotate(shoulder1Matrix, armDirectionLR * (float)M_PI / 120.f, glm::vec3(0, 0, 1));
		listeMvp[2] = cameraMatrix * bodyMatrix * shoulder1Matrix;
		listeMvp[3] = cameraMatrix * bodyMatrix * shoulder1Matrix * arm1Matrix;
		//elbow1Matrix = glm::rotate(elbow1Matrix, armDirectionUD * (float)M_PI / 360.f, glm::vec3(1, 0, 0));
		// elbow1Matrix = glm::rotate(elbow1Matrix, armDirectionLR * (float)M_PI / 360.f, glm::vec3(0, 0, 1));
		listeMvp[4] = cameraMatrix * bodyMatrix * shoulder1Matrix * arm1Matrix * elbow1Matrix;
		listeMvp[5] = cameraMatrix * bodyMatrix * shoulder1Matrix * arm1Matrix * elbow1Matrix * forearm1Matrix;


		// listeMvp[6] = cameraMatrix * bodyMatrix * shoulder1Matrix * arm1Matrix * elbow1Matrix * forearm1Matrix *  torchMatrix ????;
		// Ici futur emplacement de la lampe
		//tempoMat = (cameraMatrix * bodyMatrix * shoulder1Matrix * arm1Matrix * elbow1Matrix * forearm1Matrix * tempoMat);
		//light.pos = glm::vec3(tempoMat[0][0], tempoMat[1][1], tempoMat[2][2]);


		// BRAS GAUCHE (AU REPOS)
		// shoulder2Matrix = glm::rotate(shoulder2Matrix, -side * (float)M_PI / 240.f, glm::vec3(1, 0, 0));
		listeMvp[6] = cameraMatrix * bodyMatrix * shoulder2Matrix;
		listeMvp[7] = cameraMatrix * bodyMatrix * shoulder2Matrix * arm2Matrix;
		// elbow2Matrix = glm::rotate(elbow2Matrix, -side * (float)M_PI / 720.f, glm::vec3(1, 0, 0));
		listeMvp[8] = cameraMatrix * bodyMatrix * shoulder2Matrix * arm2Matrix * elbow2Matrix;
		listeMvp[9] = cameraMatrix * bodyMatrix * shoulder2Matrix * arm2Matrix * elbow2Matrix * forearm2Matrix;

		// Bas du corps
		/*
			TODO : Conditionner le mouvement des jambes à l'avancement du personnage
					(une fois que le déplacement du personnage sera géré)
		 */
		//thigh1Matrix = glm::rotate(thigh1Matrix, -side * (float)(2 * M_PI) / 540.f, glm::vec3(1, 0, 0));
		listeMvp[10] = cameraMatrix * bodyMatrix * thigh1Matrix;
		//knee1Matrix = glm::rotate(knee1Matrix, -side * (float) M_PI / 540.f, glm::vec3(1, 0, 0));
		listeMvp[11] = cameraMatrix * bodyMatrix * thigh1Matrix * knee1Matrix;
		listeMvp[12] = cameraMatrix * bodyMatrix * thigh1Matrix * knee1Matrix * leg1Matrix;
		listeMvp[13] = cameraMatrix * bodyMatrix * thigh1Matrix * knee1Matrix * leg1Matrix * foot1Matrix;
		//thigh2Matrix = glm::rotate(thigh2Matrix, side *(float)(2 * M_PI) / 540.f, glm::vec3(1, 0, 0));
		listeMvp[14] = cameraMatrix * bodyMatrix * thigh2Matrix;
		//knee2Matrix = glm::rotate(knee2Matrix, side *(float)M_PI / 540.f, glm::vec3(1, 0, 0));
		listeMvp[15] = cameraMatrix * bodyMatrix * thigh2Matrix * knee2Matrix;
		listeMvp[16] = cameraMatrix * bodyMatrix * thigh2Matrix * knee2Matrix * leg2Matrix;
		listeMvp[17] = cameraMatrix * bodyMatrix * thigh2Matrix * knee2Matrix * leg2Matrix * foot2Matrix;
		
		//on oublie pas de rescale

		listeMvp[0] = listeMvp[0] * scaleMatrix(0.5, 0.5, 0.8);
		listeMvp[1] = listeMvp[1] * scaleMatrix(0.3, 0.3, 0.3);
		listeMvp[2] = listeMvp[2] * scaleMatrix(0.2, 0.2, 0.2);
		listeMvp[3] = listeMvp[3] * scaleMatrix(0.1, 0.1, 0.25);
		listeMvp[4] = listeMvp[4] * scaleMatrix(0.2, 0.2, 0.2);
		listeMvp[5] = listeMvp[5] * scaleMatrix(0.1, 0.1, 0.25);
		listeMvp[6] = listeMvp[6] * scaleMatrix(0.2, 0.2, 0.2);
		listeMvp[7] = listeMvp[7] * scaleMatrix(0.1, 0.1, 0.25);
		listeMvp[8] = listeMvp[8] * scaleMatrix(0.2, 0.2, 0.2);
		listeMvp[9] = listeMvp[9] * scaleMatrix(0.1, 0.1, 0.25);
		listeMvp[10] = listeMvp[10] * scaleMatrix(0.15, 0.15, 0.38);
		listeMvp[11] = listeMvp[11] * scaleMatrix(0.2, 0.2, 0.2);
		listeMvp[12] = listeMvp[12] * scaleMatrix(0.15, 0.15, 0.38);
		listeMvp[13] = listeMvp[13] * scaleMatrix(0.2, 0.4, 0.2);
		listeMvp[14] = listeMvp[14] * scaleMatrix(0.15, 0.15, 0.38);
		listeMvp[15] = listeMvp[15] * scaleMatrix(0.2, 0.2, 0.2);
		listeMvp[16] = listeMvp[16] * scaleMatrix(0.15, 0.15, 0.38);
		listeMvp[17] = listeMvp[17] * scaleMatrix(0.2, 0.4, 0.2);


		
        // Rendering
        glUseProgram(shader->getProgramID());
        {
			//on dessine toutes nos figures. Notez l'importance d'avoir les bons index et le même nombre d'éléments dans chaque liste
			for (int i = 0; i < listeFigures.size(); i++)
			{
				try
				{
					draw(listeBuffer[i], listeFigures[i].getNbVertices(), shader, listeMvp[i], material, light, textureID);
				}
				catch (...)
				{
					return EXIT_FAILURE;
				}
			}


			/* Render flashlight */
			Material torchMat = Material(glm::vec3(1.f, 0.f, 0.0f));	// red to better distinguish it

			// TODO : parse the Flashlight.mtl to retrieve Ka, Kd, Ks ???

			glm::mat4 matrixTorch = scaleMatrix(0.03f, 0.03f, 0.03f);
			matrixTorch = glm::translate(matrixTorch, glm::vec3(-20.f, -8.f, 0.0f));
			matrixTorch = glm::rotate(matrixTorch, (float)M_PI, glm::vec3(0, 1, 0));
			matrixTorch = glm::rotate(matrixTorch, -0.2f, glm::vec3(1, 0, 0));

			glm::mat4 mvpTorch = cameraMatrix * matrixTorch;
			uint32_t flashlight_verticesNB = flashlight_vertices.size();
			if (resTorch) {
				try
				{
					draw(torchBuffer, flashlight_verticesNB, shader, mvpTorch, torchMat, light, textureID);
				}
				catch (...)
				{
					return EXIT_FAILURE;
				}
			}


			/* Render BACKGROUND */
			// If the background material color is different of 'material' (the one used for
			// the animated character), the light will not reflect on it... (?)
			Material bgMat = Material(glm::vec3(0.f, 1.f, 0.f));

			glm::mat4 matrixBackground = scaleMatrix(2.0f, 2.0f, 2.0f);
			glm::mat4 mvpBackground = cameraMatrix * matrixBackground;

			try
			{
				draw(bgBuffer, nbBgVertices, shader, mvpBackground, bgMat, light, textureID);
			}
			catch (...)
			{
				return EXIT_FAILURE;
			}

        }

        glUseProgram(0);


		//////////////////////////////////////////////////////////////////////////////////////FIN_PARTIE_ELEVE////////////////////////////////////////////////////////////////////////////////////

        //Display on screen (swap the buffer on screen and the buffer you are drawing on)
        SDL_GL_SwapWindow(window);

        //Time in ms telling us when this frame ended. Useful for keeping a fix framerate
        uint32_t timeEnd = SDL_GetTicks();

        //We want FRAMERATE FPS
        if(timeEnd - timeBegin < TIME_PER_FRAME_MS)
            SDL_Delay(TIME_PER_FRAME_MS - (timeEnd - timeBegin));
    }
    
    //Free everything
	delete shader;
	for each (GLuint buff in listeBuffer)
	{
		glDeleteBuffers(1, &buff);
	}
	
	glDeleteBuffers(1, &bgBuffer);
	glDeleteBuffers(1, &torchBuffer);

	glDeleteTextures(1, &textureID);

    if(context != NULL)
        SDL_GL_DeleteContext(context);
    if(window != NULL)
        SDL_DestroyWindow(window);

    return 0;
}