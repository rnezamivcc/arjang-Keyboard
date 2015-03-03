#ifndef SWIPE_H
#define SWIPE_H

#include "StdAfx.h"
#include "dictrun.h"

class Swipe {
public:
	struct sPNode{
		float x, y;
		MYWCHAR ch;
		USHORT hitCount;
		sPNode():x(0),y(0),ch(NUL),hitCount(0){}
		void set(float a, float b, MYWCHAR let, int cnt){
			x = a; y = b; ch = let; hitCount = cnt;
		}
	};

	struct sCandid{
		MYWCHAR word[MAX_WORD_LEN];
		USHORT pref;
		void reset(){ pref = 0; word[0] = NUL; }
	};

	Swipe() :length(0), candidCnt(0){}

	static const int PathMax = 25;
	static const int MaxCandidNum = 36;


	void addNode(float x, float y, MYWCHAR let, int count){
		sPath[length++].set(x, y, let, count);
	}

	void reset() { length = 0; }
	
	void processPathAndReset();
	void testPath(MYWCHAR *testword);

private:
	void processPath(int candidIdx, int startWordIdx, int startNodeIdx);
	void postProcess();

	int length; // actual length of chars list in sPath
	sPNode sPath[PathMax];

	int getNextSlot();
	sCandid candids[MaxCandidNum];
	int candidCnt; // number of candid words under construction or already constructed.

};

#endif
