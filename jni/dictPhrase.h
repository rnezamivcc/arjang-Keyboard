#ifndef _DICTPHRASE_H_
#define _DICTPHRASE_H_
#include "wltypes.h"
#include "utility.h"
#include "compactstore.h"
#include "dictionary.h"
#include "nGramHistory.h"
#include "dictmanager.h"
#include "nGramLearning.h"

struct CompactNode;

/////////////////////////////////////////////////////////////////////////////////////////
class ProcessPhrase
{
public:
	ProcessPhrase();
	~ProcessPhrase();
	bool InitProcessPhrase(Dictionary* dict, NGramHistory* history, NGramLearning* learning);
	void ProcessNormalPrediction(PhraseAr* arr, MYWCHAR *newInput, CompactNode*firstEndnode, MYWCHAR* nextLetter, int *nextStart, MYWCHAR* cmpWord);
	bool ProcessLearnedPhrase(PhraseAr* arr, MYWCHAR* newInput,  MYWCHAR* nextLetter, CompactNode* startNode, int *nextStart, MYWCHAR* cmpWord);
	bool ProcessP2P(PhraseAr* arr, MYWCHAR *inputPhrase, int* nextStart);

private:
	bool SetPhrasePredictions(MYWCHAR* inputWord, MYWCHAR* nextLetter, PhraseWords* stWords, PhraseAr* arr, int *nextStart,  MYWCHAR* cmpWord);
	MYWCHAR* GetPhraseReturnString(MYWCHAR *first, MYWCHAR*second, 
			MYWCHAR* third, MYWCHAR *fourth, PhraseAr* arr, int * wordCnt, bool bP2P);

	bool SetPredictionsForP2P(PhraseWords* stWords, PhraseAr* arr, int* nextStart, MYWCHAR* inputPhrase, bool bEndSpace);
	int GetP2PWords(PhraseWords* stWords, MYWCHAR* input1, MYWCHAR* input2, MYWCHAR* lastWord, int count, bool bEndSpace);
	bool IsDuplicatedPhrase(MYWCHAR *returnString, PhraseAr* arr);
	void AttachToString(MYWCHAR* returnStr, MYWCHAR* str, int *count);

	Dictionary* m_dict;
	NGramHistory* m_history;
	NGramLearning* m_learning;
};
#endif