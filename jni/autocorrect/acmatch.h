#ifndef ACMATCH_H
#define ACMATCH_H

#include "StdAfx.h"
#include "dictrun.h"

struct acMatch 
{
	acMatch();
	acMatch(MYWCHAR * chrs, int lscr);
	void set(MYWCHAR *name, USHORT len, USHORT prob, int lscr);
	static bool sortAdjScore(acMatch l, acMatch r);
	static bool sortNgramOnly(acMatch l, acMatch r);
	static bool sortLevThenNgram(acMatch l, acMatch r);

	void updateAdjustedScore();

	BYTE levscore;
	BYTE length;
	BYTE tgfreq;
	BYTE prob;
//	bool propernoun;
//	bool accronym;
//	int firstwordhash;
	float keydist;
	float adjscore;
	//int learnedtg; // learned 2-gram frequency from personal dictionary
	//int hashindex; // the hash index for this word or -1

	MYWCHAR word[MAX_WORD_LEN];
};

#endif