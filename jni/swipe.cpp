
#include "swipe.h"
#include "dictmanager.h"

extern CDictManager *wpDictManager;

/******************************
*	Processes the current swipe path, looking for best word fits.
*	Then fills up mMostPreferredWords array in dictmanager.
*   Finally resets the paths, both in swipe and in mMostPreferredWords, 
*   to have it ready for next call.
******************************/
void Swipe::processPathAndReset()
{
	candidCnt = 0; // clear candid list
	int thiscandidCnt = 0;
	int pos = 0;
	ShowInfo("processPathAndReset: path length=%d\n", length);
	memset(candids, 0, sizeof(candids));
	candids[thiscandidCnt].word[pos] = sPath[thiscandidCnt].ch;
	for (int i = 0; i < 2; i++) // we only check the first 2 letters for starting letter.
	{
		candids[thiscandidCnt].word[pos] = sPath[i].ch;
		if (wpDictManager->getNumWordsStartingWith(candids[thiscandidCnt].word) > 0)
		{
			//this candidate is valid, so spawn a new copy of this candid and start from here
			int nextIdx = getNextSlot();
			memcpy(candids[nextIdx].word, candids[thiscandidCnt].word, (pos+1) * sizeof(MYWCHAR));
			processPath(nextIdx, pos+1, i);
		}
	}
	
	postProcess();

	for (int i = 0; i < MaxCandidNum; i++)
	{
		ShowInfoIf(candids[i].word[0]!=NUL, "processPathAndReset: Candid[%d]=(%s, %d)\n", i, toA(candids[i].word), candids[i].pref);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////
void Swipe::processPath(int candidIdx, int pos, int startNodeIdx)
{
	for (int i = startNodeIdx; i < length; i++)
	{
		candids[candidIdx].word[pos] = sPath[i].ch;
		if (wpDictManager->getNumWordsStartingWith(candids[candidIdx].word) > 0)
		{
			ShowInfo("processPath: start branch for (%s)\n", toA(candids[candidIdx].word));
			//this candid is valid, so spawn a new copy of this candid and start from here
			int nextIdx = getNextSlot();
			assert(nextIdx >= 0);
			memcpy(candids[nextIdx].word, candids[candidIdx].word, (pos + 1) * sizeof(MYWCHAR));
			processPath(nextIdx, (pos + 1), i);
		}
	}
	candids[candidIdx].word[pos] = NUL;
	candids[candidIdx].pref = wpDictManager->isAWord(candids[candidIdx].word);
	bool isWord = candids[candidIdx].pref > 0;
	candids[candidIdx].word[pos*(int)isWord] = NUL;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Processes the results, like sorting, refining prefs and so .
// Output is the list of  5 best candids set in dictmanger.
void Swipe::postProcess()
{
	ShowInfo("Swipe::postProcess()\n");
	///// filter out repeat slots////////////
	for (int i = 0; i < MaxCandidNum; i++)
	{

		if (candids[i].word[0] == NUL || candids[i].word[1] == NUL)
			candids[i].word[0] = NUL;
		else{
			for (int j = i + 1; j < MaxCandidNum; j++)
			{
				if (candids[j].word[0] != NUL && (mywcscmp(candids[i].word, candids[j].word) == 0))
				{
					candids[j].reset();
				}
			}
		}
	}

	//////// adjust prefs if begin and end letter in a candid are the the same as the path.
	for (int i = 0; i < MaxCandidNum; i++)
	{
		if (candids[i].pref > 0)
		{
			MYWCHAR *thisword = candids[i].word;
			int len = mywcslen(thisword);
			candids[i].pref +=	(thisword[0] == sPath[0].ch) * 20 + 
								(thisword[len - 1] == sPath[length - 1].ch) * 20;
		}
	}
	ShowInfo("--postProcess: set and sort mostPreferredWords\n");
	//////// generate sorted output for consumption by dictmanger
	wpDictManager->resetMostPreferredWords();
	for (int i = 0; i < MaxCandidNum; i++)
	{
		MYWCHAR *thisword = candids[i].word;
		if (thisword[0] != NUL)
		{
			wpDictManager->setMostPreferredWord(thisword, candids[i].pref);
		}
	}
	wpDictManager->sortMostPreferredWords();
}

///////////////////////////////////////////////////////////////////
// find next empty slot to use. Only check for word[0] and NOT for
// pref==0 because maybe the slot is under construction and pref is not set yet!
int Swipe::getNextSlot()
{
	for (int i = 0; i < MaxCandidNum; i++)
		if (candids[i].word[0] == NUL)
			return i;
	return -1;
}
////////////////////////////////////////////////////////////////////////////////////
void Swipe::testPath(MYWCHAR *testword)
{
	length = 0;
	int len = mywcslen(testword);;
	for (int i = 0; i < len; i++)
	{
		addNode(0., 0., testword[i], 1);
	}
}

