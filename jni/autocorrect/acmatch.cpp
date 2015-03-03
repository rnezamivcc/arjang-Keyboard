//
#include "acmatch.h"

acMatch::acMatch()
{
	levscore = SCHAR_MAX;
	adjscore = INT_MAX;
	keydist = -1;
	tgfreq = 0;
	prob = 0;
	word[0] = NUL;
	length = 0;
}

acMatch::acMatch(MYWCHAR * chrs, int lscr)
{
	length = mywcslen(chrs);
	assert(length > 0);
	mywcsncpy(word, chrs, length);
	levscore = adjscore = lscr;
	keydist = -1;
	tgfreq = 0;
	prob = 0;
}

void acMatch::set(MYWCHAR *name, USHORT len, USHORT probability, int lscr)
{
	length = len;
	prob = probability;
	mywcsncpy(word, name, length);
	levscore = adjscore = lscr;
}

bool acMatch::sortAdjScore(acMatch l, acMatch r)
{
	if(r.adjscore > l.adjscore) 
		return true;
	else if(r.adjscore == l.adjscore) 
		return (l.prob > r.prob);
	else
		return false;
}

bool acMatch::sortLevThenNgram(acMatch l, acMatch r)
{
	if(r.levscore == 0 && l.levscore < 2)
		return r.tgfreq < l.tgfreq;

	if(l.levscore == 0 && r.levscore < 2) 
		return r.tgfreq < l.tgfreq;
	
	if(r.levscore > l.levscore) 
		return true;
	else if(r.levscore == l.levscore)
		return r.tgfreq < l.tgfreq;
	else
		return false;
}

bool acMatch::sortNgramOnly(acMatch l,acMatch r) 
{
	return (r.tgfreq < l.tgfreq);
}

void acMatch::updateAdjustedScore() 
{
	if(keydist < 0)
		adjscore = levscore;
	else
		adjscore = min(keydist, (float)levscore);

	adjscore -= (float)tgfreq / 100;
}