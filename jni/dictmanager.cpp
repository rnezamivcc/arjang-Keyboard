// <copyright file="dictmanager.cpp" company="WordLogic Corporation">
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
// <summary>Contains support for managing general features of dictionaries, such as adding/removing words,learning....</summary>

#include "stdafx.h"
#include "dictmanager.h"
#include "dictionary.h"
#include "wordpunct.h"
#include "userWordCache.h"
#include "userWordCacheOffline.h"
#include "searchResults.h"
#include "wordpath.h"
#include "T-Graph.h"

#ifndef _WINDOWS
#include <cctype>
#endif

DictionaryConfigurationEntry	CDictManager::m_orderedDictList[MAX_NUM_DICTS];
USHORT	CDictManager::m_nOrderedDicts = 0;
bool sEndofSentenceReached = true;
BYTE sWordPosInPhrase = 0;

extern BYTE sNumAllDictSearchSlots;    // this is == sMaxSearchRanks - 10



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// return the list of words in a dictionary. Parameter "count" is to be filled by this function as the number of words in the list 
MYWCHAR *CDictManager::getWordList(int dictIdx, int *count)
{

	//Dictionary *dict = getDictionaryEnabled(dictIdx);
	//if(dict)
	//{
	//	CCompactStore *compactStore = dict->getCompactStore();
	//	return compactStore->getWordList(count);
	//}
	
	Dictionary *dict = getTopDictionary();
	if(dict)
	{
		//ShowInfo("MK getWordList return count(%d)\n",*count);
		return dict->getCompactStore()->getWordList(count,NULL);
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
MYWCHAR *CDictManager::undoLetterOrWord() 
{
	ShowInfo("UndoLetterOrWord()\n");
	if (emptyWordPath()) 
	{
		BackToPreviousWord(true);
		if (emptyWordPath())
		{
			ShowInfo("CDictManager::undoLetterOrWord emptywordpath() = false! return current word.\n"); 
			return EMPTYSTRING;
		}
	}

	int nUndoMoves = gWordProp->nUndoMoves[gWordProp->nPathNodes];
	ShowInfo("CDictManager::undoLetterOrWord: nPathNodes:%d, nUndoMove:%d, sNumBackouts=%d \n", gWordProp->nPathNodes, nUndoMoves, sNumBackouts);
	//clearRankedSearchResults();
	MYWCHAR root[64];
	for (int i = 0; i < nUndoMoves; i++)
	{
		backspaceLetter(root, false);
	}
	goToWorkingWordPath(0); // switch to this working path:

/*** for now we don't know if we need this?!	if (gWordProp->nEVerbChars>0 && gWordProp->nPathNodes==gWordProp->nEVerbChars && gWordProp->nNullMoves==0 && gWordProp->nSP==0) 
	{
		MYWCHAR printChunkJunk[MAX_WORD_LEN];
		MYWCHAR *prefix = m_eVerbPrefixStrWordPath;
		int prefixLen = mywcslen(prefix);

		gWordProp->nEVerbChars = 0;
		for (int k = 0; prefix[k]; k++) 
		{
			mngrAdvanceLetter(prefix[k], printChunkJunk);
			// the only way we got into EVerbs is thru softletters (gestured !!), so restore that
			gWordProp->nUndoMoves[gWordProp->nPathNodes] = gWordProp->nEVerbUndoMoves - prefixLen + k + 1;
		}
	} */
	unsigned len = 0;
	return getCurrentWord(len);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
MultiLetterAdvanceAr *CDictManager::backspaceLetter(MYWCHAR *rootWord, bool donext)
{
	//ShowInfo("MK backspaceLetter rootWord:(%s)\n",toA(rootWord));
	//ShowInfo("MK backspaceLetter history1:(%s)\n",toA(m_History.NewCurrentHistory));
	//ShowInfo("MK backspaceLetter history2:(%s)\n",toA(m_History.HistoryForPhrase));
	//ShowInfo("MK backspaceLetter history3:(%s)\n",toA(m_History.BackSpaceHistory));
	//ShowInfo("MK backspaceLetter donext:(%d)\n", donext);


	m_History.nEraseLastWord =0;
	unsigned n = mywcslen(m_History.NewCurrentHistory);
    if(n > 0)
        m_History.NewCurrentHistory[n-1] = NUL;

	m_History.SetStartingWordMode(true);
	m_History.SetUpdateAfterNumbers();

	MYWCHAR *lastWordPart = GetLastWordFromPhrase(m_History.NewCurrentHistory);
	MakeLowerCase(lastWordPart);

	bool bGetStartWord = (isEmptyStr(lastWordPart) && n > 0);

	int dictIdx =0;
	CompactNode* endnode = retrieveEndNodeForString(lastWordPart, &dictIdx, true);
	bool bUpdateTGraph = (endnode && CCompactStore::isEndpoint(endnode));
	//ShowInfo("MK backspaceLetter lastWord:(%s), bGetStartWord:%d, bUpdateTGraph:%d\n",toA(lastWordPart), bGetStartWord, bUpdateTGraph);

	unsigned len = 0;
	ShowInfo("backspaceLetter donext=%d, gHistWordIdx=%d, gWorkingWordIdx=%d, nPath=%d, nNUll=%d, exSP=%d, currWord=#%s#, sNumBackouts=%d\n", 
		donext, gHistWordIdx, gWorkingWordIdx, gWordProp->nPathNodes, gWordProp->nNullMoves, gWordProp->nSP, toA(getCurrentWord(len)), sNumBackouts);

	mMultiLetterNexts.reset();
	MYWCHAR* curWord = getCurrentWord(len);
	//ShowInfo("MK backspaceLetter curWord:(%s)\n",toA(curWord));
	if (emptyWordPath()) 
	{
		ShowInfo("-emptyWordPath! try going to prev word, then call backspaceLetter() again!\n");
		BackToPreviousWord(true);
		if (emptyWordPath()) 
		{
			clearRankedSearchResults();
			ShowInfo("--still emptyWordPath ? return!\n"); 

			if(bGetStartWord)
				UpdateForStartWords();
			
			return &mMultiLetterNexts;
		}
		//ShowInfo("MK backspaceLetter unLearn lastWord:(%s)\n",toA(lastWord));
		mUserWordCache->unLearn(curWord);	
	}

	// now go over all active paths and backspace on each:
	
	for(int count =0; count <NCURWORDS; count++)
	{
		if(getWorkingWordLength(count) ==0)
		{
			gWorkingWordPaths[count].pref = 0xff - (count==0);
			continue;
		}
		goToWorkingWordPath(count); // switch to this working path:
		mngrInternalBackSpaceLetterDictionaries( false);
		ShowInfo("--backspaceletter: workingId:%d, currWord=#%s#\n", gWorkingWordIdx, toA(curWord));
	}

	pruneWorkingPaths();
	if(gWordProp->nNullMoves > 0)
	{
		//Minkyu:2013.10.08
		//Do not learn while backspacing because it will start learning weird word right away.. 
		//Ex)"miaundersyanding"-->misunderstanding===>if you backspacing from this word, it starts learning "miaundersyand"..etc.
		goToNextWord((char*)"backspaceLetter: nNullMoves > 0", false, false);
		//goToNextWord("backspaceLetter: nNullMoves > 0", NULL); 
	}
	//now find next words:
	if(donext && bGetStartWord)
		multiNextWords(rootWord, true,bUpdateTGraph);		
	
	if(bGetStartWord)
		UpdateForStartWords();

	// now do autocorrect:
	//DoAutoCorrect();

	sNumBackouts = gWordProp->nPathNodes + gWordProp->nNullMoves;
	ShowInfo("-backspaceLetter return, gHistWordIdx=%d, currWord=#%s#, sNumBackouts=%d\n", gHistWordIdx, toA(getCurrentWord(len)), sNumBackouts);
	return &mMultiLetterNexts;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDictManager::ResetTGraph(bool bAll)
{
	if(m_TGraph)
	{
		delete m_TGraph;
		m_TGraph = NULL;
	}

	if(bAll)
	{
		if(m_NGramLearning)
		{
			delete m_NGramLearning;
			m_NGramLearning = NULL;
		}
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
MultiLetterAdvanceAr *CDictManager::eraseLastWord( MYWCHAR* rootWord, unsigned &numLetterBackedOut)
{
	//ShowInfo("MK eraseLastWord history1:(%s)\n",toA(m_History.NewCurrentHistory));
	//ShowInfo("MK eraseLastWord history2:(%s)\n",toA(m_History.HistoryForPhrase));
	//ShowInfo("MK eraseLastWord rootWord:(%s)\n",toA(rootWord));

	m_History.nEraseLastWord++;
	numLetterBackedOut = 0;
	mMultiLetterNexts.reset();
	if (emptyWordPath()) 
	{
		BackToPreviousWord(true);
		if (emptyWordPath()) 
		{
			ShowInfo("!!WARNING!! eraseLastWord: check it out! why word still empty? ignore for now!!\n");
			return &mMultiLetterNexts;
		}
	}
	MYWCHAR *curWord = getCurrentWord(numLetterBackedOut, false);
	ShowInfo("eraseLastWord: erasing word #%s# with length = %d\n", toA(curWord), numLetterBackedOut);

	mUserWordCache->unLearn(curWord);
	resetWorkingWordPath(gWorkingWordIdx);

	//Minkyu:2014.01.28
	//Commented out because the engine shows the prediciton even if no root word exists when erasing the sentences or swiping deletion.
	//multiNextWords(rootWord, true);

	sNumBackouts = 0;
	m_History.ResetHistoryPhrases();

	//ShowInfo("MK eraseLastWord:(%s), history:(%s)\n",toA(curWord),toA(m_History.NewCurrentHistory));
	return &mMultiLetterNexts;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDictManager::mngrInternalBackSpaceLetterDictionaries(BOOL clearResults)
{	
	if (gWordProp->nPathNodes > 0 && gWordProp->nNullMoves == 0) 
	{
		wipeOutNode(gWPath->pathway, gWordProp->nPathNodes);
	}

	gWordProp->removeFromWordBuf();
//	if(emptyWordPath())
//		BackToPreviousWord(true);
	if(clearResults)
		clearRankedSearchResults();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// entry point for learning a word in cache and dictionaries. As user uses some words more often, these words get better
// treatment with respect to predictablity, the more and more they are used!
bool CDictManager::learnCurrentWord(MYWCHAR* inputWord, bool endphrase)
{
	if(inputWord)
	{
		return mngrLearnWord(inputWord, endphrase);
	}

	ShowInfo("learnCurrentWord: gHistWordIdx=%d, ", gHistWordIdx);
	if(gWordProp->nSP>=2)
		return false;
	unsigned len = 0;
	MYWCHAR *currword = getCurrentWord(len);
	ShowInfo("learning #%s#\n", toA(currword));

	if(currword && currword[0] != NUL)
		return mngrLearnWord(currword, endphrase);

	return false;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDictManager::LearnMultiGramWord(MYWCHAR* input)
{
	if(m_History.bBackSpace || !m_NGramLearning || m_History.bReplaced || getTopDictionaryId() != eLang_ENGLISH)
		return;

	
	MYWCHAR		word[MAX_TGRAPH_HISTORY_NUM][MAX_WORD_LEN];
	memset(word, 0, sizeof(word));

	int sp  =0;
	if(input)
	{
		int index =0;
		for(int i=0; i < MAX_PHRASE_LEN && input[i]; i++)
		{
			if(input[i] == SP)
			{
				sp++;
				index =0;
				continue;
			}

			word[sp][index++] = input[i];
		}
	}
	else
	{
		sp = m_History.GetWordFromHistory(word);
	}

	ShowInfo("MK LearnMultiGramWord:%s %s %s %s\n",toA(word[0]),toA(word[1]),toA(word[2]),toA(word[3]));
	

	if(sp > 0)
		m_NGramLearning->Learn2Gram(word);

	if(sp == MAX_TGRAPH_HISTORY_NUM-1)
		m_NGramLearning->Learn3Gram(word);		
	else if(sp == MAX_TGRAPH_HISTORY_NUM)
		m_NGramLearning->Learn4Gram(word);	
	
}			
/////////////////////////////////////////////////////////////////////////////////////////////
void CDictManager::ProcessLearningOffline(const char* filePath)
{
	//Minkyu:2013.09.23
	WLBreakIf(filePath == NULL, "!!ERROR! Offline file path is NULL!\n");
	WLBreakIf(m_pUserWordCacheOffline == NULL, "!!ERROR! m_pUserWordCacheOffline is NULL!\n");

	char* parsingSpec = (char*)"sp";
	bool bComplete = m_pUserWordCacheOffline->ReadOfflineFile(filePath, parsingSpec, false);
	if(bComplete)
	{
		m_TGraph->reset();
		m_NGramLearning = new NGramLearning();
	}
		
	WLBreakIf(!bComplete, "!!ERROR! StartLearnFromFile failed!\n");
	
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This one dynamically modifies given word's end preference in  dictionary to reflect user's preference for this word.
BOOL CDictManager::mngrLearnWord(MYWCHAR *thisword, bool endphrase) 
{

	if(m_History.bReplaced)
	{
		return false;
	}

	//ShowInfo("MK mngrLearnWord1:(%s)\n\n",toA(m_History.NewCurrentHistory));
	//ShowInfo("MK mngrLearnWord2:(%s)\n\n",toA(m_History.HistoryForPhrase));
	//ShowInfo("MK mngrLearnWord: learning #%s# gHistWordIdx=%d\n", toA(thisword), gHistWordIdx);
	trimEndingSpaces(thisword, true);

	for(int i=0; i <MAX_WORD_LEN && thisword[i]; i++)
	{
		if(isPunctuation(thisword[i]))
			thisword[i] = NUL;
	}

	if(containsSPs(thisword))
	{
		ShowInfo("!!Warning! mngrLearnWord: at this time we don't learn multi-word strings! so ignore learning \"%s\"\n", toA(thisword));
		return false;
	}

	//int excessLettersRemoved = declutterWord(thisword, gWordProp->nPathNodes);
	int len = mywcslen(thisword);
	if( len < 1) // we don't learn shorter than 2 letter words!
		return FALSE;

	int dictIdx = 0;
	CompactNode* endnode = retrieveEndNodeForString(thisword, &dictIdx, true);
	if(!endnode)
	{
		dictIdx = 0;
		endnode = QuadRetrieveCompactNode(thisword, true, &dictIdx);
	}

	if(endnode) // so the word is in one of the dictionaries. Adjust the weights there
	{
		BYTE endpref = getCompactStore(dictIdx)->getEndPreference(endnode);
		getCompactStore(dictIdx)->setEndPreference(endnode, min(endpref+1, MAXIMUM_PREFERENCE));
		mUserWordCache->putWord(thisword, MEDIUM_PREFERENCE, 0, len, endnode, endphrase);
		//ShowInfo("MK mngrLearnWord thisword1:(%s)\n\n",toA(thisword));
	}
	else
	{
		mUserWordCache->putWord(thisword, MEDIUM_PREFERENCE, 0, len, NULL, endphrase);
		//ShowInfo("MK mngrLearnWord thisword2:(%s)\n\n",toA(thisword));
		
	}
	//else // otherwise update it in cache.
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// given a word, this function finds the node path in the first dictionary it can find a path  for it.
// returns the end node and also sets the dictionary index of the containing dictionary.
////////////////////////////////////////////////////////////////////////////////////////////////////////
CompactNode * CDictManager::retrieveEndNodeForString(MYWCHAR *word, int *dictIdx, bool isEndpoint)
{
	if(!word || word[0] == NUL || SP_CR_TAB(word[0]))
		return NULL;

	for (int i=0; i < m_nOrderedDicts; i++)
	{
		Dictionary *dict = getDictionaryEnabled(i);
		if (dict) 
		{
			CompactNode *endNode = dict->retrieveEndNodeForString(word, isEndpoint);
			if(endNode)
			{
				*dictIdx = dict->GetDictIndex();
				return endNode;
			}
		}
	}
	
	return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// given a word, this function checks to see if this word exists in any of the dictionaries.
// If so, returns true along with the dictionary idex and word probability.
///////////////////////////////////////////////////////////////////////////////////////////////////////
int CDictManager::isWordInDictionaries(const MYWCHAR *word, int &dictIdx)
{
	dictIdx = -1;
	if(!word || word[0] == NUL || SP_CR_TAB(word[0]))
		return 0;

	for (int i=0; i < m_nOrderedDicts; i++)
	{
		Dictionary *dict = getDictionaryEnabled(i);
		if (dict) 
		{
			CompactNode *endNode = dict->retrieveEndNodeForString(const_cast<MYWCHAR*>(word), true);
			if(endNode)
			{
				dictIdx = dict->GetDictIndex();
				return dict->getEndPreference(endNode);
			}
		}
	}
	
	return 0;
}
///////////////////////////////////////////////////////////////////////////////////
bool CDictManager::accentEqual(MYWCHAR ch1, MYWCHAR ch2)
{
	MYWCHAR *trychars = GetCharCases(ch1, eLang_FRENCH); // use french just to get all accent variations! needs to be modified to become
	                                                     // more language specific.
	for (int i = 0; trychars[i]; i++)
	{
		if(trychars[i] == ch2)
			return true;
	}
	return false;
}
// this function should return base accent free version of ch. This means having to parse
// row of accents(utility.cpp) and pick the one containing ch, then return its base.
// 
MYWCHAR CDictManager::removeAccent(MYWCHAR ch)
{
	return GetCharRowBase(ch);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
// given a compactNode, this function finds the word path ending at this node in the first dictionary possible
////////////////////////////////////////////////////////////////////////////////////////////////////////
MYWCHAR *CDictManager::retrieveStringForEndNode(CompactNode *endNode)
{
	static MYWCHAR sStr[MAX_WORD_LEN];
	if(endNode==NULL)
		return NULL;

	for (int i=0; i < m_nOrderedDicts; i++)
	{
		Dictionary *dict = getDictionaryEnabled(i);
		if (dict) 
		{
			int len = dict->getCompactStore()->retrieveWordFromLastNode(endNode, sStr);
			if(len>0)
				return sStr;
		}
	}
	return NULL;
}
////////////////////////////////////////////////////////////////////////////////////
int CDictManager::getWordForEndNode(CompactNode *endNode, MYWCHAR *outWord)
{
	if(endNode==NULL)
		return 0;
	int len = 0;
	for (int i=0; i < m_nOrderedDicts; i++)
	{
		Dictionary *dict = getDictionaryEnabled(i);
		if (dict) 
		{
			len = dict->getCompactStore()->retrieveWordFromLastNode(endNode, outWord);
			if(len>0)
				break;
		}
	}
	return len;
}
///////////////////////////////////////////////////////////////////////////////////////
MYWCHAR *CDictManager::getWordFromNode(CompactNode *endNode)
{
	if(endNode==NULL)
		return 0;
	MYWCHAR *word = NULL;
	for (int i=0; i < m_nOrderedDicts; i++)
	{
		Dictionary *dict = getDictionaryEnabled(i);
		if (dict) 
		{
			word = dict->getCompactStore()->getWordFromNode(endNode);
			if(word)
				break;
		}
	}
	return word;

}
//////////////////////////////////////////////////////////////////////////////////////////////////////
// given a stemword, this functions fist tries to find a possible end node for it in any dictionary possible.
// If it manages, it uses the end node to find number of possible words
// starting with this word in that dictionary.
// returns number of possible words starting with stemword.
int CDictManager::getNumWordsStartingWith(MYWCHAR *stemword, int *dictId)
{
	int dictIdx;
	int ret = 0;
	MakeLowerCase(stemword);
	CompactNode *endNode = retrieveEndNodeForString(stemword, &dictIdx);
	if(endNode)
	{
		ret = getDictionaryEnabled(dictIdx)->getPossibleNumWords(endNode);
		if(dictId)
			*dictId = dictIdx;
	}
	return ret;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CCompactDict *CDictManager::getCompactDict(int dictIdx) 
{
	WLBreakIf(m_orderedDictList[dictIdx].existingDictionary == NULL, "!!ERROR!!! DictManager::getCompactDict:: dictIdx %d with name %s not loaded yet!\n", 
				dictIdx, Dictionary::GetDictName(m_orderedDictList[dictIdx].langIdx));
	return m_orderedDictList[dictIdx].existingDictionary->getCompactDict();
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CCompactStore *CDictManager::getCompactStore(int dictIdx) 
{
	WLBreakIf(m_orderedDictList[dictIdx].existingDictionary == NULL, "!!ERROR!!! DictManager::getCompactStore:: dictIdx %d with name %s not loaded yet!\n", 
				dictIdx, Dictionary::GetDictName(m_orderedDictList[dictIdx].langIdx));
	return m_orderedDictList[dictIdx].existingDictionary->getCompactStore();
}

/////////////////////////////////////////////////////////////////////////////////////////
BOOL CDictManager::CurrentWordCanbeAdded()
{
	bool canbeAdded = false;
	bool backtoprevword = BackToPreviousWordIfAny(); 
	unsigned len = 0;
	MYWCHAR *currentWord = getCurrentWord(len);
	int nLen = gWordProp->nNullMoves + gWordProp->nPathNodes;
	if(nLen > 2)
	{
		if (!isalnum(currentWord[nLen-1]) || wordEndsOnNormalPunctuation(currentWord[nLen-1])) 
			canbeAdded = gWordProp->nNullMoves > 1;
		else 
			canbeAdded = gWordProp->nNullMoves > 0;
	}
	if(backtoprevword)
		BackToWorkingWord();
	ShowInfo("CurrentWordCanBeAdded:: word=#%s#, can? %d\n", toA(currentWord), canbeAdded);
	return canbeAdded;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// adds a word to the active dictionary. If word's length is less than 2, addition is ignored. 
// It first checks to see if this word already exists in any other dictionary. If so, ignores the addition.
CompactNode* CDictManager::addWord(MYWCHAR *newWord, int pref, int *dictIdx) 
{
	ShowInfo(("MK CDictManager::addWord: word= #%s#\n"), toA(newWord));
	int wordLen = mywcslen(newWord);
	if (wordLen < 2) 
	{
		ShowInfo(("--addWord #%s# too short! Ignore it\n"), toA(newWord));
		//return false;
        return NULL;
	}

//	BOOL bAdded = CurrentWordCanbeAdded();
	if(retrieveEndNodeForString(newWord, dictIdx, true))
	{
		ShowInfo("--word #%s# already exists in dictionary %s\n", toA(newWord), Dictionary::GetDictName(m_orderedDictList[*dictIdx].langIdx));
		//return false;
        return NULL;
	}

	int newPreference = max(MEDIUM_PREFERENCE , pref) + wordLen; // for now give all new words this preference! make it intelligent later!
	return  mngrAddWord(newWord, newPreference, *dictIdx);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CompactNode* CDictManager::mngrAddWord(MYWCHAR *word, int pref, int dictIdx) 
{
	WLBreakIf(dictIdx < 0, "!!ERROR! CDictManager::mngrAddWord: dictIdx(%d) < 0 ?!\n", dictIdx); 
	ShowInfo("mngrAddWord: #%s#, pref=%d, dictIdx=%d \n", toA(word), pref, dictIdx);

	Dictionary* dict = getTopDictionary();
	dictIdx = dict->GetDictIndex();
	
	if (!m_orderedDictList[dictIdx].existingDictionary) 
	{
		return NULL;
		//char *dictname =  Dictionary::GetDictName(m_orderedDictList[dictIdx].langIdx);
		//ShowInfo("!!WARNING! dict %s doesn't exist! create it from scratch!\n", dictname);
		//m_orderedDictList[dictIdx].existingDictionary = createFromScratch(dictIdx,dictname);
		//if (!m_orderedDictList[dictIdx].existingDictionary)
		//	return NULL;
	}

	return m_orderedDictList[dictIdx].existingDictionary->addWord(word, pref);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CDictManager::deleteWord(MYWCHAR *deleteWord)
{
	ShowInfo("MK deleteCurrentWord entered:(%s)\n",toA(deleteWord));

	trimEndingSpaces(deleteWord);

	if (isEmptyStr(deleteWord))
		return FALSE;

	if(mUserWordCache)
	{
		mUserWordCache->unLearn(deleteWord);
	}

	int dictIdx = 0;
	CompactNode *endnode = retrieveEndNodeForString(deleteWord, &dictIdx, true);
	if(endnode)
	{
		return mngrDeleteWord(deleteWord, dictIdx, endnode) ;
	}

	return FALSE;
}
////////////////////////////////////////////////////////////////////////////////////
//BOOL CDictManager::mngrDeleteWordFromPersonal(MYWCHAR *word) 
//{
//	return mngrDeleteWord(word, m_personalDictIdx, NULL);
//}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CDictManager::mngrDeleteWord(MYWCHAR *word, int dictIdx, CompactNode *endnode)
{
	ShowInfo("mngrDeleteWord: called for word #%s# at dictIdx %d. \n", toA(word), dictIdx);

	if (m_orderedDictList[dictIdx].existingDictionary->deleteWord(word, endnode))
	{ 
		goToNextWord((char*)"mngrDeleteWord", false);
		return TRUE;
	}
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// looks into each dictionary to find out if a give wordpath is in fact an existing word's wordpath!
// returns the dictionary index of the containing dictionary, if it exists. Otherwise return INVALID_DICT_IDX
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CDictManager::mngrIsWordOrChunkInDictionariesIdx(int dictStart, int dictEnd, CompactNode **nodePath, BYTE *layer) 
{
	WLBreakIf(layer==NULL, "!!ERROR! mngrIsWordOrChunkInDictionariesIdx: layer is null!\n");
	for (int i = dictStart ; i < dictEnd; i++)
	{
		Dictionary *existDict = getDictionaryEnabled(i);
		if (existDict) 
		{
			if (existDict->isLayerEndpoint(nodePath, layer))
				return i;
		}
	}
	return INVALID_DICT_IDX;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
MYWCHAR *CDictManager::gatherRootEnding()
{
	bool backtoprevword = BackToPreviousWordIfAny(); 
	unsigned len = 0;
	MYWCHAR *currentWord = getCurrentWord(len);
	int nLen = gWordProp->nNullMoves + gWordProp->nPathNodes;
	if (gWordProp->nNullMoves > 0 && wordEndsOnNormalPunctuation(currentWord[nLen-1]))
		currentWord[nLen-1] = NUL;
	backtoprevword = backtoprevword && BackToWorkingWord();
	
	return currentWord;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This calculates the final preferences for selected words. It should be between MAX_PREFERENCE and MIN_PREFERENCE.
USHORT CDictManager::calcCascadingBasePreference(int dictIdx, WordValue wordValue, int layer) 
{
	USHORT idxWeight = getDictIdxWeight(dictIdx);

	// we will preserve the order of cascading dictionary, by artificial changing priority
	// according to their dictionary idx. In that way we make sure a highly preferred word in 
	// a lower dictionary never gets ahead of a low preferred word in a high dictionary
	// This is of course only valid for non-supplemental dictionaries
	int basePref = ((m_nOrderedDicts - idxWeight) * 0x100) + (wordValue == eHighValued)*0x270 + (wordValue==eLowValued)*0x220 + (wordValue == eOrdinary)*0x250;
	USHORT finalpref = (basePref + (MAX_LAYER_COUNT - layer)*0x150);
	return min(MAX_PREFERENCE, max(MIN_PREFERENCE, finalpref));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDictManager::printPreferences(MYWCHAR *afterTheseChars) 
{
#if 0
	if (m_nOrderedDicts > 1)
	return; // Not multi dictionary capable because we don't know

	for (int j = 0;j < m_nOrderedDicts ; j++) 
	{
		Dictionary *existDict = getDictionaryEnabled(j);
		if (existDict)
		{
			CompactNode *cNode = getCascadingNode(gWPath->pathway[gWPath->nPathNodes].cNode, j);
			if (cNode)
				existDict->printPreferences(afterTheseChars);
		}
	}
#endif
}

int  CDictManager::mngrGetWordsAddedSinceLastCallAndReset(BYTE *totalNumNewWords)
{ 
	*totalNumNewWords = mUserWordCache->flushLearnedWords();
	return mUserWordCache->GetWordsAddedAndReset(); 
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
NounAr* CDictManager::GetNounsFromPhrase(MYWCHAR* phrase)
{
	if(!phrase || !gPhraseEngine)
		return NULL;

	ShowInfo("MK Nouns phrase:(%s)\n", toA(phrase));

	trimEndingSpaces(phrase);
	static NounAr nounArr;

	int n = mywcslen(phrase);

	int pos =0;
	int index =0;
	MYWCHAR curWord[MAX_WORD_LEN];
	memset(curWord, 0, sizeof(curWord));
	int dictIdx = 0;
	for(int i=0; i < n+1; i++)
	{
		if(phrase[i] == SP || i == n)
		{
			//if(isUpperCase(curWord[0])) //if first letter is upper case, must be proper noun. 
			//{
			//	mywcscpy(nounArr.word[index], curWord);
			//	index++;
			//}
			//else
			{
	
				ReplaceiToI(curWord);
				MakeLowerCase(curWord);

				CompactNode* node = retrieveEndNodeForString(curWord, &dictIdx, true);
				if(!node) // Does not exist in PoS or dictionary..could be new word or learned word...so return as one of nouns.
				{
					//mywcscpy(nounArr.word[index], curWord);
					//index++;
					//WLBreakIf(index >= MAX_FOUND_PHRASES, "!!ERROR!! index for NounAr is full!!");
				}
				else
				{
					
					if(node->POStag > ePOS_NOTAG && node->POStag < ePOS_AB)
					{
						ShowInfo("MK Nouns curWord:(%s)\n", toA(curWord));
						mywcscpy(nounArr.word[index], curWord);
						index++;
						WLBreakIf(index >= MAX_FOUND_PHRASES, "!!ERROR!! index for NounAr is full!!");
					}
					
				}

			}

			pos = 0;
			memset(curWord, 0, sizeof(curWord));
			continue;
		}

		if(!isPunctuation(phrase[i]))
		{
			curWord[pos] = phrase[i];
			pos++;
		}
		
	}

	nounArr.count = index;
	return &nounArr;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
