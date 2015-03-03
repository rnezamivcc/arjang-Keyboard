#ifndef DICTTREE_H
#define DICTTREE_H

#include "wltypes.h"
#include "wordlist.h"
#include "compatibility.h"

struct PhraseHeader;
class PhraseEngine;
class PhraseEngineR;

struct WordNode;
struct CompactWord;

#define WORDSIDE_DICTIONARY		0x01
#define OBJECTSIDE_DICTIONARY	0x02

#define TREE_RULES		1
#define WORDLIST_RULES	2
#define NPASSES 6
#define MAX_POSTAG_SIZE			150000
class Dictionary;


// there is also a serialized everb definition, 
// this EVerbDefinition is used to analyze the tree,
// part of this EVerbDefinition is written out in serialized form
typedef struct EVerbDefinition 
{
	MYWCHAR triggers[4];
	MYWCHAR **variationsOnEVerbs;
	MYWCHAR  rootChars;
//	BOOL   mustMatchOneOfTheRootChars; // always false for all lang I've seen. So removed it!
//	BOOL   mustNotMatchAnyOfTheRootChars; // always true for all lang I've seen. So removed it!
	MYWCHAR  **mustEndOnOneOfTheseStrings;
	MYWCHAR  **mustNotEndOnOneOfTheseStrings;
} EVerbDefinition;

#define LangTreatmentRule LangTreatmentRule

struct DictNode;
typedef struct NGramNode // data structure for ngrams following this word
{
	NGramNode *Next; 
	DictNode       *endNode; // pointer to end node of this next word in ngram
	DWORD	pref;    // preference of this next word.
	static void sort(struct NGramNode *nodes); 
} NGramNode;

typedef struct DictNode 
{
	DictNode *CharacterList;  // list of nodes containing characters in this word
	MYWCHAR Character;          // character at this node
	DictNode *Next;			  // branching out to other words sharing the characters up to here
	DWORD Preference;         // preference for this letter
	DWORD StartPref;        // preference for when this is the end node for a word which can serve as a starting word in  a sentence.
	DWORD EndPreference;  // if this is the last letter in this word, this is the reference for the word
	DWORD WordOccurence;	// how many times this word has occurred in dictionary, used when scanning for new words Wlsync
	bool  EndPointFlag;         // shows if this is the end character for this word
	bool  SuffixFlag;
	bool  eVerbEndingFlag;
	bool  eVerbRootFlag;
	BYTE  eVerbVarIdx;
	DictNode *parent;
	NGramNode *NextWords;
	//struct ObjectListNode *ObjectList;
	int  *compactNode;	// if non-zero the node is stored in the database
	int state;
	void reset();
} DictNode;

typedef struct WordWithIndicators
{
	MYWCHAR word[MAX_WORD_LEN];
	WordWithIndicators()
	{
		word[0] = NUL;
	}

} WordWithIndicators;

typedef struct PartOfSpeechTag
{
	MYWCHAR posWord[MAX_WORD_LEN];
	ePOSTAG	ePosTag[MAX_POSTAG_NAME_SIZE]; //word can have 4 different tags.

	PartOfSpeechTag()
	{
		for(int i=0; i < MAX_WORD_LEN;i++)
		{
			posWord[i] = NUL;
		}

		for(int i=0; i < MAX_POSTAG_NAME_SIZE;i++)
		{
			ePosTag[i] = ePOS_NOTAG;
		}
	}

} PartOfSpeechTag;


typedef MYWCHAR * (*FuncPtr) (MYWCHAR *word, int *len);


typedef enum { collect, assign } SuffixAction;
typedef enum { DoubleChain, SingleChain } LINKAGE;
typedef enum { OnlyVerbRoots, OnlyNonZeroPref, OnlyZeroPref, 
		       Minimum4Characters, Minimum4CharactersLC, Minimum4CharactersLCExcludeTwoWordPhrases,
		       Minimum5Characters, Minimum5CharactersLC, Minimum5CharactersLCExcludeTwoWordPhrases,
			   Minimum8CharactersLCExcludeTwoWordPhrases, MinimumPref4	, FixWordFreqRelativeToPhrases
	} NodeSelection;

class CParsedLine;
struct CompactNode;
struct StartWord;
class CDictionaryTree
{
public:
	CDictionaryTree(eLanguage lang=eLang_NOTSET);
	~CDictionaryTree();

	DictNode *FindCharInTree(DictNode *prevDictCharNode, MYWCHAR newChar); 
	DictNode *AddCharInTree(DictNode *prevDictCharNode, MYWCHAR newChar,  DWORD dwPreference);
	void addFollowConnection(DictNode *leadNode, DictNode *listNode);
	//BOOL FindObjectInTree(DictNode *fromObjectNode, DictNode *toObjectNode);
	//void AddObjectInTree(DictNode *fromObjectNode, DictNode *toObjectNode);
	//DictNode *storeListObjectInTree(MYWCHAR *lpWord,DWORD dwPreference, MYWCHAR *leadChars);

	void printTree();
	void printTree(char *szFileName);
	void printTreeSort(char *fileName);
	void printTop2000Words();
	void printTreeNodes(char *szFileName, NodeSelection nodeSelection);
	void collectSuffixes();

	void normalizePreferences();

	CompactNode *compactTree(DictNode *startingNode, CompactNode *parent=NULL);
	DictNode *locateEndOfWordInDictionary(MYWCHAR *word);
	DictNode *locateEndOfWordInDictionary(DictNode *prevDictNode, MYWCHAR *word);
	void linkObjectsInCompactTree(DictNode *startingNode);
	void linkNextsInCompactTree(DictNode *startingNode);

	StartWord *processDictFile(	char* lpDictFileName, char *parsingSpec, 
						int storeTypeFlag,  Dictionary *existingDictionary,
						int fileId, int thresHoldPref, BOOL lowerCaseFlag, BOOL unicodeFlag);

	int processStartingWordsList(StartWord *startWords);
	bool processPhraseFile(	char* phraseFileName, char *parsingSpec, BOOL unicodeFlag, int &numPhraseProcessed, bool reset = false, bool bP2P = false);
	void setupPhraseEngine(unsigned totalNumPhrases, unsigned numPhrases, unsigned pageIdx=0);
	void compactPhraseEngine();
	CompactWord* compactPhrases(WordNode *startingNode, CompactWord *parent);
	CompactWord* setupNextPhrases(WordNode *startingNode, CompactWord *parent);

	eEndianNess checkEndianState(FILE *fp);

	BOOL processAddNewWordFromPhraseFile(char* phraseFileName, char *parsingSpec, BOOL unicodeFlag, bool bP2P);
	BOOL processPoSTag(char* PoSFileName, char *parsingSpec, BOOL unicodeFlag);
	BOOL processPhraseFollowupFile(char* FollowupFileName, char *parsingSpec, BOOL unicodeFlag);
	void blendinFile(char* lpDictFileName, char *parsingSpec, 
					 int storeTypeFlag,  Dictionary *existingDictionary, 
					 int fileId, int thresHoldPref, BOOL lowerCaseFlag, BOOL unicodeFlag);

	int storeDictionaryPhrase(	CParsedLine *lineParser, int fileId, int storeTypeFlag, DictNode *listSpecificationNode);
	BOOL addNewWordAsOneUnit(	MYWCHAR *word, int preference, CParsedLine *lineParser, int fileId, int currentStoreTypeFlag,
								DictNode *listSpecificationNode);

	static bool SetDictManager(char *dictname);

	void transferWords(DictNode *startNode, int cutoffFreq, CDictionaryTree *newDictionaryTree);
	void determineHighestOccurence(DictNode *startNode);
	void resetOccurences(DictNode *startNode, int newOccurence);

	//void assignSimilarPreferences(Dictionary *referenceDictionary);
	void printStatistics();
	int  getNumberOfChars() { return m_newCharacters;}
	void countWords(DictNode *startNode);
	void createFrequencyMap(DictNode *startNode);
	void generateListOfNewWords(CWordList *wordListObject, DictNode *startNode, int selectionFlag);
	void processWordList(CWordList *wordListObject,Dictionary *existingDictionary);
	void mergeWordListIntoTree(CWordList *wordListObject,  int whoRules);
	void mergeTreeIntoWordList(CWordList *wordListObject, DictNode *startNode);

	void locateEVerbs(EVerbDefinition *eVerbDefinitions, BOOL markEndingsAsWell);
	void setBondingChars(MYWCHAR *bondingChars);
	void setSerializedEVerbsData(void *serializedEVerbData, int nBytes);
	void setSerializedRuleData(void *serializedRuleData, int nBytes);
	
	CCompactStore *GetCompactStore() { return m_compactStore;}
	BOOL storeDictionaryWord(MYWCHAR *lpWord, DWORD dwPreference,  MYWCHAR *, MYWCHAR *, DictNode *, int storeTypeFlag, int fileId);
	bool compactToFile(char *dictFileName);
	bool compactToLDatFile(char *fullpath);
	BOOL isWordInTree(MYWCHAR *lpWord);
	
	void NGramToFile(char *fileName, eLanguage elang=eLang_NOTSET);

	int m_nNewWords;
	int m_nWordsIgnored;
	int m_newWords;
	int m_newCharacters;
	PhraseHeader	*mPhraseHeader;
	
	static unsigned countNumLinesInFile(char *filename);

private:

	 FILE *mFP;
	MYWCHAR mSepChar; 
	CParsedLine *mLineParser;
	eEndianNess mEndianflag;
	void SetUniqueWordCount(int curIndex);
	void FillPhraseHeader(FILE *fp);
	void markEVerbs(DictNode *curNode, EVerbDefinition *curEVerbDefinition, BYTE eVerbVarIdx, BOOL markEndingsAsWell) ;
	BOOL canApplyEVerbDefinition(EVerbDefinition *curEVerbDefinition, MYWCHAR rootChar, MYWCHAR *wordEnding);
	BOOL nodeHasEVerbVariationEndings(DictNode *curNode, EVerbDefinition *curEVerbDefinition, int *highestPref);
	int findSuffixes(DictNode *dictNode, SuffixAction suffixAction);
	int addToEndings(MYWCHAR *totalWord, int commonSuffixIdx, int suffixLen,  SuffixAction suffixAction);
	void printDictWord(char *word);
	bool containedInNexts(NGramNode *NextWords, DictNode *endNode);
	
	DictNode *storeWord1(MYWCHAR *lpWord, DWORD preference,  MYWCHAR *leadChars, DictNode *listSpecNode, LINKAGE linkage);
	DictNode *storeWord2(MYWCHAR *lpWord, DWORD preference, DictNode *firstWordLastNode, bool bCanChunk, USHORT startPref);

	DictNode *storeObject(MYWCHAR *lpWord, DWORD preference,DictNode *listSpecNode);
	void linkObject(DictNode *fromObjectNode, DictNode *toObjectNode);

	BOOL storeAndCountArticleWord(MYWCHAR *lpWord, int fileId, BOOL ignoreShortWords);
	void addFileReference(DictNode *dictNode, int fileId);

	int SetPhraseWord(MYWCHAR* phrase, MYWCHAR delimiter);
	void StoreNewWordsInDict(MYWCHAR* phrase,MYWCHAR delimiter);

	int SetPhraseToPhraseWord(MYWCHAR* phrase, MYWCHAR delimiter);
	void StoreNewP2PWordsInDict(MYWCHAR* phrase,MYWCHAR delimiter);

	bool SaveNGramWord(MYWCHAR* phrase,MYWCHAR delimiter, int idx);
	int SaveNGramWordFromP2P(MYWCHAR* phrase,MYWCHAR delimiter, int idx);

	///////////////////////////PoS tag/////////////////////////////////////
	int CreatePoSTag(int alphaInd);
	bool SavePoSTag(MYWCHAR* phrase,MYWCHAR delimiter);
	ePOSTAG	ConvertToPoSTagName(MYWCHAR* tagName);
	ePOSTAG* GetPoSTag(MYWCHAR* word);
	bool	FindPosTag(MYWCHAR* word, ePOSTAG* eTag, int searchType);
	void SetPoSTagValue(MYWCHAR* tagName, int alphaInd, int index);
	void PrintPoSTagResult(MYWCHAR* word);
	///////////////////////////////////////////////////////////////////////

	DictNode *m_DictTreeStartNode;

	PhraseEngine *mPhraseEngine; // offline structure to set up phrase engine for building runtime structure
	PhraseEngineR *mPhraseEngineR; // runtime phrase engine. It is built and serialized offline, and deserialized by app at load time

	CCompactStore *m_compactStore;
	eLanguage m_language;
	DWORD m_nNodes;
	DWORD m_nCharsInWordsUsed;
	DWORD m_nCharsInDictionaryUsed;
	DWORD m_nWordsUsed;
	DWORD m_nCompactNodes;
	DWORD m_nFollowPtrs;
	DWORD m_numStartingWords;

	DWORD m_cascadeThruPref; // default is 0, i.e. never cascade in a search to the lower level

	void  *m_serializedEVerbData;
	int	  m_serializedEVerbNDataBytes;
	void  *m_serializedRuleData;
	int   m_serializedRuleNDataBytes;

	MYWCHAR m_bondingChars[MAX_BONDING_CHARS+1];
	int   m_nBondingChars;

	int	  *m_freqDistrMap;
	int   m_freqMapSize;
	DWORD m_highestOccurence;
	int   m_nWordsInPhrase;
	int   m_maxWordsInTree;

	UINT	m_nPref;
	MYWCHAR m_PhraseWords[MAX_TGRAPH_HISTORY_NUM][MAX_WORD_LEN];
	MYWCHAR m_P2PWords[MAX_TGRAPH_HISTORY_NUM*2+2][MAX_WORD_LEN]; //2 for 2 colons
};

#endif   //DICTTREE_H

