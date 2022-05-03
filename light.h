#pragma once

#include <GL/glut.h>

class Light
{
public:
	Light(float x, float y, float z, int L_ID);

	void setAmbient(float r, float g, float b, float a);
	void setDiffuse(float r, float g, float b, float a);
	void setSpecular(float r, float g, float b, float a);

	int getID() const;

	void draw() const;

private:
	int lightID;
	float pos[3];
	float ambient[4];
	float diffuse[4];
	float specular[4];
};


