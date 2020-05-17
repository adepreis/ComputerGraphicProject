// SDL Libraries
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

// OpenGL Libraries
#include <GL/glew.h>
#include <GL/gl.h>

// GML libraries
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"

#include "logger.h"

#include "functions.h"
#include "structures.h"
#include "OBJLoader.h"

// objects 3D
#include "Sphere.h"
#include "Cube.h"
#include "Cylinder.h"

#include "Wall.h"

// libraries supplementaires
#include "vector"
#include "math.h"

// On definit une fenetre carree pour eviter tout probleme de rotation ou scaling.
#define WIDTH     700
#define HEIGHT    700
#define FRAMERATE 60
#define TIME_PER_FRAME_MS  (1.0f/FRAMERATE * 1e3)
#define INDICE_TO_PTR(x) ((void*)(x))

int main(int argc, char *argv[])
{
    /////////////////////////////////////////
    // SDL2 / OpenGL Context initialization : 
    /////////////////////////////////////////
    
    // Initialize SDL2
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0)
    {
        ERROR("The initialization of the SDL failed : %s\n", SDL_GetError());
        return 0;
    }

	// Init the IMG component
	if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG))
	{
		ERROR("Could not load SDL2_image with PNG files\n");
		return EXIT_FAILURE;
	}

    // Create a Window
    SDL_Window* window = SDL_CreateWindow("FPS Flashlight - Projet Info Graphique",		// Titre
                                          SDL_WINDOWPOS_UNDEFINED,               // X Position
                                          SDL_WINDOWPOS_UNDEFINED,               // Y Position
                                          WIDTH, HEIGHT,                         // Resolution
                                          SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN); // Flags (OpenGL + Show)

    // Initialize OpenGL Version (version 3.0)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    // Initialize the OpenGL Context (where OpenGL resources (Graphics card resources) lives)
    SDL_GLContext context = SDL_GL_CreateContext(window);

    // Tells GLEW to initialize the OpenGL function with this version
    glewExperimental = GL_TRUE;
    glewInit();


    // Start using OpenGL to draw something on screen
    glViewport(0, 0, WIDTH, HEIGHT); // Draw on ALL the screen

    // The OpenGL background color (RGBA, each component between 0.0f and 1.0f)
	// glClearColor(0.0, 0.0, 0.0, 1.0);			// Full Black
	glClearColor(0.2, 0.2, 0.2, 1.0);			// Light grey
	// glClearColor(128.0, 0.0, 128.0, 1.0);		// Pink

    glEnable(GL_DEPTH_TEST);	// Active the depth test



	// =========================== Flashlight model loading ==============================================
	std::vector<glm::vec3> flashlight_vertices;
	std::vector<glm::vec2> flashlight_uvs;
	std::vector<glm::vec3> flashlight_normals; // Won't be used at the moment.

	// Resultat de l'importation du modele de lampe torche :
	bool resTorch = false;

	/* This model is not adapted to our parser (but could offer more options (nMap, texture,...) : */
	// resTorch = loadOBJ("../../Models/flashlight_model/flashlight.obj", flashlight_vertices, flashlight_uvs, flashlight_normals);

	/* This model is just a test model adapted to our parser : */
	// resTorch = loadOBJ("../../Models/cube_test.obj", flashlight_vertices, flashlight_uvs, flashlight_normals);

	/* This one is a adapted model using Blender : */
	resTorch = loadOBJ("../../Models/flashLight/Flashlight.obj", flashlight_vertices, flashlight_uvs, flashlight_normals);

	if (!resTorch)
	{
		ERROR("Could not load flashlight model..\n");
		return EXIT_FAILURE;
	}

	// =========================== Background wall init (& floor ?) ==============================================

	Wall wall = Wall();
	GLuint wallBuffer = NULL;	// generate background wall buffer
	wallBuffer = createBuffer(wallBuffer, wall.getNormals(), wall.getVertices(), wall.getNbVertices());


	// Test floor using Immediate Mode :

	// glColorMaterial(GL_FRONT, GL_DIFFUSE);
	// glEnable(GL_COLOR_MATERIAL);
	// glBegin(GL_TRIANGLES);
	// 	  glVertex3f(0.5, 0.0, 0.0);
	// 	  glVertex3f(0.0, 0.0, 0.0);
	// 	  glVertex3f(0.0, 0.0, 0.5);
	// 
	// 	  glVertex3f(-0.5, 0.0, 0.0);
	// 	  glVertex3f(0.0, 0.0, 0.0);
	// 	  glVertex3f(0.0, 0.0, -0.5);
	// glEnd();


	// =========================== Texture loading ==============================================
	GLuint textureMercure = createTexture("../../Images/mercure.png");
	GLuint textureWall = createTexture("../../Images/wall.jpg");
	GLuint textureTorch = createTexture("../../Images/steel.jpg");


	/*
		Matrice de la camera
	*/
	glm::mat4 cameraMatrix(1.0f);

	cameraMatrix = glm::rotate(cameraMatrix, (float)M_PI, glm::vec3(0, 1, 0));				// place la cam derriere le perso
	// cameraMatrix = glm::rotate(cameraMatrix, -0.5f*(float)M_PI, glm::vec3(1, 0, 0));		// place la cam au dessus
	// cameraMatrix = glm::translate(cameraMatrix, glm::vec3(0.f, 0.f, 1.0f));				// tentative de reculer la cam


	// =========================== Background ==============================================
	// Background wall
	glm::mat4 matrixWall = scaleMatrix(1.8f, 1.8f, 1.5f);

	matrixWall = glm::rotate(matrixWall, 0.99f*(float)M_PI, glm::vec3(0, 1, 0));	// rot y : mur au fond : 2 murs sur les cotes
	matrixWall = glm::translate(matrixWall, glm::vec3(0.f, -0.3f, 0.1f));			// abaisse + recule legerement par rapport au perso

	glm::mat4 mvpWall = cameraMatrix * matrixWall;

	/*
		Ici, on va creer une par une toutes les figures qui composent notre personnage
			- ATTENTION : on veillera a ce que tous les objets lies a une meme figure soient au meme index dans toutes les listes
		Dans l'ordre :	- on instancie la figure
						- on l'ajoute a la liste des figures
						- on genere son buffer qu'on ajoute a la liste des buffers
						- on cree sa matrice sans prendre en compte le scaling par souci de simplification
						- on ajoute sa matrice a la liste des matrices. Attention a l'ordre des matrices. Chaque matrice doit dependre de la matrice a sa gauche, la camera etant le referentiel absolu

		On ne scale qu'une fois tous les objets crees afin de ne pas avoir besoin d'adapter le scale de tous les objets en fonction de celui des objets dont ils dependent
		On cree un premier cylindre qui sera le corps de notre personnage, l'angle de -pi / 2 permet d'orienter le cylindre comme souhaite.
		Attention, par defaut un cylindre fait face a la camera et ses faces plates sont invisibles.
		On retrouvera un angle par defaut sur les figures representant les epaules, coudes, cuisses et genoux car ce sont des articulations dans notre modele
		A l'exception des angles qui sont calcules selon les donnees du TP, toutes les valeurs ont ete trouvees par tatonnements
	*/

	std::vector <Geometry>	listeFigures;	// liste de toutes les figures creees
	std::vector <GLuint>	listeBuffer;	// liste des buffers associes aux figures
	std::vector <glm::mat4> listeMvp;		// liste des matrices associees aux figures

	Cylinder body(32);
	listeFigures.push_back(body);
	listeBuffer.push_back(generate(body));
	glm::mat4 bodyMatrix = getMatrix(0, -0.3, 0, -M_PI / 2.f, 1, 0, 0);
	listeMvp.push_back(cameraMatrix * bodyMatrix);

	Cube head = Cube();			// pas de problème de texture
	// Sphere head(32, 32);		// problème de texture
	listeFigures.push_back(head);
	listeBuffer.push_back(generate(head));
	glm::mat4 headMatrix = getMatrix(0, 0, 0.55, 0.f, 0, 0, 1);
	listeMvp.push_back(cameraMatrix * bodyMatrix * headMatrix);

	Sphere shoulder1(32, 32);
	listeFigures.push_back(shoulder1);
	listeBuffer.push_back(generate(shoulder1));
	glm::mat4 shoulder1Matrix = getMatrix(-0.32, 0, 0.3, 1.2f, 1, 0, 0);		// inclinaison initiale de 68.7deg (1.2rad) (-M_PI / 6.f pour le bras le long du corps)
	listeMvp.push_back(cameraMatrix * bodyMatrix * shoulder1Matrix);

	Cylinder arm1(32);
	listeFigures.push_back(arm1);
	listeBuffer.push_back(generate(arm1));
	glm::mat4 arm1Matrix = getMatrix(0, 0, -0.2, 0, 1, 0, 0);
	listeMvp.push_back(cameraMatrix * bodyMatrix * shoulder1Matrix * arm1Matrix);

	Sphere elbow1(32, 32);
	listeFigures.push_back(elbow1);
	listeBuffer.push_back(generate(elbow1));
	glm::mat4 elbow1Matrix = getMatrix(0, 0, -0.2, 0.5f, 1, 0, 0);				// legere inclinaison supplementaire
	listeMvp.push_back(cameraMatrix * bodyMatrix * shoulder1Matrix * arm1Matrix * elbow1Matrix);

	Cylinder forearm1(32);
	listeFigures.push_back(forearm1);
	listeBuffer.push_back(generate(forearm1));
	glm::mat4 forearm1Matrix = getMatrix(0, 0, -0.2, 0, 1, 0, 0);
	listeMvp.push_back(cameraMatrix * bodyMatrix * shoulder1Matrix * arm1Matrix * elbow1Matrix * forearm1Matrix);


	//// Create torch's buffer, matrix and MVP separately because it isn't compatible with listeFigures's element type.. ////
	uint32_t nbVerticesFlashlight = flashlight_vertices.size();
	GLuint torchBuffer = NULL; 	// Flashlight buffer

	// cast obligatoire car flashlight_vertices & flashlight_normal ne sont pas des const float * :
	torchBuffer = createBuffer(torchBuffer, glm::value_ptr(flashlight_vertices[0]), glm::value_ptr(flashlight_normals[0]), nbVerticesFlashlight);

	glm::mat4 matrixTorch = getMatrix(0.0, 0.02, -0.15, (float)M_PI, 0, 1, 0);
	matrixTorch = glm::rotate(matrixTorch, -0.5f, glm::vec3(1, 0, 0));

	glm::mat4 mvpTorch = (cameraMatrix * bodyMatrix * shoulder1Matrix * arm1Matrix * elbow1Matrix * forearm1Matrix * matrixTorch);
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	
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


	// on scale tous les objets
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

	// Flashlight scaling
	mvpTorch = mvpTorch * scaleMatrix(0.03, 0.03, 0.03);

    
	// On charge les fichiers relatifs aux shaders
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


	/////////////////// Lumiere ///////////////////////

	// on se sert de cette matrice pour calculer
	// la position de la lumiere au bout du bras
	glm::mat4 tempoMat = getMatrix(0, 0, -0.2, 0, 1, 0, 0);

	glm::vec3 lightColorBase = glm::vec3(1.0f);
	// Light light = Light(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));	// white light
	// Light light = Light(glm::vec3(0.0f, 0.0f, 0.0f), lightColorBase);				// yellow light
	Light light = Light(glm::vec3(1.0), lightColorBase);	// position custom



	////////////////  Materiau ///////////////////////

	Material bodyMaterial = Material();

	// the light didn't seems to reflect on the wall... (?)
	Material wallMaterial = Material(glm::vec3(0.58f, 0.4f, 0.42f));

	// TODO : parse the Flashlight.mtl to retrieve Ka, Kd, Ks ???

	// Light grey Torch's material
	Material torchMat = Material(glm::vec3(0.08f, 0.08f, 0.08f), 0.f, 0.5f, 1.f, 250.f);



    bool isOpened = true;

	// Ici, on instancie des variables qui vont servir a l'animation
	int t = 0;	// incremente a chaque tour de boucle

	int side = 1;	// pour changer le sens de rotation ( side * angle ) ; side inclus dans {-1, 1}
	int timer = 60; // temps avant le changement de sens de rotation, en tours de boucle

	int amplitudeArmLR, amplitudeArmUD = 50; // pour limiter rotation du bras en x et en y ; compris dans [0,100]

	displayCommands();

    // Main application loop
    while(isOpened)
    {
        // Time in ms telling us when this frame started. Useful for keeping a fix framerate
        uint32_t timeBegin = SDL_GetTicks();

		// pour changer dir gauche/droite et haut/bas du bras ; inclus dans {-1, 0, 1}
		int armDirectionLR, armDirectionUD = 0;

        // Fetch the SDL events
        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
			// Allume/eteint la lumiere de la lampe au clic
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
							light.pos += glm::vec3(-0.1f, 0.f, 0.f);	// deplace lumiere a gauche
							break;
						case SDLK_d:
							light.pos += glm::vec3(0.1f, 0.f, 0.f);		// deplace lumiere a droite
							break;
						case SDLK_z:
							light.pos += glm::vec3(0.f, 0.1f, 0.f);		// deplace lumiere en haut
							break;
						case SDLK_s:
							light.pos += glm::vec3(0.f, -0.1f, 0.f);	// deplace lumiere en bas
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
					/*	X arm movement (later) :

						if (event.motion.xrel)
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
						}
					*/
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
					break;
            }
        }

        // Clear the screen : the depth buffer and the color buffer
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);


		// A chaque tour de boucle on decremente le timer
		if (timer > 0) {
			timer--;
			if (timer == 0) {
				side = -side; 	// on change le sens de rotation
				timer = 60; 	// on reinitialise le timer
			}
		}

		//////////////////////////////////// Operations on matrices ////////////////////////////////////

		// On reinitialise les donnees de la figure principale (le corps)
		bodyMatrix = getMatrix(0, -0.3, 0, -M_PI / 2.f, 1, 0, 0);


		/*
			Ces deux lignes ne sont pas obligatoires, elles permettent de faire tourner le personnage sur lui-meme

			NOTE : Utile plus tard pour le deplacement du personnage ?
		 */
		// glm::mat4 bodyMatrixRot = glm::rotate(headMatrix, (t++) / 60.0f, glm::vec3(0, 0, 1));
		// bodyMatrix = bodyMatrix * bodyMatrixRot;



		// On recalcule toutes les matrices en fonction de la nouvelle position du corps et de la rotation des articulations 
		// Haut du corps
		listeMvp[0] = cameraMatrix * bodyMatrix;
		listeMvp[1] = cameraMatrix * bodyMatrix * headMatrix;

		// BRAS DROIT (CELUI QU'ON DIRIGE)
		shoulder1Matrix = glm::rotate(shoulder1Matrix, armDirectionUD * (float)M_PI / 120.f, glm::vec3(1, 0, 0));
		// shoulder1Matrix = glm::rotate(shoulder1Matrix, armDirectionLR * (float)M_PI / 120.f, glm::vec3(0, 0, 1));
		listeMvp[2] = cameraMatrix * bodyMatrix * shoulder1Matrix;
		listeMvp[3] = cameraMatrix * bodyMatrix * shoulder1Matrix * arm1Matrix;
		elbow1Matrix = glm::rotate(elbow1Matrix, armDirectionUD * (float)M_PI / 360.f, glm::vec3(1, 0, 0));
		// elbow1Matrix = glm::rotate(elbow1Matrix, armDirectionLR * (float)M_PI / 360.f, glm::vec3(0, 0, 1));
		listeMvp[4] = cameraMatrix * bodyMatrix * shoulder1Matrix * arm1Matrix * elbow1Matrix;
		listeMvp[5] = cameraMatrix * bodyMatrix * shoulder1Matrix * arm1Matrix * elbow1Matrix * forearm1Matrix;


		// Emplacement de la lumière de la lampe
		tempoMat = (cameraMatrix * bodyMatrix * shoulder1Matrix * arm1Matrix * elbow1Matrix * forearm1Matrix * matrixTorch);
		light.pos = tempoMat[3];


		// BRAS GAUCHE (AU REPOS)
		// shoulder2Matrix = glm::rotate(shoulder2Matrix, -side * (float)M_PI / 240.f, glm::vec3(1, 0, 0));
		listeMvp[6] = cameraMatrix * bodyMatrix * shoulder2Matrix;
		listeMvp[7] = cameraMatrix * bodyMatrix * shoulder2Matrix * arm2Matrix;
		// elbow2Matrix = glm::rotate(elbow2Matrix, -side * (float)M_PI / 720.f, glm::vec3(1, 0, 0));
		listeMvp[8] = cameraMatrix * bodyMatrix * shoulder2Matrix * arm2Matrix * elbow2Matrix;
		listeMvp[9] = cameraMatrix * bodyMatrix * shoulder2Matrix * arm2Matrix * elbow2Matrix * forearm2Matrix;

		// Bas du corps
		/*
			TODO : Conditionner le mouvement des jambes a l'avancement du personnage
					(une fois que le deplacement du personnage sera gere)
		 */
		// thigh1Matrix = glm::rotate(thigh1Matrix, -side * (float)(2 * M_PI) / 540.f, glm::vec3(1, 0, 0));
		listeMvp[10] = cameraMatrix * bodyMatrix * thigh1Matrix;
		// knee1Matrix = glm::rotate(knee1Matrix, -side * (float) M_PI / 540.f, glm::vec3(1, 0, 0));
		listeMvp[11] = cameraMatrix * bodyMatrix * thigh1Matrix * knee1Matrix;
		listeMvp[12] = cameraMatrix * bodyMatrix * thigh1Matrix * knee1Matrix * leg1Matrix;
		listeMvp[13] = cameraMatrix * bodyMatrix * thigh1Matrix * knee1Matrix * leg1Matrix * foot1Matrix;
		// thigh2Matrix = glm::rotate(thigh2Matrix, side *(float)(2 * M_PI) / 540.f, glm::vec3(1, 0, 0));
		listeMvp[14] = cameraMatrix * bodyMatrix * thigh2Matrix;
		// knee2Matrix = glm::rotate(knee2Matrix, side *(float)M_PI / 540.f, glm::vec3(1, 0, 0));
		listeMvp[15] = cameraMatrix * bodyMatrix * thigh2Matrix * knee2Matrix;
		listeMvp[16] = cameraMatrix * bodyMatrix * thigh2Matrix * knee2Matrix * leg2Matrix;
		listeMvp[17] = cameraMatrix * bodyMatrix * thigh2Matrix * knee2Matrix * leg2Matrix * foot2Matrix;


		// compute flashlight and wall position
		mvpTorch = (cameraMatrix * bodyMatrix * shoulder1Matrix * arm1Matrix * elbow1Matrix * forearm1Matrix * matrixTorch);
		mvpWall = cameraMatrix * matrixWall;
		

		// on oublie pas de rescale
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

		// flashlight re-scaling
		mvpTorch = mvpTorch * scaleMatrix(0.03, 0.03, 0.03);

		
        glUseProgram(shader->getProgramID());
        {
			/* Figures rendering */
			for (int i = 0; i < listeFigures.size(); i++)
			{
				try
				{
					draw(listeBuffer[i], listeFigures[i].getNbVertices(), shader, listeMvp[i], bodyMaterial, light, textureMercure);
				}
				catch (...)
				{
					return EXIT_FAILURE;
				}
			}

			/* Flashlight rendering */
			if (resTorch) {
				try
				{
					draw(torchBuffer, nbVerticesFlashlight, shader, mvpTorch, torchMat, light, textureTorch);
				}
				catch (...)
				{
					return EXIT_FAILURE;
				}
			}

			/* BACKGROUND Wall rendering */
			try
			{
				draw(wallBuffer, wall.getNbVertices(), shader, mvpWall, wallMaterial, light, textureWall);
			}
			catch (...)
			{
				return EXIT_FAILURE;
			}
        }

        glUseProgram(0);


        // Display on screen (swap the buffer on screen and the buffer you are drawing on)
        SDL_GL_SwapWindow(window);

        // Time in ms telling us when this frame ended. Useful for keeping a fix framerate
        uint32_t timeEnd = SDL_GetTicks();

        // We want FRAMERATE FPS
        if(timeEnd - timeBegin < TIME_PER_FRAME_MS)
            SDL_Delay(TIME_PER_FRAME_MS - (timeEnd - timeBegin));
    }
    
    // Free everything
	delete shader;
	for each (GLuint buff in listeBuffer)
	{
		glDeleteBuffers(1, &buff);
	}
	
	glDeleteBuffers(1, &wallBuffer);
	glDeleteBuffers(1, &torchBuffer);

	glDeleteTextures(1, &textureMercure);
	glDeleteTextures(1, &textureWall);
	glDeleteTextures(1, &textureTorch);

    if(context != NULL)
        SDL_GL_DeleteContext(context);
    if(window != NULL)
        SDL_DestroyWindow(window);

    return 0;
}