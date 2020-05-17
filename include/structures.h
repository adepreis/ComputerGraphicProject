#ifndef  STRUCTURES_INC
#define  STRUCTURES_INC

//GML library
#include <glm/glm.hpp>

// Represente un type de matériau.
struct Material
{
	glm::vec3 color = glm::vec3(0.0f, 0.0f, 1.0f);
	float kd = 0.5f;
	float ka = 0.5f;
	float ks = 1.0f;
	float alpha = 500.0f;

	Material() {}

	Material(glm::vec3 c) {
		color = c;
		alpha = 0.3f;
	}

	Material(glm::vec3 c, float kd, float ka, float ks, float alpha) {
		color = c;

		kd = kd;
		ka = ka;
		ks = ks;

		alpha = alpha;
	}
};

// Represente un type de lumière.
struct Light
{
public:
	//glm::vec3 direction;
	glm::vec3 pos;

	glm::vec3 color;

	glm::vec3 defaultColor;

	bool lightIsOn = true;

	Light(glm::vec3 p, glm::vec3 c) {
		pos = p;
		color = c;
		defaultColor = c;
	}

	void toogle()
	{
		if (lightIsOn) {
			color = glm::vec3(0.f, 0.f, 0.f);
			lightIsOn = false;
		}
		else {
			color = defaultColor;
			lightIsOn = true;
		}
	}
};

#endif