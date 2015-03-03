// <copyright file="dictconfig.cpp" company="WordLogic Corporation">
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
// <summary>Provides support for processing dictionary file offline and preparing the runtime final .dict file.</summary>
//
#include "StdAfx.h"
#include "dicttree.h"
#include "compactstore.h"
#include "wordanalyzer.h"
#include "parsedline.h"
#include "wordpath.h"
#include "dictionary.h"
#include "wordpunct.h"
#include "dictmanager.h"
#include "phraseEngine.h"

#include <iostream>
#include <sstream>
#include <string>
#include <iterator>
#include <fstream>>
#include <iomanip>
#include <algorithm>

//#define OBJECTLISTNAME  L"LIST"
//#define OBJECTNAME      L"OBJECT"

static DWORD sMinPref = 0x7fffffff;
static DWORD sMaxPref = 0;

static FILE* sOutputFile = NULL;

static void printRestOfWord(DictNode *dNode);
static void printRestOfWordOnCondition(DictNode *dNode, NodeSelection nodeSelection);
static void printWordIfEVerb(DictNode *dNode);

static void printallSuffixes();

BOOL normalizeWithBuckets(DictNode *startingNode,DWORD lowest,DWORD highest);

void findHighestLowestPrefs(DictNode *startingNode);
void sprintfile(TCHAR *format, ...);

static DictNode *dictNodePath[MAX_WORD_LEN];
static int dictNodeCnt = 0;

static WLHeap sHeap;
///////////////////////////////////////////////////////////////////////////
void DictNode::reset()
{
	memset(this, 0, sizeof(DictNode));
}
///////////////////////////////////////////////////////////////////
void putDictnodeIntoNodepath(DictNode *dNode)
{
	if (dictNodeCnt < (MAX_WORD_LEN - 1))
	{
		dictNodeCnt++;
		dictNodePath[dictNodeCnt] = dNode;
	}
}
///////////////////////////////////////////////////////////////////////
void takeDictnodeOutOfNodepath()
{
	if (dictNodeCnt > 0)
	{
		dictNodePath[dictNodeCnt] = NULL;
		dictNodeCnt--;
	}
}
/////////////////////////////////////////////////////////////////////////
DictNode *allocDictNode(MYWCHAR newChar, DWORD preference)
{
	assert(sHeap.available() > MB);
	DictNode *newDictNode = (DictNode *)sHeap.allocate(sizeof(DictNode));
	newDictNode->reset();
	if(newChar == 0xcccc)
		printf("Here's the problem!\n");
	newDictNode->Character = newChar;
	newDictNode->Preference = preference;
	newDictNode->state = WL_NORMAL;
	return newDictNode;
}
////////////////////////////////////////////////////////////////////////////////////
NGramNode *allocNGramNode(DictNode *thisNode, DWORD pref, NGramNode *next)
{
	assert(sHeap.available() > MB);
	NGramNode *newNGramNode = (NGramNode *)sHeap.allocate(sizeof(NGramNode));
	newNGramNode->endNode = thisNode;
	newNGramNode->pref = pref;
	newNGramNode->Next = next;
	return newNGramNode;
}
////////////////////////////////////////////////////////////////////////////////////
int NGramMultiNodeCompare (const void * a, const void * b)
{

  NGramMultiNode *nodeA = (NGramMultiNode *)a;
  NGramMultiNode *nodeB = (NGramMultiNode *)b;

  if(nodeA->pref > nodeB->pref) return -1;
  if(nodeA->pref == nodeB->pref) return 0;

  return 1;

}
////////////////////////////////////////////////////////////////////////////////////
static int mNumPhraseProcessed = 0;
static NGramMultiNode gPhraseAr[MAX_PHRASE_ARRAY];
//static unsigned long gPhraseHash[MAX_PHRASE_ARRAY*80];
static int gTotalPhraseProcessed = 0;
static PartOfSpeechTag*	gPoSTag[MAX_ALPHABET_SIZE][MAX_POSTAG_SIZE];

////////////////////////////////////////////////////////////////////////////////
CDictionaryTree::CDictionaryTree(eLanguage lang) 
{
	sHeap.set(60*MB);
	memset(this, 0, sizeof(CDictionaryTree));
	m_DictTreeStartNode = allocDictNode(NUL, 0);
	m_language = lang;
	mPhraseHeader = new PhraseHeader;

	mSepChar = SP;
	mLineParser = NULL;
	mEndianflag = testEndianNess();
	memset(gPhraseAr, 0, sizeof(gPhraseAr));
//	memset(gPhraseHash, 0, sizeof(gPhraseHash));
	memset(m_PhraseWords, 0, sizeof(m_PhraseWords));
	memset(m_P2PWords, 0, sizeof(m_P2PWords));
	memset(gPoSTag, 0, sizeof(gPoSTag));
	m_nPref =0;
}
/////////////////////////////////////////////////////////////////////////////
CDictionaryTree::~CDictionaryTree() 
{
	// you better clear out the tree first before this is done.
	m_DictTreeStartNode = NULL;
	delete m_compactStore;
	m_compactStore = NULL;
	delete mPhraseEngine;

	if(mFP != NULL)
		fclose(mFP);
	mFP = NULL;
	if(mLineParser)
		delete mLineParser;
	mLineParser = NULL;

}
//////////////////////////////////////////////////////////////////////////////////////
DictNode *CDictionaryTree::FindCharInTree(DictNode *prevDictCharNode, MYWCHAR newChar) 
{
	if (prevDictCharNode == NULL)
	{
		prevDictCharNode = m_DictTreeStartNode;
	}

	for (DictNode *listp = prevDictCharNode->CharacterList; listp != NULL; listp = listp->Next)							
	{
		if (listp->Character == newChar)
			return listp;
	}
	return NULL;
}
//////////////////////////////////////////////////////////////////////////////////////
DictNode *CDictionaryTree::AddCharInTree(DictNode *prevDictCharNode,MYWCHAR newChar,  DWORD dwPreference) 
{
	DictNode *newDictNode = allocDictNode(newChar, dwPreference);

	if (prevDictCharNode == NULL)
	{
		prevDictCharNode = m_DictTreeStartNode;
	}

	newDictNode->Next = prevDictCharNode->CharacterList;
	prevDictCharNode->CharacterList = newDictNode;
	m_nNodes++;
	return newDictNode;
}

//////////////////////////////////////////////////////////////////////////////////////
void CDictionaryTree::addFollowConnection(DictNode *prevNode, DictNode *followNode)
{
	if (prevNode == NULL)
		return;
//		prevNode = m_DictTreeStartNode;

	followNode->Next = prevNode->CharacterList;
	prevNode->CharacterList = followNode;
}
//////////////////////////////////////////////////////////////////////////////////////
/*
BOOL CDictionaryTree::FindObjectInTree(DictNode *fromObjectNode, DictNode *toObjectNode)
{
	for (ObjectListNode *listp = fromObjectNode->ObjectList; listp != NULL; listp = listp->Next)							
	{
		if (listp->ObjectNode == toObjectNode)
			return TRUE;
	}
	return FALSE;
}

void CDictionaryTree::AddObjectInTree(DictNode *fromObjectNode, DictNode *toObjectNode)
{
	ObjectListNode *listNode = (ObjectListNode *) malloc(sizeof(ObjectListNode));
	if (listNode == NULL)
	{
		printf("Trouble allocating memory in allocObjectListNode \n");
		return;
	}

	listNode->ObjectNode = toObjectNode;
	listNode->Next = fromObjectNode->ObjectList;
	fromObjectNode->ObjectList = listNode;
}
*/
//////////////////////////////////////////////////////////////////////////////////////
void CDictionaryTree::printTree(char *fileName)
{
	sOutputFile = fopen( fileName , "w");	
	if (sOutputFile == NULL)
		return;

	for (DictNode *dNode = m_DictTreeStartNode->CharacterList; dNode != NULL; dNode = dNode->Next)
		printRestOfWord(dNode);

	printallSuffixes();

	fwprintf(sOutputFile, L"Verbs on E");
	fwprintf(sOutputFile, L"\x0d\x0a");

	for (DictNode *dNode = m_DictTreeStartNode->CharacterList; dNode != NULL; dNode = dNode->Next)
		printWordIfEVerb(dNode);

	printStatistics();
	fclose(sOutputFile);
	sOutputFile = NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////
void CDictionaryTree::printTreeSort(char *fileName)
{
	sOutputFile = fopen( fileName , "w");	
	if (sOutputFile == NULL)
		return;

	for (MYWCHAR c = MYWCHAR('A') ; c <=MYWCHAR('Z'); c++)
	{
		MYWCHAR myc = c;
		for (int k = 0; k < 2; k++)
		{
			if (k == 1)
				myc = lwrCharacter(c);

			for (DictNode *dNode = m_DictTreeStartNode->CharacterList; dNode != NULL; dNode = dNode->Next)
			{
				if (dNode->Character == myc)
					printRestOfWord(dNode);
			}
		}
	}

	for (DictNode *dNode = m_DictTreeStartNode->CharacterList; dNode != NULL; dNode = dNode->Next)
	{
		MYWCHAR myc = dNode->Character;
		if ((myc >= 'a' && myc <= (MYWCHAR)'z') || (myc >= (MYWCHAR)'A' && myc <= (MYWCHAR)'Z') )
			continue;

		printRestOfWord(dNode);
	}

	printStatistics();
	fclose(sOutputFile);
	sOutputFile = NULL;
}

void CDictionaryTree::printTreeNodes(char *fileName, NodeSelection nodeSelection)
{
	sOutputFile = fopen( fileName , "w");	
	if (sOutputFile == NULL)
		return;

	for (DictNode *dNode = m_DictTreeStartNode->CharacterList; dNode != NULL; dNode = dNode->Next)
	{
		printRestOfWordOnCondition(dNode, nodeSelection);
	}

	printStatistics();
	fclose(sOutputFile);
	sOutputFile = NULL;
}

void CDictionaryTree::printTree()
{
	printTree(TREE_TXT);
}

void CDictionaryTree::collectSuffixes()
{
	for (DictNode *dNode = m_DictTreeStartNode->CharacterList; dNode != NULL; dNode = dNode->Next)
	{
		static int herecount = 0;
		herecount++;
	//	printf("herecount=%d\n", herecount);
	//	if(herecount==54) // for portuguese dict at this point dNode->Next contains garbage!! why? 
	//		printf("here at 54");
		findSuffixes(dNode, collect);
	}
	for (DictNode *dNode = m_DictTreeStartNode->CharacterList; dNode != NULL; dNode = dNode->Next)
	{
		findSuffixes(dNode, assign);
	}
}

void CDictionaryTree::printDictWord(char *word)
{
	char *chp = word;
	DictNode *dictNode;
	DictNode *prevDictNode = NULL;;

	while (*chp)
	{
		dictNode = FindCharInTree(prevDictNode, *chp);
		if(dictNode)
			printf("dictNode %c pref %d (%d) occ %d \n", dictNode->Character,dictNode->Preference, 
						dictNode->EndPreference, dictNode->WordOccurence);

		chp++;
		prevDictNode = dictNode;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CDictionaryTree::normalizePreferences()
{
	DictNode *startingNode = m_DictTreeStartNode; 

	countWords(NULL);
	if (m_newWords == 0)
		return;

	for (DictNode *dNode = startingNode->CharacterList; dNode != NULL; dNode = dNode->Next)
		printf("node %c has preference %d\n", dNode->Character, dNode->Preference);

	findHighestLowestPrefs(startingNode);

	normalizeWithBuckets(startingNode, sMinPref, sMaxPref);

	for (DictNode *dNode = startingNode->CharacterList; dNode != NULL; dNode = dNode->Next)
	{
		printf("node %c has revised preference %d\n", dNode->Character, dNode->Preference);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
/*DictNode *CDictionaryTree::storeListObjectInTree(MYWCHAR *lpWord, DWORD preference, MYWCHAR *leadChars)
{
	DictNode *leadNode = NULL;
	// make sure the lead characters path exist in the main dictionary tree

	if (wcslen(leadChars) > 0)
		leadNode = storeWord(leadChars, preference, NULL, NULL , SingleChain);
	
	if (wcslen(lpWord) <= 0)
		return NULL; // if we don't care about the listname then don't even store it

// point straight to the T of ....LIST
//  Store all the list under the name OBJECTLISTNAME with the rest of the objects
	DictNode *listNode = storeWord(lpWord, preference, OBJECTLISTNAME, NULL, DoubleChain);
	listNode->listSpecNode = listNode; // points to itself indicating it is a list node

//  instead of that connect the end of the lead (leadnode) to the end of the listname
	linkObject(leadNode, listNode); 
//	addFollowConnection(leadNode,listNode);

	return listNode;
}
*/

static MYWCHAR sCompactChar[MAX_WORD_LEN];
int sCompactIdx = 0;
/////////////////////////////////////////////////////////////////////////////////////////////////////////
CompactNode *CDictionaryTree::compactTree(DictNode *startingNode, CompactNode *parentCNode)
{
	if (startingNode == NULL)
	{
		m_numStartingWords = 0;
		startingNode = m_DictTreeStartNode;
		if (m_compactStore)
			delete m_compactStore;

		m_compactStore = new CCompactStore( m_language, m_language);	// no dictionary manager index
		if (!m_compactStore->initialize(m_newCharacters*20 + 20 , TRUE)) 
		{
			delete m_compactStore;
			m_compactStore = NULL;
			return NULL;
		}
		m_compactStore->setAllocatedFirstNode();
		m_compactStore->fillBuildHeader(m_language, m_bondingChars, m_newWords, m_newCharacters);
		sCompactIdx = 0;
	}

	if (startingNode->compactNode)
		return((CompactNode*) startingNode->compactNode);

	sCompactChar[sCompactIdx++] = startingNode->Character == NUL ? SP : startingNode->Character;


	ePOSTAG eTag1 = ePOS_NOTAG; 
	ePOSTAG eTag2 = ePOS_NOTAG; 
	if(startingNode->EndPointFlag)
	{
		MYWCHAR cmpWord[MAX_WORD_LEN];
		memset(cmpWord, 0, sizeof(cmpWord));// Need memset here, otherwise it won't get pos tag from GetPosTag. 
		int index =0;
		for(int i=0; i < MAX_WORD_LEN && sCompactChar[i]; i++)
		{
			if(sCompactChar[i] != SP)
				cmpWord[index++] = sCompactChar[i];
		}
		if(!isNumber(cmpWord))
		{
			if(cmpWord[0] == HASHTAG)
			{
				eTag1 = ePOS_HASHTAG; 
				eTag2 = ePOS_HASHTAG;
			}
			else
			{
				ePOSTAG *pTag = GetPoSTag(cmpWord);
				eTag1 = pTag[0]; 
				eTag2 = pTag[1];
			}

		}
	}

	// count follow letter nodes:
	int followCnt = 0;
	for (DictNode *dNode = startingNode->CharacterList; dNode != NULL; dNode = dNode->Next) 
	{
		followCnt++;
		m_nFollowPtrs++;
	}
	// count follow next words, if they exist:
	//int nNextWords = 0;
	//for (NGramNode *nextNode = startingNode->NextWords; nextNode != NULL; nextNode = nextNode->Next)
	//	nNextWords++;

	m_numStartingWords += (startingNode->StartPref > 0);
	CompactNode *cNode = m_compactStore->allocCompactNode(startingNode->Character,
						 (BYTE) startingNode->Preference, followCnt,
						 (BYTE) startingNode->EndPreference, false,
								startingNode->eVerbEndingFlag, startingNode->eVerbRootFlag,
								startingNode->eVerbVarIdx,
								startingNode->SuffixFlag && sCompactIdx > 5, startingNode->StartPref, eTag1, eTag2);

	m_nCompactNodes++;
	startingNode->compactNode = (int *) cNode;
	m_compactStore->setParent(cNode, parentCNode);
	// setting follow char node pointers:	
	int fillcnt = 0;
	for (DictNode *dNode = startingNode->CharacterList; dNode != NULL; dNode = dNode->Next)
	{
		CompactNode *nextCompactNode = compactTree(dNode, cNode);
		m_compactStore->updateCompactNodeAtIndx(cNode, fillcnt, nextCompactNode, PIN_FOLLOWPTRS);
		fillcnt++;
	}

	// setting next word node pointers:
	// first sort next words in decreasing pref order:
/**	NGramNode::sort(startingNode->NextWords);
	fillcnt = 0;
	for (NGramNode *nextNode = startingNode->NextWords; nextNode != NULL; nextNode = nextNode->Next)
	{
		CompactNode *next = (CompactNode *)nextNode->endNode->compactNode;
	/*	if(next==NULL)
		{
			printf("next word not set yet!\n");
			DictNode *curNode = nextNode->endNode->parent;
			DictNode *past = curNode;
			while(past)
			{
				curNode = past;
				past = past->parent;
			}
			next = compactTree(curNode, (CompactNode*)m_DictTreeStartNode->compactNode);
			assert(next != NULL);
		} // 
		if(next)
			m_compactStore->updateCompactNodeAtIndx(cNode, fillcnt, next, PIN_NEXTPTRS, PTR_SIZE);
		fillcnt++;
	}
	// setting next word pref values
	fillcnt = 0;
	for (NGramNode *nextNode = startingNode->NextWords; nextNode != NULL; nextNode = nextNode->Next)
	{
		int pref = (int)nextNode->pref;
		m_compactStore->updateCompactNodeValue(cNode, fillcnt, pref, PIN_NEXTPREFS, PREF_SIZE);
		fillcnt++;
	}
	***/
//	startingNode->compactNode = (int *) cNode;
	sCompactChar[--sCompactIdx] = NUL;
	return cNode;
}

///////////////////////////////////////////////////////////////////////////////////////////////
void NGramNode::sort(struct NGramNode *nodes)
{
	NGramNode *prev = NULL;
	for (NGramNode *nextNode = nodes; nextNode != NULL; )
	{
		if(prev && prev->pref < nextNode->pref)
		{
			// swap nodes' contents:
			DictNode *tmpEndNode = prev->endNode;
			int tmpPref = prev->pref;
			prev->endNode = nextNode->endNode;
			prev->pref = nextNode->pref;
			nextNode->endNode = tmpEndNode;
			nextNode->pref = tmpPref;
			// go back to beginning of list:
			nextNode = nodes->Next;
			prev = nodes;
		}
		else
		{
			prev = nextNode;
			nextNode = nextNode->Next;
		}
	}

	// being paranoid! check for duplicates:
	for (NGramNode *nextNode = nodes; nextNode != NULL; nextNode = nextNode->Next)
	{
		if(nextNode == nextNode->Next || (nextNode->Next && nextNode == nextNode->Next->Next))
			assert(0);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////
void CDictionaryTree::linkNextsInCompactTree(DictNode *startingNode)
{
/*	CompactNode *compactNode;

	if (startingNode == NULL)
		startingNode = m_DictTreeStartNode;
	assert ( startingNode->compactNode);
	compactNode = (CompactNode *) startingNode->compactNode;

		// first sort next words in decreasing pref order:
	NGramNode::sort(startingNode->NextWords);
	int fillcnt = 0;
	for (NGramNode *nextNode = startingNode->NextWords; nextNode != NULL; nextNode = nextNode->Next)
	{
		assert (nextNode->endNode->compactNode);
		CompactNode *next = (CompactNode *)nextNode->endNode->compactNode;
		m_compactStore->updateCompactNodeAtIndx(compactNode, fillcnt, next, PIN_NEXTPTRS, PTR_SIZE);
		fillcnt++;
	}

	// setting next word pref values
	fillcnt = 0;
	for (NGramNode *nextNode = startingNode->NextWords; nextNode != NULL; nextNode = nextNode->Next)
	{
		int pref = (int)nextNode->pref;
		m_compactStore->updateCompactNodeValue(compactNode, fillcnt, pref, PIN_NEXTPREFS, PREF_SIZE);
		fillcnt++;
	}

	for (DictNode *dNode = startingNode->CharacterList; dNode != NULL; dNode = dNode->Next)
	{
		linkNextsInCompactTree(dNode);
	}
	*/
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDictionaryTree::linkObjectsInCompactTree(DictNode *startingNode)
{
/*	CompactNode *compactNode;

	if (startingNode == NULL)
		startingNode = m_DictTreeStartNode;

	assert ( startingNode->compactNode);
	compactNode = (CompactNode *) startingNode->compactNode;

	int cnt = compactNode->Count;
	int objectCnt= m_compactStore->getObjectCount(compactNode, cnt);

	if (startingNode->listSpecNode != NULL)
	{
		assert (startingNode->listSpecNode->compactNode);
		m_compactStore->updateCompactNodePointer(compactNode, (CompactNode *) startingNode->listSpecNode->compactNode,PIN_PART_OF_LIST, PART_OF_LIST_SIZE);
	}

	if (startingNode->parent)
	{
		assert (startingNode->parent->compactNode);
		m_compactStore->updateCompactNodePointer(compactNode, (CompactNode *) startingNode->parent->compactNode, PIN_PARENT, PARENT_SIZE);
	}

	int fillcnt = 0;
	for (ObjectListNode *listNode = startingNode->ObjectList; listNode != NULL; listNode = listNode->Next)
	{
		assert (listNode->ObjectNode->compactNode);
		m_compactStore->updateCompactNode(compactNode,fillcnt, (CompactNode *) listNode->ObjectNode->compactNode,PIN_OBJECTPTRS,OBJECTPTRS_SIZE);
		fillcnt++;
	}

	for (DictNode *dNode = startingNode->CharacterList; dNode != NULL; dNode = dNode->Next)
	{
		linkObjectsInCompactTree(dNode);
	}

	assert ((objectCnt = m_compactStore->getObjectCount(compactNode, cnt)) <= 10);
*/
}

//////////////////////////////////////////////////////////////////////////////////////////
static void printRestOfWord(DictNode *dictNode) 
{
	DWORD dwWritten = 0;
	DictNode *dNode = NULL;
	int len = 0;
	MYWCHAR  mypreferencestring[100];
	memset(mypreferencestring,0, sizeof(mypreferencestring));

	putCharInWordString(dictNode->Character);

	if (dictNode->EndPointFlag)
	{
		len = wcslen(getWordString());
		WriteToFile(sOutputFile, (char *) getWordString(), wcslen(getWordString()) * sizeof(MYWCHAR));
		swprintf(mypreferencestring,100, L"\t%d",	dictNode->EndPreference);

		WriteToFile(sOutputFile, (char *) mypreferencestring, wcslen(mypreferencestring) * sizeof(MYWCHAR));
		WriteToFile(sOutputFile,"\x0d\x0a", 4);
	}

	for (dNode = dictNode->CharacterList; dNode != NULL; dNode = dNode->Next)
		printRestOfWord(dNode);

	takeCharOffWordString();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void printRestOfWordOnCondition(DictNode *dictNode, NodeSelection nodeSelection)
{
	DWORD dwWritten = 0;
	DictNode *dNode = NULL;
	int len = 0;
	BOOL printFlag = FALSE;
	MYWCHAR  mypreferencestring[100];

	putCharInWordString(dictNode->Character);
	switch (nodeSelection) 
	{
		case OnlyVerbRoots:
			if (dictNode->eVerbRootFlag)
				printFlag = TRUE;
			break;
		case OnlyNonZeroPref:
			if (dictNode->EndPointFlag && dictNode->EndPreference)
				printFlag = TRUE;
			break;
		case OnlyZeroPref:
			if (dictNode->EndPointFlag && ! dictNode->EndPreference)
				printFlag = TRUE;
			break;
		case FixWordFreqRelativeToPhrases:
			if (dictNode->EndPointFlag)
			{
				// if this is part of a phrase,
				// make sure that the leading word(s) freq is at least 
				// the same as the phrase itself.
			}
			break;

		case MinimumPref4:
			if (dictNode->EndPointFlag && dictNode->EndPreference >= 4)
				printFlag = TRUE;
			break;
		case Minimum5Characters:
			if (dictNode->EndPointFlag)
			{
				if (wcslen(getWordString()) < 5)
					dictNode->EndPreference = 1;
				printFlag = TRUE;
			}
			break;
		case Minimum5CharactersLC:
			if (dictNode->EndPointFlag)
			{
				BOOL bContainsSpace = FALSE;
				MYWCHAR *cp = getWordString(); 
				for (int i = 0; i < 5; i++)
				{
					if (cp[i] == SP)
						bContainsSpace = TRUE;
				}

				if (isLowerCase(cp[0]) && 
					( wcslen(getWordString()) < 5 ) || bContainsSpace )
					dictNode->EndPreference = 1;
				printFlag = TRUE;
			}
			break;
		case Minimum4CharactersLCExcludeTwoWordPhrases:
			if (dictNode->EndPointFlag)
			{
				int wordLen = wcslen(getWordString());
				int nSpaces = 0;
				MYWCHAR *cp = getWordString(); 
				int indivWordLen[30];

				if (isLowerCase(cp[0]))
				{
					for (int i = 0; i < wordLen ; i++)
					{
						if (cp[i] == SP)
						{
							indivWordLen[nSpaces] = i;
							nSpaces++;
						}
					}
					if (nSpaces)
						indivWordLen[nSpaces] = wordLen - indivWordLen[nSpaces-1] - 1;

					if (nSpaces == 1)
					{
						if (indivWordLen[0] < 4 && indivWordLen[1] < 4)
							break;
					}

					if (wordLen < 4 )
					{
						dictNode->EndPreference = 1;
					}
					printFlag = TRUE;
				}
				else
					printFlag = TRUE;
			}
			break;
		case Minimum5CharactersLCExcludeTwoWordPhrases:
			if (dictNode->EndPointFlag)
			{
				int wordLen = wcslen(getWordString());
				int nSpaces = 0;
				MYWCHAR *cp = getWordString(); 
				int indivWordLen[30];

				if (isLowerCase(cp[0]))
				{
					for (int i = 0; i < wordLen ; i++)
					{
						if (cp[i] == SP)
						{
							indivWordLen[nSpaces] = i;
							nSpaces++;
						}
					}
					if (nSpaces)
						indivWordLen[nSpaces] = wordLen - indivWordLen[nSpaces-1] - 1;

					if (nSpaces == 1)
					{
						if (indivWordLen[0] < 4 && indivWordLen[1] < 4)
							break;
					}

					if (wordLen < 5 )
					{
						dictNode->EndPreference = 1;
					}
					printFlag = TRUE;
				}
				else
					printFlag = TRUE;
			}
			break;
		case Minimum8CharactersLCExcludeTwoWordPhrases:
			if (dictNode->EndPointFlag)
			{
				int wordLen = wcslen(getWordString());
				int nSpaces = 0;
				MYWCHAR *cp = getWordString(); 
				int indivWordLen[200];

				if (isLowerCase(cp[0]))
				{
					for (int i = 0; i < wordLen ; i++)
					{
						if (cp[i] == SP)
						{
							indivWordLen[nSpaces] = i;
							nSpaces++;
						}
					}
					if (nSpaces)
						indivWordLen[nSpaces] = wordLen - indivWordLen[nSpaces-1] - 1;

					if (nSpaces < 2)
						break;

					if (nSpaces == 2)
					{
						if (indivWordLen[0] < 8 && indivWordLen[1] < 4) {
							break; // don't included it in the dictionary
						}
					}

					if (wordLen < 10 )
					{
						break; // don't included it in the dictionary
						dictNode->EndPreference = 1;
					}
					printFlag = TRUE;
				}
				else
					printFlag = TRUE;
			}
			break;
		default : 
			break;
	}

	if (printFlag == TRUE)
	{
		len = wcslen(getWordString());
		WriteToFile(sOutputFile, (char *) getWordString(), wcslen(getWordString()) * sizeof(MYWCHAR));

		swprintf(mypreferencestring,100,L"\t%d",	dictNode->EndPreference);
		WriteToFile(sOutputFile, (char *) mypreferencestring, wcslen(mypreferencestring) * sizeof (MYWCHAR));
		
		WriteToFile(sOutputFile, "\x0d\x0a", 4);
	}

	for (dNode = dictNode->CharacterList; dNode != NULL; dNode = dNode->Next)
	{
		printRestOfWordOnCondition(dNode, nodeSelection);
	}
	takeCharOffWordString();
}

/////////////////////////////////////////////////////////////////////////////////////////////
static DictNode *sSuffixRoot[10];
static void printallSuffixes()
{
	DictNode *dNode;
	MYWCHAR mystring[200];

	for (int i = 0; i < 10; i++)
	{
		swprintf(mystring,200,L"Suffixes len %d",i);
		WriteToFile(sOutputFile, (char *) mystring, wcslen(mystring)*sizeof(MYWCHAR));
		WriteToFile(sOutputFile, "\x0d\x0a",4);

		if (!sSuffixRoot[i])
			continue;

		for (dNode = sSuffixRoot[i]->CharacterList; dNode; dNode = dNode->Next)
		{
			printRestOfWord(dNode);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CDictionaryTree::addToEndings(MYWCHAR *totalWord, int suffixIdx, int suffixLen, SuffixAction suffixAction)
{
	MYWCHAR wconeWord[MAX_WORD_LEN];
	DictNode *dictNode;
	DictNode *prevDictNode = NULL;

	memset(wconeWord,0, sizeof(wconeWord));
	wcscpy(wconeWord,totalWord);

	if (!sSuffixRoot[suffixLen])
		sSuffixRoot[suffixLen] = allocDictNode(0, 0);

	MYWCHAR *chp = &wconeWord[suffixIdx];
	prevDictNode = sSuffixRoot[suffixLen];

	while (*chp)
	{
		dictNode = FindCharInTree(prevDictNode, *chp);
		if (dictNode == NULL)
			dictNode = AddCharInTree(prevDictNode, *chp, 0);

		prevDictNode = dictNode;
		chp++;
	}
	prevDictNode->EndPointFlag = 1;
	if (suffixAction == collect)
		prevDictNode->Preference++;

	return prevDictNode->Preference;
}

/////////////////////////////////////////////////////////////////////////////////////////
int CDictionaryTree::findSuffixes(DictNode *dictNode, SuffixAction suffixAction) 
{
	static char sChunkIndications[80];
	static char *sChunkIndP = sChunkIndications;
	static int count = 0;
	count++;
 	putCharInWordString(dictNode->Character);
	*sChunkIndP++ = dictNode->EndPointFlag;
	int val = 0;
	for (DictNode *dNode = dictNode->CharacterList; dNode != NULL; dNode = dNode->Next)
	{
		val = findSuffixes(dNode, suffixAction);
		if (dictNode->EndPointFlag && suffixAction == assign && val > 20) //it becomes a suffix if used in more than 20 occasions
		{
			dictNode->SuffixFlag = TRUE;
			val = 0;
		}
	}

	if(dictNode->CharacterList==NULL)//we reached the end of a word, now see if there is a chunk point in between the beginning and the end
	{
		for (char *indP = sChunkIndP-2 ; indP > sChunkIndications; indP--)
		{
			if (*indP)
			{
				MYWCHAR *mywordstring = getWordString();
				if(containsSPs(mywordstring))
					break;
				int suffixIdx = indP - sChunkIndications + 1;
				MYWCHAR *suffix = &mywordstring[suffixIdx];
				int suffixLen = sChunkIndP -1 - indP;
				if(NOT_SP_CR_TAB(suffix[0]) && suffixLen <= 9)
					val = addToEndings(mywordstring, suffixIdx , suffixLen, suffixAction);
				break;
			}
		}
	}
	
	*--sChunkIndP = 0;
	takeCharOffWordString();
	return val;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
static void printWordIfEVerb(DictNode *dictNode) 
{
	putCharInWordString(dictNode->Character);
	if (dictNode->EndPointFlag && dictNode->eVerbEndingFlag)
	{
		WriteToFile(sOutputFile, (char *) getWordString(), wcslen(getWordString()) * sizeof(MYWCHAR));
		WriteToFile(sOutputFile, "\x0d\x0a",4);
	}

	for (DictNode *dNode = dictNode->CharacterList; dNode != NULL; dNode = dNode->Next)
	{
		printWordIfEVerb(dNode);
	}
	takeCharOffWordString();
}

void CDictionaryTree::locateEVerbs(EVerbDefinition *eVerbDefinitions, BOOL markEndingsAsWell)
{
	for (int k = 0; eVerbDefinitions[k].triggers[0]!=NUL; k++) 
	{
		for (DictNode *dNode = m_DictTreeStartNode->CharacterList; dNode != NULL; dNode = dNode->Next)
			markEVerbs(dNode, &eVerbDefinitions[k], k, markEndingsAsWell);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
void CDictionaryTree::setSerializedEVerbsData(void *serializedEVerbData, int nBytes)
{
	m_serializedEVerbData = serializedEVerbData;
	m_serializedEVerbNDataBytes = nBytes;

	gTableSpaces[TBEVERBS].tablePtr		= serializedEVerbData;
 	gTableSpaces[TBEVERBS].sizeOfEntry  = nBytes;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
void CDictionaryTree::setSerializedRuleData(void *serializedRuleData, int nBytes)
{
	m_serializedRuleData = serializedRuleData;
	m_serializedRuleNDataBytes = nBytes;

	gTableSpaces[TBRULES].tablePtr		= serializedRuleData;
 	gTableSpaces[TBRULES].sizeOfEntry   = nBytes;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDictionaryTree::markEVerbs(DictNode *curNode, EVerbDefinition *curEVerbDefinition, BYTE eVerbVarIdx, BOOL markEndingsAsWell)
{
	putCharInWordString(curNode->Character);
	putDictnodeIntoNodepath(curNode);

	MYWCHAR *curWord = getWordString();
	int curWordLen = wcslen(curWord);
	int remainderLen = curWordLen - 1;//curEVerbDefinition->triggersLen;

	// at the deepest point we see whether this matches up with an EVerb triggerChars
	// in other words at 'e' of provide we start matching up
	if (curNode->EndPointFlag && remainderLen > 0  && wcscmp(&curWord[remainderLen], curEVerbDefinition->triggers) == 0)
	{		
		MYWCHAR rootChar = curWord[remainderLen - 1];
		if (canApplyEVerbDefinition(curEVerbDefinition, rootChar, curWord))
		{
			int highestEndPref = 0;
			// we could deal with a verb ending on e, lets test on it
			if (nodeHasEVerbVariationEndings(curNode,curEVerbDefinition, &highestEndPref))
			{
				DictNode *eVerbRootNode = dictNodePath[remainderLen]; // mark the beginning
				if (eVerbRootNode->eVerbRootFlag == FALSE)
				{
					printf("--verEnding %s  variation %s\n", toA(getWordString()), toA(curEVerbDefinition->triggers));
					// wasn't detected yet so set verbroot and everbending up
					eVerbRootNode->eVerbRootFlag = TRUE;
					eVerbRootNode->eVerbVarIdx = eVerbVarIdx;

					if (markEndingsAsWell == TRUE) 
					{
						curNode->eVerbEndingFlag = TRUE; // mark the end
					}
					else  
					{
						// the everbs are not used in the traditional form of infinitive form --> some tense variations,
						// instead, prediction stops at the root endpoint raised + suffix raised. From there we can
						// select various tenses, no space behind a non-word!!!!
						if (eVerbRootNode->EndPointFlag == FALSE)
						{
							eVerbRootNode->EndPointFlag = TRUE;
							eVerbRootNode->SuffixFlag = TRUE;
							// we pick up the strongest preference of all the tense forms and use its preference
							eVerbRootNode->EndPreference = max(highestEndPref, (int)eVerbRootNode->EndPreference);
						}
					}
				}
				else
					printf(" rootVerb has %s and %s and %s and %s\n",
							toA(getWordString()),
							toA(curEVerbDefinition->triggers),
							eVerbRootNode->EndPointFlag ? "Endpoint" : "No endpoint ",
							eVerbRootNode->SuffixFlag ? "Suffix" : "No suffix ");

				takeDictnodeOutOfNodepath();
				takeCharOffWordString();
				return;	// don't allow any multiple everb flags in one tree branch
			}
		}
	}
	for (DictNode *dNode = curNode->CharacterList; dNode != NULL; dNode = dNode->Next)
		markEVerbs(dNode, curEVerbDefinition, eVerbVarIdx, markEndingsAsWell);

	takeDictnodeOutOfNodepath();
	takeCharOffWordString();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CDictionaryTree::canApplyEVerbDefinition(EVerbDefinition *curEVerbDefinition, MYWCHAR rootChar, MYWCHAR *wordEnding)
{
	BOOL secondMatch = TRUE;
	if (curEVerbDefinition->mustEndOnOneOfTheseStrings)
	{
		secondMatch = FALSE;
		int wordEndingLen = mywcslen((const MYWCHAR*)wordEnding);

		for (int i = 0 ; curEVerbDefinition->mustEndOnOneOfTheseStrings[i]; i++)
		{
			int len = mywcslen((const MYWCHAR*)curEVerbDefinition->mustEndOnOneOfTheseStrings[i]);
			if (len <= wordEndingLen && !wcscmp(curEVerbDefinition->mustEndOnOneOfTheseStrings[i], &wordEnding[wordEndingLen - len]))
			{
				secondMatch = TRUE;
				break;
			}
		}
	}

	if (curEVerbDefinition->mustNotEndOnOneOfTheseStrings)
	{
		int wordEndingLen = mywcslen(wordEnding);

		for (int i = 0 ; curEVerbDefinition->mustNotEndOnOneOfTheseStrings[i]; i++)
		{
			int len = mywcslen((const MYWCHAR*)curEVerbDefinition->mustNotEndOnOneOfTheseStrings[i]);
			if (len <= wordEndingLen && !wcscmp(curEVerbDefinition->mustNotEndOnOneOfTheseStrings[i], &wordEnding[wordEndingLen - len])) 
			{
				return FALSE;
			}
		}
	}

	if (secondMatch == TRUE)
	{
		if (curEVerbDefinition->rootChars == rootChar)
		{
			return false; // curEVerbDefinition->mustMatchOneOfTheRootChars;
		}
		//if (curEVerbDefinition->mustNotMatchAnyOfTheRootChars)
			return TRUE;
	}
	return FALSE;
}

MYWCHAR variationWord[MAX_WORD_LEN*2];
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CDictionaryTree::nodeHasEVerbVariationEndings(DictNode *curNode, EVerbDefinition *curEVerbDefinition, int *highestPrefP)
{
	static int minimumMatch = 1;
	int nFindings = 0;
	DictNode *endOfVariationNode;
	wcscpy(variationWord, getWordString());
	int len = wcslen(variationWord);
	int highestEndPref = 0;

	if (len < 4)
		return FALSE;

	for (int i = 0; curEVerbDefinition->variationsOnEVerbs[i]; i++)
	{
		mywcscpy(&variationWord[len-1], curEVerbDefinition->variationsOnEVerbs[i]);
		endOfVariationNode = locateEndOfWordInDictionary(variationWord);
		if (endOfVariationNode && endOfVariationNode->EndPointFlag) 
		{
			nFindings++;
			highestEndPref = max((int) endOfVariationNode->EndPreference, highestEndPref);
		}
	}

	*highestPrefP = highestEndPref; 

//	printf(" matched %d times triggerChars %s  verb %s  variation %s \n",
//				nFindings,toA(curEVerbDefinition->triggerChars), toA(getWordString()), toA(variationWord));

	return (nFindings >= minimumMatch);
}

////////////////////////////////////////////////////////////////////////////////////
MYWCHAR * stripPrefixes(MYWCHAR *word, int *lenp)
{
	int nCharsToStrip = 0;

	if (_wcsnicmp( word, L"l\'",2) == 0 ||
		_wcsnicmp( word, L"d\'",2) == 0 ||
		_wcsnicmp( word, L"j\'",2) == 0 ||
		_wcsnicmp( word, L"n\'",2) == 0 ||
		_wcsnicmp( word, L"s\'",2) == 0
		)
		nCharsToStrip = 2;
	else if (_wcsnicmp( word, L"qu\'",3) == 0)
		nCharsToStrip = 3;
	else if (_wcsnicmp( word, L"all\'",4) == 0)
		nCharsToStrip = 4;
	else if (_wcsnicmp( word, L"dall\'",5) == 0 ||
			 _wcsnicmp( word, L"dell\'",5) == 0 ||
			 _wcsnicmp( word, L"sull\'",5) == 0
			 )
		nCharsToStrip = 5;

	if (nCharsToStrip)
		*lenp -= nCharsToStrip;
	return &word[nCharsToStrip];
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CheckIfFileUnicode(char* lpDictFileName)
{
	BOOL unicodeFlag = FALSE;
	MYWCHAR myBuffer[2];
	DWORD nBytesRead = 0;
	char myABuffer[0x800+1];

	FILE* hTempFile = fopen(lpDictFileName,"r"); 
	if(!hTempFile)     
	{ 
		printf("!!!!Bye! no file %s exists \n", lpDictFileName);
		return unicodeFlag;    
	}

	// lets test whether this file is Unicode or not
	nBytesRead = fread(myBuffer, 1 , 2, hTempFile) ; 
	if (nBytesRead == 2) 
	{
		if (myBuffer[0] == 0xfeff)
			unicodeFlag = TRUE;
		else 
		{
			// don't know what kind of file this is , leave the unicode flag a spec'd
			DWORD fileSize = getFileSize(hTempFile);
			int nHigh = 0;
			int nLow = 0;
			for (DWORD k=2; k < fileSize; k+=0x800)
			{
				memset(myABuffer,0, sizeof(myABuffer));
				nBytesRead = readFromFile(hTempFile, myABuffer, 0x800) ; 
				if (nBytesRead > 0)
				{
					for (DWORD l = 0; (l+1) < nBytesRead; l += 2)
					{
						if (myABuffer[l])
							nLow++;
						if (myABuffer[l+1])
							nHigh++;
					}
				}
			}

			if (nLow > (nHigh * 3))
				unicodeFlag = TRUE;
			else
				unicodeFlag = FALSE;
		}
	}
	fclose(hTempFile);
	return unicodeFlag;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// unicodeFlag if false is interpreted as UTF8, otherwise is UTF16 So regular character in ansi up to 0x7F are still the same. 
// Also control characters are the same as ANSI. In our case, we use unicodeFlag = true only for case of 
// 2byte characters. In this case, regular characters are treated as 2 bytes, but controls are same as above.
StartWord *CDictionaryTree::processDictFile( char* lpDictFileName, char *parsingSpec, int storeType, Dictionary *existingDict, 
									int fileId, int thresHoldPref, BOOL lowerCaseFlag, BOOL unicode)
{
	StartWord* pStartWord = new StartWord[MAX_PHRASE_ARRAY];

	MYWCHAR szWOneLine[MAX_WORD_LEN];
	memset(szWOneLine, 0, sizeof(szWOneLine));

	MYWCHAR currentListName[MAX_WORD_LEN];
	memset(currentListName,0,sizeof(currentListName));

	DictNode *listSpecificationNode = NULL;
	BOOL listActive = FALSE;
	DWORD nBytesRead = 0;
	BOOL bResult = FALSE;
	int thresholdPref = 0;	// process everything;
	MYWCHAR sepChar = SP;

	memset(currentListName, 0 , sizeof(currentListName));

	// check if the file is unicode. Probably not necessary anymore
	//unicodeFlag = CheckIfFileUnicode(lpDictFileName);
	// Open the dictionary file     
	FILE* fp = fopen(lpDictFileName,"r");      
	if(!fp)     
	{ 
		printf("!!!!ERROR!! CDictionaryTree::processDictFile: no file %s exists \n", lpDictFileName);
		exit(1);    
	}
	
	// reset getLine structures
	getLine(0, NULL, unicode);
	if (strcmp(parsingSpec,"tabdelimited") == 0)
	{
		sepChar = HTAB;
		parsingSpec = "sp";
	}

	CParsedLine   *lineParser = new CParsedLine(parsingSpec, thresholdPref, sepChar);
	//eEndianNess endianflag = unicode ? checkEndianState(fp) : eLITTLE_ENDIAN32;

	int nStartWordCount = 0;
	while (getLine(fp,  szWOneLine, unicode) == TRUE)
	{
		lineParser->ScanDictLine(szWOneLine);
		if(lineParser->m_StartPref > 0)
		{
			WLBreakIf(nStartWordCount >= MAX_PHRASE_ARRAY, "!!ERROR!! Not enough size for startWord!!");
			lineParser->m_StartPref = max(lineParser->m_StartPref, LEARNED_START_PREF);
			mywcscpy(pStartWord[nStartWordCount].word, lineParser->m_word);
			pStartWord[nStartWordCount].pref = lineParser->m_StartPref;
			nStartWordCount++;
		}

		if ( (lineParser->m_pref > thresHoldPref)  || existingDict || lineParser->m_bListSpecified)
		{
		//	printf("pref = %d > %d\n", lineParser->m_preference, thresHoldPref);
			m_nCharsInWordsUsed += wcslen(lineParser->m_word);
			m_nWordsUsed++;
			if (lowerCaseFlag)
				mywcslwr(lineParser->m_word);
			
			if (existingDict && existingDict->countWord(lineParser->m_word) > 0)
					continue;

			 m_nNewWords += storeDictionaryPhrase(lineParser, fileId, storeType, listSpecificationNode);
		}
		else
			m_nWordsIgnored++;
	}

	delete lineParser;
	fclose(fp);

	return pStartWord;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//Tries to read first 2 byte for BOM (Byte Order Marker) and determine its endianness
eEndianNess CDictionaryTree::checkEndianState(FILE *fp)
{
	eEndianNess endianflag = eLITTLE_ENDIAN32;
	byte bom[2];
	fread(bom, 1, 2, fp);
	if(bom[0] == 0xff && bom[1] == 0xfe)
		endianflag = eLITTLE_ENDIAN32;
	else if(bom[0] == 0xfe && bom[1] == 0xff)
		endianflag = eBIG_ENDIAN32;
	else // none of the above. So it doesn't have BOM, go back 2 byte on file pointer
		fseek(fp, 0, SEEK_SET);
	return endianflag;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CDictionaryTree::processPhraseFollowupFile(char* FollowupFileName, char *parsingSpec, BOOL unicodeFlag)
{
	// first try to add the phrases to phrase array, just in case some are missing!
    int numAdded = 0;
//	unsigned numPhrases = countNumLinesInFile(FollowupFileName);
	processPhraseFile(FollowupFileName, parsingSpec, unicodeFlag, numAdded, true, true);
	for(int i=0; i <numAdded; i++)
	{
		int len = 0;
		assert(gPhraseAr[i].endNodes[0] != NULL);
		MYWCHAR **pStr = gPhraseAr[i].getStr(m_compactStore, len);
		ShowInfo("ProcessPhrase:: %d: len=%d, #%s %s %s %s#\n", i, len,
				  toA(pStr[0]), toA(pStr[1]),toA(pStr[2]), toA(pStr[3]));
		mPhraseEngine->addPhrase(&gPhraseAr[i], true);
	}

	int count =0;
	for(int j=0; j<mNumPhraseProcessed; j++)
	{	
		// process pair wise phrases for p 2 p linking
		WordNode  *endWord1 = gPhraseAr[j++].wordNodeP;
		WordNode  *endWord2 = gPhraseAr[j].wordNodeP;
		assert(endWord2 && endWord1);
		endWord1->mNextPhraseEndWord[endWord1->nextPhraseCount] = endWord2;
		endWord1->mNextPhrasePref[endWord1->nextPhraseCount++] = gPhraseAr[j].pref;//endWord1->pref;
	}

	return TRUE;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This function process one grams which are supposed to be starting words. NOTE: this function can be called only
// after compactTree() is called on dictionaryTree.
// It is assumed that startWords is a null ending list!
int CDictionaryTree::processStartingWordsList(StartWord *startWords)
{
	if(!startWords)     
	{ 
		printf("!!!!ERROR!!!processStartingWordsList: list is empty! \n");
		return 0;    
	}
	
	gFirstNodeOfDicts[m_language] = (CompactNode *) ((GENPTR) m_compactStore->m_start_compact + sizeof(GENPTR));
		
	memset(gPhraseAr, 0, sizeof(gPhraseAr));
	int count =0;
	StartWord *startHead = startWords;
	MYWCHAR theWord[MAX_WORD_LEN];
	int pref;
	while (startHead && startHead->word[0] != NUL)// && count < MAX_PHRASE_ARRAY)
	{	
		mywcscpy(theWord, startHead->word);
		pref = startHead->pref;
		if(mywcscmp(theWord, L"I")!=0 && mywcsncmp(theWord, L"I'", 2) != 0)
			MakeLowerCase(theWord);
				
		CompactNode* endnode = m_compactStore->retrieveEndNodeForString(theWord, false);
		WLBreakIf(endnode==NULL, "!!Error!!processStartingWordsList: endnode for %s must be NOT null\n!", toA(theWord));
		gPhraseAr[count].endNodes[0] = endnode;
		gPhraseAr[count].pref = startHead->pref;
		gPhraseAr[count].startPhrase = true;
	//	mPhraseHeader->phrasesCount++;
		startHead++;
		count++;
	}


	qsort (gPhraseAr, MAX_PHRASE_ARRAY, sizeof(NGramMultiNode), NGramMultiNodeCompare);

	// now process gPhraseAr by adding its content to starting tree overlay
	int phraseCount = 0;
	NGramMultiNode stNode;
	for(int i=0; i <count; i++)
	{
		assert(gPhraseAr[i].endNodes[0] != NULL);
		mPhraseEngine->addStartPhrase(&gPhraseAr[i]);
	}

	return count;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CDictionaryTree::processPhraseFile(char* phraseFileName, char *parsingSpec, BOOL unicodeFlag, int &numPhraseProcessed, bool reset, bool bP2P)
{
	static MYWCHAR szWOneLine[MAX_PHRASE_LEN];
	numPhraseProcessed = 0;

	if( mFP == NULL || reset)
	{
		mNumPhraseProcessed = 0;
		mSepChar = SP;
		if(mLineParser)
			delete mLineParser;

		if(mFP != NULL)
			fclose(mFP);
		mFP = fopen(phraseFileName, "r");
		if(!mFP)     
		{ 
			printf("!!!!ERROR!!!processPhraseFile::Bye no file %s exists \n", phraseFileName);
			return false;    
		}
		assert(gFirstNodeOfDicts[m_language] == (CompactNode *) ((GENPTR) m_compactStore->m_start_compact + sizeof(GENPTR))); 

		memset(szWOneLine, 0, sizeof(szWOneLine));
		// reset getLine structures
		getLine(0, NULL, unicodeFlag);
		if (strcmp(parsingSpec,"tabdelimited") == 0)
		{
			mSepChar = HTAB;
			parsingSpec = "sp";
		}

		mLineParser = new CParsedLine(parsingSpec, 0, mSepChar);
		mEndianflag = checkEndianState(mFP); // this step is needed to read off the first 2 BOM bytes, if they exist!
	}

	memset(gPhraseAr, 0, sizeof(gPhraseAr));
	int count =0;
	bool res = true;
	while (count < MAX_PHRASE_ARRAY)
	{	
		if(!getLine(mFP,  szWOneLine, unicodeFlag))
		{
			if(bP2P && count%2 != 0)
				count--;
			break;
		}
		//ShowInfo("%d. %s\n",count,toA(szWOneLine));
		if(!bP2P)
			count += SaveNGramWord(szWOneLine, mSepChar, count);
		else
			count = SaveNGramWordFromP2P(szWOneLine, mSepChar, count);
	}

	numPhraseProcessed = count;
	mNumPhraseProcessed += count;
	if(res == false || bP2P || feof(mFP))
	{
		fclose(mFP);
		mFP = NULL;
		delete mLineParser;
		mLineParser = NULL;
		return false;
	}

	return true;
}
unsigned CDictionaryTree::countNumLinesInFile(char *filename)
{
	int number_of_lines = 0;
	std::ifstream myfile;
	myfile.open(filename);
	std::string line;
	while (std::getline(myfile, line))
		++number_of_lines;

	std::cout << "Number of phrase in file " << filename << " is " << number_of_lines;
	return number_of_lines;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool CDictionaryTree::SaveNGramWord(MYWCHAR* phrase, MYWCHAR delimiter, int idx)
{
	memset(m_PhraseWords, 0, sizeof(m_PhraseWords));

	int nSpace = SetPhraseWord(phrase, delimiter);
	bool bChangeToLower = true;
	if(nSpace < 1 || isNumberChar(m_PhraseWords[1][0]))
		return false;
	
	NGramMultiNode stNode;
	stNode.pref = m_nPref;
	//check if phrase already been processed!
/*	unsigned long hash = djb2_hash(phrase);
	for(int i= 0; i<gTotalPhraseProcessed; i++)
	{
		if(gPhraseHash[i] == hash)
		{
			hash = 0;
			break;
		}
	}
	
	if(hash == 0)
	{
		ShowInfo("SaveNGramWord:: Phrase %s already exists in gPhraseAr array. Ignore the duplicate!\n", toA(phrase));
		return false;
	}
	gPhraseHash[gTotalPhraseProcessed++] = hash;
	*/
	assert(gPhraseAr[idx].endNodes[0] == NULL); // this slot in gPhraseAr must be empty!
	{
		for(int k=0; k < nSpace; k++)
		{
		
			//if(mywcscmp(m_PhraseWords[k], L"I")!=0 && mywcsncmp(m_PhraseWords[k], L"I'", 2)!=0 && bChangeToLower)
				//MakeLowerCase(m_PhraseWords[k]);
				
			MYWCHAR* inputWord = m_PhraseWords[k];
			assert(!isEmptyStr(inputWord));
			{
				CompactNode* endnode = m_compactStore->retrieveEndNodeForString(inputWord, false);
				WLBreakIf(endnode==NULL, "!!Error!!SaveNGramWord: endnode for %s must be NOT null\n!", toA(inputWord));
				gPhraseAr[idx].endNodes[k] = endnode;
				//m_compactStore->setPtrFieldAtOffset((char*)&gPhraseAr[idx].endNodes[k], PTR_SIZE, 0, (UINT) endnode);
			}
		}
		SetUniqueWordCount(idx);
		mPhraseHeader->phrasesCount++;
		gPhraseAr[idx].pref = stNode.pref;
		gPhraseAr[idx].startPhrase = false;
		ShowInfo("%d. %s\n", mPhraseHeader->phrasesCount, toA(phrase));
	}
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CDictionaryTree::SaveNGramWordFromP2P(MYWCHAR* phrase,MYWCHAR delimiter, int idx)
{
	memset(m_P2PWords, 0 , sizeof(m_P2PWords));

	int nSpace = SetPhraseToPhraseWord(phrase, delimiter);
	bool bChangeToLower = true;
	if(nSpace < 1 || isNumberChar(m_P2PWords[1][0]))
		return idx;
	
	NGramMultiNode stNode;
	stNode.pref = m_nPref;
	
	int colonCnt = 0;
	int endNodeIndex = 0;
	assert(gPhraseAr[idx].endNodes[0] == NULL); // this slot in gPhraseAr must be empty!
	{
		for(int k=0; k < nSpace; k++) //actually, 2 phrasese here.
		{		
			MYWCHAR* inputWord = m_P2PWords[k];
			if(inputWord[0] == COLON) // 2 colons
			{
				colonCnt++;
				continue;
			}
			if(colonCnt == 2)
			{
				if(gPhraseAr[idx].wordCount() <= 1)
				{
					gPhraseAr[idx].reset();
					return idx;
				}
				SetUniqueWordCount(idx);
				mPhraseHeader->phrasesCount++;
				gPhraseAr[idx].pref = stNode.pref;
				gPhraseAr[idx].startPhrase = false;
				ShowInfo("%d. %s\n", mPhraseHeader->phrasesCount, toA(phrase));

				//Get next phrase after "::"
				idx++;
				endNodeIndex =0;
				assert(gPhraseAr[idx].endNodes[0] == NULL); // this slot in gPhraseAr must be empty!
				colonCnt++;
			}

			//if(mywcscmp(inputWord, L"I")!=0 && mywcsncmp(inputWord, L"I'", 2)!=0 && bChangeToLower)
				//MakeLowerCase(inputWord);
				
			
			assert(!isEmptyStr(inputWord));
			{
				CompactNode* endnode = m_compactStore->retrieveEndNodeForString(inputWord, false);
				WLBreakIf(endnode==NULL, "!!Error!!SaveNGramWordFromP2P: endnode for %s must be NOT null\n!", toA(inputWord));
				gPhraseAr[idx].endNodes[endNodeIndex++] = endnode;
				//m_compactStore->setPtrFieldAtOffset((char*)&gPhraseAr[idx].endNodes[k], PTR_SIZE, 0, (UINT) endnode);
			}
		}

		if(gPhraseAr[idx].wordCount() <= 1)
		{
			gPhraseAr[idx].reset();
			return idx;
		}
		SetUniqueWordCount(idx);
		mPhraseHeader->phrasesCount++;
		gPhraseAr[idx].pref = stNode.pref;
		gPhraseAr[idx].startPhrase = false;
		ShowInfo("%d. %s\n", mPhraseHeader->phrasesCount, toA(phrase));
	}

	WLBreakIf(colonCnt !=3, "!!Error!!SaveNGramWordFromP2P: colons do not exist?\n!");
	idx++;

	return idx;

}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
// sets up offline phrase engine. This engine is later on compactified, so can be used at run time.
void CDictionaryTree::setupPhraseEngine(unsigned totalNumPhrases, unsigned numPhrases, unsigned pageIdx)
{
	if(!mPhraseEngine && pageIdx == 0)
	{
		mPhraseEngine = new PhraseEngine();
		mPhraseHeader->eLang = m_language;
		mPhraseEngine->set(totalNumPhrases, *mPhraseHeader, m_compactStore);
	}
//	else
//		mPhraseEngine->setMemoryStore(pageIdx);

	int phraseCount = 0;
//	NGramMultiNode stNode;
	for(int i=0; i <numPhrases; i++)
	{
		assert(gPhraseAr[i].endNodes[0] != NULL);
	//	memcpy(&stNode, &gPhraseAr[i], sizeof(NGramMultiNode));
		mPhraseEngine->addPhrase(&gPhraseAr[i]);
	}
	mPhraseEngine->mPhraseFileHeader.rootChildCount = mPhraseEngine->mActualNextCount;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDictionaryTree::compactPhraseEngine()
{
	// first initialize compact phrase engine mPhraseEngineR:
	mPhraseEngine->mPhraseFileHeader.phrasesCount = PhraseEngine::sTotalPhraseCount;
	mPhraseHeader->phrasesCount = PhraseEngine::sTotalPhraseCount;
	mPhraseHeader->startWordCount = mPhraseEngine->mActualStartCount;
	mPhraseHeader->rootChildCount = mPhraseEngine->mActualNextCount;
//	mPhraseEngine->sMemArray.heap.unCommit();
	mPhraseEngineR = new PhraseEngineR(*mPhraseHeader);

	UINT memSize = mPhraseEngine->sMemArray.memUsed();
	mPhraseEngineR->initialize(memSize);
	mPhraseEngineR->mCStore = mPhraseEngine->mCStore;

	int nextcount = mPhraseEngineR->mHeader.rootChildCount;
	
	assert(!mPhraseEngineR->mRoot);
	mPhraseEngineR->mRoot = mPhraseEngineR->allocCompactWord(0, 0, nextcount, mPhraseHeader->startWordCount);

	// now, compactify phrases
	compactPhrases(NULL, NULL);

	// then, setting up next phrases. 
	setupNextPhrases(NULL, NULL);
}

bool sortWordNodeByEndAddress(const WordNode *word1, const WordNode *word2) { return (UINT)word1->endCNode < (UINT)word2->endCNode; }
////////////////////////////////////////////////////////////////////////////////////////////////////////
CompactWord* CDictionaryTree::compactPhrases(WordNode *startingNode, CompactWord *parent)
{
	int nextcount=0, nextPhraseCount=0;
	BYTE pref = 0;
	CompactNode *endCNode = NULL;
	USHORT postag = ePOS_NOTAG;
	//ePOSTAG* pTag = NULL;
	CompactWord* cWord = mPhraseEngineR->mRoot;
	WordNode **nextWordsP;

	if(parent)
	{
		assert(startingNode);
		nextcount = startingNode->count;
		nextPhraseCount = startingNode->nextPhraseCount;
		pref = startingNode->pref;
		endCNode = startingNode->endCNode;
		postag = (startingNode->POSTags[2] << 6 | startingNode->POSTags[3]);
		assert(pref >= startingNode->endPref);
		assert(startingNode->endPref == 0 || startingNode->endFlag);
		assert(!startingNode->endFlag || startingNode->endPref > 0);
		cWord = mPhraseEngineR->allocCompactWord(postag, pref, nextcount, nextPhraseCount, startingNode->endPref, startingNode->endFlag);
		assert(cWord);
		startingNode->cWord = cWord;
		nextWordsP = startingNode->mNextWords;
	}
	else
	{
		nextcount = mPhraseEngineR->mHeader.rootChildCount;
		nextPhraseCount = mPhraseEngineR->mHeader.startWordCount;
		nextWordsP = mPhraseEngine->mHeadWords;
	}

	mPhraseEngineR->setEndCNode(cWord, endCNode);
	mPhraseEngineR->setParent(cWord, parent);

	//setting next words pointers
	// first sort the list of nextwords based on word's endnode address in ascending order:
	std::sort(nextWordsP, nextWordsP+nextcount, sortWordNodeByEndAddress);

	for(int j=0; j<nextcount; j++)
	{
		WordNode *wordnode = nextWordsP[j];
		CompactWord *nextCompactWord = compactPhrases(wordnode, cWord);
		mPhraseEngineR->updateCompactWordAtIndx(cWord, j, (UINT)nextCompactWord, PE_NEXTWORDS);
	}

	return cWord;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
CompactWord* CDictionaryTree::setupNextPhrases(WordNode *startingNode, CompactWord *parent)
{
	int nextcount=0, nextPhraseCount=0;
	CompactWord* cWord = mPhraseEngineR->mRoot;
	WordNode **nextWordsP;

	if(parent)
	{
		assert(startingNode);
		nextcount = startingNode->count;
		nextPhraseCount = startingNode->nextPhraseCount;
		nextWordsP = startingNode->mNextWords;
		cWord = startingNode->cWord;
	}
	else
	{
		nextcount = mPhraseEngineR->mHeader.rootChildCount;
		nextPhraseCount = mPhraseEngineR->mHeader.startWordCount;
		nextWordsP = mPhraseEngine->mHeadWords;
	}

	//setting next words pointers
	for(int j=0; j<nextcount; j++)
	{
		WordNode *wordnode = nextWordsP[j];
		CompactWord *nextCompactWord = setupNextPhrases(wordnode, cWord);
	}

	//setting next phrases
	assert(nextPhraseCount<MaxNextPhrases);
	for(int i=0; i<nextPhraseCount; i++)
	{
		WordNode *wordNode;
		BYTE pref = 0;
		if(parent)
		{
			wordNode = startingNode->mNextPhraseEndWord[i];
			pref = startingNode->mNextPhrasePref[i];
		}
		else
		{
			wordNode = mPhraseEngine->mStartWords[i].word;
			pref = mPhraseEngine->mStartWords[i].pref;
		}

		assert(wordNode);
		CompactWord *nextCWord = wordNode->cWord;
		assert(nextCWord);
		mPhraseEngineR->updateCompactWordValueAtIndex(cWord, i, (UINT)nextCWord, pref, PE_NEXTPHRASES, PREF_SIZE);
	}
	return cWord;
}
///////////////////////////////////////////////////////////////////////////////
bool CDictionaryTree::compactToLDatFile(char *fullpath)
{
	//open file to write:
	ShowInfo("CDictionaryTree::compactToLDatFile: file %s \n", fullpath);
	FILE *fp = fopen(fullpath, "wb");
	if(fp!=NULL)
	{
		CCompactStore *cStore = GetCompactStore();
		// first write out the dictionary part:
		assert(cStore && cStore->m_buildHeader && cStore->m_available > cStore->m_start_compact);
		cStore->m_buildHeader->totalDataSize = cStore->m_available - cStore->m_start_compact;
		assert((int)cStore->m_buildHeader->totalDataSize > 0);
		WriteToFile(fp, (char *) cStore->m_buildHeader, cStore->m_buildHeader->sizeTotal);
		WriteToFile(fp, cStore->m_start_compact, cStore->m_buildHeader->totalDataSize);

		//next write out the phrase part:
	//	gPhraseHeader.eLang = m_language;
	//	gPhraseHeader.totalDataSize =  mPhraseEngineR->m_available - mPhraseEngineR->m_start_compact;
		mPhraseEngineR->mHeader.totalDataSize = mPhraseEngineR->m_available - mPhraseEngineR->m_start_compact;
		WriteToFile(fp, (char *)&mPhraseEngineR->mHeader, sizeof(PhraseHeader));
		WriteToFile(fp, mPhraseEngineR->m_start_compact, mPhraseEngineR->mHeader.totalDataSize);

		fclose(fp);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CDictionaryTree::SetDictManager(char *dictname)
{
	static CDictManager * gDictManager = NULL;
	if(gDictManager)
	{
		gDictManager->Destroy();
		gDictManager = NULL;
	}

	gDictManager = new CDictManager();
	if(gDictManager->Create(dictname, ""))
	{
		return true;
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CDictionaryTree::processAddNewWordFromPhraseFile(char* phraseFileName, char *parsingSpec,BOOL unicodeFlag, bool bP2P)
{
	MYWCHAR szWOneLine[MAX_PHRASE_LEN];
	memset(szWOneLine,0,sizeof(szWOneLine));
  
	MYWCHAR sepChar = SP;
	FILE* fp = fopen(phraseFileName,"r");      
	if(!fp)     
	{ 
		printf("!!!!processAddNewWordFromPhraseFile: Bye no file %s exists \n", phraseFileName);
		return FALSE;    
	}
	
	// reset getLine structures
	getLine(0, NULL, unicodeFlag);
	if (strcmp(parsingSpec,"tabdelimited") == 0)
	{
		sepChar = HTAB;
		parsingSpec = "sp";
	}

	CParsedLine   *lineParser = new CParsedLine(parsingSpec, 0, sepChar);
//	eEndianNess endianflag = unicodeFlag ? checkEndianState(fp) : eLITTLE_ENDIAN32;

	while (getLine(fp,  szWOneLine, unicodeFlag) == TRUE)
	{	
	
		if(!bP2P)
			StoreNewWordsInDict(szWOneLine, SP);
		else
			StoreNewP2PWordsInDict(szWOneLine, SP);
	}

	fclose(fp);
	delete lineParser;

	return TRUE;
}
//////////////////////////////////////////////////////////////////////////////////////////////
int CDictionaryTree::SetPhraseWord(MYWCHAR* phrase, MYWCHAR delimiter)
{
	int n =mywcslen(phrase);
	memset(m_PhraseWords, 0, sizeof(m_PhraseWords));

	int nSpace =0;
	int index =0;
	m_nPref =0;
	for(int i=0; i < n; i++)
	{
		if( nSpace > 3 || (isNumberChar(phrase[i]) && nSpace > 1))
		{
			MYWCHAR pref[MAX_WORD_LEN];
			int ind=0;
			for(int k=i; k < n;k++)
			{
				if(phrase[k] == HTAB || phrase[k] == SP)
				{
					break;
				}
				pref[ind++] = phrase[k];
			}
			pref[ind] = NUL;
			m_nPref = myStrToInt(pref);
			break;
		}
		if(phrase[i] == delimiter)
		{
			nSpace++;
			index =0;
		}
		else
		{
			m_PhraseWords[nSpace][index++] = phrase[i];
		}
	}

	return nSpace;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CDictionaryTree::StoreNewWordsInDict(MYWCHAR* phrase,MYWCHAR delimiter)
{
	
	int nSpace = SetPhraseWord(phrase, delimiter);
	if(nSpace < 2)
	{
		return;
	}
	for(int k=0; k < nSpace; k++)
	{
		//if(mywcscmp(m_PhraseWords[k], L"I")!=0)
		//	MakeLowerCase(m_PhraseWords[k]);
				
		MYWCHAR* inputWord = m_PhraseWords[k];

		assert(!isEmptyStr(inputWord));
		{
			if(!isNumber(inputWord) && !isWordInTree(inputWord))
			{
				ShowInfo("NEW WORD:%s--------------->(%s)\n",toA(inputWord), toA(phrase));
				addNewWordAsOneUnit(inputWord,1,NULL,1,WORDSIDE_DICTIONARY,NULL);
			}
		}
	}
}
/////////////////////////////////////////////////////////////////////////////////////
int CDictionaryTree::SetPhraseToPhraseWord(MYWCHAR* phrase, MYWCHAR delimiter)
{
	int n =mywcslen(phrase);
	memset(m_P2PWords, 0, sizeof(m_P2PWords));

	int nSpace =0;
	int index =0;
	m_nPref =0;
	bool bColon = false;
	for(int i=0; i < n; i++)
	{
		if(phrase[i] == COLON)
		{
			nSpace++;
			index =0;
			m_P2PWords[nSpace][0] = phrase[i];
			bColon  = true;
			continue;
		}


		if( nSpace > 9 || (isNumberChar(phrase[i]) && nSpace > 1))
		{
			MYWCHAR pref[MAX_WORD_LEN];
			int ind=0;
			for(int k=i; k < n;k++)
			{
				if(phrase[k] == HTAB || phrase[k] == SP)
				{
					break;
				}
				pref[ind++] = phrase[k];
			}
			pref[ind] = NUL;
			m_nPref = myStrToInt(pref);
			break;
		}

		if(phrase[i] == delimiter)
		{
			nSpace++;
			index =0;
		}
		else
		{ 
			if(bColon)
			{
				nSpace++;
				bColon  = false;
			}
			m_P2PWords[nSpace][index++] = phrase[i];
		}
	}

	return nSpace;

}
/////////////////////////////////////////////////////////////////////////////////////
void CDictionaryTree::StoreNewP2PWordsInDict(MYWCHAR* phrase,MYWCHAR delimiter)
{
	int nSpace = SetPhraseToPhraseWord(phrase, delimiter);
	for(int k=0; k < nSpace; k++)
	{
		//if(mywcscmp(m_P2PWords[k], L"I")!=0)
		//	MakeLowerCase(m_P2PWords[k]);
				
		MYWCHAR* inputWord = m_P2PWords[k];

		assert(!isEmptyStr(inputWord));
		{
			if(!isNumber(inputWord) && !isWordInTree(inputWord) && m_P2PWords[k][0] != COLON)
			{
				ShowInfo("NEW WORD for P2P:%s---------->%s\n",toA(inputWord), toA(phrase));
				addNewWordAsOneUnit(inputWord,1,NULL,1,WORDSIDE_DICTIONARY,NULL);
			}
		}
	}
}
/////////////////////////////////////////////////////////////////////////////////////
BOOL CDictionaryTree::processPoSTag(char* PoSFileName, char *parsingSpec, BOOL unicodeFlag)
{
	MYWCHAR szWOneLine[MAX_PHRASE_LEN];
	memset(szWOneLine,0,sizeof(szWOneLine));
  
	MYWCHAR sepChar = SP;
	FILE* fp = fopen(PoSFileName,"r");      
	if(!fp)     
	{ 
		printf("!!!!Bye no file %s exists \n", PoSFileName);
		return FALSE;    
	}
	
	// reset getLine structures
	getLine(0, NULL, unicodeFlag);
	if (strcmp(parsingSpec,"tabdelimited") == 0)
	{
		sepChar = HTAB;
		parsingSpec = "sp";
	}

	CParsedLine   *lineParser = new CParsedLine(parsingSpec, 0, sepChar);
//	eEndianNess endianflag = unicodeFlag ? checkEndianState(fp) : eLITTLE_ENDIAN32;

	int count =0;
	while (getLine(fp,  szWOneLine, unicodeFlag) == TRUE)
	{	
		if(SavePoSTag(szWOneLine, sepChar))
		{
			count++;
			//ShowInfo("PoS %d:(%s)\n",count, toA(szWOneLine));
		}
	}

	fclose(fp);
	delete lineParser;

	////////////////////Testing/////////////////////////
	//MYWCHAR* tp = L"mark";
	//PrintPoSTagResult(tp);
	//tp = L"of";
	//PrintPoSTagResult(tp);
	//tp = L"iaaf";
	//PrintPoSTagResult(tp);
	//tp = L"english";
	//PrintPoSTagResult(tp);
	//tp = L"minkyu";
	//PrintPoSTagResult(tp);
	//tp = L"zyzzyvas";
	//PrintPoSTagResult(tp);
	/////////////////////////////////////////////////////

	return TRUE;
}
/////////////////////////////////////////////////////////////////////////////////////
void CDictionaryTree::PrintPoSTagResult(MYWCHAR* word)
{
	ShowInfo("-----------------------eTag word:%s--------------------------\n", toA(word));
	ePOSTAG* eTag = GetPoSTag(word);
	for(int i=0; i < MAX_POSTAG_NAME_SIZE; i++)
	{
		ePOSTAG t = eTag[i];
		ShowInfo("eTag:%d\n", t);
	}
	ShowInfo("\n");
}
/////////////////////////////////////////////////////////////////////////////////////
int CDictionaryTree::CreatePoSTag(int alphaInd)
{
	int index = -1;
	for(int i=0; i < MAX_POSTAG_SIZE;i++)
	{
		if(gPoSTag[alphaInd][i] == NULL)
		{
			gPoSTag[alphaInd][i] = new PartOfSpeechTag();
			index = i;
			break;
		}
	}

	return index;
}
/////////////////////////////////////////////////////////////////////////////////////
bool CDictionaryTree::SavePoSTag(MYWCHAR* phrase, MYWCHAR delimiter)
{
	MYWCHAR posWord[MAX_WORD_LEN];
	memset(posWord, 0, sizeof(posWord));
	int idx= 0;
	for(idx=0; idx < MAX_WORD_LEN && phrase[idx] != SP; idx++)
	{
		posWord[idx] = phrase[idx];
	}

	WLBreakIf(idx==0, "!!ERROR!! PosTag No Word???\n");
	//!!!Find index using LAST letter, not FIRST letter. Same as letter in CompactNode's Letter.
	int alphaInd = GetAlphabetIndex(posWord[idx-1]);

	idx = CreatePoSTag(alphaInd);
	WLBreakIf(idx < 0,"!!ERROR!! PosTag array is full!!\n");
	mywcscpy(gPoSTag[alphaInd][idx]->posWord, posWord);
	int len = mywcslen(phrase);
	int startPos =0;
	for(int k=len-1; k >=0; k--)
	{
		if(phrase[k] == SP)
		{
			startPos = k+1;
			break;
		}
	}

	int nPos =0;
	MYWCHAR tagName[MAX_WORD_LEN];
	memset(tagName, 0 , sizeof(tagName));
	ePOSTAG eTag = ePOS_NOTAG;
	for(int k = startPos; k < len; k++)
	{
		if(isPunctuation(phrase[k]))
		{
			SetPoSTagValue(tagName, alphaInd, idx);
			nPos =0;
			memset(tagName, 0 , sizeof(tagName));
			continue;
		}
		
		tagName[nPos] = phrase[k];
		nPos++;
	}

	SetPoSTagValue(tagName, alphaInd, idx);

	return true;
}
/////////////////////////////////////////////////////////////////////////////////////
void CDictionaryTree::SetPoSTagValue(MYWCHAR* tagName, int alphaInd, int index)
{
	WLBreakIf(gPoSTag[alphaInd][index] == NULL,"!!ERROR!! SetPoSTagValue array is NULL!!\n");
	for(int p=0; p < MAX_POSTAG_NAME_SIZE; p++)
	{
		if(gPoSTag[alphaInd][index]->ePosTag[p] == ePOS_NOTAG)
		{
			gPoSTag[alphaInd][index]->ePosTag[p] = ConvertToPoSTagName(tagName); 
			WLBreakIf(gPoSTag[alphaInd][index]->ePosTag[p] == ePOS_NOTAG, "!!ERROR! NO Tag assigned for Tag %s and Word %s!!!\n",toA(tagName), toA(gPoSTag[alphaInd][index]->posWord));
			break;
		}
	}
}
/////////////////////////////////////////////////////////////////////////////////////
ePOSTAG	CDictionaryTree::ConvertToPoSTagName(MYWCHAR* tagName)
{
	MakeLowerCase(tagName);

	if(mywcscmp(tagName, L"ab") == 0) 
		return ePOS_AB;
	else if(mywcscmp(tagName, L"abo") == 0 || mywcscmp(tagName, L"apo") == 0) 
		return ePOS_ABO;
	else if(mywcscmp(tagName, L"abs") == 0)
		return ePOS_ABS;
	else if(mywcscmp(tagName, L"adj") == 0 || mywcscmp(tagName, L"avb") == 0 || mywcscmp(tagName, L"ad") == 0 || mywcscmp(tagName, L"cs") == 0)
		return ePOS_ADJ;
	else if(mywcscmp(tagName, L"adr") == 0)
		return ePOS_ADR;
	else if(mywcscmp(tagName, L"adt") == 0)
		return ePOS_ADT;
	else if(mywcscmp(tagName, L"adv") == 0)
		return ePOS_ADV;
	else if(mywcscmp(tagName, L"ajv") == 0)
		return ePOS_AJV;
	else if(mywcscmp(tagName, L"ap") == 0)
		return ePOS_AP;
	else if(mywcscmp(tagName, L"an") == 0)
		return ePOS_AN;
	else if(mywcscmp(tagName, L"cf") == 0)
		return ePOS_CF;
	else if(mywcscmp(tagName, L"cnt") == 0 || mywcscmp(tagName, L"ctn") == 0 || mywcscmp(tagName, L"ctr") == 0)
		return ePOS_CNT;
	else if(mywcscmp(tagName, L"con") == 0)
		return ePOS_CON;
	else if(mywcscmp(tagName, L"cv") == 0)
		return ePOS_CV;
	else if(mywcscmp(tagName, L"int") == 0)
		return ePOS_INT;
	else if(mywcscmp(tagName, L"no") == 0)
		return ePOS_NO;
	else if(mywcscmp(tagName, L"nc") == 0)
		return ePOS_NC;
	else if(mywcscmp(tagName, L"nm") == 0)
		return ePOS_NM;
	else if(mywcscmp(tagName, L"nmo") == 0)
		return ePOS_NMO;
	else if(mywcscmp(tagName, L"npo") == 0)
		return ePOS_NPO;
	else if(mywcscmp(tagName, L"nvi") == 0)
		return ePOS_NVI;
	else if(mywcscmp(tagName, L"nps") == 0)
		return ePOS_NPS;
	else if(mywcscmp(tagName, L"ns") == 0 || mywcscmp(tagName, L"ncs") == 0)
		return ePOS_NS;
	else if(mywcscmp(tagName, L"pp") == 0)
		return ePOS_PP;
	else if(mywcscmp(tagName, L"pn") == 0 || mywcscmp(tagName, L"pr") == 0)
		return ePOS_PN;
	else if(mywcscmp(tagName, L"ppt") == 0)
		return ePOS_PPT;
	else if(mywcscmp(tagName, L"pat") == 0)
		return ePOS_PAT;
	else if(mywcscmp(tagName, L"pre") == 0)
		return ePOS_PRE;
	else if(mywcscmp(tagName, L"sym") == 0)
		return ePOS_SYM;
	else if(mywcscmp(tagName, L"va") == 0)
		return ePOS_VA;
	else if(mywcscmp(tagName, L"vas") == 0)
		return ePOS_VAS;
	else if(mywcscmp(tagName, L"vi") == 0)
		return ePOS_VI;
	else if(mywcscmp(tagName, L"vip") == 0)
		return ePOS_VIP;
	else if(mywcscmp(tagName, L"vis") == 0)
		return ePOS_VIS;
	else if(mywcscmp(tagName, L"vt") == 0)
		return ePOS_VT;
	else if(mywcscmp(tagName, L"vts") == 0)
		return ePOS_VTS;
	else if(mywcscmp(tagName, L"vtss") == 0)
		return ePOS_VTSS;
	else if(mywcscmp(tagName, L"vtp") == 0)
		return ePOS_VTP;
	else if(mywcscmp(tagName, L"art") == 0)
		return ePOS_ART;
	else if(mywcscmp(tagName, L"np") == 0)
		return ePOS_NP;
	else if(mywcscmp(tagName, L"pno") == 0)
		return ePOS_PNO;
	else if(mywcscmp(tagName, L"nw") == 0)
		return ePOS_NW;
	else if(mywcscmp(tagName, L"nn") == 0)
		return ePOS_NN;

	return ePOS_NOTAG;
}
/////////////////////////////////////////////////////////////////////////////////////
ePOSTAG* CDictionaryTree::GetPoSTag(MYWCHAR* word)
{
	static ePOSTAG eTag[MAX_POSTAG_NAME_SIZE] = { ePOS_NOTAG, ePOS_NOTAG, ePOS_NOTAG, ePOS_NOTAG };
//	for(int i=0; i < MAX_POSTAG_NAME_SIZE; i++)
//	{
//		eTag[i] = ePOS_NOTAG;
//	}
	return eTag;
	int i = 0;
	for(; i < MAX_POSTAG_NAME_SIZE; i++)
	{
		if(FindPosTag(word, eTag, i))
		{
			break;
		}
	}

	return eTag;
}
//////////////////////////////////////////////////////////////////////////////////////////////
bool CDictionaryTree::FindPosTag(MYWCHAR* word, ePOSTAG* eTag, int searchType)
{
	MYWCHAR newWord[MAX_WORD_LEN];
	memset(newWord, 0 ,sizeof(newWord));
	mywcscpy(newWord, word);
	if(searchType == 1)
	{
		newWord[0] = uprCharacter(newWord[0]);
	}
	else if(searchType == 2)
	{
		MakeUpperCase(newWord);
	}
	int n = mywcslen(newWord);
	int alphaIndex = GetAlphabetIndex(newWord[n-1]);
	bool bFound = false;
	for(int i=0; i < MAX_POSTAG_SIZE; i++)
	{
		if(gPoSTag[alphaIndex][i] && mywcscmp(gPoSTag[alphaIndex][i]->posWord, newWord) == 0)
		{	
			for(int k=0; k < MAX_POSTAG_NAME_SIZE; k++)
			{
				eTag[k] = gPoSTag[alphaIndex][i]->ePosTag[k];
			}
			bFound = true;
			break;
		}
	}


	return bFound;
}
/////////////////////////////////////////////////////////////////////////////////////
int findBeginIdx(MYWCHAR *word, int maxlen)
{
	//Minkyu:2013.11.28
	//Commented out the bottome line. It stop adding next nodes if curword is single character such as "I" or"a".

	//if(maxlen <= 0)
	//	return -1;


	int curIdx = maxlen;
	while(curIdx > 0 && word[curIdx] != SP)
		curIdx--;
	if( word[curIdx] == SP)
		curIdx++;
	return curIdx;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CDictionaryTree::storeDictionaryPhrase(CParsedLine *lineParser, int fileId, int storeTypeFlag, DictNode *listSpecificationNode)
{
	MYWCHAR *word = lineParser->m_word;
	// split the word up into multi words if it happens to be a phrase
	// on every occurence of space we add an endpoint and add the word itself
	trimEndingSpaces(word);
	int fullLen = mywcslen(word);
	WLBreakIf(word[fullLen] != NUL, "!!ERROR! storeDictionaryPhrase: word #%s# should end on NUL!\n", toA(word));
	int endIdx = fullLen-1;

	int beginIdx = findBeginIdx(word, endIdx);



	int wordcount = 0;
	int j = 0;
	MYWCHAR wordToBeAdded[MAX_WORD_LEN];
	DictNode *firstWordLastNode = NULL;
	while (beginIdx >=0 && endIdx >=beginIdx)
	{
		memset(wordToBeAdded, 0, sizeof(wordToBeAdded));
		mywcsncpy(wordToBeAdded, &word[beginIdx], endIdx-beginIdx+1);
		firstWordLastNode = storeWord2(wordToBeAdded, lineParser->m_pref, firstWordLastNode, lineParser->m_bCanChunk, lineParser->m_StartPref);
		wordcount++;
		endIdx = beginIdx-2;
		beginIdx = findBeginIdx(word, endIdx);
		if(beginIdx<0)
			break;
	}

	lineParser->m_StartPref = 0;
	return wordcount;
}
//////////////////////////////////////////////////////////////////////////////////////////////
bool CDictionaryTree::containedInNexts(NGramNode *NextWords, DictNode *endNode)
{
	NGramNode *tmpNGram = NextWords;
	while(tmpNGram)
	{
		if(tmpNGram->endNode == endNode)
			return true;
		tmpNGram = tmpNGram->Next;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
DictNode *CDictionaryTree::storeWord2(MYWCHAR *lpWord, DWORD preference, DictNode *firstWordLastNode, bool bCanChunk, USHORT startPref)
{
	DictNode *dictNode = NULL;
	DictNode *prevDictNode = NULL;
	DictNode *firstNode = prevDictNode;
	MYWCHAR *chp = lpWord;
	if(lpWord==NULL || lpWord[0]==NUL)
		return NULL;
	while (*chp)
	{
		dictNode = FindCharInTree(prevDictNode, *chp);
		
		if (dictNode != NULL)
		{
			dictNode->Preference = max(preference, dictNode->Preference);
		}
		else
		{
			dictNode = AddCharInTree(prevDictNode, *chp, preference);
			m_nCharsInDictionaryUsed++;
		}
		dictNode->parent = prevDictNode;
		prevDictNode = dictNode;
		chp++;
	}
	dictNode->EndPreference = max(preference, dictNode->EndPreference); // set the endpoint preference for this word




	dictNode->StartPref = startPref;
	//dictNode->bChunkable = false;

	m_highestOccurence = max(++dictNode->WordOccurence, m_highestOccurence);
	if(dictNode->EndPreference > 0)
		dictNode->EndPointFlag = true;
	else
		dictNode->EndPointFlag = false;

	if(firstWordLastNode && !containedInNexts(dictNode->NextWords, firstWordLastNode))
	{
		dictNode->NextWords = allocNGramNode(firstWordLastNode, preference, dictNode->NextWords); 
	}

	
	
	return dictNode;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CDictionaryTree::addNewWordAsOneUnit(MYWCHAR *word, int pref, CParsedLine *lineParser, int fileId, int storeType, DictNode *listSpecNode)
{
	BOOL bRet = FALSE;
	if (word[0])
	{
		if (lineParser)
		{
			bRet = storeDictionaryWord(word, lineParser->m_pref, lineParser->m_description , lineParser->m_leadChars, 
										listSpecNode, storeType, fileId);
		}
		else
		{
			bRet = storeDictionaryWord(word, pref, L"" , L"", NULL, storeType, fileId);
		}
	}
	return bRet;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// store the words in the following way
// 1. In case of storage on the WORDSIDE or on the OBJECTSIDE, 
//    let the objects point to each other.
// 2. In case of storage on the WORDSIDE as well as the OBJECTSIDE, let the objects
//    point to the otherside.
//    for instance take : bill 1000 spruce 
//     where bill on the wordside points to spruce on the object-side and vice-versa.
//     where spruce on the wordside points to bill on the object-side and vice-versa.
//	   This guarantees an automatic transfer to the object-word where possibly other objects
//     are linked as well.
//   
BOOL CDictionaryTree::storeDictionaryWord(MYWCHAR *lpWord, DWORD dwPreference, MYWCHAR *description, MYWCHAR *leadChars,
											 DictNode *listSpecNode, int storeTypeFlag, int fileId)
{
	DictNode *wordSideWordNode = NULL;
	DictNode *wordSideDescriptionNode = NULL;
	DictNode *objectSideWordNode = NULL;
	DictNode *objectSideDescriptionNode = NULL;

	if (storeTypeFlag & WORDSIDE_DICTIONARY)
	{
		if (listSpecNode || wcslen(description) > 0)
		{
			if (listSpecNode && leadChars && !leadChars[0])
				listSpecNode = NULL;
			wordSideWordNode = storeWord1(lpWord, dwPreference, leadChars, listSpecNode, DoubleChain);

		//	wordSideWordNode->listSpecNode = listSpecNode;
			if (wcslen(description) > 0)
			{
				wordSideDescriptionNode = storeWord1(description, dwPreference / 2, leadChars, listSpecNode, DoubleChain);
			//	wordSideDescriptionNode->listSpecNode = listSpecNode;
			}
		}
		else
		{
			wordSideWordNode = storeWord1(lpWord,dwPreference,leadChars, listSpecNode, SingleChain);
			addFileReference(wordSideWordNode, fileId);
		}
	}
		
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDictionaryTree::addFileReference(DictNode *dictNode, int fileId)
{
	if (dictNode->state == WL_NOT_PERMANENT_DELETED_SESSION)
		dictNode->state = WL_NORMAL;

	if (fileId == 0)
		return;

/*	for (int i = 0 ; i < NFILEREFS; i++)
	{
		if (! dictNode->fileRefs[i]) 
		{
			dictNode->fileRefs[i] = fileId;
			return;
		} 
		else if (dictNode->fileRefs[i] == fileId)
			return;
	}
	*/
}

//////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CDictionaryTree::isWordInTree(MYWCHAR *lpWord)
{
	MYWCHAR *chp = lpWord;
	DictNode *dictNode;
	DictNode *prevDictNode = NULL;

	if ( lpWord==NULL || lpWord[0] == NUL)
		return FALSE;

	while (*chp)
	{
		dictNode = FindCharInTree(prevDictNode, *chp);
		if (dictNode == NULL)
			return FALSE;
		prevDictNode = dictNode;
		chp++;
	}
	if (dictNode && dictNode->EndPointFlag == 1)
		return TRUE;
	return FALSE;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DictNode *CDictionaryTree::storeWord1(MYWCHAR *lpWord, DWORD preference,  MYWCHAR *leadChars, DictNode *listSpecNode, LINKAGE linkage)
{
	DictNode *dictNode = NULL;
	DictNode *prevDictNode = NULL;
	DictNode *firstNode;
	BOOL newWord = FALSE;
	MYWCHAR *leadCharp = leadChars;
	MYWCHAR *chp = lpWord;

//  store the entries under the lead chars not the listname anymore(back to storage under the T from PHONE LIST)
	if (listSpecNode)
		prevDictNode = listSpecNode;
	else
	{
		while (leadCharp && *leadCharp != NUL)
		{
			prevDictNode = FindCharInTree(prevDictNode,*leadCharp);
			assert(prevDictNode);
			leadCharp++;
		}
	}

	firstNode = prevDictNode;

	m_nCharsInDictionaryUsed += wcslen(lpWord);
	while (*chp)
	{
		dictNode = FindCharInTree(prevDictNode, *chp);
		if (dictNode != NULL)
		{
			dictNode->Preference = max(preference, dictNode->Preference);
		}
		else
		{
			dictNode = AddCharInTree(prevDictNode, *chp, preference);
			newWord = TRUE;
		}
		if (linkage == DoubleChain && prevDictNode != firstNode)
			dictNode->parent = prevDictNode;

		prevDictNode = dictNode;
		chp++;
	}

	dictNode->EndPreference = max(preference, dictNode->EndPreference); // set the endpoint preference for this word
	
	m_highestOccurence = max(++dictNode->WordOccurence, m_highestOccurence);

	if(dictNode->EndPreference > 0)
		dictNode->EndPointFlag = true;
	else
		dictNode->EndPointFlag = false;

	//if (listSpecNode)
	//	dictNode->listSpecNode = listSpecNode;

	return dictNode;
}

void CDictionaryTree::linkObject(DictNode *fromObjectNode, DictNode *toObjectNode)
{
	if (fromObjectNode == NULL || toObjectNode == NULL)
		return;

	//if (! FindObjectInTree(fromObjectNode, toObjectNode))
	{
	//	AddObjectInTree(fromObjectNode, toObjectNode);
	}
}

DictNode *CDictionaryTree::storeObject(MYWCHAR *lpWord, DWORD dwPreference,DictNode *listSpecNode)
{
	DictNode *dictNode;

	//dictNode = storeWord(lpWord, dwPreference,OBJECTNAME, NULL, DoubleChain);
	//if (listSpecNode)
	//	dictNode->listSpecNode = listSpecNode;
	return dictNode;
}


DictNode *CDictionaryTree::locateEndOfWordInDictionary(MYWCHAR *word)
{
	return locateEndOfWordInDictionary(m_DictTreeStartNode, word);
}

DictNode *CDictionaryTree::locateEndOfWordInDictionary(DictNode *prevDictNode, MYWCHAR *word)
{
	MYWCHAR *leadCharp = word;

	while (leadCharp && *leadCharp != NUL)
	{
		prevDictNode = FindCharInTree(prevDictNode,*leadCharp);
		if (prevDictNode == NULL)
			return NULL;
		leadCharp++;
	}
	return prevDictNode;
}

void findHighestLowestPrefs(DictNode *startingNode)
{
	// include the endpoint preference as well
	sMaxPref = max(sMaxPref, startingNode->EndPreference);
	if (startingNode->EndPreference>0 && startingNode->EndPreference < sMinPref)
		sMinPref = startingNode->EndPreference;

	for (DictNode *dNode = startingNode->CharacterList; dNode != NULL; dNode = dNode->Next)
	{
		findHighestLowestPrefs(dNode);
		sMaxPref = max(sMaxPref, dNode->Preference);
		if (dNode->Preference>0 && dNode->Preference < sMinPref)
			sMinPref = dNode->Preference;
	}
}

/*static void assignEndPointPreference(DictNode *dictNode, Dictionary *referenceDictionary)
{
	putCharInWordString(dictNode->Character);
	if (dictNode->EndPointFlag)
	{
		dictNode->EndPreference = referenceDictionary->findPreferenceForOccurence(dictNode->WordOccurence);
	}

 	for (DictNode *dNode = dictNode->CharacterList; dNode != NULL; dNode = dNode->Next)
	{
		assignEndPointPreference(dNode, referenceDictionary);
	}
	takeCharOffWordString();
}

void CDictionaryTree::assignSimilarPreferences(Dictionary *referenceDictionary)
{
	for (DictNode *dNode = m_DictTreeStartNode->CharacterList; dNode != NULL; dNode = dNode->Next)
	{
		assignEndPointPreference(dNode, referenceDictionary);
	}
}
*/
void CDictionaryTree::printStatistics()
{
	printf(("Nnodes encountered : %d\n"),m_nNodes);
	printf(("m_nCharsInWordsUsed : %d\n"),m_nCharsInWordsUsed);
	printf(("nWordsUsed : %d\n"),m_nWordsUsed);
	printf(("nWordsIgnored : %d\n"),m_nWordsIgnored);
	printf(("nNewWords : %d\n"),m_nNewWords);
	printf(("nCompactNodes  : %d\n"),m_nCompactNodes);
	printf(("nFollowPtrs  : %d\n"),m_nFollowPtrs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDictionaryTree::generateListOfNewWords(CWordList *wordListObject, DictNode *startNode, int selectionFlag)
{
	DictNode *rootNode = startNode;
	DictNode *dNode;

	if (startNode == NULL)
	{
		countWords(NULL);
		rootNode = m_DictTreeStartNode;
		wordListObject->increaseWordListSize(m_newWords, m_newCharacters);
	}
	else 
	{
		putCharInWordString(rootNode->Character);
	}

	if (rootNode->EndPointFlag && rootNode->state & selectionFlag)
	{
		wordListObject->insertWord(getWordString(), rootNode->WordOccurence, 
								  rootNode->EndPreference, rootNode->state);
	}

	for (dNode = rootNode->CharacterList; dNode != NULL; dNode = dNode->Next)
	{
		generateListOfNewWords(wordListObject, dNode, selectionFlag);
	}

	if (startNode != NULL) 
		takeCharOffWordString();
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDictionaryTree::countWords(DictNode *startNode)
{
	DictNode *rootNode = startNode;
	DictNode *dNode;

	if (startNode == NULL)
	{
		m_newWords = 0;
		m_newCharacters = 0;
		rootNode = m_DictTreeStartNode;
	} 
	else
	{
		putCharInWordString(rootNode->Character);
	}

	if (rootNode->EndPreference>0)
	{
		m_newWords++;
		m_newCharacters += getStringLength() + 1;
	}

	for (dNode = rootNode->CharacterList; dNode != NULL; dNode = dNode->Next)
		countWords(dNode);

	if (startNode != NULL) 
	{
		takeCharOffWordString();
	}
}
//////////////////////////////////////////////////////////////////////////////////
void CDictionaryTree::createFrequencyMap(DictNode *startNode)
{
	DictNode *rootNode = startNode;
	DictNode *dNode;

	if (startNode == NULL)
	{
		m_newWords = 0;
		m_newCharacters = 0;
		rootNode = m_DictTreeStartNode;
	} 
	else 
	{
		putCharInWordString(rootNode->Character);
	}

	if (rootNode->EndPointFlag) 
	{
		// a word or phrase occurred this many times
		if (rootNode->WordOccurence > m_freqMapSize)
			DebugBreak("CDictionaryTree::createFrequencyMap");
		m_freqDistrMap[rootNode->WordOccurence]++;
		m_newWords++;
		m_newCharacters += wcslen(getWordString()) + 1;
	}

	for (dNode = rootNode->CharacterList; dNode != NULL; dNode = dNode->Next)
		createFrequencyMap(dNode);

	if (startNode != NULL) 
	{
		takeCharOffWordString();
	}
}
////////////////////////////////////////////////////////////////////////////////
void CDictionaryTree::determineHighestOccurence(DictNode *startNode)
{
	DictNode *rootNode = startNode;
	DictNode *dNode;

	if (startNode == NULL)
	{
		m_highestOccurence = 0;
		rootNode = m_DictTreeStartNode;
	} 

	if (rootNode->EndPointFlag) 
	{
		// a word or phrase occurred this many times
		if (rootNode->WordOccurence > m_highestOccurence)
			m_highestOccurence = rootNode->WordOccurence;
	}

	for (dNode = rootNode->CharacterList; dNode != NULL; dNode = dNode->Next)
		determineHighestOccurence(dNode);
}
/////////////////////////////////////////////////////////////////////////////
bool CDictionaryTree::compactToFile(char *fullpath)
{
	return m_compactStore->compactToFile(fullpath);
}

///////////////////////////////////////////////////////////////////////////////////////////////
void CDictionaryTree::SetUniqueWordCount(int curIndex)
{
	if(curIndex == 0) //first phrase should have all unique words.
	{
		for(int k=0;k<MAX_TGRAPH_HISTORY_NUM;k++)
		{
			if(gPhraseAr[curIndex].endNodes[k])
				mPhraseHeader->wordCount++;	
		}
		return;
	}

	int count =0;
	for(int i=0; i <MAX_TGRAPH_HISTORY_NUM;i++)
	{
		CompactNode* cmpNode = gPhraseAr[curIndex].endNodes[i];
		bool bExist = false;
		for(int j=0; j < curIndex && cmpNode;j++)
		{
			for(int k=0; k < MAX_TGRAPH_HISTORY_NUM;k++)
			{
				CompactNode* tmpNode = gPhraseAr[j].endNodes[k];
				if(tmpNode == cmpNode)
				{
					bExist = true;
					break;
				}
			}
			if(bExist)
				break;
		}

		if(!bExist && cmpNode)
			count++;
	}
	mPhraseHeader->wordCount+= count;
}

///////////////////////////////////////////////////////////////////////////////////////////////
void CDictionaryTree::FillPhraseHeader(FILE *fp)
{
	mPhraseHeader->eLang = m_language;
	fwrite(mPhraseHeader, sizeof(PhraseHeader), 1, fp);
}

///////////////////////////////////////////////////////////////////////////////////////////////
void CDictionaryTree::NGramToFile(char *fileName, eLanguage elang)
{
	FILE *fp = fopen(fileName, "wb");
	if(fp)
	{
		FillPhraseHeader(fp);
		int count =0;
		for(int i=0; i <MAX_PHRASE_ARRAY;i++)
		{
			if(gPhraseAr[i].endNodes[0] == NULL)
			{
				break;
			}

			fwrite(&gPhraseAr[i], sizeof(NGramMultiNode), 1, fp);
			//ShowInfo("NGRamToFile:%s %s %s\n",toA(gPhraseAr[i].word[0]),toA(gPhraseAr[i].word[1]),toA(gPhraseAr[i].word[2]),toA(gPhraseAr[i].word[3]));
			count++;
		}

		fclose(fp);
	}

	///////////////////////////////////////////////////////////////////
     // just for testing....
	if( !(fp = fopen(fileName,"rb")) )    
	{ 
		return;    
	}

	PhraseHeader header;
	fread(&header,sizeof(PhraseHeader),1,fp);
	int count =0;
	for(int i=0; i < MAX_PHRASE_ARRAY;i++)
	{
		NGramMultiNode stNode;
		fread(&stNode, sizeof(NGramMultiNode), 1, fp); 
		
		if(stNode.endNodes[0] == NULL)
		{
			break;
		}

		count++;
	}

	fclose(fp);
}
/////////////////////////////////////////////////////////////////////////
void CDictionaryTree::setBondingChars(MYWCHAR *bondingChars)
{
	int i=0;
	for ( ; i < MAX_BONDING_CHARS && bondingChars[i]; i++)
		m_bondingChars[i] = bondingChars[i];
	m_nBondingChars = i;
}

void CDictionaryTree::printTop2000Words()
{
#ifdef SORTTREE
	FILE *fp = CreateFile( ("D:\\words2000.txt") , GENERIC_WRITE | GENERIC_READ,
				FILE_SHARE_READ,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	
	if (fp == INVALID_HANDLE_VALUE)
		return;

	for (DictNode *dNode = m_DictTreeStartNode->CharacterList; dNode != NULL; dNode = dNode->Next)
		countDistributionRangeAndUse(dNode);

	// run though the list and calculate the cut-off point for entries
	// delete those entries so that in the next step we only collect words
	// for the entries (read preferences) who make up the top 2000 words
	int total = 0;
	for (NumberNode *np = startOfNumberList.next; np != NULL; np = np->next)
		total += np->nWords;

	NumberNode *nextNp;
	for (NumberNode *np = startOfNumberList.next; np != NULL; np = nextNp)
	{
		if ((total - np->nWords) > 2000)
		{
			nextNp = np->next;
			total -= np->nWords;
			LocalFree(np);
			startOfNumberList.next = nextNp;
		}
		else 
			break;
	}

	for (DictNode *dNode = m_DictTreeStartNode->CharacterList; dNode != NULL; dNode = dNode->Next)
		rememberRestOfWord(dNode);

	printTopWords(fp);
	CloseHandle(fp);
#endif  // SORTTREE
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
int leftShiftWord(MYWCHAR *words)
{
	int i = 0;
	int nWordsLeft = 0; 

	MYWCHAR *sp = wcschr(words, SP);
	if (sp)
	{
		sp++;
		for (; *sp ; sp++,i++)
		{
			if (*sp == SP)
				nWordsLeft++;
			words[i] = *sp;
		}
	}
	words[i] = 0;
	return nWordsLeft;
}
