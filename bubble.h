#pragma once

#include <iostream>
#include <vector> 
#include <math.h>
#include "material.h"
#include <math.h>
#include "Vector3f.h"

class Bubble {
public:
	Bubble();
	Bubble(Material& m);
	~Bubble();

	void setRadius(float r);
	float getRadius() const;

	void setSlice(float sl);
	void setStack(float st);

	void setCenter(const Vector3f& v);
	Vector3f getCenter() const;

	void setVelocity(const Vector3f& v);
	Vector3f getVelocity();

	bool operator ==(Bubble bubble);

	void operator =(Bubble& bubble);

	void move();
	void setMTL(const Material& m);
	void draw() const;

	bool is_moving;
	bool stop_aftershooting;
	bool is_on;

	Material mtl;
	void setColor(int co);
	int getColor();
	void update_pos();
	void set_int_pos(int x, int y);
	std::vector<int> get_int_pos();
	bool attach;
	bool is_falling;

private:
	float radius;
	int slice, stack;
	Vector3f center, velocity;
	int color;
	std::vector<int> int_position;
	int boundaryX, boundaryY;
};