#ifndef COMPACTSTORE_H
#define COMPACTSTORE_H


#include "wltypes.h"
#include "compatibility.h"
#include "wordpath.h"

#ifdef WL_SDK
#include <stdio.h>
#endif

#define NPREFERENCES	5
#define NEVERBCASES		4
#define MAX_EVERB_ENDINGS 15
#define MAX_CHUNK_ARR_SIZE_EVERB		2

#define SIZE16MB ((intptr_t)0x1000000)
#define SIZE64KB ((intptr_t)0x10000)
#define SIZE4KB  ((intptr_t)0x1000)

#define HIGHESTPREFERENCE 0xFF   // max value preference, i.e. what is possible from its data type, NOT the max preference that is acceptable!


typedef struct CompactNode 
{
	MYWCHAR Letter;
	BYTE pref;
	BYTE Count;
	BYTE Code;
	BYTE POStag; // primary POS tag for this node, if it is end node of a word. The value is mapped to ePOSTAG
//	BYTE NextCount; // count of next words to follow this word, if this is an end node for the word.
	
	CompactNode()
	{
		Letter = NUL;
		pref = 0;
		Count=0;
		Code =0;
		POStag = 0;
	}
} CompactNode;

const unsigned sBasicSize = sizeof(CompactNode);

typedef struct SerializedEVerbDefinition 
{
	MYWCHAR triggers[4];
	MYWCHAR **variationsOnEVerbs;
} EVerbDefinitionR;

// keep the full struct size sizeof(int_ptr) bytes aligned to prevent misalignment
typedef struct langTreatmentRule 
{
	MYWCHAR triggerChar;
	MYWCHAR *triggerStr;
	MYWCHAR *replacementStr;
	BYTE   minNullMoves:4;
	BYTE   maxNullMoves:4;
    BYTE   terminatedByOneSP:4;
	BYTE  wholeWord:4;
//	bool  correctPrediction:4;
//	bool  correctDisplay:4;
} LangTreatmentRule;
/////////////////////////////////////
typedef struct StartWord
{
	MYWCHAR word[MAX_WORD_LEN];
	BYTE		pref;	
	StartWord()
	{
		word[0] = NUL;
		pref = 0;
	}

}StartWord;

class CCompactDict;
class CWordList;
struct NextWordInfo;
struct DictHeader;
class CWordListSerializer;
void deserializeEVerbs(void *eVerbs, long maxOffset);
void deserializeRules(void *eVerbsTablePtr, long maxOffset);
void serializeEVerbs(void *eVerbsTablePtr, long maxOffset, long memoffset);
void serializeRules(void *rulesTablePtr, long maxOffset, long memoffset);

////////////////////////////////////////////////////////////////////////////////////////////
class CCompactStore
{
public:
    CCompactStore(int dictIdx, eLanguage elang);
    ~CCompactStore();
//#if defined(DEBUG)
	void setCompactDict(CCompactDict *dict) { m_compactDict = dict;}
//#endif
	void resetConfiguration(int dictIdx) { m_dictIdx = dictIdx; }
	BOOL compactToFile(char *szFileName);
	CompactNode *fileToCompact(char *szFileName, BOOL forceFlag);
	BOOL initialize(size_t memSize, BOOL createDict);

	bool CheckPtrValueExist(char *p, UINT val, int count);
	GENPTR queryPtrFieldAtOffset(char *p, int offset);
	void setPtrFieldAtOffset(char *p, int offset, size_t val);
	
	BYTE getEndPreference(CompactNode *node);
	void setEndPreference(CompactNode *currentNode,  BYTE pref);

	ePOSTAG getSecondPOSTag(CompactNode *currentNode);
	void setSecondPOSTag(CompactNode *currentNode, ePOSTAG tag);


//	CompactNode *pointingToObject(CompactNode *cNode, MYWCHAR *word);
	//MYWCHAR *constructNameFromObject(CompactNode *cNode, int *cntP);
	//MYWCHAR *getListName(CompactNode *cNode, int *cntP,  LISTINDICATOR listindicator);
//	MYWCHAR *getDescription(CompactNode *cNode, int *cntP);

	//static BOOL isChunkInCompactStore(CompactNode *cNode) { return ((cNode->Code & CODE_ENDPOINT) && cNode->Count>0); }
	BOOL isChunk(CompactNode *cNode);

	BOOL HasChunkEndings(MYWCHAR *word);
	BOOL HasChunkEndingsForEverbs(CompactNode *endNode);
	static BOOL isEndpoint(CompactNode *cNode) { return (cNode->Code & CODE_ENDPOINT)!=0; }
	static BOOL IsDeadEnd(CompactNode *cNode){ return ((cNode->Code & CODE_ENDPOINT) && cNode->Count==0); }
	CompactNode *allocCompactNode(MYWCHAR letter, BYTE pref, int nFollowChars,
					BYTE endPref, bool dynamic, 
					bool eVerbEndingFlag, bool eVerbRootFlag, BYTE everbVarIdx,
					bool suffixFlag, BYTE startPref, ePOSTAG tag1, ePOSTAG tag2=ePOS_NOTAG); 

	void fillCompactNode(CompactNode *cp, MYWCHAR letter, BYTE pref, BYTE nFollowChars,
		BYTE endPref, BYTE undirCodes, BYTE everbVarIdx,  BYTE startPref, ePOSTAG tag1, ePOSTAG tag2);
	void updateCompactNodeAtIndx(CompactNode *cp, int index, CompactNode *nextCompactNode, int pinPointAt);
	void updateCompactNodePointer(CompactNode *cp, CompactNode *pNode, int pinPointAt);
	void updateCompactNodeValue(CompactNode *cp, int nIndex, int value, int pinPointAt, int sizeOfField);

	BOOL fillBuildHeader(eLanguage lang, MYWCHAR *bondingChars = NULL, int numWords = 0, int numChars=0);
	BOOL serializeDictHeader();

	int getBuildNumber() { return m_dictHeader->buildNumber; }
	int getVersionNumber() { return m_dictHeader->versionNumber; }

	BOOL usedUpEditableSpace(){/*if less than 512 bytes of free space stop it!*/ return ((m_mem_size - m_valid_mem_size) < 0x200);}
	void releaseCompactStore();
	void setAllocatedFirstNode();
	CompactNode *getAllocatedFirstNode() { WLBreakIf(!mRoot, "!!ERROR! CompactStore.h: root is null!!\n"); return mRoot; }
	void allocateFirstEmptyNode();
	static CompactNode ** getFollowPtrs (const CompactNode *cp) {return (CompactNode **)((char*)cp + (size_t)sBasicSize);}
	char *getParentPointer(CompactNode *cp);
	void fixPointers(long memdiff);

//	CompactNode ** getNextWordsPtrs(CompactNode *cp);
//	BYTE *getNextWordPrefsPtrs(CompactNode *cp);

	void setNodeDynamicBit(CompactNode *cp) { cp->Code = cp->Code | CODE_DYNAMIC; }
	
	BOOL isNodeAnEVerbEnding(CompactNode *cp) {return  (cp->Code & CODE_EVERB_ENDING) != 0;}
	BYTE getEVerbVarIdx(CompactNode *currentNode);
	void setEVerbVarIdx(CompactNode *currentNode, BYTE everbVarIdx);

	int getNumRootChildren();
	CompactNode *getChildByIndex(int index, CompactNode *node=NULL);
	CompactNode *nextLetterNode(CompactNode *currentNode, MYWCHAR letter);
	int getInternalPreferredLetters(CompactNode *currentNode, WCHAR *prefLetters, CompactNode *ptrSet[],int nLetToRetrieve, BOOL letOnly);
	void get2MostPreferredNodes(CompactNode *currentNode, CompactNode *ptrSet[]);

//	BOOL doLoadPreferences(TCHAR *szPrefFileName, int bytesPerPreference);
	CompactNode **next2PrefNodes(CompactNode *currentNode);
	inline void setPreference(CompactNode *curNode, BYTE newPref) {curNode->pref = newPref;}
	void reEvaluateBranches(CompactNode *node, BYTE endPref, CompactNode *nextNode,CompactNode *sNextNode);

	BOOL insertAFollowPtr(CompactNode *cNode, int pathPref);
	//BOOL insertNnextPtrs(CompactNode *node, NextWordInfo *nexts, int n);
	//void removeEndpointBit(CompactNode *cNode);
//	CompactNode *createNoEndpointCompactNode(CompactNode *cNode);
	CompactNode * addWord(MYWCHAR *word, int pref);
	int addNextWords(CompactNode *node, NextWordInfo *nexts, int count=1);
	CompactNode * addWordEndNode(CompactNode *endNode, int pref=LOW_PREFERENCE);
	BOOL deleteWord(CompactNode *thisNode);
//	void switchToNewLocation(CompactNode *cNode, CompactNode *orgChildNode, CompactNode *redirChildNode);
	BOOL nonDeletedDependencies(CompactNode *cNode);
//	void removeNodes(CompactNode *startNode);
	BOOL isAlwaysBondingLetter(MYWCHAR letter);
	EVerbDefinitionR *getEVerbs();
	LangTreatmentRule   *getTreatmentRules();
	LangTreatmentRule *findMatchingTreatmentRule(LangTreatmentRule *treatSpecAr, 
								MYWCHAR letter, int *nCommonCharsP, MYWCHAR *flattenedUserCharsBuf, int flatLen, int lastWordLen);

	void setBondingCharsInDictionaryHeader(MYWCHAR *bondingChars);
	MYWCHAR *getWordList(int *count, CompactNode *startNode=NULL);

	CompactNode** getTop10WordsStartingWith(MYWCHAR *startingPart, int *count);

#ifdef WL_SDK
	//BOOL printProximities(MYWCHAR *afterTheseChars);
	//void printPreferences(MYWCHAR *afterTheseChars);
//	void addWordsToWordList(CWordList *wordListObject, CompactNode *startNode, int fileRef);
//	void addWordsToWordList(CWordListSerializer *wordListObject, CompactNode *startNode);
//	void removeWordsFromWordList(CWordList *wordListObject, CompactNode *startNode);
//	void printNodesWithSimilarPrefInfo(CompactNode *startNode, FILE* hFile);
	//BOOL doDumpPreferences(BOOL mapFlag, TCHAR *szPrefFileName, int bytesPerPreference);
	
	char		*m_dictionaryMap;
	BOOL		m_updatedDictionaryMapFlag;
	char		*m_preferenceExtract;
	int			m_bytesPerPreference;
	int			m_nPreferences;
#endif

//	BOOL doDumpPreferences(BOOL mapFlag, TCHAR *szPrefFileName, int bytesPerPreference);
	DictHeader *GetDictHeader() { return m_dictHeader; }
	eLanguage GetDictLanguage() { return m_eLang; }

	CompactNode *generateNodeFromOffset(char *offset);
	int retrieveWordFromLastNode(CompactNode *lastNode, MYWCHAR *wordToFill);
	MYWCHAR *getWordFromNode(CompactNode *node);
	int retrieveWordFromLastNodeInReverse(CompactNode *lastNode, MYWCHAR *wordToFill);
	CompactNode *retrieveEndNodeForString(const MYWCHAR *word, bool isEndpoint2);
	CompactNode * fitInDictionary(const wchar_t *word,CompactNode *cNode, bool isEndpoint2);

	CompactNode *getParent(CompactNode *cNode);
	void setParent(CompactNode *cNode, CompactNode *parent);
	void IncrementDynamicChanges();
	int GetDynamicChangeCount() { return m_nDynamicChanges;}
	
	bool IsThisNoun(CompactNode* node) {return (node && (node->Code & CODE_ENDPOINT) && node->POStag == ePOS_NC);}
	
	char *getOffset() { return m_start_compact; }
//private:

	static int spaceCalc (CompactNode *cp, int toKnowAt);
	static int newSpaceToReserve(int allCode, int toKnowAt, int followCnt);

	void adjustSubset(int cnt, CompactNode **nextPtrs, int memdiff, int shiftPoint);
	void adjustPointers( INT memdiff, GENPTR shiftPoint);	

	CompactNode *mRoot; // first node in word dirctionary tree.

//#if defined(DEBUG)
    CCompactDict   *m_compactDict;
//#endif
	DictHeader		*m_dictHeader;
	DictHeader		*m_buildHeader;
	char			*m_available;
	char			*m_start_compact; // aligned address
	char			*m_block_compact;
	unsigned		m_mem_size;		// valid (filled) memory, rounded on 8K
	unsigned		m_valid_mem_size; // valid (filled) memory in bytes
	unsigned		mTotalDictionarySize; // this is total header + data size. Used in runtime to load dictionary
	eLanguage		m_eLang;
	BYTE			m_dictIdx;
	BYTE			m_nDynamicChanges;  // num of dictinary changes, such as add or delete to updating words, since last flush.
};

#endif // COMPACTSTORE_H
