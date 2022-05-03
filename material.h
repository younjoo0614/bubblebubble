#pragma once
#include "GL/glut.h"

class Material {
public:
	Material();
	void setEmission(float r, float g, float b, float a);
	void setAmbient(float r, float g, float b, float a);
	void setDiffuse(float r, float g, float b, float a);
	void setSpecular(float r, float g, float b, float a);
	void setShininess(float sh);

	const float* getEmission() const;
	const float* getAmbient() const;
	const float* getDiffuse() const;
	const float* getSpecular() const;
	const float* getShininess() const;

	bool operator == (Material& m);

private:
	float emission[4], ambient[4], diffuse[4], specular[4], shininess[1];
};


