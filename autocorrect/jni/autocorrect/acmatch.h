#ifndef ACMATCH_H
#define ACMATCH_H

#if defined (WIN32)
#include "../jni.h"
#else
#include <jni.h>
#endif

#include <string>

using namespace std;

class acMatch {
public:
	acMatch();
	acMatch(basic_string<jchar> chrs,int lscr);

	static bool sortAdjScore(acMatch l,acMatch r);
	static bool sortNgramOnly(acMatch l,acMatch r);
	static bool sortLevThenNgram(acMatch l,acMatch r);

	void updateAdjustedScore();

	int levscore;
	int prob;
	bool propernoun;
	bool accronym;
	int firstwordhash;
	int tgfreq;
	float keydist;
	float adjscore;
	int learnedtg; // learned 2-gram frequency from personal dictionary
	int hashindex; // the hash index for this word or -1

	basic_string<jchar> chars;
};

#endif