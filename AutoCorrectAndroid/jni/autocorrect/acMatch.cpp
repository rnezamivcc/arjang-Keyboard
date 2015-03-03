#include "acMatch.h"

acMatch::acMatch(string s,int scr) {
	match = s;
	score = scr;
	prob = 0;
}

acMatch::acMatch() {
	match = string();
	score = 65536;
	prob = 0;
}


bool acMatch::sortAscScoreDescPrio(acMatch l,acMatch r) {
	if(r.score > l.score) {
		return true;
	} else if(r.score == l.score) {
		return (l.prob > r.prob);
	} else {
		return false;
	}
}

bool acMatch::sortAdjusted(acMatch l,acMatch r) {
	float raj, laj;
	int dp;
	
	if(r.score == 0) {
		return false;
	} else if(l.score == 0 && r.score > 0) {
		return true;
	}

	raj = r.adjustedScore();
	laj = l.adjustedScore();
	dp = abs(r.prob - l.prob);

	if(abs(raj-laj) < 0.5 && dp > 5000) {
		return (l.prob > r.prob);
	} else {
		return (r.adjustedScore() > l.adjustedScore());
	}
}

float acMatch::adjustedScore() {
	float as;

	as = score; //min(score,kbdist);

	if(fletter == 0) {
		as += 0.5;
	}

	as -= ngram;

	return as;
}

bool acMatch::sortDescNgramAscScoreDescPrio(acMatch l,acMatch r) {
	if(r.ngram < l.ngram) {
		return true;
	} else if(r.ngram == l.ngram) {
		if(r.score > l.score) {
			return true;
		} else if(r.score == l.score) {
			return (l.prob > r.prob);
		} else {
			return false;
		}
	} else {
		return false;
	}
}

bool acMatch::sortAdjustedNgramsFirst(acMatch l,acMatch r) {
	float raj, laj;
	int dp;

	if(r.score == 0) {
		return false;
	} else if(l.score == 0 && r.score > 0) {
		return true;
	}

	if(l.ngram == 0 && r.ngram > 0) {
		return false;
	} else if(l.ngram > 0 && r.ngram == 0) {
		return true;
	}

	raj = r.adjustedScore();
	laj = l.adjustedScore();
	dp = abs(r.prob - l.prob);

	if(abs(raj-laj) < 0.5 && dp > 10) {
		return (l.prob > r.prob);
	} else {
		return (r.adjustedScore() > l.adjustedScore());
	}
}
