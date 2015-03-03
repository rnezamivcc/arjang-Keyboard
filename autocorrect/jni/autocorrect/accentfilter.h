#ifndef ACCENTFILER_H
#define ACCENTFILTER_H

#if defined (WIN32)
#include "../jni.h"
#else
#include <jni.h>
#endif

#include <algorithm>

using namespace std;

class accentFilter {
public:
	accentFilter(jchar* aindex,jchar* avalues,int n);
	~accentFilter();

	bool areEqual(jchar ch1,jchar ch2);
	jchar removeAccent(jchar ch);

	//debug
	JNIEnv* env;
private:
	jchar* index;
	jchar* values;
	int naccents;

	// debug
	void logout(const char* s);
};

#endif