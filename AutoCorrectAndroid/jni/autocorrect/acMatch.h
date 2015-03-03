#ifndef ACMATCH
#define ACMATCH

#include <string>
#include <cassert>
#include <cmath>

using namespace std;

class acMatch {
public:
	acMatch(string s,int scr);
	acMatch();

	static bool sortAscScoreDescPrio(acMatch l,acMatch r);
	static bool sortAdjusted(acMatch l,acMatch r);
	static bool sortDescNgramAscScoreDescPrio(acMatch l,acMatch r);
	static bool sortAdjustedNgramsFirst(acMatch l,acMatch r);


	string match;
	int score;
	int prob;
	float kbdist;
	float ngram;
	int fletter;

	float adjustedScore();
};

#endif
