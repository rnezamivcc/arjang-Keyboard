#include "corrector.h"

void Corrector::strtoupper(char* s) {
	while(*s != 0) {
		*s = (char)toupper(*s);
		s++;
	}
}

string Corrector::strtoupper(string s) {
	int c;
	string ret;

	ret = s;

	for(c = 0;c < s.length();c++) {
		ret[c] = (char)toupper(ret[c]);
	}

	return ret;
}

Corrector::Corrector(const unsigned char* bufKeyboard,const unsigned int* hshes,unsigned int nhash,const unsigned char* prb,int tgi,int tgc,unsigned int* tgrams,const char* spc,unsigned char* tgp) {
	// TODO: refactor this. Eventually all  corrections should be done through this class, so constants (puncts, surrounds) won't be passed it.
	// etc...

	if(bufKeyboard != NULL) {
		kbd = new kbdDistance(bufKeyboard);
	} else {
		kbd = NULL;
	}

	hashes = hshes;
	nhashes = nhash;
	prob = prb;
	special = spc;
	tgcount = tgc;
	tgindex = tgi;
	twograms = tgrams;
	tgprobs = tgp;
}

// eventually, this will be refactored to include matches as part of the corrector class
void Corrector::spaceReplace(const char* s,vector<acMatch>* realwords,bool upcase,int sbefore,int safter) {
	const char* spacekeys;
	unsigned int* bufKI;
	int spacekeylen;
	int slen;
	int c, cb, cc;
	char nospace[40];
	unsigned int ha, hb;
	int pra, prb;
	string cppstr;
	acMatch spm;

	if(kbd != NULL) {
		slen = strlen(s);
		spacekeys = kbd->getSpaceKeys();

		if(slen > 40) { return; }

		//LOGI("spacekeys = %s",spacekeys);

		spacekeylen = strlen(spacekeys);

		for(c = 1;c < slen-1;c++) {
			for(cb = 0;cb < spacekeylen;cb++) {
				if(s[c] == spacekeys[cb]) {
					memset(nospace,0,40);
					strcpy(nospace,s);

					nospace[c] = 0;

					ha = wordHash(nospace);
					hb = wordHash(nospace+c+1);

					pra = getProb(ha);
					prb = getProb(hb);

					if(pra <= 0 || prb <= 0) { continue; }

					if(upcase || ((pra & 128) == 128)) {
						nospace[0] = (char)toupper(nospace[0]);
					}

					if((prb & 64) == 64) {
						strtoupper(nospace);
					}

					if((prb & 128) == 128) {
						nospace[c+1] = (char)toupper(nospace[c+1]);
					}

					if((prb & 64) == 64) {
						strtoupper(&nospace[c+1]);
					}

					nospace[c] = ' ';

					if(sbefore != -1) {
						cppstr = special[sbefore] + string(nospace);
					} else {
						cppstr = string(nospace);
					}

					if(safter != -1) {
						cppstr += special[safter];
					}

					spm = acMatch(cppstr,1,true,slen);

					spm.ngram = 0;

					if(tgindex != -1) {
						for(cc = 0;cc < tgcount;cc++) {
							if(twograms[tgindex+cc] == ha) {
								spm.ngram = tgprobs[tgindex+cc];
								break;
							}
						}
					}

					spm.prob = ((pra & 63) + (prb & 63)) / 2;

					spm.kbdist = 1;

					realwords->push_back(spm);
				}
			}
		}
	}
}

void Corrector::filterMatches(vector<acMatch>* matches,bool upcase,vector<acMatch>* realwords,const char* slower,int sbefore,int safter) {
	int c, cb;
	int hoff;
	unsigned int hash;
	pair<unsigned const int*,unsigned const int*> bounds;
	char ch;
	bool countlast = false;
	int slen = strlen(slower);
	int mlen;

	if(kbd != NULL) {
		if(!(kbd->isSpaceKey(slower[slen-1])) && kbd->dist(slower[slen-1],slower[slen-2]) > 0.5) {
			countlast = true;
		}
	}

	for(c = 0;c < matches->size();c++) {
		hash = wordHash((*matches)[c].match.c_str());

		bounds = equal_range(hashes,hashes+nhashes,hash);

		if(bounds.first != hashes+nhashes) {
			if(*bounds.first == hash) {
				ch = (*matches)[c].match[0];

				mlen = (*matches)[c].match.length();

				if(kbd->dist(ch,*slower) <= kbd->dist('a','s')) {
					(*matches)[c].fletter = 1;
				} else {
					(*matches)[c].fletter = 0;
				}

				if(countlast && ((*matches)[c].match[mlen-1] == slower[slen-1])) {
					(*matches)[c].lletter = 1;
				}

				(*matches)[c].ngram = 0;

				if(tgindex != -1) {
					// TODO: binary search

					for(cb = 0;cb < tgcount;cb++) {
						if(twograms[tgindex+cb] == hash) {
							(*matches)[c].ngram = tgprobs[tgindex+cb];
							break;
						}
					}
				}

				hoff = bounds.first - hashes;

				(*matches)[c].prob = prob[hoff] & 63;

				if(kbd != NULL) {
					(*matches)[c].kbdist = std::keylev((*matches)[c].match.c_str(),slower,kbd);
				} else {
					(*matches)[c].kbdist = -1;
				}

				if((*matches)[c].prob == 0) { continue; }

				if(((prob[hoff] & 128) == 128) || upcase) {
					(*matches)[c].match[0] = (char)toupper((*matches)[c].match[0]);
				}

				if((prob[hoff] & 64) == 64) {
					//LOGI("accronym %s",matches[c].match.c_str());
					(*matches)[c].match = strtoupper((*matches)[c].match);
				}

				if(sbefore != -1) {
					(*matches)[c].match = special[sbefore] + (*matches)[c].match;
				}

				if(safter != -1) {
					(*matches)[c].match += special[safter];
				}

				realwords->push_back((*matches)[c]);
			}
		}
	}
}

int Corrector::getProb(unsigned int hash) {
	return std::getProb(hash,hashes,nhashes,prob);
}


