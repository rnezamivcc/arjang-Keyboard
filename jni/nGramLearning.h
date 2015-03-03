#ifndef _NGRAMLEARNING_H_
#define _NGRAMLEARNING_H_

#include "wltypes.h"
#include "utility.h"
#include "compactstore.h"
#include "dictionary.h"

#define LEARNED_PHRASE_PREF				3			
#define BULKLOADING_PHRASE_PREF			LEARNED_PHRASE_PREF*4
#define MIN_NGRAM_LEARNING_FREQUENCY	21
#define MAX_NGRAM_LEARNING_FREQUENCY	250

/////////////// support for starting word caching ///////////////////////
typedef struct CompactNodeBlk
{
	CompactNode *endCNode;
	USHORT startPref;
}CompactNodeBlk;
/////////////////////////////////////////////////////////////////////////////////////////
typedef struct StartWordCache
{
	CompactNodeBlk startWords[MAX_START_WORD_CACHE_SIZE];
	StartWordCache() { reset(); }
	void reset()
	{
		for(int i=0; i<MAX_START_WORD_CACHE_SIZE; i++)
			startWords[i].endCNode = NULL;
		count = countSinceSorted =countSinceLastSaved = lowestPrefIdx = 0;
		cacheFilename[0] = NUL;
	}
	void addStartWord(CompactNode *node, USHORT pref);
	void deleteStartWord(CompactNode *node);

	CompactNode **getTop5StatWords(MYWCHAR *head=NULL);
	USHORT GetStartWordPref(CompactNode *node);
	void SetSavedStartWordPref(CompactNode *node, USHORT newPref);

	USHORT count; // number of words in cache
	USHORT countSinceSorted; // number of updates been done to cache since last sorting.
	USHORT countSinceLastSaved; // number of updats since last time cache was saved out to cachefile.
	USHORT lowestPrefIdx;
	char cacheFilename[64];
	void saveOutCachedStartingWords();
	void loadStartWordsCacheFromFile(const char*filename);
	static int NodeCompare(const void *a, const void *b);
}StartWordCache;
/////////////////////////////////////////////////////////////////////////////////////////
typedef struct N3GramLearningBlk
{
	CompactNode* p3GramNode[N3Gram];
	bool		b3FirstUpperCase[N3Gram];
	USHORT learnPref3;
	N3GramLearningBlk()
	{
		memset(this, 0, sizeof(N3GramLearningBlk));
	}
}N3GramLearningBlk;
/////////////////////////////////////////////////////////////////////////////////////////
typedef struct N4GramLearningBlk
{
	CompactNode* p4GramNode[N4Gram];
	bool		b4FirstUpperCase[N4Gram];
	USHORT learnPref4;
	N4GramLearningBlk()
	{
		memset(this, 0, sizeof(N4GramLearningBlk));
	}
}N4GramLearningBlk;

/////////////////////////////////////////////////////////////////////////////////////////
typedef struct N2GramLearningBlk
{
	CompactNode* p2GramNode[N2Gram];
	bool		b2FirstUpperCase[N2Gram];
	USHORT learnPref2;
	N2GramLearningBlk()
	{
		memset(this, 0, sizeof(N2GramLearningBlk));
	}
}N2GramLearningBlk;

/////////////////////////////////////////////////////////////////////////////////////////
typedef struct N2GramLearningNoDictBlk
{
	MYWCHAR		arr2GramString[N2Gram][MAX_WORD_LEN];
	USHORT learnPref2;
	N2GramLearningNoDictBlk()
	{
		memset(this, 0, sizeof(N2GramLearningBlk));
	}
}N2GramLearningNoDictBlk;

/////////////////////////////////////////////////////////////////////////////////////////
typedef struct NGramLearningCache
{
	N2GramLearningBlk st2Gram[MAX_NGRAM_LEARNING];
	N3GramLearningBlk st3Gram[MAX_NGRAM_LEARNING];
	N4GramLearningBlk st4Gram[MAX_NGRAM_LEARNING];
	
	USHORT count2; // number of words in cache
	USHORT countSinceSorted2; // number of updates been done to cache since last sorting.
	USHORT lowestPrefIdx2;

	USHORT count3; // number of words in cache
	USHORT countSinceSorted3; // number of updates been done to cache since last sorting.
	USHORT countSinceLastSaved3; // number of updats since last time cache was saved out to cachefile.
	USHORT lowestPrefIdx3;

	USHORT count4; // number of words in cache
	USHORT countSinceSorted4; // number of updates been done to cache since last sorting.
	USHORT countSinceLastSaved4; // number of updats since last time cache was saved out to cachefile.
	USHORT lowestPrefIdx4;

	void Add2Learning(CompactNode* node[N2Gram], MYWCHAR word[MAX_TGRAPH_HISTORY_NUM][MAX_WORD_LEN]);
	void Add3Learning(CompactNode* node[N3Gram], MYWCHAR word[MAX_TGRAPH_HISTORY_NUM][MAX_WORD_LEN], USHORT prefFromFile, bool bBulkLoading);
	void Add4Learning(CompactNode* node[N4Gram], MYWCHAR word[MAX_TGRAPH_HISTORY_NUM][MAX_WORD_LEN], USHORT prefFromFile, bool bBulkLoading);

	bool IsEmpty(int i, int type);
	
	bool IsSameNodes2(int i, CompactNode* node[N2Gram]);
	void SetNodes2(int i, CompactNode* node[N2Gram]);

	bool IsSameNodes3(int i, CompactNode* node[N3Gram]);
	void SetNodes3(int i, CompactNode* node[N3Gram]);

	bool IsSameNodes4(int i, CompactNode* node[N4Gram]);
	void SetNodes4(int i, CompactNode* node[N4Gram]);

	void CopyUpperCase(MYWCHAR word[MAX_TGRAPH_HISTORY_NUM][MAX_WORD_LEN], int type, int index);
	void SetFirstUpperCase(int i, int k, bool bUpper, int type);
	void Reset(int i, int type);
	
	static int N3GramLearningCompare(const void *a, const void *b);
	static int N4GramLearningCompare(const void *a, const void *b);
	static int N2GramLearningCompare(const void *a, const void *b);

	char CacheFilename3[1024];
	char CacheFilename4[1024];

	void SaveOutCachedGramLearning(int type, bool bBulkLoading);
	bool LoadGramLearningCacheFromFile(const char*filename, int type);

	void AddLearningFromFile(MYWCHAR* inputLine, USHORT learnPref);

	NGramLearningCache()
	{
		memset(this, 0, sizeof(NGramLearningCache));
	}

}NGramLearningCache;

///////////////////////////////////////////////////////////////////////////////////////
typedef struct N2GramLearningNoDict
{
	N2GramLearningNoDictBlk st2GramNoDict[MAX_NGRAM_LEARNING];

	USHORT NoDictCount2; // number of words in cache
	USHORT NoDictCountSinceSorted2; // number of updates been done to cache since last sorting.
	USHORT NoDictLowestPrefIdx2;

	bool IsEmptyNoDict(int i);
	void SortNoDict2();

	bool IsSameNodes2NoDict(int i, MYWCHAR arrWord[N2Gram][MAX_WORD_LEN]);
	void SetNodes2NoDict(int i, MYWCHAR arrWord[N2Gram][MAX_WORD_LEN]);
	
	void Add2LearningNoDict(MYWCHAR word[MAX_TGRAPH_HISTORY_NUM][MAX_WORD_LEN]);
	void ResetNoDict(int i);

	N2GramLearningNoDict()
	{
		memset(this, 0, sizeof(N2GramLearningNoDict));
	}

}N2GramLearningNoDict;

///////////////////////////////////////////////////////////////////////////////////////
struct PhraseWords
{
	MYWCHAR		ar1Gram[MAX_PHRASE_ALLOWED][MAX_WORD_LEN];
	MYWCHAR		ar2Gram[MAX_PHRASE_ALLOWED][MAX_WORD_LEN];
	MYWCHAR		ar3Gram[MAX_PHRASE_ALLOWED][MAX_WORD_LEN];
	MYWCHAR		ar4Gram[MAX_PHRASE_ALLOWED][MAX_WORD_LEN];
	USHORT		phrasePref[MAX_PHRASE_ALLOWED];

	void ResetThis(int k)
	{
		phrasePref[k] = 0;
		memset(ar1Gram[k], 0, sizeof(ar1Gram[k]));
		memset(ar2Gram[k], 0, sizeof(ar2Gram[k]));
		memset(ar3Gram[k], 0, sizeof(ar3Gram[k]));
		memset(ar4Gram[k], 0, sizeof(ar4Gram[k]));

	}

	PhraseWords()
	{
		memset(this, 0, sizeof(PhraseWords));
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
class NGramLearning
{
public:
	NGramLearning();
	~NGramLearning();

	void Learn2Gram(MYWCHAR	word[MAX_TGRAPH_HISTORY_NUM][MAX_WORD_LEN]);
	void Learn3Gram(MYWCHAR	word[MAX_TGRAPH_HISTORY_NUM][MAX_WORD_LEN], bool bulk = false);
	void Learn4Gram(MYWCHAR	word[MAX_TGRAPH_HISTORY_NUM][MAX_WORD_LEN], bool bulk = false);

	int GetNGramLearningAlphabetIndex(CompactNode* node, MYWCHAR* word);

	int Find2LearningWord(MYWCHAR secondWord[MAX_TGRAPH_SIZE][MAX_WORD_LEN], USHORT learnedPref[MAX_TGRAPH_SIZE],
		Dictionary* dict, CompactNode* p2GramStartNode, MYWCHAR* cmpWord, MYWCHAR* N2StartWord, bool bIgnorePref);

	int Find2LearningNoDicWord(MYWCHAR* N2StartWord, MYWCHAR secondWord[MAX_TGRAPH_SIZE][MAX_WORD_LEN], 
					USHORT learnedPref[MAX_TGRAPH_SIZE], MYWCHAR* cmpWord, bool bIgnorePref);

	int Find3LearningWord(MYWCHAR thirdWord[MAX_TGRAPH_SIZE][MAX_WORD_LEN], USHORT learnedPref[MAX_TGRAPH_SIZE],
		Dictionary* dict, CompactNode* p3GramStartNode, CompactNode* p2GramStartNode);
	
	int Find4LearningWord(MYWCHAR fourthWord[MAX_TGRAPH_SIZE][MAX_WORD_LEN], USHORT learnedPref[MAX_TGRAPH_SIZE],
		Dictionary* dict, CompactNode* p4GramStartNode, CompactNode* p3GramStartNode, CompactNode* p2GramStartNode);


	int Find3LearningPhrase(MYWCHAR* newInput, CompactNode* startNode, Dictionary* dict, PhraseWords* stWords);
	int Find4LearningPhrase(MYWCHAR* newInput, CompactNode* startNode, Dictionary* dict, PhraseWords* stWords);
	void addStartWordToCache(CompactNode *endnode, USHORT pref);
	inline StartWordCache* GetStartWordCache() { return &mStartWordCache; } 

	void reset();
private:
	CompactNode* CheckWordForCases(MYWCHAR* word, int* nodeCount);

	void Add2LearningToCache(CompactNode* node[N2Gram], MYWCHAR word[MAX_TGRAPH_HISTORY_NUM][MAX_WORD_LEN]);
	void Add2LearningNoDictToCache(MYWCHAR word[MAX_TGRAPH_HISTORY_NUM][MAX_WORD_LEN]);

	void Add3LearningToCache(CompactNode* node[N3Gram], MYWCHAR word[MAX_TGRAPH_HISTORY_NUM][MAX_WORD_LEN], bool bulk);
	void Add4LearningToCache(CompactNode* node[N4Gram], MYWCHAR word[MAX_TGRAPH_HISTORY_NUM][MAX_WORD_LEN], bool bulk);
	bool IsDuplicateWord(MYWCHAR arr[MAX_TGRAPH_SIZE][MAX_WORD_LEN], MYWCHAR* compWord);

	StartWordCache mStartWordCache;
	N2GramLearningNoDict		m_2CacheNoDict[MAX_ALPHABET_SIZE];

	NGramLearningCache		m_2Cache[MAX_ALPHABET_SIZE];
	NGramLearningCache		m_3Cache[MAX_ALPHABET_SIZE];
	NGramLearningCache		m_4Cache[MAX_ALPHABET_SIZE];
};

#endif