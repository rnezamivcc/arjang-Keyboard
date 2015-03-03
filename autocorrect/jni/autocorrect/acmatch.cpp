
#include "acmatch.h"

acMatch::acMatch() {
	levscore = INT_MAX;
	adjscore = INT_MAX;
	keydist = -1;
	tgfreq = 0;
	learnedtg = 0;
}

acMatch::acMatch(basic_string<jchar> chrs, int lscr) {
	chars = chrs;
	levscore = lscr;
	adjscore = lscr;
	keydist = -1;
	tgfreq = 0;
	learnedtg = 0;
}

bool acMatch::sortAdjScore(acMatch l,acMatch r) {
	if(r.adjscore > l.adjscore) {
		return true;
	} else if(r.adjscore == l.adjscore) {
		return (l.prob > r.prob);
	} else {
		return false;
	}
}

bool acMatch::sortLevThenNgram(acMatch l,acMatch r) {
	if(r.levscore == 0 && l.levscore < 2) {
		return r.tgfreq < l.tgfreq;
	}

	if(l.levscore == 0 && r.levscore < 2) {
		return r.tgfreq < l.tgfreq;
	}

	/*if(r.levscore == 1 && l.levscore == 0 ||  r.levscore == 0 && l.levscore == 1 || r.levscore == 0 && r.levscore == 0) {
		return r.tgfreq < l.tgfreq;
	}*/
	
	if(r.levscore > l.levscore) {
		return true;
	} else if(r.levscore == l.levscore) {
		return r.tgfreq < l.tgfreq;
	} else {
		return false;
	}
}

bool acMatch::sortNgramOnly(acMatch l,acMatch r) {
	return (r.tgfreq < l.tgfreq);
}

void acMatch::updateAdjustedScore() {
	if(keydist < 0) {
		adjscore = levscore;
	} else {
		adjscore = min(keydist,(float)levscore);
	}

	adjscore -= (float)tgfreq / 100;

	adjscore -= (float)learnedtg / 10;
}