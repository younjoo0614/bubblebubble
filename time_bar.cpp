#include "time_bar.h"

Time_bar::Time_bar() {
	second = 5;
	time_up = false;
	start = std::chrono::system_clock::now();
	boundaryX = 320;
	boundaryY = 320;
	ratio = 1;
	shot = false;
	auto now = std::chrono::system_clock::now();
	paused= std::chrono::duration_cast<std::chrono::milliseconds>(now - now);
	pause_time = false;
	
}

Time_bar::Time_bar(float sec, int X, int Y) {
	second = sec;
	time_up = false;
	boundaryX = X;
	boundaryY = Y;
	start = std::chrono::system_clock::now();
	ratio = 1;
	auto now = std::chrono::system_clock::now();
	paused = std::chrono::duration_cast<std::chrono::milliseconds>(now - now);
	shot = false;
	pause_time = false;
}

void Time_bar::returning_ratio() {
	auto now = std::chrono::system_clock::now();
	std::chrono::duration<float> sec = std::chrono::duration_cast<std::chrono::milliseconds>(now - start);
	ratio = 1 - (float)((sec-paused).count() / second);
	if (ratio <= 0) time_up = true;
}

void Time_bar::pause_start() {
	start_pause = std::chrono::system_clock::now();
}

void Time_bar::pause_end() {
	auto end_pause = std::chrono::system_clock::now();
	paused += std::chrono::duration_cast<std::chrono::milliseconds>(end_pause - start_pause);
}

void Time_bar::draw_time() {
	if (ratio >= 0) {
		glBegin(GL_POLYGON);
		glColor3f(0, 0.5, 0.5);
		glVertex2f(0.3 * boundaryX + 0.6 * ratio * boundaryX, 0.8 * boundaryY);
		glVertex2f(0.3 * boundaryX, 0.8 * boundaryY);
		glVertex2f(0.3 * boundaryX, 0.72 * boundaryY);
		glVertex2f(0.3 * boundaryX + 0.6 * ratio * boundaryX, 0.72 * boundaryY);
		glEnd();
	}
}