#ifndef CORRECTOR
#define CORRECTOR

#include <vector>
#include <cstring>
#include <cstdlib>
#include <cctype>
#include <string>
#include <algorithm>

#include "levenshtein.h"
#include "acMatch.h"
#include "kbdDistance.h"
#include "wordhash.h"

using namespace std;

class Corrector {
public:
	Corrector(const unsigned char* bufKeyboard,const unsigned int* hshes,unsigned int nhash,const unsigned char* prb,int tgi,int tgc,unsigned int* tgrams,const char* spc,unsigned char* tgp);
	void spaceReplace(const char* s,vector<acMatch>* realwords,bool upcase,int sbefore,int safter);

	void filterMatches(vector<acMatch>* matches,bool upcase,vector<acMatch>* realwords,const char* slower,int sbefore,int safter);

	int getProb(unsigned int hash);
private:
	string strtoupper(string s);
	void strtoupper(char* s);

	const unsigned char* prob;
	const unsigned int* hashes;
	unsigned int nhashes;
	int tgcount, tgindex;
	unsigned int* twograms;
	const char* special;
	unsigned char* tgprobs;
	bool countlast;

	kbdDistance* kbd;
};

#endif
