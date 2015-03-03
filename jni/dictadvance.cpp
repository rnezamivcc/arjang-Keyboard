// <copyright file="dictadvance.cpp" company="WordLogic Corporation">
// Copyright (c) 2000, 2013 All Right Reserved, http://www.wordlogic.com/
//
// THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
// KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// </copyright>
// <author>Reza Nezami</author>
// <email>rnezami@wordlogic.com</email>
// <date>2012-06-10</date>
// <summary>Provides engine support for advancing current word by adding next letter to it or advancing on a phrase.</summary>

#include "stdafx.h"
#include "dictmanager.h"
#include "dictionary.h"
#include "wordpunct.h"
#include "userWordCache.h"
#include "userWordCacheOffline.h"
#include "searchResults.h"
#include "wordpath.h"
#include "wordpathext.h"
#include "T-Graph.h"
#include "phraseEngine.h"
#include "dictPhrase.h"
#include "compactstore.h"
#include "autocorrect.h"
#include <ctype.h>

#include <time.h>

BYTE CDictManager::sNumBackouts = 0;
extern BYTE sNumAllDictSearchSlots;    // this is == sMaxSearchRanks - 10

#define MAX_EMAIL_SIZE	3
#define MAX_AFTER_NUMBER_SIZE 21

const char* EmailArr[] = {"@gmail.com", "@yahoo.com", "@outlook.com"};
const char* AfterNumbersArr[] = 
{
	"am",
	"pm",
	"o'clock",
	"minute",
	"minutes",
	"month",
	"months",
	"million",
	"millions",
	"second",
	"seconds",
	"hour",
	"hours",
	"dollar",
	"dollars",
	"day",
	"days",
	"year",
	"years",
	"billion",
	"billions"
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dynamic learning
// This function assumes we are dealing with an ended word and new word is empty
// if thisword is not null, it means we are learning for cache, not a new word.
/*BOOL CDictManager::mngrLearnLatestWord(MYWCHAR *thisword) 
{
	return false;
	// retrieve latest word and its preference, remove trailing punctuation etc
	BOOL result = FALSE;
	BOOL backToCurrentWord = BackToPreviousWordIfAny();

	if (m_autoLearnActive && backToCurrentWord) 
	{
		bool isNewWord = (gWordProp->nPathNodes + gWordProp->nNullMoves) >= 3;
		int len;
		MYWCHAR *currentWord = getCurrentWord(len);
		ShowInfo("mngrLearnLatestWord  usercharsBuf #%s# nPath %d nNull %d, currWord=%s \n",toA(&gWordProp->charsBuf[1]), gWordProp->nPathNodes, gWordProp->nNullMoves, toA(currentWord));
		int excessLettersRemoved = trimEndingSpaces(currentWord);
		if(mywcslen(currentWord) >2)
		{
			excessLettersRemoved += declutterWord(currentWord, gWordProp->nPathNodes);
			if(excessLettersRemoved == gWordProp->nNullMoves)
				isNewWord = false;
			int finallen = mywcslen(currentWord);
			if(finallen > 2)
			{
				mUserWordCache->putWord(currentWord, MEDIUM_PREFERENCE, finallen, isNewWord);
				result = TRUE;
			}
		}
	}
	else if(thisword) // learning for cache
	{
		int excessLettersRemoved = trimEndingSpaces(thisword);
		declutterWord(thisword, gWordProp->nPathNodes);
		int finallen = mywcslen(thisword);
		ShowInfo("mngrLearnLatestWord: add word #%s# to cache at pos %d if finallen(%d)>2\n", toA(thisword), sWordPosInPhrase, finallen);
		if(finallen > 2)
		{
			mUserWordCache->putWord(thisword, LOW_PREFERENCE, finallen, false);
			result = TRUE;
		}
	}

	backToCurrentWord = backToCurrentWord && BackToWorkingWord();
	return result;
}
*/
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CDictManager::advanceLetter(MYWCHAR letter, MYWCHAR **printPart, BOOL startSentence)
{
	if (m_noDictionariesLoaded || gWordProp->nPathNodes>=MAX_WORD_LEN)
	{
		ShowInfo("-No dictionary loaded or exceeded word path size!!\n");
		return FALSE;
	}
	
	static MYWCHAR advLetPart[3] = {NUL, NUL, NUL}; // letter + possible SP + NUL
	advLetPart[0] = letter; advLetPart[1]=advLetPart[2] = NUL;
	*printPart = advLetPart;
	return mngrAdvanceLetter(letter, advLetPart, startSentence);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// interface for advancing a letter in current word. For internal engine use only!
BOOL CDictManager::mngrAdvanceLetter(MYWCHAR letter, MYWCHAR *printChunk, BOOL startSentence) 
{
	unsigned len = 0;
	MYWCHAR *rootWord = getCurrentWord(len);
	ShowInfo("mngrAdvanceLetter: letter: #%c#, maxLayerId=%d, gHistWordIdx=%d, nPath=%d, sNumBackout=%d, currWord=#%s#\n", 
		   letter, gWordProp->maxLayerId, gHistWordIdx, gWordProp->nPathNodes, sNumBackouts, toA(rootWord));
	WLBreakIf(*gWordProp->charsBufP == SP, "!!ERROR! advanceLetter with last letter being SP?! we should have moved to nextword?\n");
	sWordPosInPhrase = startSentence;
	bool doAdvance = true;
	BYTE ret = 0;
	bool sp_cr_tab = SP_CR_TAB(letter);
	bool cr_tab_lf = CR_TAB_LF (letter);
	if(sp_cr_tab || cr_tab_lf)
		doAdvance = false;

	PuncChoice *pc = findPunctuationChoice(letter);
	if(pc || gWordProp->nNullMoves > 0)
		doAdvance = false;
	
	if(pc && pc->nSPBefore == 0 && FirstMoveAfterSPTerminatedWord())
	{
		backspaceLetter(rootWord);
		doAdvance = false;
	}
	
	if(doAdvance)
		ret = mngrFillNextNodeDictionaries(letter, eExploring); //now advance the wordpath with this letter

	ShowInfo("--advanceLetter after advance on #%c#, pc=%x,  nNull=%d, ret=%d.\n", letter, pc, gWordProp->nNullMoves, ret);
//	if(pc)
//		learnCurrentWord(true); // learn here, before pc letter is added to gWordProp
	if (!cr_tab_lf)
		putLetterInWordpath(letter, ret, pc);

	ShowInfo("--wordUnderConstruction \"%s\", nPath=%d, nNull=%d\n", toA(getCurrentWord(len)), gWordProp->nPathNodes, gWordProp->nNullMoves);
	if(ret)
	{
		WLBreakIf(gWordProp->nNullMoves, "!!ERROR! advanceLetter: gWordProp->nNullMoves > 0 ?\n");
		sNumBackouts = gWordProp->nPathNodes;
		doLanguageSpecificCorrections(letter, printChunk);
	}	
	else
	{
		if(pc)
		{
			if (pc->nSPAfter>0)// &&(gWordProp->nNullMoves>0 || !partOfWordsInAnyDictionaries()))
				printChunk[1] = SP;
			goToNextWord((char*)"AdvanceLetter_pc", pc);
		}
		else if(sp_cr_tab || cr_tab_lf)
		{
			WLBreakIf(!sp_cr_tab && ! cr_tab_lf, "!!ERROR! why we are ending this word?\n");
			sNumBackouts = 0;
			goToNextWord((char*)"AdvanceLetter_ret sp_cr_tab || cr_tab_lf", gWordProp->nSP<2);
		}
	}

	ShowInfo("--mngrAdvanceLetter: ret=%d, printChunk=#%s#, gHistWordIdx=%d,nPathNodes=%d, sNumBackouts=%d nNull=%d,  exSP=%d \n",
		ret, toA(printChunk), gHistWordIdx, gWordProp->nPathNodes, sNumBackouts, gWordProp->nNullMoves, gWordProp->nSP);
	return ret;
}

/////////////////////////////////////////////////////////////////////
// checks if this letter is a valid next letter for current word under construction
// if so, returns its preference. Otherwise returns 0
BYTE CDictManager::getLetterPref(MYWCHAR letter)
{
	BYTE ret = 0;
	if(gWordProp->nNullMoves > 0)
		return 0;
	for (int j = 0; j < m_nOrderedDicts; j++)
	{
		Dictionary *existDict = getDictionaryEnabled(j);
		if (existDict)
			ret = max(ret, existDict->getLetterPref(letter));
	}
	return ret;
}

///////////////////////////////////////////////////////////////////////////////////////////
void CDictManager::LearnStartingWords(MYWCHAR *word)
{
	MYWCHAR* inputWord = word ? word : m_History.GetLearnStartingWord();
	ShowInfo("CDictManager::LearnStartingWords:Learn Starting history1:(%s), wordToLearn=(%s)\n",toA(m_History.NewCurrentHistory), toA(inputWord));
	if(!inputWord)
	{
		ShowInfo("---Word to learn is empty!!!\n");
		return;
	}

	MakeLowerCase(inputWord);
	ShowInfo("MK Learn Starting inputWord:(%s)\n",toA(inputWord));
	int dictIdx =0;
	CCompactStore *mStore = getTopDictionary()->getCompactStore();
	CompactNode* endnode = retrieveEndNodeForString(inputWord, &dictIdx, true);
	if(mStore && endnode && CCompactStore::isEndpoint(endnode))
	{
		USHORT savedPref = m_NGramLearning->GetStartWordCache()->GetStartWordPref(endnode); //gPhraseEngine->getStartPreference(endnode);
		if(savedPref == 0)
		{
			ShowInfo("--word is not a starting word in PhraseEngine, adding startingWord (%s) with default pref=%d to cache\n", toA(inputWord), (LEARNED_START_PREF+START_PREF_INCREMENT));
			m_NGramLearning->addStartWordToCache(endnode, LEARNED_START_PREF);
			/*(	gPhraseEngine->addStartingWord(endnode, (LEARNED_START_PREF+START_PREF_INCREMENT));
			savedPref = gPhraseEngine->getStartPreference(endnode);
			WLBreakIf( savedPref < LEARNED_START_PREF, "!!!!ERROR! CDictManager::LearnStartingWords: even after learning: startPref < LEARNED_START_PREF!!\n");
		*/
		}
		else
		{
			WLBreakIf( savedPref < LEARNED_START_PREF, "!!!!ERROR! CDictManager::LearnStartingWords: startPref < LEARNED_START_PREF!!\n");
			savedPref = min((savedPref + START_PREF_INCREMENT), MAX_START_PREF);

			m_NGramLearning->GetStartWordCache()->SetSavedStartWordPref(endnode, savedPref);

			//gPhraseEngine->setStartPreference(endnode, savedPref);
			//BYTE savedPref1 = gPhraseEngine->getStartPreference(endnode);
		}
	}	
}

////////////////////////////////////////////////////////////////////////////////////////////////////
MYWCHAR*  CDictManager::DeleteStartingWords(MYWCHAR* wordPart)
{
	ShowInfo("CDictManager::DeleteStartingWords: deleting word (%s)\n", toA(wordPart));
	MYWCHAR* wordToDelete = m_History.GetWordToDelete(wordPart);
	if(isEmptyStr(wordToDelete))
		return NULL;

	MakeLowerCase(wordToDelete);

	ShowInfo("MK DeleteStartingWords word:(%s)\n",toA(wordToDelete));

	int dictIdx =0;
	CompactNode* endnode = retrieveEndNodeForString(wordToDelete, &dictIdx, true);
	if(endnode && CCompactStore::isEndpoint(endnode))
	{
		if(m_NGramLearning->GetStartWordCache()->GetStartWordPref(endnode) > 0)
			m_NGramLearning->GetStartWordCache()->deleteStartWord(endnode);	
	}

	return wordToDelete;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// test advance only on solid paths, so null moves are terminated
// test advance only on non-terminating letters! so sp_tab_cr or punctions are rejected!
// assumes letters are ordered based or prefs, so letters[0] is the highest probable!
// punctuations and SP or TAB or similarly string ending chars should be sent here as single letter advance!
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
MultiLetterAdvanceAr *CDictManager::advanceMultiLetters(MYWCHAR *letters, BYTE *prefs, MYWCHAR *rootWordP,
														MYWCHAR **printPart, BOOL startSentence, BOOL literal)
{
#ifdef TIMETEST //_DEBUG
	if(m_Start == 0)
		m_Start = clock();
	else
	{
		double diff = clock() - m_Start;
		ShowInfo("Time between keys:%f\n", diff);
		m_Start += diff;
	}

	clock_t start, end;
	start = clock();
#endif

	WLBreakIf(!letters, "!!ERROR! advanceMultiLetters: letters is NULL!!\n");
	//ShowInfo("MK advanceMultiLetters letters(%s),rootWordP(%s)\n",toA(letters),toA(rootWordP));
	//ShowInfo("MK advanceMultiLetters history1:(%s)\n",toA(m_History.NewCurrentHistory));
	//ShowInfo("MK advanceMultiLetters history2:(%s)\n",toA(m_History.HistoryForPhrase));
	//ShowInfo("MK advanceMultiLetters history3:(%s)\n",toA(m_History.BackSpaceHistory));
	WLBreakIf(rootWordP==NULL, "\n!!ERROR! CDictManager::advanceMultiLetters: rootWord data not set!\n");
	int len = constructCurWord(rootWordP);
	bool bStartWord = false;
	if(!isPunctuation(letters[0]))
	{
		m_History.bReplaced = false;
		if(!isalpha(letters[0]))
			m_History.bStartingWordMode = false;
	}
	else
	{
		if(letters[0] != COMMA)
			bStartWord = true;	
	}

	m_History.nEraseLastWord = 0;
	if(letters[0] != SP)
		m_History.CheckNumber();
	
	int numletters = mywcslen(letters);
	//ShowInfo("MK advanceMultiLetters rootWordP(%s)\n",toA(rootWordP));

	m_History.CheckEmail(letters, numletters);

	bool bUpdateTGraph = true;
	m_History.bSpace = false;
	if(letters[0] == SP && rootWordP)
	{	
		m_History.ResetForSpace();
		LearnStartingWords();
	}

	if(letters[0] != SP && len == 0)
		bUpdateTGraph = m_History.CheckUpdateTGraph();
	
	ShowInfo("\nAdvanceMultiLetters: numlets=%d, letters=#%s#, gHistWordIdx=%d, gWorkingWordIdx=%d, nPath=%d, nNull=%d, sNumBackouts=%d, currWord=#%s#, literal=%d\n", 
		     numletters, toA(letters), gHistWordIdx, gWorkingWordIdx, gWordProp->nPathNodes, gWordProp->nNullMoves, sNumBackouts, toA(rootWordP), literal);
	
	sWordPosInPhrase = startSentence;

	//resetting multiletter next word array:
	mMultiLetterNexts.reset();
	sDoFillFinalList = false;

	static MYWCHAR advLetPart[3] = {NUL, NUL, NUL}; // letter + possible SP + NUL
	advLetPart[0] = letters[0]; advLetPart[1]=advLetPart[2] = NUL;
	*printPart = advLetPart;

	for(int i=0; i<NCURWORDS; i++)  // reset advance flag for each working path.
		gWorkingPathsAdvanced[i] = false;

	MYWCHAR runningRootWord[MAX_WORD_LEN];
	mywcscpy(runningRootWord, rootWordP);
		
	PuncChoice *pc = findPunctuationChoice(letters[0]);
	
	// first take care of non advancing cases such as punctuation or sp or tab .... In these cases we expact numLetters=1!
	bool doAdvance = true;
	if(numletters == 1)
	{
		ShowInfo("--one letter advance: #%c#:\n", char2A(letters[0]));
		MYWCHAR letter = letters[0];
		bool sp_cr_tab = SP_CR_TAB(letter);
		if(CR_TAB_LF (letter))
		{
			letter = SP;
			sp_cr_tab = true;
		}

		if(pc && pc->nSPBefore == 0 && !literal && FirstMoveAfterSPTerminatedWord())
		{
			backspaceLetter(runningRootWord, false);
			mMultiLetterNexts.nBackSpace++;
		}

		if(sp_cr_tab || (pc && /*!pc->canBondWords &&*/ !literal) || gWordProp->nNullMoves > 0)
			doAdvance = false;
	    
		if(!doAdvance)
		{
			if (pc && pc->nSPAfter>0  && !literal)// && (gWordProp->nNullMoves>0 || !partOfWordsInAnyDictionaries()))
				advLetPart[1] = SP;
			if(sp_cr_tab && getWorkingWordLength() == 0) // takes care of multiple SP_CR_LN_TAB: turn them into SP attached to end of prev word!
			{
				ShowInfo("----multiple SP! attach to prev word!\n");
				BackToPreviousWord(false);
				putLetterInWordpath(SP, 0);
				BackToWorkingWord();
			}
			else
			{
				putLetterInWordpath(letter, 0, !literal ? pc : NULL );
				if(sp_cr_tab || (pc && !literal))
				{
					goToNextWord((char*)"advanceMultiLetters: sp_cr_tab || Punctuation", pc );
				}
				if(!pc)
				{
					int numNextWords = 0;
					MYWCHAR rootWord[MAX_WORD_LEN];
					GetNextWords(&numNextWords, rootWord, true, letters); 
					
					updateMultiLetterNextWords(1, rootWord, true, true,bUpdateTGraph, m_History.bSpace); //adjust multiletter next word array based on this nextwords:
					mMultiLetterNexts.setActualNexts();
				
				}
			}
			sNumBackouts = gWordProp->nPathNodes + gWordProp->nNullMoves;

			if(m_History.bIsThisEmail)
			{
				UpdateForEmails();
			}
			else
			{
				//ShowInfo("MK advanceMultiLetters bStartWord(%d), m_History.bStartingWordMode(%d)\n",bStartWord, m_History.bStartingWordMode)
				if(bStartWord || m_History.bStartingWordMode)
					UpdateForStartWords();
				
			}

			mMultiLetterNexts.setActualNexts();
			//ShowInfo("MK actualnext for advanceMultiLetters1:(%d)\n", mMultiLetterNexts.nActualNexts);
			for(int i=0; i<mMultiLetterNexts.nActualNexts; i++)
			{
				if(mywcslen(mMultiLetterNexts.nextWords[i]) == 4 && mMultiLetterNexts.nextWords[i][0] == 'i')
					mMultiLetterNexts.nextWords[i][0] = uprCharacter(mMultiLetterNexts.nextWords[i][0]);

				if(mMultiLetterNexts.nShouldUpperCase[i] > 0)
				{
					mMultiLetterNexts.nextWords[i][0] = uprCharacter(mMultiLetterNexts.nextWords[i][0]);
				}
				ShowInfo("MK actualnext for advanceMultiLetters1:(%s)\n", toA(mMultiLetterNexts.nextWords[i]));
			}

			//DoAutoCorrect(letters);


			return &mMultiLetterNexts;
		}
	}

	m_History.SetStartingWordMode(false);

	// next, go over all active paths and try to advance each one on this letter: (any null move kills the path!
	bool doneAdvancing = false;
	int count = 0;
	BYTE ret = 0;
	for(; count < NCURWORDS && !doneAdvancing; count++)
	{
		if(gWorkingPathsAdvanced[count]==true || gWorkingWordPaths[count].pref == 0xff)
			continue;
		goToWorkingWordPath(count); // switch to this working path:
		ShowInfo("--advancing on #%s# with letters:\n", toA(getWorkingWord(count)));  
		for(int i=0; i<numletters; i++)
		{
			MYWCHAR letter = letters[i];
			
			// first check if this is a valid letter to advance on:
			if(SP_CR_TAB(letter) ||  CR_TAB_LF (letter))
			{
				ShowInfo("!!WARNING! advanceMultiLetters: we don't multiAdvance on #%c#!\n", char2A(letter));
				continue;
			}
				
			// test if letter is a valid move for current word under construction. Only if it is main (0) working path we advance on nulls!
			BYTE thisPref = getLetterPref(letter);
			if(thisPref == 0 && count > 0 )
				continue;
			
			// next, find next working path based on current working path, so we can continue advancing for other letters.
            thisPref = min((prefs[i] + thisPref), (int)MAXIMUM_PREFERENCE);
			int nextWorkingIdx = findNextAvailableWorkingPath();
			if(gWorkingWordPaths[nextWorkingIdx].pref != 0xff && gWorkingWordPaths[nextWorkingIdx].pref > thisPref)
			{
			//	ShowInfo("!!WARNING! advanceMultiLetters: running out of available slots! minPref > %d!!\n", thisPref);
				continue;
			//	doneAdvancing = true;
			}
			else if((i+1) != numletters)
			{   // next, deep copy workingpath to a next one:
				deepCopyWorkingPath(gWorkingWordPaths[nextWorkingIdx], *gWPath, getWorkingHeap(nextWorkingIdx)); 
			}
			
			// now, advance on the letter on current working path:

			///////////////////Minkyu:2014.05.07//////////////////////////
			//First, find next words with lower case.
			letter = lwrCharacter(letter);
			//////////////////////////////////////////////////////////////
			ret = mngrFillNextNodeDictionaries(letter, eExploring);
			//WLBreakIf(!ret, "!!ERROR! advanceMultiLetters: mngrFillNextNodeDictionaries failed on letter (%c)!! why?!\n", char2A(letter));
			putLetterInWordpath(letter, ret, NULL);
			gWorkingPathsAdvanced[gWorkingWordIdx] = true;
			gWorkingWordPaths[gWorkingWordIdx].pref = thisPref; //gWorkingWordPaths[gWorkingWordIdx].pref==0xff ? 
														//thisPref : (thisPref + gWorkingWordPaths[gWorkingWordIdx].pref)/2;
			//if(ret && numletters==1) // FIXME!!!!
			//	doLanguageSpecificCorrections(letter, advLetPart);

			// find next words for this letter:
			int numNextWordsPerLetter = 0;
			GetNextWords(&numNextWordsPerLetter, runningRootWord, false, letters); 

			updateMultiLetterNextWords((USHORT)prefs[i], runningRootWord, true, true, bUpdateTGraph, m_History.bSpace); //adjust multiletter next word array based on this nextwords:
			
			// switch to next available working path for next letter, if it exists
			if(!doneAdvancing)
				goToWorkingWordPath(nextWorkingIdx);
		}

		if(getWorkingWordLength(count) < getWorkingWordLength(0)) // means this path didn't manage to advance, so free it for possible other paths branching into it.
			gWorkingWordPaths[count].pref = 0xff;
		if(gWorkingPathsAdvanced[gWorkingWordIdx] == false)
			gWorkingWordPaths[gWorkingWordIdx].pref = 0xff;
	}

	mMultiLetterNexts.setActualNexts();
	//ShowInfo("MK actualnext for advanceMultiLetters2:(%d)\n", mMultiLetterNexts.nActualNexts);
	MYWCHAR* lastPart = GetLastWordFromPhrase(m_History.HistoryForPhrase);
	bool upperCase = ((letters && isUpperCase(letters[0])) || (!isEmptyStr(rootWordP) &&  isUpperCase(lastPart[0])));

	for(int i=0; i<mMultiLetterNexts.nActualNexts; i++)
	{
		if(upperCase)
			mMultiLetterNexts.nextWords[i][0] = uprCharacter(mMultiLetterNexts.nextWords[i][0]);

		if(letters && letters[0] == APOSTROPHE )
		{
			if(mMultiLetterNexts.nextWords[i][0] == 'i')
				//mMultiLetterNexts.nextWords[i][0] = uprCharacter(mMultiLetterNexts.nextWords[i][0]);
				ReplaceiToI(mMultiLetterNexts.nextWords[i]);
		}

		if(mMultiLetterNexts.nShouldUpperCase[i] > 0)
		{
			mMultiLetterNexts.nextWords[i][0] = uprCharacter(mMultiLetterNexts.nextWords[i][0]);
		}
		ShowInfo("MK actualnext for advanceMultiLetters2:(%s)\n", toA(mMultiLetterNexts.nextWords[i]));
	}

	sDoFillFinalList = true;
//	fillFiveMostPreferredWords();
	goToWorkingWordPath(0);  // going back to default workingpath.
	sNumBackouts = gWordProp->nPathNodes + gWordProp->nNullMoves;

	// now do autocorrect:
	//DoAutoCorrect(letters);

#ifdef TIMETEST
	end = clock();
	double diff = Diffclock(end, start);
	ShowInfo("Time AdvanceMultiLetters prev History:(%s)-->%f\n", toA(m_History.HistoryForPhrase), diff);
#endif

	return &mMultiLetterNexts;
}
//Move this to TGraph::UpdateAutoCorrect()
///////////////////////////////////////////////////////////////////////////////////////////
// This is entry point for doing autocorrect and applying the result to th output list of 
// of predictions, which is mMultiLetterNexts. 
// Note: if correction list is not empty, we will add at least the top correction to the predictin list.
// returns number of corrections added to the list.
///////////////////////////////////////////////////////////////////////////////////////////
int CDictManager::DoAutoCorrect(MYWCHAR *advances)
{
	int len = mywcslen(advances);
	for(int i=0; i<len; i++)
			mAutoCorrect->addChar(advances[i]);

	goToWorkingWordPath(0);
	CorrectionList * corrList = mAutoCorrect->getPredictions(5);
	int addedNum = 0;
	// now merge nextwords list with correction list:
	if(corrList->fillCount > 0)
	{
		mMultiLetterNexts.add(corrList->corrections[0].word, MAX_PREFERENCE);
		addedNum++;
		for(int i=1; i<corrList->fillCount; i++)
		{
			mMultiLetterNexts.add(corrList->corrections[i].word, MIN_PREFERENCE);
			addedNum++;
		}
	}
	return addedNum;
}
////////////////////////////////////////////////////////////////////////////////////////////
MYWCHAR* CDictManager::GetSwipePath(MYWCHAR* letters)
{

	static MYWCHAR newWord[MAX_WORD_LEN];
	int len = mywcslen(letters);
	int index =0;
	int dictInd = getTopDictionary()->GetDictIndex();
	CompactNode *topNode = gFirstNodeOfDicts[dictInd];

	CompactNode* nextNode = NULL;
	for(int i=0; i< len; i++)
	{
		nextNode = mTopDictionary->getCompactStore()->nextLetterNode(topNode,letters[i]);
		if(nextNode)
		{
			newWord[index++] = letters[i];
			topNode = nextNode;
		}
	}

	ShowInfo("CDictManager::GetSwipePath:path[%s], suggestion[%s],length:%d\n", toA(letters),toA(newWord), index);
	newWord[index] = NUL;

	return newWord;
}

////////////////////////////////////////////////////////////////////////////////////////////
MultiLetterAdvanceAr* CDictManager::advanceLetterSwipe(MYWCHAR* letters, bool complete)
{
	static MultiLetterAdvanceAr arr;
	arr.reset();

	if(isEmptyStr(letters))
		return &arr;

	//MakeLowerCase(letters);
	ShowInfo("CDictManager::advanceLetterSwipe letters:[%s]\n", toA(letters));
	mAutoCorrect->ClearBuffer();

	MYWCHAR* input = GetSwipePath(letters);
	int len = mywcslen(input);
	for(int i=0; i<len;i++)
	{
		mAutoCorrect->addChar(input[i]);
	}
	
	CorrectionList * corrList = mAutoCorrect->getPredictions(5);
	if(!corrList)
		return &arr;

	BYTE totalCorrectionCount = corrList->fillCount;
	if(totalCorrectionCount == 0)
		return &arr;

	for(int i=0; i < totalCorrectionCount && i < NMWORDPREDICTIONS;i++)
	{
		mywcscpy(arr.nextWords[i], corrList->corrections[i].word); 
		arr.prefs[i] = MIN_PREFERENCE+i;
		arr.bCorrection[i] = true;
		arr.nCorrections++;	
		ShowInfo("MK advanceLetterSwipe corrections:[%s], pref[%hu]\n", toA(arr.nextWords[i]), arr.prefs[i]);
	}

	arr.setActualNexts();

	return &arr;
}
////////////////////////////////////////////////////////////////////////////////////////////
// this function adds "order'th term" to the end of the number. It assumes the input str is a number
// Since this is a number, then numStr has at least 2 empty char slot at the end to be used, as is done bellow!
// REZA: BAD !! this whole function has to be moved to dictionary since it is language specific!
void CDictManager::UpdateForNumbers(MYWCHAR *numStr, USHORT basePref)
{
	//ShowInfo("MK UpdateForNumbers input numStr:(%s)\n",toA(numStr));

	Dictionary* dict = getTopDictionary();
	if(isEmptyStr(numStr) || !dict || dict->GetDictLanguage() == eLang_KOREAN)
	{
		//ShowInfo("MK UpdateForNumbers return\n");
		return;
	}

	static const char *numEnding[4] = { "st", "nd", "rd", "th"};

	char* szNumStr = toA(numStr);

	int n = (int)strlen(szNumStr);
	//ShowInfo("MK UpdateForNumbers szNumStr:(%s)[%d]\n",szNumStr, n);
	char numChar = szNumStr[n-1];

	int idx = (numChar - '3') > 0 ? 3 : numChar - '1';
	if(idx < 0)
		idx = 3;

	if(n == 2)
	{
		int nNum = atoi(szNumStr);
		if(nNum >= 10 && nNum <= 13)
			idx = 3;
	}

	char newNumStr[MAX_WORD_LEN];
	memset(newNumStr, 0, sizeof(newNumStr));
	for(int i=0; i < n; i++)
	{
		newNumStr[i] = szNumStr[i];
	}
	//ShowInfo("MK UpdateForNumbers newNumStr1:(%s)[%d]\n",newNumStr, n);
	newNumStr[n] = numEnding[idx][0];
	newNumStr[n+1] = numEnding[idx][1];

	//ShowInfo("MK UpdateForNumbers newNumStr2:(%s)[%d]\n",newNumStr, n);
	numStr = toW(newNumStr);

	mMultiLetterNexts.reset();//Array should be empty here.

	//ShowInfo("MK UpdateForNumbers final numStr:(%s)\n", toA(numStr));
	mywcscpy(mMultiLetterNexts.nextWords[0], numStr);
	mMultiLetterNexts.prefs[0] = basePref+10;
	//ShowInfo("MK UpdateForNumbers:(%s)[%d]\n", toA(mMultiLetterNexts.nextWords[0]), mMultiLetterNexts.prefs[0]);

}

////////////////////////////////////////////////////////////////////////////////////////////
void CDictManager::UpdateForDegrees(USHORT basePref)
{
	int index =0;
	for(int i =0; i < NMWORDPREDICTIONS; i++)
	{
		if(index == sNumDegreeAdded)
			break;

		if(isEmptyStr(mMultiLetterNexts.nextWords[i]) && index < 2)
		{
			mMultiLetterNexts.nextWords[i][0] = sDegree[index];
			mMultiLetterNexts.nextWords[i][1] = NUL;
			mMultiLetterNexts.prefs[i] = basePref+i;
			index++;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CDictManager::updateMultiLetterNextWords(USHORT prefbase, MYWCHAR *curLetter, bool complete, 
											  bool replace, bool bUpdateTGraph, bool bSpace)
{
	//ShowInfo("MK updateMultiLetterNextWords history:(%s)\n",toA(m_History.NewCurrentHistory));
	//ShowInfo("MK updateMultiLetterNextWords history2:(%s)\n",toA(m_History.HistoryForPhrase));
	//ShowInfo("MK updateMultiLetterNextWords history3:(%s)\n",toA(m_History.BackSpaceHistory));
	//ShowInfo("MK updateMultiLetterNextWords curLetter:(%s)\n",toA(curLetter));
	//ShowInfo("MK updateMultiLetterNextWords bSpace:(%d)\n",bSpace);
	//ShowInfo("MK updateMultiLetterNextWords m_History.bSpace:(%d)\n",m_History.bSpace);
	//ShowInfo("MK updateMultiLetterNextWords bUpdateTGraph:(%d)\n",bUpdateTGraph);
	//ShowInfo("MK updateMultiLetterNextWords complete:(%d)\n",complete);
	//ShowInfo("MK updateMultiLetterNextWords replace:(%d)\n",replace);

	WLBreakIf(mTopDictionary == NULL, "!!ERROR! updateMultiLetterNextWords: mTopDictionary is null!\n");
	int startIdx = sNumAllDictSearchSlots+NWORDPREFERENCES; 
	int endIndex = startIdx + PWresult.nApprovedResults;
	
	for (int i = startIdx; i < endIndex; i++)
	{
		SearchResultEntry *mysrep = &gRankedSearchResults[i];
		USHORT lowest = mysrep->cascadingPref + prefbase;
		int lowestIdx = -1;
		for(int j=0; j<NMWORDPREDICTIONS; j++)
		{
			if(mMultiLetterNexts.prefs[j]!=0xffff && mywcscmp(mysrep->predText, mMultiLetterNexts.nextWords[j])==0)
			{
				mMultiLetterNexts.prefs[j] = max(mMultiLetterNexts.prefs[j], mysrep->cascadingPref);
				break;
			}
			if(mMultiLetterNexts.prefs[j]==0xffff)
			{
				mywcscpy(mMultiLetterNexts.nextWords[j], mysrep->predText);
				mMultiLetterNexts.prefs[j] = mysrep->cascadingPref+prefbase;
				lowestIdx = -1;
				break;
			}
			if(mMultiLetterNexts.prefs[j] < lowest)
			{
				lowest= mMultiLetterNexts.prefs[j];
				lowestIdx = j;
			}
		}

		if(lowestIdx != -1)
		{
			mywcscpy(mMultiLetterNexts.nextWords[lowestIdx], mysrep->predText);
			mMultiLetterNexts.prefs[lowestIdx] = mysrep->cascadingPref + prefbase;
		}
	}

	MYWCHAR* lastWord = m_History.GetNumberPredictionWord(curLetter, bSpace);
	//ShowInfo("MK updateMultiLetterNextWords lastWord:(%s)\n",toA(lastWord));
	if(!isEmptyStr(lastWord))
	{
		USHORT basePref = calcCascadingBasePreference(1, eOrdinary, 0);
		UpdateForNumbers(lastWord, basePref);
		UpdateForDegrees(basePref);
	}
	else
	{
		bool bAllowTGraph = true;
		bool bProcessedTGraph = false;
		//CASE:"go t "-->TGraph should not be processed.
		if(bSpace && isEmptyStr(curLetter))
		{
			MYWCHAR* temp = GetLastWordFromPhrase(m_History.NewCurrentHistory);
			int dictIdx = 0;
			bAllowTGraph = (NULL != QuadRetrieveCompactNode(temp, false, &dictIdx));
		}
		//Minkyu: 2013.10.01
		if(bAllowTGraph && m_TGraph && (complete || replace) && bUpdateTGraph)
		{
			if(!isEmptyStr(curLetter))
			{
				m_TGraph->Process(curLetter, &mMultiLetterNexts);
			}
			else
			{
				m_TGraph->Process(NULL, NULL);	
			}
			bProcessedTGraph = true;
		}

		UpdateMultiLetterNextWordsCase(curLetter, bSpace, complete);
	
		if(m_History.GetUpdateForStartWordsWithLetter())
			UpdateForStartWordsWithLetter(curLetter);

		if(m_History.GetUpdateAfterNumbers(curLetter))
			UpdateAfterNumbers(curLetter);

		if(bAllowTGraph && bProcessedTGraph) //only if P-Graph AND T-Graph have no predictions.
			m_TGraph->ProcessNouns(curLetter);

		m_TGraph->UpdateAutoCorrect(curLetter, &mMultiLetterNexts);
		m_TGraph->reset();
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
CompactNode* CDictManager::QuadRetrieveCompactNode(MYWCHAR* word, bool CopyActualWord, int *retDictIdx)
{
	if(isEmptyStr(word))
		return NULL;

	MYWCHAR actualWord[MAX_WORD_LEN];
	mywcscpy(actualWord, word);

	trimEndingSpaces(actualWord, true);
	ReplaceiToI(actualWord);

	int dictIdx=0;
	CompactNode* node = retrieveEndNodeForString(actualWord, &dictIdx, true); //1. Check all lower case.
	if(!node)
	{
		actualWord[0] = uprCharacter(actualWord[0]);
		node = retrieveEndNodeForString(actualWord, &dictIdx, true); //2. Check First upper case.
		if(!node && actualWord[1] != NUL)
		{
			MakeUpperCase(actualWord);
			node = retrieveEndNodeForString(actualWord, &dictIdx, true); //3. Check all upper case.
			if(!node )//iPad, iPod
			{
				MakeLowerCase(actualWord);
				actualWord[1] = uprCharacter(actualWord[1]);
				node = retrieveEndNodeForString(actualWord, &dictIdx, true); //4. Check special case such as iPad, iPhone. 
				if(!node)
				{
					return NULL;
				}
			}
		}
	}

	if(CopyActualWord && node)
	{
		mywcscpy(word, actualWord);
		*retDictIdx = dictIdx;
	}
	return node;
}
////////////////////////////////////////////////////////////////////////////////////////////
void CDictManager::UpdateAfterNumbers(MYWCHAR* curLetter)
{
	//ShowInfo("MK UpdateAfterNumbers history(%s), actual num(%d)\n",toA(m_History.NewCurrentHistory),m_History.nNumber );
	//ShowInfo("MK UpdateAfterNumbers curLetter:(%s)\n",toA(curLetter));

	MultiLetterAdvanceAr tempArr;
	tempArr.reset();

	bool bAllowedAll = false;
	if(m_History.nNumber > 0 && m_History.nNumber < 13)
		bAllowedAll = true;
	

	USHORT  basePref = calcCascadingBasePreference(1, eOrdinary, 0);
	int addPref = MAX_AFTER_NUMBER_SIZE;

	MYWCHAR* pWord =  NULL;
	for(int i =0; i < MAX_AFTER_NUMBER_SIZE;i++)
	{
		pWord = toW(AfterNumbersArr[i]);
		int n = mywcslen(pWord);

		int curLen = mywcslen(curLetter);

		bool bSame = true;
		for(int p=0; p < curLen; p++)
		{
			if(p > n || curLetter[p] != pWord[p] )
			{
				bSame = false;
				break;
			}
		}

		if(bSame)
		{
			for(int k=0; k < NMWORDPREDICTIONS;k++)
			{
				if(isEmptyStr(tempArr.nextWords[k]))
				{
					if(bAllowedAll)
					{
						mywcscpy(tempArr.nextWords[k],pWord);
						tempArr.prefs[k] = basePref+addPref;
						addPref--;
						break;
					}
					else
					{
						if(i >  2)
						{
							mywcscpy(tempArr.nextWords[k],pWord);
							tempArr.prefs[k] = basePref+addPref;
							addPref--;
							break;
						}
						else
						{
							break;
						}
					}
				}
			}
		}
	}

	tempArr.nBackSpace = mMultiLetterNexts.nBackSpace;

	for(int i=0; i < NMWORDPREDICTIONS; i++)
	{
		if(isEmptyStr(mMultiLetterNexts.nextWords[i]))
			break;
		
		if(!IsDuplicate(&tempArr, mMultiLetterNexts.nextWords[i]))
		{
			for(int k=0; k <NMWORDPREDICTIONS;k++)
			{
				if(isEmptyStr(tempArr.nextWords[k]))
				{
					mywcscpy(tempArr.nextWords[k], mMultiLetterNexts.nextWords[i]);
					tempArr.prefs[k] = mMultiLetterNexts.prefs[i];
					break;
				}
			}
		}	
	}

	ConvertWordArr2Chunks(&tempArr);
	mMultiLetterNexts.reset();
	mMultiLetterNexts = tempArr;
	mMultiLetterNexts.setActualNexts();
}

////////////////////////////////////////////////////////////////////////////////////////////
void CDictManager::UpdateMultiLetterNextWordsCase(MYWCHAR *curLetter, bool bSpace, bool complete)
{
	if(mTopDictionary->GetDictLanguage() == eLang_KOREAN)
	{
		return;
	}
	
	
	if((!curLetter && !bSpace && !m_History.bChangedCurrentHistory) || //For backspacing
		(!curLetter && !bSpace && m_History.bStartedUpperCaseLetter && m_History.bChangedCurrentHistory && !complete)) //AdvaceWord with holding down
	{
		//ShowInfo("MK UpdateMultiLetterNextWordsCase curLetter1:(%s)\n",toA(curLetter));

		//ShowInfo("MK UpdateMultiLetterNextWordsCase1 History:(%s)\n",toA(m_History.NewCurrentHistory));
		MYWCHAR* temp = GetLastWordFromPhrase(m_History.NewCurrentHistory);

		if(!isEmptyStr(temp) && (isUpperCase(temp[0]) || m_History.bStartedUpperCaseLetter))
		{
			for(int i=0; i < NMWORDPREDICTIONS;i++)
			{
				if(!isEmptyStr(mMultiLetterNexts.nextWords[i]))
				{			
					mMultiLetterNexts.nextWords[i][0] = uprCharacter(mMultiLetterNexts.nextWords[i][0]);
					//ShowInfo("MK UpdateMultiLetterNextWordsCase1:%s\n",toA(mMultiLetterNexts.nextWords[i]));
				}
			}
		}
	}
	else if(!isEmptyStr(curLetter) && isUpperCase(curLetter[0]))
	{
		for(int i=0; i < NMWORDPREDICTIONS;i++)
		{
			if(!isEmptyStr(mMultiLetterNexts.nextWords[i]))
			{		
				mMultiLetterNexts.nextWords[i][0] = uprCharacter(mMultiLetterNexts.nextWords[i][0]);
				//ShowInfo("MK UpdateMultiLetterNextWordsCase2:%s\n",toA(mMultiLetterNexts.nextWords[i]));
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////
void CDictManager::UpdateForEmails()
{
	mMultiLetterNexts.reset();

	USHORT  basePref = calcCascadingBasePreference(1, eOrdinary, 0);
	MYWCHAR* emails = NULL;
	
	int addPref = LEARNED_START_PREF;
	for(int i=0; i < MAX_EMAIL_SIZE;i++)
	{
		emails = toW(EmailArr[i]);
		mywcscpy(mMultiLetterNexts.nextWords[i], emails);
		mMultiLetterNexts.prefs[i] = basePref+ addPref;
		addPref--;
	}

	mMultiLetterNexts.setActualNexts();
}

////////////////////////////////////////////////////////////////////////////////////////////
void CDictManager::UpdateForStartWordsWithLetter(MYWCHAR* curLetter)
{
	//ShowInfo("MK UpdateForStartWordsWithLetter1:(%s)\n", toA(curLetter));
	if(!curLetter)
		return;

	USHORT  basePref = calcCascadingBasePreference(1, eOrdinary, 0);

	MultiLetterAdvanceAr tempArr;
	tempArr.reset();

	MYWCHAR cmpWord[MAX_WORD_LEN];
	mywcscpy(cmpWord, curLetter);
	trimEndingSpaces(cmpWord, true);

	int arrIndex =0;
	//Since we built starting words with all lowercase, an input letter should be lowercase to get startWords from Phrase Engine. 
	PhraseNode* pStartWords = gPhraseEngine->getStartWords(cmpWord);
	//ShowInfo("MK UpdateForStartWordsWithLetter2:(%s)\n", toA(curLetter));
	if(!pStartWords)
		return;

	bool startUp = isUpperCase(curLetter[0]);

	int count =0;
	while(pStartWords && (arrIndex < NMWORDPREDICTIONS))
	{
		int len = 0;
		MYWCHAR *startWord = (pStartWords->getStr(len))[0];
		if(startWord==NULL)
			break;
		if(mywcscmp(cmpWord, startWord) == 0)
		{
			pStartWords++;
			continue;
		}
		count++;
		if(startUp)
			startWord[0] =uprCharacter(startWord[0]);
	
		int curLen = mywcslen(curLetter);
		int n = mywcslen(startWord);
		bool bSame = true;
		for(int p=0; p < curLen; p++)
		{
			if(p > n || curLetter[p] != startWord[p] )
			{
				bSame = false;
				break;
			}
		}

		if(bSame && arrIndex < NMWORDPREDICTIONS && !IsDuplicate(&tempArr, startWord))
		{
			//ShowInfo("MK UpdateForStartWordsWithLetter startWord:(%s)\n", toA(startWord));
			mywcscpy(tempArr.nextWords[arrIndex], startWord);
			tempArr.prefs[arrIndex] = basePref + pStartWords->pref;
			arrIndex++;
		}
		pStartWords++;
	}

	ReplaceStartWordCache(&tempArr, basePref, curLetter);

	if(arrIndex < NMWORDPREDICTIONS)
	{
		for(int i=0; i <NMWORDPREDICTIONS; i++)
		{
			if(isEmptyStr(tempArr.nextWords[i]))
			{
				for(int k=0; k < NMWORDPREDICTIONS; k++)
				{
					MYWCHAR* text = mMultiLetterNexts.nextWords[k];
					if(!isEmptyStr(text) && !IsDuplicate(&tempArr, text))
					{
						mywcscpy(tempArr.nextWords[i], text);
						tempArr.prefs[i] = mMultiLetterNexts.prefs[k];
						break;
					}
				}
			}
		}
	}

	tempArr.nBackSpace  = mMultiLetterNexts.nBackSpace;
//	ConvertWordArr2Chunks(&tempArr); // likely not needed anymore! just supposed to trim end spaces!
	mMultiLetterNexts = tempArr;
	mMultiLetterNexts.setActualNexts();

	//ShowInfo("MK UpdateForStartWordsWithLetter3:(%s)\n", toA(curLetter));
}

////////////////////////////////////////////////////////////////////////////////////////////
MultiLetterAdvanceAr * CDictManager::UpdateForStartWords()
{
	ShowInfo(" CDictManager::UpdateForStartWords\n");
	static MultiLetterAdvanceAr tempArr;
	tempArr.reset();
	//ShowInfo("--1");
	WLBreakIf( getTopDictionary() == NULL, "!!ERROR! CDictManager::UpdateForStartWords: why topdictionary not set!!\n");
	//ShowInfo("--2=%p", gWordProp);

	if(gWordProp->nNullMoves > 1) // check this condition is strict enough. We shouldn't get blank starting words where already a word is under construction!
		return NULL;
	//ShowInfo("--3");
	PhraseNode* pStartWords = gPhraseEngine->getStartWords(NULL);
	if(!pStartWords)
		return &tempArr;

	USHORT  basePref = calcCascadingBasePreference(1, eOrdinary, 0);
	int arrIndex = 0;
	//ShowInfo("--4");

	while(pStartWords)
	{
		if(arrIndex >= NMWORDPREDICTIONS)
			break;

		int len = 0;
		MYWCHAR *startWord = (pStartWords->getStr(len))[0]; 
		//ShowInfo("--startWord=(%s)", toA(startWord));

		if(startWord==NULL)
			break;

		startWord[0] = uprCharacter(startWord[0]);
		if(!IsDuplicate(&tempArr, startWord))
		{
			mywcscpy(tempArr.nextWords[arrIndex], startWord);
			tempArr.prefs[arrIndex] = basePref + pStartWords->pref;
			arrIndex++;
		}
		pStartWords++; 
	}

	ReplaceStartWordCache(&tempArr, basePref, NULL);

	if(arrIndex > 0 )
	{
		ConvertWordArr2Chunks(&tempArr);
		tempArr.nBackSpace  = mMultiLetterNexts.nBackSpace;
		mMultiLetterNexts.reset();
		mMultiLetterNexts = tempArr;
		mMultiLetterNexts.setActualNexts();
	}

	return &mMultiLetterNexts;
}
////////////////////////////////////////////////////////////////////////////////////////////
void CDictManager::ReplaceStartWordCache(MultiLetterAdvanceAr*arr, USHORT basePref, MYWCHAR* curLetter)
{
	WLBreakIf(mTopDictionary==NULL, "!!ERROR! CDictManager::ReplaceStartWordCache: topdictionary is null!!\n");
	StartWordCache* pStartCache = m_NGramLearning->GetStartWordCache();
	if(!pStartCache)
		return;

	MYWCHAR curWord[5][MAX_WORD_LEN];
	USHORT cachePref[5];
	int count =0;
	CompactNode** ppNode = pStartCache->getTop5StatWords(curLetter);
	while(ppNode[count] && count < 5)
	{
		int len = mTopDictionary->getCompactStore()->retrieveWordFromLastNode(ppNode[count],curWord[count]);
		if(len > 0)
		{
			cachePref[count] = basePref+ppNode[count]->pref;
			//ShowInfo("MK CompareStartWordCache cache:%s, pref:%hu\n", toA(curWord[count]), cachePref[count] );
			count++;
		}
	}
		
	for(int i=0; i < count; i++)
	{
		for(int k= 0; k < NMWORDPREDICTIONS; k++)
		{
			if(isEmptyStr(arr->nextWords[k]))
				break;

			if(cachePref[i] > arr->prefs[k] && !IsDuplicate(arr, curWord[i]))
			{
				mywcscpy(arr->nextWords[k], curWord[i]);
				arr->prefs[k] = cachePref[i];
				if(!curLetter)
				{
					arr->nextWords[k][0] = uprCharacter(arr->nextWords[k][0]);
				}
				else
				{
					if(isUpperCase(curLetter[0]))
						arr->nextWords[k][0] = uprCharacter(arr->nextWords[k][0]);
				}
				
				//ShowInfo("MK CompareStartWordCache replaced:%s, pref:%hu\n", toA(arr->nextWords[k]), arr->prefs[k] );
				break;
			}
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////
// Assumes arr words have not been augmented by Periods or Spaces for chunking!
bool CDictManager::IsDuplicate(MultiLetterAdvanceAr* arr, MYWCHAR* startWord)
{
	MYWCHAR temp[MAX_WORD_LEN];
	MYWCHAR temp2[MAX_WORD_LEN];

	for(int k=0; k <NMWORDPREDICTIONS; k++)
	{
		if(isEmptyStr(arr->nextWords[k]))
			break;
		
		//Arr could have upper case or ending space or ending dots and startWord has nothing at the end. 
		//To compare, make sure these words have the same conditions without modifying original words.
		mywcscpy(temp, arr->nextWords[k]);
		trimEndingSpaces(temp, true);

		mywcscpy(temp2,startWord);
		trimEndingSpaces(temp2, true);
		if(mywcscmp(temp2,temp)==0)
			return true;	
	}

	return false;
}
/////////////////////////////////////////////////////////////////////////////////////////////
int CDictManager::ConvertWordArr2Chunks(MultiLetterAdvanceAr *arr)
{
	int index = 0;
	for(int k=0; k <NMWORDPREDICTIONS; k++)
	{
		if(!isEmptyStr(arr->nextWords[k]))
		{
			ConvertWordForChunk(arr->nextWords[index++]);
		}
	}
	return index;
}

////////////////////////////////////////////////////////////////////////////////////////////
void CDictManager::ConvertWordForChunk(MYWCHAR* wordInput)
{
	trimEndingSpaces(wordInput);
//	mywcscat(wordInput, sSpaces); // removing spaces at the end, just the same as we removed ... already! 
								 // If needed for display, add it on the app side! not here!
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
PhraseAr * CDictManager::ProcessPhrasePrediction(MYWCHAR *inputWord, bool bFromBackSpacing, MYWCHAR* nextLetter)
{
	//Testing purpose: DO NOT DELETE!!
	//ShowInfo("MK ProcessPhrasePrediction history1:(%s)\n",toA(m_History.NewCurrentHistory));
	//ShowInfo("MK ProcessPhrasePrediction history2:(%s)\n",toA(m_History.HistoryForPhrase));
	//ShowInfo("MK ProcessPhrasePrediction history3:(%s)\n",toA(m_History.BackSpaceHistory));
	//ShowInfo("MK ProcessPhrasePrediction inputWord:(%s)\n",toA(inputWord));
	//ShowInfo("MK ProcessPhrasePrediction nextLetter:(%s)\n",toA(nextLetter));

	mPhraseAr.reset();
	//return &mPhraseAr;

	ProcessPhrase processPhrase;
	Dictionary* dict = getTopDictionary();

	if(!dict || dict->GetDictLanguage() == eLang_KOREAN || (!inputWord && !nextLetter) || 
		bFromBackSpacing || !processPhrase.InitProcessPhrase(dict, &m_History, m_NGramLearning))
	{
		return &mPhraseAr;	
	}

	MYWCHAR *pHist = m_History.GetHistoryForPhrasePredictions(inputWord, nextLetter);
	//pHist = makeWord(L"I");
	//ShowInfo("MK ProcessPhrasePrediction pHist:(%s)\n",toA(pHist));

	bool bEndSpace = HasEndingSpace(pHist);
	bool bCompare = true;
	MYWCHAR newInput[MAX_WORD_LEN];
	if(bEndSpace)
	{
		mywcscpy(newInput, GetLastWordFromPhrase(pHist));
	}
	else
	{
		MYWCHAR* lastWord = GetLastWordFromPhrase(pHist);
		int dictIdx = 0;
		CompactNode* cNode = QuadRetrieveCompactNode(lastWord, true, &dictIdx);
		if(cNode && mywcscmp(lastWord, toW("I")) != 0)
		{
			mywcscpy(newInput, lastWord);
			bCompare = false;
		}
		else
			mywcscpy(newInput, m_History.GetNewWordInputForPhrase(pHist));
		
	}
	
	//ShowInfo("MK ProcessPhrasePrediction newInput:(%s)\n",toA(newInput));
	trimEndingSpaces(newInput, true);

	if(isEmptyStr(newInput))
		return &mPhraseAr;	
	
	//Minkyu:2013.11.27
	//DO NOT predict if you face punctuation when backspacing.
	int n = mywcslen(newInput);
	for(int i=n-1; i >=0; i--)
	{
		if(isPunctuation(newInput[i]))
			return &mPhraseAr;
	}
	/////////////////////////////////////////////////////////

	ReplaceiToI(newInput);

	int dictIdx =0;
	CompactNode* firstEndnode = retrieveEndNodeForString(newInput, &dictIdx, true);
	if(!firstEndnode)
		return &mPhraseAr;

	int nextStart =0;
	MYWCHAR* cmpWord = NULL;
	if(!bEndSpace && bCompare)
		cmpWord = GetLastWordFromPhrase(pHist);

	//1. Find Learned Phrases.
	if(processPhrase.ProcessLearnedPhrase(&mPhraseAr, newInput, nextLetter, firstEndnode, &nextStart, cmpWord))
		PrintPhrases(&mPhraseAr, 0, (char*)"Learned");
	
	if(nextStart == MAX_PHRASE_ALLOWED)
	{
		mPhraseAr.setActualNexts();
		return &mPhraseAr;
	}

	//2. Find P2P Phrases.
	//ShowInfo("MK ProcessPhrasePrediction pHist for P2P:(%s)\n",toA(pHist));
	int temp = nextStart;
	m_History.bProcessedP2P= processPhrase.ProcessP2P(&mPhraseAr, pHist, &nextStart);
	if(m_History.bProcessedP2P)
		PrintPhrases(&mPhraseAr, temp, (char*)"P2P");
	
	if(nextStart == MAX_PHRASE_ALLOWED)
	{
		mPhraseAr.setActualNexts();
		return &mPhraseAr;
	}

	//3. Find Normal Phrases.
	temp = nextStart;
	processPhrase.ProcessNormalPrediction(&mPhraseAr, newInput, firstEndnode, nextLetter, &nextStart, cmpWord);
	PrintPhrases(&mPhraseAr, temp, (char*)"Normal");
	

	//4. Show total found phrases.
	PrintPhrases(&mPhraseAr, 0, (char*)"Final");

	mPhraseAr.setActualNexts();
	return &mPhraseAr;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDictManager::PrintPhrases(PhraseAr* arr, int startPos, char* szType)
{
	//return;

	ShowInfo("\n\n");	
	for(int i=startPos; i <MAX_PHRASE_ALLOWED;i++)
	{
		if(!isEmptyStr(mPhraseAr.phrases[i]))
			ShowInfo("MK ProcessPhrasePrediction %s Phrase: *********%s(%hu)*********\n", szType, toA(mPhraseAr.phrases[i]), mPhraseAr.prefs[i]);	
	}

}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MultiLetterAdvanceAr::reset()
{
	for(int i=0; i< NMWORDPREDICTIONS; i++)
	{
		prefs[i] = INVALID_PREFERENCE;
		nextWords[i][0] = NUL;
		nShouldUpperCase[i] = 0;
		bCorrection[i] = false;
	}
	nActualNexts = 0;
	nBackSpace = 0;	
	nCorrections = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////
void MultiLetterAdvanceAr::setActualNexts()
{
	nActualNexts = 0;
	for(int i=0; i< NMWORDPREDICTIONS; i++)
	{
		if((prefs[i] != INVALID_PREFERENCE && prefs[i] > 0))
		{
			nActualNexts++;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////
void MultiLetterAdvanceAr::set(int num, MYWCHAR **nexts)
{
	reset();
	if(num<= 0)
		return;
	nActualNexts = num;
	for(int i=0; i<num; i++)
	{
		prefs[i] = num-i;
		WLBreakIf(nexts[i]==NULL, "!!ERROR! MultiLetterAdvanceAr::set: empty string at %d\n", i);
		mywcscpy(nextWords[i], nexts[i]);
	}
}
///////////////////////////////////////////////////////////////////////////////
void MultiLetterAdvanceAr::add(MYWCHAR *word, USHORT pref)
{
	int lowestIdex = 0;
	for(int i=0; i< NMWORDPREDICTIONS; i++)
	{
		if(prefs[i]==INVALID_PREFERENCE)
		{
			lowestIdex = i;
			break;
		}
		else if(prefs[i] < prefs[lowestIdex])
			lowestIdex = i;
	}
			
	prefs[lowestIdex] = pref;
	mywcscpy(nextWords[lowestIdex], word);
}
/////////////////////////////////////////////////////////////////
void PhraseAr::reset()
{
	for(int i=0; i< MAX_PHRASE_ALLOWED; i++)
	{
		prefs[i] = INVALID_PREFERENCE;
		phrases[i][0] = NUL;
	}
	count = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////
void PhraseAr::setActualNexts()
{
	count = 0;
	for(int i=0; i< MAX_PHRASE_ALLOWED; i++)
	{
		if((prefs[i] != INVALID_PREFERENCE && prefs[i] > 0))
		{
			count++;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////
void PhraseAr::set(int num, MYWCHAR **nexts)
{
	reset();
	if(num<= 0)
		return;
	count = num;
	for(int i=0; i<num; i++)
	{
		prefs[i] = num-i;
		WLBreakIf(nexts[i]==NULL, "!!ERROR! MultiLetterAdvanceAr::set: empty string at %d\n", i);
		mywcscpy(phrases[i], nexts[i]);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CDictManager::partOfWordsInAnyDictionaries()
{
	for (int i = 0 ; i < m_nOrderedDicts; i++)
	{
		Dictionary *existDict = getDictionaryEnabled(i);
		if (existDict) 
		{
			if (existDict->partOfOtherWords(FALSE))
				return TRUE;
		}
	}
	return FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CDictManager::partOfWordsInADictionary(int start, int end, int exceptIdx, BOOL wordByItself) 
{
	if (gWordProp->nNullMoves)
		return FALSE;

	for (int i = start ; i < end; i++) 
	{
		if (i == exceptIdx)
			continue;
		Dictionary *existDict = getDictionaryEnabled(i);
		if (existDict) 
		{
			if (existDict->partOfOtherWords(wordByItself))
				return TRUE;
		}
	}
	return FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CDictManager::advanceWord(MYWCHAR *wordPart, MYWCHAR **printPart, MultiLetterAdvanceAr *&nextwordsAr, bool wordComplete, bool replace) 
{
	//This should be fixed from Java side, but temporarily fix here.
	//Type "Bit" then select "Bite" from the very first word, it returns input word with 6 ending spaces for some reason...ONLY in the very first word.
	int nEndSpace = GetSpaceCount(wordPart);
	if(nEndSpace > 3) 
	{
		int endSP =0;
		for(int i=0; i < MAX_WORD_LEN && wordPart[i]; i++)
		{
			if(wordPart[i] == SP)
				endSP++;		
			if(endSP > 3)
				wordPart[i] = NUL;
		}
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//MYWCHAR* pp = makeWord(L"please contact me ");
	//mywcscpy(m_History.NewCurrentHistory, pp);
	//ShowInfo("MK advanceWord Input:(%s), Ending Space:%d\n",toA(wordPart), nEndSpace);
	//ShowInfo("MK advanceWord history1:(%s)\n",toA(m_History.NewCurrentHistory));
	//ShowInfo("MK advanceWord history2:(%s)\n",toA(m_History.HistoryForPhrase));
	//ShowInfo("MK advanceWord history3:(%s)\n",toA(m_History.BackSpaceHistory));
	//ShowInfo("MK advanceWord wordComplete:(%d), replace:(%d)\n",wordComplete, replace);


	MYWCHAR originalWordPart[MAX_WORD_LEN];
	mywcscpy(originalWordPart, wordPart);
	if(wordComplete && replace)
	{
		m_History.bReplaced = true;
		MYWCHAR* prevWord = DeleteStartingWords(wordPart);
		if(!prevWord)
			prevWord = GetLastWordFromPhrase(m_History.NewCurrentHistory);

		if(!isEmptyStr(prevWord) && !isEmptyStr(wordPart))
		{
			MYWCHAR replaceWord[MAX_WORD_LEN];
			mywcscpy(replaceWord, wordPart);
			MakeLowerCase(replaceWord);
			MakeLowerCase(prevWord);
			ShowInfo("MK advanceWord prevWord:(%s), replaceWord:(%s)\n", toA(prevWord), toA(replaceWord));
			mUserWordCache->replace(prevWord, replaceWord);	

		}
		//ShowInfo("MK advanceWord history2:(%s)\n",toA(m_History.NewCurrentHistory));	
	}
	m_History.bStartingWordMode = false;
	m_History.bUpdateAfterNumbers = false;
	m_History.bAdvanceWordComplete = wordComplete;

	if(isUpperCase(wordPart[0]))
		m_History.bStartedUpperCaseWord = true;
	
	MYWCHAR* p = GetLastWordFromPhrase(m_History.NewCurrentHistory);

	//Minkyu:2013.11.27
	//DO NOT predict if you face punctuation when backspacing.
	if(!m_History.IsPredictionAllowed(wordPart, p))
	{
		//ShowInfo("MK advanceWord puncCount=(%s)\n",toA(wordPart));
		mMultiLetterNexts.reset();
		nextwordsAr = &mMultiLetterNexts;
		return sNumBackouts;
	}

	m_History.bEmailForAdvanceWord = false;
	wordPart = m_History.CheckConvertedEmail(wordPart);
	
	///////////////////////////////

	static MYWCHAR s_printPart[MAX_PHRASE_LEN];
	mMultiLetterNexts.reset();
	nextwordsAr = &mMultiLetterNexts;
	BYTE numOfLettersBacout = sNumBackouts;
	if (isEmptyStr(wordPart ) || m_noDictionariesLoaded) 
	{
		ShowInfo("!!!WARNING!!advanceWord! wordPart == 0 or m_noDictionariesLoaded! do nothing!!\n");
		sNumBackouts = 0;
		goToNextWord((char*)"advanceWord wordPart=0", false);
		if(!wordComplete && !replace && isEmptyStr(wordPart))
		{
			int leng = mywcslen(p);
			if(p[leng-1] == DOT)
				UpdateForStartWords();
		}
		return numOfLettersBacout;
	}

	memset(s_printPart, 0 , sizeof(s_printPart));
	*printPart = s_printPart;

	//Minkyu:2013.11.21
	m_History.SetChangeCurrentHistory(wordPart,wordComplete,replace);
	

	//ShowInfo("MK advanceWord wordComplete(%d),replace(%d)\n",wordComplete,replace);
	return mngrAdvanceWordC(wordPart, printPart, nextwordsAr, wordComplete, replace, originalWordPart);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// advances a selected word as next word and modifies word histories and word path to reflect this choice 
// and finds next possible words.
BYTE CDictManager::mngrAdvanceWordC(MYWCHAR *wordPart, MYWCHAR **printPart, MultiLetterAdvanceAr *&nextwordsAr, 
									BOOL complete, bool replace, MYWCHAR* originalWordPart)
{

#ifdef TIMETEST
	clock_t start, end;
	start = clock();
#endif

	MYWCHAR truncatedWord[MAX_WORD_LEN];
	memset(truncatedWord, 0, sizeof(truncatedWord));
	MYWCHAR *truncWordFreeOfEVerb = truncatedWord;
	BYTE numOfLettersBacout = sNumBackouts;
	unsigned curlen = 0;
	ShowInfo("\nmngrAdvanceWordC: wordPart=#%s#, complete=%d, gHistWordIdx=%d, gWorkingWordIdx=%d, nPath=%d, curWord=#%s#, sNumBackouts=%d, replace=%d\n", 
				toA(wordPart), complete, gHistWordIdx, gWorkingWordIdx, gWordProp->nPathNodes, toA( getCurrentWord(curlen)), sNumBackouts, replace);
	
	bool currentWordEmpty = emptyWordPath();
	bool learn = true;
	if(replace)
	{
		if(currentWordEmpty)
		{
			BackToPreviousWord(true);
			mUserWordCache->replace( getCurrentWord(curlen), wordPart);
			learn = false;
		}
		resetWorkingWordPath();
		ShowInfo("--resetWordPath since replace flag set! gWorkingWordIdx=%d, nPath=%d\n", gWorkingWordIdx, gWordProp->nPathNodes);
	}

	BYTE len = mywcslen(wordPart);
	if(len==1 && SP_CR_TAB(wordPart[0]))
	{
		goToNextWord((char*)"mngrAdvanceWordC sp_cr_tab_lf", false);
		int numNextWords = 0;
		MYWCHAR rootWord[MAX_WORD_LEN];
		GetNextWords(&numNextWords, rootWord, true); 
		updateMultiLetterNextWords(1, NULL, complete, replace); 
		mMultiLetterNexts.setActualNexts();
		nextwordsAr = &mMultiLetterNexts;
		*printPart[0] = wordPart[0];
		return 0;
	}

	SearchResultEntry *srep = findSearchEntryWithLowestDictIdInCache(wordPart, &len);
	ShowInfo("--srep: len=%d, srep.predText=#%s#, srep.text=#%s#, predType=%d\n", len, toA(srep->predText), toA(srep->text), srep->predType);
	
	//Minkyu:2014.05.01
	//srep has really weird result for srep->predText for backspaing, which will call advanceWord or advanceWordComplete from Java.
	//For advanceWord, current word should be used to find next words.
	mywcsncpy(truncatedWord, wordPart, len);
	//mywcsncpy(truncatedWord, srep->predText, srep->resultLen);

	unsigned count = 0;
	if (srep->eVerbRetrievalActive) 
	{
		ShowInfo("--everbRetrievalActive, %d\n", srep->eVerbPrefixLen);
		// turn this into a higher level backspace letter
		gWordProp->nEVerbUndoMoves = gWordProp->nUndoMoves[gWordProp->nPathNodes];
		for (int i = 0; i < srep->eVerbPrefixLen; i++)
			mngrInternalBackSpaceLetterDictionaries( FALSE);
		gWordProp->nEVerbChars = gWordProp->nPathNodes;
	} 
	else if (PWresult.areEVerbsSearchResultsFound && srep->eVerbPrefixLenQuickList > 0)
	{
		// the text was adjusted because of EVerb findings on other results. get the searchresult text back in line before we advance it 
		truncWordFreeOfEVerb = &truncatedWord[srep->eVerbPrefixLenQuickList];
		//truncatedWordLen -= srep->eVerbPrefixLenQuickList;
	} 
	
	// add SP to current word's end if needed, before advancing next word!
	if(gWordProp->nPathNodes>0 && gWordProp->nSP==0 && (srep->predType==eDictNextPredict || srep->predType==eLearnNextPredict))
	{
		putLetterInWordpath(SP, 0);
		(*printPart)[count++] = SP;	
		numOfLettersBacout = 0;
	}

	// now advance to nextword in working set of NCURWORDS words:
	ShowInfo("--advance on all letters of #%s#, numOfLettersBacout=%d\n", toA(truncWordFreeOfEVerb), numOfLettersBacout);
	if(!emptyWordPath())
	{
		ShowInfoIf(truncWordFreeOfEVerb[0]==SP, "!!WARNING! mngrAdvanceWordC: why current word #%s# start with SP?!\n", toA(truncWordFreeOfEVerb));
		if(numOfLettersBacout>0 && srep->predType < eDictNextPredict)
		{
			resetAllWorkingWordPaths();
		}
		else
		{
			WLBreakIf(*gWordProp->charsBufP!=SP, "!!ERROR! mngrAdvanceWordC: this should be a next word, so currword=#%s# should have finished by SP?\n", toA(getCurrentWord(curlen)) );
			goToNextWord((char*)"mngrAdvanceceWordC_CUR_WORD_NOT_EMPTY!!", false, false);
		}
	}

	// now fill in the advance word:
	ShowInfo("---advance at gWorkingWordIdx(%d), gHistWordIdx(%d) on word #%s#:\n", gWorkingWordIdx, gHistWordIdx, toA(truncWordFreeOfEVerb));
	BYTE ret = 0;
	for(int k=0; truncWordFreeOfEVerb[k] != NUL; k++) 
	{
		ShowInfoIf(truncWordFreeOfEVerb[k]==SP, "!!WARNING! mngrAdvanceWordC: why there is a SP in this word at location %d!\n", k);
		if(truncWordFreeOfEVerb[k]==SP)
		{
			putLetterInWordpath(SP, 0);
			(*printPart)[count++] = SP;
			goToNextWord((char*)"mngrAdvanceceWordC_SP_IN_MIDDLE!!", false, learn);
			continue;
		}
		if(gWordProp->nNullMoves == 0)
			ret = mngrFillNextNodeDictionaries(truncWordFreeOfEVerb[k], ePredicting);

	//	ShowInfo("---advance on word #%s# at char %c, ret=%d, nPathNodes=%d\n", toA(truncWordFreeOfEVerb), char2A(truncWordFreeOfEVerb[k]), ret, gWordProp->nPathNodes);
		(*printPart)[count++] = truncWordFreeOfEVerb[k];
		putLetterInWordpath(truncWordFreeOfEVerb[k], ret);
		gWordProp->nUndoMoves[gWordProp->nPathNodes+gWordProp->nNullMoves] = (BYTE)(k + 1);	
	}
	
	// now take care of end_of_word space:
	MYWCHAR rootWord[MAX_WORD_LEN];
	memset(rootWord,0,sizeof(rootWord));
	if(complete)// || !srep->isChunk|| (srep->dictIdx==0xff && !isChunk(gWordProp->nPathNodes))) // last condition is for case of multiletter advance, since we don't have the prediction in SearchResultEntry! 
	{
		ShowInfo("--mngrAdvanceceWordC advanced SP since it's Complete!\n");
		putLetterInWordpath(SP, 0);
		(*printPart)[count++] = SP;
		sNumBackouts = 0;
		goToNextWord((char*)"mngrAdvanceceWordC_Complete", false, learn);
	}
	else
	{
		sNumBackouts = gWordProp->nPathNodes;
	}
	int nNextWords = 0;
	GetNextWords(&nNextWords, rootWord, true); 

	//ShowInfo("MK mngrAdvanceWord rootWord:(%s), original(%s)\n", toA(rootWord), toA(originalWordPart));

	//ShowInfo("MK mngrAdvanceWord complete:(%d), replace(%d)\n", complete, replace);
	if(!complete && !replace) //For advanceWord from Java when backspacing.
		m_History.SetStartingWordMode(true);
	

	updateMultiLetterNextWords(1, rootWord, complete, replace); //adjust multiletter next word array based on this nextwords:
	if(!complete && !replace && !isEmptyStr(originalWordPart))//After backspacing!
	{
		MYWCHAR input[MAX_WORD_LEN];
		input[0] = NUL;
		mywcscpy(input, originalWordPart);
		m_TGraph->UpdateAutoCorrect(input, &mMultiLetterNexts);
	}
	
	mMultiLetterNexts.setActualNexts(); 
	ShowInfo("\n");
	for(int i=0; i<mMultiLetterNexts.nActualNexts; i++)
	{
		if((isUpperCase(originalWordPart[0]) && !complete && !replace) 
			|| (GetSpaceCount(mMultiLetterNexts.nextWords[i]) == 3 && mywcslen(mMultiLetterNexts.nextWords[i]) == 4 
			&& mMultiLetterNexts.nextWords[i][0] == 'i'))//When advanceWord being called for backspacing..check 
		{
			mMultiLetterNexts.nextWords[i][0] = uprCharacter(mMultiLetterNexts.nextWords[i][0]);
		}

		
		if(mMultiLetterNexts.nextWords[i][0] == 'i' && mMultiLetterNexts.nextWords[i][1] == APOSTROPHE)
			//mMultiLetterNexts.nextWords[i][0] = uprCharacter(mMultiLetterNexts.nextWords[i][0]);
			ReplaceiToI(mMultiLetterNexts.nextWords[i]);


		if(mMultiLetterNexts.nShouldUpperCase[i] > 0)
		{
			mMultiLetterNexts.nextWords[i][0] = uprCharacter(mMultiLetterNexts.nextWords[i][0]);
		}
		ShowInfo("MK mngrAdvanceWord actual nextwordsAr: %d: (%s)\n", i, toA(mMultiLetterNexts.nextWords[i]));
	}

	printPart = ProcessingBackoutNum(wordPart, originalWordPart, printPart, &numOfLettersBacout);

	// now do autocorrect:
	//DoAutoCorrect(wordPart);

	m_History.ResetAfterAdvanceWord();
	LearnStartingWords();

#ifdef TIMETEST
	end = clock();
	double diff = Diffclock(end, start);
	ShowInfo("Time AdvaneWord prev History:(%s)-->%f\n", toA(m_History.NewCurrentHistory), diff);
#endif
	nextwordsAr = &mMultiLetterNexts;
	return numOfLettersBacout;
}
////////////////////////////////////////////////////////////////////////////////////////////
MYWCHAR** CDictManager::ProcessingBackoutNum(MYWCHAR* wordPart, MYWCHAR*originalWordPart, MYWCHAR **printPart, BYTE* numOfLettersBacout)
{
	//ShowInfo("MK ProcessingBackoutNum wordPart(%s), original(%s), printPart(%s), bProcessP2P(%d)\n", toA(wordPart), toA(originalWordPart), toA(*printPart), m_History.bProcessedP2P);
	if(printPart == NULL || isEmptyStr(*printPart))
		return printPart;

	MYWCHAR newPrint[MAX_PHRASE_LEN];
	mywcscpy(newPrint, *printPart);

	bool bSelectP2P = false;
	if(m_History.bProcessedP2P)
	{
		//ShowInfo("MK ProcessPhrasePrediction originalWordPart:(%s)\n", toA(originalWordPart));
		for(int i=0; i < MAX_PHRASE_ALLOWED; i++)
		{
			if(!HasEndingSpace(originalWordPart))
				trimEndingSpaces(m_History.PhraseP2P[i]);
			//ShowInfo("MK ProcessPhrasePrediction m_History.PhraseP2P[i]:(%s)\n", toA(m_History.PhraseP2P[i]));
			if(!isEmptyStr(m_History.PhraseP2P[i]) &&
				mywcscmp(m_History.PhraseP2P[i], originalWordPart) == 0 )
			{
				bSelectP2P = true;
				//ShowInfo("MK ProcessPhrasePrediction selectP2P:(%s)\n", toA(originalWordPart));
				break;
			}
		}
	}

	if(bSelectP2P)
	{
		ChangeUpperForI(originalWordPart,newPrint);
		ChangeSpecialUpperCase(newPrint, originalWordPart);
	
		if(m_History.IsZeroBackoutNum(wordPart))
		{
			*numOfLettersBacout = RecalculateBackoutNumForNumbers(wordPart, *numOfLettersBacout);
			*numOfLettersBacout = RecalculateBackoutNumForEmails(wordPart, *numOfLettersBacout);
		}
		else
		{
			*numOfLettersBacout = 0;
			MYWCHAR temp[MAX_PHRASE_LEN];
			memset(temp,0,sizeof(temp));
			temp[0] = SP;
			mywcscat(temp, newPrint);
			mywcscpy(newPrint, temp);

			//ShowInfo("MK ProcessingBackoutNum newPrint(%s)\n",toA(newPrint));
		}
	}
	else
	{
		//Minkyu:2013.11.15
		BYTE tempNum = RecalculateBackoutNum(wordPart);
		if(tempNum > 0)
			*numOfLettersBacout = tempNum;	
		
		*numOfLettersBacout = RecalculateBackoutNumForNumbers(wordPart, *numOfLettersBacout);
		*numOfLettersBacout = RecalculateBackoutNumForEmails(wordPart, *numOfLettersBacout);

		ChangeUpperForI(originalWordPart,newPrint);
		ChangeSpecialUpperCase(newPrint, originalWordPart);
	}

	if(GetSpaceCount(newPrint) > 1 )
	{
		for(int i=0; i < MAX_PHRASE_LEN && originalWordPart[i]; i++)
		{
			if(isUpperCase(originalWordPart[i]))
				newPrint[i] = uprCharacter(newPrint[i]);
		}
	}

	if(mywcscmp(*printPart, newPrint) != 0)
		mywcscpy(*printPart, newPrint);

	//ShowInfo("MK ProcessingBackoutNum return(%s)\n",toA(*printPart));
	return printPart;
}
////////////////////////////////////////////////////////////////////////////////////////////
void CDictManager::ChangeUpperForI(MYWCHAR* input, MYWCHAR*print)
{
	MYWCHAR temp[MAX_WORD_LEN];
	mywcscpy(temp,input);

	trimEndingSpaces(temp);

	if(mywcslen(temp) == 1 && (temp[0] =='I' || temp[0] == 'A'))
	{
		print[0] = temp[0];
		trimEndingSpaces(print);
		print[1] = SP;
	}
	if(m_History.bStartedUpperCaseWord)
		print[0] = uprCharacter(print[0]);	
	
	//EX) "What should I"
	for(int i=0; i < MAX_PHRASE_LEN && print[i];i++)
	{
		if(print[i] == 'i')
		{
			if((print[i+1] == NUL || print[i+1] == SP) && (print[i-1] == NUL || print[i-1] == SP))
				print[i] = uprCharacter(print[i]);	
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////
void CDictManager::ChangeSpecialUpperCase(MYWCHAR* input,MYWCHAR* original)
{
	//Ex: iPhone, CEO, iPad..
	//ShowInfo("MK ChangeSpecialUpperCase input:(%s), original:(%s)\n",toA(input),toA(original));
	for(int i=0; i < MAX_PHRASE_LEN && input[i] != NUL; i++)
	{
		if(input[i] == SP || isPunctuation(input[i]))
			break;
		
		if(isUpperCase(original[i]))
			input[i] = uprCharacter(input[i]);
		else
			input[i] = lwrCharacter(input[i]);	
	}
}
////////////////////////////////////////////////////////////////////////////////////////////
BYTE CDictManager::RecalculateBackoutNum(MYWCHAR* wordInput)
{
	//ShowInfo("MK IsZeroBackoutNum history:(%s)\n",toA(m_History.NewCurrentHistory));
	//ShowInfo("MK IsZeroBackoutNum history2:(%s)\n",toA(m_History.HistoryForPhrase));
	//ShowInfo("MK IsZeroBackoutNum wordInput:(%s)\n",toA(wordInput));

	if(m_History.bDeleteFirstWord)
		return 0;
	
	trimEndingSpaces(wordInput);
	int nSpaceCount =GetSpaceCount(wordInput);
	BYTE newBackoutCount =0;
	if(nSpaceCount == 0)//< MAX_TGRAPH_HISTORY_NUM-2)
	{
		return newBackoutCount;
	}
	int nCount =0;
	if(nSpaceCount >= 1)//MAX_TGRAPH_HISTORY_NUM-2)
	{
		for(int i=0; i <MAX_PHRASE_LEN && wordInput[i] != NUL; i++)
		{
			if(wordInput[i] != NUL && wordInput[i] != SP)
			{
				nCount++;
			}
			else
			{
				break;
			}
		}
	}
	int n = mywcslen(m_History.HistoryForPhrase);
	int temp =0;
	if(HasEndingSpace(m_History.HistoryForPhrase) && !isPunctuation(m_History.HistoryForPhrase[n-2]))
	{
		temp++;
	}

	newBackoutCount = (BYTE)(nCount+temp);
	if(m_History.nEraseLastWord > 0)
	{
		newBackoutCount++;
		m_History.nEraseLastWord = 0;
	}
	return newBackoutCount;
}

////////////////////////////////////////////////////////////////////////////////////////////
BYTE CDictManager::RecalculateBackoutNumForNumbers(MYWCHAR* wordInput, BYTE backoutNum)
{
	if(IsOrdinalNumbers(wordInput))
	{
		if(HasEndingSpace(m_History.HistoryForPhrase))
		{ 
			int numCount =0;
			for(int i=0; i <MAX_PHRASE_LEN && wordInput[i] != NUL; i++)
			{
				if(isNumberChar(wordInput[i]))
				{
					numCount++;
				}
			}
			numCount = numCount+1;
			backoutNum = backoutNum+numCount;
		}
	}

	return backoutNum;
}

////////////////////////////////////////////////////////////////////////////////////////////
BYTE CDictManager::RecalculateBackoutNumForEmails(MYWCHAR* wordInput, BYTE backoutNum)
{
	if(IsThisEmail(wordInput))
	{
		MYWCHAR* p = GetLastWordFromPhrase(m_History.HistoryForPhrase);
		int histLen = mywcslen(p);
		backoutNum = histLen;
		mMultiLetterNexts.reset();
	}

	return backoutNum;
}


////////////////////////////////////////////////////////////////////////////////////////////
void CDictManager::SetHistoryFromJNI(MYWCHAR *inputWord, bool backspace)
{
	if(m_History.SetHistory(inputWord, backspace))
		LearnMultiGramWord();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
MYWCHAR *getWordFrom(int pos, MYWCHAR letter)
{
	static MYWCHAR sWordFrom[MAX_WORD_LEN];
	int len = (int)(gWordProp->charsBufP - &gWordProp->charsBuf[pos]);
	mywcscpy(sWordFrom, &gWordProp->charsBuf[pos]);
	sWordFrom[len+1] = letter;
	sWordFrom[len+2] = NUL;
	return sWordFrom;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
MYWCHAR *getWordFromTo(int start, int end)
{
	static MYWCHAR sWordFromTo[MAX_WORD_LEN];
	mywcsncpy(sWordFromTo, &gWordProp->charsBuf[start], end-start);
	sWordFromTo[end-start] = NUL;
	return sWordFromTo;
}

////////////////////////////////////////////////////////////////////////////////////////
// Used for continuous typing. Purges a layer by walking along the extnodes in the wordpath 
// and deleting nodes belonging to this subword. Each subword is specified with its layer index.
void CDictManager::PurgeTopLayers(int topcontaininglayer)
{
	WLBreakIf(gWordProp->maxLayerId<1, "!!ERROR! InvalidateTopLayer! we can't invalidate base layer!\n");
	
	//now all layers from topcontaininglayer till gWordProp->maxLayerId must be invalidated!
	BYTE startpos = gWordProp->layerStartPos[topcontaininglayer+1];
	WLBreakIf(startpos==0, "!!ERROR! PurgeTopLayers: startpos==0!\n");
	ShowInfo("PurgeTopLayers: all layers from layer %d {starting from pos=%d on} going to be invalidated!\n", topcontaininglayer, startpos);

	int endnode = gWordProp->nPathNodes + 1;
	for(int i = startpos-1; i<=endnode; i++)
	{
		for (int j = 0; j < m_nOrderedDicts; j++) 
		{
			ExtPathNode *extPathNode = gWPathway[i].dictNode[j];
			if(extPathNode==EXT_NODE_NOTSET || extPathNode->nExtCNodes <= topcontaininglayer)
				continue;
			
			ExtCNode *extCNodes = extPathNode->extCNodes;
		    while(extCNodes[extPathNode->nExtCNodes-1].layer() > topcontaininglayer)
			{
				*((int*)(&extPathNode->extCNodes[extPathNode->nExtCNodes-1])) = 0;
				if(--extPathNode->nExtCNodes==0)
				{
					gWPathway[i].dictNode[j] = EXT_NODE_NOTSET;
					break;
				}
			}
		}
	}
	for(int k=topcontaininglayer; k< gWordProp->maxLayerId; k++)
	{
		gWordProp->layerStartPos[k+1] = 0;
		gWordProp->layerEndPos[k] = 0;
		gWordProp->layerEndPref[k] = 0;
	}
	gWordProp->maxLayerId = topcontaininglayer;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// for string ending at location loc in current wordpath, find the maximum end preference for layer specified by "layer".
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CDictManager::getWordPref(BYTE endPointInDicts, BYTE loc, BYTE layer)
{
	BYTE pref = 0;
	for (int j = 0; j < m_nOrderedDicts; j++)
	{
		if (isBitTurnedOn(endPointInDicts, j))
		{
			Dictionary *existDict = getDictionaryEnabled(j);
			WLBreakIf(!existDict || j!=existDict->GetDictIndex(), 
				"!!ERROR!! ? getTopWordPref::!existDict || j!=existDict->GetDictIndex() for existDict=%x endPointInDicts=%d and j=%d!\n", existDict, endPointInDicts, j);
			ExtPathNode *extPathNode = gWPathway[loc].dictNode[j];
			WLBreakIf(extPathNode == EXT_NODE_NOTSET, "!!ERROR! getTopWordPref: extPathNode == EXT_NODE_NOTSET!\n");
			for(int i=0; i<extPathNode->nExtCNodes; i++)
			{
				if(extPathNode->extCNodes[i].layer() == layer && existDict->getCompactStore()->isEndpoint(extPathNode->extCNodes[i].cNode))
					pref = max(pref, existDict->getEndPreference(extPathNode->extCNodes[i].cNode));
			}
		}
	}
	return pref;
}

///////////////////////////////////////////////////////////////////////////////////
// Used for continuous typing. Verifies if the currently ended subword is actually
// the top most current subword. If not the top subword is purged in current wordpath,
// and original one is duplicated as an alternate path.
bool CDictManager::UpdateWordLayers(USHORT layersEnding, USHORT containingLayers, BYTE endInDicts)
{
	bool toplayerEnds = true;
	if(gWordProp->maxLayerId > 0 && layersEnding) // first check if this new layer is going to invalidate any previous layer
	{
		if(!isBitTurnedOn16(layersEnding, gWordProp->maxLayerId)) // top layer is not the one ending the word. Take care of it!
		{  
			toplayerEnds = false;
			int layerId = gWordProp->maxLayerId-1;
			while(layerId >=0) 
			{
				BYTE wordPref = getWordPref(endInDicts, gWordProp->nPathNodes+1, layerId); 
				if(gWordProp->layerEndPref[layerId] <= wordPref || (layerId==0 && wordPref))
				{
					PurgeTopLayers(layerId);
					toplayerEnds = true;
					break;
				}
				layerId--;
			}
		}
		else if(gWordProp->maxLayerId>1 && isBitTurnedOn16(layersEnding, gWordProp->maxLayerId-1))
		{   // if the 2  top layers both end, see if the top one has lower priority. If so delete it!
			BYTE toplayerwordPref = getWordPref(endInDicts, gWordProp->nPathNodes+1, gWordProp->maxLayerId);
			BYTE wordPref = getWordPref(endInDicts, gWordProp->nPathNodes+1, gWordProp->maxLayerId-1); 
			if(toplayerwordPref <= wordPref)
				PurgeTopLayers(gWordProp->maxLayerId-1);
		}
	}
	
	if(toplayerEnds)
	{
		gWordProp->layerEndPos[gWordProp->maxLayerId++]	= gWordProp->nPathNodes + 1;	
		gWordProp->layerStartPos[gWordProp->maxLayerId] = gWordProp->nPathNodes + 2;
	}
	return toplayerEnds;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CDictManager::mngrFillNextNodeDictionaries(MYWCHAR letter, StepsType stepsType) 
{
	if(gWordProp->nNullMoves > 0)
	{
		ShowInfo("!!mngrFillNextNodeDictionaries: nNullMoves > 0!  for workingidx=%d, so ignore advancing!\n", gWorkingWordIdx);
		return 0;
	}
	// which layers in current WordPunc this letter contained in. It's a bit pattern, so maximum 16 layers is allowed!
	USHORT existInLayers = 0; 
	USHORT endInLayers = 0;

	BYTE fittedInSomeDict = mngrAttemptFillNodeDictionaries(letter, &existInLayers, &endInLayers);

	if(!fittedInSomeDict)
		return fittedInSomeDict;

	gWordProp->spacelessTyping = !(existInLayers & 0x1);
	if(gWordProp->maxLayerId > 1 && existInLayers) // first check if this new layer is going to invalidate any previous layer
	{
		if(isBitTurnedOn16(existInLayers, gWordProp->maxLayerId) == false) // top layer does not contain the current letter. So invalidate it!
			PurgeTopLayers(gWordProp->maxLayerId-1);
	}

	if(gWordProp->maxNumExtNodes>=(MAX_LAYER_COUNT-1)) // put a new starting node on next node, then add new node for the letter after
	{
		ShowInfo("Warning!!mngrFillNextNodeDictionaries: maxNumExtNodes>=MAX_LAYER_COUNT-1! stop continuous typing!\n");
		return fittedInSomeDict;
	}
	
	if(m_spacelessTyping && endInLayers)
	{
		gWordProp->endsInDicts = isEndpoint(gWordProp->nPathNodes + 1);
		gWordProp->existInLayers = existInLayers;
		WLBreakIf(gWordProp->endsInDicts == 0, "!!ERROR! mngrFillNextNodeDictionaries: endPointInDicts==0! when layerEnds != 0!!\n");
	//	ShowInfo("--layersExist=%x, endPointInDicts=%x, layerEnds=%x\n", existInLayers, gWordProp->endsInDicts, endInLayers);
		if(UpdateWordLayers(endInLayers, existInLayers, gWordProp->endsInDicts))
		{
			gWordProp->endsInDicts = isEndpoint(gWordProp->nPathNodes + 1);
			for (int j = 0; j < m_nOrderedDicts; j++)
			{
				if (isBitTurnedOn(gWordProp->endsInDicts, j))
				{
					Dictionary *existDict = getDictionaryEnabled(j);
					WLBreakIf(!existDict || j!=existDict->GetDictIndex(), "!!ERROR!! ? mngrFillNextNodeDictionaries::!existDict || j!=existDict->GetDictIndex()!!\n");
					existDict->putFirstNodeInOnPathwayNode(gWordProp->nPathNodes + 1);
				}
			}
		}
	}
	else
		gWordProp->endsInDicts = 0;

	if(letter == SP && !isBitTurnedOn16(existInLayers, 0))
		return 0;
	return fittedInSomeDict;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CDictManager::mngrAttemptFillNodeDictionaries(MYWCHAR letter, USHORT *layers, USHORT *layerEnds)
{
	BYTE fittedInDict = 0;
	for (int j = 0; j < m_nOrderedDicts; j++) 
	{
		Dictionary *existDict = getDictionaryEnabled(j);
		if(!existDict)
			continue;
		
		WLBreakIf(j!=existDict->GetDictIndex(), "!!ERROR!! mngrAttemptFillNodeDictionaries::j!=existDict->GetDictIndex()\n");
		gWordProp->existInDicts[j] = existDict->fillNextPathNode(letter, layers, layerEnds);
		if(gWordProp->existInDicts[j])
			turnOnBit(&fittedInDict, j);
	}

	return fittedInDict;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDictManager::doLanguageSpecificCorrections(MYWCHAR letter, MYWCHAR *printChunk)
{

	unsigned flatLen = 0;
	MYWCHAR *curWord = getCurrentWord(flatLen);

//	ShowInfo(("doLanguageSpecificCorrections flattenedUserCharBuf #%s#  \n"), toA(flattenedUserCharsBuf));
	for (int j = 0; j < m_nOrderedDicts; j++)
	{
		Dictionary *existDict = getDictionaryEnabled(j);
		if (existDict && existDict->GetDictLanguage() != eLang_ENGLISH) 
		{
		//	ShowInfo("---for dict %s\n", existDict->GetDictName(existDict->GetDictIndex()));
			int nCommonChars = 0;
			int lastWordLen = 0;
			LangTreatmentRule *matchingRule = NULL;
			LangTreatmentRule *treatmentRules = existDict->getTreatmentRules();
			if (treatmentRules)
				matchingRule = existDict->getCompactStore()->findMatchingTreatmentRule(treatmentRules, letter, &nCommonChars, curWord, flatLen, lastWordLen);

			if (matchingRule) 
			{
				int triggerLen = mywcslen(matchingRule->triggerStr);
				//int nNewPathNodes = gWordProp->nPathNodes + gWordProp->nNullMoves;

				int applyDictIdx = partOfWordsInADictionary(j, j, -1, TRUE) ? j : -1;
				ShowInfo("doLanguage matching Rule applies to dictIDx %x \n", applyDictIdx);

				if (applyDictIdx >= 0)
				{
					// first reduce the correction string, in case they overlap like della' ==> dell'
					// we only need to back out 2 instead of the whole string
					int nBackoutChars = triggerLen - nCommonChars;
					int replaceLen = mywcslen(matchingRule->replacementStr);

					if (matchingRule->terminatedByOneSP)
						nBackoutChars++;

					for (int i = 0 ; i < nBackoutChars; i++)
						mngrInternalBackSpaceLetterDictionaries( FALSE);

					for (int i = nCommonChars; i < replaceLen; i++) 
					{
						MYWCHAR replaceChar = matchingRule->replacementStr[i];
						findPunctuationChoice( replaceChar);
						if (SP_CR_TAB(replaceChar)) 
						{
							gWordProp->nSP++;
							if ((i + 1) < replaceLen && NOT_SP_CR_TAB(matchingRule->replacementStr[i + 1]))
								goToNextWord((char*)"doLanguageSpecificCorrections", false);
						} 
						else 
						{
							BOOL ret = mngrFillNextNodeDictionaries(replaceChar, eExploring);
							putLetterInWordpath(replaceChar, ret);
							if (matchingRule->terminatedByOneSP && i == (replaceLen-1))
							{
							//	ret = mngrFillNextNodeDictionaries( SP, eExploring);
								putLetterInWordpath(SP, ret);
							}
						}
					}

					mywcscpy(printChunk, &matchingRule->replacementStr[nCommonChars]);
					if (matchingRule->terminatedByOneSP)
						printChunk[1] = SP;

					return;
				}
			}
		}
	}
}

