#ifndef AUTOCORRECT_H
#define AUTOCORRECT_H

#if defined (WIN32)
#include "jni.h"
#else
#include <jni.h>
#endif

#include <fstream>
#include <algorithm>
#include <deque>
#include <cfloat>

class AutoCorrect;

#include "autocorrect/hash.h"
#include "autocorrect/acgraph.h"
#include "autocorrect/acmatch.h"
#include "autocorrect/keyposition.h"
#include "autocorrect/personaldict.h"
#include "autocorrect/personalword.h"

using namespace std;

struct tgHeader {
	int offset;
	int n;
};

struct twogram {
	int h1;
	int h2;
	int freq;
};

class acchar {
public:
	acchar(jchar c) {
		ch = c;
		status = 0;
	}

	jchar ch;
	short status;
};

class AutoCorrect {
public:
	AutoCorrect(const char* filename,const char* personalfilename,JNIEnv* jnienv);
	~AutoCorrect();

	void setEnv(JNIEnv* jnienv,jobject obj);

	void addChar(jchar ch);
	void addChars(const jchar* chars,int n);
	void addPrediction(const jchar* chars,int n);
	void backspace(int n);
	void setBeforeCursor(const jchar* chars,int n);
	void setAfterCursor(const jchar* chars,int n);

	void save();

	void reverseCorrection(int si,jstring autocorrect,jstring original);

	void loadKeyPositions(jchar* kl,float* dm,float* sd, int n);

	bool isWord(const jchar* chars,int len);
	
	bool correctionPending();

	void getSecondCorrection(int si,const jchar* ochars,int olen,const jchar* achars,int alen,jchar** cchars,int* clen);

	vector<acMatch>* getPredictions(int lim); // leaks memory so be sure to free the return value after use

	personaldict* personalDict;
	friend class personaldict;

	void logout(const char* s);

	int twogramoverridethreshold;
	bool correctvalidwords;
	bool correctwordcase;
private:
	JNIEnv* env;
	jobject jobj;
	deque<acchar> buffer;
	deque<acchar> aftercursor;

	acGraph* graph;
	int nhashes;
	const int* hashes;
	const char* probs;
	tgHeader* tgindex;
	twogram* twograms;
	int ntwograms;
	jchar* keylist;
	float* distmatrix;
	float* spacedist;
	int nkeys;
	bool hasngrams;
	accentFilter* deaccent;

	jclass clsCharacter;
	jmethodID midToLowerCase;
	jmethodID midIsUpperCase;
	jmethodID midIsLowerCase;
	jmethodID midToUpperCase;

	int isWord(const jchar* chars,int len,int* prob,int *hash,int* hi);

	int getHashIndex(const jchar* chars,int len);
	int getHashIndex(const jchar* chars,int len,int* hp);
	int filterMatches(vector<acMatch>* matches,vector<acMatch>* filtered);
	void getCorrections(const jchar* s,int slen,vector<acMatch>* corrections,int previousi,int maxcost);
	int getHashForIndex(int hi);
	
	void addTwogramProb(vector<acMatch>* matches,twogram* twograms,int ntgs);
	void addForwardTwogramProb(vector<acMatch>* matches,int hash);
	void addKeyboardDistance(const jchar* s,int slen,vector<acMatch>* matches);
	void addLearnedTwogram(vector<acMatch>* matches,int previoushash);

	bool determinedCorrect(int fi,int ei);

	void getLastWord(int starti,int endi,int* fs,int* fe);

	void getMissedSpaces(const jchar* s,int slen, vector<acMatch>* corrections,int previousi);

	int getTwogramFreq(tgHeader twogram,int hash);

	float keyDist(jchar ch1,jchar ch2);
	float keyLev(const jchar* s1,int len1,const jchar* s2,int len2);

	bool bufferUpdated();

	void makeCorrection(int si,int ei,acMatch* replacement,bool inputUpperCase,int type);

	deque<acchar>::iterator bufferIterator(int pos);
	deque<acchar>::iterator bufferInsertChars(deque<acchar>::iterator it,const jchar* chars,int len);

	// text helper functions
	bool isSeparator(jchar ch);
	jchar toLower(jchar ch);
	jchar toUpper(jchar ch);
	bool isLower(jchar ch);
	bool isUpper(jchar ch);
	bool containsNoLowerCase(const jchar* chars,int len);

	// debug helper functions
	void logout(const char* s,int d);
	void logBuffer();
	void logBufferStatus();

	jobject SystemOut;
	jmethodID midPrintLn;

	void printCorrections(vector<acMatch>* corrections);

	char* jstoc(basic_string<jchar> s) {
		char* ret = (char*)calloc(s.length()+1,1);
		unsigned int c;
		
		for(c = 0;c < s.length();c++) {
			ret[c] = (char)s[c];
		}

		ret[c] = 0;

		return ret;
	}

	//void javalog(const char* s);
};

#endif