#pragma once
#include <chrono>
#include "GL/glut.h"

class Time_bar {
public:
	Time_bar();
	Time_bar(float sec, int x, int y);
	void returning_ratio();
	void draw_time();
	void pause_start();
	void pause_end();

	bool shot;
	bool time_up;
	float second;
	float ratio;
	bool pause_time;

	int boundaryX, boundaryY;
	std::chrono::system_clock::time_point start;
	std::chrono::system_clock::time_point start_pause;
	std::chrono::duration<float> paused;
};