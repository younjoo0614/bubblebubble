//칸을 모두 분할해서 짝수번째 줄은 10개씩, 홀수 번째 줄은 9개씩 버블의 자리를 정해놓고 
//각 버블은 변수로 boundaryX, boundaryY를 받고 현재 자리를 vector<int>로 가지고 있게 했습니다.
//2차원 벡터 bubblestack에 그에 맞춰서 realstack에 저장 ex) 2번째 줄 5번째 버블-> realstack[1][4]
//PAUSE 기능, TIME 조절 기능, 검은 색 찬스 볼, 점수 표시, 점선 기능 추가(x누르면 점선 생기고 다시 누르면 없어짐)
//fall down 대신 반지름이 줄어들어 사라지는 것으로 효과 변경

#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <random>
#include "detectstop.h"
#include "arrow.h"
#include "light.h"
#include "time_bar.h"
#include <algorithm>
#include <FreeImage.h>
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

static GLuint textureID;
GLubyte* textureData;
int imageWidth, imageHeight;
#define WIDTH 640
#define HEIGHT 640
#define boundaryX WIDTH/2
#define boundaryY HEIGHT/2
#define PI 3.141592

using namespace std;

vector<vector<bool>> is_checked(14); //(1,1)->bool
vector<vector<Bubble*>> bubblestack(14);

Light light(boundaryX, boundaryY, boundaryX / 2, GL_LIGHT0);
Arrow arrow(0);
Material red, yellow, green, blue, black;
Material color_list[5];

Bubble* bubble_p;
Bubble* next_bubble_p;
Bubble waiting(red);

enum Color { RED, YELLOW, GREEN, BLUE, BLACK };
Time_bar* time_bar;
int score;
float second=8;

void set_colorlist() {
	red.setAmbient(0.6, 0.0, 0.0, 1);
	red.setDiffuse(0.5, 0.4, 0.4, 1);
	red.setSpecular(0.7, 0.04, 0.04, 1);
	red.setEmission(0.1, 0.1, 0.1, 1);
	red.setShininess(10);
	yellow.setAmbient(0.5, 0.5, 0.0, 1);
	yellow.setDiffuse(0.5, 0.5, 0.4, 1);
	yellow.setSpecular(0.7, 0.7, 0.04, 1);
	yellow.setEmission(0.1, 0.1, 0.1, 1);
	yellow.setShininess(0);
	green.setAmbient(0.0, 0.6, 0.0, 1);
	green.setDiffuse(0.4, 0.5, 0.4, 1);
	green.setSpecular(0.04, 0.7, 0.7, 1);
	green.setEmission(0.1, 0.1, 0.1, 1);
	green.setShininess(0);
	blue.setAmbient(0.0, 0.00, 0.6, 1);
	blue.setDiffuse(0.01, 0.01, 0.01, 1);
	blue.setSpecular(0.04, 0.7, 0.7, 1);
	blue.setEmission(0.1, 0.1, 0.1, 1);
	blue.setShininess(0);
	black.setAmbient(0.02, 0.02, 0.02, 1);
	black.setDiffuse(0.01, 0.01, 0.01, 1);
	black.setSpecular(0.5, 0.5, 0.5, 1);
	black.setEmission(0.4, 0.4, 0.4, 1);
	black.setShininess(0);
	color_list[RED] = red;
	color_list[YELLOW] = yellow;
	color_list[GREEN] = green;
	color_list[BLUE] = blue;
	color_list[BLACK] = black;
}

int random_mtr() {
	random_device rd;
	mt19937 gen(rd());
	uniform_int_distribution<int> dis(1, 9);
	int num = dis(gen);
	int color;
	if (num == 1 || num == 2) color = RED;
	else if (num == 3 || num == 4) color = YELLOW;
	else if (num == 5 || num == 6) color = GREEN;
	else if (num == 7 || num == 8) color = BLUE;
	else color = BLACK;
	return color;
}

void after_shot() {
	waiting.setMTL(next_bubble_p->mtl);
	waiting.setCenter(Vector3f(0, -0.7 * boundaryY, 0));
	waiting.setColor(next_bubble_p->getColor());
	delete next_bubble_p;
	int temp = random_mtr();
	next_bubble_p = new Bubble(color_list[temp]);
	next_bubble_p->setColor(temp);
	next_bubble_p->setCenter(Vector3f(-0.5 * boundaryX, -0.8 * boundaryY, 0));
}

void ProcessSpecialkeys(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_LEFT:
		if (arrow.angle < 80) arrow.angle += 5;
		break;
	case GLUT_KEY_RIGHT:
		if (arrow.angle > -80) arrow.angle -= 5;
	}
	glutPostRedisplay();
}

void ProcessNormalkeys(unsigned char key, int x, int y) {
	if (!bubble_p->is_moving) {
		if (key == 32) {
			bubble_p->is_moving = true;
			Vector3f v(- 2*sin((float)arrow.angle * PI / 180),  2*cos((float)arrow.angle * PI / 180), 0);
			bubble_p->setVelocity(v);
			after_shot();
			time_bar->shot = true;
		}
	}
	if (key == 'p'||key=='P') {
		time_bar->pause_time = !time_bar->pause_time;
		if (time_bar->pause_time) {
			time_bar->pause_start();
		}
		else {
			time_bar->pause_end();
		}
	}
	if (key == 'x' || key == 'X') {
		arrow.is_axis = !arrow.is_axis;
	}
}

void write_sentence(void* font, string c, float x, float y) {
	glColor3f(0.0, 1.0, 0.0);
	glRasterPos2f(x, y);
	for (unsigned int i = 0; i < c.size(); i++) {
		glutBitmapCharacter(font, c[i]);
	}
}

FIBITMAP* createBitMap(char const* filename) {
	FREE_IMAGE_FORMAT format = FreeImage_GetFileType(filename, 0);

	if (format == -1) {
		cout << "Could not find image: " << filename << " - Aborting." << endl;
		exit(-1);
	}

	if (format == FIF_UNKNOWN) {
		cout << "Couldn't determine file format - attempting to get from file extension..." << endl;
		format = FreeImage_GetFIFFromFilename(filename);

		if (!FreeImage_FIFSupportsReading(format)) {
			cout << "Detected image format cannot be read!" << endl;
			exit(-1);
		}
	}

	FIBITMAP* bitmap = FreeImage_Load(format, filename);

	int bitsPerPixel = FreeImage_GetBPP(bitmap);

	FIBITMAP* bitmap32;
	if (bitsPerPixel == 32) {
		cout << "Source image has " << bitsPerPixel << " bits per pixel. Skipping conversion." << endl;
		bitmap32 = bitmap;
	}
	else {
		cout << "Source image has " << bitsPerPixel << " bits per pixel. Converting to 32-bit colour." << endl;
		bitmap32 = FreeImage_ConvertTo32Bits(bitmap);
	}
	return bitmap32;
}

void generateTexture() {
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageWidth, imageHeight, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, textureData);
}

void initTexture() {
	FIBITMAP* bitmap32 = createBitMap("newtex.png");
	imageWidth = FreeImage_GetWidth(bitmap32);
	imageHeight = FreeImage_GetHeight(bitmap32);
	textureData = FreeImage_GetBits(bitmap32);
	generateTexture();
}

void drawSquareWithTexture() {
	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);	glVertex3f(-0.5 * boundaryX, -0.7 * boundaryY, 0.0);
	glTexCoord2f(0, 1);	glVertex3f(-0.5 * boundaryX, 0.5 * boundaryY, 0.0);
	glTexCoord2f(1, 1);	glVertex3f(0.5 * boundaryX, 0.5 * boundaryY, 0.0);
	glTexCoord2f(1, 0); glVertex3f(0.5 * boundaryX, -0.7 * boundaryY, 0.0);
	glEnd();
	glDisable(GL_TEXTURE_2D);
}

void init() {
	score = 0;
	initTexture();
	for (int i = 0; i < 14; i++) {
		if (i % 2 == 0) {
			is_checked[i] = vector<bool>(11, false);
			bubblestack[i] = vector<Bubble*>(11,nullptr);
		}
		else {
			is_checked[i] = vector<bool>(11, false);
			bubblestack[i] = vector<Bubble*>(11,nullptr);
		}
	}
	
	for (int i = 0; i < 10; i++) {
		int temp = i % 4;
		Bubble* bubble = new Bubble(color_list[temp]);
		bubble->setColor(temp);
		bubble->setCenter(Vector3f(-0.5 * boundaryX + (2 * i + 1) * bubble->getRadius(), 0.5 * boundaryY - bubble->getRadius(), 0));
		bubble->set_int_pos(0, i);
		bubblestack[0][i] = bubble;	
	}

	int temp = random_mtr();
	bubble_p = new Bubble(color_list[temp]);
	bubble_p->setColor(temp);
	bubble_p->setCenter(Vector3f(0, -0.7 * boundaryY, 0));

	int temp1 = random_mtr();
	next_bubble_p = new Bubble(color_list[temp1]);
	next_bubble_p->setColor(temp1);
	next_bubble_p->setCenter(Vector3f(-0.5 * boundaryX, -0.8 * boundaryY, 0));
	time_bar = new Time_bar(8, boundaryX, boundaryY);
}

vector<Bubble*> del_list;
void make_del_list(Bubble* bubble) {
	is_checked[bubble->get_int_pos()[0]][bubble->get_int_pos()[1]] = true;
	if (bubble->getColor() == BLACK) {
		for (auto i : bubblestack) {
			for (auto j : i) {
				if (j == bubble || j == nullptr) {}
				else if (j->getCenter() - bubble->getCenter() <= 4 * bubble->getRadius() * bubble->getRadius()) {
					del_list.push_back(j);
				}
			}
		}
	}
	else if (bubble->get_int_pos()[0] % 2 == 0) {
		if (bubble->get_int_pos()[0] != 0 && bubble->get_int_pos()[1] != 0 && bubble->get_int_pos()[1] != 9) {
			//check right up
			if (bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1]] != nullptr
				&& bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1]]->getColor() == bubble->getColor()
				&& !is_checked[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1]]) {
				del_list.push_back(bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1]]);
				is_checked[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1]] = true;
				make_del_list(bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1]]);
			}
			//check right
			if (bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] + 1] != nullptr
				&& bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] + 1]->getColor() == bubble->getColor()
				&& !is_checked[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] + 1]) {
				del_list.push_back(bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] + 1]);
				is_checked[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] + 1] = true;
				make_del_list(bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] + 1]);
			}
			//check right down
			if (bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]] != nullptr
				&& bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]]->getColor() == bubble->getColor()
				&& !is_checked[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]]) {
				del_list.push_back(bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]]);
				is_checked[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]] = true;
				make_del_list(bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]]);
			}
			//check left down
			if (bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] - 1] != nullptr
				&& bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] - 1]->getColor() == bubble->getColor()
				&& !is_checked[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] - 1]) {
				del_list.push_back(bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] - 1]);
				is_checked[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] - 1] = true;
				make_del_list(bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] - 1]);
			}
			//check left
			if (bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] - 1] != nullptr
				&& bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] - 1]->getColor() == bubble->getColor()
				&& !is_checked[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] - 1]) {
				del_list.push_back(bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] - 1]);
				is_checked[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] - 1] = true;
				make_del_list(bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] - 1]);
			}
			//check left up
			if (bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1] - 1] != nullptr
				&& bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1] - 1]->getColor() == bubble->getColor()
				&& !is_checked[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1] - 1]) {
				del_list.push_back(bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1] - 1]);
				is_checked[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1] - 1] = true;
				make_del_list(bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1] - 1]);
			}
		}
		else if (bubble->get_int_pos()[0] == 0 && bubble->get_int_pos()[1] != 0 && bubble->get_int_pos()[1] != 9) {
			//check right
			if (bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] + 1] != nullptr
				&& bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] + 1]->getColor() == bubble->getColor()
				&& !is_checked[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] + 1]) {
				del_list.push_back(bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] + 1]);
				is_checked[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] + 1] = true;
				make_del_list(bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] + 1]);
			}
			//check right down
			if (bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]] != nullptr
				&& bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]]->getColor() == bubble->getColor()
				&& !is_checked[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]]) {
				del_list.push_back(bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]]);
				is_checked[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]] = true;
				make_del_list(bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]]);
			}
			//check left down
			if (bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] - 1] != nullptr
				&& bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] - 1]->getColor() == bubble->getColor()
				&& !is_checked[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] - 1]) {
				del_list.push_back(bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] - 1]);
				is_checked[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] - 1] = true;
				make_del_list(bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] - 1]);
			}
			//check left
			if (bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] - 1] != nullptr
				&& bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] - 1]->getColor() == bubble->getColor()
				&& !is_checked[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] - 1]) {
				del_list.push_back(bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] - 1]);
				is_checked[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] - 1] = true;
				make_del_list(bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] - 1]);
			}
		}
		else if (bubble->get_int_pos()[0] != 0 && bubble->get_int_pos()[1] == 0) {
			//check right up
			if (bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1]] != nullptr
				&& bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1]]->getColor() == bubble->getColor()
				&& !is_checked[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1]]) {
				del_list.push_back(bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1]]);
				is_checked[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1]] = true;
				make_del_list(bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1]]);
			}
			//check right
			if (bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] + 1] != nullptr
				&& bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] + 1]->getColor() == bubble->getColor()
				&& !is_checked[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] + 1]) {
				del_list.push_back(bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] + 1]);
				is_checked[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] + 1] = true;
				make_del_list(bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] + 1]);
			}
			//check right down
			if (bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]] != nullptr
				&& bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]]->getColor() == bubble->getColor()
				&& !is_checked[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]]) {
				del_list.push_back(bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]]);
				is_checked[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]] = true;
				make_del_list(bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]]);
			}
		}
		else if (bubble->get_int_pos()[0] != 0 && bubble->get_int_pos()[1] == 9) {
			//check left down
			if (bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] - 1] != nullptr
				&& bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] - 1]->getColor() == bubble->getColor()
				&& !is_checked[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] - 1]) {
				del_list.push_back(bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] - 1]);
				is_checked[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] - 1] = true;
				make_del_list(bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] - 1]);
			}
			//check left
			if (bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] - 1] != nullptr
				&& bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] - 1]->getColor() == bubble->getColor()
				&& !is_checked[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] - 1]) {
				del_list.push_back(bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] - 1]);
				is_checked[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] - 1] = true;
				make_del_list(bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] - 1]);
			}
			//check left up
			if (bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1] - 1] != nullptr
				&& bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1] - 1]->getColor() == bubble->getColor()
				&& !is_checked[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1] - 1]) {
				del_list.push_back(bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1] - 1]);
				is_checked[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1] - 1] = true;
				make_del_list(bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1] - 1]);
			}
		}
		else if (bubble->get_int_pos()[0] == 0 && bubble->get_int_pos()[1] == 0) {
			//check right
			if (bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] + 1] != nullptr
				&& bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] + 1]->getColor() == bubble->getColor()
				&& !is_checked[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] + 1]) {
				del_list.push_back(bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] + 1]);
				is_checked[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] + 1] = true;
				make_del_list(bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] + 1]);
			}
			//check right down
			if (bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]] != nullptr
				&& bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]]->getColor() == bubble->getColor()
				&& !is_checked[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]]) {
				del_list.push_back(bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]]);
				is_checked[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]] = true;
				make_del_list(bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]]);
			}
		}
		else if (bubble->get_int_pos()[0] == 0 && bubble->get_int_pos()[1] == 9) {
			//check left down
			if (bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] - 1] != nullptr
				&& bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] - 1]->getColor() == bubble->getColor()
				&& !is_checked[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] - 1]) {
				del_list.push_back(bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] - 1]);
				is_checked[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] - 1] = true;
				make_del_list(bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] - 1]);
			}
			//check left
			if (bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] - 1] != nullptr
				&& bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] - 1]->getColor() == bubble->getColor()
				&& !is_checked[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] - 1]) {
				del_list.push_back(bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] - 1]);
				is_checked[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] - 1] = true;
				make_del_list(bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] - 1]);
			}
		}
	}
	else if (bubble->get_int_pos()[0] % 2 == 1) {
		if (bubble->get_int_pos()[1] != 0 && bubble->get_int_pos()[1] != 8) {
			//check right up
			if (bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1] + 1] != nullptr
				&& bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1] + 1]->getColor() == bubble->getColor()
				&& !is_checked[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1] + 1]) {
				del_list.push_back(bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1] + 1]);
				is_checked[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1] + 1] = true;
				make_del_list(bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1] + 1]);
			}
			//check right
			if (bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] + 1] != nullptr
				&& bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] + 1]->getColor() == bubble->getColor()
				&& !is_checked[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] + 1]) {
				del_list.push_back(bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] + 1]);
				is_checked[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] + 1] = true;
				make_del_list(bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] + 1]);
			}
			//check right down
			if (bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] + 1] != nullptr
				&& bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] + 1]->getColor() == bubble->getColor()
				&& !is_checked[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] + 1]) {
				del_list.push_back(bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] + 1]);
				is_checked[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] + 1] = true;
				make_del_list(bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] + 1]);
			}
			//check left down
			if (bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]] != nullptr
				&& bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]]->getColor() == bubble->getColor()
				&& !is_checked[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]]) {
				del_list.push_back(bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]]);
				is_checked[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]] = true;
				make_del_list(bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]]);
			}
			//check left
			if (bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] - 1] != nullptr
				&& bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] - 1]->getColor() == bubble->getColor()
				&& !is_checked[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] - 1]) {
				del_list.push_back(bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] - 1]);
				is_checked[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] - 1] = true;
				make_del_list(bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] - 1]);
			}
			//check left up
			if (bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1]] != nullptr
				&& bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1]]->getColor() == bubble->getColor()
				&& !is_checked[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1]]) {
				del_list.push_back(bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1]]);
				is_checked[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1]] = true;
				make_del_list(bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1]]);
			}
		}
		else if (bubble->get_int_pos()[1] == 0) {
			//check right up
			if (bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1] + 1] != nullptr
				&& bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1] + 1]->getColor() == bubble->getColor()
				&& !is_checked[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1] + 1]) {
				del_list.push_back(bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1] + 1]);
				is_checked[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1] + 1] = true;
				make_del_list(bubblestack[bubble->get_int_pos()[0] - 1][1]);
			}
			//check right
			if (bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] + 1] != nullptr
				&& bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] + 1]->getColor() == bubble->getColor()
				&& !is_checked[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] + 1]) {
				del_list.push_back(bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] + 1]);
				is_checked[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] + 1] = true;
				make_del_list(bubblestack[bubble->get_int_pos()[0]][ 1]);
			}
			//check right down
			if (bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] + 1] != nullptr
				&& bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] + 1]->getColor() == bubble->getColor()
				&& !is_checked[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] + 1]) {
				del_list.push_back(bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] + 1]);
				is_checked[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] + 1] = true;
				make_del_list(bubblestack[bubble->get_int_pos()[0] + 1][1]);
			}
			//check left down
			if (bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]] != nullptr
				&& bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]]->getColor() == bubble->getColor()
				&& !is_checked[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]]) {
				del_list.push_back(bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]]);
				is_checked[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]] = true;
				make_del_list(bubblestack[bubble->get_int_pos()[0] + 1][0]);
			}
			//check left up
			if (bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1]] != nullptr
				&& bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1]]->getColor() == bubble->getColor()
				&& !is_checked[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1]]) {
				del_list.push_back(bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1]]);
				is_checked[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1]] = true;
				make_del_list(bubblestack[bubble->get_int_pos()[0] - 1][0]);
			}
		}
		else if (bubble->get_int_pos()[1] == 8) {
			//check right up
			if (bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1] + 1] != nullptr
				&& bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1] + 1]->getColor() == bubble->getColor()
				&& !is_checked[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1] + 1]) {
				del_list.push_back(bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1] + 1]);
				is_checked[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1] + 1] = true;
				make_del_list(bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1] + 1]);
			}
			//check right down
			if (bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] + 1] != nullptr
				&& bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] + 1]->getColor() == bubble->getColor()
				&& !is_checked[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] + 1]) {
				del_list.push_back(bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] + 1]);
				is_checked[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] + 1] = true;
				make_del_list(bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] + 1]);
			}
			//check left down
			if (bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]] != nullptr
				&& bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]]->getColor() == bubble->getColor()
				&& !is_checked[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]]) {
				del_list.push_back(bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]]);
				is_checked[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]] = true;
				make_del_list(bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]]);
			}
			//check left
			if (bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] - 1] != nullptr
				&& bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] - 1]->getColor() == bubble->getColor()
				&& !is_checked[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] - 1]) {
				del_list.push_back(bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] - 1]);
				is_checked[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] - 1] = true;
				make_del_list(bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] - 1]);
			}
			//check left up
			if (bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1]] != nullptr
				&& bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1]]->getColor() == bubble->getColor()
				&& !is_checked[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1]]) {
				del_list.push_back(bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1]]);
				is_checked[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1]] = true;
				make_del_list(bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1]]);
			}
		}
	}
}

void check_alone(Bubble* &bubble) {
	bubble->attach = true;
	if (bubble->get_int_pos()[0] % 2 == 0) {
		if (bubble->get_int_pos()[0] != 0 && bubble->get_int_pos()[1] != 0 && bubble->get_int_pos()[1] != 9) {
			//check right up
			if (bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1]] != nullptr
				&& !bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1]]->attach) {
				bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1]]->attach = true;
				check_alone(bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1]]);
			}
			//check right
			if (bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] + 1] != nullptr
				&& !bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] + 1]->attach) {
				bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] + 1]->attach = true;
				check_alone(bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] + 1]);
			}
			//check right down
			if (bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]] != nullptr
				&& !bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]]->attach) {
				bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]]->attach = true;
				check_alone(bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]]);
			}
			//check left down
			if (bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] - 1] != nullptr
				&& !bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] - 1]->attach) {
				bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] - 1]->attach = true;
				check_alone(bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] - 1]);
			}
			//check left
			if (bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] - 1] != nullptr
				&& !bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] - 1]->attach) {
				bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] - 1]->attach = true;
				check_alone(bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] - 1]);
			}
			//check left up
			if (bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1] - 1] != nullptr
				&& !bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1] - 1]->attach) {
				bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1] - 1]->attach = true;
				check_alone(bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1] - 1]);
			}
		}
		else if (bubble->get_int_pos()[0] == 0 && bubble->get_int_pos()[1] != 0 && bubble->get_int_pos()[1] != 9) {
			//check right
			if (bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] + 1] != nullptr
				&& !bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] + 1]->attach) {
				bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] + 1]->attach = true;
				check_alone(bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] + 1]);
			}
			//check right down
			if (bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]] != nullptr
				&& !bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]]->attach) {
				bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]]->attach = true;
				check_alone(bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]]);
			}
			//check left down
			if (bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] - 1] != nullptr
				&& !bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] - 1]->attach) {
				bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] - 1]->attach = true;
				check_alone(bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] - 1]);
			}
			//check left
			if (bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] - 1] != nullptr
				&& !bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] - 1]->attach) {
				bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] - 1]->attach = true;
				check_alone(bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] - 1]);
			}
		}
		else if (bubble->get_int_pos()[0] != 0 && bubble->get_int_pos()[1] == 0) {
			//check right up
			if (bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1]] != nullptr
				&& !bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1]]->attach) {
				bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1]]->attach = true;
				check_alone(bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1]]);
			}
			//check right
			if (bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] + 1] != nullptr
				&& !bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] + 1]->attach) {
				bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] + 1]->attach = true;
				check_alone(bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] + 1]);
			}
			//check right down
			if (bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]] != nullptr
				&& !bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]]->attach) {
				bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]]->attach = true;
				check_alone(bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]]);
			}
		}
		else if (bubble->get_int_pos()[0] != 0 && bubble->get_int_pos()[1] == 9) {
			//check left down
			if (bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] - 1] != nullptr
				&& !bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] - 1]->attach) {
				bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] - 1]->attach = true;
				check_alone(bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] - 1]);
			}
			//check left
			if (bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] - 1] != nullptr
				&& !bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] - 1]->attach) {
				bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] - 1]->attach = true;
				check_alone(bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] - 1]);
			}
			//check left up
			if (bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1] - 1] != nullptr
				&& !bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1] - 1]->attach) {
				bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1] - 1]->attach = true;
				check_alone(bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1] - 1]);
			}
		}
		else if (bubble->get_int_pos()[0] == 0 && bubble->get_int_pos()[1] == 0) {
			//check right
			if (bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] + 1] != nullptr
				&& !bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] + 1]->attach) {
				bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] + 1]->attach = true;
				check_alone(bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] + 1]);
			}
			//check right down
			if (bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]] != nullptr
				&& !bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]]->attach) {
				bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]]->attach = true;
				check_alone(bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]]);
			}
		}
		else if (bubble->get_int_pos()[0] == 0 && bubble->get_int_pos()[1] == 9) {
			//check left down
			if (bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] - 1] != nullptr
				&& !bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] - 1]->attach) {
				bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] - 1]->attach = true;
				check_alone(bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] - 1]);
			}
			//check left
			if (bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] - 1] != nullptr
				&& !bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] - 1]->attach) {
				bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] - 1]->attach = true;
				check_alone(bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] - 1]);
			}
		}
	}
	else {
		if (bubble->get_int_pos()[1] != 0 && bubble->get_int_pos()[1] != 8) {
			//check right up
			if (bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1] + 1] != nullptr
				&& !bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1] + 1]->attach){ 
				bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1] + 1]->attach = true;
				check_alone(bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1] + 1]);
			}
			//check right
			if (bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] + 1] != nullptr
				&& !bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] + 1]->attach) {
				bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] + 1]->attach = true;
				check_alone(bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] + 1]);
			}
			//check right down
			if (bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] + 1] != nullptr
				&& !bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] + 1]->attach) {
				bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] + 1]->attach = true;
				check_alone(bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] + 1]);
			}
			//check left down
			if (bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]] != nullptr
				&& !bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]]->attach) {
				bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]]->attach = true;
				check_alone(bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]]);
			}
			//check left
			if (bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] - 1] != nullptr
				&& !bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] - 1]->attach) {
				bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] - 1]->attach = true;
				check_alone(bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] - 1]);
			}
			//check left up
			if (bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1]] != nullptr
				&& !bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1]]->attach) {
				bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1]]->attach = true;
				check_alone(bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1]]);
			}
		}
		else if (bubble->get_int_pos()[1] == 0) {
			//check right up
			if (bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1] + 1] != nullptr
				&& !bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1] + 1]->attach) {
				bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1] + 1]->attach = true;
				check_alone(bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1] + 1]);
			}
			//check right
			if (bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] + 1] != nullptr
				&& !bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] + 1]->attach) {
				bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] + 1]->attach = true;
				check_alone(bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] + 1]);
			}
			//check right down
			if (bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] + 1] != nullptr
				&& !bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] + 1]->attach) {
				bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] + 1]->attach = true;
				check_alone(bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] + 1]);
			}
			//check left down
			if (bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]] != nullptr
				&& !bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]]->attach) {
				bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]]->attach = true;
				check_alone(bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]]);
			}
			//check left up
			if (bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1]] != nullptr
				&& !bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1]]->attach) {
				bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1]]->attach = true;
				check_alone(bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1]]);
			}
		}
		else if (bubble->get_int_pos()[1] == 8) {
			//check right up
			if (bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1] + 1] != nullptr
				&& !bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1] + 1]->attach) {
				bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1] + 1]->attach = true;
				check_alone(bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1] + 1]);
			}
			//check right down
			if (bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] + 1] != nullptr
				&& !bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] + 1]->attach) {
				bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] + 1]->attach = true;
				check_alone(bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1] + 1]);
			}
			//check left down
			if (bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]] != nullptr
				&& !bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]]->attach) {
				bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]]->attach = true;
				check_alone(bubblestack[bubble->get_int_pos()[0] + 1][bubble->get_int_pos()[1]]);
			}
			//check left
			if (bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] - 1] != nullptr
				&& !bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] - 1]->attach) {
				bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] - 1]->attach = true;
				check_alone(bubblestack[bubble->get_int_pos()[0]][bubble->get_int_pos()[1] - 1]);
			}
			//check left up
			if (bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1]] != nullptr
				&& !bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1]]->attach) {
				bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1]]->attach = true;
				check_alone(bubblestack[bubble->get_int_pos()[0] - 1][bubble->get_int_pos()[1]]);
			}
		}
	}
}

void drop(Bubble* bubble) {
	make_del_list(bubble);

	for (auto i : del_list) {
		is_checked[i->get_int_pos()[0]][i->get_int_pos()[1]] = false;
	}
	is_checked[bubble->get_int_pos()[0]][bubble->get_int_pos()[1]] = false;

	if (bubble->getColor() == 4 || size(del_list) > 1) {
		int x = bubble->get_int_pos()[0];
		int y = bubble->get_int_pos()[1];	
		bubblestack[x][y]->is_falling = true;
		
		unsigned int n = del_list.size();
		for (unsigned int i = 0; i < n; i++) {
			int x = del_list[i]->get_int_pos()[0];
			int y = del_list[i]->get_int_pos()[1];
			bubblestack[x][y]->is_falling = true;
			
		}
		if (del_list.size() > 3) {
			score += 20 * (del_list.size() + 1);
		}
		else score += 10 * (del_list.size() + 1);
	}
	
	del_list.clear();
	
	for (auto &i : bubblestack) {
		for (auto &j : i) {
			if (j != nullptr&&j->get_int_pos()[0]==0) {
				check_alone(j);
			}
		}
	}
	for (auto& i : bubblestack) {
		for (auto& j : i) {
			if (j != nullptr && !j->attach) {
				int x = j->get_int_pos()[0];
				int y = j->get_int_pos()[1];
				bubblestack[x][y]->is_falling = true;
		
			}
		}
	}
	
	
	for (unsigned int i = 0; i < bubblestack.size(); i++) {
		for (unsigned int j = 0; j < bubblestack[i].size(); j++) {
			if (bubblestack[i][j] != nullptr) {
				if (bubblestack[i][j]->attach) bubblestack[i][j]->attach = false;
			}
		}
	}
}

void end_game(bool end) {
	for (unsigned int i = 0; i < bubblestack.size(); i++) {
		for (unsigned int j = 0; j < bubblestack[i].size(); j++) {
			delete bubblestack[i][j];
		}
	}
	delete bubble_p;
	delete next_bubble_p;
}

void idle() {
	bubble_p->move();
	DetectStop detect(bubble_p);
	detect.bubblestack = bubblestack;
	if (bubble_p->getCenter()[0] + bubble_p->getRadius() >= (float)0.5 * boundaryX ||
		bubble_p->getCenter()[0] - bubble_p->getRadius() <= -(float)0.5 * boundaryX)
		bubble_p->setVelocity(Vector3f(-bubble_p->getVelocity()[0], bubble_p->getVelocity()[1], bubble_p->getVelocity()[2]));

	if (detect.touch(bubble_p) || bubble_p->getCenter()[1] + bubble_p->getRadius() >= (float)0.5 * boundaryY) {
		if (bubble_p->is_moving && bubble_p->getCenter()[1] < -0.7 * boundaryY + bubble_p->getRadius()) {
			bool end = true;
			end_game(end);
			exit(0);
		}
		detect.afterstop(bubble_p);
		bubblestack[bubble_p->get_int_pos()[0]][bubble_p->get_int_pos()[1]] = bubble_p;
		drop(bubblestack[bubble_p->get_int_pos()[0]][bubble_p->get_int_pos()[1]]);
		
		bubble_p = new Bubble(waiting.mtl);
		bubble_p->setColor(waiting.getColor());
		bubble_p->setCenter(waiting.getCenter());
		bubble_p->stop_aftershooting = false;
		delete time_bar;
		time_bar = new Time_bar(second, boundaryX, boundaryY);
	}
	if (!time_bar->shot&&!time_bar->pause_time) time_bar->returning_ratio();
	for (unsigned int i = 0; i < bubblestack.size(); i++) {
		for (unsigned int j = 0; j < bubblestack[i].size(); j++) {
			if (bubblestack[i][j] != nullptr && bubblestack[i][j]->is_falling) {
				bubblestack[i][j]->setRadius(bubblestack[i][j]->getRadius() - 0.5);
				
				if (bubblestack[i][j]->getRadius() <=0 ) {
					delete bubblestack[i][j];
					bubblestack[i][j] = nullptr;
				}
			}
		}
	}
	for (auto& i : bubblestack) {
		for (auto& j : i) {
			if (j != nullptr && j->get_int_pos()[0] == 0) {
				check_alone(j);
			}
		}
	}
	for (auto& i : bubblestack) {
		for (auto& j : i) {
			if (j != nullptr && !j->attach) {
				int x = j->get_int_pos()[0];
				int y = j->get_int_pos()[1];
				bubblestack[x][y]->is_falling = true;
				
			}
		}
	}
	for (unsigned int i = 0; i < bubblestack.size(); i++) {
		for (unsigned int j = 0; j < bubblestack[i].size(); j++) {
			if (bubblestack[i][j] != nullptr && bubblestack[i][j]->is_falling) {
				bubblestack[i][j]->setRadius(bubblestack[i][j]->getRadius() - 0.5);
			
				if (bubblestack[i][j]->getRadius() <= 0) {
					delete bubblestack[i][j];
					bubblestack[i][j] = nullptr;
				}
			}
		}
	}

	for (unsigned int i = 0; i < bubblestack.size(); i++) {
		for (unsigned int j = 0; j < bubblestack[i].size(); j++) {
			if (bubblestack[i][j] != nullptr) {
				if (bubblestack[i][j]->attach) bubblestack[i][j]->attach = false;
			}
		}
	}

	glutPostRedisplay();
}

void displayFunc() {
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-boundaryX, boundaryX, -boundaryY, boundaryY, -100.0, 100.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Draw 2D
	write_sentence(GLUT_BITMAP_HELVETICA_18, "TIME", 0.3 * boundaryX, 0.83 * boundaryY);
	write_sentence(GLUT_BITMAP_HELVETICA_18, "SCORE", -0.6 * boundaryX, 0.83 * boundaryY);
	write_sentence(GLUT_BITMAP_HELVETICA_18, to_string(score), -0.6 * boundaryX, 0.73 * boundaryY);
	write_sentence(GLUT_BITMAP_HELVETICA_18, "NEXT", -0.55 * boundaryX, -0.9 * boundaryY);
	time_bar->draw_time();
	drawSquareWithTexture();
	arrow.draw_line();

	// Draw 3D
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(light.getID());
	light.draw();

	if (time_bar->time_up) {
		ProcessNormalkeys(32, 0, 0);
	}

	bubble_p->draw();
	for (auto& i : bubblestack) {
		for (auto& j : i) {
			if (j != nullptr) j->draw();
		}
	}

	next_bubble_p->draw();
	if (bubble_p->is_moving) waiting.draw();

	arrow.location[0] = 0;
	arrow.location[1] = -0.7 * boundaryY;
	arrow.location[2] = 0;
	arrow.draw_arrow();

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glDisable(light.getID());

	glutSwapBuffers();
}

void sub_menu_function(int option) {
	if (option == 1) second = 2.5f;
	else if (option == 2) second = 5.0f;
	else if (option == 3) second = 8.0f;
	else if (option == 4) second = 10.0f;
}

void main_menu_function(int option) {
	if (option == 999)
		exit(0);
}

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(800, 100);
	glutInitWindowSize(640, 640);
	glutCreateWindow("Bubble Bubble (6/14 is my birthday)");
	set_colorlist();
	init();

	glutSpecialFunc(ProcessSpecialkeys);
	glutKeyboardFunc(ProcessNormalkeys);

	int mainmenu, submenu;
	submenu = glutCreateMenu(sub_menu_function);
	glutAddMenuEntry("2.5s", 1);
	glutAddMenuEntry("5s", 2);
	glutAddMenuEntry("8s", 3);
	glutAddMenuEntry("10s", 4);
	mainmenu = glutCreateMenu(main_menu_function);
	glutAddMenuEntry("Quit", 999);
	glutAddMenuEntry("Go", 0);
	glutAddSubMenu("Time setup", submenu);
	glutAttachMenu(GLUT_RIGHT_BUTTON);


	glutDisplayFunc(displayFunc);
	glutIdleFunc(idle);


	glutMainLoop();
	for (unsigned int i = 0; i < bubblestack.size(); i++) {
		for (unsigned int j = 0; j < bubblestack[i].size(); j++) {
			delete bubblestack[i][j];
		}
	}
	delete bubble_p;
	delete next_bubble_p;
	_CrtDumpMemoryLeaks();
	return 0;
}