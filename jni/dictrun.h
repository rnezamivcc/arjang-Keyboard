
#ifndef DICTRUN_H
#define DICTRUN_H

#include "utility.h"

#define  NWORDS 8   // max number of words already accepted in history.
#define  NCURWORDS 16   // max number of current words under construction at the same time!
#define  MAX_LAYER_COUNT 16
#define  NWORDPREFERENCES 5
#define MAXNUMCORRECTIONS 5
#define  NMWORDPREDICTIONS 10
#define  NumSearchWordsPerDict (2*NWORDPREFERENCES)
#define MAX_FOLLOWING_LETTERS  30
#define MAXOBJECTCOUNT 10

const USHORT MAX_PREFERENCE = 0xFFF0;
const USHORT MIN_PREFERENCE = 0x0100;
const USHORT INVALID_PREFERENCE = 0xffff;
typedef enum { eOrdinary=0, eLowValued, eHighValued } WordValue;

#if defined(__LP64__) || defined(_WIN64) 
#define LDAT	(char*)"64.ldat"
#else
#define LDAT	(char*)"32.ldat"
#endif

const BYTE  MAX_NUM_DICTS	= 4;      // max number of dictionaries simultanously loaded. 
const BYTE INVALID_DICT_IDX	= 0xFF;

//const BYTE	NO_SUCCESSOR_PARENT  = 0xfe;
const BYTE	DEF_SUCCESSOR_PARENT = 0;
const BYTE  INVALID_SUCCESSOR      = 0xff;
const BYTE	DEFAULT_Layer	   = 0;
#define EXT_NODE_NOTSET	((ExtPathNode *) 0x00000000)

enum eSearchMode { eWideSearch = 0, eDeepSearch = 1, eNumSearchModes = 2};

typedef enum { eExploring=0, ePredicting }						StepsType;

typedef enum {
	eDictWordPredict=0, 
	eLearnWordPredict, 
	eDictNextPredict, 
	eLearnNextPredict, 
	eCorrectionPrediction
} PredictionType;

struct CompactNode;
// extcnode is basic structure used for representing each word's letters. layer indicates which layer this word belong to in a wordpunc data.
// Each layer represent one subword in wordpunc. This is used for continous typing feature.
// Flavor is used for distinguishing various accented versions of a word. This also includes upper or lower case versions.
typedef struct extcnode
{
	CompactNode		*cNode;
	BYTE			flags;    // first 4 bits indicates current node's layer, last 4 bits idicates current node's flavor
	BYTE			prev;	  // indicates which layer or flavor of previous letter this letter is linked to, to construct the word
	BYTE			next1;    // indicates which layer or flavor of next letter it is attached to. 
	BYTE			next2;    // There are maximum 2 possible next letters.

	//extcnode(): cNode(NULL), flags(0), prev(0), next1(DEF_SUCCESSOR_PARENT), next2(DEF_SUCCESSOR_PARENT) {}

	BYTE	layer() { return flags & 0x0f; }
	BYTE    flavor() { return flags & 0xf0; }
	void    SetLayerFlavor(BYTE layer, BYTE flavor){ flags = (layer & 0x0f) & (flavor << 4);}
	void    SetLayer(BYTE layer) { flags = flags & (layer & 0x0f);}
	void    Setflavor(BYTE flavor) { flags = flags & (flavor << 4);}
	void    SetNext1(BYTE layer, BYTE flavor) { next1 = (layer & 0x0f) & (flavor << 4);}
	void    SetNext2(BYTE layer, BYTE flavor) { next2 = (layer & 0x0f) & (flavor << 4);}
	BYTE   next1Layer() { return next1 & 0x0f; }
	BYTE   next1Flavor() { return next1 & 0xf0; }
	BYTE   next2Layer() { return next2 & 0x0f; }
	BYTE   next3Flavor() { return next2 & 0xf0; }

    extcnode()
	{
        cNode = NULL;
        flags = 0;
        prev = 0;
        next1 = DEF_SUCCESSOR_PARENT;
        next2 = DEF_SUCCESSOR_PARENT;
	}

} ExtCNode;

typedef struct extpathnode 
{
	BYTE  nExtCNodes;		// number of extCnodes to follow. They include various layers and flavors of a letter in a word.
	ExtCNode extCNodes[1];
	extpathnode(): nExtCNodes(0) {}
} ExtPathNode;

// this is the top level entry unit in a word path. Each entry follows the path simultanously in multiple dictionaries.
// first entry, dictNode[0], for each path is the intial node for each active dictionary. Actual word's nodes start from dictNode 1.
typedef struct nodeEntry
{
	ExtPathNode *dictNode[MAX_NUM_DICTS];
	nodeEntry() {reset();}
	void reset() {for(int i=0; i< MAX_NUM_DICTS; i++) dictNode[i]=NULL;}
	void init(int wordIdx, bool historyHeap=false); // initializes to the first node of each active dictionary
} NodeEntry; // first entry of this structure is never used and contains the root of the dictionaries

// contains all properties needed to manage a word under construction at runtime.
typedef struct wordProps 
{
	BYTE		nPathNodes;
	BYTE		nNullMoves;
	BYTE		nSP;
	BYTE        dictIdx;
//	BYTE		objectActive; // this is not used! idea is for linking objects to a word, like contacts, context, grammer, meaning, .....
	WordValue	wordValue;	// how to value this result
	BYTE		maxLayerId;  // maximum top layer index in this word. Each layer represent a word
	BYTE		maxNumExtNodes; // max number of extended nodes upder any pathnode item in word path. These nodes contain both various layes and flavors!
	BYTE		nEVerbChars;
	BYTE		nEVerbUndoMoves;
	BYTE		spacelessTyping;     //  a flag to indicate this string of chars is currently in spaceless typing territory!
	BYTE	    endsInDicts;         // each bit represent if current word or subword ends in the corresponding dict.
	BYTE	    existInLayers;         // each bit represent if current word exists in corresponding layer, say 0x6 means current word only exist in layers 1 and 2 but not 0.
	BYTE   nUndoMoves[MAX_WORD_LEN];		// amount of chars to undo on undoLetterOrWord
	BYTE   layerStartPos[MAX_LAYER_COUNT];
	BYTE   layerEndPos[MAX_LAYER_COUNT];		// nPathNodes at this point before 
	BYTE   layerEndPref[MAX_LAYER_COUNT];
	BYTE   existInDicts[MAX_NUM_DICTS]; // each slot says if corresponding dictionary in currently active dictionary list contains this word up to current point.
	MYWCHAR  charsBuf[MAX_WORD_LEN]; // exactly what the user typed
	MYWCHAR  *charsBufP;   // pointer to the last character in charsBuf
	void addToWordBuf(MYWCHAR userTypedLetter);
	void removeFromWordBuf();
	wordProps()  {reset();}
	void reset();
}WordProps;

typedef struct wordPath 
{
	WordProps wp;
	NodeEntry pathway[MAX_WORD_LEN];
	BYTE	wordIdx;  // index of this word in words history list or working words list.
	                  // For history list, starts at -1 and decreases. For working list, it starts from 0 on.
	BYTE	pref; // probability of this word be used as the current word among all the other under construction words. Used for multi-pointer advance
	void set(int idx);
	void reset(int idx= 0);
	wordPath()
	{
		memset(pathway,0,sizeof(pathway));
		pref = 0;
		wp.reset();
	}

} WordPath;

typedef struct punctuationChoice
{
	MYWCHAR puncChar;
	BYTE   nSPAfter:2;      // num of spaces after punctuation
	BYTE   nSPBefore:2;     // num of spaces before punctuation
	BYTE   canTerminate:2;
	BYTE   canBondWords:2;  // canBondWordsOnPredictionFailure
} PuncChoice;

typedef struct PhraseHeader
{
	eLanguage	eLang;
	UINT phrasesCount;
	UINT startWordCount;
	UINT wordCount;
	UINT rootChildCount;
	UINT totalDataSize; // size of compact data from header on, to be written out to ldat file.
	PhraseHeader()
	{
		eLang = eLang_NOTSET; 
		phrasesCount=wordCount=rootChildCount=totalDataSize=totalDataSize=0;
	}
}PhraseHeader;

typedef struct searchresultentry 
{
	MYWCHAR *text;
	MYWCHAR *predText;		        // before upper case destroys it all
	int textHash;					//  a hash built based on text. This makes string comparision easier!
	PredictionType predType;  // learned prediction versus static predictions
	WordValue  wordValue;	        // how to value this result
	BOOL   isChunk;
	BOOL   everbFlag;			// the terminating node represents an everb
	BOOL   eVerbRetrievalActive;  // looking for words, where the base was recognized has an EVerb
	BYTE   dictIdx;				// the index of the dictionary where this word was found
	BYTE   resultLen;			// length of the word as retrieved during the search
	BYTE   clingonDictIdx;		 // the first dictionary idx (always) which contains clingonNode
	BYTE   clingonCharPos;		 // the position in the remainder of the word for this node
	BYTE   pathPref;		 // separate top layer, preference of the path leading to this word
	BYTE   endPref;    // separate top layer, end preference of this word
//	BYTE   nOffspringWords;		 // number of words stem from this word, like caller from call. 
	BYTE   layer;
	BYTE   eVerbPrefixLen;// number of nodes to backtrack to reach the base of the everb
	BYTE   eVerbPrefixLenQuickList;// number of characters added in the quicklist to support smooth printout
	USHORT cascadingPref;   // cascading preference of this word
	CompactNode *clingonNode;    // the highest chunk node of this word
} SearchResultEntry; 

////////////////////////////////////////////////////////////////////////////////////
typedef struct searchResultsPerDict
{
	BYTE baseIdx; // base index in main SearchResultEntry array where this dictionary range of slots starts from
	BYTE nWords;    // num of occuppied slots currently in this range slots. It is in range [0, NumSearchWordsPerDict)
	BYTE sortedEntries[NumSearchWordsPerDict]; // up to maximum NumSearchWordsPerDict slots in gRankedSearchResults are index to here. range is always kept sorted in ascending pref order. This array is an array of links for that.
	void reset(BYTE baseIdx = 0);
	void sort();
	BYTE next(USHORT pref, MYWCHAR *wordResult, int len, int textHash = 0);      // return next slot suitable for usage as "empty", or to modify existing one. 
	searchResultsPerDict(): baseIdx(0), nWords(0) { }
} SearchResultsPerDict;

typedef struct PreferredWordResult
{
	BYTE nBreadCrumbs;
	BYTE nBreadCrumbsAllocated;
	BYTE nRankedSearchResults;  // total search result from all dictionaries
	BYTE nActiveDictionaries;
	BYTE nApprovedResults;
	BYTE nApprovedSpares;
	SearchResultsPerDict *summarizedResults;
	BOOL areEVerbsSearchResultsFound;
	void reset();
	void PickTopFive();
} PreferredWordResult;

typedef struct PreferredEndResult
{
	CompactNode *endNodes[NumSearchWordsPerDict];
	USHORT prefs[NumSearchWordsPerDict];
	BYTE nWordsFound;
	PreferredEndResult()
	{
		memset(this, 0, sizeof(PreferredEndResult));
	}
	void reset() 
	{
		memset(this, 0, sizeof(PreferredEndResult));
	}
	
	void addNode(CompactNode *node, USHORT pref);
}PreferredEndResult;

////////////////////////////////////////////////////////////////////////////////////

typedef struct prefLetterNode
{
	BYTE pref;
	MYWCHAR posWordChunks;
	MYWCHAR letter;
} PrefLetterNode;

typedef struct prefLetterCNode
{
	BYTE pref;
	CompactNode *cNode;
	MYWCHAR     *variation;
} PrefLetterCNode;


#endif
