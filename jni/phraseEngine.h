
#ifndef PHRASEENGINE_H
#define PHRASEENGINE_H

#include "compatibility.h"
#include "dictrun.h"

////////////////// Phrase Engine ///////////////////////////////////
#define BasePref 0x0a // default preference for a phrase.

struct NGramMultiNode;
class CCompactStore;

#define MaxInNextWords 3500  // max number of in tree next words off of each word's node. 
							// Optimization todo: make it dynamic by double processing input file at load time

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Minkyu:2014.06.17
//Changed this from 200 to 500.
//Got crashed while building dictionary because I added more starting words in dictionary and it looked like 
//"mStartWords[MaxNextPhrases]" has not enough slots to save starting words.
#define MaxNextPhrases 500 // max number of phrases to follow a certain phrase.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define MaxRootNextWords 18000 // this is really just a wild guess and should definitely determined dynamically later!

#ifdef WL_SDK

struct CompactWord;

BYTE clampInt2Byte(UINT value);
///////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct WordNode
{
	CompactNode *endCNode; // pointer to endnode of this word. We construct the word using this info!
	WordNode *parent; // pointer to parent node of this word node.
	BYTE POSTags[4]; // reserved for POS tags. Can contain up to 4 tags from the list POSTAGs.
		//phrase continuation. In this case the last WordNode pointer in nextWords point to end WordNode of the next phrase.			
	CompactWord *cWord; // corresponding compactWord constructed for this WordNode

	USHORT count; // actual number of nextWords.
	BYTE  nextPhraseCount; // actual number of next phrases. Maximum is MaxNextPhrases
	BYTE pref; // preference of this word. This has meaning only wrt its parent and specifies its
				// order in children list of the parent.
	BYTE endPref; // preference for the phrase ending at this node. If no phrase ends here, endPref is 0.
	BYTE startFlag; // flag to indicate this word can be starting word for a sentence. 
	BYTE endFlag; // flag to indicate this word ends a phrase.
	WordNode *mNextWords[MaxInNextWords]; // pointer to phrases continuing from this word! This list
											// is kept sorted in descending order.
	WordNode *mNextPhraseEndWord[MaxNextPhrases]; // pointer to end word of next phrases to follow this phrase, 
							// if this is the end word of the phrase. Otherwise all 0!
	BYTE	mNextPhrasePref[MaxNextPhrases]; // preferences for the next phrases
	
	WordNode()
	{
		memset(this, 0, sizeof(WordNode));
	}
	WordNode(CompactNode *endnode, UINT preference, BYTE endflag = 0):
		endCNode(endnode), endPref(0)
	{
		memset(mNextWords, 0, MaxInNextWords*sizeof(WordNode*));
		parent = NULL;
		POSTags[0] = POSTags[1] = POSTags[2] = POSTags[3] = 0;
		startFlag = false;
		cWord = NULL;
		assert(preference > 0 && preference < USHRT_MAX);
		pref = preference;
		if(endflag)
			endPref = pref;
		assert(endPref <= pref);
	}
	void setPOSTags(BYTE tag1, BYTE tag2, BYTE tag3=0, BYTE tag4=0)
	{
		POSTags[0] = tag1;
		POSTags[1] = tag2;
		POSTags[2] = tag3;
		POSTags[3] = tag4;
	}
	void setEndPreference(UINT preference)
	{
		assert(preference > 0);
		endPref = preference;
		assert(endPref <= pref);
	}

	WordNode * addPhrase(CompactNode **endnodes, UINT preference = BasePref);
	bool findPhrase(CompactNode **endNodes, UINT preference = BasePref);
	void update(CompactNode *next, UINT preference, BYTE ends)
	{
		endCNode = next;
		pref = preference;
	}
	void reset();
	WordNode *getNextWordNode(CompactNode *endNode);
	~WordNode();
//	void release(); // releases and deletes all nodes in this subtree
}WordNode;
////////////////////////////////////////////////////////////////////////////////////
class PhraseEngine
{
public:
	struct StartWordUnit
	{
		WordNode *word;
		UINT pref;
	};

	static WLArray sMemArray;  // static array to house PhraseEngine data structure

	WordNode *mHeadWords[MaxRootNextWords];
	USHORT mActualNextCount; // actual count of next Words in nextWords[]
	StartWordUnit mStartWords[MaxNextPhrases]; // assuming at most MaxInNextWords start words exist. Increase it if necessary!
	int mActualStartCount;
	eLanguage mDictIdx; // index of dictionary for which current phrase engine is constructed. 
	static USHORT sTotalPhraseCount;
	static USHORT sTotalWordCount;
	CCompactStore *mCStore; // 
	static bool sInitialized; // we only load one phrase engine, which means only for top dictionary.
	
	PhraseHeader mPhraseFileHeader;

	static WordNode *createAWordNode(CompactNode *pCNode, USHORT pref);


public:
	PhraseEngine(): mDictIdx(eLang_NOTSET), 
					mActualNextCount(0),
					mActualStartCount(0),
					mCStore(NULL)
	{
		memset(mHeadWords, 0 , MaxRootNextWords*sizeof(WordNode*));
		memset(mStartWords, 0 , MaxNextPhrases*sizeof(WordNode*));
	}
	void set(unsigned totalNumNodes, PhraseHeader &header, CCompactStore *cstore);
	//void setMemoryStore(unsigned pageIdx);

	void init(FILE *fp);
	WordNode* findPhrase(CompactNode **endNodes);
	NGramMultiNode *fixupEndNodes(NGramMultiNode *phrase);
	bool shouldBeIgnored(CompactNode *endnode);
	WordNode* addPhrase(NGramMultiNode *phrase, bool fixup=false);
	void addStartPhrase(NGramMultiNode *phrase);
	static bool isInitialized() { return sInitialized; }
	int getMaxNumPhraseCount() { return mPhraseFileHeader.phrasesCount; }
}; // PhraseEngine
#endif // WL_SDK

////////////////////////////////////////////////////////////////////////////////////////////////////
// this class is used in runtime. It is a compactified version of the PhraseEngine tree.
typedef struct CompactWord
{
	USHORT POSTags; // reserved for POS tags. Can contain up to 3  (5 bits each) tags from the list POSTAGs. Last 2 bit is next
	USHORT nextCount; // number of next words following this word, i.e. number of children off of this root node. 
						//!!! Important: the list of next words IS SORTED based on word's compactWord address. IT IS CRITICAL THAT THIS LIST
						// REMAINS SORTED! This is done at data cooking stage and should be maintained if dynamically adding new child nodes.
	BYTE nextPhraseCount; // number of next Phrases that follow this phrase. Note: if this is not 0xFF it means 
		                    // this can be the end word of the current phrase, starting from the root. If it is 0xFF it means
		                    // this word is not an end word for any phrase. Maximum 0xFE next phrases can follow another phrase!
	BYTE	pref;
	void setEndWordFlag() 
	{ 
		POSTags = (1 << 15); 
	}
	bool getEndWordFlag() 
	{
		return (POSTags >> 15) != 0; 
	}
	void setNodePOSTags(BYTE tag1, BYTE tag2)
	{
		assert((tag1 & ~0x3f) == 0);
		assert((tag2 & ~0x3f) == 0);
		USHORT tagset = (tag1 << 6) | (tag2 & 0x3f);
		USHORT res = POSTags >> 12;
		POSTags = res << 12;
		POSTags = POSTags | tagset;
	}
	
	CompactWord(): POSTags(ePOS_NOTAG), nextCount(0), nextPhraseCount(0), pref(0) {}

	//WordNodeR *parent; // pointer to parent node of this word node.
	
		//phrase continuation. In this case the last WordNode pointer in nextWords point to end WordNode of the next phrase.
	//WordNodeR *nextWords; // pointer to phrases continuing from this word! This list is kept sorted in descending order.
							// NOTE!! This field should be the last field in this structure!
	//void initialize(DWORD memSize); 
		
	//void addPhrase(CompactNode **endnodes, BYTE preference = BasePref);
	//bool findPhrase(CompactNode **endNodes, BYTE preference = BasePref);
	//void setNGramArray(int arIdx, int depth);
	void getPOSTags(ePOSTAG &tag1, ePOSTAG &tag2)
	{
		tag1 = (ePOSTAG) (POSTags & 0x3f);
		tag2 = (ePOSTAG) ((POSTags >> 6) & 0x3f);
	}

	~CompactWord();
}CompactWord;

/***************** 
   This is the User Interface to phrase engine at runtime. All queries should be migrated to this
   structure. It's only 4 byte long, but will contains tons of functionality in terms of UI calls.
   Currently you create a PhraseNode and set its compactWord. Then use getNextPhrases(i) to get next 
   phrases following the phrase whose end word node is the compactWOrd. i is used to index into more 
   than one next phrase. So you call for example getNextPhrases(o) to get first one, then getNextPhrases(1)
   to get the second next phrase. When there are no more, it returns null. If there is a next phrase, this
   function returns a PhraseNode for that next phrase. Then you can use getStr() on that phraseNode to
   extract its phrase string as an array of words.
******************/
struct PhraseNode
{
	CompactWord *endCWord;
	USHORT pref;
	BYTE nGrams;
	PhraseNode(CompactWord *cword) : endCWord(cword), pref(BasePref) { nGrams = length();}
	int length();
	MYWCHAR **getStr(int &len);
	PhraseNode *getNextPhrase(int i);
	PhraseNode() : endCWord(NULL) {nGrams = 0; pref = 0;}
	void unset() { endCWord = NULL; nGrams =0; pref = 0; }
	bool isSet() { return endCWord!=NULL; }
	void set(CompactWord *endword) { endCWord = endword; pref = endCWord->pref; nGrams = length();}
	static PhraseNode *getPhraseFromCompactNodeList(CompactNode **nodelist);
	CompactNode **getCNodeList();
};

class PhraseEngineR;
extern PhraseEngineR *gPhraseEngine;
class PhraseEngineR
{
public:
	PhraseEngineR(): m_start_compact(NULL), 
					 m_mem_size(0),
					 mCStore(NULL)
	{
		memset(&mHeader, 0 , sizeof(PhraseHeader));
		mRoot = NULL;
	}

	PhraseEngineR(PhraseHeader &header):m_start_compact(NULL), 
										m_mem_size(0),
										mCStore(NULL)
	{
		memcpy(&mHeader, &header, sizeof(PhraseHeader));
		mRoot = NULL;
	}

	PhraseHeader mHeader;
	CompactWord *mRoot;

	char	*m_available;
	char	*m_start_compact; // aligned address
	char	*m_block_compact;
	DWORD	m_mem_size;		// valid (filled) memory, rounded on 8K
	DWORD	m_valid_mem_size; // valid (filled) memory in bytes

	CCompactStore *mCStore; // 
	static bool sInitialized; // we only load one phrase engine, which means only for top dictionary.
	static bool isInitialized() { return sInitialized; }
	void init(FILE *fp);

	void initialize(DWORD memSize); 
	
	static int spaceCalc (CompactWord *cp, int toKnowAt);
	static int newSpaceToReserve(int toKnowAt, int followCnt, int nextCount, bool isEndPoint);
	
	void setEndCNode(CompactWord *cWord, CompactNode *endNode);
	
	void setParent(CompactWord *cWord, CompactWord *parent);
	CompactWord *getParent(CompactWord *cWord);
	
	void setEndPref(CompactWord *cWord, BYTE endpref);
	BYTE getEndPref(CompactWord *cWord);

	CompactWord *getNextPhrase(CompactWord *cWord, int i, int &pref);
	void setCompactNode(CompactWord *cNode, CompactNode *cnode);
	
	void updateCompactWordAtIndx(CompactWord *cp, int index, size_t value, int pinPointAt);
	void updateCompactWordPointer(CompactWord *cp, size_t value, int pinPointAt);
	void updateCompactWordValueAtIndex(CompactWord *cp, int index, size_t word, int value, int pinPointAt, int sizeValue);

	void setPtrFieldAtOffset(char *p, int offset, size_t val);
	CompactNode *getCWordCNode(CompactWord *cword, int idx, CompactWord **target);
	intptr_t getValueAtIndex(CompactWord *cw, int type, int size, int index, int inc=0, int inOffset=0);
	void setValueAtIndex(CompactWord *cw, int type, int size, int index, int inc, int inOffset, uintptr_t value);
	CompactNode *getWordCompactNode(CompactWord *);

	CompactWord *loadCompact(FILE *fp);

	void adjustCompactNodePointers( INT memdiff, INT shiftPoint, CompactWord *root=NULL);

	CompactWord *allocCompactWord(USHORT POSTags, BYTE pref, int nNextWords, BYTE nextPhraseCount, BYTE endpref=0, bool ending=false);
	void fillCompactWord(CompactWord *wordnode, USHORT POSTags, BYTE pref, int nNextWords, BYTE nextPhraseCount, BYTE endpref, bool ending);

	// POS TAG support:
	void setExtraPOSTags(CompactWord *cWord, BYTE tag3, BYTE tag4=0);
	ePOSTAG* getPOSTags(CompactWord *cWord, int &count);

	// search functions:
	CompactWord *findNextWordChild(CompactWord *root, CompactNode *endNode);
	CompactWord *getRootStartingWithWords(MYWCHAR **startingWords, int numStartingWords=1);
	CompactWord *getRootStartingWithNodes(CompactNode **startingNodes, int numStartingWords=1);
	int setNGrams(CompactNode *end1, CompactNode *end2, MYWCHAR* cmpWord = NULL, int minNGram = 0);
	int setNGramsBasedOnPartGram(MYWCHAR *partWord);
	bool setNGramArray(CompactWord *cw, int arIdx, int minNGram =0);

	PhraseNode *getStartWords(MYWCHAR *lets=NULL);
	int fillStartWords(PhraseNode* arr, MYWCHAR *let);
	BYTE getStartPreference(CompactNode *node);
	void setStartPreference(CompactNode *currentNode,  BYTE pref);
	void addStartingWord(CompactNode *currentNode,  BYTE pref);

	static PhraseNode gPhrases[MAX_FOUND_PHRASES];

	static int sGramArPos;

	static int PhraseNodeCompare(const void *a, const void *b);

}; // PhraseEngineR
#endif
