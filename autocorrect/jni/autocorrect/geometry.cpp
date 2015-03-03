
#include "geometry.h"

float std::rectDist(float ax1,float ay1,float ax2,float ay2,float bx1,float by1,float bx2,float by2) {
	float dx;
	float dy;

	dx = max(ax1,bx1) - min(ax2,bx2);
	dy = max(ay1,by1) - min(ay2,by2);

	if(dx < 0) { return max(dy,(float)0); }
	if(dy < 0) { return dx; }
	return sqrt((dx*dx)+(dy*dy));
}

float std::rectPointDist(float px,float py,float x1,float y1,float x2,float y2) {
	float dx;
	float dy;

	dx = min(abs(x1-px),abs(x2-px));
	dy = min(abs(y1-py),abs(y2-py));

	return sqrt((dx*dx) + (dy*dy));
}

float std::hypot(float x,float y) {
	return sqrt((x*x) + (y*y));
}