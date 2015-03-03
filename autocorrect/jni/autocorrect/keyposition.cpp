
#include "keyposition.h"

keyposition::keyposition(jchar c,float xa,float ya,float xb,float yb) {
	x1 = xa;
	y1 = ya;
	x2 = xb;
	y2 = yb;
	ch = c;
}

keyposition::keyposition() {
	x1 = 0;
	y1 = 0;
	x2 = 0;
	y2 = 0;
	ch = 0;
}

/*
*	Sorts key position records into the natural order for their respective characters with the distinction that
*	space always come first.
*/
bool keyposition::operator< (const keyposition& right) const {
	if(this->ch == (jchar)' ') {
		if(right.ch == (jchar)' ') {
			return false;
		} else {
			return true;
		}
	} else if(right.ch == (jchar)' ') {
		return false;
	} else {
		return this->ch < right.ch;
	}
}