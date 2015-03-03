#ifndef KEYPOSITION_H
#define KEYPOSITION_H

#include "StdAfx.h"
#include "dictrun.h"

class keyposition {
public:
	keyposition();
	keyposition(MYWCHAR c,float xa,float ya,float xb,float yb);

	bool operator< (const keyposition& right) const;

	float x1, y1, x2, y2;
	MYWCHAR ch;
};

#endif