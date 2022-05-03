#pragma once

class Vector3f {
public:
	Vector3f();
	Vector3f(float x, float y, float z);
	void setPos(float x, float y, float z);
	float getx();
	float gety();
	float getz();
	float& operator[](const int i);
	float operator[] (const int i) const;
	float operator -(Vector3f v);

private:
	float pos[3];

};
Vector3f operator+(const Vector3f& v1, const Vector3f& v2);
bool operator== (Vector3f v1, Vector3f v2);
float dotproduct(Vector3f v1, Vector3f v2);