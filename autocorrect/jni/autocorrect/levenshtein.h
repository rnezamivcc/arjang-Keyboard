#ifndef LEVENSHTEIN
#define LEVENSHTEIN

#include <jni.h>
#include <android/log.h>

#include <algorithm>
#include <cstdlib>

#include "kbdDistance.h"

using namespace std;

namespace std {
	// compute the Levensthein distance between two strings, not nearly as efficieint as using the graph, but faster than rebuilding the graph for each added custom word
	int lev(const char* s1,const char* s2);
	float keylev(const char* s1,const char* s2,kbdDistance* kbd);
};

#endif
