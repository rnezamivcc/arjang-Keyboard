// <copyright file="dictionary.cpp" company="WordLogic">
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
// <date>2012-06-10</date>
// <summary>This is topmost interface to a dictionary on disk. It is the main interface for a dictionary for main dictmanager class.</summary>

#include "stdafx.h"
#include "dictionary.h"
#include "utility.h"
#include "compactdict.h"
#include "searchResults.h"
#include "wordpath.h"
#include "wordpathext.h"
#include "dictmanager.h"


char Dictionary::sDictNames[eLang_COUNT][MAX_DICT_NAME_LENGTH] = {0};
void Dictionary::SetupDictionaryNames()
{
	static bool sDictionarynamesSet = false;
	if(!sDictionarynamesSet)
	{
		ShowInfo("SetupDictionaryNames: setting dictionarynames for first time\n");
		for(int i = 0; i< eLang_COUNT; i++)
		{
			strcpy(sDictNames[i], getDictionaryname((eLanguage)i));
		}
		sDictionarynamesSet = true;
	}
}

Dictionary::Dictionary(eLanguage elang, int dictIdx): m_eLang(elang), m_dictIdx(dictIdx), m_compactDict(NULL), m_compactStore(NULL), m_pathname(NULL)
{
}

Dictionary::~Dictionary()
{
	Destroy(FALSE);
}

/////////////////////////////////////////////////////////////////////////////////////////////
BOOL Dictionary::Load(char *pathname, BOOL forceFlag, BOOL savepath)
{
	ShowInfo("Dictionary::Create entered priority order idx=%d, eLang = %d, fullpath=%s \n", m_dictIdx, m_eLang, pathname);
	if(savepath)
	{
		m_pathname = (char *) malloc( (strlen(pathname) + 1));
		strcpy(m_pathname, pathname);
	}
	else
		m_pathname = NULL;

	m_compactStore = new CCompactStore(m_dictIdx, m_eLang);
	CompactNode * firstNode = m_compactStore->fileToCompact(pathname, forceFlag);
	if (firstNode)
	{
		m_compactDict = new CCompactDict(m_compactStore, m_dictIdx, m_eLang);
#if defined(DEBUG)
		m_compactStore->setCompactDict(m_compactDict);
#endif
		if (m_dictIdx >= 0)
		{
			gFirstNodeOfDicts[m_dictIdx] = firstNode; 
			gWPath->pathway[0].reset();
			convertCNodeToPathNode(&gWPath->pathway[0].dictNode[m_dictIdx], gFirstNodeOfDicts[m_dictIdx], gWorkingWordIdx, 0, 0);
		}
		return TRUE;
	}
	else
		m_compactStore->releaseCompactStore();

	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL Dictionary::CreateFromScratch()
{
	ShowInfo("Dictionary::creating dictionary from scratch idx=%d\n", m_dictIdx);
	m_compactStore = new CCompactStore(m_dictIdx, m_eLang);

	// minimum size of a dictionary is 64 k
	if (! m_compactStore->initialize(4, TRUE))
	{
		delete m_compactStore;
		m_compactStore = NULL;
		return FALSE;
	}

	m_compactDict = new CCompactDict(m_compactStore, m_dictIdx, m_eLang);
	//eLanguage elang = findLanguage(m_pathname);
	m_compactStore->fillBuildHeader(m_eLang);
	m_compactStore->allocateFirstEmptyNode();

	if (m_dictIdx >= 0)
	{
		gFirstNodeOfDicts[m_dictIdx] = m_compactStore->getAllocatedFirstNode();
		gWPath->pathway[0].reset();
		convertCNodeToPathNode(&gWPath->pathway[0].dictNode[m_dictIdx], gFirstNodeOfDicts[m_dictIdx], gWorkingWordIdx, 0, 0);
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
void Dictionary::resetConfiguration(eLanguage langIdx)
{
	m_eLang = langIdx;
	m_compactStore->resetConfiguration(langIdx);
	m_compactDict->resetConfiguration(langIdx);
	gFirstNodeOfDicts[langIdx] = m_compactStore->getAllocatedFirstNode();
	//gWPath->pathway[0].reset();
	convertCNodeToPathNode(&gWPath->pathway[0].dictNode[langIdx], gFirstNodeOfDicts[langIdx], gWorkingWordIdx, 0, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
void Dictionary::Destroy(BOOL justReleaseFlag)
{
	if (m_compactStore)
	{
		if (justReleaseFlag == FALSE)
		{
			if (m_compactStore->GetDynamicChangeCount())
				m_compactStore->compactToFile(createFullPathFileName(Dictionary::GetDictName(m_eLang), LDAT));
		}
		m_compactStore->releaseCompactStore();

		delete m_compactStore;
		m_compactStore = NULL;
	}
	m_dictIdx = -1;
	if (m_compactDict)
	{
		delete m_compactDict;
		m_compactDict = NULL;
	}
	if (m_pathname) 
	{
		free( m_pathname);
		m_pathname = NULL;
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Dictionary::flushOutToDisk()
{
	ShowInfo(" Dictionary::flushOutToDisk %s\n", createFullPathFileName(Dictionary::GetDictName(m_eLang), LDAT));
	m_compactStore->serializeDictHeader();
	m_compactStore->compactToFile(createFullPathFileName(Dictionary::GetDictName(m_eLang), LDAT));
}
////////////////////////////////////////////////////////////////////////////////////////////////
BOOL Dictionary::usedUpEditableSpace()
{
	return m_compactStore->usedUpEditableSpace();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
CompactNode * Dictionary::retrieveEndNodeForString(MYWCHAR *word, bool isEndpoint)
{
	return m_compactDict->retrieveEndNodeForString(word, isEndpoint);
}
//////////////////////////////////////////////////////////////////////////////////////////
int Dictionary::getPossibleNumWords(CompactNode *endNode)
{
	return m_compactDict->getNumPossibleWords(endNode);
}
//////////////////////////////////////////////////////////////////////////////////////////////
BOOL Dictionary::fillNextPathNode(MYWCHAR letter, USHORT *layers, USHORT *layerEnds)
{
	return m_compactDict->fillNextPathNode(letter, gWordProp->nPathNodes, layers, layerEnds);
}
////////////////////////////////////////////////////////////////////////////////////////////////
BYTE Dictionary::getLetterPref(MYWCHAR letter)
{
	return m_compactDict->getLetterPref(letter);
}
///////////////////////////////////////////////////////////////////////////
LangTreatmentRule *Dictionary::getTreatmentRules()
{
	return m_compactStore->getTreatmentRules();
}
////////////////////////////////////////////////////////////////////////////////////////////////
void Dictionary::putFirstNodeInOnPathwayNode(int pathlen)
{
	 m_compactDict->putFirstNodeInOnPathwayNode(pathlen);
}
//////////////////////////////////////////////////////////////////////////////////////////////
void Dictionary::nextWords(eSearchMode searchMode, WordValue wordValue, int layer)
{
//	ShowInfo("Dictionary::nextWords: dictIdx=%d, m_eLang=%d, name=%s\n", m_dictIdx, m_eLang, GetDictName(m_eLang));
	clearBreadCrumbCache();  // save the dynamically changed preferences in the bread crumbs and restore the originals
	m_compactDict->nextWords(searchMode, layer);
	restoreBreadCrumbs();
}
////////////////////////////////////////////////////////////////////////////////////////////////
void Dictionary::collectEVerbs(WordValue wordValue, int layer, BYTE eVerbVarIdx)
{
	gWordProp->wordValue = wordValue;	// set it in the gWordProp struct, prevent stack grow on recursion
	m_compactDict->collectEVerbs(layer, eVerbVarIdx);
}
/////////////////////////////////////////////////////////////////////////////////////////////////
void Dictionary::backspaceLetter()
{
	m_compactDict->backspaceNode();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int Dictionary::nextLetters(PrefLetterNode *allPrefLetters, int nMaxLettersToRetrieve, int *nFilledLettersP, BOOL auCharsOnly)
{
	return m_compactDict->getPreferredLetters(allPrefLetters, nMaxLettersToRetrieve, nFilledLettersP, auCharsOnly);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL Dictionary::isLayerEndpoint(CompactNode **nodePath, BYTE *layer)
{
	return m_compactDict->isLayerEndpoint(nodePath, layer);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
//BOOL Dictionary::isFakeChunk()
//{
//	return m_compactDict->isFakeChunk();
//}
///////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL Dictionary::isChunk(int node)
{
	return m_compactDict->isChunk(node);
}
//////////////////////////////////////////////////////////////////////////////
BOOL Dictionary::isEndpoint(int node)
{
	return m_compactDict->isEndpoint(node);
}
//////////////////////////////////////////////////////////////////////////////
BOOL Dictionary::partOfOtherWords(BOOL wordByItself)
{
	return m_compactDict->partOfOtherWords(wordByItself);
}
/////////////////////////////////////////////////////////////////////////////////
BOOL Dictionary::isNodeAnEVerbEnding(int *nTrailingChars, BYTE *eVerbVarIdx)
{
	return m_compactDict->isNodeAnEVerbEnding(nTrailingChars, eVerbVarIdx);
}
////////////////////////////////////////////////////////////////////////////////////
void Dictionary::takeOutTheEVerb(int nPath, BYTE *pref)
{
	m_compactDict->takeOutTheEVerb(nPath, pref);
}
///////////////////////////////////////////////////////////////////////////////////
void Dictionary::putTheEVerbBackIn(int nPathNodes, BYTE *originalPref)
{
	m_compactDict->putTheEVerbBackIn(nPathNodes, originalPref);
}
//////////////////////////////////////////////////////////////////////////////////
CompactNode* Dictionary::addWord(MYWCHAR *word, int preference)
{
	return m_compactStore->addWord(word, preference);
}
/////////////////////////////////////////////////////////////////////////////////////
BOOL Dictionary::deleteWord(MYWCHAR *word, CompactNode *endnode)
{
#if defined(DEBUG)
	ShowInfo("deleteWord:Number of words before deletion of #%s# = %d\n", toA(word), m_compactDict->getNumWords(m_compactStore->getAllocatedFirstNode()));
#endif
	//int len = mywcslen(word);
	if(m_compactStore->deleteWord(endnode))
	{		
		m_compactStore->GetDictHeader()->totalNumWords--;
		return TRUE;
	}
	return FALSE;
}
//////////////////////////////////////////////////////////////////////////////////////////
#ifdef WL_SDK
void Dictionary::CreatePreferencesMap(int bytesPerPreference)
{
#if 0
	m_compactStore->doDumpPreferences(TRUE, NULL, bytesPerPreference);
#endif
}
//////////////////////////////////////////////////////////////////////////////////////////
void Dictionary::printPreferences(MYWCHAR *afterTheseChars)
{
//W	m_compactStore->printPreferences(afterTheseChars);
}

/*void Dictionary::addWordsToWordList(CWordList *wordListObject)
{
	m_compactStore->addWordsToWordList(wordListObject, NULL, -1);
}

void Dictionary::addWordsToWordList(CWordList *wordListObject, int fileRef)
{
	m_compactStore->addWordsToWordList(wordListObject, NULL, fileRef);
}

void Dictionary::removeWordsFromWordList(CWordList *wordListObject)
{
	m_compactStore->removeWordsFromWordList(wordListObject, NULL);
}
*/
BOOL Dictionary::countWord(MYWCHAR *wordPart)
{
	return m_compactDict->doCountWord(wordPart);
}
#endif // WK_SDK
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
// given a word's end node and an array of empy strings, it fills the array with possible
// next words following this word, in descending order.
// It take the node, wordsAr: the array for filling the next words, num: the number of next words required.
// It returns the actual number of next words deposited in wordsAr.
int Dictionary::findNextWords(CompactNode *node, WordsArray *wordsAr, int num)
{
/*	int count = node->NextCount;
	if(count==0 || num >=MAX_NUM_PREDICTIONS)
		return 0;
	WLBreakIf(count >= 0xff, "!!ERROR! Dictionary::findNextWords! count > 0xff!!\n");
	CompactNode **nexts = m_compactStore->getNextWordsPtrs(node);
	BYTE *prefs = m_compactStore->getNextWordPrefsPtrs(node);
	
	count = min(count, MAX_NUM_PREDICTIONS);
	*/	
	// the following algorithm searches the nextwords and finds num most probable next words.
	// TODO: we should have the nextwords already sorted when buliding dictionary and also when
	// modifying it, to avoid having to do the seach bellow everytime!
/*	int sorted[MAX_NUM_PREDICTIONS];
	int lastIdx = -1, curIdx=0;
	BYTE lastPref = MAXIMUM_PREFERENCE;
	BYTE slots[0xff];
	memset(slots, 0, 0xff);
	for(int i = 0; i<num; i++)
	{
		curIdx = -1;
		for(int j = 0; j<count; j++)
		{
			if( !slots[j] && ( curIdx <0 || (prefs[j] <= lastPref && prefs[j] > prefs[curIdx] && j!= lastIdx) ) )
				curIdx = j;
		}
		if(curIdx < 0)
			break;
		sorted[i] = curIdx;
		lastIdx = curIdx;
		lastPref = prefs[curIdx];
		slots[curIdx] = 1; // flag this slot as taken!
	}
*/
/*	int ret = num;
	for(int i=0; ret<MAX_NUM_PREDICTIONS && i<count; i++)
	{
		CompactNode *cp = (CompactNode *) m_compactStore->queryPtrFieldAtOffset((char*)nexts, PTR_SIZE * i); //sorted[i]);
		WLBreakIf(!cp, "!!ERROR! Dictionary::findNextWords! cp is null!\n");
		if(m_compactStore->isEndpoint(cp))
		{
			m_compactStore->retrieveWordFromLastNode(cp, wordsAr->words[ret++]);
			// remove duplication! TODO: find out why sometimes we have duplication: for instance after "I've" we have 2 "been" !!
			for(int j = 0; j< (ret-1); j++)
			{
				if(mywcscmp(wordsAr->nextWords[j], wordsAr->nextWords[ret-1])==0)
				{
					wordsAr->nextWords[--ret][0] = NUL;
					break;
				}
			}
		}
	}
	return ret;
	*/
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////
// we assume dictionary name ends with extention .ldat. 
eLanguage Dictionary::SetDictName(char *name)
{
	char *lowername = mytolower(name);
	int totallen = (int)strlen(lowername);
	int i = totallen -1;
	while(lowername[i] != '\\' && lowername[i] !='/' && i>=0)
		i--;
	lowername = i==0 ? &lowername[0] : &lowername[i+1];
	int extlen = strlen(LDAT);
	for(int i=0; i<eLang_COUNT; i++)
	{
		if(sDictNames[i] && lowername)
		{
			if(strncmp(lowername, sDictNames[i], strlen(lowername)-extlen) ==0)
				return (eLanguage)i;
		}
	}
	ShowInfo("ERROR!! missing dictionary %s in our database. Add it. We ignore it for now!!\n", lowername);
	printf("ERROR!! missing dictionary %s in our database. Add it. We ignore it for now!!\n", lowername);
	return eLang_DEFAULT;   // !!this means we don't have this dictionary in our database. Adde it!
}

int Dictionary::GetDictIdx(char *name)
{
	for(int i=0; i<eLang_COUNT; i++)
	{
		if (strcmp(sDictNames[i], name) == 0)
		{
			return i;
		}
	}
	return -1;

}
char* Dictionary::GetDictName(char *name)
{
	for(int i=0; i<eLang_COUNT; i++)
	{
		if (!strcmp(sDictNames[i], name))
		{
			return sDictNames[i];
		}
	}
	return NULL;
}

char* Dictionary::GetDictName(int langId)
{
	if(langId < 0 || langId > eLang_COUNT)
		return NULL;
	return sDictNames[langId];
}
