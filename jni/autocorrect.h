#ifndef AUTOCORRECT_H
#define AUTOCORRECT_H

#include <algorithm>
#include <deque>

#include "StdAfx.h"
#include "dictrun.h"


#ifdef __APPLE__
#include "acmatch.h"
#include "keyposition.h"
#else
#include "autocorrect/acmatch.h"
#include "autocorrect/keyposition.h"
#endif

using namespace std;

struct acchar 
{
	acchar(MYWCHAR c):ch(c), status(0) {}
	MYWCHAR ch;
	short status;
};

struct CorrectionList
{
	acMatch corrections[2*MAXNUMCORRECTIONS];
	BYTE fillCount;
	CorrectionList(): fillCount(0) {}
	void addCorrection(MYWCHAR *word, unsigned prob, unsigned score);
	void updateSlot(int slot, MYWCHAR *word, unsigned prob, unsigned score);
	int exist(MYWCHAR *str);
	void singleSort(int newinsert);
	void clear() { fillCount=0; };
};

class CDictManager;
class AutoCorrect 
{
public:
	AutoCorrect(CDictManager *dictMgr);
	~AutoCorrect() {}

	void addChar(MYWCHAR ch);
	void addChars(MYWCHAR* chars, int n);
	void addPrediction(MYWCHAR* chars, int n);
	void backspace(int n);
	void setBeforeCursor(MYWCHAR* chars, int n);
	void setAfterCursor(MYWCHAR* chars, int n);

	void loadKeyPositions(MYWCHAR* kl,float* dm,float* sd, int n);

	bool isWord(MYWCHAR* chars);
	bool correctionPending();
	void reverseCorrection(int si, int olen, int alen, MYWCHAR *ochars);
	float keyDist(MYWCHAR ch1,MYWCHAR ch2);

	void getSecondCorrection(int si, MYWCHAR* ochars, int olen, MYWCHAR* achars, int alen, MYWCHAR** cchars, int* clen);

	int isWord(MYWCHAR* chars, int* prob);
	CorrectionList* getPredictions(int count=MAXNUMCORRECTIONS);

	CorrectionList* getCorrections(MYWCHAR *word);

	bool correctvalidwords;
	bool correctwordcase;

	const static int MaxBufferSize = 512;
	void ClearBuffer() {mBuffer.clear(); }

private:
	deque<acchar> mBuffer;
	//deque<acchar> mAftercursor;
	static CorrectionList mCorrections;
	
	MYWCHAR *getBufferedWord(int startIdx, int len, MYWCHAR *out=NULL);
	CDictManager *mDictMgr;

	MYWCHAR* keylist;
	float* distmatrix;
	float* spacedist;
	int nkeys;

	int filterMatches(acMatch* matches, acMatch* filtered, int num);
	int getCorrections(MYWCHAR* s, int slen, int maxcost, CorrectionList &list=mCorrections);
	void computeRow(CompactNode *node, MYWCHAR* search, int* prow, MYWCHAR* curPath, int depth, int maxcost, CorrectionList &list=mCorrections);
	
	void addKeyboardDistance(MYWCHAR* s, int slen);
	void addLearnedTwogram(acMatch* matches, int previoushash);

	bool determinedCorrect(int fi, int ei);
	MYWCHAR * getLastWord(int starti, int endi, int* fs, int* fe, MYWCHAR *out=NULL);
	void getMissedSpaces(MYWCHAR* s, int slen, CorrectionList &list=mCorrections);
	float keyLev(const MYWCHAR* s1, int len1, const MYWCHAR* s2, int len2);
	bool bufferUpdated();

	int maxLevCost(int len) 
	{ 
		if(len > 3 && len <=5) 
			return 2;
		else if(len> 5)
			return 3;
		return 1;
	}


	void makeCorrection(int si,int ei, acMatch* replacement, bool inputUpperCase, int type);

	deque<acchar>::iterator bufferIterator(int pos);
	deque<acchar>::iterator bufferInsertChars(deque<acchar>::iterator it, MYWCHAR* chars, int len);

	// debug helper functions
	void logBuffer();

	void printCorrections(acMatch* corrections, int len);
};

#endif