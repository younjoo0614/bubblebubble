#include "bubble.h"

Bubble::Bubble() {
	center.setPos(0, 0, 0);
	velocity.setPos(1, 1, 0);

	mtl.setEmission(0.1, 0.1, 0.1, 1);
	mtl.setAmbient(0.1, 0.1,0.1, 1);
	mtl.setDiffuse(0.5, 0.5, 0.5, 1);
	mtl.setSpecular(1.0, 1.0, 1.0, 1);
	mtl.setShininess(10);

	setRadius(16);
	setSlice(20);
	setStack(20);

	is_moving = false;
	stop_aftershooting = false;

	int_position.push_back(0);
	int_position.push_back(0);

	boundaryX = 320;
	boundaryY = 320;

	is_on = true;
	color = 9;
	attach = false;
	is_falling = false;
}

Bubble::Bubble(Material& m) {
	center.setPos(0, 0, 0);
	velocity.setPos(1, 1, 0);

	mtl.setEmission(0.1, 0.1, 0.1, 1);
	mtl.setAmbient(m.getAmbient()[0], m.getAmbient()[1], m.getAmbient()[2], m.getAmbient()[3]);
	mtl.setDiffuse(0.5, 0.5, 0.5, 1);
	mtl.setSpecular(1.0, 1.0, 1.0, 1);
	mtl.setShininess(10);

	setRadius(16);
	setSlice(20);
	setStack(20);

	is_moving = false;
	stop_aftershooting = false;

	int_position.push_back(0);
	int_position.push_back(0);

	boundaryX = 320;
	boundaryY = 320;

	is_on = true;
	color = 9;
	attach = false;
	is_falling = false;
}

Bubble::~Bubble() {
	while (radius > 0) {
		radius -= 2;
	}
}

void Bubble::setRadius(float r) {
	radius = r;
}

float Bubble::getRadius() const {
	return radius;
}

void Bubble::setSlice(float sl) {
	slice = sl;
}

void Bubble::setStack(float st) {
	stack = st;
}

void Bubble::setCenter(const Vector3f& v) {
	center.setPos(v[0], v[1], v[2]);
}

Vector3f Bubble::getCenter() const {
	return center;
}

void Bubble::setVelocity(const Vector3f& v) {
	velocity.setPos(v[0], v[1], v[2]);
}

Vector3f Bubble::getVelocity() {
	return velocity;
}

bool Bubble::operator==(Bubble bubble) {
	return (this->getCenter() - bubble.getCenter() < this->radius * this->radius);
}

void Bubble::operator=(Bubble& bubble) {
	this->setMTL(bubble.mtl);
	this->setCenter(bubble.getCenter());
	this->setColor(bubble.getColor());
	this->is_on=(bubble.is_on);
}

void Bubble::move() {
	if (is_moving)	center = center + velocity;
	update_pos();
}

void Bubble::setMTL(const Material& m) {
	mtl.setAmbient(m.getAmbient()[0], m.getAmbient()[1], m.getAmbient()[2], m.getAmbient()[3]);
	mtl.setDiffuse(m.getDiffuse()[0], m.getDiffuse()[1], m.getDiffuse()[2], m.getDiffuse()[3]);
	mtl.setEmission(m.getEmission()[0], m.getEmission()[1], m.getEmission()[2], m.getEmission()[3]);
	mtl.setSpecular(m.getSpecular()[0], m.getSpecular()[1], m.getSpecular()[2], m.getSpecular()[3]);
	mtl.setShininess(m.getShininess()[0]);
}

void Bubble::draw() const {
	
		glMaterialfv(GL_FRONT, GL_EMISSION, mtl.getEmission());
		glMaterialfv(GL_FRONT, GL_AMBIENT, mtl.getAmbient());
		glMaterialfv(GL_FRONT, GL_DIFFUSE, mtl.getDiffuse());
		glMaterialfv(GL_FRONT, GL_SPECULAR, mtl.getSpecular());
		glMaterialfv(GL_FRONT, GL_SHININESS, mtl.getShininess());
		glPushMatrix();
		glTranslatef(center[0], center[1], center[2]);
		glutSolidSphere(radius, slice, stack);

		glPopMatrix();
	
}



void Bubble::setColor(int co) {
	color = co;
}

int Bubble::getColor() {
	return color;
}

void Bubble::update_pos() {
	float x = this->getCenter()[0];
	float y = this->getCenter()[1];
	int_position[0] = (int)((0.5 * boundaryY - 16  - y + 8 * sqrt(3)) / (sqrt(3) * 16));
	if (int_position[0] == 13)
		std::cout << "1" << std::endl;
	if (int_position[0] % 2 == 0) {
		int_position[1] = (int)((x + 0.5 * boundaryX) / (2 * radius));
	}
	else {
		int_position[1] = (int)((x + 0.5 * boundaryX - radius) / (2 * radius));
	}
}

void Bubble::set_int_pos(int x, int y) {
	int_position[0] = x;
	int_position[1] = y;
}

std::vector<int> Bubble::get_int_pos() {
	return int_position;
}