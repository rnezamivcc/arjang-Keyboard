// <copyright file="wordpath.cpp" company="WordLogic Corporation">
// Copyright (c) 2000, 2013 All Right Reserved, http://www.wordlogic.com/
//
// THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
// KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// </copyright>
// <author>Reza Nezami</author>
// <email>rnezami@wordlogic.com</email>
// <date>2012-06-10</date>
// <summary>Provides tools and data structure for current word under constructions, as well as an array of previous words.</summary>


#include "StdAfx.h"
#include "utility.h"
CPPExternOpen

#include "compactstore.h"
#include "dictmanager.h"
#include "wordpath.h"
#include "wordpathext.h"
#include "wordpunct.h"

unsigned gHistWordIdx = 0; // current word index in words history. We keep track of last NWORDS accepted by user.
unsigned gWorkingWordIdx = 0; // index of current word index in current possibly multiple parallel paths taken, one for each letter.

WordPath gHistWordPaths[NWORDS]; // wordpaths for already entered words. We keep track of last NWORDS words.
WordPath gWorkingWordPaths[NCURWORDS]; // wordpaths for currently under construction words. 
bool gWorkingPathsAdvanced[NCURWORDS];  
//BYTE gWorkingPathPrefs[NCURWORDS]; // preferece for each of the paths under construction. 0xff means not-active yet.

CompactNode *gFirstNodeOfDicts[MAX_NUM_DICTS];

WordPath *gWPath = &gWorkingWordPaths[0];
NodeEntry *gWPathway = gWorkingWordPaths[0].pathway;
WordProps *gWordProp = &gWorkingWordPaths[0].wp;

extern CDictManager *wpDictManager;


////////////////////////////////////////////////////////////////////////////////////
void resetHistoryWordPath(unsigned idx)
{
	WLBreakIf(idx<0 || idx >  NWORDS, "!!ERROR!resetHistoryWordPath: idx=%d out of range!\n", idx);
	getHistHeap(idx)->reset();
	gHistWordPaths[idx].reset();
}

///////////////////////////////////////////////////////////////////////////////////
// puts the current word under construction in word history and reset all word histories
///////////////////////////////////////////////////////////////////////////
void goToNextWord(char *whoStr, bool endphraseword, bool learn)
{
	ShowInfo("goToNextWord: %s, HistIdx=%d, workingIdx=%d, nPath=%d, nNull=%d, endphraseword=%d\n", 
		whoStr, gHistWordIdx, gWorkingWordIdx, gWordProp->nPathNodes, gWordProp->nNullMoves, endphraseword);
	if(learn)
		wpDictManager->learnCurrentWord(NULL, endphraseword);

	ShowInfo("goToNextWord: calling deepCopyWorkingPath: gHistWordIdx:%d:", gHistWordIdx);
	if(deepCopyWorkingPath(gHistWordPaths[gHistWordIdx], *gWPath, getHistHeap(gHistWordIdx)))
	{
		gHistWordIdx = (gHistWordIdx + 1) % NWORDS;
		ShowInfo("---Done copying: new gHistWordIdx=%d\n", gHistWordIdx);
		resetHistoryWordPath(gHistWordIdx);
		resetAllWorkingWordPaths();
	}
/*	memcpy(&gHistWordPaths[gHistWordIdx], gWPath, sizeof(WordPath));
	gHistWordPaths[gHistWordIdx].wp.charsBufP =
		&gHistWordPaths[gHistWordIdx].wp.charsBuf[gHistWordPaths[gHistWordIdx].wp.nPathNodes+ gHistWordPaths[gHistWordIdx].wp.nNullMoves];

	// deep copy the pathway:
	NodeEntry *pathway = gHistWordPaths[gHistWordIdx].pathway;
	for(int i=0; i<=gWordProp->nPathNodes; i++)
	{
		for(int j=0; j<CDictManager::getNumberOfDictionaries(); j++)
		{
			ExtPathNode *extPathNode = gWPathway[i].dictNode[j];
			if(extPathNode)
			{
				WLBreakIf(extPathNode->nExtCNodes< 1, "!!ERROR! nextWordPath: extPathNode->nExtCNodes(%d) < 1?!!\n", extPathNode->nExtCNodes);
				int sizeToAlloc = sizeof(ExtPathNode) + (extPathNode->nExtCNodes-1) * sizeof(ExtCNode);
				pathway[i].dictNode[j] = (ExtPathNode*)getHistHeap(gHistWordIdx)->allocate(sizeToAlloc);
				memcpy(pathway[i].dictNode[j], extPathNode, sizeToAlloc);
			}
		}
	}
   */
}

///////////////////////////////////////////////////////////////////////////////////
// deep copy current working word path into provided one
bool deepCopyWorkingPath(WordPath &dest, WordPath &src, WLHeap *heap)
{
	if((src.wp.nPathNodes + src.wp.nNullMoves) == 0)
	{
		WLBreakIf(dest.wp.nPathNodes + dest.wp.nNullMoves > 0, "!!ERROR! deepCopyWorkingPath: src is empty but not history!\n");
		//dest.reset();
		//heap->reset();
		return false;
	}
	if((dest.wp.nPathNodes + dest.wp.nNullMoves)> 0)
	{
		dest.reset();
		heap->reset();
	}

	memcpy(&dest, &src, sizeof(WordPath));
	dest.wp.charsBufP = &dest.wp.charsBuf[dest.wp.nPathNodes + dest.wp.nNullMoves];

	// deep copy the pathway:
	NodeEntry *pathway = dest.pathway;
	NodeEntry *srcpathway = src.pathway;
	for(unsigned i=1; i<=dest.wp.nPathNodes; i++)
	{
		for(unsigned j=0; j<CDictManager::getNumberOfDictionaries(); j++)
		{
			ExtPathNode *extPathNode = srcpathway[i].dictNode[j];
			if(extPathNode)
			{
				WLBreakIf(extPathNode->nExtCNodes< 1, "!!ERROR! goToNextWord: extPathNode->nExtCNodes(%d) < 1?!!\n", extPathNode->nExtCNodes);
				int sizeToAlloc = sizeof(ExtPathNode) + (extPathNode->nExtCNodes-1) * sizeof(ExtCNode);
				pathway[i].dictNode[j] = (ExtPathNode*)heap->allocate(sizeToAlloc);
				memcpy(pathway[i].dictNode[j], extPathNode, sizeToAlloc);
			}
		}
	}
	return true;
}
/////////////////////////////////////////////////////////////////////////////////
void goToWorkingWordPath(unsigned idx)
{
	//ShowInfo("goToWorkingWordPath, idx=%d\n", idx);
	WLBreakIf(idx < 0 || idx >= NCURWORDS, "!!ERROR! goToWorkingWordPath! wrong idx(%d)!!\n", idx);
	
	gWPath  = &gWorkingWordPaths[idx];
	gWPathway = gWorkingWordPaths[idx].pathway;  
	gWordProp = &gWorkingWordPaths[idx].wp;
	gWorkingWordIdx = idx;
}
///////////////////////////////////////////////////////////////////////////////////
void resetWorkingWordPath(unsigned idx)
{
//	ShowInfo("resetWorkingWordPath: idx %d\n", idx);
	getWorkingHeap(idx)->reset();
	gWorkingWordPaths[idx].reset();
	gWorkingPathsAdvanced[idx] = false;
}
int getWorkingWordLength(int idx) 
{ 
	return gWorkingWordPaths[idx].wp.nPathNodes + gWorkingWordPaths[idx].wp.nNullMoves; 
}
MYWCHAR *getWorkingWord(int idx)
{
	static MYWCHAR sWorkingWord[MAX_WORD_LEN];
	mywcscpy(sWorkingWord, &gWorkingWordPaths[idx].wp.charsBuf[1]);
	return sWorkingWord;
}
////////////////////////////////////////////////////////////////////////////////////
void resetAllWorkingWordPaths()
{
	ShowInfo("resetAllWorkingWordPaths:\n");
	for(unsigned i=0; i<NCURWORDS; i++)
	{
		if((gWorkingWordPaths[i].wp.nPathNodes + gWorkingWordPaths[i].wp.nNullMoves)>0)
			resetWorkingWordPath(i);
		else
			gWorkingWordPaths[i].pref = 0xFF;
	}
	gWorkingWordIdx = 0;
	gWorkingWordPaths[0].pref = 0xfe; // 0xfe means it is the only working path active and all others are inactive!
	gWPath  = &gWorkingWordPaths[gWorkingWordIdx];
	gWPathway = gWorkingWordPaths[gWorkingWordIdx].pathway;  
	gWordProp = &gWorkingWordPaths[gWorkingWordIdx].wp;
}
////////////////////////////////////////////////////////////////////////////////////
BYTE findNextAvailableWorkingPath()
{
	BYTE lowestidx = 1;
	for(unsigned i=1; i<NCURWORDS; i++)
	{
		if(i==gWorkingWordIdx || gWorkingPathsAdvanced[i])
			continue;
		if(gWorkingWordPaths[i].pref == 0xff)
		{
	//		WLBreakIf(gWorkingPathsAdvanced[i], "!!ERROR! findNextAvailableWorkingPath: how come pref is set but not advance tag?!\n");
			return i;
		}
		else if(gWorkingWordPaths[i].wp.nNullMoves > 0) // wee keep default working path, which is 0, alive even with null move! It's to keep track of main user's inputs
		{
		//	gWorkingPathsAdvanced[i] = false;
			gWorkingWordPaths[i].pref = 0xff;
			return i;
		}
		else
		{
			if(gWorkingWordPaths[i].pref < gWorkingWordPaths[lowestidx].pref)
			{
				lowestidx = i;
			}
		}
	}
	// if here, means all path taken, so find lowest pref one and return its index:
	return lowestidx;
}

////////////////////////////////////////////////////////////////////////////////////
// goes through working paths and remove duplicates. Potential duplications can happen when backspacing!
int pruneWorkingPaths()
{
	int numPrunes = 0;
	MYWCHAR word1[MAX_WORD_LEN];

	for(unsigned i=0; i<NCURWORDS; i++)
	{
		if(gWorkingWordPaths[i].pref == 0xff )
			continue;
		mywcscpy(word1, getWorkingWord(i));
		for(unsigned j=i+1; j<NCURWORDS; j++)
		{
			if(gWorkingWordPaths[j].pref == 0xff )
				continue;
			if(    gWorkingWordPaths[i].wp.nPathNodes == gWorkingWordPaths[j].wp.nPathNodes
				&& gWorkingWordPaths[i].wp.nNullMoves == gWorkingWordPaths[j].wp.nNullMoves
				&& *gWorkingWordPaths[i].wp.charsBufP == *gWorkingWordPaths[j].wp.charsBufP
				&& (mywcscmp(word1, getWorkingWord(j)) == 0))
			{   // duplicate found! remove it:
				gWorkingWordPaths[j].reset();
				gWorkingPathsAdvanced[j] = false;
				numPrunes++;
			}
		}
	}
	return numPrunes;
}

////////////////////////////////////////////////////////////////////////////////////
// initialize first node for all dicts:
void NodeEntry::init(int wordIdx, bool isWorkingPath)
{
	for(int i=0; i<MAX_NUM_DICTS; i++)
	{
		if(gFirstNodeOfDicts[i])
		{
			if(isWorkingPath)
				dictNode[i] = (ExtPathNode *)getWorkingHeap(wordIdx)->allocate(sizeof(ExtPathNode));
			else
				dictNode[i] = (ExtPathNode *)getHistHeap(wordIdx)->allocate(sizeof(ExtPathNode));
			convertCNodeToPathNode(&dictNode[i], gFirstNodeOfDicts[i], wordIdx, 0, 0);
		}
	}
	if(isWorkingPath)
		getWorkingHeap(wordIdx)->setClearStart();
	else
		getHistHeap(wordIdx)->setClearStart();
}
/////////////////////////////////////////////////////////////////////////////////////////////
/// initial setup of a WordPath is done here. Done when dictionaries are reset or changed.
void WordPath::set(int idx)
{
	wp.reset();
	memset(&pathway[0], 0, (MAX_WORD_LEN-1) * sizeof(NodeEntry));
	wordIdx = idx>=0 ?  idx : -idx-1;
	pref = 0xff - (idx==0);
	pathway[0].init(wordIdx, idx >=0);
}
/////////////////////////////////////////////////////////////////////////////////////////////
/// resets wordpath, assuming wordpath is already set and its first pathway node is initialized.
/// This is to be called during regular reseting of wordpath for next use.
void WordPath::reset(int idx)
{
	wp.reset();
	pref = 0xff - (idx==0);
	wordIdx = idx>=0 ?  idx : -idx-1;
	memset(&pathway[1], 0, (MAX_WORD_LEN-1) * sizeof(NodeEntry));
}

/////////////////////////////////////////////////////////////////////////////////////////////////
void clearRankedSearchResults();
void fullInitializeWordPaths(bool soft)
{
	ShowInfo("fullInitializeWordPaths\n");
	for (int i = 0 ;  i < NWORDS; i++)
	{
		if(soft && gHistWordPaths[i].wp.nPathNodes == 0)
			continue;
		if(soft)
			gHistWordPaths[i].reset(-i-1);
		else
			gHistWordPaths[i].set(-i-1);
	}
	for (int i = 0 ;  i < NCURWORDS; i++)
	{
		if(soft && gWorkingWordPaths[i].wp.nPathNodes == 0)
			continue;
		if(soft)
			gWorkingWordPaths[i].reset(i);
		else
			gWorkingWordPaths[i].set(i);
	}
	
	if(!soft)
	{
		gHistWordIdx = 0;
		setUpWordPathPointers(false);
	}
	else
		clearRankedSearchResults();

	gWorkingWordPaths[0].pref = 0xfe;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void wipeOutNode(NodeEntry *nspe, int pathIdx)
{
	for (int j=0; j < MAX_NUM_DICTS; j++) 
	{
		nspe[pathIdx].dictNode[j] = EXT_NODE_NOTSET;
		gWordProp->existInDicts[j] = true;
	}
	int toplayer = gWordProp->maxLayerId;
	if(toplayer > 0)
	{
		toplayer--;
		if(gWordProp->layerEndPos[toplayer] >= pathIdx)
		{
			gWordProp->layerEndPos[toplayer] = 0;
			gWordProp->layerEndPref[toplayer] = 0;
			gWordProp->layerStartPos[toplayer+1] = 0;
			gWordProp->maxLayerId = toplayer;
			gWordProp->maxNumExtNodes = toplayer+1;  // not an exact way of updating maxNumExtNodes, but good enough for now! have to find a better way if problem arises!
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
CompactNode *getNodePathAtPosForDict(int pos, int dictId, int layer)
{
	ExtPathNode *extPathNode = gWPathway[pos].dictNode[dictId];
	if (extPathNode == NULL)
		return NULL;

	ExtCNode *curExtCNodes = extPathNode->extCNodes;
	for (int i = 0; i < extPathNode->nExtCNodes; i++)
	{
		ExtCNode *lp = &curExtCNodes[i];
		if(lp->layer() == layer)
			return lp->cNode;
	}

	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////
void setUpWordPathPointers(bool inHistory)
{
	if(inHistory)
	{
		gWPath = &gHistWordPaths[gHistWordIdx];
		gWPathway = gHistWordPaths[gHistWordIdx].pathway;  
		gWordProp = &gHistWordPaths[gHistWordIdx].wp;
		if(emptyWordPath())
			getHistHeap(gHistWordIdx)->reset();
	}
	else
	{
		gWPath = &gWorkingWordPaths[gWorkingWordIdx];
		gWPathway = gWorkingWordPaths[gWorkingWordIdx].pathway;  
		gWordProp = &gWorkingWordPaths[gWorkingWordIdx].wp;
		if(emptyWordPath())
			getWorkingHeap(gWorkingWordIdx)->reset();
	}
}
///////////////////////////////////////////////////////////////////////////
void BackToPreviousWord(bool doCopy)
{
	ShowInfo("BackToPreviousWord: gHistWordIdx=%d, gWorkingWordIdx=%d, doCopy=%d", gHistWordIdx, gWorkingWordIdx, doCopy);
	if (gHistWordIdx == 0)
		gHistWordIdx = NWORDS - 1;
	else
		gHistWordIdx = (gHistWordIdx - 1) % NWORDS;

	if(doCopy)
	{
		deepCopyWorkingPath(gWorkingWordPaths[gWorkingWordIdx], gHistWordPaths[gHistWordIdx], getWorkingHeap(gWorkingWordIdx));
		resetHistoryWordPath(gHistWordIdx);
	}
	else
		setUpWordPathPointers(true);
	unsigned len=0;
	ShowInfo(": new currword=#%s# \n", toA(getCurrentWord(len)));
}
////////////////////////////////////////////////////////////////////////////////////////
BOOL BackToPreviousWordIfAny(bool doCopy)
{
	if (emptyWordPath())
	{
		BackToPreviousWord(doCopy); // modifies gWordProp and gWpSpace !!
		if (!emptyWordPath())
			return TRUE;
		ShowInfo("!!BackToPreviousWordIfAny: currWordIdx=%d: previous word is empty! so back to current word!\n", gHistWordIdx);
		BackToWorkingWord();
	}
	return FALSE;
}
//////////////////////////////////////////////////////////////////////////////
bool BackToWorkingWord()
{
	ShowInfo("BackToWorkingWord: gHistWordIdx=%d, gWorkingWordIdx=%d ", gHistWordIdx, gWorkingWordIdx);
	gHistWordIdx = (gHistWordIdx + 1) % NWORDS;
	setUpWordPathPointers(false);
	unsigned len=0;
	ShowInfo(": new currword=#%s# \n", toA(getCurrentWord(len)));
	return true;
}
/////////////////////////////////////////////////////////////////////////////////////////////////
BOOL matchesFirstNodes(CompactNode *cNode)
{
	int nDicts = wpDictManager->getNumberOfDictionaries();
	for (int j = 0; j < nDicts; j++) 
	{
		if (cNode == gFirstNodeOfDicts[j])
			return TRUE;
	}
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////
void WordProps::addToWordBuf( MYWCHAR letter)
{
	charsBufP++;
	*charsBufP = letter;
}
//////////////////////////////////////////////////////////////////////////////////////
void WordProps::removeFromWordBuf()
{
	WLBreakIf ((nNullMoves + nPathNodes) <= 0, "!!removeFromWordBuf: path length ==0!");
	WLBreakIf (charsBufP <= (WCHAR *) &charsBuf[0], "!!removeFromWordBuf: charsBufP out of sync!!");
	if(*charsBufP==SP)
	{
		WLBreakIf(nSP<=0, "!!removeFromWordBuf! nSP out of sync!!");
		nSP--;
	}

	*charsBufP = NUL;
	charsBufP--;
	if(nNullMoves > 0)
		nNullMoves--;
	else
		nUndoMoves[nPathNodes--] = 0;
}
//////////////////////////////////////////////////////////////////////////////////////////////
void putLetterInWordpath(MYWCHAR letter, BYTE dictIdxFittedIn, PuncChoice *pc)
{
	gWordProp->addToWordBuf(letter);
	if (dictIdxFittedIn > 0) 
	{
		gWordProp->nPathNodes++;
		gWordProp->dictIdx = dictIdxFittedIn;
		gWordProp->nUndoMoves[gWordProp->nPathNodes] = 1;
	}
	else
	{
		gWordProp->nNullMoves++;
	}
	gWordProp->nSP += (letter==SP);
	if(pc && pc->nSPAfter>0)
	{
		gWordProp->addToWordBuf(SP);
		gWordProp->nNullMoves++;
		gWordProp->nSP++;
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void  WordProps::reset() 
{
	memset(this, 0, sizeof(wordProps)); 
	charsBufP = charsBuf; 
	layerStartPos[0] = 1;
	for(int i=0 ;i<MAX_NUM_DICTS; i++)
		existInDicts[i] = true;   // at start of a new word, we assume all dictionaries may contain this word!
	
	//memset(nUndoMoves, 1, MAX_WORD_LEN); Reza: is it needed??
//	CDictManager::sNumBackouts = 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////	
bool startPathWithNode(CompactNode *node, int dictIdx)
{
	ExtPathNode *extPathNode = gWPathway[0].dictNode[dictIdx];
	WLBreakIf(extPathNode == EXT_NODE_NOTSET, "!!ERROR! startPathWithNode: first node is not set for dictIdx=%d\n", dictIdx);
	convertCNodeToPathNode(&extPathNode, node, gWorkingWordIdx, 0, 0);
	gWPathway[0].dictNode[dictIdx] = extPathNode;
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////
bool previousWordEndsWithSP()
{
	bool ret = false;
	if(BackToPreviousWordIfAny())
	{  
		unsigned len = 0;
		ShowInfo("previousWordEndsWithSP:backToCurrentWord=true word=#%s#\n",  toA(getCurrentWord(len)));
		if(*gWordProp->charsBufP == SP)
			ret = true;
		BackToWorkingWord();
	}

	return ret;
}
////////////////////////////////////////////////////////////////////////////////////////
bool firstCharAfterConnectionChar()
{
	PuncChoice *pc = findPunctuationChoice(*(gWordProp->charsBufP));
	return (pc && pc->canBondWords);
}

///////////////////////////////////////////////////////////////////////////////////
bool FirstMoveAfterSPTerminatedWord()
{
	bool ret = false;
	if (BackToPreviousWordIfAny())	
	{
		if(*gWordProp->charsBufP == SP)
			ret = true;
		BackToWorkingWord();    
	}
	return ret;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL terminatedByOneSpace()
{
	int nNodes = gWordProp->nPathNodes + gWordProp->nNullMoves;
	return nNodes > 1 && (gWordProp->charsBuf[nNodes] == SP && gWordProp->charsBuf[nNodes-1] != SP);
}

//////////////////////////////////////////////////////////////
#ifdef _DEBUG
void printWordPaths(char *whenText)
{
	ShowInfo(("history %s :\n"), whenText);
	for (int i = 0; i < NWORDS; i++)
	{
		ShowInfo(("--[%d] #%s# PaN %d nNu %d nExS %d \n"),i,toA(&gHistWordPaths[i].wp.charsBuf[1]), 
			gHistWordPaths[i].wp.nPathNodes, gHistWordPaths[i].wp.nNullMoves, gHistWordPaths[i].wp.nSP);
	}
}
#endif

bool isCurWordNotInDictionaries() { return gWordProp->nNullMoves  > 0;  }

//////////////////////////////////////////////////////////////////////////
int constructCurWord(WCHAR *word)
{
	if(gWordProp->nPathNodes + gWordProp->nNullMoves == 0)
	{
		word[0] = NUL;
		return 0;
	}
	int len = mywcscpy(word, &gWordProp->charsBuf[1]);
	len -= trimEndingSpaces(word);
	return len;
}
////////////////////////////////////////////////////////////////////////////
MYWCHAR * getCurrentWord(unsigned &len, bool stripped) 
{
	static MYWCHAR s_currWord[MAX_WORD_LEN];
	memset(s_currWord, 0 , sizeof(s_currWord));
	len = mywcscpy(s_currWord, &gWordProp->charsBuf[1]);
	if(stripped)
		len -= trimEndingSpaces((MYWCHAR*)s_currWord);
	return s_currWord;
}
/////////////////////////////////////////////////////////////////////////////
void getNLastCharacters(MYWCHAR *buf, int nChars)
{
	int len = (int)(gWordProp->charsBufP - &gWordProp->charsBuf[0]);
	WLBreakIf(nChars > len, "!!!ERROR:getNLastCharacters!\n");
	mywcsncpy(buf, gWordProp->charsBufP - nChars + 1, nChars);
	buf[nChars] = NUL;
}
///////////////////////////////////////////////////////////////////////////////
MYWCHAR getCharAtPosition(unsigned n)
{
	WLBreakIf((n < 0 || n > gWordProp->nPathNodes), "!!ERROR!! getCharAtPosition: pos out of range!\n");
	return gWordProp->charsBuf[n];
}

///////////////////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
void printWordPathInfo()
{
	ShowInfo(("=====> printWordPathInfo: cwi [%d] <============\n"), gHistWordIdx);
	for (int k = 0; k < NWORDS; k++)
	{
		WordProps *mywp = &(gHistWordPaths[k].wp);
		ShowInfo(("cwi [%d] PathNodes %d nNullMo %d, ImSpa %d ExSpa %d"),
			k,mywp->nPathNodes,mywp->nNullMoves, mywp->nSP);
	}
}
#endif

//////////////////////////////////////////////////////////////////////////////////
//void deleteWordFromPersonalDictionary(MYWCHAR *word)
//{
//	wpDictManager->mngrDeleteWordFromPersonal(word);
//}

#ifdef DEBUG
void printNodePath(int wpIdx, int dictIdx, int nNodes)
{
	WordPath *mycwp = &gHistWordPaths[wpIdx];
	NodeEntry *mycwpspace = mycwp->pathway;

	for (int k=0; k <= nNodes+1; k++)
	{
		ExtPathNode *extPathNode = gWPathway[k].dictNode[dictIdx];
		if (extPathNode == NULL)
		{
			CompactNode *cNode = mycwpspace[k].dictNode[dictIdx]->extCNodes[0].cNode;
			ShowInfo(("dict %d: WordPath[%d] : node %d: %x"), dictIdx, wpIdx, k, cNode);
		}
		else
		{
			ExtCNode *curExtCNodes = extPathNode->extCNodes;
			for (int i = 0; i < extPathNode->nExtCNodes; i++)
			{
				ExtCNode *lp = &curExtCNodes[i];
				ShowInfo(("dict %d: WordPath[%d] : node %d: %c  %x, layer=%d, flavor=%d\n"),
					dictIdx, wpIdx,k, lp->cNode != gFirstNodeOfDicts[dictIdx] ? *(char *) lp->cNode : SP, lp->cNode, lp->layer(), lp->flavor());
			}
		}
	}
}
#endif


//////////////////// gWordString //////////////////////////////////////////////////////
struct wordStringStr
{
	MYWCHAR word[MAX_WORD_LEN+1];
	MYWCHAR *wordP;
	BYTE Count;
	wordStringStr()
	{
		reset();
	}
	void reset()
	{
		word[0] = 0;
		wordP = word;
		Count = 0;
	}
	void addChar(MYWCHAR c)
	{
		assert(Count < (MAX_WORD_LEN-1));
		{
			Count++;
			*wordP++ = c;
			*wordP = NUL;
		}
	}
	void removeChar()
	{
		assert(Count > 0);
		Count--;
		*(--wordP) = NUL;
	}
	MYWCHAR *getString()
	{
		return word;
	}
};
static wordStringStr gWordString;

void putCharInWordString(MYWCHAR c)
{
	gWordString.addChar(c);
}
void takeCharOffWordString()
{
	gWordString.removeChar();
}
MYWCHAR *getWordString()
{
	return gWordString.getString();
}
BYTE getStringLength() 
{ 
	return gWordString.Count; 
}

///////////////////////////////////////////////////////////////////////////////////////

CPPExternClose