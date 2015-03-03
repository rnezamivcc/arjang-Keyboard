// This is the main DLL file.
#include "StdAfx.h"
#include "PredictionEngine.h"
#include "userWordCache.h"
#include "utility.h"

//extern "C" {

static CDictManager *sDictManager = NULL;
bool PredictionEngine::setDictManager()
{
	if (!sDictManager)
	{
		sDictManager = new CDictManager();
		if (!sDictManager->Create(NULL, NULL))
		{
			printf("!!ERROR!! setDictManager:: failed initializaing! now it's just a dumb keyboard!\n");
			delete sDictManager;
			sDictManager = NULL;
		}
	}
	return sDictManager != NULL;
}

bool PredictionEngine::Initialize()
{
	return setDictManager();
}

bool PredictionEngine::ResetDictionaryConfiguration()
{
	return sDictManager && sDictManager->resetConfiguration();
}

void PredictionEngine::Reset(bool cleanHistory)
{
	if (sDictManager)
	{
		if(cleanHistory)
			sDictManager->fullReset();
		else
		    sDictManager->reset();
	}
}

void PredictionEngine::BackspaceLetter()
{ 	   
	MYWCHAR rootWord[MAX_WORD_LEN];
	if(sDictManager)
		sDictManager->backspaceLetter(rootWord);
}

bool PredictionEngine::IsChunk(MYWCHAR *word)
{
	return sDictManager && sDictManager->isChunk(word);
}

bool PredictionEngine::AdvanceLetter(MYWCHAR  letter, bool startSentence)
{
   if (sDictManager) 
   {
		callLog("inside AdvanceLetter: sTestDictMgr");
		MYWCHAR *printPartW = NULL;
		bool bRet = sDictManager->advanceLetter(letter, &printPartW, startSentence);
		BOOL bAdded = sDictManager->CurrentWordCanbeAdded();
		return bRet;
	}
	return false;
}

bool PredictionEngine::AdvanceWord(MYWCHAR*  word, bool complete)
{
   if (sDictManager) 
   {
		MYWCHAR *printPartW = NULL;
		MultiLetterAdvanceAr *nextwords;
		return sDictManager->advanceWord(word, &printPartW, nextwords, complete);;
   }
   return 0;
}

MYWCHAR* PredictionEngine::NextLetters(int *posArray)
{
   if (sDictManager) 
   {
		return sDictManager->nextLetters(&posArray);
   }
   return NULL;

}

int PredictionEngine::GetDictPriority(int dictIdx)
{
	if (sDictManager)
	{
		DictionaryConfigurationEntry *dcen = sDictManager->GetOrderedDict(dictIdx);
		if(dcen!= NULL)
			return dcen->priority;
	}
	return -1;
}

int PredictionEngine::GetDictVersion(int dictIdx)
{
	if (sDictManager)
	{
		return sDictManager->getDictVersionNumber(dictIdx);
	}
	return -1;
}

int PredictionEngine::GetDictBuild(int dictIdx)
{
	if (sDictManager)
	{
		return sDictManager->getDictBuildNumber(dictIdx);
	}

	return -1;
}

bool PredictionEngine::SetDictSetting(int langIdx, int priority, bool enabled)
{
	if (sDictManager)
	{
		bool res = sDictManager->SetOrderedDict(langIdx, priority, enabled);
		if(res)
		{
			return true;
		}
	}
	return false;

}

bool PredictionEngine::GetDictInfo(int dictIdx, DictInfo info)
{
   printf("Inside GetDictInfo for idx=%d: ", dictIdx);
	if (sDictManager)
	{
		DictionaryConfigurationEntry *dcen = sDictManager->GetOrderedDict(dictIdx);
		if(dcen == NULL)
		{
			printf("--reached end of dictionary list at index=%d\n", dictIdx);
			return false;
		}
		const char* name = Dictionary::GetDictName(dcen->langIdx);
		MYWCHAR *wname = toW(name);
		mywcscpy(info.name, wname);
		info.langIdx = dcen->langIdx;
		info.priority = dcen->priority;
	}

	return true;

}

MYWCHAR *PredictionEngine::GetRootText()
{
    callLog("inside GetRootText");
	if (sDictManager)
	{
		return sDictManager->gatherRootEnding();
	}
	return NULL;

}

MYWCHAR *PredictionEngine::UndoLetterOrWord()
{
    callLog("inside UndoLetterOrWord");
	if (sDictManager) 
	{
		return sDictManager->undoLetterOrWord();
	}
	return NULL;
}

//DECLDIR int AutoAddedWords(AddWordsInfo info);
int PredictionEngine::ListWords(int dictId, MYWCHAR **words)
{
   callLog("inside ListWords");
	int nPosWords = 0;
	if (sDictManager) 
	{
		*words = sDictManager->getWordList(dictId, &nPosWords);
	}
	return nPosWords;
}

int PredictionEngine::NextWords(MYWCHAR ***words)
{
    callLog("inside NextWords");
	int nPosWords = 0;
	MYWCHAR rootWords[MAX_WORD_LEN];
	if (sDictManager) 
		*words = sDictManager->GetNextWords(&nPosWords, rootWords);
	else
		*words = NULL;
	return nPosWords;
}

bool PredictionEngine::currentWordCanBeAdded()
{
    callLog("inside currentWordCanBeAdded");
	if (sDictManager)
		return sDictManager->CurrentWordCanbeAdded();
	
	return false;
}

bool PredictionEngine::AddWord(MYWCHAR *wordpart)
{
    callLog("inside AddWord");
	int dictIdx = 0;
	return sDictManager && sDictManager->addWord(wordpart, LOW_PREFERENCE, &dictIdx);
}

bool PredictionEngine::DeleteWord(MYWCHAR *word)
{
    callLog("inside DeleteWord");
	return sDictManager && sDictManager->deleteWord(word);
}

void PredictionEngine::SetAutoLearn(bool bAutoLearn)
{
    callLog("inside SetAutoLearn");
	if(sDictManager)
		sDictManager->setAutoLearn(bAutoLearn);
}

//}
