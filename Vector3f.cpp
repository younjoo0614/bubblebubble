#include "Vector3f.h"

Vector3f::Vector3f() :pos{ 0.0,0.0,0.0 } {
	setPos(0, 0, 0);
}
Vector3f::Vector3f(float x, float y, float z) {
	setPos(x, y, z);
}
void Vector3f::setPos(float x, float y, float z) {
	pos[0] = x;
	pos[1] = y;
	pos[2] = z;
}
float Vector3f::getx() {
	return pos[0];
}
float Vector3f::gety() {
	return pos[1];
}
float Vector3f::getz() {
	return pos[2];
}
float& Vector3f::operator[](const int i) {
	return pos[i];
}
float Vector3f::operator[] (const int i) const {
	return pos[i];
}

float Vector3f::operator-(Vector3f v) {
	return (((*this)[0] - v[0]) * ((*this)[0] - v[0]) + ((*this)[1] - v[1]) * ((*this)[1] - v[1]) + ((*this)[2] - v[2]) * ((*this)[2] - v[2]));
}

Vector3f operator+(const Vector3f& v1, const Vector3f& v2) {
	return Vector3f(v1[0] + v2[0], v1[1] + v2[1], v1[2] + v2[2]);
}

bool operator ==(Vector3f v1, Vector3f v2) {
	return (v1[0] == v2[0]) && (v1[1] == v2[1]) && (v1[2] == v2[2]);
}

float dotproduct(Vector3f v1, Vector3f v2) {
	float t = 0;
	for (int i = 0; i < 3; i++) {
		t += v1[i] * v2[i];
	}
	return t;
}