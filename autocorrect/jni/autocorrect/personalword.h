#ifndef PERSONALWORD_H
#define PERSONALWORD_H

#include <jni.h>

#include <string>

#include "hash.h"
#include "acmatch.h"

using namespace std;

class personalword {
public:
	personalword();
	personalword(const jchar* s,int n,float p);

	const jchar* getChars();
	int getLength();
	int getHash();

	acMatch generateMatch(int lev);

	float prob;

	friend class personaldict;
private:
	basic_string<jchar> chars;

	int attrs;
};

#endif