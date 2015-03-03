// <copyright file="autocorrect.cpp" company="WordLogic">
// Copyright (c) 2001, 2013 All Right Reserved, http://www.wordlogic.com/
//
// THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
// KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// </copyright>
// <author>Reza Nezami</author>
// <email>rnezami@wordlogic.com</email>
// <date>2014-05-25</date>
// <summary>.</summary>
#include "stdafx.h"
#include "dictrun.h"
#include "autocorrect.h"
#include "utility.h"
#include "dictmanager.h"
#include <string.h>

#ifdef _WINDOWS
#include <stdarg.h>
#include <stdio.h>
#else
#include <stdlib.h>
#include <ctype.h>
#include <wchar.h>
#include <unistd.h>
#ifndef __APPLE__
#include <android/log.h>
#endif
#endif

CorrectionList AutoCorrect::mCorrections;
////////////////////////////////////////////////////////////////////////////////////////
AutoCorrect::AutoCorrect(CDictManager *dictmgr):mDictMgr(dictmgr)
{
	keylist = NULL;
	spacedist = NULL;
	distmatrix = NULL;

	correctvalidwords = true;
	correctwordcase = true;
}

///////////////////////////////////////////////////////////////////////////////////////////////
void AutoCorrect::addChar(MYWCHAR ch) 
{
	ShowInfo("AutoCorrect::adding character (%c), mBuffer.size()=%d\n", char2A(ch), (int)mBuffer.size());
	mBuffer.push_back(acchar(ch));

	if(mBuffer.size() > MaxBufferSize) 
		mBuffer.pop_front();

	bufferUpdated();
}

/////////////////////////////////////////////////////////////////////////////////////
void AutoCorrect::addChars(MYWCHAR* chars, int n) 
{	
	ShowInfo("AutoCorrect::addChars #%s#, n=%d, mBuffer length = %d\n", toA(chars),n, (int)mBuffer.size());
	if(n >= MaxBufferSize) 
		setBeforeCursor(chars, n);
	else 
	{
		for(int c = 0; c < n; c++)
			mBuffer.push_back(chars[c]);

		while(mBuffer.size() > MaxBufferSize)
			mBuffer.pop_front();
	}

	bufferUpdated();
}

////////////////////////////////////////////////////////////////////////////////
void AutoCorrect::addPrediction(MYWCHAR* chars, int n) 
{
	ShowInfo("AutoCorrect::addPrediction %s, %d\n", toA(chars), n);
	setBeforeCursor(chars, n);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
void AutoCorrect::backspace(int n) 
{
	for(int c = 0; c < n && !mBuffer.empty(); c++) 
		mBuffer.pop_back();

	bufferUpdated();  // if no correction has been made, we'll consider calling bufferLow in the AutoCorrectInterface
}
////////////////////////////////////////////////////////////////////////
void AutoCorrect::setBeforeCursor(MYWCHAR *chars, int n) 
{
	ShowInfo("AutoCorrect::setBeforeCursor(%s)\n", toA(chars));
	WLBreakIf(n!= mywcslen(chars), "!!ERROR! utoCorrect::setBeforeCursor: mywcslen(%s) != %d !!\n", toA(chars), n);
	mBuffer.clear();
	int o = max( n - MaxBufferSize, 0);

	for(int c = o; c < n; c++) 
		mBuffer.push_back(chars[c]);
}

////////////////////////////////////////////////////////////////////////
void AutoCorrect::setAfterCursor(MYWCHAR* chars, int n) 
{
/*	ShowInfo("AutoCorrect::setAfterCursor(%s)\n", toA(chars));
	WLBreakIf(n!= mywcslen(chars), "!!ERROR! utoCorrect::setAfterCursor: mywcslen(%s) != %d !!\n", toA(chars), n);
	
	mAftercursor.clear();
	int o = max( n - MaxBufferSize, 0);
	
	for(int c = o; c < n; c++) 
		mAftercursor.push_back(chars[c]);
		*/
}

////////////////////////////////////////////////////////////////////////
bool AutoCorrect::bufferUpdated() 
{
	int wordprob;
	bool correctionmade = false;
	bool uppercaseword;

	if(mBuffer.empty()) 
	{
		ShowInfo("AutoCorrect::bufferUpdated::buffer empty\n");
		return false;
	} 

	// if the last character is a space, then we take this as a cue to perform auto-correct
	if(isSeparator(mBuffer.back().ch)) 
	{
		ShowInfo("--AutoCorrect::bufferUpdated:: separator detected:\n");
		int lastchari = (int)(mBuffer.size() - 2);
		while(lastchari >= 0 && isSeparator(mBuffer[lastchari].ch)) 
			lastchari--;
		lastchari++;

		int firstchari = lastchari-1;
		while(firstchari >0 && !isSeparator(mBuffer[firstchari].ch)) 
			firstchari--;
		if(isSeparator(mBuffer[firstchari].ch))
			firstchari++;

		int wordlen = lastchari - firstchari;

		if(wordlen > 0) 
		{
			MYWCHAR wordo[MAX_WORD_LEN];
			MYWCHAR word[MAX_WORD_LEN];

			for(int i = firstchari; i < lastchari; i++) 
				wordo[i-firstchari] = mBuffer[i].ch;
			wordo[wordlen] = NUL;
			uppercaseword = isUpperCase(wordo[0]);

			for(int i = 0; i < wordlen; i++) 
				word[i] = tolower(wordo[i]);
			word[wordlen] = NUL;

			bool donecorrections = false;

			if(!isWord(word, &wordprob)) 
			{
				if(!determinedCorrect(firstchari, lastchari)) 
				{
					if(wordlen > 1) 
					{
						int numCorrections = getCorrections(word, wordlen, maxLevCost(wordlen));
						getMissedSpaces(word, wordlen);

						addKeyboardDistance(word, wordlen);
						printCorrections(mCorrections.corrections, numCorrections);

						if(numCorrections > 0) 
						{
							correctionmade = true;
							makeCorrection(firstchari, lastchari, &(mCorrections.corrections[0]), uppercaseword, 0);
						}
					}

					donecorrections = true;
				}
			} 
		}
	}

	return correctionmade;
}

/////////////////////////////////////////////////////////////////////////////////////////////
bool AutoCorrect::determinedCorrect(int fi,int ei) 
{
	for(int i = fi; i < ei; i++) 
	{
		if(mBuffer[i].status == 1) 
			return true; 
	}

	return false;
}

/*****
* isWord
*	Determines if the specified word exists in the currently loaded auto-correct dictionary.
*	This is the first step in determining whether or not correction should occur.
*
*	chars	-	the UTF-16 characters in the word
*	len		-	the number of characters in the array <i>chars</i>
*
******/
bool AutoCorrect::isWord(MYWCHAR* chars) 
{
	int prob = 0;
	return isWord(chars, &prob) != 0;
}

/*****
* isWord
*	Determines if the specified word exists in the currently loaded auto-correct dictionary.
*	If it does exist, some basic information about the word is returned which can be found
*	with an O(1) lookup using the index value found with the O(log n) hash lookup.
*
*	chars	-	the UTF-16 characters in the word
*	len		-	the number of characters in the array <i>chars</i>
*	prob	-	a pointer to an integer to be set with the probability value
*	hp		-	a pointer to an integer to be set with the hash value for this word
*	hi		-	a pointer to an integer to be set with the index value for the hash
*					Note: if the word is a personal word then this is set to -1
*
*	Jonathon Simister (December, 2012)
******/
int AutoCorrect::isWord(MYWCHAR* word, int* prob) 
{
	WLBreakIf(!word, "!!WARNING! AutoCorrect::isWord() word is not terminated!!\n");
//	ShowInfoIf(word && word[0], "AutoCorrect:isWord(%s)\n", toA(word));
	int dictIdx = -1;
	//mDictMgr->isChunk(const_cast<MYWCHAR*>(chars));
	*prob = mDictMgr->isWordInDictionaries(word, dictIdx);
	
//	ShowInfo("AutoCorrect::isWord: (%s) is dictionary word? %d!\n", toA(chars), ret);
	return (prob!=0) + !dictIdx; // this is to simulate returing 2 if word is in personal dictionary
}

MYWCHAR *AutoCorrect::getBufferedWord(int startIdx, int len, MYWCHAR *out)
{
	static MYWCHAR sWord[MAX_WORD_LEN];
	out = out==NULL ? sWord : out;
	for(int i = 0; i < len; i++) 
		out[i] = tolower(mBuffer[i+startIdx].ch);
	out[len] = NUL;
	return out;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
CorrectionList * AutoCorrect::getPredictions(int maxCount) 
{
	ShowInfo("AutoCorrect::getPredictions(%d)\n", maxCount);
	maxCount = min(maxCount, MAXNUMCORRECTIONS);

	mCorrections.clear();

	if(mBuffer.size() < 1 || isSeparator(mBuffer.back().ch)) 
		return &mCorrections;

	int si=0, ei=0;
	MYWCHAR *wordP = getLastWord(0, (int)(mBuffer.size()-1), &si, &ei);
	int wordlen = ei - si;

	if(wordlen < 2) // no  correction for 1 letter words
		return &mCorrections;
	
	MYWCHAR word[MAX_WORD_LEN];
	mywcs2lower(word, wordP, wordlen);

	int correctionNum = getCorrections(word, wordlen, maxLevCost(wordlen));
	if(correctionNum==0)
		return &mCorrections;

	getMissedSpaces(word, wordlen);
	addKeyboardDistance(word, wordlen);

	correctionNum = min(correctionNum, maxCount);
	if(isUpperCase(mBuffer[si].ch))
	{
		for(int i = 0; i < correctionNum; i++) 
			mCorrections.corrections[i].word[0] = toupper(mCorrections.corrections[i].word[0]);
	}

	ShowInfo("AutoCorrect::gotPredictions(): correction results:");
	printCorrections(mCorrections.corrections, correctionNum);

	return &mCorrections;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
CorrectionList * AutoCorrect::getCorrections(MYWCHAR *word) 
{
	ShowInfo("AutoCorrect::getCorrections(%s)\n", toA(word));
	int maxCount = MAXNUMCORRECTIONS;

	int wordlen = mywcslen(word);

	if(wordlen < 2) // no  correction for 1 letter words
		return &mCorrections;
	
	mCorrections.clear();

	int correctionNum = getCorrections(word, wordlen, maxLevCost(wordlen));
	if(correctionNum==0)
		return &mCorrections;

	getMissedSpaces(word, wordlen);
	addKeyboardDistance(word, wordlen);

	correctionNum = min(correctionNum, maxCount);

	ShowInfo("---AutoCorrect::getCorrections(): correction results:");
	printCorrections(mCorrections.corrections, correctionNum);

	return &mCorrections;
}
/*****
*   filterMatches
*	filters a tentative list of matches produced by the Levenshtein distance algorithm by matches found in the mGraph
*	which are not actual words using a hash look-up. This function also adds probability and part
*	of speech data to the matches.
*
*	matches - a pointer to a vector containing the tentative matches to be filtered
*	filtered - a pointer to a vector which will be filled with only the real words from <i>matches</i>
*	returns - the number of real words found in <i>matches</i>
*
*	Jonathon Simister (December 14, 2012)
******/
int AutoCorrect::filterMatches(acMatch* matches, acMatch* filtered, int num) 
{
	int n = 0;
	int prob=0;
	for(int i = 0; i < num; i++) 
	{
		assert(isWord(matches[i].word, &prob));
		{
			matches[i].prob = (prob & 63);
			matches[i].tgfreq = 0;
			filtered[n++] = matches[i];
		}
	}

	return n;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
void AutoCorrect::getMissedSpaces(MYWCHAR* s, int slen, CorrectionList &list) 
{
	return;
	ShowInfo("AutoCorrect::getMissedSpaces:: s(%s), slen(%d)\n", toA(s), slen);
	CorrectionList firstwords;
	acMatch first;
	for(int i = 2; i < slen; i++) 
	{
		if(i <= 5) 
		{
			int pref = 0;
			mywcsncpy(first.word, s, i);
			if(isWord(first.word, &pref)) 
				firstwords.addCorrection(first.word, pref, 0);
			else
				continue;
		} 
		else 
			getCorrections(s, i, 1, firstwords);

		for(int c = 0; c < firstwords.fillCount; c++) 
		{
			int maxCost = slen-i <= 5 ? 0 : 1;
			CorrectionList localList;
			int correctionNum = getCorrections(s+i, slen-i, maxCost, localList);

			for(int j = 0; j < correctionNum; i++) 
			{
				USHORT score = 1 + localList.corrections[j].levscore + firstwords.corrections[c].levscore;
				list.corrections[i].tgfreq = (localList.corrections[j].tgfreq + firstwords.corrections[c].tgfreq) / 2;
                int prob = min((localList.corrections[j].prob + firstwords.corrections[c].prob) / 2.0, 255.0);
				MYWCHAR tmpWord[MAX_WORD_LEN];
				//int tmplength = firstwords.corrections[c].length + 1 + localList.corrections[j].length;
				mywcscpy(tmpWord, firstwords.corrections[c].word);
				tmpWord[firstwords.corrections[c].length] = SP;
				mywcscat(tmpWord, localList.corrections[j].word);
				list.addCorrection(tmpWord, prob, score); 
			}
		}
	}
}

/********************************************************
* getCorrections
*	Performs a preliminary search for corrections for a specific block of characters and stores the results in provided array.
*	s		-	the text array to be corrected
*	slen	-	the number of characters in the text array <i>s</i>
*	maxcost - the maximum allowable Levenshtein distance between the text and possible corrections
*	returns numbers of corrections found.
*   prevousi is removed. CorrectionList is assumed to contain all slots needed and all are empty! Reza(2014)
******************************************************************/
int AutoCorrect::getCorrections(MYWCHAR* s, int slen, int maxcost, CorrectionList &list) 
{
	ShowInfo("AutoCorrect::getCorrections: s(%s), slen(%d), maxcost(%d)\n", toA(s), slen, maxcost);
	list.clear();
	int strow[MAX_WORD_LEN];
	MYWCHAR slower[MAX_WORD_LEN];
	for(int i = 0; i < slen; i++) 
	{
		slower[i] = tolower(s[i]);
		strow[i] = i;
	}
	slower[slen] = NUL;
	strow[slen]= slen;

	MYWCHAR curPath[MAX_WORD_LEN];
	curPath[0] = NUL;
	int nChildren = mDictMgr->getCompactStore(1)->getNumRootChildren();
	for(int i = 0; i < nChildren; i++)
	{
		CompactNode *node = mDictMgr->getCompactStore(1)->getChildByIndex(i);
		computeRow(node, slower, strow, curPath, 0, maxcost, list);
	}

//	filterMatches(matches, corrections, matchNum);
	return list.fillCount;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void AutoCorrect::computeRow(CompactNode *node, MYWCHAR* search, int *prow, MYWCHAR* path, int depth, int maxcost, CorrectionList &list) 
{
	if(depth > 15 || list.fillCount >= MAXNUMCORRECTIONS*2)
		return;

	path[depth] = node->Letter;
	path[depth+1] = NUL;

	if(node->Letter == (MYWCHAR)'\'') 
	{
		for(int i = 0; i < node->Count; i++) 
		{
			CompactNode *child = mDictMgr->getCompactStore(1)->getChildByIndex(i, node);
			computeRow(child, search,  prow, path, depth+1, maxcost, list);
		}
		return;
	}

	int row [MAX_WORD_LEN];
	int columns = mywcslen(search) + 1;
	WLBreakIf(columns>=MAX_WORD_LEN, "!!ERROR AutoCorrect::computeRow: increase row size!\n");
	if(depth > columns) 
	{
		int cost = prow[columns-1] + 1;
		if(cost <= maxcost) 
		{
			int len = mywcslen(path);
			if(len > 2)
			{
				int prob;
				if(isWord(path, &prob)) 
					list.addCorrection(path, prob, cost);
			}
			row[columns-1] = cost + 1;
			for(int i = 0; i < node->Count; i++) 
			{
				CompactNode *child = mDictMgr->getCompactStore(1)->getChildByIndex(i, node);
				computeRow(child, search, row, path, depth+1, maxcost, list);
			}
		}
	} 
	else 
	{
		int insertCost, deleteCost, replaceCost;
		int rmin = 100000;
		row[0] = prow[0] + 1;

		for(int i = 1; i < columns; i++) 
		{
			insertCost = row[i-1] + 1;
			deleteCost = prow[i] + 1;

			if( mDictMgr->accentEqual(search[i-1], node->Letter)) 
				replaceCost = prow[ i-1];
			else 
				replaceCost = prow[i-1] + 1;

			row[i] = min(insertCost, min(deleteCost, replaceCost));
			rmin = min(rmin, row[i]);
		}

		int length = mywcslen(path);
		if(length > 2 && row[columns-1] <= maxcost) 
		{
			int prob;
			if(isWord(path, &prob)) 
			{
				list.addCorrection(path, prob, row[columns-1]);
			}
		}

		if(rmin <= maxcost) 
		{
			for(int i = 0; i < node->Count; i++) 
			{
				CompactNode *child = mDictMgr->getCompactStore(1)->getChildByIndex(i, node);
				computeRow(child, search, row, path, depth+1, maxcost, list);
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void CorrectionList::addCorrection(MYWCHAR *word, unsigned pref, unsigned score)
{
	ShowInfoIf(fillCount >= (2*MAXNUMCORRECTIONS-1), "CorrectionList::addCorrection: (%s) list is full!\n", toA(word));

	int len = mywcslen(word);
	assert(len>0);

    int normalizedScore = max(0, min((int)score, 3));
	int probability = myClamp(pref + (3-normalizedScore)*15, 0, 255, 255);

	// first check if word is already in correctionList
	if(int found = exist(word) >= 0)
	{
		updateSlot(found, word, probability, score);
		return;
	}

	// next check if lowest prob possible slot is of higher probability than this word: If so  return!
	if(fillCount == (2*MAXNUMCORRECTIONS-1))
	{
		if(corrections[fillCount].prob > probability)
		{
			ShowInfo("CorrectionList::addCorrection: correction list is already full with higher prob results! Ignore #%s#\n", toA(word));
			return;
		}
	}

	// next find the slot and fill it up:
	corrections[fillCount].set(word, len, probability, score);
	// now do incremental sort:
	singleSort(fillCount);

	fillCount = min(fillCount+1, 2*MAXNUMCORRECTIONS-1);

}
///////////////////////////////////////////////////////////////////////////////
// sorts based on prob value.
// assumes list is already sorted except possibly for the newslot location.
void CorrectionList::singleSort(int newslot)
{
	if(newslot ==0)
		return;
	// sort if the newly inserted element is in the last slot(or the list from newslot on is already sorted). This is the most common case.
	if(newslot<=fillCount && corrections[newslot].prob > corrections[newslot-1].prob) 
	{
		int insertSlot = newslot;
		while(insertSlot > 0 && corrections[insertSlot].prob > corrections[insertSlot-1].prob)
		{
			acMatch tmp = corrections[insertSlot];
			corrections[insertSlot] = corrections[insertSlot-1];
			corrections[insertSlot-1] = tmp;
			insertSlot--;
		}
	}
	else if(corrections[newslot].prob < corrections[newslot+1].prob)
	{ // if newly inserted element is in the middle of the list and has lower prob than the elements bellow it! (Not Common!)
		int insertSlot = newslot;
		while(insertSlot < fillCount && corrections[insertSlot].prob < corrections[insertSlot+1].prob)
		{
			acMatch tmp = corrections[insertSlot];
			corrections[insertSlot] = corrections[insertSlot+1];
			corrections[insertSlot+1] = tmp;
			insertSlot++;
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////
void CorrectionList::updateSlot(int idx, MYWCHAR *word, unsigned prob, unsigned score)
{
	corrections[idx].set(word, mywcslen(word), prob, score);
	singleSort(idx);
}
//////////////////////////////////////////////////////////////////////////////
// checks if str already exists in the list. If so, returns its slot, otherwise returns -1.
int CorrectionList::exist(MYWCHAR *str)
{
	for(int i=0; i<fillCount; i++)
		if(mywcscmp(corrections[i].word, str)==0)
			return i;
	return -1;
}

//////////////////////////////////////////////////////////////////////////////////////
void AutoCorrect::logBuffer() 
{
	int size = (int)mBuffer.size();
	ShowInfo("AutoCorrect::logBuffer()%d\n", size);
	return;
	static MYWCHAR *slog = NULL;
	if(slog==NULL)
		slog = new MYWCHAR[50];

	if(slog==NULL)
		return;
	for(int i = 0; i < size; i++)
	{
		ShowInfo("AutoCorrect %c", char2A(mBuffer[i].ch));
		slog[i] = mBuffer[i].ch;
	}
	slog[size] = NUL;
	ShowInfo("(%s)\n", toA(slog));
}

/***********************************************************************************
* makeCorrection
*	Makes a correction to the buffer and the attached program through a call to the correctionMade method in the AutoCorrectInterface
*
*	si				-	the start index of the characters to be replaced
*	ei				-	the end index of the characters to be replaced
*	replacement		-	the match which is to be used for correction
*	inputUpperCase	-	true if the input string, to be corrected, starts with an upper-case character
*	type			-	the type of correction to be made (0 lev, 1 n-gram replace of valid word, 3&4 case corrections)
*
*	Jonathon Simister (December, 2012)
************************************************************************************/
void AutoCorrect::makeCorrection(int si, int ei, acMatch *replacement, bool inputUpperCase, int type) 
{
	if((type == 2 || type == 3) && !correctwordcase) 
		return;

	if(type == 1 && !correctvalidwords) 
		return;

	//int previoussize = mBuffer.size();
	int replen = replacement->length;
	int prevlen = ei-si;
	MYWCHAR repchars[MAX_WORD_LEN];
	MYWCHAR prevchars[MAX_WORD_LEN];

	for(int c = 0; c < replen; c++) 
		repchars[c] = replacement->word[c];
	repchars[replen] = NUL;

	if(0)//replacement->accronym) 
	{
//		for(int c = 0; c < replen; c++)
//			repchars[c] = toupper(repchars[c]);
//		repchars[replen] = NUL;
	} 
	else if(/*replacement->propernoun ||*/ inputUpperCase) 
		repchars[0] = toupper(repchars[0]);

	for(int c = 0; c < prevlen; c++) 
		prevchars[c] = mBuffer[si+c].ch;
	prevchars[prevlen] = NUL;

	deque<acchar>::iterator it, it2, rit;
	it2 = bufferIterator(ei);
	it2 = bufferInsertChars(it2, repchars, replen);

	if(type != 0) 
	{
		rit = it2;
		for(int c = 0; c < replen; c++) 
		{
			rit->status = 2; // mark the characters as coming from a non-Levenshtein correction
			rit++;
		}
	}

	it = bufferIterator(si);
	mBuffer.erase(it, it2);
	logBuffer();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
deque<acchar>::iterator AutoCorrect::bufferInsertChars(deque<acchar>::iterator it, MYWCHAR* chars, int len) 
{
	for(int c = len-1; c >= 0; c--) 
		it = mBuffer.insert(it, acchar(chars[c]));

	return it;
}

/*****
* correctionPending
*	Determine if the word currently being typed would exist if it were ended in its current state.
*	Jonathon Simister (December, 2012)
******/
bool AutoCorrect::correctionPending() 
{
	ShowInfo("AutoCorrect::correctionPending() called");
	if(mBuffer.empty() || isSeparator(mBuffer.back().ch)) 
		return false; 

	logBuffer();
	int ws,we;
	MYWCHAR *thisword = getLastWord(0, (int)(mBuffer.size()-1), &ws, &we);
	int wordlen = we-ws;
	ShowInfo("-correctionPending(): mBuffer.size()=%d, ws=%d, we=%d\n ", (int)mBuffer.size(), ws, we);
	if(wordlen < 2) 
		return false; 

	return isWord(thisword)==false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AutoCorrect::reverseCorrection(int si, int olen, int alen, MYWCHAR *ochars) 
{
	ShowInfo("AutoCorrect::reverseCorrection: si=%d, replacementLen=%d, alen=%d, replacement=(%s), buffersize=%d\n", si, olen, alen, toA(ochars),  (int)mBuffer.size());
	int nsi = (int)(mBuffer.size() - si - 1);
	deque<acchar>::iterator it, it2, it3, ait;
	//logBuffer();
	ShowInfo("--nsi=%d\n", nsi);
	it2 = bufferIterator(nsi+alen); // iterator to one past the last character of the string inserted by auto-correct
	it3 = bufferInsertChars(it2, ochars, olen); // inserts the original characters back and gets an iterator to the first of them
	it = it3;

	for(int c = 0; c < olen; c++) 
	{
		it->status = 1;
		it++;
	}
	
	it = bufferIterator(nsi); // gets the iterator to the first of the characters inserted by auto-correct
	ait = it;

	bool nonlev = false;
	for(int c = 0; c < alen; c++) 
	{
		ait++;
		if((ait->status & 2) == 2)
		{
			nonlev = true;
			break;
		}
	}

	mBuffer.erase(it, it3); // erases the characters inserted by auto-correct
//	logBuffer();
}

/*******************************************************************
* getLastWord
*	Find the last full word in the character buffer.
*
*	starti	-	the start index for the buffer (typically 0)
*	endi	-	the end index for the buffer (either len-1 or some lower number (before the last word) to search for (for example) the second last word.
*	fs		-	a pointer to an integer which will be set to the index of the first character of the word found
*	fe		-	a pointer to an integer which will be set to the index of the first character *after* the word found (lenfound = fe-fs)
*   out     -   pointer to character array which will contain the last word.
*	Note: Unlike some other functions which accept pointers in this code, it is NOT acceptable to pass a NULL-pointer to either fs or fe. After all,
*	what is the point of searching for a string if you're not going to use the result?
*
*	Jonathon Simister (December, 2012)
******/
MYWCHAR * AutoCorrect::getLastWord(int starti, int endi, int *fs, int *fe, MYWCHAR *out) 
{
	ShowInfo("AutoCorrect::getLastWord: starti=%d, endi=%d\n", starti, endi);
	static MYWCHAR sWord[MAX_WORD_LEN];
	out = out==NULL ? sWord : out;

	int c = endi;
	while(isSeparator(mBuffer[c].ch))
	{
		if(c == starti)
		{
			*fe = starti;
			*fs = starti;
			return out;
		}
		c--;
	}
	*fe = c+1;

	out[c+1] = NUL;
	while(true)
	{
		out[c] = mBuffer[c].ch;
		if(c == starti) 
		{
			*fs = c;
			break;
		} 
		else if(isSeparator(mBuffer[c].ch)) 
		{
			*fs = c+1;
			break;
		} 
		else
			c--;
	}

	WLBreakIf((*fe - *fs) >= MAX_WORD_LEN, "!!ERROR! AutoCorrect::getLastWord: wordlen > MAX_WORD_LEN!!\n");
	return &out[*fs];
}

/********************************************************************************
* getHashIndex
*	Performs a binary search for the hash of a string of characters and then returns the index found or -1 if nothing was found.
*	This index can then be used, for example, to lookup the probability of a word.
*
*	chars	-	the characters to search for (MYWCHAR*)
*	len		-	the number of characters pointed to by <i>chars</i>
*
*	Jonathon Simister (December, 2012)
***************************************************/
void AutoCorrect::getSecondCorrection(int si, MYWCHAR* ochars, int olen, MYWCHAR* achars, int alen, MYWCHAR** cchars, int* clen)
{
	static MYWCHAR retchars[MAX_WORD_LEN];

	int correctionNum = getCorrections(ochars, olen, maxLevCost(olen));
	addKeyboardDistance(ochars, olen);
	sort(mCorrections.corrections, mCorrections.corrections+correctionNum-1, &acMatch::sortAdjScore);

	if(correctionNum >= 2) 
	{
		mywcscpy(retchars, mCorrections.corrections[1].word);
		*clen = mCorrections.corrections[1].length;
		*cchars = retchars;
	} 
	else
	{
		*cchars = NULL;
		*clen = 0;
	}
}

////////////////////////////////////////////////////////////////////////////////
void AutoCorrect::printCorrections(acMatch* matches, int len) 
{
#if defined(_DEBUG)
	ShowInfo("\nAutoCorrect::printCorrections: num of corrections = %d:\n", len);
	ShowInfo("AutoCorrect::printCorrections:\n");
		for(int i=0; i < len; i++) 
			ShowInfo( "%s, ", toA(matches[i].word) );
#endif
}
///////////////////////////////////////////////////////////////////////////////////////
void AutoCorrect::loadKeyPositions(MYWCHAR* kl, float* dm, float* sd, int n) 
{
	if(keylist != NULL) 
		delete[] keylist;

	if(distmatrix != NULL) 
		delete[] distmatrix;
	
	if(spacedist != NULL) 
		delete[] spacedist;
	
	keylist = kl;
	distmatrix = dm;
	spacedist = sd;
	nkeys = n;
}

/****************************************************************************
* keyLev
*	A modified version of Levenshtein distance which gives a lower (1/3) weight to edits where a character has
*	been substitued for another character which it is close (less than 40% of the maximum key separation) to
*	on the keyboard.
*
*	Jonathon Simister (December, 2012)
*******************************************************************************/
float AutoCorrect::keyLev(const MYWCHAR* s1, int len1, const MYWCHAR* s2, int len2)
{
    float matrix[MAX_WORD_LEN][MAX_WORD_LEN];

	for(int i = 0; i < len1+1; i++) 
	{
		matrix[i][0] = (float)i;
	}

	for(int i = 0; i < len2+1; i++) 
		matrix[0][i] = (float)i;

    for (int i = 1; i <= len1; i++) 
	{
 		MYWCHAR ch1 = s1[i-1];
        for (int j = 1; j <= len2; j++)
		{
            MYWCHAR ch2 = s2[j-1];
            if(ch1 == ch2) 
            	matrix[i][j] = matrix[i-1][j-1];
            else 
			{
            	float dist = keyDist(ch1,ch2);
            	if(dist == 0) 
            		matrix[i][j] = matrix[i-1][j-1];
            	else 
				{
            		float deleteCost = matrix[i-1][j] + 1;
            		float insertCost = matrix[i][j-1] + 1;
					float replaceCost = 0.f;
            		if(dist < 0.2) 
            			replaceCost = matrix[i-1][j-1] + (float)0.3333333;
            		else if(dist < 0.4)  
            			replaceCost = matrix[i-1][j-1] + (float)0.6666666; 
					else 
            			replaceCost = matrix[i-1][j-1] + 1;

            		matrix[i][j] = min(deleteCost, min(insertCost, replaceCost));
            	}
            }
        }
    }

	return matrix[len1][len2];
}

/******************************************************
* keyDist
*	Determines the distance between two keys, relative to other keys on the keyboard, within the range [0,1].
*
*	Jonathon Simister (December, 2012)
********************************************************/
float AutoCorrect::keyDist(MYWCHAR ch1, MYWCHAR ch2) 
{
	pair<MYWCHAR*,MYWCHAR*> bounds;
	int i1, i2;
	int rowlen;

	if(mDictMgr->getTopDictionaryId() != eLang_ENGLISH) 
	{
		ch1 = mDictMgr->removeAccent(ch1);
		ch2 = mDictMgr->removeAccent(ch2);
	}

	// if one of the characters is a space then lookup the other character in the keylist and
	// then return the value at its index in the spacedist array
	if(ch1 == SP) 
	{
		if(spacedist != NULL) 
		{
			bounds = equal_range(keylist, keylist+nkeys, ch2);
			if(*bounds.first == ch2)
			{
				i2 = (int)(bounds.first - keylist);
				return spacedist[i2];
			} 
			else 
				return 1;
		}
		else 
			return 1;
	}

	if(ch2 == SP) 
	{
		if(spacedist != NULL) 
		{
			bounds = equal_range(keylist, keylist+nkeys, ch1);

			if(*bounds.first == ch1)
			{
				int idx = (int)(bounds.first - keylist);
				return spacedist[idx];
			} 
			else 
				return 1;
		}
		else 
			return 1;
	}

	bounds = equal_range(keylist, keylist+nkeys, ch1);
	if(*bounds.first == ch1) 
		i1 = (int)(bounds.first - keylist);
	else 
		return 1;

	bounds = equal_range(keylist, keylist+nkeys, ch2);

	if(*bounds.first == ch2)
		i2 = (int)(bounds.first - keylist);
	else 
		return 1;

	if(i1 == i2) 
		return 0; 
	if(i1 > i2) 
		swap(i1,i2); 

	// the following code determines the 1-dimensional location on the packed distance matrix for a key-pair
	// I believe there is a more efficient, non-trivial way to find this, but it's not a major priority right now.
	rowlen = nkeys-1;
	int i = 0;

	for(int c = 0; c < i1; c++) 
		i += rowlen--;

	i += ((i2 - i1) - 1);
	return distmatrix[i];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void AutoCorrect::addKeyboardDistance(MYWCHAR *s, int slen) 
{
	if(keylist == NULL || distmatrix == NULL) 
		return; 

	for(int i = 0; i < mCorrections.fillCount; i++)
	{
		mCorrections.corrections[i].keydist = keyLev(s, slen, mCorrections.corrections[i].word, mCorrections.corrections[i].length);
		mCorrections.corrections[i].updateAdjustedScore();
	}
	sort(mCorrections.corrections, mCorrections.corrections+mCorrections.fillCount-1, &acMatch::sortAdjScore);
}

deque<acchar>::iterator AutoCorrect::bufferIterator(int pos) 
{
	deque<acchar>::iterator it = mBuffer.begin();
	while(pos > 0) 
	{
		it++;
		pos--;
	}

	return it;
}