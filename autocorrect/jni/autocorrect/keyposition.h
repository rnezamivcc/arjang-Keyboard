#ifndef KEYPOSITION_H
#define KEYPOSITION_H

#if defined (WIN32)
#include "../jni.h"
#else
#include <jni.h>
#endif

class keyposition {
public:
	keyposition();
	keyposition(jchar c,float xa,float ya,float xb,float yb);

	bool operator< (const keyposition& right) const;

	float x1, y1, x2, y2;
	jchar ch;
};

#endif