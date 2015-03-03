#include "acstdafx.h"
#include "personalword.h"

personalword::personalword(const jchar* s,int n,float p) {
	prob = p;
	chars = basic_string<jchar>(s,n);
	attrs = 0;
}

personalword::personalword() {
	prob = 0;
}

const jchar* personalword::getChars() {
	return chars.c_str();
}

int personalword::getLength() {
	return chars.length();
}

int personalword::getHash() {
	return Hash::wordHash(chars.c_str(),chars.length());
}

acMatch personalword::generateMatch(int lev) {
	acMatch ret = acMatch(chars,lev);

	ret.tgfreq = 0;
	ret.hashindex = -1;
	ret.propernoun = ((attrs & 1) == 1);
	ret.accronym = ((attrs & 2) == 2);
	ret.prob = prob;

	return ret;
}