// <copyright file="searchResults.cpp" company="WordLogic Corporation">
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
// <summary>Provides engine support for searching dictionaries for next words.</summary>


#include "StdAfx.h"
#include "compactstore.h"
#include "dictmanager.h"
#include "searchResults.h"
#include "wordpunct.h"
#include "wordpath.h"
#include "userWordCache.h"

CPPExternOpen

MYWCHAR  gEVerbPrefix[4+1]; // max length of a prefix is 4, + 1 for null ending.
BYTE   gEVerbVarIdx;
int    gEVerbPrefixLenQuickList = 0;
int    gEVerbPrefixLen = 0;

extern CDictManager *wpDictManager;

SearchResultEntry *gRankedSearchResults = NULL; 

BYTE sMaxSearchRanks = 0;	// in last 10 spots, first 5 contain the five final results, the final 5 contain Everbs
BYTE sNumAllDictSearchSlots = 0;    // this is == sMaxSearchRanks - 10

static MYWCHAR *sRankedSearchBuf = NULL;
static MYWCHAR *sFreeRankedSearchBufP = NULL;
static MYWCHAR *sRankedSearchPredictionBuf = NULL;
static int		sRankedSearchPredictionBufSize = 0;
static MYWCHAR *sFreeRankedSearchPredictionBufP = NULL;

PreferredWordResult PWresult;
PreferredEndResult PWEresult;
int gCurSearchLayer = 0;
static BreadCrumb *sBreadCrumbs = NULL;
static const BYTE sNumBCentries = 0xF0;


//////////////////////////// sAssembledPathNodes /////////////////////////////////////////////
static CompactNode *sAssembledPathNodes[MAX_WORD_LEN+1];
static int sAssembledPathCnt = 0;

void clearAssembledPathCnt()
{
	sAssembledPathCnt = 0;
}

void putNodeInAssembledPath(CompactNode *cNode)
{
	if (sAssembledPathCnt < MAX_WORD_LEN - 1) 
	{
		sAssembledPathNodes[sAssembledPathCnt++] = cNode;
	}
}

void takeNodeFromAssembledPath()
{
	assert (sAssembledPathCnt > 0);
	sAssembledPathNodes[--sAssembledPathCnt] = NULL;
}

void clearAssembledPath()
{
	memset(sAssembledPathNodes, 0, sAssembledPathCnt * sizeof(CompactNode*));
	sAssembledPathCnt = 0;
}

void printAssembledPath(int count)
{
#ifdef DEBUG
	MYWCHAR szBuf[100];
	swprintf((wchar_t *)szBuf, 100, L"%d:   ", count);
	for (int i = 0; i < sAssembledPathCnt; i++)
	{
		szBuf[i+4] = sAssembledPathNodes[i]->Letter;
	}
	szBuf[sAssembledPathCnt+4] = '\n';
	szBuf[sAssembledPathCnt+5] = NUL;
	OutputDebugString(toA(szBuf));
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////
void setEVerbPrefix(MYWCHAR *prefix, BYTE eVerbVarIdx)
{
	int maxlen = 1;
	if(prefix==NULL)
	{
		gEVerbPrefix[0] = gEVerbPrefix[1] = NUL;
	}
	else
	{
		int maxlen = min(mywcslen(prefix), 4);
		mywcsncpy(gEVerbPrefix, prefix, maxlen);
	}
	gEVerbPrefixLenQuickList = maxlen;
	gEVerbVarIdx = eVerbVarIdx;
}

////////////////////////////////////////////////////////////////
int getEVerbPrefixLen()
{
	return gEVerbPrefixLenQuickList;
}

MYWCHAR *getEverbPrefix()
{
	return gEVerbPrefix;
}

void setEVerbPrefixLen(int len)
{
	gEVerbPrefixLen = len;
}
////////////////////////////////////////////////////////////////////////////////////////
void PreferredEndResult::addNode(CompactNode *node, USHORT pref)
{
	WLBreakIf(pref==0 || pref == TERMINATED_PREFERENCE, "!!ERROR! PreferredEndResult::addNode: adding node with forbidden pref!\n");
	MYWCHAR *word = wpDictManager->getWordFromNode(node);
	ShowInfo("PWEResult: add search Word : #%s#, pref=%d\n", toA(word), pref);
	int i = 0;
	for(; i<nWordsFound && prefs[i]; i++)
	{
		if(pref > prefs[i])
		{
			for(int j=nWordsFound; j>i; j--)
			{
				prefs[j] = prefs[j-1];
				endNodes[j] = endNodes[j-1];
			}
			break;
		}
	}

	if(i<NumSearchWordsPerDict)
	{
		prefs[i] = pref;
		endNodes[i] = node;
		nWordsFound = min(nWordsFound+1, NumSearchWordsPerDict);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////
void clearRankedSearchResults()
{
//	if (PWresult.nRankedSearchResults == 0) // nothing happened to it, no need to initialize it again !!
	{
//		PWresult.areEVerbsSearchResultsFound = FALSE;
//		PWresult.nApprovedResults = 0;
//		return;
	}

	memset(gRankedSearchResults, 0, sizeof(SearchResultEntry) * sMaxSearchRanks);
	PWresult.reset();

	sFreeRankedSearchBufP = sRankedSearchBuf;
	memset(sRankedSearchPredictionBuf, 0, sRankedSearchPredictionBufSize);
	sFreeRankedSearchPredictionBufP = sRankedSearchPredictionBuf;
}

//////////////////////////////////////////////////////////////////////////////////////////
void PreferredWordResult::reset()
{
	nRankedSearchResults = 0;
	nApprovedResults = 0;
	nApprovedSpares = 0;
	areEVerbsSearchResultsFound = FALSE;
	for (int i = 0; i < nActiveDictionaries; i++)
		summarizedResults[i].reset( 10 * i);
}

//////////////////////////////////////////////////////////////////////////////////
void allocateSearchStorage()
{
	releaseSearchStorage();
	int numActiveDictionaries = CDictManager::getNumberOfDictionaries();
	sNumAllDictSearchSlots = numActiveDictionaries * NumSearchWordsPerDict; 
	sMaxSearchRanks = sNumAllDictSearchSlots + NumSearchWordsPerDict;  // the last NumSearchWordsPerDict is for final sorted search results and EVerbs.
	ShowInfo("allocateSearchStorage: sNumAllDictSearchSlots = %d, sMaxSearchRanks = %d\n", sNumAllDictSearchSlots, sMaxSearchRanks);
	int size = sMaxSearchRanks * (MAX_WORD_LEN*sizeof(MYWCHAR) + sizeof(SearchResultEntry)) +sizeof (SearchResultEntry);
	gRankedSearchResults = (SearchResultEntry *) calloc(1, size);
	sRankedSearchBuf = (MYWCHAR *) ((unsigned long) gRankedSearchResults + (sMaxSearchRanks + 1) * sizeof(SearchResultEntry));
	sFreeRankedSearchBufP = sRankedSearchBuf;

	sRankedSearchPredictionBufSize = ((MAX_WORD_LEN+1)*sizeof(MYWCHAR) * MAX_NUM_PREDICTIONS);
	sRankedSearchPredictionBuf = (MYWCHAR *) calloc(1, sRankedSearchPredictionBufSize);
	sFreeRankedSearchPredictionBufP = sRankedSearchPredictionBuf;

	PWresult.nRankedSearchResults = 0;
	PWresult.nActiveDictionaries = numActiveDictionaries;
	PWresult.summarizedResults = (SearchResultsPerDict *) calloc(1, sizeof(SearchResultsPerDict) * numActiveDictionaries);
	for(int i=0; i< numActiveDictionaries; i++)
		PWresult.summarizedResults[i].reset( NumSearchWordsPerDict * i);

	sBreadCrumbs = (BreadCrumb *) calloc( 1, sizeof(BreadCrumb) * sNumBCentries);
	PWresult.nBreadCrumbsAllocated = sNumBCentries;
	PWresult.nBreadCrumbs = 0;
}

/////////////////////////////////////////////////////////////////////////////////////
void releaseSearchStorage()
{
	if (gRankedSearchResults)
	{
		free(gRankedSearchResults);
		gRankedSearchResults = NULL;
		sFreeRankedSearchBufP = sRankedSearchBuf = NULL;
		sNumAllDictSearchSlots = sMaxSearchRanks = 0;
	}

	if (sRankedSearchPredictionBuf)
	{
		free(sRankedSearchPredictionBuf);
		sRankedSearchPredictionBuf = NULL;
		sRankedSearchPredictionBufSize = 0;
	}
	
	free(PWresult.summarizedResults);
	PWresult.summarizedResults = NULL;

	if (sBreadCrumbs)
	{
		free(sBreadCrumbs);
		sBreadCrumbs = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
void putTheStrongestUniqueFiveInFront()
{
	PWresult.PickTopFive();
}

bool TopFiveContainsThisResult(int hash)
{
	int start = sNumAllDictSearchSlots+NWORDPREFERENCES;
	int end = start + NWORDPREFERENCES;
	for(int i = start; i<end; i++)
		if(gRankedSearchResults[i].textHash == hash)
			return true;
	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// copy top 5 prefered results to the last 5 location in search buffer, to be presented to user
// strategy: parse each PerDict sector and pick one top result based on absolute cascading preference
void PreferredWordResult::PickTopFive()
{
	ShowInfo("PickTopFive:\n");
	const int nFinalWordsAndSpares = NWORDPREFERENCES + NWORDPREFERENCES;  // we pick NWORDPREFERENCES more results as spare, if possible, in case we have clingon nodes among our finals!
													 // they will replace every final word being cut as a result of applying clingon!
	USHORT backupSearchResultsDictIdx[nFinalWordsAndSpares];
	int backupSearchResultsIdx[nFinalWordsAndSpares];
	int n = 0;
	for(; n <  nFinalWordsAndSpares; n++)
	{
		USHORT curTopPref = 0;
		BYTE chosenIdx = 0;
		BYTE resultSlotIdx = 0;
		for(int i=0; i<nActiveDictionaries; i++)
		{
			for(int j=0; j<NumSearchWordsPerDict; j++)
			{
				BYTE slotIdx = summarizedResults[i].sortedEntries[j];
				if(gRankedSearchResults[slotIdx].dictIdx == INVALID_DICT_IDX)
					continue;
				USHORT respref = gRankedSearchResults[slotIdx].cascadingPref;
				if(curTopPref < respref)
				{
					curTopPref = respref;
					chosenIdx = i;
					resultSlotIdx = j;
				}
			}
		}
		if(chosenIdx < 0 || curTopPref == 0)
			break;
		
		backupSearchResultsDictIdx[n] = gRankedSearchResults[summarizedResults[chosenIdx].sortedEntries[resultSlotIdx]].dictIdx;
		backupSearchResultsIdx[n] = (chosenIdx<<8) | (resultSlotIdx);
		int finalIdx = n<NWORDPREFERENCES? sNumAllDictSearchSlots+NWORDPREFERENCES+n : sNumAllDictSearchSlots+(n-NWORDPREFERENCES);
		if(!TopFiveContainsThisResult(gRankedSearchResults[summarizedResults[chosenIdx].sortedEntries[resultSlotIdx]].textHash))
		{
			gRankedSearchResults[finalIdx] = gRankedSearchResults[summarizedResults[chosenIdx].sortedEntries[resultSlotIdx]];
			ShowInfo("-- [%d]=(#%s#, %d)\n", finalIdx, toA(gRankedSearchResults[finalIdx].text), gRankedSearchResults[finalIdx].cascadingPref);
		}
		else
			n--;
		gRankedSearchResults[summarizedResults[chosenIdx].sortedEntries[resultSlotIdx]].dictIdx = INVALID_DICT_IDX;
	}
	PWresult.nApprovedResults = min(n, NWORDPREFERENCES);
	PWresult.nApprovedSpares = max(0, n - NWORDPREFERENCES);
	ShowInfo("--PickTopFive: nApprovedSearch=%d, spares=%d\n", PWresult.nApprovedResults, PWresult.nApprovedSpares);
	// restore backedup parameters:
	for(int i = 0; i<n; i++)
	{
		int dictSlot = backupSearchResultsIdx[i]>>8;
		int resultSlot  = backupSearchResultsIdx[i] & 0xff;
		gRankedSearchResults[summarizedResults[dictSlot].sortedEntries[resultSlot]].dictIdx = backupSearchResultsDictIdx[i];
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// returns the most suitable entry in the list for overwrite. If there is an empty slot, returns it,
// otherwise finds the lowest prefered entry and returns it for overwrite. Most important thing
// is it does it while keeping the list sorted in descending order! while not doing any
// copying of searchresult entries around!
/////////////////////////////////////////////////////////////////////////
BYTE SearchResultsPerDict::next(USHORT pref, MYWCHAR *word, int len, int hash)
{
	int i=0;
	for(; i< NumSearchWordsPerDict && 
		   gRankedSearchResults[sortedEntries[i]].dictIdx != INVALID_DICT_IDX && 
		  (gRankedSearchResults[sortedEntries[i]].textHash != hash); i++); // looking for already existing search item

	if(i<NumSearchWordsPerDict && gRankedSearchResults[sortedEntries[i]].textHash == hash)
		return sortedEntries[i];

	//looking for shorter version of word if already existing:
	i = min(NumSearchWordsPerDict-1, i);
	if(gRankedSearchResults[sortedEntries[i]].dictIdx != INVALID_DICT_IDX)
	{
		for(int k = 0; k<i; k++)
		{
			MYWCHAR *thisWord = gRankedSearchResults[sortedEntries[k]].text;
			int thisLen = gRankedSearchResults[sortedEntries[k]].resultLen;
			int comLen = min(len,thisLen);
			if(mywcsncmp(thisWord, word, comLen)==0)
			{
				return sortedEntries[k];
			}
		}
	}

	// sorting stage: see if there is an earlier slot whose pref < pref:
	int j = 0;
	for(; j<i; j++)
	{
		if(gRankedSearchResults[sortedEntries[j]].cascadingPref < pref)
			break;
	}
	// if list is full and sorted, then just return last slot and let user decide if they want to ovewrite it with the new data
	if(j==NumSearchWordsPerDict) 
		return sortedEntries[NumSearchWordsPerDict-1];

	// found the right slot to fit in new data, so push all slots from there on one step forward to make room for this new slot:
	int ret = sortedEntries[i];
	while(i>j)
	{
		sortedEntries[i] = sortedEntries[i-1];
		i--;
	}
	sortedEntries[j] = ret;
	return ret;	
}
////////////////////////////////////////////////////////////////////////////
void SearchResultsPerDict::reset(BYTE base)
{
	nWords = 0;
	baseIdx = base;
	for(int i=0; i<NumSearchWordsPerDict; i++)
	{
		sortedEntries[i] = baseIdx+ i;
		gRankedSearchResults[baseIdx+ i].dictIdx = INVALID_DICT_IDX;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// just in case if sorting needed. Usually we won't since the list is kept sorted (refer to next())
void SearchResultsPerDict::sort()
{
	for(int i=1; i< NumSearchWordsPerDict; i++)
	{
		for(int j = 0; j< i; j++)
		{
			if(	gRankedSearchResults[sortedEntries[i]].dictIdx != INVALID_DICT_IDX && 
				gRankedSearchResults[sortedEntries[j]].endPref < gRankedSearchResults[sortedEntries[i]].endPref)
			{
				BYTE temp = sortedEntries[i];
				sortedEntries[i] = sortedEntries[j];
				sortedEntries[j] = temp;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
USHORT calcCascadingPreference(CompactNode *cNode, int pref, int dictIdx, CCompactStore *compactStore, WordValue value, int layer)
{
	USHORT  basePref = wpDictManager->calcCascadingBasePreference(dictIdx, value, layer);
	return basePref + (USHORT)pref;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void addSearchResult(CompactNode *cnode, int pref, MYWCHAR *wordResult, int wrLen, BOOL objectFlag, CCompactStore *cStore, int dictIdx)
{
	//MYWCHAR *originalFreeRankedSearchBufP = sFreeRankedSearchBufP;
	WLBreakIf((mywcslen(wordResult) != wrLen), "ERROR!addSearchResult: len(wordResult) != wrLen!\n");
	int   nCharPos = 0;

	WLBreakIf((cnode == NULL), "\n!!!ERROR!!!addSearchResult: why cnode is INVALID!!!\n");

	//Minkyu:2014.08.01
	//Commented out because it gets weird prediction after backspacing. 
	//Ex)Type "Mo", then backspace("M") and type "o"-->"Mo".
	//Since TGraph checks all upper case thing, Getting consistant prediction from PGraph is necessary.
	
	//if(isUpperCase(wordResult[0]))
	//{
	//	bool userTypedUppercase = isUpperCase(gWordProp->charsBuf[1]);
	//	if(userTypedUppercase)// || sWordPosInPhrase) || (!containsSPs(wordResult) && gWordProp->nPathNodes < 2 && sWordPosInPhrase==1))
	//		gWordProp->wordValue = eHighValued;
	//	else// if(sWordPosInPhrase>1 && !userTypedUppercase)
	//		gWordProp->wordValue = eLowValued;
	//}
	//else
		gWordProp->wordValue = eOrdinary;

	USHORT cascadingPref = calcCascadingPreference(cnode, pref, dictIdx, cStore,  gWordProp->wordValue, gCurSearchLayer);
 //	ShowInfo(("addSearchResult: trying #%s#, cascadingPref=%d, dictIdx=%d endPref=%d, sWordPosInPhrase=%d, wordvalue=%d: \n"), toA(wordResult), cascadingPref, dictIdx, cStore->getEndPreference(cnode), sWordPosInPhrase, gWordProp->wordValue);
	int hash = (int)djb2_hash(wordResult);
	BYTE slotIdx = PWresult.summarizedResults[dictIdx].next(cascadingPref, wordResult, wrLen, hash);
	SearchResultEntry *freesrep = &gRankedSearchResults[slotIdx];
	if(freesrep->textHash == hash) // this is an update to an existing search result
	{
	//	ShowInfo("--already exist! update pref and return\n");
		freesrep->cascadingPref = max(freesrep->cascadingPref, cascadingPref);
		return;
	}

	// take care of chunks and lowest length words
	int minLen = min(wrLen, freesrep->resultLen);
	if(mywcsncmp(wordResult, freesrep->text, minLen)==0)
	{
		freesrep->cascadingPref = max(freesrep->cascadingPref, cascadingPref);
		//freesrep->nOffspringWords++;
		if(wrLen < freesrep->resultLen)
		{
		//	ShowInfo("--longer version exists : #%s#! replace it and return!\n", toA(freesrep->text));
			mywcsncpy(sFreeRankedSearchBufP, wordResult, wrLen);
			freesrep->text = sFreeRankedSearchBufP;
			freesrep->isChunk  = cStore->isChunk(cnode);
			// put spaces or ... after every word
			//if (cStore->isChunk(cnode))
			//{
			//	mywcscat(sFreeRankedSearchBufP, sPeriods);
			//	freesrep->isChunk = TRUE;
			//}
			//else
		//	{
				//mywcscat(sFreeRankedSearchBufP, sSpaces);
			//	freesrep->isChunk = FALSE;
			//}
			// advance buf pointer with string + null Character
			sFreeRankedSearchBufP += mywcslen(sFreeRankedSearchBufP) + 1; // WCHAR pointer arithmetic
			freesrep->resultLen = wrLen;
			freesrep->everbFlag = cStore->isNodeAnEVerbEnding(cnode);
			freesrep->endPref = max(freesrep->endPref, cStore->getEndPreference(cnode));
			freesrep->pathPref = max(freesrep->pathPref, pref);
			freesrep->clingonNode = findClingonNodeInAssembledPath(cStore, &nCharPos);
			freesrep->clingonDictIdx = dictIdx;
			freesrep->clingonCharPos = nCharPos;
			freesrep->layer = gCurSearchLayer;
		}

		return;
	}
	else
		freesrep->textHash = hash;

 //	ShowInfo("--adding it\n");
	freesrep->dictIdx = dictIdx;
	freesrep->endPref = cStore->getEndPreference(cnode);;
	freesrep->cascadingPref = cascadingPref;
	freesrep->pathPref = pref;
	mywcsncpy(sFreeRankedSearchBufP, wordResult, wrLen);

	freesrep->predType = eDictWordPredict;
	freesrep->everbFlag = cStore->isNodeAnEVerbEnding(cnode);
	freesrep->text = sFreeRankedSearchBufP;
	freesrep->resultLen = wrLen;
	freesrep->clingonNode = findClingonNodeInAssembledPath(cStore, &nCharPos);
	freesrep->clingonDictIdx = dictIdx;
	freesrep->clingonCharPos = nCharPos;
	freesrep->layer = gCurSearchLayer;
	freesrep->wordValue = gWordProp->wordValue;
//	freesrep->nOffspringWords = 1;
	freesrep->isChunk = cStore->isChunk(cnode);
	// put spaces or ... after every word
	//if (cStore->isChunk(cnode))
	//{
	//	mywcscat(sFreeRankedSearchBufP, sPeriods);
	//	freesrep->isChunk = TRUE;
	//}
	//else
	//{
	//	mywcscat(sFreeRankedSearchBufP, sSpaces);
	//	freesrep->isChunk = FALSE;
	//}

	// advance buf pointer with string + null Character
	sFreeRankedSearchBufP += mywcslen(sFreeRankedSearchBufP) + 1; // WCHAR pointer arithmetic
	freesrep->eVerbRetrievalActive = wpDictManager->isEVerbRetrievalActive();
	PWresult.summarizedResults[dictIdx].nWords++;
//	discountWordIfRepresented(ssr, freesrep, searchMode);
//	increaseSearchResults(ssr,  searchMode); 
}

///////////////////////////////////////////////////////////////////////////////////////////
void lettersSort(PrefLetterNode *allPrefLetters, int nEntries)
{
	for (int i = 0 ; i < (nEntries - 1); i++)
	{
		int sortingNow = i;
		PrefLetterNode x = allPrefLetters[sortingNow];
		for (int j = i+1 ; j < nEntries; j++)
		{
			if (allPrefLetters[j].pref > x.pref)
			{
				sortingNow = j;
				x = allPrefLetters[j];
			}
		}
		allPrefLetters[sortingNow] = allPrefLetters[i];
		allPrefLetters[i] = x;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////
void prefCNodeSort(PrefLetterCNode *allPrefCNodes, int nEntries)
{
	for (int i = 0 ; i < (nEntries - 1); i++)
	{
		int sortingNow = i;
		PrefLetterCNode x = allPrefCNodes[sortingNow];
		for (int j = i+1 ; j < nEntries; j++)
		{
			if (allPrefCNodes[j].pref > x.pref)
			{
				sortingNow = j;
				x = allPrefCNodes[j];
			}
		}
		allPrefCNodes[sortingNow] = allPrefCNodes[i];
		allPrefCNodes[i] = x;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////
int getOrCreateAllPrefLettersIdx(PrefLetterNode *allPrefLetters, int nMaxEntries, MYWCHAR letterToFind)
{	
	int i = 0;
	for (i = 0 ; i < nMaxEntries; i++)
	{
		if (!allPrefLetters[i].letter || allPrefLetters[i].letter == letterToFind)
		{
			allPrefLetters[i].letter = letterToFind;
			return i;
		}
	}

	WLBreakIf (i==(nMaxEntries - 1), "!!ERROR!!getOrCreateAllPrefLettersIdx: all entries are full!!\n");
	return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////
void includeEVerbPrefixInResults()
{
	for (int i = 0; i < sNumAllDictSearchSlots; i++)
	{
		SearchResultEntry *srep = &gRankedSearchResults[i];
		if (srep->text && srep->eVerbRetrievalActive) 
		{
			PWresult.areEVerbsSearchResultsFound = TRUE;
			break;
		}
	}

	if (PWresult.areEVerbsSearchResultsFound)
	{
		// depending whether the Everbs are used, adjust text or zeroize everbLenQuickList
		for (int j = 0; j < sMaxSearchRanks; j++) 
		{
			SearchResultEntry *srepj = &gRankedSearchResults[j];
			if (srepj->eVerbRetrievalActive == FALSE &&  srepj->text) 
			{
				srepj->text -= gEVerbPrefixLenQuickList; // for non roots make sure we step back to the root, (everbs already done !)
				srepj->resultLen += gEVerbPrefixLenQuickList;
			}
		}
	}
}

///////////////////////////BreadCrumb code///////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
void clearBreadCrumbCache()
{
	PWresult.nBreadCrumbs = 0;
}
void wipeoutBreadCrumbs()
{
	memset(sBreadCrumbs, 0, sizeof(sBreadCrumbs)* sNumBCentries);
	PWresult.nBreadCrumbs = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////
void dropBreadCrumb(CompactNode *node,  BYTE dictIdx, CCompactStore *cStore)
{
	//ShowInfo("dropCrumb: %x: %c, %d\n", node, node->a, node->pref);
	int nBreadCrumbs = PWresult.nBreadCrumbs;
	for (int i = 0 ; i < nBreadCrumbs; i++)
	{
		BreadCrumb *bc = &sBreadCrumbs[i];
		if (bc->node == node)
		{
			//ShowInfo("--arealy crumb exist!\n");
			assert(bc->dictIdx == dictIdx);
			return;
		}
	}

	WLBreakIf (nBreadCrumbs >= PWresult.nBreadCrumbsAllocated, "!!!ERROR!!! dropBreadCrumb: WordLogic end of breadcrumbs\n");
	BreadCrumb *bc = &sBreadCrumbs[PWresult.nBreadCrumbs++];
	bc->node = node;
	bc->dictIdx = dictIdx;
	assert(bc->dictIdx < MAX_NUM_DICTS);
	bc->pref = node->pref;
	bc->endPref = cStore->getEndPreference(node);
}

///////////////////////////////////////////////////////////////////////////////////////////
void restoreBreadCrumbs()
{
	for (int i = 0; i < PWresult.nBreadCrumbs; i++)
	{
		//ShowInfo("BreadCrumb: %x: %c, pref=%d -> %d\n", sBreadCrumbs[i].node, sBreadCrumbs[i].node->a, sBreadCrumbs[i].node->pref, sBreadCrumbs[i].Preference);
		sBreadCrumbs[i].node->pref = sBreadCrumbs[i].pref;
		if(sBreadCrumbs[i].endPref)
		{
			CCompactStore *cStore = wpDictManager->getCompactStore(sBreadCrumbs[i].dictIdx);
			cStore->setEndPreference(sBreadCrumbs[i].node, sBreadCrumbs[i].endPref);
			WLBreakIf((sBreadCrumbs[i].endPref>0) && (sBreadCrumbs[i].node->Code & CODE_ENDPOINT == 0), "!!ERROR! restoreBreadCrumbs: endPref>0 but Code is not set!\n");
		}
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

MYWCHAR *getPrefixLayerPart(int *len, BYTE layer)
{
	static MYWCHAR sPrefixEx[MAX_WORD_LEN];
	memset(sPrefixEx, 0, MAX_WORD_LEN*sizeof(MYWCHAR));
	int clen = 0;
	for(int i=0; i<layer; i++)
	{
		int begin = gWordProp->layerStartPos[i];
		int end =  i < gWordProp->maxLayerId ? gWordProp->layerEndPos[i] : gWordProp->nPathNodes;
		MYWCHAR *thisword = &gWordProp->charsBuf[begin]; 	
		clen += (end+2-begin);
		mywcsncat(sPrefixEx, thisword, end+1-begin);
		sPrefixEx[clen-1] = SP;
	}
//	int depthOffset = gWordProp->layerEndPos[layerId]+1;
//	int endlenghth = gWordProp->charsBufP - &gWordProp->charsBuf[depthOffset];
//	mywcsncat(sPrefixEx, &gWordProp->charsBuf[depthOffset], endlenghth);
	*len = clen;// + endlenghth;
	return sPrefixEx;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
void storePredictedNextWordsResults()
{
	//ShowInfo("storePredictedNextWordsResults: -backouts, prefixlen, predText:\n");
	sFreeRankedSearchPredictionBufP = sRankedSearchPredictionBuf;
	int startIdx =  sNumAllDictSearchSlots+NWORDPREFERENCES; 
	int endIndex = startIdx + PWresult.nApprovedResults;
	unsigned len = 0;
	MYWCHAR *curWord = getCurrentWord(len);
	WLBreakIf(mywcslen(curWord) != gWordProp->nPathNodes, "!!ERROR?? storePredictedNextWordsResults: curWord is not the same length as nPath?\n");
	for (int i = startIdx ; i < endIndex; i++)
	{
		SearchResultEntry *srep = &gRankedSearchResults[i];
		int fulWordLen = srep->resultLen + sNumCharsAdded;
		int prefixlen = 0;
		MYWCHAR * prefixEx = getPrefixLayerPart(&prefixlen, srep->layer);
		if(prefixlen > 0)
		{
			mywcsncpy(sFreeRankedSearchPredictionBufP, prefixEx, prefixlen);
			sFreeRankedSearchPredictionBufP += prefixlen;
			mywcsncpy(sFreeRankedSearchPredictionBufP, srep->text, fulWordLen);
			sFreeRankedSearchPredictionBufP[fulWordLen] = NUL;
			srep->predText = &sFreeRankedSearchPredictionBufP[-prefixlen]; 
		}
		else
		{
			mywcsncpy(sFreeRankedSearchPredictionBufP, &srep->text[0], fulWordLen);
			sFreeRankedSearchPredictionBufP[fulWordLen] = NUL;
			srep->predText = sFreeRankedSearchPredictionBufP; 
		}
		//ShowInfo("--=%d, =%d, =#%s#, ",CDictManager::sNumBackouts, prefixlen,  toA(srep->predText));
		sFreeRankedSearchPredictionBufP += fulWordLen + 1;
	}
	//ShowInfo("\n");
}

// Searches search result entries to find the selected one which corresponds to text "word". 
// It also does a bit of cache management: if there are search results which came from learned cache and 
// none were the the search result selected, then we decrease "importance" of these cache results in cache!
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SearchResultEntry *findSearchEntryWithLowestDictIdInCache(MYWCHAR *word, BYTE *nCharsToCmp)
{
	static SearchResultEntry emptySearchResult;
	SearchResultEntry *srep = NULL;
//	ShowInfo( "findSearchEntryWithLowestDictIdInCache: nApprovedResults=%d, looking for #%s#, gHistWordIdx=%d\n", PWresult.nApprovedResults, toA(wordPart), gHistWordIdx);
	
	int startIdx =  sNumAllDictSearchSlots+NWORDPREFERENCES; 
	int endIndex = startIdx + PWresult.nApprovedResults;
	int cmpLen = *nCharsToCmp;
	for (int i = startIdx ; cmpLen > 0 && i < endIndex; i++)
	{
		SearchResultEntry *thisres = &gRankedSearchResults[i];
		int offsetIdx = 0;
		if(gWordProp->maxLayerId == 0)
			offsetIdx = (gWordProp->nPathNodes>2 && thisres->predText[gWordProp->nPathNodes] == SP) ? gWordProp->nPathNodes+1 : CDictManager::sNumBackouts;
		if (!mywcsncmp(thisres->predText, word, cmpLen)) //srep->resultLen))
		{
			srep = thisres;
			break;
		}
	//	else if(thisres->predictionType == eLearnPredicting)
	//	{
	//		wpDictManager->getUserCache()->depreciate(thisres->predText);
	//	}
	}

	if(!srep)
	{	
		srep = &emptySearchResult;
		srep->dictIdx = INVALID_DICT_IDX;
		srep->predText = word;
		trimEndingSpaces(word);
		srep->resultLen = mywcslen(word);
		srep->eVerbRetrievalActive = FALSE;
		PWresult.areEVerbsSearchResultsFound = FALSE;
		srep->predType = eLearnWordPredict;
		*nCharsToCmp = srep->resultLen;
		ShowInfo("findSearchEntryWithLowestDictIdInCache: searching failed for word #%s# among %d entries!\n", toA(word), PWresult.nApprovedResults);
	}
	return srep;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// fills a searchResultEntry with given info. This is used for inserting cache results in final search array.
void fillSrep(SearchResultEntry *freesrep, MYWCHAR *text, BOOL bChunkFlag)
{
//	ShowInfo("fillSrep: text=%s\n", toA(text));
	memset(freesrep, 0, sizeof(SearchResultEntry));
	freesrep->isChunk = bChunkFlag;
	freesrep->dictIdx = INVALID_DICT_IDX;
	freesrep->resultLen = mywcslen(text);
	freesrep->clingonDictIdx = INVALID_DICT_IDX;
	freesrep->wordValue = gWordProp->wordValue;
	
	mywcsncpy(sFreeRankedSearchBufP, text, freesrep->resultLen);
	freesrep->text = sFreeRankedSearchBufP;
	freesrep->isChunk = bChunkFlag;
	// put spaces or ... after every word
	//if (bChunkFlag)
	//{
	//	mywcscat(sFreeRankedSearchBufP, sPeriods);
	//	freesrep->isChunk = TRUE;
	//}
	//else
	{
	//	mywcscat(sFreeRankedSearchBufP, sSpaces);
	//	freesrep->isChunk = FALSE;
	}

	freesrep->predText = freesrep->text;	//original prediction text, not displayed
	sFreeRankedSearchBufP += mywcslen(sFreeRankedSearchBufP) + 1; // WCHAR pointer arithmetic
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
void weakenRepresentedNodes(int eVerbPrefixlen)
{
	int weakendIdxs[MAX_NUM_PREDICTIONS];
	int nWeakend = 0;

	memset(weakendIdxs,0 , sizeof(weakendIdxs));
	int startIdx = sNumAllDictSearchSlots+NWORDPREFERENCES;
	int maxresults = startIdx + PWresult.nApprovedResults;
	for (int i = startIdx ; i < maxresults; i++)
	{
		SearchResultEntry *srep = &gRankedSearchResults[i];

		//BOOL foundRepresentation = FALSE;
		for (int j = startIdx ; j < maxresults; j++) 	// look in this loop for a representation
		{
			SearchResultEntry *testsrep = &gRankedSearchResults[j];
			if (i != j && testsrep->resultLen < srep->resultLen && mywcsncmp(testsrep->text, srep->text, testsrep->resultLen) == 0)
			{
				// found representation, now weaken this representation such that an everb result can kick it out.
				weakendIdxs[nWeakend++] = i;
				srep->cascadingPref = wpDictManager->calcCascadingBasePreference(srep->dictIdx, eLowValued, srep->layer);
				break;
			}
		}
	}

	// find the lowest weakend search result and give it the lowest cascad pref possible
	int minimalIncrease = 1;
	int nWeakFound = 0;
	while (nWeakFound < nWeakend)
	{
		int lowestIdx = -1;
		USHORT lowestCasPref = 0x7fff;
		for (int k=0; k < nWeakend; k++)
		{
			if (!weakendIdxs[k])
				continue;

			if (gRankedSearchResults[weakendIdxs[k]].cascadingPref < lowestCasPref) 
			{
				lowestIdx = k;
				lowestCasPref = gRankedSearchResults[weakendIdxs[k]].cascadingPref;
			}
		}
		if (lowestIdx == -1)
			return;

		gRankedSearchResults[weakendIdxs[lowestIdx]].pathPref = minimalIncrease;
		nWeakFound++;
		minimalIncrease++;
		weakendIdxs[lowestIdx] = 0;
	}
}

//////////////////////////////////////////////////////////////////////////////////
void applyClingonMethodOnResults()
{
	//ShowInfo("applyClingonMethodOnResults: \n");
	int startIdx = sNumAllDictSearchSlots+NWORDPREFERENCES;
	int maxresults = startIdx + PWresult.nApprovedResults;
	int numSpares = PWresult.nApprovedSpares;

	// first take care of clingon cases
	int clingonedCnt = 0;
	int spareSlot = sNumAllDictSearchSlots;
	for (int i = maxresults-1 ; i >= startIdx; i--)
	{
		SearchResultEntry *srep = &gRankedSearchResults[i];
		if (srep->clingonNode && srep->clingonCharPos > gWordProp->nPathNodes) 
		{
			// make sure the clingon word is not represented by a shorter or equiv base.
			BOOL foundRepresentation = FALSE;
			for (int j = startIdx ; j < maxresults; j++)
			{
				SearchResultEntry *testsrep = &gRankedSearchResults[j];
				if (i != j && testsrep->resultLen <= srep->resultLen && !mywcsncmp(testsrep->text, srep->text, testsrep->resultLen))
				{
					foundRepresentation = TRUE; // found representation, forget about clingon mutilation
					break;
				}
			}
			if (!foundRepresentation && !srep->eVerbRetrievalActive && clingonedCnt < numSpares)
			{
				WLBreakIf(gRankedSearchResults[spareSlot+clingonedCnt].text==NULL, "gRankedSearchResults spareSlot is null for\n");
				srep->dictIdx = srep->clingonDictIdx;

				// shrink the text to reflect the clingonNode 
				srep->isChunk = TRUE;
				int shrinkTo = gWordProp->nPathNodes + srep->clingonCharPos+1;
				//mywcscpy(&srep->text[shrinkTo], sPeriods); 
				//mywcscpy(&srep->text[shrinkTo], sSpaces); 
				srep->resultLen = shrinkTo;
				clingonedCnt++;
			}
		}
	}

	// now take care of possible repeats or when prediction is the same as current word! 
	for (int i = startIdx ; i < maxresults; i++)
	{
		SearchResultEntry *srep = &gRankedSearchResults[i];	
		for(int j= i+1; j < maxresults; j++)
		{
			SearchResultEntry *sreptest = &gRankedSearchResults[j];
			if(gRankedSearchResults[spareSlot].text && srep->resultLen == sreptest->resultLen && mywcsncmp(sreptest->text, srep->text, sreptest->resultLen) == 0)
			{
				gRankedSearchResults[j] = gRankedSearchResults[spareSlot++];
			}
			assert((spareSlot-sNumAllDictSearchSlots) <= numSpares);
		}

		// following takes care of prediction being the same as current word
		if(srep->resultLen == gWordProp->nPathNodes && srep->layer == 0)
		{
			if(spareSlot < startIdx && gRankedSearchResults[spareSlot].text) // if there is a spare result, replace this result with it,
				gRankedSearchResults[i] = gRankedSearchResults[spareSlot++];
			else
			{   // otherwise just eliminiate this result!
				for(int j= i; j<(maxresults-1); j++)
					gRankedSearchResults[j] = gRankedSearchResults[j+1];
				gRankedSearchResults[maxresults-1].text = NULL;
				PWresult.nApprovedResults--;
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
CompactNode *findClingonNodeInAssembledPath(CCompactStore *compactStore, int *nPosP)
{
	if (compactStore) 
	{
		if (gWordProp->nPathNodes <= 2 || sAssembledPathCnt <2)
			return NULL;

//		int minRequiredPref = sAssembledPathNodes[sAssembledPathCnt]->pref;
		// make sure the clingon path has at least two characters, we don't want too short
		*nPosP = 0;
		for (int i = 2; i < (sAssembledPathCnt-1); i++) 
		{
			if (compactStore->isChunk(sAssembledPathNodes[i]))
			{
				*nPosP = i;
				return sAssembledPathNodes[i];
			}
		}
	}
	return NULL;
}

BOOL enoughResultsFromAllDictionaries()
{
	int numResults = 0;
	for(int i=0; i<PWresult.nActiveDictionaries; i++)
		numResults += PWresult.summarizedResults[i].nWords;
	PWresult.nRankedSearchResults = numResults;
	return numResults >= 2 * NWORDPREFERENCES;
}

CPPExternClose