// PredictionEngine.h

#pragma once

#ifndef _DLL_TUTORIAL_H_
#define _DLL_TUTORIAL_H_
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "dictmanager.h"
#include "userWordCache.h"
#include "dictionary.h"


#if defined MYDLL_EXPORT
#define DECLDIR __declspec(dllexport)
#else
#define DECLDIR __declspec(dllimport)
#endif
 
extern "C"
{
class DECLDIR PredictionEngine{
public:
struct DictInfo 
{
	BYTE priority;
	BYTE langIdx;
	MYWCHAR name[100];
};

	 bool setDictManager();
	 bool Initialize();
	 bool ResetDictionaryConfiguration();
	 void Reset(bool clearHistory);
	 void BackspaceLetter();
	 bool IsChunk(MYWCHAR *word);
	 bool AdvanceLetter(MYWCHAR  letter, bool startSentence=false);
	 bool AdvanceWord(MYWCHAR*  word, bool complete);
	 MYWCHAR* NextLetters(int *posArray);
	 int GetDictPriority(int dictIdx);
	 int GetDictVersion(int dictIdx);
	 int GetDictBuild(int dictIdx);
	 bool SetDictSetting(int langIdx, int priority, bool enabled);
	 bool GetDictInfo(int dictIdx, DictInfo info);
	 MYWCHAR *GetRootText();
	 MYWCHAR *UndoLetterOrWord();
//	 int AutoAddedWords(AddWordsInfo info);
	 int ListWords(int dictId, MYWCHAR **words);
	 int NextWords(MYWCHAR ***words);
	 bool currentWordCanBeAdded();
	 bool AddWord(MYWCHAR *wordpart);
	 bool DeleteWord(MYWCHAR *word);
	 void SetAutoLearn(bool autolearn);
};
 
}

#endif
