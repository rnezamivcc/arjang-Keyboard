// <copyright file="dictnext.cpp" company="WordLogic Corporation">
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
// <summary>Contains support for predicting next letter and next words based on previously input word from user.</summary>

#include "stdafx.h"
#include "wordpath.h"
#include "dictmanager.h"
#include "dictionary.h"
#include "wordpunct.h"
#include "userWordCache.h"
#include "searchResults.h"

extern BYTE sNumAllDictSearchSlots;    // this is == sMaxSearchRanks - 10

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// main entry for searching dictionaries for next words. It provides interface to two types of search: wideSearch and DeepSearch.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDictManager::mngrInternalNextWordsInDictionaries(int layer) 
{
	//ShowInfo("\nFIRST DO WIDE SEARCH: \n");
	mngrNextWordsInDictionaries(0, m_nOrderedDicts, eWideSearch, eOrdinary, layer);
//	if (!enoughResultsFromAllDictionaries()) 
	{
	//	ShowInfo("--NOW DO DEEP SEARCH: \n");
		mngrNextWordsInDictionaries(0, m_nOrderedDicts, eDeepSearch, eOrdinary, layer);
	}
	enoughResultsFromAllDictionaries();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDictManager::mngrNextWordsInDictionaries(int start, int end, eSearchMode searchMode, WordValue wordValue, int layer) 
{
	gWordProp->wordValue = wordValue; // set it in the gWordProp struct, prevent stack grow on recursion
	for (int j = start; j < end; j++)
	{
		Dictionary *dict = getDictionaryEnabled(j);
		if(dict)
		{
			WLBreakIf(j!=dict->GetDictIndex(), "!!ERROR!! ? mngrNextWordsInDictionaries::j!=existDict->GetDictIndex()\n");
			dict->nextWords(searchMode, wordValue, layer);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDictManager::mngrNextLettersInDictionaries() 
{
	int nLettersRetrieved = 0;
	PrefLetterNode allPrefLetters[MAX_FOLLOWING_LETTERS];
	int nAllPrefLetters = 0;
	BOOL spaceIsPartOfLetterSelection = FALSE;
	int nRequiredLetters = NPREFERENCES;
	int	filledTillHere = 0;

	memset(allPrefLetters, 0, sizeof(allPrefLetters));

	for (int i = 0; i < m_nOrderedDicts; i++)
	{
		if (nAllPrefLetters >= 2*nRequiredLetters)
			break;
		Dictionary *existDict = getDictionaryEnabled(i);
		if (!existDict)
			continue;
		nLettersRetrieved = existDict->nextLetters(allPrefLetters, nRequiredLetters, &nAllPrefLetters, FALSE);
		filledTillHere = max(filledTillHere, nAllPrefLetters);
	}

	nAllPrefLetters = filledTillHere;
	lettersSort(allPrefLetters, nAllPrefLetters);

	int lowestNLetters = min(nRequiredLetters, nAllPrefLetters);
	for (int p = 0; p < lowestNLetters; p++) 
	{
		m_prefLetters[p] = allPrefLetters[p].letter;
		m_posWordChunks[p] = allPrefLetters[p].posWordChunks;
		spaceIsPartOfLetterSelection = spaceIsPartOfLetterSelection || (m_prefLetters[p] == SP);
	}

	if (nAllPrefLetters < NPREFERENCES && !spaceIsPartOfLetterSelection) 
	{
		BYTE layer = 0;
		if (gWordProp->nNullMoves==0 && mngrIsWordOrChunkInDictionariesIdx(0, m_nOrderedDicts, NULL, &layer) >= 0) 
		{
			m_prefLetters[nAllPrefLetters] = SP;
			m_posWordChunks[nAllPrefLetters] = 1;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
MYWCHAR *CDictManager::nextLetters(int **posWordsAr) 
{
	memset(m_prefLetters, 0, sizeof(m_prefLetters));
	memset(m_posWordChunks, 0, sizeof(m_posWordChunks));
	*posWordsAr = m_posWordChunks;
	if (m_noDictionariesLoaded || emptyWordPath())
	{
		ShowInfo("nextLetters: emptyWordPath: return m_prefLetters #%s#\n", toA(m_prefLetters));
		return m_prefLetters;
	}

/*
	int midpoint = sNumAllDictSearchSlots+NWORDPREFERENCES;
	int endIndex = midpoint + PWresult.nApprovedResults - 1;
	int startIdx = endIndex - (NWORDPREFERENCES + PWresult.nApprovedResults);
	for(int i=endIndex, count=0; i> startIdx && count<NWORDPREFERENCES; i--, count)
	{
		int j = i >= midpoint ? midpoint+(endIndex-i) : sNumAllDictSearchSlots + (midpoint-i-1);
		SearchResultEntry *srep = &gRankedSearchResults[j];
		if(srep == NULL || srep->text==NULL)
			continue;
		MYWCHAR *text = srep->layer == 0 ? srep->text : srep->predText;
		int pathlen = srep->layer == 0 ? gWordProp->nPathNodes : gWordProp->nPathNodes + srep->layer;
		MYWCHAR nextletter = text[gWordProp->nPathNodes] != SP? text[pathlen] : text[pathlen+1];
		nextletter = lwrCharacter(nextletter);
		if(wordContainsLetter(m_prefLetters, nextletter))
			continue;
		m_prefLetters[count] = nextletter;
		m_posWordChunks[count++] = srep->nOffspringWords;
	}	
	ShowInfo("-NextLetters1 %s\n", toA(m_prefLetters));

	memset(m_prefLetters, 0, sizeof(m_prefLetters));
	*/
	mngrNextLettersInDictionaries();
	mywcslwr(m_prefLetters);
//	ShowInfo("-NextLetters2: %s\n", toA(m_prefLetters));
	
//	ShowInfo(TEXT("-nextLetters returning #%s#, gHistWordIdx=%d, nPath=%d\n"), toA(m_prefLetters), gHistWordIdx, gWordProp->nPathNodes);
	*posWordsAr = m_posWordChunks;
	return m_prefLetters;
}

/////////////////////////////////////////////////////////////////////////////////////////
BOOL CDictManager::isChunk(MYWCHAR *word)
{
	//ShowInfo("MK isChunk:(%s)\n",toA(word));
	trimEndingSpaces(word);
	Dictionary* dict = getTopDictionary();
	if(!dict || dict->GetDictLanguage() == eLang_KOREAN)
	{
		return false;
	}

	if(dict->GetDictLanguage() == eLang_ENGLISH)
	{
		if(dict->getCompactStore()->HasChunkEndings(word))
		{
			return true;
		}
	}
	else
	{
		int dictIdx =0;
		return retrieveEndNodeForString(word, &dictIdx, true) != NULL;
	}

	return false;

	//return retrieveEndNodeForString(word, &dictIdx, true) != NULL;

}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
SearchResultEntry *CDictManager::findLowestPrefInFinalFive(	PredictionType lookingFor, int *finalFiveIdxP) 
{
	SearchResultEntry *mysrep = NULL;
	USHORT minPref = USHRT_MAX;
	int minIdx = -1;
	int i = 0;
	SearchResultEntry *lowestsrep = NULL;
	int startIdx =  sNumAllDictSearchSlots+NWORDPREFERENCES; 
	int endIndex = startIdx + PWresult.nApprovedResults;

	for (i = startIdx; i < endIndex; i++) 
	{
		mysrep = &gRankedSearchResults[i];
		if (mysrep->cascadingPref <= minPref && mysrep->predType == lookingFor)
		{
			minPref = mysrep->cascadingPref;
			minIdx = i;
			lowestsrep = mysrep;
		}
	}
	*finalFiveIdxP = minIdx;
	return lowestsrep;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// This is a short cut to set mMostPreferredWords. The caller is responsible for providing 
//  sorting and actual word as valid persistent pointers which stay valid.
void CDictManager::setMostPreferredWord(MYWCHAR *word, int pref)
{
	int lowestIdx = 0;
	USHORT lowval = mMostPreferredWordPrefs[lowestIdx];
	for (int i = 1; i < NWORDPREFERENCES; i++)
	{
		if (mMostPreferredWordPrefs[i] < lowval)
		{
			lowestIdx = i;
			lowval = mMostPreferredWordPrefs[i];
		}
	}
	
	if (pref > lowval)
	{
		mMostPreferredWords[lowestIdx] = word;
		mMostPreferredWordPrefs[lowestIdx] = pref;
	}
}

void CDictManager::sortMostPreferredWords()
{
	int i, j;
	for (i = 0; i < NWORDPREFERENCES; i++){
		int tmppref = mMostPreferredWordPrefs[i];
		MYWCHAR* tmpval = mMostPreferredWords[i];
		for (j = i + 1; j < NWORDPREFERENCES; j++){
			if (tmppref < mMostPreferredWordPrefs[j]){
				mMostPreferredWordPrefs[i] = mMostPreferredWordPrefs[j];
				mMostPreferredWordPrefs[j] = tmppref;
				mMostPreferredWords[i] = mMostPreferredWords[j];
				mMostPreferredWords[j] = tmpval;
				tmppref = mMostPreferredWordPrefs[i];
				tmpval = mMostPreferredWords[i];
			}
		}
	}
	for (int i = 0; i < NWORDPREFERENCES; i++){
		ShowInfo("mMostPreferredWord[%d]=(%s)\n", i, toA(mMostPreferredWords[i]));
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
// push everything further to the back and insert this one in front
void shiftFinalFiveUp(int includingFinalFiveIdx)
{
	int startIdx =  sNumAllDictSearchSlots+NWORDPREFERENCES; 
	for (int i = includingFinalFiveIdx; i >=startIdx && gRankedSearchResults[i].text; i--) 
		gRankedSearchResults[sNumAllDictSearchSlots + i + 1] = gRankedSearchResults[sNumAllDictSearchSlots + i];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
bool sReplacedOneCacheItem = false;
BOOL CDictManager::replaceInFinalFive(MYWCHAR *text, PredictionType predType, USHORT pref) 
{
	int replaceIdx = 0;
	SearchResultEntry *freesrep = findLowestPrefInFinalFive(eDictWordPredict, &replaceIdx);
	if (freesrep && freesrep->cascadingPref <= pref) 
	{
	//	shiftFinalFiveUp(replaceIdx); // always put in front of the list
	//	freesrep = &gRankedSearchResults[sNumAllDictSearchSlots];
		USHORT newpref = max(pref, freesrep->cascadingPref);
		fillSrep(freesrep, text, false);
		freesrep->predType = predType;
		freesrep->cascadingPref = newpref;
		sReplacedOneCacheItem = true;
		return TRUE;
	}
	return FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////
bool CDictManager::UpdateInFinalFive(MYWCHAR *text, USHORT cascPref) 
{
	int startIdx =  sNumAllDictSearchSlots+NWORDPREFERENCES; 
	int endIndex = startIdx + PWresult.nApprovedResults;
	int len = mywcslen(text);
	for (int i = startIdx; i < endIndex; i++)
	{
		SearchResultEntry *mysrep = &gRankedSearchResults[i];
		if (len == mysrep->resultLen && !mywcsncmp(text, mysrep->predText, len))
		{
			mysrep->cascadingPref = max(mysrep->cascadingPref, cascPref);
			return true;
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// inserts a text in one of the final search result entries in a unique way. 
bool CDictManager::addUniqueToFinalFive(MYWCHAR *text, PredictionType predType, USHORT pref)
{
	WLBreakIf(!text || text[0]==NUL, "!!ERROR! addUniqueToFinalFive! empty text entered!\n");
	if (UpdateInFinalFive(text, pref)) // look for the entry
		return FALSE; // is already there, update it and get out!

	if (PWresult.nApprovedResults < NWORDPREFERENCES) 
	{
		int startIdx =  sNumAllDictSearchSlots+NWORDPREFERENCES; 
		SearchResultEntry *freesrep = &gRankedSearchResults[startIdx+ PWresult.nApprovedResults];
		fillSrep(freesrep, text, false);
		freesrep->predType = predType;
		freesrep->cascadingPref = pref;
		PWresult.nApprovedResults++;
		sReplacedOneCacheItem = true;
		return true;
	}
	else
	{
		if(!sReplacedOneCacheItem && replaceInFinalFive(text, predType, pref))
			return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// This codes pick MAX_NUM_PREDICTIONS of next words predictions from the first dictionary which
// can provide them! TODO: we should change it to pick the top MAX_NUM_PREDICTIONS of next words 
// from all dictionaries. This requires WordsArray has smarts to insert elements in order of preferences.
static WordsArray sNextWordsAr;
MYWCHAR** CDictManager::findNextWords(int *num)
{
	int count =0;
	sNextWordsAr.reset();
	for(int i=0; i<m_nOrderedDicts; i++)
	{
		Dictionary *dict = getDictionaryEnabled(i);
		if (dict) 
		{
			ExtPathNode *extPathNode = gWPathway[gWordProp->nPathNodes].dictNode[dict->GetDictIndex()];
			if(extPathNode)
			{
				ExtCNode *extCNodes = extPathNode->extCNodes;
				for (int k = 0; k < extPathNode->nExtCNodes; k++)
				{
					CompactNode *node = extCNodes[k].cNode;
				//	if(node && CCompactStore::isEndpoint(node) && node->NextCount > 0 && extCNodes[k].layer() ==0)
					{
						count = dict->findNextWords(node, &sNextWordsAr, count);
					}
				}
			}
		}
	}
	*num = min(count, MAX_NUM_PREDICTIONS);
	return sNextWordsAr.nextWords;
}

//////////////////////////////////////////////////////////////////////////////
void CDictManager::fillFiveMostPreferredWords() 
{
	memset(mMostPreferredWords, 0, sizeof(mMostPreferredWords));
	int startIdx =  sNumAllDictSearchSlots+NWORDPREFERENCES; 
	int endIndex = startIdx + PWresult.nApprovedResults;
	for (int i = startIdx, di=0; i < endIndex; i++) 
		mMostPreferredWords[di++] = gRankedSearchResults[i].predText;

	ShowInfo("fillFiveMostPreferredWords: (dictId, endpref, word) (nApprovedResults=%d: \n", PWresult.nApprovedResults);
	for (int j = startIdx; j < endIndex; j++)
		ShowInfo("(%d,%d, #%s#),", gRankedSearchResults[j].dictIdx, gRankedSearchResults[j].endPref, toA(mMostPreferredWords[j - startIdx]));
}
///////////////////////////////////////////////////////////////////////////////////////
MultiLetterAdvanceAr *CDictManager::multiNextWords( MYWCHAR *rootWord, bool doNext, bool bUpdateTGraph)
{
	WLBreakIf(rootWord==NULL, "!!ERROR! CDictManager::nextWords: rootWord data not set!\n");
	//ShowInfo("MK multiNext rootWord:(%s)\n",toA(rootWord));
	constructCurWord(rootWord);

	ShowInfo("multiNextWords: gHistoryWordIdx=%d, gWorkingWordIdx=%d, nPath=%d, nNull=%d, exSP=%d, curr=#%s#\n", 
		     gHistWordIdx, gWorkingWordIdx, gWordProp->nPathNodes, gWordProp->nNullMoves, gWordProp->nSP,  toA(rootWord));
	mMultiLetterNexts.reset();
	MYWCHAR runningRootWord[MAX_WORD_LEN];
	for(int count =0; count <NCURWORDS; count++)
	{
		if(getWorkingWordLength(count) ==0) 
		{
			gWorkingWordPaths[count].pref = 0xff - (count==0);
			if(count != 0)
				continue;
		}
		goToWorkingWordPath(count); // switch to this working path:
		int numNextWordsPerLetter = 0;
		GetNextWords(&numNextWordsPerLetter, runningRootWord, doNext); 
		updateMultiLetterNextWords(1, NULL, true, true, bUpdateTGraph); //adjust multiletter next word array based on this nextwords:
	}

	mMultiLetterNexts.setActualNexts(); 
	goToWorkingWordPath(0); // switch to this working path:

	for(int i=0; i<mMultiLetterNexts.nActualNexts; i++)
		ShowInfo("--: next: #%s#, ", toA(mMultiLetterNexts.nextWords[i]));
	ShowInfo("\n");

	return &mMultiLetterNexts;;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
MYWCHAR **CDictManager::GetNextWords(int *nPosWordsPtr, MYWCHAR *rootWord, bool doNext, MYWCHAR* letters) 
{
	clearRankedSearchResults();
	WLBreakIf(rootWord==NULL, "!!ERROR! CDictManager::nextWords: rootWord data not set!\n");
	constructCurWord(rootWord);

	//ShowInfo("MK test2 getNextWOrds:(%s)\n", toA(rootWord));
	ShowInfo("nextWords: gHistoryWordIdx=%d, gWorkingWordIdx=%d, nPath=%d, nNull=%d, exSP=%d, sNumBackouts=%d, curr=#%s#\n", 
		     gHistWordIdx, gWorkingWordIdx, gWordProp->nPathNodes, gWordProp->nNullMoves, gWordProp->nSP, sNumBackouts, toA(rootWord));

	*nPosWordsPtr = 0;
	bool doInternalNext = true;
	if(emptyWordPath() || gWordProp->nNullMoves > 0 || m_noDictionariesLoaded)
	{
		ShowInfo("---!!!!emptyWordPath==%d or nNullMoves(%d) > 0, or m_noDictionariesLoaded(%d), so  don't doInternalNext!\n", 
			        emptyWordPath(), gWordProp->nNullMoves, m_noDictionariesLoaded);
		doInternalNext = false;
	}

	if (doInternalNext)
		mngrInternalNextWords(rootWord); 

	*nPosWordsPtr = addNextAndLearnedPredictions(doNext, letters);

	if(sDoFillFinalList)
	{
		fillFiveMostPreferredWords();	
		*nPosWordsPtr = PWresult.nApprovedResults;
		//for(int i =0; i<*nPosWordsPtr; i++)
			//ShowInfo("--return from NextWords: %d=%s\n", i, toA(mMostPreferredWords[i]));
	}


	ShowInfo("\n-nextWords: returning gHistWordIdx=%d, gWorkingWordPathIdx=%d, nPath=%d, nNull=%d, sNumBackouts=%d\n",
				gHistWordIdx, gWorkingWordIdx, gWordProp->nPathNodes, gWordProp->nNullMoves, sNumBackouts);
	return mMostPreferredWords;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDictManager::mngrInternalNextWords(MYWCHAR *root) 
{
	ShowInfo("mngrInternalNextWords\n");
	
	setEVerbPrefix();

	BYTE eVerbVarIdx = 0xff;
	int nTrailingChars = 0;
	BYTE eVerbDictIdx = isNodeAnEVerbEnding(0, m_nOrderedDicts, &nTrailingChars, &eVerbVarIdx);
	if (eVerbDictIdx != INVALID_DICT_IDX && nTrailingChars > 0) 
	{
		ShowInfo("--mngrInternalNextWords: everb ending : dictIdx=%d, nTrailingChar=%d\n", eVerbDictIdx, nTrailingChars);
		//		setEVerbPrefix(L"e");
		WLBreakIf(nTrailingChars >= 6, "!!ERROR!!mngrInternalNextWords: everb length > 6? Ignore the excess chars!! \n");
		MYWCHAR trailingChars[6];
		getNLastCharacters(trailingChars, nTrailingChars);
		setEVerbPrefix(trailingChars, eVerbVarIdx);
	}

	//wipeoutBreadCrumbs();
	// scan now for next words. 
	mngrInternalNextWordsInDictionaries(0);
	if(gWordProp->maxLayerId > 0 && gWordProp->endsInDicts)
	{
		for(int layer=gWordProp->maxLayerId; layer>0; layer--)
		{
			if(gWordProp->layerStartPos[layer] <= gWordProp->nPathNodes)
				mngrInternalNextWordsInDictionaries(layer);
		}
	}
	

	if (eVerbDictIdx >= 0 && nTrailingChars > 0) // all words continuing from 'e' have higher prio than everb derivatives
	{
	//	weakenRepresentedNodes( getEVerbPrefixLen()); // we might get some everb results, which are considered more valuable than already represented nodes
		BYTE *pref = NULL; //[MAX_NUM_DICTS*NEVERBCASES];	// store the original pref temporarily here as we terminated this for everbs.  N strains Cases supported per EVERB 
		takeOutTheEVerb(eVerbDictIdx, pref); // we don't reset the summarized results, whatever was retrieved will not be simply outnumbered by the eVerb findings.

		for (int layer = 0; layer <= gWordProp->maxLayerId; layer++)
			mngrCollectEVerbsInDictionaries(eVerbDictIdx, eOrdinary, layer, eVerbVarIdx);

		// NOTE: only when we actually find a result on an EVerb we adjust the text pointers
		if (gWordProp->nSP == 0) // take out the EVerbPrefix
		{
			int nCharsInRootWord = mywcslen(root);
			if (nCharsInRootWord - getEVerbPrefixLen() >= 0)
				root[nCharsInRootWord - getEVerbPrefixLen()] = NUL;
		}

		putTheEVerbBackIn(eVerbDictIdx, pref);
		putTheStrongestUniqueFiveInFront();
	}
	else
	{
		//Minkyu:2013.09.18
		//This should be called only once no matter what kind of verbs is.
		putTheStrongestUniqueFiveInFront();
	}

	setEVerbPrefix(); // clean out the prefix

	applyClingonMethodOnResults();

	// store the strings as they were found in the retrieval of the next Words.
	// isChunk and /or advanceWord will work off of these prediction strings from now on
	storePredictedNextWordsResults();
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// adds next words based on current word as well as learned frequently  words, to the current 
// predicted next words. Main challenge is to estimate correct preference for these additions
// returns number of added next words.
int CDictManager::addNextAndLearnedPredictions(bool doNext, MYWCHAR* letters) 
{
	//ShowInfo("MK addNextAndLearnedPredictions history1:(%s)\n",toA(m_History.NewCurrentHistory));
	//ShowInfo("MK addNextAndLearnedPredictions history2:(%s)\n",toA(m_History.HistoryForPhrase));
	//ShowInfo("MK addNextAndLearnedPredictions history3:(%s)\n",toA(m_History.BackSpaceHistory));
	//ShowInfo("MK addNextAndLearnedPredictions back:%d\n",m_History.nDeleteFirstWordIndex);
	//ShowInfo("MK addNextAndLearnedPredictions letters:(%s)\n",toA(letters));

	unsigned len = 0;
	MYWCHAR *curWord = getCurrentWord(len, true);
	//ShowInfo("MK addNextAndLearnedPredictions: currWord=#%s#, len=%d, doNext=%d\n", toA(curWord), len,doNext);

	int numLearnedPredictions = 0;
	MYWCHAR **learnedPredictions = NULL;
	USHORT prefs[MAX_NUM_PREDICTIONS];
	memset(prefs, 0, sizeof(prefs));
	int numAddedPredictions = 0;

	int nextWordsCount = 0;
	int numlearnedNextWords = 0;
	MYWCHAR **nextwords = NULL;
	MYWCHAR **nextLearnedWords = NULL;
	
	// if len>0 => look for learned predictions based off of current chunk, if nothing! look for nextWords in dict, if possible, and then in cache!\n");
	if(len > 0 && !isNumber(curWord) && !IsOrdinalNumbers(curWord))
	{
		ShowInfo("--len>0 => in a word => look for learned predictions!\n");
		learnedPredictions = mUserWordCache->findPredictionsStartingWith(curWord, prefs, &numLearnedPredictions);
		if(doNext && numLearnedPredictions==0 && len > 1) // means there are no word prediction in cache based on current chunk. Check for next words now:
		{
			ShowInfo("---numLearnedPredictions==0 so go for next words in dict if possible: nullmove(%d), nSP(%d), isEndpoint(%d)\n", 
				gWordProp->nNullMoves, gWordProp->nSP, isEndpoint(gWordProp->nPathNodes));
			if( (gWordProp->nNullMoves==0 || (gWordProp->nNullMoves==1 && gWordProp->nSP==1)) && isEndpoint(gWordProp->nPathNodes) )
				nextwords = findNextWords(&nextWordsCount);
			else
			{
				sNextWordsAr.reset();
				nextwords = sNextWordsAr.nextWords;
			}
			ShowInfo("---now go for learned next words:\n");
			nextLearnedWords = mUserWordCache->GetPossibleFollowWords(curWord, len, &numlearnedNextWords);
		}

		if(!isEmptyStr(curWord))
		{
			BuildTGraph(curWord);
		}
	}
	else if(doNext)// len=0 => word ended, => go to prev word and look for nextWords: first in dict, then in cache!
	{
		ShowInfo("--len=0 => go to prev word and look for nextWords: first in dict, then in cache!\n");
		if(BackToPreviousWordIfAny())
		{
			if( gWordProp->nNullMoves == 0 || (gWordProp->nNullMoves==1 && gWordProp->nSP==1) || (letters && letters[0] == SP))
			{
				nextwords = findNextWords(&nextWordsCount);
				if(!isEmptyStr(curWord))
				{
					BuildTGraph(curWord);
				}
			}
			//Minkyu:2014.05.09
			//DO NOT UNCOMMENT THIS LINE!!
			//len is alwyas zero and it won't find learned word from findPunctuationChoiceAtEndOfWord(curWord[len-1])!!
			len = constructCurWord(curWord);
			///////////////////////////////////////////////////////////////////////////////////////////////////////////
			ShowInfo("---Now find learned next words after curWord=#%s# :\n ", toA(curWord));
			if(!findPunctuationChoiceAtEndOfWord(curWord[len-1]))
			{
				if(!isNumber(curWord) && !IsOrdinalNumbers(curWord))
				{
					nextLearnedWords = mUserWordCache->GetPossibleFollowWords(curWord, len,  &numlearnedNextWords);
					ShowInfo("-- Found %d learned next words in cache!\n", numlearnedNextWords);
				}
			}
			BackToWorkingWord();
		}
	}

	// Now mix the results for learned and dictionary results:
	if( nextWordsCount > 0 || numlearnedNextWords>0)
	{
		ShowInfo("--nextWordsCount=%d, numlearnedNextWords=%d: \n", nextWordsCount, numlearnedNextWords);
		// first, make sure if there are learned next words, at least one of them makes it into final result:
		if(numlearnedNextWords>0 && nextWordsCount == MAX_NUM_PREDICTIONS)
			nextWordsCount--; 
		// next, add dictionary next words:
		sReplacedOneCacheItem = false;

		for(int i=0; i<nextWordsCount; i++)
			numAddedPredictions += addUniqueToFinalFive(nextwords[i], eDictNextPredict, LOW_PREFERENCE+nextWordsCount-i);
		
		// finally, add learned next words:
		for(int i=nextWordsCount; (i-nextWordsCount)<numlearnedNextWords && i<MAX_NUM_PREDICTIONS; i++)
		{
			numAddedPredictions += addUniqueToFinalFive(nextLearnedWords[i-nextWordsCount], eLearnNextPredict, LOW_PREFERENCE+numlearnedNextWords+nextWordsCount-i);
			
		}
	}

	if (numLearnedPredictions > 0) 
	{
		sReplacedOneCacheItem = false;
		for (int i = 0; learnedPredictions[i] && i<numLearnedPredictions; i++)
		{
			ShowInfo("addLearnedPredictions: adding learnedPrediction #%s#\n", toA(learnedPredictions[i]));
			if (prefs[i])
			{
				numAddedPredictions += addUniqueToFinalFive(learnedPredictions[i], eLearnWordPredict, prefs[i]);
			}	 
		}
	}
	
	return numAddedPredictions;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// checks if previous word is a phrase. Returns the char location of last word
MYWCHAR* previousWordContainsSubWord()
{
	MYWCHAR * ret = NULL;
	if(BackToPreviousWordIfAny()) 
	{  
		//if(gWordProp->nSP > 0 && !(gWordProp->nSP==1 && *gWordProp->charsBufP==SP))
		if(*gWordProp->charsBufP==SP)
		{
			unsigned len = 0;
			ShowInfo("prevWordPartOfPhrase: previous word=#%s#\n", toA(getCurrentWord(len)));
			//int offset = gWordProp->charsBufP - &gWordProp->charsBuf[0];
			MYWCHAR *lastChar = gWordProp->charsBufP-1;
			while(lastChar != gWordProp->charsBuf && *lastChar != SP)
				lastChar--;
			
			if(lastChar != &gWordProp->charsBuf[0])
				ret = &lastChar[1];
		}
		BackToWorkingWord();
	}

	return ret;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDictManager::mngrCollectEVerbsInDictionaries(int eVerbDictIdx, WordValue wordValue, int layer, BYTE eVerbVarIdx) 
{
	gWordProp->wordValue = wordValue;
	Dictionary *existDict = getDictionaryEnabled(eVerbDictIdx);
	WLBreakIf(!existDict, "!!ERROR!! mngrCollectEVerbsInDictionaries: dictionary is null!!\n") ;
	existDict->collectEVerbs(wordValue, layer, eVerbVarIdx);
}

//////////////////////////////////////////////////////////////////////////////////////
// note : this is a hint to put encapsulation back in , so not as partofOtherWords for instance
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CDictManager::isNodeAnEVerbEnding(int startDictIdx, int endDictIdx, int *nTrailChars, BYTE *eVerbVarIdx) 
{
	for (int j = startDictIdx; j < endDictIdx; j++) 
	{
		Dictionary *existDict = getDictionaryEnabled(j);
		WLBreakIf((existDict && j!=existDict->GetDictIndex()), "!!ERROR!! ? isNodeAnEVerbEnding::j!=existDict->GetDictIndex()\n");
		if (existDict && gWordProp->existInDicts[j]) 
		{
			if (existDict->isNodeAnEVerbEnding(nTrailChars, eVerbVarIdx))
				return j;
		}
	}
	return INVALID_DICT_IDX;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDictManager::takeOutTheEVerb(int eVerbDictIdx, BYTE pref[]) 
{
	// this is a simple version, theoretically spoken it should be done on all the supplemental and cascading dictionaries
//#if 0
//	int startDictIdx = eVerbDictIdx;
//	int nPath = gWordProp->nPathNodes - getEVerbPrefixLen() + 1;
//
//	if (m_personalDictIdx != INVALID_DICT_IDX && eVerbDictIdx < 2)
//		startDictIdx = 0;
//
//	for (int j = startDictIdx; j < m_nOrderedDicts; j++)
//	{
//		Dictionary *existDict = getDictionaryEnabled(j);
//		if (!existDict)
//			continue;
//
//		existDict->takeOutTheEVerb(nPath, &pref[j*NEVERBCASES]);
//	}
//#endif
	m_eVerbRetrievalActive = TRUE;
	setEVerbPrefixLen( getEVerbPrefixLen());
	mywcscpy(m_eVerbPrefixStrWordPath, getEverbPrefix()); // set this globally until we decided whether support multiple everbs on 1 search, right now we dont !!
	gWordProp->nPathNodes -= getEVerbPrefixLen();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDictManager::putTheEVerbBackIn(int eVerbDictIdx, BYTE originalPref[]) 
{
	// this is a simple version, theoretically spoken it should be done
	// on all the supplemental and cascading dictionaries
	gWordProp->nPathNodes += getEVerbPrefixLen();
	setEVerbPrefixLen(0);
	m_eVerbRetrievalActive = FALSE;

#if 0
	int startDictIdx = eVerbDictIdx;
	for (int j = startDictIdx; j < m_nOrderedDicts; j++)
	{
		Dictionary *existDict = getDictionaryEnabled(j);
		if (existDict)
			existDict->putTheEVerbBackIn(gWordProp->nPathNodes, &originalPref[j*NEVERBCASES]);
	}
#endif
}

/////////////////////////////////////////////////////////////////////////////
BYTE CDictManager::isChunk(int dictNode)
{
	BYTE ret = 0;
	for (int j = 0; j < m_nOrderedDicts; j++)
	{
		Dictionary *existDict = getDictionaryEnabled(j);
		if (existDict && existDict->isChunk(dictNode)) 
			turnOnBit(&ret, j);
	}
	return ret;
}

/////////////////////////////////////////////////////////////////////////////
BYTE CDictManager::isEndpoint(int dictNode)
{
	BYTE ret = 0;
	for (int j = 0; j < m_nOrderedDicts; j++)
	{
		Dictionary *existDict = getDictionaryEnabled(j);
		if (existDict && existDict->isEndpoint(dictNode)) 
			turnOnBit(&ret, j);
	}
	return ret;
}