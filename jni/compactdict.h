#ifndef COMPACTDICT_H
#define COMPACTDICT_H

#include "dictrun.h"

class CCompactStore;

typedef struct searchPathSelect
{
	BYTE pref;
	BYTE textlen;
	CompactNode *cNode;
	MYWCHAR     text[40];
} SearchPathSelect;


class CCompactDict
{
public:
	CCompactDict(CCompactStore *compactStore, int dictIdx, eLanguage elang);
	~CCompactDict() {}

	int getPreferredLetters(PrefLetterNode *allPrefLetters, int nMaxLettersToRetrieve, int *nFilledLettersP, BOOL auCharsOnly);

	void constructWordPath(ExtCNode *excnode);
	ExtCNode *getLayerNode(int path, BYTE extIdx);
	void nextWords(eSearchMode searchMode,  int layer);
	CompactNode *buildAssembledPath(CompactNode *cNode, MYWCHAR *variation);
	void collectEVerbs(int layer, BYTE eVerbVarIdx);

	BOOL isNodeAnEVerbEnding(int *nTrailingChars, BYTE *eVerbVarIdx);
	void takeOutTheEVerb(int nPathNodes, BYTE *pref);
	void putTheEVerbBackIn(int nPathNodes, BYTE *originalPref);

	CompactNode *findMatchingNodeForAlternateLetter(MYWCHAR letter, int junctionPoint, CompactNode **succ2NodeP);

	void backspaceNode ();
	CompactNode * retrieveEndNodeForString(MYWCHAR *word, bool isEndPoint = false);
	void resetConfiguration(int dictIdx) { m_dictIdx = dictIdx; }
	BOOL advancePathWithLetter(MYWCHAR letter, int nPath);
	BOOL fillNextPathNode(MYWCHAR letter, int nPath, USHORT *layers, USHORT *layerEnds);
	BOOL isLayerEndpoint(CompactNode **nodePath, BYTE *layer, BOOL *chunkFlagP = NULL);
	//BOOL isFakeChunk();
	BOOL isChunk(int node);
	BOOL isEndpoint(int node);
	BOOL partOfOtherWords(BOOL wordByItself);
		
	BYTE getLetterPref(MYWCHAR letter); 
	
	void putFirstNodeInOnPathwayNode(int nPath);

	int getNumPossibleWords(CompactNode *currentNode=NULL);
	int getNumWords(CompactNode *currentNode=NULL);

#ifdef WL_SDK
	void printAllWords(CompactNode *currentNode, int count=-1);
	BOOL doCountWord(MYWCHAR *wordPart);
	void printPrefLetters(CompactNode *currentNode);
#elif defined(DEBUG)
	void printAllWords(CompactNode *currentNode, int count=-1) {}
#endif

	int findTop10WordsInSubtree(CompactNode *node);

private:
	CompactNode ** nextNodeLetterCases(CompactNode *cNode, MYWCHAR letter);
	BOOL nextPreferredWord(CompactNode *curNode, eSearchMode searchMode);

	int preferredWordEndNodes(CompactNode *node, eSearchMode searchMode, int maxWord2Find = NumSearchWordsPerDict);
	bool nextPreferredWordEndNode(CompactNode *root, eSearchMode searchMode);
	bool findMostPrefWordEndNode(CompactNode *node, eSearchMode searchMode);

	BOOL findMostPreferredWord(CompactNode *curNode,  eSearchMode searchMode);
	void learnUserPreferences(CompactNode *currentNode, CompactNode *selNode);
	BOOL fillNextPathNodeBC(int nNextPath, ExtCNode *extcNode, MYWCHAR letter, BYTE prev, BOOL *isNextNodeEndNode);
	BYTE topUpPathNode(int nPath, CompactNode *succ1, BYTE prev, BYTE flags);
	int  findEVerbHeader(BYTE *eVerbVarIdx);
	void collectFiveStrongestEVerbs(CompactNode *cNode, BYTE eVerbVarIdx, SearchPathSelect *prefEVerb);
	CompactNode * fitInDictionary(WCHAR *word, CompactNode *cNode=NULL, bool isEndpoint = false);

	CCompactStore *m_compactStore;
	MYWCHAR m_assembledWord[MAX_WORD_LEN];
	MYWCHAR *m_assembledWordPtr;
	int m_dictIdx;
	eLanguage	m_eLang;
};

#endif // COMPACTDICT_H
