#pragma once

#include "bubble.h"
#include <vector>
#include <map>

using namespace std;
class DetectStop {
public:
	DetectStop(Bubble* bubble);
	bool touch(Bubble* bubble);
	void afterstop(Bubble* bubble);

	vector<vector<Bubble*>> bubblestack;
	vector<vector<Bubble>> realstack;
	static vector<vector<bool>> is_filled;
	vector<vector<int>> color;

	int boundaryX, boundaryY;
};