# Computer Graphics Project

Polytech ET3 IT - Noted project for S6

## Objectives

Implement the different topics seen in class/TPs :

1. Scan conversion (lines, circles)
2. 2D/3D transformations, 3D object representations
3. Viewing/projections, hidden surface removal
4. Illumination, shading, texture mapping
5. Texture mapping continued, color and color models
6. Clipping/filling

## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development purpose.

### Prerequisites

Things you need to install the project :

- [cmake](https://cmake.org/download/) - portable version is enough
- [Visual Studio 15](https://visualstudio.microsoft.com/fr/vs/older-downloads/) - only 2017 edition works

### Installing

Here are some instructions on how to get the development env running.

First, clone this repository with the following command :
`git clone https://github.com/adepreis/ComputerGraphicProject`


Then, *using cmake-gui* :

You have to give cmake where the source code is, for example :

```
D:\...\ProjectFolder\ComputerGraphicProject
```

Then where to build the binaries, here in the corresponding path :

```
D:\...\ProjectFolder\ComputerGraphicProject\build
```

Then click 'Configure' twice : the first time you'll have to specify the 2017 version of Visual Studio.
Once the configuration is done, click on the 'Generate' button.
You will now find the `/build` folder in your project. Just execute the `ComputerGraphicProject.sln` in it in order to open the Visual Studio environment.

Get a coffee.

### Now, VS is fully loaded
In Solution Explorer, choose the solution node's context (right-click on ComputerGraphicProject) menu and then choose 'Set as StartUp Project'.

For first time use, you will have to make sure that in **'Debug' tab > 'Properties' > 'Debugging'**, the **'Working Directory'** is currently `$(ProjectDir)/bin`.

Now you could run the project after clicking the 'Start' button.
**(No additional parameters if you use the command line)**

There you are!

> ~~Note : Textures have to be in the `/build/bin/Images` folder !~~

> ~~Note : Objects have to be in the `/build/bin/Models` folder !~~


---

## Commands :
No controller needed, only a keyboard and a mouse.

To interact with our project, the keys to use are :
- `z` : move forward
- `s` : move backward
- `q` : move sideways to the left
- `d` : move sideways to the right
- `Left Click` on the mouse to turn on/off the flashlight
- You control the arm with your mouse cursor

---

## Built With

* [cmake](https://cmake.org/) - Open-source cross-platform tool designed to build, test and package software.
* [VisualStudio 2017](https://visualstudio.microsoft.com/fr/vs/) - IDE used
* [OpenGL](https://www.opengl.org/) (version 3.0) - 2D and 3D graphics API

## Authors

* **B Lucas** - *Debuggage, vue FPS (?)* - @0xWryth
* **D Antonin** - *Importation modèle torche, controls, murs, texture* - @adepreis
* ~~**D Guillaume** - *work description* - @gdescomps~~

---

## Acknowledgments / Inspiration

* [Tobias isenberg](http://tobias.isenberg.cc/) - computer graphics instructor
* Mickaël Sereno - PhD student in charge of computer graphics TPs
* SDL 2.0 Library [wiki](https://wiki.libsdl.org/)
* [Tutoriel : Charger un modèle 3D](http://www.opengl-tutorial.org/fr/beginners-tutorials/tutorial-7-model-loading/)
* [thebookofshaders](https://thebookofshaders.com/)