
#ifndef SEARCHRESULTS_H
#define SEARCHRESULTS_H

CPPExternOpen
#include "dictrun.h"

	
typedef struct BreadCrumb
{
	BYTE   pref;
	BYTE   endPref;
	BYTE   dictIdx;
	CompactNode *node;
} BreadCrumb;


class CCompactStore;

extern SearchResultEntry *gRankedSearchResults; 

extern PreferredWordResult PWresult;

extern PreferredEndResult PWEresult;

extern int gCurSearchLayer;

CompactNode *findClingonNodeInAssembledPath(CCompactStore *compactStore,  int *nPosP);

void putNodeInAssembledPath(CompactNode *cNode);
void takeNodeFromAssembledPath();
void clearAssembledPath();
void printAssembledPath(int count);

void weakenRepresentedNodes(int eVerbPrefixlen);
void applyClingonMethodOnResults();

void dropBreadCrumb(CompactNode *dNode, BYTE dictIdx, CCompactStore *cstor);
void clearRankedSearchResults();
void clearBreadCrumbCache();
void wipeoutBreadCrumbs();

void addSearchResult(CompactNode *cNode, int pref, MYWCHAR *wordResult, int wrLen, BOOL objectFlag, CCompactStore *compactStore, int dictIdx);

void fillSrep(SearchResultEntry *freesrep, MYWCHAR *text, BOOL bChunkFlag);

void releaseSearchStorage();
void allocateSearchStorage();

BOOL enoughResultsFromAllDictionaries();
BOOL enoughResults(int dictIdx,  eSearchMode searchMode);

void setEVerbPrefixLen(int len);
void setEVerbPrefix(MYWCHAR *prefix=NULL,  BYTE eVerbVarIdx=0xff);
int  getEVerbPrefixLen();
MYWCHAR *getEverbPrefix();
MYWCHAR *getEVerbPrefixStrWordPath();
//BOOL searchResultIsPartOfSearchRoot(MYWCHAR *wordResult);

void restoreBreadCrumbs();

SearchResultEntry *findSearchEntryWithLowestDictIdInCache(MYWCHAR *wordPart, BYTE *charsToCompareTruncated);

void putTheStrongestUniqueFiveInFront();
void includeEVerbPrefixInResults();

//USHORT calcCascadingPreference(CompactNode *cNode, int dictIdx, CCompactStore *compactStore, WordValue wordValue, int layer);
USHORT calcCascadingPreference(CompactNode *cNode, int pref, int dictIdx, CCompactStore *compactStore, WordValue value, int layer);

void storePredictedNextWordsResults();

void lettersSort(PrefLetterNode *allPrefLetters, int nEntries);
void prefCNodeSort(PrefLetterCNode *allPrefCNodes, int nEntries);
int getOrCreateAllPrefLettersIdx(PrefLetterNode *allPrefLetters, int nEntries, MYWCHAR letterToFind);

CPPExternClose

#endif
