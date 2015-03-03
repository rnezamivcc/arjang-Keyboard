#ifndef HASH_H
#define HASH_H

#include <jni.h>

class Hash {
public:
	static int wordHash(const jchar* chars,int len);
};

#endif