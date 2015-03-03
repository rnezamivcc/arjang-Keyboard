#ifndef _TGRAPH_H_
#define _TGRAPH_H_
#include "wltypes.h"
#include "dictmanager.h"
class Dictionary;
struct CompactNode;
struct NGramHistory;

#define TGRAPH_ADD_PREFERENCE			5
#define NO_DICT_INDICATOR				"NoDictIndicator"

typedef struct TGraphBlk
{
	UINT		NGram;
	UINT		casPref;
	UINT		LearnedPref;
	BYTE		nUpperCase;

	CompactNode* pNode;

	MYWCHAR* GetTGraphWord();
	void SetTGraphNode(MYWCHAR *word);

	TGraphBlk()
	{
		memset(this, 0, sizeof(TGraphBlk));
	}

}TGraphBlk;

typedef struct TGraphProps
{
	TGraphBlk		stTGraphBlk[MAX_TGRAPH_SIZE];	

	CompactNode *p2GramStartNode;
	CompactNode *p3GramStartNode;
	CompactNode	*p4GramStartNode;

	MYWCHAR		N2StartWord[MAX_WORD_LEN];
	MYWCHAR		N3StartWord[MAX_WORD_LEN];
	MYWCHAR		N4StartWord[MAX_WORD_LEN];

	MYWCHAR SavedLearnedWord[MAX_TGRAPH_SIZE][MAX_WORD_LEN];

	int totalCount;
	void ResetTGraphBlk();
	void SortRest();
	bool IsNGramProcessed();
	bool IsOnly2Grams();
	bool IsLearnedNGramProcessed();
	void ResetProps() {memset(this, 0, sizeof(TGraphProps));}

	TGraphProps()
	{
		ResetProps();
	}
} TGraphProps;


class TGraph
{
public:
	TGraph(Dictionary *topdict=NULL);
	~TGraph() {}
	void set(MYWCHAR* curHistory, MYWCHAR* curWord, NGramHistory* hist);
	void Process(MYWCHAR* curLetters, MultiLetterAdvanceAr *arr=NULL);
	void ProcessNouns(MYWCHAR* curWord);
	void reset();
	void UpdateAutoCorrect(MYWCHAR* input, MultiLetterAdvanceAr* pArr);

private:
	
	int Process2Gram();
	void Process3Gram();
	void Process4Gram();
	void ProcessAPOSTROPHE();
	void Update();
	int SetLearningTGraph(MYWCHAR learnedWord[MAX_TGRAPH_SIZE][MAX_WORD_LEN], USHORT learnedPref[MAX_TGRAPH_SIZE], int nGram, int start,  USHORT uPref);
	void ChangeI(MYWCHAR* curHistory, int len);

	int SetSavedWords(MYWCHAR word[MAX_WORD_LEN],int index);
	int SetNGramHistoryWord(MYWCHAR* pHistory, NGramHistory* stHistory, int arrIndex, int length, int nSpaceIndex);

	bool CheckLetter(MYWCHAR *word);
	//void SortTGraph(UINT value[MAX_TGRAPH_SIZE]);
	void MergeIntoPGraph();
	int Set3GramWord(MYWCHAR thirdWord[MAX_TGRAPH_SIZE][MAX_WORD_LEN], USHORT learnedPref[MAX_TGRAPH_SIZE], USHORT *uPref);
	int Set4GramWord(MYWCHAR fourthWord[MAX_TGRAPH_SIZE][MAX_WORD_LEN], USHORT learnedPref[MAX_TGRAPH_SIZE], USHORT *uPref);

	bool IsNodeProperNoun(CompactNode* node);
	MYWCHAR* GetApostropheWord(MYWCHAR* word);
	void SetNounPredictionArr(bool plural);
	int GetDuplicated(MultiLetterAdvanceAr* arr, MYWCHAR* word);

	TGraphProps		m_TProps;
	MYWCHAR*	m_curLetters;
	USHORT		m_basePref;
	Dictionary* m_dict;
	MultiLetterAdvanceAr		m_PGraphArr;
	NGramHistory*				m_TGraphHistory;
};

#endif