#include "detectstop.h"

DetectStop::DetectStop(Bubble* bubble) {
	boundaryX = 320;
	boundaryY = 320;
}

bool DetectStop::touch(Bubble* bubble) {
	/*for (auto i : bubblestack) {
		for (auto j : i) {
			if (j != nullptr) {
				float cDiff = bubble->getCenter() - j->getCenter();
				float rDiff = 4 * bubble->getRadius() * bubble->getRadius();
				if (bubble->getCenter() - j->getCenter() <= 4 * bubble->getRadius() * bubble->getRadius()) {
					return true;
				}
			}
			
		}
	}*/


	for (int i = 0; i < bubblestack.size(); i++) {
		for (int j = 0; j < bubblestack[i].size(); j++) {
			if (bubblestack[i][j] != nullptr) {
				float cDiff = bubble->getCenter() - bubblestack[i][j]->getCenter();
				float rDiff = 4 * bubble->getRadius() * bubble->getRadius();
				if (bubble->getCenter() - bubblestack[i][j]->getCenter() <= 4 * bubble->getRadius() * bubble->getRadius()) {
					return true;
				}
			}

		}
	}
	return false;
}

void DetectStop::afterstop(Bubble* bubble) {
		int y = bubble->get_int_pos()[0];
		int x = bubble->get_int_pos()[1];
		float pos_y=0;
		float pos_x=0;
		if (y == 0) {
			pos_y = 0.5 * boundaryY - bubble->getRadius();
			pos_x = (2 * x + 1) * bubble->getRadius()-0.5*boundaryX;
		}
		else if (y % 2 == 0) {
			pos_y = 0.5 * boundaryY-bubble->getRadius() - y * bubble->getRadius() * sqrt(3);
			pos_x = (2 * x + 1) * bubble->getRadius()-0.5*boundaryX;
		}
		else if (y % 2 == 1) {
			pos_y = 0.5*boundaryY-bubble->getRadius() - y * bubble->getRadius() * sqrt(3);
			pos_x = (2 * x +2) * bubble->getRadius()-0.5*boundaryX;
		}
		bubble->setCenter(Vector3f(pos_x, pos_y, 0));
		bubble->setVelocity(Vector3f(0, 0, 0));
		bubble->is_moving = false;
		bubble->stop_aftershooting = true;
}