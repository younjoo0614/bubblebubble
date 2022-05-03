#pragma once

#include "GL/glut.h"
#include "material.h"
#include "math.h"

class Arrow {
public:
	Arrow();
	Arrow(int ang);
	void draw_arrow();
	void draw_line();
	int location[3];
	int angle;
	Material mtl;
	bool is_axis;
};