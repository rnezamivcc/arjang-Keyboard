// <copyright file="dictconfig.cpp" company="WordLogic Corporation">
// Copyright (c) 2000, 2014 All Right Reserved, http://www.wordlogic.com/
//
// THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
// KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// </copyright>
// <author>Reza Nezami</author>
// <email>reza.nezami@gmail.com</email>
// <date>2014-03-10</date>
// <summary>Provides support for keeping ngram history while ngram is being developed by user's input. 
//          It provides various support to detect various context in current ngram under development.</summary>
//
#include "StdAfx.h"
#include "nGramHistory.h"
#include "utility.h"


////////////////////////////////////////////////////////////////////////////////////////////////
void NGramHistory::NGramHistoryReset()
{
	memset(this, 0, sizeof(NGramHistory));
}

///////////////////////////////////////////////////////////////
void NGramHistory::CheckNumber()
{
	MYWCHAR *temp = NewCurrentHistory;
	trimEndingSpaces(temp);

	int histLen = mywcslen(temp);
		
	if(histLen-1 >=0 && isNumberChar(temp[histLen-1]))
	{
		bUpdateAfterNumbers = true;
		MYWCHAR* p = GetLastWordFromPhrase(temp);
		MYWCHAR numChar[MAX_WORD_LEN];
		memset(numChar, 0, sizeof(numChar));
		int index =0;
		for(int i=0; i <MAX_WORD_LEN && p[i] != NUL;i++)
		{
			if(isNumberChar(p[i]))
			{
				numChar[index] = p[i];
				index++;
			}
		}
		if(isNumber(numChar))
			nNumber = atoi(toA(numChar));
	}
}

///////////////////////////////////////////////////////////////
bool  NGramHistory::CheckEmail(MYWCHAR *letters, int len)
{
	bool ret = false;
	if(len<=0)
		len = mywcslen(letters);
	while( len >=0 && letters[len] != AT)
		len--;
	if(len>=0 && letters[len] == AT)
		ret = true;

	bIsThisEmail = ret;

	return ret;
}

///////////////////////////////////////////////////////////////
bool  NGramHistory::CheckUpdateTGraph()
{
	//ShowInfo("MK CheckUpdateTGraph1:(%s)\n",toA(NewCurrentHistory));
	//ShowInfo("MK CheckUpdateTGraph2:(%s)\n",toA(HistoryForPhrase));
	int n=mywcslen(HistoryForPhrase);
	if(n == 0)
		return true;
	
	if(n-2 >=0)
	{	
		if(HasEndingSpace(HistoryForPhrase) && !isPunctuation(HistoryForPhrase[n-2]) )
			 return true;
	}

	return false;
}

///////////////////////////////////////////////////////////////
void NGramHistory::SetStartingWordMode(bool backspace)
{
	//ShowInfo("MK starting Word Mode:(%s)\n",toA(NewCurrentHistory));
	if(!backspace)
	{
		MYWCHAR temp[MAX_PHRASE_LEN];
		mywcscpy(temp, NewCurrentHistory);

		trimEndingSpaces(temp);
		int n = mywcslen(temp);
		bStartingWordMode = (n== 0 || (isPunctuation(temp[n-1]) && (temp[n-1] != COMMA)));
	}
	else
	{
		int n = mywcslen(NewCurrentHistory);
		if(NewCurrentHistory[n-1] == SP && n-2 >=0 && !isNumberChar(NewCurrentHistory[n-2]))
			bStartingWordMode = true;
		else
			bStartingWordMode = false;	
	}

	//ShowInfo("MK starting Word bStartingWordMode:(%d)\n",bStartingWordMode);
}

///////////////////////////////////////////////////////////////
void NGramHistory::SetUpdateAfterNumbers()
{
	//ShowInfo("MK SetUpdateAfterNumbers:(%s)\n",toA(NewCurrentHistory));

	MYWCHAR *lastWordPart = GetLastWordFromPhrase(NewCurrentHistory);

	//ShowInfo("MK SetUpdateAfterNumbers lastwordpart:(%s)\n",toA(lastWordPart));

	bUpdateAfterNumbers = isNumber(lastWordPart);
}

///////////////////////////////////////////////////////////////
bool NGramHistory::SetHistory(MYWCHAR *inputWord, bool backspace)
{
	if(!inputWord)
		return false;

	int startPos = -1;
	MYWCHAR phrase[MAX_PHRASE_LEN];
	if(inputWord[0] == SP)
	{
		memset(phrase, 0, sizeof(phrase));
		for(int i=0; i < MAX_PHRASE_LEN && inputWord[i]; i++)
		{
			if(inputWord[i] != SP)				
			{
				startPos = i;
				break;
			}
		}

		if(startPos != -1)
		{
			int index =0;
			for(int i=startPos; i < MAX_PHRASE_LEN && inputWord[i]; i++)
			{
				phrase[index++] = inputWord[i];
			}
		}
	}

	if(startPos != -1)
	{	
		mywcscpy(NewCurrentHistory, phrase);
		mywcscpy(HistoryForPhrase, phrase);

		if(backspace)
		{
			memset(BackSpaceHistory,0,sizeof(BackSpaceHistory));
			mywcscpy(BackSpaceHistory, phrase);
		}

		//ShowInfo("MK SetHistoryFromJNI:(%s)\n\n",toA(NewCurrentHistory));
		if(!backspace && HasEndingSpace(NewCurrentHistory) && !isEmptyStr(phrase))
		{
			//ShowInfo("MK SetHistoryFromJNI LearnMultiGramWord\n\n");
			return true;
		}

	}
	else
	{
		if(mywcslen(inputWord) > MAX_PHRASE_LEN)
		{
			memset(NewCurrentHistory,0,sizeof(NewCurrentHistory));
			memset(HistoryForPhrase,0,sizeof(HistoryForPhrase));
			memset(BackSpaceHistory,0,sizeof(BackSpaceHistory));
			return false;
		}

		mywcscpy(NewCurrentHistory, inputWord);
		mywcscpy(HistoryForPhrase, inputWord);

		if(backspace)
		{
			memset(BackSpaceHistory,0,sizeof(BackSpaceHistory));
			mywcscpy(BackSpaceHistory, inputWord);
		}

		//ShowInfo("MK SetHistoryFromJNI:(%s)\n\n",toA(NewCurrentHistory));
		if(!backspace && HasEndingSpace(NewCurrentHistory) && !isEmptyStr(inputWord))
		{
			//ShowInfo("MK SetHistoryFromJNI LearnMultiGramWord\n\n");
			return true;
		}

	}

	bBackSpace = backspace;



	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
bool NGramHistory::PreChangeCurrentHistory(MYWCHAR* wordPart,bool complete, bool replace)
{
	//ShowInfo("MK ChangeCurrentHistory1 history1:(%s)\n",toA(NewCurrentHistory));
	//ShowInfo("MK ChangeCurrentHistory1 history2:(%s)\n",toA(HistoryForPhrase));
	//ShowInfo("MK ChangeCurrentHistory1 history3:(%s)\n",toA(BackSpaceHistory));
	//ShowInfo("MK ChangeCurrentHistory1 wordPart(%s)\n", toA(wordPart));

	bChangedCurrentHistory = true;
	trimEndingSpaces(wordPart);

	MYWCHAR* lastPart= GetLastWordFromPhrase(NewCurrentHistory);

	bool bEmail = IsThisEmail(lastPart);

	bStartedUpperCaseLetter = isUpperCase(lastPart[0]);

	//MakeLowerCase(lastPart);
	if(mywcscmp(lastPart, wordPart) == 0)
	{
		//trimEndingSpaces(NewCurrentHistory, true);
		trimEndingSpaces(NewCurrentHistory);
		return true;
	}

	bool bHasEndSpace = HasEndingSpace(NewCurrentHistory);

	//DO NOT delete punctuation here.
	//trimEndingSpaces(NewCurrentHistory, true);
	trimEndingSpaces(NewCurrentHistory);

	//Case for backspacing: advanceWordComplete "Please". Then backspacing until "ple". Select "Please" again from the bar.
	//History should be "please"..not "ple please".
	bool bSpecialBackSpace = false;
	if(bHasEndSpace && !isEmptyStr(HistoryForPhrase) && !HasEndingSpace(HistoryForPhrase) )
		bSpecialBackSpace = true;
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int nlen = mywcslen(NewCurrentHistory);
	//CASE: prediction is "6th" and select "6th" by user.
	bool bNumberCase = false;
	if(isNumberChar(wordPart[0]))
	{
		MYWCHAR endHistChar = NewCurrentHistory[nlen-1];
		if(isNumberChar(endHistChar) && endHistChar == wordPart[0])
			bNumberCase = true;	
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	if((!bHasEndSpace || bSpecialBackSpace || bNumberCase) && !bEmail)
	{
		DeleteLastWordFromPhrase(NewCurrentHistory);
		mywcscat(NewCurrentHistory, wordPart);
		//ShowInfo("MK ChangeCurrentHistory1 preChange1(%s)\n", toA(NewCurrentHistory));
		return true;
	}

	//ShowInfo("MK ChangeCurrentHistory1 preChange2(%s)\n", toA(NewCurrentHistory));

	return false;
}

///////////////////////////////////////////////////////////////
void NGramHistory::SetChangeCurrentHistory(MYWCHAR* wordPart, bool complete, bool replace)
{
	int SpNum = GetSpaceCount(wordPart);
	if(SpNum >= MAX_TGRAPH_HISTORY_NUM)
	{
		memset(NewCurrentHistory,0,sizeof(NewCurrentHistory));
		mywcscpy(NewCurrentHistory,wordPart);
		//ShowInfo("MK ChangeCurrentHistory Final1:(%s)\n",toA(NewCurrentHistory));
		return;
	}

	bool bAddedWordPart = PreChangeCurrentHistory(wordPart,complete,replace);

	if(!isEmptyStr(NewCurrentHistory))
	{
		if(!IsThisEmail(NewCurrentHistory))
			AddEndSpace(NewCurrentHistory);		
	}
	
	if(!bAddedWordPart)
	{
		if(!HasEndingSpace(NewCurrentHistory))
		{
			AddEndSpace(NewCurrentHistory);		
		}
		mywcscat(NewCurrentHistory, wordPart);
		AddEndSpace(NewCurrentHistory);		
	}

	int nSP = GetSpaceCount(NewCurrentHistory);
	
	if(nSP >= MAX_TGRAPH_HISTORY_NUM)
	{
		//ShowInfo("MK trim history before1:(%s)\n",toA(NewCurrentHistory));
		TrimCurrentHistory(nSP+1);
	}

	memset(BackSpaceHistory,0,sizeof(BackSpaceHistory));
	mywcscpy(BackSpaceHistory,NewCurrentHistory);

	//ShowInfo("MK ChangeCurrentHistory Final2:(%s)\n",toA(NewCurrentHistory));
}

///////////////////////////////////////////////////////////////
void NGramHistory::TrimCurrentHistory(int nSpace)
{
	MYWCHAR temp[MAX_PHRASE_LEN];
	mywcscpy(temp,NewCurrentHistory);

	int nCutSpace = nSpace-MAX_TGRAPH_HISTORY_NUM;

	int nSP =0;
	for(int i=0; i < MAX_PHRASE_LEN && temp[i] != NUL; i++)
	{
		if(temp[i] == SP)
			nSP++;
		
		if(temp[i] == SP && nSP == nCutSpace)
		{
			temp[i] = ASTERISK;
			break;
		}

		temp[i] = ASTERISK;
	}

	memset(NewCurrentHistory,0,sizeof(NewCurrentHistory));
	int index =0;
	for(int i=0; i < MAX_PHRASE_LEN && temp[i] != NUL; i++)
	{
		if(temp[i] != ASTERISK)
		{
			NewCurrentHistory[index++] = temp[i];
		}
	}

	//ShowInfo("MK trim history after:(%s)\n",toA(NewCurrentHistory));
}

///////////////////////////////////////////////////////////////
void NGramHistory::CutCurrentBackSpaceHistory()
{
	trimEndingSpaces(BackSpaceHistory);

	MYWCHAR temp[MAX_PHRASE_LEN];
	mywcscpy(temp,BackSpaceHistory);

	int nSP =0;
	for(int i=0; i < MAX_PHRASE_LEN && temp[i] != NUL; i++)
	{
		if(temp[i] == SP)
			nSP++;
	
		if(temp[i] == SP && nSP == 1)
		{
			temp[i] = HTAB;
			break;
		}

		temp[i] = HTAB;
	}

	memset(BackSpaceHistory, 0, sizeof(BackSpaceHistory));
	int index =0;
	for(int i=0; i < MAX_PHRASE_LEN && temp[i] != NUL; i++)
	{
		if(temp[i] != HTAB)
			BackSpaceHistory[index++] = temp[i];	
	}
}

///////////////////////////////////////////////////////////////////////////////////////
MYWCHAR* NGramHistory::ChangeCurrentHistory()
{
	static MYWCHAR curHistory[MAX_PHRASE_LEN];

	if(bBackSpace)
	{
		//ShowInfo("MK curHistory history3:(%s)\n",toA(BackSpaceHistory));
		if(!bSpace)
		{
			DeleteLastWordFromPhrase(BackSpaceHistory);

			int nBack = mywcslen(BackSpaceHistory);
			if(BackSpaceHistory[nBack] != SP)
				BackSpaceHistory[nBack] = NUL;	
		}

		mywcscpy(curHistory, BackSpaceHistory);
		
		int nLen = mywcslen(curHistory);
		if(HasEndingSpace(curHistory) && curHistory[nLen-2]== SP)
			curHistory[nLen-1] = NUL;
		

		//ShowInfo("MK curHistory1:(%s), bSpace(%d)\n",toA(curHistory),bSpace);
		int nSP = GetSpaceCount(curHistory);

		bool bTrimHistory = true;
		if(!bSpace && nSP == MAX_TGRAPH_HISTORY_NUM-2)
		{
			MYWCHAR temp[MAX_PHRASE_LEN];
			mywcscpy(temp, BackSpaceHistory);

			trimEndingSpaces(temp);
			bTrimHistory = (GetSpaceCount(temp) != 1);
		}

		if((bTrimHistory && (nSP == MAX_TGRAPH_HISTORY_NUM && !bSpace)) ||
			(nSP >= MAX_TGRAPH_HISTORY_NUM-1 && bSpace && !HasEndingSpace(curHistory)))
		{
			CutCurrentBackSpaceHistory();	
			mywcscpy(curHistory, BackSpaceHistory);
			//ShowInfo("MK curHistory2:(%s)\n",toA(curHistory));
		}
		bBackSpace = false;
		bSpace = false;
	}
	else
	{		

		AddEndSpace(NewCurrentHistory);		
		
		mywcscpy(curHistory, NewCurrentHistory);
		//ShowInfo("MK curHistory3:(%s)\n",toA(curHistory));

		int n = mywcslen(curHistory);
		if(n >=2 && curHistory[n-2] == SP && curHistory[n-1] == SP)
			curHistory[n-1] = NUL;
		
		int nSP = GetSpaceCount(curHistory);
		if(nSP >= MAX_TGRAPH_HISTORY_NUM)
		{
			//ShowInfo("MK trim history before2:(%s)\n",toA(NewCurrentHistory));
			TrimCurrentHistory(nSP+1);
			mywcscpy(curHistory, NewCurrentHistory);
			//ShowInfo("MK curHistory4:(%s)\n",toA(curHistory));
		}
	}
	return curHistory;
}

///////////////////////////////////////////////////////////////////////////////////
MYWCHAR* NGramHistory::GetWordToDelete(MYWCHAR* wordPart)
{
	
	trimEndingSpaces(NewCurrentHistory);
	static MYWCHAR *wordToDelete = DeleteLastWordFromPhrase(NewCurrentHistory, true);

	AddEndSpace(NewCurrentHistory);

	mywcscat(NewCurrentHistory, wordPart);
	myReverseStr(wordToDelete);


	return wordToDelete;
}

////////////////////////////////////////////////////////////////////////////////////
MYWCHAR* NGramHistory::GetLearnStartingWord()
{
	MYWCHAR *temp = NewCurrentHistory;
	if(NewCurrentHistory[0] == SP)
		temp++;
	int n = mywcslen(temp);
	//ShowInfo("MK Learn Starting history2:(%s), length:%d\n",toA(temp), n);
	if(temp[n-1] == SP)
		n--;
			
	if(n > MAX_PHRASE_LEN)
		return NULL;
	
	int index = -1;
	for(int i = n-1; i >= 0; i--)
	{
		if(isPunctuation(temp[i]) && temp[i] != COMMA)
		{
			index = i;
			break;
		}
	}

	MYWCHAR phrase[MAX_PHRASE_LEN];
	memset(phrase,0,sizeof(phrase));

	if(temp[index+1] == SP && index != -1)
	{
		int wordInd = 0;
		for(int i = index+2; i < n; i++)
		{
			phrase[wordInd] = temp[i];
			wordInd++;
		}
	}

	if(GetSpaceCount(temp) == 0 && index == -1)
		mywcscpy(phrase, temp);
	

	static MYWCHAR inputWord[MAX_WORD_LEN];
	memset(inputWord,0,sizeof(inputWord));
	for(int i =0; i < MAX_WORD_LEN && word[i]!= (MYWCHAR *)NUL; i++)
	{
		if(phrase[i] == SP)
			break;
		
		inputWord[i] = phrase[i];
	}

	return inputWord;
}
/////////////////////////////////////////////////////////////////////////
MYWCHAR* NGramHistory::CheckConvertedEmail(MYWCHAR* wordPart)
{
	if(IsThisEmail2(wordPart))
	{
		MYWCHAR tpEmail[MAX_WORD_LEN];
		mywcscpy(tpEmail,wordPart);

		if(tpEmail[0] == AT)//@gmail.com or @yahoo.com
		{
			MYWCHAR convertedEmail[MAX_WORD_LEN];
			memset(convertedEmail,0,sizeof(convertedEmail));
			for(int i=1; i < MAX_WORD_LEN && tpEmail[i] != NUL; i++)
				convertedEmail[i-1] = tpEmail[i];
			
			mywcscpy(wordPart,convertedEmail);
		}
		bEmailForAdvanceWord = true;
	}
	return wordPart;
}
////////////////////////////////////////////////////////////////////////////////////
bool NGramHistory::IsPredictionAllowed(MYWCHAR* wordPart,MYWCHAR* lastWord)
{
	int nSpace = GetSpaceCount(wordPart);
	if(nSpace < MAX_TGRAPH_HISTORY_NUM-2)
	{
		int n = mywcslen(wordPart);
		bool bPunc = false;
		for(int i=n-1; i >=0; i--)
		{
			if(isPunctuation(wordPart[i]))
			{
				bPunc = true;	
				break;
			}
		}

		if(bPunc && !IsThisEmail(lastWord) && !IsThisEmail(wordPart))
			return false;
		
	}

	return true;
}
//////////////////////////////////////////////////////////////////////////////////////////
int NGramHistory::GetWordFromHistory(MYWCHAR word[MAX_TGRAPH_HISTORY_NUM][MAX_WORD_LEN])
{
	static MYWCHAR inputHistory[MAX_PHRASE_LEN];
	mywcscpy(inputHistory,NewCurrentHistory);
	//MYWCHAR*ppp = makeWord(L"me. Call right now. ");
	//mywcscpy(inputHistory,ppp);
	//ShowInfo("MK LearnMultiGramWord inputHistory:(%s)\n", toA(inputHistory));

	int nLen = mywcslen(inputHistory);

	int nSpace = GetSpaceCount(inputHistory);
	if(nSpace < MAX_TGRAPH_HISTORY_NUM-2 || nSpace > MAX_TGRAPH_HISTORY_NUM || !HasEndingSpace(inputHistory) )
		return 0;
	

	int puncInd = -1;
	for(int i=nLen-1; i >= 0;i--)
	{
		if(isPunctuation(inputHistory[i]))
		{
			puncInd = i;
			break;
		}
	}


	int start =0;
	if(puncInd != -1)
	{
		MYWCHAR		temp[MAX_PHRASE_LEN];
		mywcscpy(temp, inputHistory);
		memset(inputHistory,0,sizeof(inputHistory));
		int newIdx =0;
		for(int i=puncInd+1; i < MAX_PHRASE_LEN && temp[i] != NUL;i++)
		{	
			if(newIdx ==0 && temp[i] == SP)
				continue;
			
			inputHistory[newIdx] = temp[i];
			newIdx++;	
		}

		if(isEmptyStr(inputHistory))
		{
			mywcscpy(inputHistory, temp);
			for(int i=0; i < nLen; i++)
			{
				if(isPunctuation(inputHistory[i]) && inputHistory[i+1] == SP && inputHistory[i+2] != NUL)
				{
					start = i;
					break;
				}
			}
		}
		
	}

	int sp = 0;
	int nPos =0;
	for(int i=start;i < MAX_PHRASE_LEN && inputHistory[i];i++)
	{	
		if(inputHistory[i] == SP)
		{
			sp++;
			nPos=0;
		}
		else
		{
			if(!isPunctuation(inputHistory[i]))
			{
				word[sp][nPos] = inputHistory[i];
				nPos++;
			}
		}
	}
	return sp;
}
////////////////////////////////////////////////////////////////////////////////////////////
bool NGramHistory::IsZeroBackoutNum(MYWCHAR* wordInput)
{
	//ShowInfo("MK IsZeroBackoutNum history:(%s)\n",toA(NewCurrentHistory));
	//ShowInfo("MK IsZeroBackoutNum history2:(%s)\n",toA(HistoryForPhrase));
	//ShowInfo("MK IsZeroBackoutNum wordInput:(%s)\n",toA(wordInput));

	//For P2P, type--"contact me a", then select "at your convinience" from phrase bar. 
	if(!HasEndingSpace(HistoryForPhrase) && GetSpaceCount(wordInput) > 0)
	{
		MYWCHAR* last = GetLastWordFromPhrase(HistoryForPhrase);
		MYWCHAR firstPart[MAX_WORD_LEN];
		memset(firstPart, 0, sizeof(firstPart));
		int index = 0;
		for(int i =0; i < MAX_PHRASE_LEN && wordInput[i]; i++)
		{
			if(wordInput[i] == SP)
				break;

			firstPart[index++] = wordInput[i];

		}
		int len = mywcslen(last);
		if(!isEmptyStr(firstPart) && mywcsncmp(last, wordInput, len) == 0)
		{
			//ShowInfo("MK IsZeroBackoutNum true1:\n");
			return true;
		}
	}

	if(GetSpaceCount(wordInput) == 0 || HasEndingSpace(HistoryForPhrase) 
		|| isEmptyStr(HistoryForPhrase))
	{
		//ShowInfo("MK IsZeroBackoutNum true2:\n");
		return true;
	}

	return false;
}
//////////////////////////////////////////////////////////////////////////////////////////
MYWCHAR* NGramHistory::GetHistoryForPhrasePredictions(MYWCHAR* inputWord, MYWCHAR* nextLetter)
{
	//ShowInfo("MK GetHistoryForPhrasePredictions history1:(%s)\n",toA(NewCurrentHistory));
	//ShowInfo("MK GetHistoryForPhrasePredictions history2:(%s)\n",toA(HistoryForPhrase));
	//ShowInfo("MK GetHistoryForPhrasePredictions history3:(%s)\n",toA(BackSpaceHistory));
	//ShowInfo("MK GetHistoryForPhrasePredictions inputWord:(%s)\n",toA(inputWord));
	//ShowInfo("MK GetHistoryForPhrasePredictions nextLetter:(%s)\n",toA(nextLetter));
	//ShowInfo("MK GetHistoryForPhrasePredictions bSpace:(%d)\n",bSpace);

	MYWCHAR inputPhrase[MAX_PHRASE_LEN*2];
	mywcscpy(inputPhrase, HistoryForPhrase);

	MYWCHAR* histLast = GetLastWordFromPhrase(inputPhrase);
	MakeLowerCase(histLast);
	bool bEndSpace = false;
	static MYWCHAR* newInput = NULL;
	if(!isEmptyStr(inputWord) && isEmptyStr(nextLetter) && HasEndingSpace(inputPhrase) && mywcscmp(histLast, inputWord) != 0)
	{
		mywcscat(inputPhrase, inputWord);
		//ShowInfo("MK GetHistoryForPhrasePredictions-1:(%s)\n", toA(inputPhrase));
		newInput = GetLast4Words(inputPhrase);
		bEndSpace = true;
	}

	if(isEmptyStr(inputWord) && !isEmptyStr(nextLetter) )
	{
		mywcscat(inputPhrase, inputWord);
		mywcscat(inputPhrase, nextLetter);
		//ShowInfo("MK GetHistoryForPhrasePredictions-2:(%s)\n", toA(inputPhrase));
		newInput = GetLast4Words(inputPhrase);
	}

	//"Please l"->back space->"Please"->type "l"-->"Please l"======>No phrase prediction.
	if(isEmptyStr(inputWord) && !isEmptyStr(NewCurrentHistory) && isEmptyStr(HistoryForPhrase) && !isEmptyStr(nextLetter) )
	{
		mywcscpy(inputPhrase, NewCurrentHistory);
		mywcscat(inputPhrase, nextLetter);
		//ShowInfo("MK GetHistoryForPhrasePredictions-3:(%s)\n", toA(inputPhrase));
		newInput = GetLast4Words(inputPhrase);
	}

	if(!isEmptyStr(inputWord) && !isEmptyStr(nextLetter))
	{
		mywcscat(inputPhrase, nextLetter);
		//ShowInfo("MK GetHistoryForPhrasePredictions-4:(%s)\n", toA(inputPhrase));
		newInput = GetLast4Words(inputPhrase);
	}

	if(HasEndingSpace(NewCurrentHistory) && !HasEndingSpace(HistoryForPhrase) && isEmptyStr(nextLetter) 
		&& !isEmptyStr(inputWord))
	{
		DeleteLastWordFromPhrase(inputPhrase);

		mywcscat(inputPhrase, inputWord);
		//ShowInfo("MK GetHistoryForPhrasePredictions-5:(%s)\n", toA(inputPhrase));
		newInput = GetLast4Words(inputPhrase);
		if(bSpace)
		{
			bEndSpace = true;
		}
		else
		{
			if(bAdvanceWordComplete)
			{
				bEndSpace = true;
				bAdvanceWordComplete = false;
			}
		}
	}

	//Backspacing or erase last word
	if(!isEmptyStr(newInput))
	{
		newInput = GetLast4Words(inputPhrase);
		if(bEndSpace)
			AddEndSpace(newInput);
		
		//ShowInfo("MK GetHistoryForPhrasePredictions return1:(%s)\n", toA(newInput));
		return newInput;
	}
	else
	{
		//ShowInfo("MK GetHistoryForPhrasePredictions return2:(%s)\n", toA(HistoryForPhrase));
		return HistoryForPhrase;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
MYWCHAR* NGramHistory::GetLast4Words(MYWCHAR* inputPhrase)
{
	
	//ShowInfo("MK GetLast4Words inputPhrase:(%s)\n", toA(inputPhrase));
	static MYWCHAR reverseInput[MAX_PHRASE_LEN];
	int index =0;
	memset(reverseInput, 0, sizeof(reverseInput));
	int n = mywcslen(inputPhrase);
	int sp =0;
	bool bApostrophe = false;
	for(int i = n-1; i >=0; i--)
	{
		if(inputPhrase[i] == SP)
			sp++;

		if(isPunctuation(inputPhrase[i]) || sp >= MAX_TGRAPH_HISTORY_NUM+1)
			break;
		
		if(inputPhrase[i] == APOSTROPHE)
			bApostrophe = true;

		reverseInput[index++] = inputPhrase[i];
	}

	trimEndingSpaces(reverseInput);

	if(!isEmptyStr(reverseInput))
		myReverseStr(reverseInput);

	if(bApostrophe)
		ReplaceiToI(reverseInput);

	//ShowInfo("MK GetLast4Words reverseInput:(%s)\n", toA(reverseInput));

	return reverseInput;
	
}
//////////////////////////////////////////////////////////////////////////////////////////
void NGramHistory::SetDeleteFirstWord(MYWCHAR* inputWord, MYWCHAR* nextLetter,  MYWCHAR* cmpWord)
{
	//ShowInfo("MK ProcessPhrasePrediction space(%d)\n", bSpace );
	//ShowInfo("MK ProcessPhrasePrediction history1:(%s)\n",toA(NewCurrentHistory));
	//ShowInfo("MK ProcessPhrasePrediction history2:(%s)\n",toA(HistoryForPhrase));
	//ShowInfo("MK ProcessPhrasePrediction history3:(%s)\n",toA(BackSpaceHistory));
	//ShowInfo("MK ProcessPhrasePrediction inputWord:(%s)\n",toA(inputWord));
	//ShowInfo("MK ProcessPhrasePrediction nextLetter:(%s)\n",toA(nextLetter));
	
	if(cmpWord)
	{
		bDeleteFirstWord = true;
		return;
	}

	if(!bSpace && HasEndingSpace(NewCurrentHistory) && HasEndingSpace(HistoryForPhrase)
		&& HasEndingSpace(BackSpaceHistory) && !isEmptyStr(inputWord) && isEmptyStr(nextLetter))
	{
		//ShowInfo("MK ProcessPhrasePrediction bDeleteFirstWord true1\n");
		bDeleteFirstWord = true;
		return;
	}

	if(!bSpace && HasEndingSpace(NewCurrentHistory) && !HasEndingSpace(HistoryForPhrase)
		&& HasEndingSpace(BackSpaceHistory) && !isEmptyStr(inputWord) && isEmptyStr(nextLetter))
	{
		MYWCHAR* lastword = GetLastWordFromPhrase(HistoryForPhrase);
		MakeLowerCase(lastword);
		//ShowInfo("MK ProcessPhrasePrediction lastword:(%s)\n", toA(lastword));
		if(mywcscmp(lastword, inputWord) != 0)
		{
			//ShowInfo("MK ProcessPhrasePrediction bDeleteFirstWord true2\n");
			bDeleteFirstWord = true;
			return;
		}
	}

	if(bSpace)
	{
		bDeleteFirstWord = true;
		//ShowInfo("MK ProcessPhrasePrediction bDeleteFirstWord true3\n");
	}
	else
	{
		bDeleteFirstWord = false;
		//ShowInfo("MK ProcessPhrasePrediction bDeleteFirstWord false\n");
	}

}
///////////////////////////////////////////////////////////////////////////////////////////////////////
MYWCHAR* NGramHistory::GetNewWordInputForPhrase(MYWCHAR* history)
{
	if(!history)
		return NULL;

	//"Please le", then igonore "le" and return "please"

	static MYWCHAR newInput[MAX_WORD_LEN];
	memset(newInput,0,sizeof(newInput));
	int index =0;
	int n = mywcslen(history);
	bool bStart = false;
	for(int i=n-1; i >= 0; i--)
	{
		if(bStart && history[i] == SP)
			break;

		if(history[i] == SP)
		{
			bStart = true;
			continue;
		}
		
		if(bStart)
			newInput[index++] = history[i];
	}

	if(!isEmptyStr(newInput))
		myReverseStr(newInput);

	return newInput;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
MYWCHAR* NGramHistory::GetNumberPredictionWord(MYWCHAR* curLetter, bool bInputSpace)
{
	static MYWCHAR* retStr = NULL;
	retStr = GetLastWordFromPhrase(HistoryForPhrase);
	//ShowInfo("MK GetNumberPredictionWord retStr:(%s), curLetter:(%s), bInputSpace:%d\n",toA(retStr), toA(curLetter), bInputSpace);

	if(!isNumber(retStr) || !isEmptyStr(curLetter))
	{
		//ShowInfo("MK GetNumberPredictionWord return NULL1\n");
		return NULL;
	}

	if(bInputSpace)
	{
		//ShowInfo("MK GetNumberPredictionWord return retStr:(%s)\n",toA(retStr));
		return retStr;
	}
	else
	{
		//Spacing after backspacing.
		MYWCHAR* last = GetLastWordFromPhrase(BackSpaceHistory);
		//ShowInfo("MK GetNumberPredictionWord last from BackSpaceHistory:(%s)\n",toA(last));
		if(isNumber(last))
			return retStr;
	}

	//ShowInfo("MK GetNumberPredictionWord return NULL2\n");
	return NULL;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
bool NGramHistory::GetUpdateForStartWordsWithLetter()
{
	return (bStartingWordMode && !bSpace);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
bool NGramHistory::GetUpdateAfterNumbers(MYWCHAR* curLetter)
{
	return (curLetter && !bStartingWordMode && !bSpace && bUpdateAfterNumbers);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
void NGramHistory::ResetForSpace()
{
	bUpdateAfterNumbers = false;
	bSpace = true;
	bStartingWordMode  = false;
	SetStartingWordMode(false);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
void NGramHistory::ResetAfterAdvanceWord()
{
	bChangedCurrentHistory = false;
	bStartedUpperCaseLetter = false;
	bStartedUpperCaseWord = false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
void NGramHistory::ResetHistoryPhrases()
{
	memset(NewCurrentHistory,0,sizeof(NewCurrentHistory));
	memset(HistoryForPhrase,0,sizeof(HistoryForPhrase));
	memset(BackSpaceHistory,0,sizeof(BackSpaceHistory));
}