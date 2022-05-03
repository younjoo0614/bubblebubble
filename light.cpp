#include "Light.h"

Light::Light(float x, float y, float z, int L_ID) {
	pos[0] = x;
	pos[1] = y;
	pos[2] = z;
	lightID = L_ID;

	setAmbient(1, 1, 1, 1);
	setDiffuse(1, 1, 1, 1);
	setSpecular(1, 1, 1, 1);
}

void Light::setAmbient(float r, float g, float b, float a) {
	ambient[0] = r;
	ambient[1] = g;
	ambient[2] = b;
	ambient[3] = a;
}
void Light::setDiffuse(float r, float g, float b, float a) {
	diffuse[0] = r;
	diffuse[1] = g;
	diffuse[2] = b;
	diffuse[3] = a;
}
void Light::setSpecular(float r, float g, float b, float a) {
	specular[0] = r;
	specular[1] = g;
	specular[2] = b;
	specular[3] = a;
}

int Light::getID() const {
	return lightID;
}

void Light::draw() const {
	glPushMatrix();

	float light_ambient[] = { ambient[0], ambient[1], ambient[2], ambient[3] };
	float light_diffuse[] = { diffuse[0], diffuse[1], diffuse[2], diffuse[3] };
	float light_specular[] = { specular[0], specular[1], specular[2], specular[3] };
	float light_position[] = { pos[0], pos[1], pos[2], 1 };

	glLightfv(lightID, GL_AMBIENT, light_ambient);
	glLightfv(lightID, GL_DIFFUSE, light_diffuse);
	glLightfv(lightID, GL_SPECULAR, light_specular);
	glLightfv(lightID, GL_POSITION, light_position);

	glPopMatrix();
}