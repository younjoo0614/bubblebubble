#include "arrow.h"
#define PI 3.141592

Arrow::Arrow() {
	angle = 0;
	mtl.setEmission(0.1, 0.1, 0.1, 1);
	mtl.setAmbient(0.3, 0.3, 0.3, 1);
	mtl.setDiffuse(0.5, 0.5, 0.5, 1);
	mtl.setSpecular(1.0, 1.0, 1.0, 1);
	mtl.setShininess(10);
}

Arrow::Arrow(int ang) {
	angle = ang;
	mtl.setEmission(0.1, 0.1, 0.1, 1);
	mtl.setAmbient(0.3, 0.3, 0.3, 1);
	mtl.setDiffuse(0.5, 0.5, 0.5, 1);
	mtl.setSpecular(1.0, 1.0, 1.0, 1);
	mtl.setShininess(10);
}

void Arrow::draw_arrow() {
	glShadeModel(GL_SMOOTH);
	glMaterialfv(GL_FRONT, GL_EMISSION, mtl.getEmission());
	glMaterialfv(GL_FRONT, GL_AMBIENT, mtl.getAmbient());
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mtl.getDiffuse());
	glMaterialfv(GL_FRONT, GL_SPECULAR, mtl.getSpecular());
	glMaterialfv(GL_FRONT, GL_SHININESS, mtl.getShininess());
	glPushMatrix();
	glTranslatef(location[0], location[1], location[2]);
	glRotatef(-90, 1.0, 0.0, 0.0);
	glRotatef(angle, 0.0, -1.0, 0.0);
	glutSolidCone(50, 60, 10, 10);

	glPopMatrix();
}

void Arrow::draw_line() {
	if (is_axis) {
		glColor3f(1, 1, 1);
		glLineWidth(1.0);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1, 0x0F0F);
		glBegin(GL_LINES);
		glVertex2f(0, -224);
		if (1.0 / tan(angle * PI / 180) < 12.0 / 5.0 && tan(angle * PI / 180) > 0) {
			float x = -160;
			float y = 1.0 / tan(angle * PI / 180) * 160 - 224;
			glVertex2f(x, y);
		}
		else if (1.0 / tan(angle * PI / 180) >= 12.0 / 5.0 && tan(angle * PI / 180) > 0) {
			float x = -384 * tan(angle * PI / 180);
			float y = 160;
			glVertex2f(x, y);
		}
		else if (-1.0 / tan(angle * PI / 180) < 12.0 / 5.0 && tan(angle * PI / 180) < 0) {
			float x = 160;
			float y = -1.0 / tan(angle * PI / 180) * 160 - 224;
			glVertex2f(x, y);
		}
		else if (-1.0 / tan(angle * PI / 180) > 12.0 / 5.0 && tan(angle * PI / 180) < 0) {
			float x = -384 * tan(angle * PI / 180);
			float y = 160;
			glVertex2f(x, y);
		}
		glEnd();
		glFlush();
	}
}