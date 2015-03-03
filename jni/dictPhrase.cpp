// <copyright file="nGramLearning.cpp" company="WordLogic Corporation">
// Copyright (c) 2000, 2014 All Right Reserved, http://www.wordlogic.com/
//
// THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
// KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// </copyright>
// <author>Minkyu Lee</author>
// <email>minkyulee17@yahoo.com</email>
// <date>2014-04-08</date>
// <summary>
//        </summary>

#include "StdAfx.h"
#include "dictPhrase.h"
#include "dictionary.h"

extern CDictManager *wpDictManager;

ProcessPhrase::ProcessPhrase()
{
}
ProcessPhrase::~ProcessPhrase()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
bool ProcessPhrase::InitProcessPhrase(Dictionary* dict, NGramHistory* history, NGramLearning* learning)
{
	m_dict = dict;
	m_history = history;
	m_learning = learning;

	if(!m_dict || !m_history || !m_learning)
		return false;

	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
void ProcessPhrase::ProcessNormalPrediction(PhraseAr* arr, MYWCHAR *newInput, CompactNode*firstEndnode, 
												  MYWCHAR* nextLetter, int* nextStart, MYWCHAR* cmpWord)
{
	if(!firstEndnode)
		return;

#ifdef TIMETEST
	clock_t start, end;
	start = clock();
#endif

	PhraseWords stWords;
	int MaxNGrams = wpDictManager->setNGramFrom2Grams(firstEndnode, NULL, NULL, N3Gram);
	WLBreakIf(MaxNGrams > MAX_FOUND_PHRASES, "!!ERROR! MaxNGrams > 255 for %s!!\n", toA(newInput));

	int index=0;
	for(int k=0; k < MaxNGrams;k++)
	{
		WLBreakIf(PhraseEngineR::gPhrases[k].isSet() == false, "!!ERROR! TGraph: Set3GramWord:gPhrases[%i] is not set!!\n", k);
		if(index >= MAX_PHRASE_ALLOWED)
			break;

		int phraseLen = PhraseEngineR::gPhrases[k].nGrams;
		if(phraseLen >=N3Gram)
		{
			MYWCHAR **words = PhraseEngineR::gPhrases[k].getStr(phraseLen);
			if(!words)
				return;
			//ShowInfo("Phrase:%s %s %s %s\n", toA(words[0]), toA(words[1]), toA(words[2]), toA(words[3]));
			if(!isEmptyStr(words[1]) && phraseLen >=N2Gram)
				mywcscpy(stWords.ar2Gram[index], words[1]);
			if(!isEmptyStr(words[2]) && phraseLen >=N3Gram)
				mywcscpy(stWords.ar3Gram[index], words[2]);
			if(!isEmptyStr(words[3]) && phraseLen >=N4Gram)
				mywcscpy(stWords.ar4Gram[index], words[3]);
			
			stWords.phrasePref[index] = PhraseEngineR::gPhrases[k].pref;
			index++;
		}
	}

	SetPhrasePredictions(newInput, nextLetter, &stWords, arr, nextStart, cmpWord);

#ifdef TIMETEST
    end = clock();
    double diff = Diffclock(end, start);
    ShowInfo("Time: ProcessNormalPrediction:%f", diff);
	
#endif
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ProcessPhrase::IsDuplicatedPhrase(MYWCHAR *returnString, PhraseAr* arr)
{
	for(int i=0; i < MAX_PHRASE_ALLOWED; i++)
	{
		if(isEmptyStr(arr->phrases[i]))
			break;

		if(mywcscmp(returnString, arr->phrases[i]) == 0)
			return true;
	}

	return false;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ProcessPhrase::SetPhrasePredictions(MYWCHAR* inputWord, MYWCHAR* nextLetter, PhraseWords* stWords, PhraseAr* arr, int* nextStart, MYWCHAR* cmpWord)
{
	m_history->SetDeleteFirstWord(inputWord, nextLetter, cmpWord);
	bool bReturn = false;
	MYWCHAR *first = NULL, *second = NULL, *third= NULL, *fourth = NULL;
	USHORT  basePref = wpDictManager->calcCascadingBasePreference(1, eOrdinary, 0);
	for(int i=0; i <MAX_PHRASE_ALLOWED;i++)
	{
		if(isEmptyStr(stWords->ar2Gram[i]))
			break;

		if(cmpWord && mywcsncmp(stWords->ar2Gram[i], cmpWord, mywcslen(cmpWord)) != 0)
			continue;

        if(isEmptyStr(cmpWord))
            first = inputWord;

		second = stWords->ar2Gram[i];
		third = stWords->ar3Gram[i];
		fourth = stWords->ar4Gram[i];
		
		int nCount =0;
		MYWCHAR *returnString = GetPhraseReturnString(first, second, third, fourth, arr, &nCount, false);

		if(!isEmptyStr(returnString) && !IsDuplicatedPhrase(returnString, arr) 
			&& nCount >= MAX_TGRAPH_HISTORY_NUM-2  && *nextStart < MAX_PHRASE_ALLOWED)
		{
			mywcscpy(arr->phrases[*nextStart],returnString);
			arr->prefs[*nextStart] = basePref+stWords->phrasePref[i];
			*nextStart = *nextStart+1;
			bReturn = true;
		}
	}

	return bReturn;
}
MYWCHAR* ProcessPhrase::GetPhraseReturnString(MYWCHAR *first, MYWCHAR*second, 
		MYWCHAR* third, MYWCHAR *fourth, PhraseAr* arr, int * wordCnt, bool bP2P)
{
	
	static	MYWCHAR returnString[MAX_PHRASE_LEN];
	memset(returnString,0,sizeof(returnString));

	int nCount =0;

	bool bGetFirstWord = false;
	if(!bP2P)
	{
		if(!isEmptyStr(first) && !m_history->bDeleteFirstWord)
			bGetFirstWord = true;
	}
	else
	{
		if(!isEmptyStr(first))
			bGetFirstWord = true;
	}

	if(bGetFirstWord)
		AttachToString(returnString, first, &nCount);
	
	if(!isEmptyStr(second))
		AttachToString(returnString, second, &nCount);
	
	if(!isEmptyStr(third))
		AttachToString(returnString, third, &nCount);
		
	if(!isEmptyStr(fourth))
		AttachToString(returnString, fourth, &nCount);
		
	bool bExist = IsDuplicatedPhrase(returnString, arr);

	if(bExist)
		return NULL;

	*wordCnt = nCount;
	return returnString;
}

///////////////////////////////////////////////////////////////////////////////////////////////
void ProcessPhrase::AttachToString(MYWCHAR* returnStr, MYWCHAR* str, int *count)
{
	mywcscat(returnStr, str);
	AddEndSpace(returnStr);
	*count = *count+1;
}
///////////////////////////////////////////////////////////////////////////////////////////////
bool ProcessPhrase::ProcessP2P(PhraseAr* arr, MYWCHAR *inputPhrase, int* nextStart)
{

	//ShowInfo("MK ProcessP2P1:(%s)\n",toA(m_history->NewCurrentHistory));
	//ShowInfo("MK ProcessP2P2:(%s)\n",toA(m_history->HistoryForPhrase));
	//ShowInfo("MK ProcessP2P3:(%s)\n",toA(m_history->BackSpaceHistory));
	//ShowInfo("MK ProcessP2P inputPhrase:(%s)\n",toA(inputPhrase));

#ifdef TIMETEST
	clock_t start, end;
	start = clock();
#endif

	bool bEndSpace = HasEndingSpace(inputPhrase);
	trimEndingSpaces(inputPhrase);
	
	memset(m_history->PhraseP2P, 0, sizeof(m_history->PhraseP2P));
	int nSP = GetSpaceCount(inputPhrase);
	if(nSP == 0)
		return false;

	//"what if there was s"-->Need one more spot for current letters.
	const int maxSize = MAX_TGRAPH_HISTORY_NUM+1;
	MYWCHAR newInputWord[maxSize][MAX_WORD_LEN];
	memset(newInputWord, 0 , sizeof(newInputWord));
	
	int index =0;
	int pos = 0;
	for(int i=0; i < MAX_PHRASE_LEN*2 && inputPhrase[i]; i++)
	{
		if(inputPhrase[i] == SP)
		{
			index++;
			pos =0;
			continue;
		}
		newInputWord[index][pos++] = inputPhrase[i];

	}
	int LastWordInd = maxSize;
	for(int i=maxSize-1; i >= 0; i--)
	{
		if(!isEmptyStr(newInputWord[i]) && LastWordInd == maxSize)
		{
			LastWordInd = i;
		}

		if(!isEmptyStr(newInputWord[i]))
		{
			if(newInputWord[i][0] == 'I' && (newInputWord[i][1] == NUL || newInputWord[i][1] == APOSTROPHE ))
				continue;
			MakeLowerCase(newInputWord[i]);
		}
	}
	//ShowInfo("MK ProcessP2P newInputWord:(%s) (%s) (%s) (%s), lastWordInd(%d)\n",toA(newInputWord[0]), toA(newInputWord[1]), toA(newInputWord[2]), toA(newInputWord[3]), LastWordInd);

	if(!bEndSpace)
		LastWordInd--;

	if(LastWordInd <= 0)
		return false;

	MYWCHAR* lastWord = newInputWord[LastWordInd];
	PhraseWords stP2PWords;

	int count =0;
	int newIndex = LastWordInd;
	for(int i=0; i <maxSize; i++)
	{
		if(count >= MAX_PHRASE_ALLOWED)
			break;

		if(newIndex >= 0)
			count = GetP2PWords(&stP2PWords, newInputWord[newIndex-1], newInputWord[newIndex], lastWord, count, bEndSpace);
	
		newIndex--;
	}

	if(SetPredictionsForP2P(&stP2PWords, arr, nextStart, inputPhrase, bEndSpace))
	{
#ifdef TIMETEST
		end = clock();
		double diff = Diffclock(end, start);
		ShowInfo("Time: ProcessP2P:%f", diff);
#endif

		return true;
	}
	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ProcessPhrase::SetPredictionsForP2P(PhraseWords* stWords, PhraseAr* arr, int* nextStart, MYWCHAR* inputPhrase, 
										 bool bEndSpace)
{
	MYWCHAR * first = NULL, *second = NULL, *third= NULL, *fourth = NULL;
	USHORT  basePref =wpDictManager->calcCascadingBasePreference(1, eOrdinary, 0);
	bool bReturn  = false;
	int index =0;
	MYWCHAR* lastWordOfInput = GetLastWordFromPhrase(inputPhrase);

	for(int i=0; i <MAX_PHRASE_ALLOWED;i++)
	{
		if(isEmptyStr(stWords->ar1Gram[i]))
			break;

		if(bEndSpace)
		{
			if(!isEmptyStr(lastWordOfInput) && mywcscmp(lastWordOfInput, stWords->ar1Gram[i]) == 0)
				first = NULL;
			else
				first = stWords->ar1Gram[i];

		}
		else //"contact me a"-->"at your convenience" //"contact me c"-->find next phrase starting with "c".
		{
			int len = mywcslen(lastWordOfInput);
			if(!isEmptyStr(lastWordOfInput) && mywcsncmp(lastWordOfInput, stWords->ar1Gram[i], len) != 0)
				continue;

			first = stWords->ar1Gram[i];
		}
		second = stWords->ar2Gram[i];
		third = stWords->ar3Gram[i];
		fourth = stWords->ar4Gram[i];

		int nCount =0;
		MYWCHAR *returnString = GetPhraseReturnString(first, second, third, fourth, arr, &nCount, true);
		if(!isEmptyStr(returnString) && !IsDuplicatedPhrase(returnString, arr) 
			&& nCount >= MAX_TGRAPH_HISTORY_NUM-2  && *nextStart < MAX_PHRASE_ALLOWED)
		{
			mywcscpy(arr->phrases[*nextStart],returnString);
			arr->prefs[*nextStart] = basePref+stWords->phrasePref[i];
			*nextStart = *nextStart+1;
			bReturn = true;

			mywcscpy(m_history->PhraseP2P[index], returnString);
			//ShowInfo("MK ProcessPhrasePrediction selectP2P returnString:(%s)\n", toA(m_history->PhraseP2P[index]));
			index++;
		}
	}

	return bReturn;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int ProcessPhrase::GetP2PWords(PhraseWords* stWords, MYWCHAR* input1, MYWCHAR* input2, MYWCHAR* lastWord, int count, bool bEndSpace)
{
	if(isEmptyStr(input1) || isEmptyStr(input2))
	{
		return count;
	}

	int len =0;
	int	MaxNGrams = wpDictManager->setNGramFrom2Grams(input1, input2);
	//ShowInfo("MK ProcessP2P inputs:%s %s\n", toA(input1), toA(input2));
	//ShowInfo("MK ProcessP2P lastWord:%s\n", toA(lastWord));
	//ShowInfo("MK ProcessP2P nextLetter:%s\n", toA(nextLetter));
	//ShowInfo("MK ProcessP2P phraseCount:%d\n", phraseCount);

	for(int i=0; i<MaxNGrams; i++)
	{
		PhraseNode *phNode =  &PhraseEngineR::gPhrases[i];
		MYWCHAR** phrase = phNode->getStr(len);
		if(!phrase)
			return count;
		//ShowInfo("Phrase:len(%d): %s %s %s %s\n", len, toA(phrase[0]), toA(phrase[1]), toA(phrase[2]), toA(phrase[3]));
		MYWCHAR* phraseLast = phrase[len-1];
		bool bSkip = false;
		if(mywcscmp(phraseLast, lastWord) != 0 && bEndSpace)
			bSkip = true;
	
		//This is only P2P phrase.
		//Case1: "make your dreams "
		//Case2: "make your dreams c"

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//**"contact me" is the prhase that can be the root for both P2P and normal phrase and it has to be checked if P2P phrase can be used depending on the cases.//
		//"contact me at" as normal phrase.																												             //
		//"contact me::at your convenience" as P2P.																										             //
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		//Case3: "contact me "
		//Case4: "contact me at "
		//Case5: "contact me at y"
		int nextPhCount = 0;
		PhraseNode *nextPhrase = phNode->getNextPhrase(nextPhCount++);
		while(nextPhrase && count < MAX_PHRASE_ALLOWED)
		{
			int nextlen =0;
			MYWCHAR** words = nextPhrase->getStr(nextlen);
			if(!words)
				return count;
			//ShowInfo("MK ProcessP2P NextPhrase:nextlen(%d): %s %s %s %s\n", nextlen, toA(words[0]), toA(words[1]), toA(words[2]), toA(words[3]));
			MYWCHAR* cmpWord = words[0];
			if(bSkip && mywcscmp(cmpWord, lastWord) != 0)
			{
				nextPhrase = phNode->getNextPhrase(nextPhCount++);
				continue;
			}

			if(!isEmptyStr(words[0]) && nextlen >= N1Gram)
				mywcscpy(stWords->ar1Gram[count], words[0]);
			if(!isEmptyStr(words[1]) && nextlen >= N2Gram)
				mywcscpy(stWords->ar2Gram[count], words[1]);
			if(!isEmptyStr(words[2]) && nextlen >= N3Gram)
				mywcscpy(stWords->ar3Gram[count], words[2]);
			if(!isEmptyStr(words[3]) && nextlen >= N4Gram)
				mywcscpy(stWords->ar4Gram[count], words[3]);
			
			stWords->phrasePref[count] = nextPhrase->pref;
			nextPhrase = phNode->getNextPhrase(nextPhCount++);
			count++;
			
		}
	}

	//ShowInfo("MK ProcessP2P inputs finish:%s %s\n", toA(input1), toA(input2));
	return count;
	
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ProcessPhrase::ProcessLearnedPhrase(PhraseAr* arr, MYWCHAR* newInput, MYWCHAR* nextLetter, CompactNode* startNode, int* nextStart, MYWCHAR* cmpWord)
{
	if(!m_dict || !m_learning || !newInput || !m_history)
		return false;

#ifdef TIMETEST
	clock_t start, end;
	start = clock();
#endif

	PhraseWords stWords;
	int count4 = m_learning->Find4LearningPhrase(newInput, startNode, m_dict, &stWords);
	int count3 = 0;

	if(count4 < MAX_PHRASE_ALLOWED)
		count3 = m_learning->Find3LearningPhrase(newInput, startNode, m_dict, &stWords);
	
	if(count4+count3 > 0)
	{
		if(SetPhrasePredictions(newInput, nextLetter, &stWords, arr, nextStart, cmpWord))
		{
#ifdef TIMETEST
			end = clock();
			double diff = Diffclock(end, start);
			ShowInfo("Time: ProcessLearnedPhrase:%f", diff);
#endif
			return true;
		}
	}
		
	return false;
}