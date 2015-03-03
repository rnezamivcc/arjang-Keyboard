
#ifndef NGRAMHISTORY_H
#define NGRAMHISTORY_H

#include "wltypes.h"
#include "dictrun.h"

struct CompactNode;

/////////////////////////////////////////////////////////////////////////////
typedef struct NGramHistory 
{
	MYWCHAR		word[MAX_TGRAPH_HISTORY_NUM][MAX_WORD_LEN];

	CompactNode* historyNode[MAX_TGRAPH_HISTORY_NUM];
	int			newSpaceIndex[MAX_TGRAPH_HISTORY_NUM];

	MYWCHAR		NewCurrentHistory[MAX_PHRASE_LEN];
	MYWCHAR		HistoryForPhrase[MAX_PHRASE_LEN];
	MYWCHAR		BackSpaceHistory[MAX_PHRASE_LEN];

	MYWCHAR		PhraseP2P[MAX_PHRASE_ALLOWED][MAX_PHRASE_LEN];

	bool bBackSpace;
	bool bSpace;
	bool bChangedCurrentHistory;
	bool bStartedUpperCaseLetter;
	bool bStartedUpperCaseWord;
	bool bIsThisEmail;
	bool bEmailForAdvanceWord;
	bool bStartingWordMode;
	bool bUpdateAfterNumbers;
	int nNumber;
	int nEraseLastWord;
	bool bDeleteFirstWord; //For phrase prediction. 
	bool bProcessedP2P;
	bool bAdvanceWordComplete;
	bool bReplaced;

	void NGramHistoryReset();
	void CheckNumber();
	bool CheckEmail(MYWCHAR *letters, int len=0);
	bool CheckUpdateTGraph();

	void SetStartingWordMode(bool backspace);
	void SetUpdateAfterNumbers();
	bool SetHistory(MYWCHAR *inputWord, bool backspace);

	bool PreChangeCurrentHistory(MYWCHAR* wordPart,bool complete,bool replace);
	void SetChangeCurrentHistory(MYWCHAR* wordPart, bool complete,bool replace);
	MYWCHAR* ChangeCurrentHistory();
	void TrimCurrentHistory(int nSpace);
	void CutCurrentBackSpaceHistory();
	MYWCHAR* GetWordToDelete(MYWCHAR* wordPart);
	MYWCHAR* GetLearnStartingWord();
	MYWCHAR* CheckConvertedEmail(MYWCHAR* wordPart);
	bool	IsPredictionAllowed(MYWCHAR* wordPart, MYWCHAR* lastWord);
	int GetWordFromHistory(MYWCHAR	word[MAX_TGRAPH_HISTORY_NUM][MAX_WORD_LEN]);
	bool IsZeroBackoutNum(MYWCHAR* wordInput);
	MYWCHAR* GetHistoryForPhrasePredictions(MYWCHAR* inputWord, MYWCHAR* nextLetter);
	void SetDeleteFirstWord(MYWCHAR* inputWord, MYWCHAR* nextLetter,  MYWCHAR* cmpWord);
	MYWCHAR* GetNewWordInputForPhrase(MYWCHAR* history);
	MYWCHAR* GetNumberPredictionWord(MYWCHAR* curLetter,  bool bInputSpace);
	MYWCHAR* GetLast4Words(MYWCHAR* inputPhrase);
	bool GetUpdateForStartWordsWithLetter();
	bool GetUpdateAfterNumbers(MYWCHAR* curLetter);
	void ResetForSpace();
	void ResetAfterAdvanceWord();
	void ResetHistoryPhrases();

	NGramHistory()
	{
		NGramHistoryReset();	
	}

} NGramHistory;


#endif