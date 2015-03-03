// <copyright file="compatiblity.cpp" company="WordLogic">
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
// <summary>provides engine support for phrases. This inculde basic data structure setup and modifictiona and access.</summary>
#include "stdafx.h"
#include "phraseEngine.h"
#include "dictmanager.h"
#include "utility.h"
#include "compactstore.h"
#include <stddef.h>

PhraseNode PhraseEngineR::gPhrases[MAX_FOUND_PHRASES];

int PhraseEngineR::sGramArPos = 0; // keeps track of global count of static array s3Grams being filled so far.
bool PhraseEngineR::sInitialized = false;

extern CDictManager *wpDictManager;

#ifdef WL_SDK

WLArray PhraseEngine::sMemArray;
bool PhraseEngine::sInitialized = false;

USHORT PhraseEngine::sTotalPhraseCount = 0;
USHORT PhraseEngine::sTotalWordCount = 0;

// This is rather a hack for clamping non-normalized mish mash frequency values to a BYTE!!
// assumes value is not bigger than 2 times of USHRT_MAX ! If so, fix it somehow!
BYTE clampInt2Byte(UINT value)
{
	BYTE ret = (BYTE)value;
	if(value >= UCHAR_MAX)
	{
		float prop = float(value)/float(USHRT_MAX*2);		
		ret = (BYTE) (prop * float(UCHAR_MAX));
		ret = min(ret, UCHAR_MAX-1);
		ret = max(ret, 1);
	}
	return ret;
}
///////////////////////////////////////////////////////////////////////////////////////////
void WordNode::reset()
{
	endCNode = NULL;
	for(int i=0; i<MaxInNextWords; i++)
		mNextWords[i] = NULL;
	pref = BasePref;
}

//////////////////////////////////////////////////////////////////////////////////////////
void PhraseEngine::set(unsigned totalNumNodes, PhraseHeader &header, CCompactStore *cstore)
{
	memcpy(&mPhraseFileHeader, &header, sizeof(PhraseHeader));
	int testnum = MAX_PHRASE_ARRAY * 20 * 3;
	sMemArray.releaseAll();
	// we multiply number of phrase by 10 for stream processing! this is ugly but hey, it is offline, so as long as memory doesn't run out it is ok!!
	sMemArray.set(totalNumNodes * 2, sizeof(WordNode)); // 4 words per phrase is max we would have!
	mDictIdx = mPhraseFileHeader.eLang;
	mCStore = cstore;
	memset(mHeadWords, 0, MaxRootNextWords*sizeof(WordNode*));
	sInitialized = true;
}

///////////////////////////////////////////////////////////////////////////////////////////
//void PhraseEngine::setMemoryStore(unsigned pageIdx)
//{
//	sMemArray.setHeapPage(pageIdx);
//}

//////////////////////////////////////////////////////////////////////////////////////////
void PhraseEngine::init(FILE *fp)
{
	fread(&mPhraseFileHeader, sizeof(PhraseHeader), 1, fp);

	if(mDictIdx > eLang_NOTSET)
		sMemArray.releaseAll();
	else
		sMemArray.set(mPhraseFileHeader.phrasesCount*4, sizeof(WordNode)); // 4 words per phrase is max we would have!

	mDictIdx = mPhraseFileHeader.eLang;
	eLanguage dictId = wpDictManager->getTopDictionaryId();
	WLBreakIf(dictId != mDictIdx, "!!ERROR!! PhraseEngine::init: mismatch dictId PhraseFile?! (engine)%d!=(phraseFile)%d\n", dictId,mDictIdx); 
	mCStore = wpDictManager->getCompactStore(dictId);
	memset(mHeadWords, 0, MaxRootNextWords*sizeof(WordNode*));
	sInitialized = true;
}

/****************************************************************************
picks a slot from memory array and use it to create a new WordNode and return it.
******************************************************************************/
WordNode *PhraseEngine::createAWordNode(CompactNode *pCNode, USHORT pref)
{
	char *mem =sMemArray.next();
	WLBreakIf(mem==NULL, "!!ERROR! PhraseEngine:createAWordNode: failed to allocate memory! Increase heap size!\n");
	WordNode*node =  new(mem) WordNode(pCNode, pref);
	sTotalWordCount++;
	return node;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// adds a part-phrase in the subtree under this wordnode. It is assumed that either this node's word is
// null (so it is root node) or the phrase-part from the root to this node are already part of the total
// phrase, which ends with partphrase presented by endnodes.
WordNode * WordNode::addPhrase(CompactNode **endnodes, UINT preference)
{
	WLBreakIf(endnodes == NULL || endnodes[0]==NULL, "!!ERROR! WordNode::addPhrase: endnodes or endnodes[0] is NULL!\n");
	BYTE normalizedPref = clampInt2Byte(preference);
	bool added = false;
	int i = 0;
	for(; i<count; i++)
	{
		if(mNextWords[i]->endCNode == endnodes[0])
		{
			mNextWords[i]->pref = max(mNextWords[i]->pref, normalizedPref);
			added = true;
			break;
		}
	}
	
	WLBreakIf(i >= MaxInNextWords, "!!ERROR! WordNode::addPhrase: nextWords slot are full! Increase MAXPHRASEPERWORD and try adding the phrase again!\n");

	if(!added)
	{
		mNextWords[i] = PhraseEngine::createAWordNode(endnodes[0], normalizedPref);
		mNextWords[i]->parent = this;
		count++;
	}
	WordNode *ret = mNextWords[i];

	if(endnodes[1])
		ret = mNextWords[i]->addPhrase(&endnodes[1], normalizedPref);
	else
	{
		mNextWords[i]->endFlag = true;
		mNextWords[i]->setEndPreference(normalizedPref);
	}

	return ret;
}

//////////////////////////////////////////////////////////////////////////////////
NGramMultiNode * PhraseEngine::fixupEndNodes(NGramMultiNode *phrase)
{
	static NGramMultiNode thePhrase;
	thePhrase.set(phrase);
	int i = 0;
	while(thePhrase.endNodes[i])
	{
		CompactNode *curNode = mCStore->generateNodeFromOffset((char*)thePhrase.endNodes[i]);
		thePhrase.endNodes[i++] = curNode;
	}
	thePhrase.startPhrase = phrase->startPhrase;
	return &thePhrase;
}

//////////////////////////////////////////////////////////////////////////////
bool PhraseEngine::shouldBeIgnored(CompactNode *endnode)
{
	return false;

	static const int sIgnoreListCount = 4;
	static const wchar_t *sIgnoreStartWordList[sIgnoreListCount] = { L"a", L"an", L"and", L"or"};
	static CompactNode *sIgnoreEndNodes[sIgnoreListCount];
	static bool sIgnoreListSet = false;
	if(!sIgnoreListSet)
	{
		for(int i=0; i<sIgnoreListCount; i++)
			sIgnoreEndNodes[i] = mCStore->retrieveEndNodeForString((const MYWCHAR*)sIgnoreStartWordList[i], false);
		sIgnoreListSet = true;
	}

	// do the test
	for(int i=0; i<sIgnoreListCount; i++)
	{
		if(endnode == sIgnoreEndNodes[i])
			return true;
	}
	return false;
}
//////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
// Entry point for adding a phrase to phrase engine data structure. 
// It starts from root, and recursively goes down the word tree and adds proper nodes as it is needed.
// This function is only used for adding phrases at load time, so it is assumed that endnodes need fixing (offset adjustment)!
WordNode* PhraseEngine::addPhrase(NGramMultiNode *phrase, bool isP2P)
{
	if(phrase == NULL) 
		return NULL;
	int wordCount = phrase->wordCount();
//	if(wordCount > 3)
//		return NULL;
	if(wordCount< 2 && !isP2P)
		return NULL;

	NGramMultiNode *curPhrase =phrase; // isP2P ? fixupEndNodes(phrase) : phrase;

	WordNode *headnode = NULL; 

	// ignore  phrases such as "and a few", anything starting with a words in ignoreStartWordList
	CompactNode *curNode = curPhrase->endNodes[0];
	if(!isP2P && shouldBeIgnored(curNode))
		return NULL;

	int i=0;
	for(; i<MaxRootNextWords && mHeadWords[i]; i++)
	{
		if(mHeadWords[i]->endCNode == curNode)
		{	
			headnode = mHeadWords[i];
			break;
		}
	}
		
	if(!headnode)
	{
		WLBreakIf(i>=MaxRootNextWords || mHeadWords[i], "!!!ERROR!! PhraseEngine::addPhrase! MaxRootNextWords has been overtaken!!\n");
		headnode = mHeadWords[i] = createAWordNode(curNode, curPhrase->pref);
		mActualNextCount++;
	}

	curPhrase->wordNodeP = headnode;
	headnode->startFlag = curPhrase->startPhrase;
	if(curPhrase->endNodes[1])
		curPhrase->wordNodeP = headnode->addPhrase(&(curPhrase->endNodes[1]), curPhrase->pref);

	assert(curPhrase->wordNodeP != NULL);
	sTotalPhraseCount++;
	return headnode;
}

///////////////////////////////////////////////////////////////////////////////
void PhraseEngine::addStartPhrase(NGramMultiNode *phrase)
{
	WordNode *found = NULL;
	assert(phrase->startPhrase);
	for(int j=0; j<MaxRootNextWords && mHeadWords[j]; j++)
	{
		if(phrase->endNodes[0] == mHeadWords[j]->endCNode)
		{
			found = mHeadWords[j];
			break;
		}
	}
	if(!found)
		found = addPhrase(phrase, true);
	assert(found);
	mStartWords[mActualStartCount].word = found;
	mStartWords[mActualStartCount++].pref = phrase->pref;
}

////////////////////////////////////////////////////////////////////////////////////////////
// verifies if a phrase exist in subtree under this node. If it exist, then updates its preference.
bool WordNode::findPhrase(CompactNode **endNodes, UINT preference)
{
	if(endNodes == NULL || endNodes[0] != endCNode) // this should really be an assert and not 'if'!
		return false;
	if(endNodes[1] == NULL) 
	{	
		pref = max(pref, preference);
		return true;
	}

	for(int i=0; i<count; i++)
	{
		if(mNextWords[i]->endCNode == endNodes[1])
			return mNextWords[i]->findPhrase(&endNodes[1], preference);
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////
WordNode* PhraseEngine::findPhrase(CompactNode **endNodes)
{
	if(endNodes == NULL || endNodes[0] != NULL) // this should really be an assert and not 'if'!
		return NULL;
	for(int i=0; i<MaxRootNextWords && mHeadWords[i]; i++)
	{
		if(mHeadWords[i]->findPhrase(endNodes))
			return mHeadWords[i];
	}
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////
WordNode *WordNode::getNextWordNode(CompactNode *endNode)
{
	for(int i=0; i<count; i++)
	{
		if(mNextWords[i]->endCNode == endNode)
			return mNextWords[i];
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////
WordNode::~WordNode()
{
	for(int i=0; i<MaxInNextWords && mNextWords[i]; i++)
		delete mNextWords[i];
	PhraseEngine::sMemArray.release((char*)this);
}

#endif //WL_SDK

//////////////////////////////////////////////////////////////////////////////////////////
/////// runtime part of phrase engine ///////////////////////////////////
int PhraseNode::length()
{
	int len = 0;
	CompactWord *par = endCWord;
	while(par && par != gPhraseEngine->mRoot)
	{
		par = gPhraseEngine->getParent(par);
		len++;
	}
	return len;
}
//////////////////////////////////////////////////////////////////////////
MYWCHAR **PhraseNode::getStr(int &len)
{
	static MYWCHAR thePhrase[4][MAX_WORD_LEN];
	static MYWCHAR *sPhraseP[4];
	sPhraseP[0] = sPhraseP[1] = sPhraseP[2] = sPhraseP[3] = NULL;
	CompactNode *cNode;
	len = length();
	WLBreakIf(len > 4, "!!ERROR! PhraseNode::getStr(): we should not have 4 grams or more!\n");
	if(len==0)
		return sPhraseP;
	int i = len-1;
	sPhraseP[i] = NULL;
	cNode = gPhraseEngine->getWordCompactNode(endCWord);
	WLBreakIf( cNode == NULL, "!!ERROR! PhraseNode::getStr: cNode is NULL!!\n");

	wpDictManager->getWordForEndNode(cNode, thePhrase[i]);
	sPhraseP[i] = thePhrase[i];
	CompactWord *parent = endCWord;
	while(--i >= 0)
	{
		parent = gPhraseEngine->getParent(parent);
		cNode = gPhraseEngine->getWordCompactNode(parent);
		WLBreakIf( cNode == NULL, "!!ERROR! PhraseNode::getStr: cNode is NULL!!\n");

		wpDictManager->getWordForEndNode(cNode, thePhrase[i]);
		sPhraseP[i] = thePhrase[i];
	}
	return sPhraseP;
}

///////////////////////////////////////////////////////////////////////////
PhraseNode *PhraseNode::getNextPhrase(int i)
{
	static PhraseNode sPhrase;
	int pref = 0;
	CompactWord *cw = gPhraseEngine->getNextPhrase(endCWord, i, pref);
	if(cw==NULL)
		return NULL;
	
	sPhrase.endCWord = cw;
	sPhrase.pref = pref;
	return &sPhrase;
}
////////////////////////////////////////////////////////////////////////////
PhraseNode *PhraseNode::getPhraseFromCompactNodeList(CompactNode **nodelist)
{
	static PhraseNode sPhraseFromNode;
	if(nodelist==NULL || nodelist[0]==NULL)
		return NULL;
	int len = 0;
	
	while(len < 3 && nodelist[len]) // assuming we are dealing with at most 3 grams.
		len++;
	CompactNode *endnode = nodelist[len-1];
	WLBreakIf(endnode==NULL, "!!ERROR! PhraseNode::getPhraseFromCompactNodeList: null endnode!\n");
	CompactWord *rootWord = gPhraseEngine->getRootStartingWithNodes(&endnode, 1);
	WLBreakIf(rootWord==NULL, "!!ERROR! PhraseNode::getPhraseFromCompactNodeList: null rootWord!\n");
	sPhraseFromNode.set(rootWord);
	return &sPhraseFromNode;
}
///////////////////////////////////////////////////////////////////////////
CompactNode **PhraseNode::getCNodeList()
{
	static CompactNode *sPhraseEndNodes[5];
	int len = length();
	WLBreakIf(len <=0 || len>4,"!!ERROR! PhraseNode::getCNodeList: phrase is empty or max length > 4! %d > 4 !!\n", len);
	sPhraseEndNodes[len] = NULL;
	sPhraseEndNodes[--len] = gPhraseEngine->getWordCompactNode(endCWord);
	WLBreakIf( sPhraseEndNodes[len] == NULL, "!!ERROR! PhraseNode::getCNodeList: sPhraseEndNodes[len] is NULL!!\n");

	CompactWord *parent = endCWord;
	while(len!=0)
	{
		parent = gPhraseEngine->getParent(parent);
		sPhraseEndNodes[--len] = gPhraseEngine->getWordCompactNode(parent);
		WLBreakIf( sPhraseEndNodes[len] == NULL, "!!ERROR! PhraseNode::getCNodeList: sPhraseEndNodes[len] is NULL!!\n");
	}
	return sPhraseEndNodes;
}

///////////////////////////////////////////////////////////////////////////
void PhraseEngineR::init(FILE *fp)
{
	eLanguage dictId = wpDictManager->getTopDictionaryId();

	//mCStore = wpDictManager->getCompactStore(dictId);
	//Minkyu:2014.07.11
	//m_orderedDictList[index] has 4 for the max and index has to be top priority, which currently using it.
	//dictId from  wpDictManager->getTopDictionaryId(); can be from -1 to 15.
	mCStore = wpDictManager->getCompactStore(1);

	WLBreakIf(mCStore==NULL || mCStore->mTotalDictionarySize <=0, "!!ERROR!!PhraseEngine runtime file is not set properly!\n");

	fseek(fp, mCStore->mTotalDictionarySize, SEEK_SET);
	
	fread(&mHeader, sizeof(PhraseHeader), 1, fp);

	WLBreakIf(dictId !=  mHeader.eLang, "!!ERROR!! PhraseEngineR::init: mismatch dictId PhraseFile?! (engine)%d!=(phraseFile)%d\n", dictId, mHeader.eLang); 
	sInitialized = true;
}

/////////////////////////////////////////////////////////////////////////////////////////
CompactWord *PhraseEngineR::loadCompact(FILE *fp)
{
	int memSize = mHeader.totalDataSize;
	m_mem_size = (memSize + (SIZE64KB - 1)) & ~(SIZE64KB-1) + SIZE4KB;	

	m_block_compact = (char *) calloc(1, m_mem_size);
	if (m_block_compact == NULL) 
	{
		ShowInfo("!!!ERROR!!PhraseEngineR::loadCompact! No memory available for size 0x%x", m_mem_size); 
		return NULL;
	}
	
	m_start_compact = (char *)((uintptr_t)(m_block_compact + (SIZE4KB - 1)) & ~(SIZE4KB - 1));
	ShowInfo("PhraseEngineR:initialize: memsize=%x, m_mem_size=%x, m_block=%x, m_start=%x\n", memSize, m_mem_size, m_block_compact, m_start_compact);
	
	m_available = m_start_compact + memSize;

	if (! readFromFile(fp, (char *) m_start_compact, memSize))
	{
		ShowInfo("!!!ERROR!! PhraseEngineR::loadCompact: Failed to read %d bytes from ldat file !\n", memSize); 
		return NULL;
	}
	
	return (CompactWord*) (m_start_compact + sizeof(uintptr_t));
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int PhraseEngineR::PhraseNodeCompare(const void *a, const void *b)
{
	PhraseNode *nodeA = (PhraseNode *)a;
	PhraseNode *nodeB = (PhraseNode *)b;

	if(nodeA->pref > nodeB->pref) return -1;
	if(nodeA->pref == nodeB->pref) return 0;

	return 1;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// based on part of first word, all phrases starting with words containing with the part word are returned!
int PhraseEngineR::setNGramsBasedOnPartGram(MYWCHAR *partWord)
{
	int nRoots = 0;
	CompactNode **top10Words = mCStore->getTop10WordsStartingWith(partWord, &nRoots);
	memset(gPhrases, 0, sizeof(gPhrases));
	sGramArPos = 0;

	for(int i=0; i<nRoots; i++)
	{
		CompactWord *rootWord = getRootStartingWithNodes(&top10Words[i], 1);
		if(rootWord)
			setNGramArray(rootWord, i);
	}
		
	qsort(gPhrases, sGramArPos, sizeof(PhraseNode), PhraseNodeCompare);
	return sGramArPos;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// This function serializes the subtree under this node to the static array.
// arIdx keeps track of current slot index in global array s3Grams corresponding to this node.
bool PhraseEngineR::setNGramArray(CompactWord *cw, int arIdx, int minNGram)
{
	if(sGramArPos>= MAX_FOUND_PHRASES)
		return false;

	if(minNGram == 0)
	{
		if(cw->nextCount==0 || cw->getEndWordFlag())
			gPhrases[sGramArPos++].set(cw);
	}
	else
	{
		int curLen = 0;
		if(sGramArPos > 0)
			curLen = gPhrases[sGramArPos-1].length();

		if((cw->nextCount==0 || cw->getEndWordFlag()) && (curLen == 0 || curLen >= minNGram))
			gPhrases[sGramArPos++].set(cw);
	}

	bool ret = true;
	
	for(int i=0; i<cw->nextCount && ret ; i++)
	{
		CompactWord *next = (CompactWord *)getValueAtIndex(cw, PE_NEXTWORDS, PTR_SIZE, i);
		ret = setNGramArray(next, arIdx+i);	
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////
PhraseNode* PhraseEngineR::getStartWords(MYWCHAR *lets)
{
	static PhraseNode sStartWords[NMWORDPREDICTIONS+1];;
	int count = fillStartWords(sStartWords, lets);
	if(count==0)
		return NULL;
	return sStartWords;
}

////////////////////////////////////////////////////////////////////////////////////////////
// retrieves NMWORDPREDICTIONS number of start words. StartWords are implemented as start Phrases.
// It is assumed that start words are in descending order wrt their freq.
// If string "let" is not null, it is used to filter start words based on those which start with "let"
int PhraseEngineR::fillStartWords(PhraseNode *arr, MYWCHAR *let)
{
	int count = 0;
	MYWCHAR curWord[MAX_WORD_LEN];
	for(int i=0; i< mRoot->nextPhraseCount && count<NMWORDPREDICTIONS; i++)
	{
		CompactWord *cWord = (CompactWord *)getValueAtIndex(mRoot, PE_NEXTPHRASES, PTR_SIZE, i, PREF_SIZE);
		CompactNode *cNode = getWordCompactNode(cWord);
		WLBreakIf( cNode == NULL, "!!ERROR! PhraseEngineR::fillStartWords: cNode is NULL!!\n");

		mCStore->retrieveWordFromLastNode(cNode, curWord);
		if(!let || mywcsncmp(curWord, let, mywcslen(let)) == 0)
		{
			arr[count].endCWord = cWord;
			BYTE pref = (int)getValueAtIndex(mRoot, PE_NEXTPHRASES, PREF_SIZE, i, PTR_SIZE, PTR_SIZE);
			arr[count++].pref = pref;
		}
	}
	arr[count].unset();
	return count;
}

///////////////////////////////////////////////////////////////////////////////////////
BYTE PhraseEngineR::getStartPreference(CompactNode *cp)
{
	for(int i=0; i< mRoot->nextPhraseCount; i++)
	{
		CompactWord *cWord = (CompactWord *)getValueAtIndex(mRoot, PE_NEXTPHRASES, PTR_SIZE, i, PREF_SIZE);
		CompactNode *cNode = getWordCompactNode(cWord);
		WLBreakIf( cNode == NULL, "!!ERROR! PhraseNode::getStartPreference: cNode is NULL!!\n");

		if(cp == cNode)
		{
			BYTE pref = (int)getValueAtIndex(mRoot, PE_NEXTPHRASES, PREF_SIZE, i, PTR_SIZE, PTR_SIZE);
			return pref;
		}
	}
	 
	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////
void PhraseEngineR::setStartPreference(CompactNode *cp,  BYTE pref)
{
	for(int i=0; i< mRoot->nextPhraseCount; i++)
	{
		CompactWord *cWord = (CompactWord *)getValueAtIndex(mRoot, PE_NEXTPHRASES, PTR_SIZE, i, PREF_SIZE);
		CompactNode *cNode = getWordCompactNode(cWord);
		WLBreakIf( cNode == NULL, "!!ERROR! PhraseEngineR::setStartPreference: cNode is NULL!!\n");

		if(cp == cNode)
		{
			ShowInfo("PhraseEngineR::setStartPreference: updating startWord pref %d --> %d\n", cWord->pref, pref);
			setValueAtIndex(mRoot, PE_NEXTPHRASES, PREF_SIZE, i, PTR_SIZE, PTR_SIZE, pref);
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
void PhraseEngineR::addStartingWord(CompactNode *currentNode,  BYTE pref)
{
/*	ShowInfo("PhraseEngineR::addStartingWord: adding new StartingWord! for now just replace lowest one!\n");
	WLBreakIf( pref < (LEARNED_START_PREF + START_PREF_INCREMENT),"--!!ERROR pref < (LEARNED_START_PREF + START_PREF_INCREMENT)\n");
	for(int i=0; i< mRoot->nextPhraseCount; i++)
	{
		CompactWord *cWord = (CompactWord *)getValueAtIndex(mRoot, PE_NEXTPHRASES, PTR_SIZE, i, PREF_SIZE);
		BYTE savedpref = (int)getValueAtIndex(mRoot, PE_NEXTPHRASES, PREF_SIZE, i, PTR_SIZE, PTR_SIZE);
		if(savedpref <= LEARNED_START_PREF)
		{
			BYTE newPref =  min(pref, MAX_PREF);
			ShowInfo("PhraseEngineR::addStartingWord: adding startWord pref %d --> %d\n", LEARNED_START_PREF, newPref);
			cWord->
			setValueAtIndex(mRoot, PE_NEXTPHRASES, PREF_SIZE, i, PTR_SIZE, PREF_SIZE, newPref);
			break;
		}
	}
*/
}

////////////////////////////////////////////////////////////////////////////////////////////
// given first and possibly second word endnodes, this functions sets all possible 3 and 4 grams
// starting with these words in a static array which can then be accessed directly by user.
// It returns the actual number of phrases found. Static array can contain max MAX_SAVED_WORDS_COUNT endnodes.
int PhraseEngineR::setNGrams(CompactNode *end1, CompactNode *end2, MYWCHAR* cmpWord, int minNGram)
{
#ifdef TIMETEST
	clock_t start, end;
	start = clock();
#endif
	CompactNode *nodes[2]= {end1, end2};
	CompactWord *rootWord = getRootStartingWithNodes(nodes, (end2!=NULL)+1);
	if(!rootWord)
		return 0;

	memset(gPhrases, 0, sizeof(gPhrases));
	sGramArPos = 0;
	if(end2!=NULL && rootWord->getEndWordFlag())
		gPhrases[sGramArPos++].set(rootWord);

	MYWCHAR curWord[MAX_WORD_LEN];
	int wordLen =0;
	for(int i=0; i<rootWord->nextCount && i<MaxInNextWords; i++)
	{
		CompactWord *next = (CompactWord *)getValueAtIndex(rootWord, PE_NEXTWORDS, PTR_SIZE, i);
		if(cmpWord)
		{
			CompactNode *cNode = getWordCompactNode(next);
			WLBreakIf( cNode == NULL, "!!ERROR! PhraseEngineR::setNGrams: cNode is NULL!!\n");

			mCStore->retrieveWordFromLastNode(cNode, curWord);
			wordLen= mywcslen(cmpWord);
		}

		if(!cmpWord || (cmpWord && mywcsncmp(curWord, cmpWord, wordLen) == 0))
		{
			if(setNGramArray(next, sGramArPos, minNGram) == false)
				break;
		}

	}
	qsort(gPhrases, sGramArPos, sizeof(PhraseNode), PhraseNodeCompare);

#ifdef TIMETEST
	end = clock();
	double diff = Diffclock(end, start);
	ShowInfo("Time setNGrams:%f\n", diff);
#endif

	return sGramArPos;
}

///////////////////////////////////////////////////////////////////////////////////////////
CompactWord *PhraseEngineR::findNextWordChild(CompactWord *root, CompactNode *endNode)
{
	CompactWord *result;
	CompactNode *mid;
	//UINT target = (unsigned long)endNode;
	int numChildren = root->nextCount;
	int rangeStart = 0, rangeEnd = numChildren-1;
	while(rangeStart <= rangeEnd)
	{
		int midRange = (rangeStart+rangeEnd)/2;
		mid = getCWordCNode(root, midRange, &result);
		if(mid==endNode)
		{
			return result;
		}
		else if(mid < endNode)
			rangeStart = midRange + 1;
		else
			rangeEnd = midRange - 1;
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////
// given first and possibly second words (in startWords), this funciton returns the root of subtree
// which contains all phrases starting with these words.
CompactWord *PhraseEngineR::getRootStartingWithWords(MYWCHAR **startWords, int numStartWords)
{
	if(startWords == NULL || startWords[0]==NULL || numStartWords==0)
		return NULL;

	CompactNode *nodes[2] = {NULL, NULL};
	int dictIdx =0;
	nodes[0] = wpDictManager->retrieveEndNodeForString(startWords[0], &dictIdx, false);
	if(!nodes[0])
		return NULL;
	if(startWords[1] != NULL)
		nodes[1] = wpDictManager->retrieveEndNodeForString(startWords[1], &dictIdx, false);

	return getRootStartingWithNodes(nodes, numStartWords);
}

#define _BINARY_SEARCH
///////////////////////////////////////////////////////////////////////////////////////////////
// given first and possibly second words endNodes (in startNodes), funciton returns the root of subtree
// which contains all phrases starting with these words.
// !!!IMPORTANT!!! this function assumes nextwords are sorted by address, so it can use binary search!
CompactWord *PhraseEngineR::getRootStartingWithNodes(CompactNode **startNodes, int num)
{
	if(startNodes == NULL || startNodes[0]==NULL || num==0)
		return NULL;
#if TIMETEST
	clock_t start, end;
	start = clock();
#endif
	CompactWord *result = NULL;	
	CompactWord *next = NULL;

#ifdef _BINARY_SEARCH
	next = findNextWordChild(mRoot, startNodes[0]);
	result = next;
	if(startNodes[1] != NULL && next && num>1)
		result = findNextWordChild(next, startNodes[1]);

#else

	int numChildren = mRoot->nextCount;
	for(int i=0; i<numChildren; i++)
	{
		next = (CompactWord *)getValueAtIndex(mRoot, PE_NEXTWORDS, PTR_SIZE, i);
		CompactNode *cnode = getWordCompactNode(next);
		if(cnode && cnode == startNodes[0])
		{
			result = next;
			break;
		}
	}

	if(startNodes[1] != NULL && next && num>1)
	{
		int numChildren = next->nextCount;
		for(int i=0; i<numChildren; i++)
		{
			CompactWord *next1 = (CompactWord *)getValueAtIndex(next, PE_NEXTWORDS, PTR_SIZE, i);
			CompactNode *cnode = getWordCompactNode(next1);
			if(cnode == startNodes[1])
			{
				result = next1;
				break;
			}
		}
	}

#endif // _BINARY_SEARCH

#if TIMETEST
		end = clock();
		double diff = Diffclock(end, start);
		ShowInfo("Time getRootStartingWithNodes:%f\n", diff);
#endif

	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void PhraseEngineR::initialize(DWORD memSize)
{
	memSize = memSize / 10; // 10 is conservative guess for compactification ratio!! actual ratio is close to 20 times!

	m_mem_size = (memSize + (SIZE64KB - 1)) & ~(SIZE64KB - 1) + SIZE4KB;

	m_block_compact = (char *) calloc(1, m_mem_size);
	if (m_block_compact == NULL) 
	{
		ShowInfo("!!!No memory available for size 0x%x", m_mem_size); 
		return;
	}
	
	m_start_compact = (char *) ((GENPTR) (m_block_compact + (SIZE4KB - 1)) & ~(SIZE4KB - 1));
	ShowInfo("PhraseEngineR:initialize: memsize=%x, m_mem_size=%x, m_block=%x, m_start=%x\n", memSize,
														m_mem_size, m_block_compact, m_start_compact);
	m_available = 0;

	*(intptr_t *) m_start_compact = (intptr_t) m_start_compact; // record the address ths chunk was allocated on

	// valid (filled) memory in bytes
	m_valid_mem_size = memSize;
	m_available = m_start_compact + sizeof(intptr_t);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ePOSTAG* PhraseEngineR::getPOSTags(CompactWord *cw, int &count)
{
	static ePOSTAG tags[4] = {ePOS_NOTAG, ePOS_NOTAG, ePOS_NOTAG, ePOS_NOTAG};
	CompactNode *endNode = gPhraseEngine->getWordCompactNode(cw);
	WLBreakIf(endNode == NULL, "!!ERROR! getPOSTags: endNode is null!!\n");

	count = 0;
	if(endNode->POStag)
		tags[count++] = (ePOSTAG)endNode->POStag;
	if(endNode->Code & CODE_POSEXT)
		tags[count++] = mCStore->getSecondPOSTag(endNode);
	
	cw->getPOSTags(tags[count], tags[count+1]);
	count = count + (tags[count] != ePOS_NOTAG) + (tags[count+1] != ePOS_NOTAG);

	return tags;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PhraseEngineR::setExtraPOSTags(CompactWord *cp, BYTE tag3, BYTE tag4)
{
//	int offset = spaceCalc(cp, PE__POS_EXT
//	setValueAtOffset((char *)cp, PREF_SIZE, offset, tag3);
//	setValueAtOffset((char *)cp, PREF_SIZE, offset+1, tag4);

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// shift any compactNode pointer by memdiff bytes to the right, starting from address startAddress on.
// Currently there is only one CompactNode in CompactWord structure: its end node. 
void PhraseEngineR::adjustCompactNodePointers( INT memdiff, INT startAddress, CompactWord *root)
{
	if(root == NULL)
		root = mRoot;
	for(int i=0; i<root->nextCount; i++)
	{
		CompactWord *child = (CompactWord*)getValueAtIndex(root, PE_NEXTWORDS, PTR_SIZE, i);
		CompactNode *cnode = getWordCompactNode(child);
		WLBreakIf( cnode == NULL, "!!ERROR! PhraseEngineR::adjustCompactNodePointers: cNode is NULL!!\n");

		if((long)cnode >= startAddress)
			setEndCNode(child, (CompactNode*)((long)cnode+memdiff));
		adjustCompactNodePointers(memdiff, startAddress, child);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CompactWord *PhraseEngineR::allocCompactWord(USHORT POSTags, BYTE pref, int nNextWords, BYTE nextPhraseCount, BYTE endpref, bool ending) 
{
	int spaceNeeded = sizeof(CompactWord) + // base size
						+ PTR_SIZE // corresponding word's end compactNode.
						+ PTR_SIZE  // parent CompactWord
						+ PREF_SIZE * ending // end preference, if it is an end word for a phrase
						+ nNextWords * PTR_SIZE // next compact words in phrases
						+ nextPhraseCount * (PTR_SIZE + PREF_SIZE); // connecting phrases and their corresponding preferences
	
	assert(((unsigned int)(m_available-m_start_compact)) < m_mem_size);

	CompactWord *compactWord = (CompactWord *) m_available;
	m_available += spaceNeeded;
	fillCompactWord(compactWord, POSTags, pref, nNextWords, nextPhraseCount, endpref, ending);

	return compactWord;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PhraseEngineR::fillCompactWord(CompactWord *wordnode, USHORT POSTags, BYTE pref, int nNextWords, BYTE nextPhraseCount, BYTE endpref, bool ending) 
{
	wordnode->POSTags = POSTags;
	wordnode->pref = pref;
	wordnode->nextCount = nNextWords;
	wordnode->nextPhraseCount = nextPhraseCount;
	if(ending)
	{
		wordnode->setEndWordFlag();
		setEndPref(wordnode, endpref);
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PhraseEngineR::setEndCNode(CompactWord *cWord, CompactNode *endNode)
{
	mCStore->setPtrFieldAtOffset((char*)cWord, sizeof(CompactWord), (unsigned long) endNode);
}
////////////////////////////////////////////////////////////////////////////////////
void PhraseEngineR::setParent(CompactWord *cNode, CompactWord *parent)
{
	updateCompactWordPointer(cNode, (unsigned long)parent, PE_PARENT);
}
////////////////////////////////////////////////////////////////////////////////////
CompactWord *PhraseEngineR::getParent(CompactWord *cWord)
{
	CompactWord *cp = (CompactWord*)getValueAtIndex(cWord, PE_PARENT, PTR_SIZE, 0, 0);
	return cp;
}
////////////////////////////////////////////////////////////////////////////////////
void PhraseEngineR::setEndPref(CompactWord *cp, BYTE endpref)
{
	assert(cp->getEndWordFlag());
	int offset = spaceCalc(cp, PE_ENDPOINT);
	setValueAtOffset((char *)cp, PREF_SIZE, offset, endpref);
}
/////////////////////////////////////////////////////////////////////////////////////////
BYTE PhraseEngineR::getEndPref(CompactWord *cp)
{
	WLBreakIf(cp->getEndWordFlag() == 0, "!!!ERROR! PhraseEngineR::getEndPref: this is not an end cWord!!!\n");
	WLBreakIf(cp == NULL, "!!!ERROR! PhraseEngineR::getEndPref: cWord cp is NULL!!\n");
	int offset = spaceCalc(cp, PE_ENDPOINT);
	return (BYTE)queryValueAtOffset((char *) cp, PREF_SIZE, offset);
}

////////////////////////////////////////////////////////////////////////////////////
CompactWord *PhraseEngineR::getNextPhrase(CompactWord *cWord, int i, int &pref)
{
	if(i>= cWord->nextPhraseCount)
		return NULL;
	CompactWord *cp = (CompactWord *)getValueAtIndex(cWord, PE_NEXTPHRASES, PTR_SIZE, i, PREF_SIZE);
	pref = (int)getValueAtIndex(cWord, PE_NEXTPHRASES, PREF_SIZE, i, PTR_SIZE, PTR_SIZE);
	return cp;
}
////////////////////////////////////////////////////////////////////////////////////
void PhraseEngineR::setCompactNode(CompactWord *cNode, CompactNode *endnode)
{
	updateCompactWordPointer(cNode, (unsigned long)endnode, PE_COMPACTNODE);
}

/////////////////////////////////////////////////////////////////////////////////
CompactNode *PhraseEngineR::getCWordCNode(CompactWord *cword, int idx, CompactWord **target)
{
	*target = (CompactWord *)getValueAtIndex(cword, PE_NEXTWORDS, PTR_SIZE, idx);
	return getWordCompactNode(*target);
}
/////////////////////////////////////////////////////////////////////////////////
intptr_t PhraseEngineR::getValueAtIndex(CompactWord *cw, int type, int size, int index, int inc, int inOffset)
{
	WLBreakIf(cw == NULL, "!!!ERROR! PhraseEngineR::getValueAtIndex: Node cw is NULL!!\n");
	int offset = spaceCalc(cw, type);
	offset += (size+inc) * index;
	intptr_t ptrVal = (intptr_t) queryValueAtOffset((char*)cw, size, offset+ inOffset);
	if(size > 1)
		ptrVal += (long) m_start_compact;
	return ptrVal;
}

/////////////////////////////////////////////////////////////////////////////////
void PhraseEngineR::setValueAtIndex(CompactWord *cw, int type, int size, int index, int inc, int inOffset, uintptr_t value)
{
	WLBreakIf(cw == NULL, "!!!ERROR! PhraseEngineR::setValueAtIndex: Node cw is NULL!!\n");
	int offset = spaceCalc(cw, type);
	offset += (size+inc) * index;
	char *base = (char*)cw + offset;
	setValueAtOffset(base, PREF_SIZE, inOffset, value);
}

////////////////////////////////////////////////////////////////////////////////
CompactNode *PhraseEngineR::getWordCompactNode(CompactWord *cw)
{
	WLBreakIf(cw == NULL, "!!!ERROR! PhraseEngineR::getWordCompactNode: Node cw is NULL!!\n");
	int offset = spaceCalc(cw, PE_COMPACTNODE);
	UINT ptrVal = (UINT) queryValueAtOffset((char*)cw, PTR_SIZE, offset);
	return (CompactNode *)(ptrVal + (unsigned long)mCStore->m_start_compact);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
int PhraseEngineR::newSpaceToReserve(int toKnowUpto, int nextWordCount, int nextPhraseCount, bool isEndPoint)
{ 
	int spaceNeeded = sizeof(CompactWord);
	
	for (int i=0; i != toKnowUpto; i++)
	{
		switch (i) 
		{
			case PE_COMPACTNODE:
				spaceNeeded += PTR_SIZE;
				break;
			case PE_PARENT:
				spaceNeeded +=  PTR_SIZE;
				break;
			case PE_ENDPOINT:
				spaceNeeded += (PREF_SIZE * isEndPoint);
				break;
			case PE_NEXTWORDS:
				spaceNeeded += nextWordCount * PTR_SIZE;
				break;
			case PE_NEXTPHRASES:
				spaceNeeded += nextPhraseCount * PTR_SIZE;
				break;
			case PE_NEXTPHRASEPREFS:
				spaceNeeded += nextPhraseCount * PREF_SIZE;
				break;
			default:
				WLBreak("!!ERROR!PhraseEngineR::newSpaceToReserve: wrong index value %d!\n", i);
				break;
		}
	}

	return spaceNeeded;
}
/////////////////////////////////////////////////////////////////////////////////////
int PhraseEngineR::spaceCalc(CompactWord *cp, int toKnowAt)
{
	bool isEndPoint = cp->getEndWordFlag();
	WLBreakIf(cp == NULL, "!!!ERROR! PhraseEngineR::spaceCalc: Node cp is NULL!!\n");
	return newSpaceToReserve(toKnowAt, cp->nextCount, cp->nextPhraseCount, isEndPoint);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
void PhraseEngineR::updateCompactWordAtIndx(CompactWord *cp, int index, size_t value, int pinPointAt)
{
	int offset = spaceCalc(cp, pinPointAt);
	char *cNodePtrs = (char *)cp + offset;
	setPtrFieldAtOffset(cNodePtrs, PTR_SIZE * index, value);
}
/////////////////////////////////////////////////////////////////////////////////////////////////
void PhraseEngineR::updateCompactWordPointer(CompactWord *cp, size_t value, int pinPointAt)
{ 
	int offset = spaceCalc(cp, pinPointAt);
	char *cNodePtrs = (char *) cp + offset;
	setPtrFieldAtOffset(cNodePtrs, 0, value);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
// sets a consecutive pair of (Word,Value) at offset specified by index. The total sum of the pair is
// sizeWord + sizeValue, each corresponding to the element in the pair.
void PhraseEngineR::updateCompactWordValueAtIndex(CompactWord *cp, int index, size_t word, int value, int pinPointAt, int sizeValue)
{
	int offset = spaceCalc(cp, pinPointAt) + index*(PTR_SIZE+sizeValue);
	char *cNodePtrs = (char *)cp + offset;
	setPtrFieldAtOffset(cNodePtrs, 0, word);
	setValueAtOffset(cNodePtrs, sizeValue, PTR_SIZE, value);
}
//////////////////////////////////////////////////////////////////////////////////////
void PhraseEngineR::setPtrFieldAtOffset(char *base, int offset, size_t val)
{
	WLBreakIf(base == NULL, "!!!ERROR! setPtrFieldAtOffset: base pointer is NULL!!\n");
	size_t newval = val;
	if (val > (size_t)m_start_compact)
	{
		newval -= (size_t)m_start_compact; 
	}
	WLBreakIf((newval>>24) != 0, "!!!ERROR!PhraseEngineR::setPtrFieldAtOffset: ptr size should be only 3 bytes!\n");
	setValueAtOffset(base, PTR_SIZE, offset,  (UINT)newval);
}
/////////////////////////////////////////////////////////////////////////////////////////
