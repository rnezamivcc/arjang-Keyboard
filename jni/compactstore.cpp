// <copyright file="compactstore.cpp" company="WordLogic Corporation">
// Copyright (c) 2000, 2013 WordLogic Corporation Inc. All Right Reserved, http://www.wordlogic.com/
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
// <summary>This is innermost structure which contains a dictionary info and provides interface to dictionary in runtime, such as accessing it, loading it, ....</summary>

#include "stdafx.h"
#include "compactstore.h"
#include "utility.h"
#include "dictionary.h"
#include "dictmanager.h"
#include "compactdict.h"

#ifdef WL_SDK
#include "wordlist.h"
#include "compactdict.h"
#endif
#include <string.h>
#if defined(DEBUG)
#include "compactdict.h"
#endif

char sBuildTimeStamp[] = { __TIMESTAMP__ };
const int gDictBuildNumber = 0;   // increment this for any new build which has any change in it.
const int gSoftwareVersion = 4;   // icrement this each time header or node structure has changed.

MYWCHAR *gListOfWords = NULL;
int gNumOfWords = 0;

const int sDiCtWrapHeaderSize = sizeof(DictHeader);
const int sTableSlotSize = sizeof(TableSpace);
const int sTableSpacesSize = sizeof(gTableSpaces);
const int nTableSpaces = sTableSpacesSize / sTableSlotSize;
const int MAX_CHUNK_ARR_SIZE	=	14; // REZA: BAD BAD BAD!! this and following 2 arrays must be removed and moved back into everb data in dictionary, not HERE!!

const wchar_t* chunkArray[MAX_CHUNK_ARR_SIZE] = {L"hood", L"s", L"es", L"ed",L"ly", L"in",L"out", L"able", L"ment", L"n't", L"ful", L"ability", L"port", L"ing"};
const wchar_t* chunkArrayForEverb[] = {L"ible", L"ibility"};

extern PreferredEndResult PWEresult;

////////////////////////////////////////////////////////////////////////////////////////////////
CCompactStore::CCompactStore(int dictIdx, eLanguage elang): m_dictIdx(dictIdx), m_eLang(elang)
{
	m_dictHeader = NULL;
	m_buildHeader = NULL;
	m_available = NULL;
	m_start_compact = NULL;
	m_block_compact = NULL;
	mRoot = NULL;
	m_mem_size = 0;		// valid (filled) memory is rounded on 64K
	m_valid_mem_size = 0;// valid (filled) memory in bytes
	m_nDynamicChanges = 0;

#ifdef WL_SDK
	m_dictionaryMap = NULL;
	m_updatedDictionaryMapFlag = FALSE;
	m_bytesPerPreference = 0;
	m_nPreferences = 0;
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////
CCompactStore::~CCompactStore()
{
	releaseCompactStore();
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// For every field the number of repetitions is set
// The first 5 fields are fixed in this order meaning character, code,count , preference, nextCount 
// After that it is pretty much open of what will follow.
//
// For every possible field we need to know how many occurences of that field will occur.
// By default if a field occurs it occurs only once.
// The field containing a follow-up pointer occurs more than once and its number
// is found in the code/count/nextCount.
// The field containing a object pointer occurs nObject count times.
BOOL CCompactStore::initialize(size_t memSize, BOOL createDict) 
{
	m_mem_size = (memSize + (SIZE64KB - 1)) & ~(SIZE64KB-1) + SIZE4KB;	

	m_block_compact = (char *) calloc(1, m_mem_size);
	if (m_block_compact == NULL) 
	{
		ShowInfo("!!!No memory available for size 0x%x", m_mem_size); 
		return FALSE;
	}

 	m_start_compact = (char *) ((uintptr_t) (m_block_compact + (SIZE4KB - 1)) & ~(SIZE4KB - 1));
	ShowInfo("CompactStore:initialize: memsize=0x%x, m_mem_size=%p, m_block=0x%x, m_start=%p\n", memSize, m_mem_size, m_block_compact, m_start_compact);

	*(intptr_t *)m_start_compact = (intptr_t) m_start_compact; // record the address ths chunk was allocated on

	// valid (filled) memory in bytes
	m_valid_mem_size = memSize;

	if (createDict)
		m_available = m_start_compact + sizeof(intptr_t);
	else
		m_available = m_start_compact + memSize;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
void CCompactStore::releaseCompactStore()
{
	free( m_dictHeader);
	m_dictHeader = NULL;
	free(m_buildHeader);
	m_buildHeader = NULL;
	free (m_block_compact);
	m_start_compact = 0;
	m_block_compact = 0;
	//m_memOffset = 0;
	m_available = 0;
	m_mem_size = 0;
	m_valid_mem_size = 0;

#ifdef WL_SDK
	free(gListOfWords);
	gListOfWords = NULL;
#endif

}
///////////////////////////////////////////////////////////////////////////
CompactNode *CCompactStore::getParent(CompactNode *cNode)
{
	int offset = spaceCalc(cNode, PIN_PARENT);
	return (CompactNode *) queryPtrFieldAtOffset((char*)cNode, offset);
}
///////////////////////////////////////////////////////////////////////////
void CCompactStore::setParent(CompactNode *cNode, CompactNode *parent)
{
	updateCompactNodePointer(cNode, parent, PIN_PARENT);
}
///////////////////////////////////////////////////////////////////////////
char *CCompactStore::getParentPointer(CompactNode *cp)
{
	int offset = spaceCalc(cp, PIN_PARENT);
    return (char *)cp + offset;
}

//////////////////////////////////////////////////////////////////////////////////////////
void CCompactStore::fixPointers(long memdiff)
{
	ShowInfo("fixPointers: m_start_compact=%x, *m_start_compact = %x\n", m_start_compact, *(UINT *) m_start_compact);
	*(uintptr_t *) m_start_compact = (uintptr_t) m_start_compact;

	WLBreakIf(PTR_SIZE != 3, "---!!ERROR!!fixPointers: go through time-expensive adjust pointers? PTR_SIZE(%d) > 3 ? \n", PTR_SIZE);
	return;
//	adjustPointers(memdiff, 0);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCompactStore::fillCompactNode(CompactNode *cp, MYWCHAR letter, BYTE pref, BYTE nFollowChars,
						BYTE endPref, BYTE undirCodes, BYTE everbVarIdx, BYTE startPref, ePOSTAG tag1, ePOSTAG tag2)
{
	cp->Letter = letter;
	cp->pref = pref;
	cp->Count =  nFollowChars;
	cp->Code = undirCodes;
	cp->POStag = (BYTE)ePOS_NOTAG;

	WLBreakIf((!(undirCodes & CODE_ENDPOINT) && endPref != 0), "!!ERROR! fillCompactNode: endPref>0 but it is not endpoint!!\n");
	if(endPref > 0)
	{
		setEndPreference(cp, endPref);
		cp->POStag = (BYTE)tag1;
		if(tag2)
			setSecondPOSTag(cp, tag2);
	}

	if (undirCodes & CODE_EVERB_ROOT)
		setEVerbVarIdx(cp, everbVarIdx);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CompactNode *CCompactStore::allocCompactNode(MYWCHAR letter, BYTE pref, int nFollowChars,
								BYTE endPref, bool dynamic, 
								bool eVerbEndingFlag, bool eVerbRootFlag, BYTE everbVarIdx,
								bool suffixFlag, BYTE startPref, ePOSTAG tag1, ePOSTAG tag2) 
{
	BYTE undirCodes = 0;

	if (endPref > 0)
		undirCodes |= CODE_ENDPOINT;
	if(dynamic)
		undirCodes |= CODE_DYNAMIC;
	if (eVerbEndingFlag)
		undirCodes |= CODE_EVERB_ENDING;
	if (eVerbRootFlag)
		undirCodes |= CODE_EVERB_ROOT;
	if (suffixFlag)
		undirCodes |= CODE_SUFFIX;
	if (tag2 != ePOS_NOTAG)
		undirCodes |= CODE_POSEXT;

	int spaceNeeded = newSpaceToReserve(undirCodes, PIN_WHOLE, nFollowChars);

	CompactNode *compactNode = (CompactNode *) m_available;
	m_available += spaceNeeded;
	fillCompactNode(compactNode, letter, pref, nFollowChars,  endPref, undirCodes, everbVarIdx, startPref, tag1, tag2);

	return compactNode;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
int CCompactStore::newSpaceToReserve(int code, int toKnowUpto, int followCnt)
{ 
	int spaceNeeded = sBasicSize;		// WCHAR + preference  + cnt + code + POSTag
	
	for (int i=0; i != toKnowUpto; i++)
	{
		switch (i) 
		{
			case PIN_FOLLOWPTRS:
				spaceNeeded += followCnt * PTR_SIZE;
				break;
			case PIN_PARENT:
				spaceNeeded += PTR_SIZE;
				break;
			case PIN_ENDPOINT:
				if(code & CODE_ENDPOINT)
					spaceNeeded += PREF_SIZE;
				break;
			case PIN_EVERB_VARIDX:
				if(code & CODE_EVERB_ROOT)
					spaceNeeded += EVERB_VARIDX_SIZE;
				break;
			case PIN_POS_EXT:
				if(code & CODE_POSEXT)
					spaceNeeded += POSTAG_SIZE;
				break;
			default:
				WLBreak("!!ERROR!newSpaceToReserve: wrong index value %d!\n", i);
				break;
		}
	}

	return spaceNeeded;
}
/////////////////////////////////////////////////////////////////////////////////////
int CCompactStore::spaceCalc(CompactNode *cp, int toKnowAt)
{
	WLBreakIf(cp == NULL, "!!!ERROR! spaceCalc: Node cp is NULL!!\n");
	return newSpaceToReserve(cp->Code, toKnowAt, cp->Count);
}

/////////////////////////////////////////////////////////////////////////////////////
bool CCompactStore::CheckPtrValueExist(char *base, UINT testval, int count)
{
	for(int i=0; i<count; i++)
	{
		int offset = i * PTR_SIZE;
		UINT val = (UINT) queryValueAtOffset(base, PTR_SIZE, offset);
		if(val == testval)
			return true;
	}
	return false; 
}

/////////////////////////////////////////////////////////////////////////////////////
GENPTR CCompactStore::queryPtrFieldAtOffset(char *base, int offset)
{
	WLBreakIf(base == NULL, "!!!ERROR! queryPtrFieldAtOffset: pointer is NULL!!\n");
	GENPTR ptrVal = (GENPTR) queryValueAtOffset(base, PTR_SIZE, offset);

	ptrVal += (GENPTR) m_start_compact; 
	
	return ptrVal;
}

//////////////////////////////////////////////////////////////////////////////////////
void CCompactStore::setPtrFieldAtOffset(char *base, int offset, size_t val)
{
	WLBreakIf(base == NULL, "!!!ERROR! setPtrFieldAtOffset: base pointer is NULL!!\n");
	size_t newval = val;
	if (val > (size_t)m_start_compact)
	{
		newval -= (size_t)m_start_compact; 
	}
	WLBreakIf((newval>>24) != 0, "!!!ERROR!CCompactStore::setPtrFieldAtOffset: ptr size should be only 3 bytes!\n");
	setValueAtOffset(base, PTR_SIZE, offset,  (UINT)newval);
}

////////////////////////////////////////////////////////////////////////////////////////
/*CompactNode ** CCompactStore::getNextWordsPtrs (CompactNode *cp)
{
	WLBreakIf(cp == NULL, "!!!ERROR! getNextWordsPtrs: Node cp is NULL!!\n");
	int offset = spaceCalc(cp, PIN_NEXTPTRS);
	return (CompactNode **) ((char *) cp + offset);
}
////////////////////////////////////////////////////////////////////////////////////////
BYTE * CCompactStore::getNextWordPrefsPtrs (CompactNode *cp)
{
	WLBreakIf(cp == NULL, "!!!ERROR! getNextWordPrefsPtrs: Node cp is NULL!!\n");
	int offset = spaceCalc(cp, PIN_NEXTPREFS);
	return (BYTE*) ((char *) cp + offset);
}
*/
///////////////////////////////////////////////////////////////////////////////////////
BYTE CCompactStore::getEndPreference(CompactNode *cp)
{
	WLBreakIf(cp == NULL, "!!!ERROR! getEndPreference: Node cp is NULL!!\n");
	int offset = spaceCalc(cp, PIN_ENDPOINT);
	return (BYTE)queryValueAtOffset((char *) cp, PREF_SIZE, offset);
}

//////////////////////////////////////////////////////////////////////////////////////////
void CCompactStore::setEndPreference(CompactNode *cp,  BYTE pref)
{
	int offset = spaceCalc(cp, PIN_ENDPOINT);
	setValueAtOffset((char *)cp, PREF_SIZE, offset, pref);
}
/////////////////////////////////////////////////////////////////////////////////////
ePOSTAG CCompactStore::getSecondPOSTag(CompactNode *cp)
{
	if(cp && cp->Code & CODE_POSEXT)
	{
		int offset = spaceCalc(cp, PIN_POS_EXT);
		return (ePOSTAG)queryValueAtOffset((char *) cp, POSTAG_SIZE, offset);
	}
	ShowInfo("WARNING!! CCompactStore::getSecondPOSTag: either cp(%x) is null or it doesn't have secondary POS tag set!!\n", cp);
	return ePOS_NOTAG;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CCompactStore::setSecondPOSTag(CompactNode *cp, ePOSTAG tag)
{
	WLBreakIf (!(cp->Code & CODE_POSEXT), "!!ERROR! setSecondPOSTag: node does NOT have second POS tag code!?\n");
	int offset = spaceCalc(cp, PIN_POS_EXT);
	setValueAtOffset((char *)cp, POSTAG_SIZE, offset, tag);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
void CCompactStore::updateCompactNodeAtIndx(CompactNode *cp, int index, CompactNode *pNode, int pinPointAt)
{
	int offset = spaceCalc(cp, pinPointAt);
	char *cNodePtrs = (char *)cp + offset;
	setPtrFieldAtOffset(cNodePtrs,  PTR_SIZE * index, (size_t) pNode);
}
/////////////////////////////////////////////////////////////////////////////////////////////////
void CCompactStore::updateCompactNodePointer(CompactNode *cp, CompactNode *pNode, int pinPointAt)
{ 
	int offset = spaceCalc(cp, pinPointAt);
	char *cNodePtrs = (char *) cp + offset;
	setPtrFieldAtOffset(cNodePtrs,  0 ,(size_t) pNode);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CCompactStore::updateCompactNodeValue(CompactNode *cp, int index, int value, int pinPointAt, int sizeOfField)
{
	int offset = spaceCalc(cp, pinPointAt);
	offset = offset + index*sizeOfField;
	setValueAtOffset((char *)cp, sizeOfField, offset, value);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void CCompactStore::setAllocatedFirstNode()
{
	mRoot = (CompactNode *) ((GENPTR) m_start_compact + sizeof(GENPTR)); 
	gFirstNodeOfDicts[m_dictIdx] = mRoot; 
}

/////////////////////////////////////////////////////////////////////////////////////////////////
void CCompactStore::allocateFirstEmptyNode()
{
	allocCompactNode(	NUL, 0, 0,
						0,	 // endpoint preference
						false,     // dynamicflag
						false,false,0, //everb stuff
						false, 0, ePOS_NOTAG);
	m_valid_mem_size = (size_t) m_available - (size_t) m_start_compact ;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CCompactStore::getEVerbVarIdx(CompactNode *cp)
{
	WLBreakIf(!cp, "!!ERROR! getEVerbVarIdx: cp is NULL!!\n");
	WLBreakIf (!(cp->Code & CODE_EVERB_ROOT), "!!ERROR! getEVerbVarIdx: node is not everb root!?\n");
	int offset = spaceCalc(cp, PIN_EVERB_VARIDX);
	return queryValueAtOffset((char *)cp, EVERB_VARIDX_SIZE, offset); 
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CCompactStore::setEVerbVarIdx(CompactNode *cp, BYTE everbVarIdx)
{	
	WLBreakIf(!cp, "!!ERROR! setEVerbVarIdx: cp is NULL!!\n");
	WLBreakIf (!(cp->Code & CODE_EVERB_ROOT), "!!ERROR! setEVerbVarIdx: node is not everb root!?\n");
	int offset = spaceCalc(cp, PIN_EVERB_VARIDX);
	setValueAtOffset((char *)cp, EVERB_VARIDX_SIZE, offset, everbVarIdx);
}
//////////////////////////////////////////////////////////////////////////////////
int CCompactStore::getNumRootChildren()
{
	const CompactNode *root = getAllocatedFirstNode();
	if(root)
		return root->Count;
	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////
CompactNode *CCompactStore::getChildByIndex(int index, CompactNode *node)
{
	WLBreakIf(node && index>=node->Count, "!!ERROR! getChildByIndex: node is null or index>=node->Count!\n");
	node = node? node: mRoot;
	CompactNode **nextPtrs = getFollowPtrs(node);
	return (CompactNode *) queryPtrFieldAtOffset((char*)nextPtrs, PTR_SIZE * index);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
// only called internally, cannot be called from user interface
CompactNode *CCompactStore::nextLetterNode(CompactNode *cnode, MYWCHAR letter)
{
	int cnt = 0;
	if (!cnode || letter == NUL || !(cnt = cnode->Count))
		return NULL;

	WLBreakIf(cnode != getAllocatedFirstNode() && cnode->Letter==NUL, "!!ERROR! CCompactStore::nextLetterNode: node->a is NUL!\n");
	CompactNode **nextPtrs = getFollowPtrs(cnode);
//	ShowInfo("nextLetterNode: node=%x, nexts=%x\n", currentNode, nextPtrs);
	for (int i=0; i < cnt; i++)
	{
		CompactNode *cp = (CompactNode *) queryPtrFieldAtOffset((char*)nextPtrs, PTR_SIZE * i);
	//	ShowInfo(":%x", cp->a);
		if (cp->Letter == letter)
			return cp;
	}
//	ShowInfo("\n");
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////
BOOL CCompactStore::isChunk(CompactNode *cNode)
{
	//Minkyu:2014.04.10
	//Crashed here.
	if(!cNode)
		return FALSE;

	if(cNode->POStag == ePOS_HASHTAG || m_eLang == eLang_KOREAN)
		return FALSE;

	int allCode = cNode->Code;
	return ((allCode & CODE_ENDPOINT) == CODE_ENDPOINT);
}

//////////////////////////////////////////////////////////////////////////
BOOL CCompactStore::HasChunkEndings(MYWCHAR *word)
{
	CompactNode* endnode = retrieveEndNodeForString(word, true);
	if(!isChunk(endnode))
		return false;

	if(HasChunkEndingsForEverbs(endnode))
	{
		return true;
	}

	for(int i=0; i <MAX_CHUNK_ARR_SIZE; i++)
	{
		if(fitInDictionary(chunkArray[i], endnode, true))
			return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////
BOOL CCompactStore::HasChunkEndingsForEverbs(CompactNode *endNode)
{
	if(endNode && isNodeAnEVerbEnding(endNode))
	{
		for(int i=0; i <MAX_CHUNK_ARR_SIZE_EVERB; i++)
		{
			if(fitInDictionary(chunkArrayForEverb[i], endNode, true))
			{
				return true;
			}
		}
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////
CompactNode *CCompactStore::retrieveEndNodeForString(const MYWCHAR *word, bool isEndpoint)
{
	if(isEmptyStr(word) || SP_CR_TAB(word[0]))
		return NULL;

	CompactNode *cNode = gFirstNodeOfDicts[m_dictIdx];
	return fitInDictionary((wchar_t *)word, cNode, isEndpoint);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CompactNode * CCompactStore::fitInDictionary(const wchar_t *word, CompactNode *cNode, bool endpoint)
{
	if (!word)
	{
		return NULL;
	}
	else if(!word[0])
	{
		if(endpoint && !isEndpoint(cNode)) // known in this dictionary as a word or a chunk
		{
			return NULL;
		}
		return cNode;
	}

	CompactNode *nextNode = nextLetterNode(cNode, word[0]);
	if (nextNode != NULL)
	{
		return fitInDictionary(&word[1], nextNode, isEndpoint);
	}

	return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CCompactStore::getInternalPreferredLetters(CompactNode *cp,  MYWCHAR *prefLetters,
											   CompactNode *prefPtrs[], int nLettersToRetrieve, BOOL auLettersOnly)
{
	BYTE mypref[0x100]; // allow space for upper-case letters, lower-case + numbers + some punctuations
	int cnt = cp->Count;

	assert(cnt < 0x100);  // sanity check! 
	if (cnt == 0)
	{
		prefLetters[0] = NUL;
		prefPtrs[0] = NULL;
		return 0;
	}

	memset(mypref, 0, sizeof(mypref));
	CompactNode **pfollows = getFollowPtrs(cp);
	for (int i = 0; i < cnt; i++)
	{
		CompactNode *p = (CompactNode *) queryPtrFieldAtOffset((char*)pfollows, PTR_SIZE * i);
		mypref[i] = p->pref;
		if (auLettersOnly)
		{
			MYWCHAR c = p->Letter;
			if (! isUCSpecialCharacter(c) && ! isLCSpecialCharacter(c))
			  mypref[i] = TERMINATED_PREFERENCE;	// not a special character
		}
	}

	int nFound = 0;
	for (; nFound < cnt && nFound < nLettersToRetrieve; nFound++)
	{
		int highestIdx = 0;
		BYTE highestPref = HIGHESTPREFERENCE;
		for (int i = 0; i < cnt; i++)
		{
			if (mypref[i] == TERMINATED_PREFERENCE)		// already selected and depleted
				continue;

			if (highestPref == HIGHESTPREFERENCE || (mypref[i] > highestPref))
			{
				highestPref = mypref[i];
				highestIdx = i;
			}
		}
		if (highestPref == HIGHESTPREFERENCE) // Couldn't select a character anymore, they have all been depriciated in the search for most preferred words
			break; 

		CompactNode *p = (CompactNode *) queryPtrFieldAtOffset((char*)pfollows, PTR_SIZE * highestIdx);
		prefLetters[nFound] = p->Letter;
		prefPtrs[nFound] = p;
		
		mypref[highestIdx] = TERMINATED_PREFERENCE;
	}
	prefLetters[nFound] = NUL;
	prefPtrs[nFound] = NULL;
	return nFound;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCompactStore::reEvaluateBranches(CompactNode *node, BYTE endPref, CompactNode *nextNode, CompactNode *sNextNode)
{
	if (sNextNode && (nextNode->pref==TERMINATED_PREFERENCE || sNextNode->pref > nextNode->pref))
	{	
	//	ShowInfoIf(max(sNextNode->pref, endPref)==0, "0=%x: %c preference %d--> 0!\n", node, node->a, node->pref);
        setPreference(node, max(sNextNode->pref, endPref));
	}
	else
	{  
		if (nextNode->pref == TERMINATED_PREFERENCE)
		{
		//	ShowInfoIf(endPref==0, "1=%x: %c preference %d--> 0!\n", node, node->a, node->pref);
			setPreference(node, endPref);
		}
		else
		{
		//	ShowInfoIf(max(nextNode->pref, endPref)==0, "2=%x: %c preference %d--> 0!\n", node, node->a, node->pref);
            setPreference(node, max(nextNode->pref, endPref));
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CompactNode **CCompactStore::next2PrefNodes(CompactNode *currentNode)
{
	static  CompactNode *next2LettersPtrs[3];

	next2LettersPtrs[0] = NULL;
	next2LettersPtrs[1] = NULL;
	next2LettersPtrs[2] = NULL;
	get2MostPreferredNodes(currentNode, next2LettersPtrs);
	return next2LettersPtrs;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCompactStore::get2MostPreferredNodes(CompactNode *curNode, CompactNode *prefNodes[])
{
	prefNodes[0] = prefNodes[1] = NULL;
	BYTE mypref = 0;
	BYTE mypref1 = 0;
	WLBreakIf(curNode==NULL || curNode->Letter==NUL, "!!ERROR! get2MostPreferredNodes: either node or its char is NUL!\n");
	int cnt = curNode->Count;
	CompactNode **nextPtrs = getFollowPtrs(curNode);
	for (int i = 0; i < cnt; i++)
	{
		CompactNode *p = (CompactNode *) queryPtrFieldAtOffset((char*)nextPtrs, PTR_SIZE * i);
		if(p->pref == TERMINATED_PREFERENCE)  // this node already processed in this search session
			continue;
		if( p->pref > mypref1)
		{
			if( p->pref > mypref)
			{ 
				prefNodes[1] = prefNodes[0];
				prefNodes[0] = p;
				mypref1 = mypref;
				mypref = p->pref;
			}
			else if(p->pref > mypref1)
			{
				prefNodes[1] = p;
				mypref1 = p->pref;
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////// Start of dynamic support for add and delete info into dictionary //////////////////////////
// Following few functions are for adding or deleting nodes.This buids the basis for dynamic nature of our dictionaries!
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CCompactStore::deleteWord(CompactNode *endnode)
{
	bool ret = false;
	int allCode = endnode->Code;
	if (allCode & CODE_ENDPOINT)
	{
		setEndPreference(endnode, 0);
		endnode->Code = endnode->Code & ~CODE_ENDPOINT;
		// first take care of pref for this node:
		if(endnode->Count == 0)
			endnode->pref = 0;
		else
		{
			CompactNode **followNodes = next2PrefNodes(endnode);
			endnode->pref = followNodes[0]->pref;
		}	
		//now walk back and check parent's status to make sure this word path is not a cul-de-sac for searching!!
		CompactNode *parent = getParent(endnode);
		while(parent)
		{
			CompactNode **followNodes = next2PrefNodes(parent);
			if(!followNodes[0])
				break;

			if(parent->pref == followNodes[0]->pref)
				break;
			else
				parent->pref = followNodes[0]->pref;
			parent = getParent(endnode);
		}
		IncrementDynamicChanges();
		ret = true;
	}
	else 
		ShowInfo("deleteWord: endnode is not an endpoint! do nothing for deleteNode!\n");
#if defined(DEBUG)
	ShowInfo("deleteWord:Number of words after deletion = %d\n",m_compactDict->getNumWords(getAllocatedFirstNode()));
#endif
	return ret;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// adjusts the node pointer in dictionary after a memory move. Adjustment happens by the amount of memdiff and
// from address shiftPoint on  to the end of the dictionary.
void CCompactStore::adjustPointers( INT memdiff, GENPTR shiftPoint)
{
	WLBreakIf(shiftPoint == 0, "!!ERROR! adjustPointers shiftPoint ==0!!\n");
	CompactNode *cp = getAllocatedFirstNode();
	CompactNode **nextPtrs;

	while ((GENPTR) cp < (GENPTR) m_available)
	{   // shifting follow letter nodes pointers:
		if (cp->Count)
		{
			nextPtrs = getFollowPtrs(cp);
			adjustSubset(cp->Count, nextPtrs, memdiff, shiftPoint);
		}

		// shift parent node:
		CompactNode *parent = getParent(cp); 
		if((GENPTR) parent >= shiftPoint)
		{
			char *base = getParentPointer(cp);
			setPtrFieldAtOffset(base, 0, (size_t)parent + memdiff);
		}

		cp = (CompactNode *) ((GENPTR)cp + spaceCalc(cp, PIN_WHOLE));
	}

	// next, adjust compactNode pointers in phrase engine
	gPhraseEngine->adjustCompactNodePointers(memdiff, shiftPoint);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
void CCompactStore::adjustSubset(int cnt, CompactNode **nextPtrs, int memdiff, int shiftPoint)
{
	for (int i = 0 ; i < cnt; i++)
	{
		int offset = i*PTR_SIZE;
		char *p = (char *)queryPtrFieldAtOffset((char*)nextPtrs, offset);
		if ((long) p >= shiftPoint)
			setPtrFieldAtOffset((char*)nextPtrs, offset, (size_t)p + memdiff);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This code adds a new word to dictionary dynamically. It needs to check how much of the node chain
// already exist in the tree, then build the remaining and update all tree in terms of adjusting addresses!
CompactNode *CCompactStore::addWord(MYWCHAR *word, int pref)
{
	int n = mywcslen(word);
	for(int j=0; j < n; j++)
	{
		if(isPunctuation(word[j]))
		{
			word[j] = NUL;
		}
	}

	ShowInfo("CompactStore::addWord: #%s#, pref=%d\n", toA(word), pref);
	CompactNode *cNode = getAllocatedFirstNode();
//#if defined(DEBUG)
//	m_compactDict->printAllWords(getAllocatedFirstNode());
//	ShowInfo("addWord:Number of words Before = %d\n",m_compactDict->getNumWords(getAllocatedFirstNode()));
//#endif
	CompactNode *prevNode = cNode;
	int i = 0;
	int len = mywcslen(word);
	while (word && word[i] != NUL)
	{
		prevNode = cNode;
		cNode = nextLetterNode(prevNode, word[i]);
		if (cNode == NULL)
			break;

		if (cNode->Count > 0 && cNode->pref < pref) 
			cNode->pref = pref;
		i++;
	}

	int numAlreadyExistingNodes = i;
	if (cNode) // in this case we are adding a word which is part of an, already existing in dictionary, bigger word
	{
		WLBreakIf(i!=len, "!!ERROR! CCompactStore::addWord: error in parsing word #%s# at pos %d!\n", toA(word), i);
		addWordEndNode(cNode, pref);
	}
	else // put a new pointer in to the rest of the new word and append the rest of the word as nodes to the end
	{
		int memorylayoutchanged = insertAFollowPtr(prevNode, pref);

		// put the rest of the word in
		int lastindex = len - 1;
		while (word && i <= lastindex)
		{
			cNode = allocCompactNode(word[i], pref, i!=lastindex, 
							(i == lastindex)*pref,	 // endpoint preference
							true,     // dynamicflag
							false, false, 0,
							false, 0, ePOS_NEW_WORD, ePOS_NEW_WORD);
			m_valid_mem_size = (size_t) m_available - (size_t) m_start_compact ;
			setParent(cNode, prevNode);
			prevNode = cNode;
			if (i < lastindex)
				updateCompactNodeAtIndx(cNode, 0, (CompactNode *) m_available, PIN_FOLLOWPTRS);
			i++;
		}

		m_dictHeader->totalNumWords++;
		m_dictHeader->totalNumChars += (len-numAlreadyExistingNodes);
		IncrementDynamicChanges();
		if(memorylayoutchanged)
			fullInitializeWordPaths(true);
	}

//#if defined(DEBUG)
//	m_compactDict->printAllWords(getAllocatedFirstNode());
//	ShowInfo("addWord:Number of words after = %d\n",m_compactDict->getNumWords(getAllocatedFirstNode()));
//#endif

	return cNode;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This function added a new follow char to cNode. For this, we do the following:
// 2) Move the content of dictionary from this node up to the end, by enough space to 
//    make room for update to the current node! This method keeps dictionary memory footprint tight
// returns true if memory footprint actually changes, and false if not.
BOOL CCompactStore::insertAFollowPtr(CompactNode *cNode, int pref)
{
	int nOldFollows = cNode->Count;
	//int nextCnt = cNode->NextCount;
	int nNewFollows = nOldFollows + 1;
	int allCode = cNode->Code;

	CompactNode **oldfollowPtrs = getFollowPtrs(cNode);
	ShowInfoIf(nNewFollows > 0x40, "!!!WARNING! insertAFollowPtr! nFollowChars > 64!!!\n");
	if (nNewFollows > 0x40)
	{
		// we should be able to find an empty pointer spot. a-z,A-Z, 0-9 plus punctuation variations + special char variations
		// maximum mount ups to 256, however very unlikely all of them are used.
		for (int j = 0 ; j < nOldFollows; j++) 
		{
			CompactNode *p = (CompactNode *) queryPtrFieldAtOffset((char*)oldfollowPtrs, PTR_SIZE * j);
			if (p==NULL) // We found an empty pointer, reuse it!
			{
				updateCompactNodeAtIndx(cNode, j, (CompactNode *) m_available, PIN_FOLLOWPTRS);
				return false;
			}
		}
	}
	WLBreakIf(nNewFollows > 0x100, "!!!ERROR! insertAFollowPtr! nFollowChars > 256!!!\n");

	int oldFullSpace = spaceCalc(cNode, PIN_WHOLE);
	int newFullSpace = newSpaceToReserve(allCode, PIN_WHOLE, nNewFollows);
	WLBreakIf((newFullSpace - oldFullSpace)!= PTR_SIZE, "!!ERROR! insertAFollowPtr: size incre %d != 3!??\n", (newFullSpace - oldFullSpace));

	// move the contents away to make space available for the new followNodePtr 	
	int oldEndOfFollowsSpace = spaceCalc(cNode, PIN_PARENT);
	char *source = (char*)cNode + oldEndOfFollowsSpace;
	char *dest   = source + PTR_SIZE;
	size_t lenToMove = (size_t)m_available - (size_t)source;
	assert(lenToMove > 0);
	// first adjust all the pointers for the shift in memory 
	adjustPointers( PTR_SIZE, (long) source);
	// now do the shift in memory
	memmove((void*)dest, (void*)source, lenToMove);

	m_available += PTR_SIZE;
	m_valid_mem_size = (size_t)m_available - (size_t)m_start_compact ;

	// here we set the new follow pointer at the empty slot of PTR_SIZE which we created by above memmove. We assume
	// the new follow node is going to be created at the end of the dictionary at m_available address!
	updateCompactNodeAtIndx(cNode, cNode->Count, (CompactNode *) m_available, PIN_FOLLOWPTRS);
	cNode->Count = nNewFollows;
	cNode->pref = max(cNode->pref, pref);

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////
// inserts n next word ptrs for this node. 
/*BOOL CCompactStore::insertNnextPtrs(CompactNode *node, NextWordInfo *nexts, int n)
{
	if(n==0)
		return false;

	int nextCnt = node->NextCount;
	int allCode = node->Code;
	CompactNode **curNextPtrs = getNextWordsPtrs(node);
	BYTE *nextPrefs = getNextWordPrefsPtrs(node);
	
	//first check if any of the nexts words already exist! if so, just update them:
	CompactNode *cp;
	int numNewNextWords = n;
	for(int j=0; j<n; j++)
	{
		for(int i=0; i<nextCnt; i++)
		{
			cp = (CompactNode *) queryPtrFieldAtOffset((char*)curNextPtrs, PTR_SIZE * i);
			WLBreakIf(!cp, "!!ERROR! insertNnextPtrs: nextword empty!\n");
			//retrieveWordFromLastNode(cp, word);
			if(cp == nexts[j].endNode)
		//	if(mywcscmp(word, nexts[j].text)==0)
			{
				nextPrefs[i] = max(nextPrefs[i], nexts[j].pref);
				nexts[j].pref = 0;
				numNewNextWords--;
				break;
			}
		}
	}
	if(numNewNextWords==0)
		return false;
	//now inserts the remaining new next words pointers. Here's the steps:
	// 1) compute total displacement needed because of new nextnodes and their prefs
	// 2) do the memory shift.
	// 3) fill in the node's next pointer and prefs using the inputs provided in arguments
	// 4) adjust the address for the pointers which will be shifted.
	
	int nNewNexts = nextCnt + numNewNextWords;
	WLBreakIf(nNewNexts >= 0xFF, "!!!ERROR! insertNnextPtrs! nNewNexts > 256!!!\n");
	int oldFullSpace = spaceCalc(node, PIN_WHOLE);
	int oldNextSpace = spaceCalc(node, PIN_NEXTPTRS);
	int newFullSpace = newSpaceToReserve(allCode, PIN_WHOLE, node->Count, nNewNexts);
	int displacement = numNewNextWords*(PTR_SIZE+PREF_SIZE);
	WLBreakIf((newFullSpace - oldFullSpace)!= displacement, "!!ERROR! insertNnextPtrs: size incrementation %d != 3!??\n", (newFullSpace - oldFullSpace));

	// move the contents away to make space available for the new next Ptrs 	
	char *source = (char*)node + oldFullSpace;
	int lenToMove = (DWORD)m_available - (DWORD)source;
	if(lenToMove > 0)
	{
		char *dest   = source + displacement;
	
		// saving off next words preferences
		int oldEndOfNextSpace = spaceCalc(node, PIN_NEXTPREFS);
		BYTE tmpNextPrefs[0xFF];
		if(nextCnt>0)
			memcpy(tmpNextPrefs, (char*)node+oldEndOfNextSpace, nextCnt*PREF_SIZE);

		// saving off everbVarIdx 
		BYTE everbVarIdx = 0xFF; // this means there is no everbVarIdx
		if(isNodeAnEVerbEnding(node))
			everbVarIdx = getEVerbVarIdx(node);

		// adjust all the pointers for the shift in memory 
		adjustPointers(displacement, (INT)source);
		// do the shift in memory
		memmove((void*)dest, (void*)source, lenToMove);
	
		// put back next word prefs
		if(nextCnt>0)
		{
			int NewEndOfNextSpace = oldEndOfNextSpace + numNewNextWords*PTR_SIZE;
			memcpy((char*)node+NewEndOfNextSpace, tmpNextPrefs, nextCnt*PREF_SIZE);
		}
		// put back everbIdx
		if(everbVarIdx != 0xFF)
			setEVerbVarIdx(node, everbVarIdx);
	}
	m_available += displacement;
	m_valid_mem_size = (DWORD)m_available - (DWORD)m_start_compact;

	// fill in the  new next pointers:
	for(int k=0; k<n; k++)
	{
		WLBreakIf(nexts[k].pref==0, "!!ERROR! insertNnextPtrs! nextpref is 0!\n");
		CompactNode *updatedEndNode = nexts[k].endNode;
		if((char*)updatedEndNode >= source)
			updatedEndNode = (CompactNode *)((char*)updatedEndNode + displacement);
		updateCompactNodeAtIndx(node, node->NextCount++, updatedEndNode, PIN_NEXTPTRS);
		updateCompactNodeValue(node, node->NextCount-1, nexts[k].pref, PIN_NEXTPREFS, PREF_SIZE);
	}

#if defined(DEBUG)
	ShowInfo("insertNnextPtrs:Number of words after = %d\n",m_compactDict->getNumWords(getAllocatedFirstNode()));
#endif

	return lenToMove > 0;
}
*/
////////////////////////////////////////////////////////////////////////////////////////////
//adds a word, assuming we have all the node chain existing in dictionary, so just need to 
// update the endpoint preference and propagate the preference up the tree branch.
CompactNode *CCompactStore::addWordEndNode(CompactNode *cNode, int pref)
{
	ShowInfo("CompactStore::addWord: endNode=#%c#, pref=%d\n", cNode->Letter, pref);
	WLBreakIf(!cNode, "!!ERROR! CCompactStore::addWord: endNode is NULL?!\n");
#if defined(DEBUG)
	m_compactDict->printAllWords(getAllocatedFirstNode());
	ShowInfo("addWord_cp:Number of words before = %d\n",m_compactDict->getNumWords(getAllocatedFirstNode()));
#endif

	if(getEndPreference(cNode) >= pref)
		return NULL;
	setEndPreference(cNode, pref);
	cNode->Code = cNode->Code | CODE_ENDPOINT;

	CompactNode *parent = cNode;
	while(parent && parent != gFirstNodeOfDicts[m_dictIdx])
	{
		parent->pref = min(max(parent->pref, pref), (int)MAXIMUM_PREFERENCE);
		parent = getParent(parent);
	}
#if defined(DEBUG)
	m_compactDict->printAllWords(getAllocatedFirstNode());
	ShowInfo("addWordcp:Number of words after = %d\n",m_compactDict->getNumWords(getAllocatedFirstNode()));
#endif
		
	IncrementDynamicChanges();
	return cNode;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
const int gMaxNumRunTimeChanges = 25; // max num of runtime changes before flushing the dictionary to disc.
void CCompactStore::IncrementDynamicChanges()
{
	m_nDynamicChanges++;
	if(m_nDynamicChanges > gMaxNumRunTimeChanges)
	{
		serializeDictHeader();
		compactToFile(createFullPathFileName(Dictionary::GetDictName(m_eLang), LDAT));
		m_nDynamicChanges = 0;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////// End of dynamic support for add and delete info into dictionary //////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
CompactNode *CCompactStore::generateNodeFromOffset(char *nodeOffset)
{
	uintptr_t nodeptr = (uintptr_t)nodeOffset + (uintptr_t)m_start_compact;
	return (CompactNode *) nodeptr;
}
///////////////////////////////////////////////////////////////////////////////////////
int CCompactStore::retrieveWordFromLastNode(CompactNode *cp, MYWCHAR *wordToFill)
{
	if(!cp)
		return 0;

	int maxLen = MAX_WORD_LEN-1; // assumed wordToFill is an array of 64 wchar.
	wordToFill[maxLen--] = NUL;
	wordToFill[maxLen--] = cp->Letter;
	CompactNode *startNode = cp;
	CompactNode *parent = NULL; //m_compactStore->getParent(lastNode);
	while((parent= getParent(startNode)))
	{
		WLBreakIf((size_t)parent < (size_t)getAllocatedFirstNode() || (size_t)parent >= (size_t)m_available,  
			"!!ERROR! CCompactStore::retrieveWordFromLastNode: parent out of rang! %x\n", parent);
		if(parent->Letter == NUL)
			break;
		wordToFill[maxLen--] = parent->Letter;
		startNode = parent;
	}	
	maxLen++;
	int len = MAX_WORD_LEN-maxLen;

	//ShowInfo("retrieveWordFromLastNode: len=%d, nextWord=#%s#\n", len, toA(&wordToFill[maxLen]));
	
	for(int i = 0; i<len; i++)
		wordToFill[i] = wordToFill[maxLen+i];

	return len;
}
/////////////////////////////////////////////////////////////////////////////////////
MYWCHAR *CCompactStore::getWordFromNode(CompactNode *node)
{
	if(node==NULL || (node->Code & CODE_ENDPOINT)==0)
		return NULL;
	static MYWCHAR sRetWord[MAX_WORD_LEN];
	retrieveWordFromLastNode(node, sRetWord);
	return sRetWord;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CCompactStore::retrieveWordFromLastNodeInReverse(CompactNode *cp, MYWCHAR *wordToFill)
{
	int len = 0;
	//int maxLen = MAX_WORD_LEN-1; // assumed wordToFill is an array of 4 x 64 wchar, ready to be filled in reverse!
	*wordToFill = cp->Letter;
	wordToFill--;
	len++;
	CompactNode *startNode = cp;
	CompactNode *parent = NULL;
	while((parent= getParent(startNode)))
	{
		WLBreakIf((unsigned long)parent < (unsigned long)getAllocatedFirstNode() || (unsigned long)parent >= (unsigned long)m_available,
				"!!ERROR! CCompactStore::retrieveWordFromLastNodeInReverse: parent out of rang! %x\n", parent);
		if(parent->Letter == NUL)
			break;
		*wordToFill = parent->Letter;
		wordToFill--;
		startNode = parent;
		len++;
	}	

	//ShowInfo("retrieveWordFromLastNodeInReverse: len=%d, Word=#%s#\n", len, toA(&wordToFill[0]));
	return len;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// look at the dictionary for certain bonding characters
BOOL CCompactStore::isAlwaysBondingLetter(MYWCHAR letter)
{
	for (int i = 0; (i < MAX_BONDING_CHARS && m_dictHeader->bondingChars[i]); i++)
	{
		if (m_dictHeader->bondingChars[i] == letter) 
			return TRUE;
	}
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////
EVerbDefinitionR *CCompactStore::getEVerbs()
{
	return (EVerbDefinitionR *)m_dictHeader->tableSpaces[TBEVERBS].tablePtr;
}
/////////////////////////////////////////////////////////////////////////////////////
LangTreatmentRule *CCompactStore::getTreatmentRules()
{
	return (LangTreatmentRule *) m_dictHeader->tableSpaces[TBRULES].tablePtr;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LangTreatmentRule *CCompactStore::findMatchingTreatmentRule(LangTreatmentRule *treatSpecAr, MYWCHAR letter, int *nCommonCharsP,
															MYWCHAR *word, int flatLen, int lastWordLen)
{
	//ShowInfo(TEXT("findMatchingTreatmentRule: flatUserBuf #%s#, flatLen %d, lastWordLen %d nPath %d nNull %d\n"),
	//		toA(flattenedUserCharsBuf), flatLen, lastWordLen, gWordProp->nPathNodes, gWordProp->nNullMoves);

	int lastWordIdx = flatLen - lastWordLen;
	int curLastwordLen = lastWordLen;
	if (terminatedByOneSpace())
		curLastwordLen--;
	for (int i = 0; treatSpecAr[i].triggerChar; i++)
	{
		LangTreatmentRule *treatSpec = &treatSpecAr[i];
	//	ShowInfo(TEXT("-findMatch treatSpec %d triggerStr %s ReplaceStr %s \n"), i, toA(treatSpec->triggerStr), toA(treatSpec->replacementStr));

		if (treatSpec->triggerChar == letter)
		{
			if (treatSpec->terminatedByOneSP == TRUE && terminatedByOneSpace() == FALSE) 
			{
			//	ShowInfo(TEXT("--findMatch rule %d: failed #%s# #%c# , repl #%s# failed 1 nImpl %d nExpl %d\n"), i, toA(treatSpec->triggerStr),
			//		treatSpec->triggerChar, toA(treatSpec->replacementStr),gWordProp->nImplicitSP, gWordProp->nSP);
				continue;
			}

			int triggerLen = mywcslen(treatSpec->triggerStr);
			if(curLastwordLen != triggerLen)
			{
			//	ShowInfo("--findMatch: rule %d, failed! triggerStr len is different from currentWordLen: %d != %d %s != %s\n", i, curLastwordLen, triggerLen, toA(treatSpec->triggerStr), toA(flattenedUserCharsBuf))
				continue;
			}

			WCHAR lwTriggerStr[8]; // asssuming there are no trigger str larger than 30, i.e triggerLen < 30!
			mywcs2lower(lwTriggerStr, &word[lastWordIdx], curLastwordLen);
			if (treatSpec->wholeWord == TRUE)
			{
			//	ShowInfo(TEXT("-findMatch-mywcsncmp #%s# #%s# %d \n"), toA(&flattenedUserCharsBuf[lastWordIdx]), toA(treatSpec->triggerStr), lastWordLen);
				if (mywcsncmp(lwTriggerStr, treatSpec->triggerStr, curLastwordLen) != 0) 
				{
			//		ShowInfo(TEXT("--findMatch rule %d, #%s# #%c# , repl #%s# failed 3 \n"), i, toA(treatSpec->triggerStr), char2A(treatSpec->triggerChar[0]), toA(treatSpec->replacementStr));
					continue;
				}
			}
			else if (mywcsncmp(&word[flatLen-triggerLen], treatSpec->triggerStr, triggerLen) != 0)
			{
			//	ShowInfo(TEXT("--findMatch rule %d, #%s# #%c# , repl #%s# failed 4 \n"), i, toA(treatSpec->triggerStr), char2A(treatSpec->triggerChar[0]), toA(treatSpec->replacementStr));
					continue;
			}

			if (gWordProp->nNullMoves < treatSpec->minNullMoves || gWordProp->nNullMoves > treatSpec->maxNullMoves)
			{
			//	ShowInfo(TEXT("--findMatch #%s# #%c# , repl #%s# failed 5 "), toA(treatSpec->triggerStr), treatSpec->triggerChar[0], toA(treatSpec->replacementStr));
					continue;
			}

			int k = 0;
			if(treatSpec->replacementStr !=  treatSpec->triggerStr)
			{
				for ( k = 0; k < triggerLen; k++)
				{
					if (treatSpec->replacementStr[k] != treatSpec->triggerStr[k])
						break;
				}
			}
			*nCommonCharsP = k; 
			ShowInfo("--foundMatchRule succes!! #%s# #%c# , repl #%s# nCommon %d \n", toA(treatSpec->triggerStr), treatSpec->triggerChar, toA(treatSpec->replacementStr), *nCommonCharsP);
			return treatSpec;
		}
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// Serialization codes used for loading/saving/baking dictionaries
//////////////////////////////////////////////////////////////////////////////////////////////////////
void deserializeEVerbs(void *eVerbsTablePtr, long maxOffset)
{
	ShowInfo("deserializeEVerb %x , maxOffset %x \n", eVerbsTablePtr, maxOffset);
	EVerbDefinitionR *serializedEVerbs = (EVerbDefinitionR *)eVerbsTablePtr;
	char *serializedMemory = (char *) eVerbsTablePtr;
	for (int i = 0; serializedEVerbs[i].triggers[0] != NUL; i++)
	{
		ShowInfo("--deserializeEVerb processing everb %d triggerChars %s\n", i,  toA(serializedEVerbs[i].triggers));
		long offset = (long) serializedEVerbs[i].variationsOnEVerbs;
		if (offset < (long)maxOffset && offset > 0) 
		{
			setValueAtOffset((char*)&serializedEVerbs[i].variationsOnEVerbs, sizeof(GENPTR), 0, (long) &serializedMemory[offset]);
		}

		MYWCHAR **variations = (MYWCHAR **) queryValueAtOffset((char *) &(serializedEVerbs[i].variationsOnEVerbs), sizeof(int *), 0);
	
		int j = 0;
		while (j < MAX_EVERB_ENDINGS)
		{
			MYWCHAR *variationsj = (MYWCHAR *) queryValueAtOffset((char *) variations, sizeof(int *), j * sizeof(int *));

			if (variationsj == 0)
				break;

			offset = (long) variationsj;

			if (offset < maxOffset && offset > 0) 
			{
				setValueAtOffset((char *) variations, sizeof(int *), j * sizeof(int *), (long) &serializedMemory[offset]);
			}
			j++;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
void serializeEVerbs(void *eVerbsTablePtr, int maxOffset, long memoffset)
{
	ShowInfo("serializeEVerb %x , maxOffset %x \n", eVerbsTablePtr, maxOffset);
	EVerbDefinitionR *serializedEVerbs = (EVerbDefinitionR *)eVerbsTablePtr;
	//char *serializedMemory = (char *) eVerbsTablePtr;
	for (int i = 0; serializedEVerbs[i].triggers[0] != NUL; i++)
	{
		ShowInfo("--serializeEVerb processing everb %d triggerChars %s \n", i, toA(serializedEVerbs[i].triggers));
		long offset = (long) serializedEVerbs[i].variationsOnEVerbs - memoffset;

		MYWCHAR **variations = (MYWCHAR **) queryValueAtOffset((char *) &(serializedEVerbs[i].variationsOnEVerbs), sizeof(int *), 0);
		if (offset < maxOffset && offset > 0) 
		{
			setValueAtOffset((char*)&serializedEVerbs[i].variationsOnEVerbs, sizeof(int *), 0, offset);
		}
	
		int j = 0;
		while (j < MAX_EVERB_ENDINGS)
		{
			MYWCHAR *variationsj = (MYWCHAR *) queryValueAtOffset((char *) variations, sizeof(int *), j * sizeof(int *));

			if (variationsj == 0)
				break;

			offset = (long) variationsj - memoffset;

			if (offset < maxOffset && offset > 0) 
			{
				setValueAtOffset((char *) variations, sizeof(int *), j * sizeof(int *), offset);
			}
			j++;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
void deserializeRules(void *rulesTablePtr, long maxOffset)
{
	ShowInfo("deserializeRules %x , maxOffset %x \n", rulesTablePtr, maxOffset);
	LangTreatmentRule *serializedRules = (LangTreatmentRule *) rulesTablePtr;
	char *serializedMemory = (char *) rulesTablePtr;
	for (int i = 0; serializedRules[i].triggerChar; i++)
	{
		ShowInfo("--processing  rule %d: triggers %x, %x, %x \n", i, serializedRules[i].triggerChar, serializedRules[i].triggerStr, serializedRules[i].replacementStr);
	//	int offset = (int) serializedRules[i].triggerChar;
	//	if (offset < maxOffset && offset > 0)
	//	{
	//		setValueAtOffset((char*)&serializedRules[i].triggerChar, sizeof(int *),0,(int) &serializedMemory[offset]);
	//	}

		long offset = (long) serializedRules[i].triggerStr;
		if (offset < maxOffset && offset > 0) 
		{
			setValueAtOffset((char*)&serializedRules[i].triggerStr, sizeof(long *),0,(long) &serializedMemory[offset]);
		}

		offset = (long) serializedRules[i].replacementStr;
		if (offset < maxOffset && offset > 0)
		{
			setValueAtOffset((char*)&serializedRules[i].replacementStr, sizeof(long *),0,(long) &serializedMemory[offset]);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
void serializeRules(void *rulesTablePtr, int maxOffset, long memoffset)
{
	ShowInfo("serializeRules %x , maxOffset %x \n", rulesTablePtr, maxOffset);
	LangTreatmentRule *serializedRules = (LangTreatmentRule *) rulesTablePtr;
	//char *serializedMemory = (char *) rulesTablePtr;
	for (int i = 0; serializedRules[i].triggerChar; i++)
	{
		ShowInfo("--processing  rule %d: triggers %x, %x, %x \n", i, serializedRules[i].triggerChar, serializedRules[i].triggerStr, serializedRules[i].replacementStr);
		long offset = (long) serializedRules[i].triggerChar - memoffset;
		if (offset < maxOffset && offset > 0)
		{
			setValueAtOffset((char*)&serializedRules[i].triggerChar, sizeof(int *),0, offset);
		}

		offset = (long) serializedRules[i].triggerStr - memoffset;
		if (offset < maxOffset && offset > 0) 
		{
			setValueAtOffset((char*)&serializedRules[i].triggerStr, sizeof(int *),0, offset);
		}

		offset = (long) serializedRules[i].replacementStr - memoffset;
		if (offset < maxOffset && offset > 0)
		{
			setValueAtOffset((char*)&serializedRules[i].replacementStr, sizeof(int *),0,offset);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CCompactStore::setBondingCharsInDictionaryHeader(MYWCHAR *bondingChars)
{
	for ( int i=0; i < MAX_BONDING_CHARS && bondingChars[i]; i++)
		m_dictHeader->bondingChars[i] = bondingChars[i];
}

///////////////////////////////////////////////////////////////////////////////////////////////////
MYWCHAR *CCompactStore::getWordList(int *count, CompactNode *startNode)
{
	static int sWordCounter = 0;
	CompactNode *rootNode = startNode;
	CompactNode *dNode;
	//ShowInfo("getWordList: entered; count=%d, %x\n", *count, startNode);
	if (startNode == NULL)
	{
		sWordCounter = 0;
		int wordcount = m_dictHeader->totalNumWords;
		rootNode = getAllocatedFirstNode();
		if(gListOfWords != NULL) // first delete previously allocated list.
		{
			free(gListOfWords);
			gListOfWords = NULL;
		}
		gNumOfWords = wordcount+5;
		//ShowInfo("--word count= %d, buffersize=%d\n", wordcount, gNumOfWords*MAX_WORD_LEN*sizeof(MYWCHAR));
		gListOfWords = (MYWCHAR*)calloc(1, gNumOfWords*sizeof(MYWCHAR)*MAX_WORD_LEN);
		*count = gNumOfWords;
	}
	else 
		putCharInWordString(rootNode->Letter);
	
	if(getEndPreference(rootNode) > 0)
	{
		//ShowInfo("getWordList: %s, code=%x, %d\n", toA(getWordString()), rootNode->Code, sWordCounter);
		if(rootNode->POStag == ePOS_NEW_WORD)
		{
			mywcsncpy(&gListOfWords[sWordCounter*MAX_WORD_LEN], getWordString(), MAX_WORD_LEN-1);
			sWordCounter++;
		}

	}

	if(sWordCounter > gNumOfWords)
		ShowInfo("--!!warning reached the limit of word count! %d. Ignore the rest!\n", sWordCounter);

	int cnt = rootNode->Count;
	CompactNode **nextPtrs = getFollowPtrs(rootNode);
	for (int i = 0; i < cnt; i++)
	{
		dNode = (CompactNode *) queryPtrFieldAtOffset((char*)nextPtrs, PTR_SIZE * i);
		getWordList(count, dNode);
	}
	if (startNode != NULL)
		takeCharOffWordString();
	*count = sWordCounter;
	return gListOfWords;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
CompactNode** CCompactStore::getTop10WordsStartingWith(MYWCHAR *startPart, int *count)
{
	int i = 0;
	CompactNode *root = getAllocatedFirstNode();
	while(startPart && startPart[i]!= NUL)
	{
		CompactNode *next = nextLetterNode(root, startPart[i]);
		if(!next)
			next = nextLetterNode(root, switchCase(startPart[i]));
		if(!next)
		{
			*count = 0;
			return NULL;
		}
		root = next;
		i++;
	}
	WLBreakIf(i!=mywcslen(startPart), "!!ERROR! getTop10WordsStartingWith! i(%d)!=strlen(%s)!!\n", i, toA(startPart));
	m_compactDict->findTop10WordsInSubtree(root);
	*count = PWEresult.nWordsFound;
	if(PWEresult.nWordsFound > 0)
		return PWEresult.endNodes;
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CCompactStore::fillBuildHeader(eLanguage language, MYWCHAR *bondingChars, int numWords, int numChars)
{
	unsigned totalSize = sDiCtWrapHeaderSize + sTableSpacesSize;

	for (unsigned i = 0; i < nTableSpaces; i++)
	{
		if (gTableSpaces[i].sizeOfEntry) 
		{
			totalSize = alignOnLong(totalSize);
			totalSize += gTableSpaces[i].sizeOfEntry;
		}
	}

	if (! (m_dictHeader = (DictHeader *) calloc(1, totalSize)))
		return FALSE;

	m_dictHeader->sizeTotal = totalSize;
	m_dictHeader->totalNumChars = m_dictHeader->totalNumWords = 0;
	strcpy(m_dictHeader->copyright, gCopyWrite);
	sprintf(m_dictHeader->createdTimeStr, "%s" , sBuildTimeStamp);
	m_dictHeader->buildNumber = gDictBuildNumber;
	m_dictHeader->versionNumber = gSoftwareVersion;
	m_dictHeader->totalNumChars = numChars; //m_nCharsInDictionaryUsed;
	m_dictHeader->totalNumWords = numWords; //m_nWordsUsed;

	m_dictHeader->language = language;
	m_dictHeader->loadAddress = (intptr_t)m_start_compact;
	m_dictHeader->endianNess = testEndianNess();

	unsigned offset = sDiCtWrapHeaderSize + sTableSpacesSize;
//	m_dictHeader->tableSpaces = (TableSpace *) ((char *)m_dictHeader + sDiCtWrapHeaderSize);
//	memcpy(m_dictHeader->tableSpaces,  gTableSpaces, sTableSpacesSize);
//	offset += sTableSpacesSize;

	for (unsigned i = 0; i <nTableSpaces; i++)
	{
		m_dictHeader->tableSpaces[i].sizeOfEntry = gTableSpaces[i].sizeOfEntry;
		if (gTableSpaces[i].sizeOfEntry)
		{
			//offset = alignOnLong(offset);
			WLBreakIf(((long)m_dictHeader + offset)&(int)3 != 0, "!!WARNING! fillBuildHeader! table Offset not aligned on 4!!\n");
			memcpy((char *)m_dictHeader + offset, gTableSpaces[i].tablePtr, gTableSpaces[i].sizeOfEntry);
			m_dictHeader->tableSpaces[i].tablePtr = (char *)m_dictHeader + offset;
			offset += gTableSpaces[i].sizeOfEntry;
		}
	}

	m_dictHeader->sizeTotal = totalSize;

	for ( int i=0; i < MAX_BONDING_CHARS && bondingChars && bondingChars[i]; i++)
		m_dictHeader->bondingChars[i] = bondingChars[i];

	// now save off dictheader to be used for later if need to serialize back to disk:
	assert(!m_buildHeader);
	if (! (m_buildHeader = (DictHeader *) calloc(1, totalSize)))
		return FALSE;
	else
		memcpy(m_buildHeader, m_dictHeader, totalSize);

	return TRUE;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This function is called before writing out dictionary file at runtime for dynamic dictinaries. This updates the
// dictHeader and serializes pointer in there: i.e. changing absolute pointers to relative pointers.
BOOL CCompactStore::serializeDictHeader()
{
	sprintf(m_buildHeader->createdTimeStr, "%s" , sBuildTimeStamp);
	m_buildHeader->totalNumChars = m_dictHeader->totalNumChars;
	m_buildHeader->totalNumWords = m_dictHeader->totalNumWords;

/*
    int totalSize = sDiCtWrapHeaderSize + sTableSpacesSize;
    int offset = (int)m_dictHeader + totalSize;
	if (m_dictHeader->tableSpaces[TBEVERBS].tablePtr)
	{
		TableSpace *tbsp = &m_dictHeader->tableSpaces[TBEVERBS]; 
		ShowInfo("deserializeEVerb called,  m_dictionaryheader %x sizeTotal %d ", m_dictHeader, m_dictHeader->sizeTotal); 
		serializeEVerbs(tbsp->tablePtr, tbsp->sizeOfEntry, offset);
	}

	if (m_dictHeader->tableSpaces[TBRULES].tablePtr)
	{
		TableSpace *tbsp = &m_dictHeader->tableSpaces[TBRULES]; 
		ShowInfo("deserializeRules called,  m_dictionaryheader %x sizeTotal %d \n", m_dictHeader, m_dictHeader->sizeTotal); 
		serializeRules(tbsp->tablePtr, tbsp->sizeOfEntry, offset);
	}
	*/
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
// Saves a runtime dictionary to disk. ///////////////////////////////////////////////////////////
BOOL CCompactStore::compactToFile(char *szFileName)
{
	BOOL   bResult = FALSE;
	if (m_buildHeader && (m_available - m_start_compact) > 0)
	{
		ShowInfo("CCompactStore::compactToFile: file %s \n", szFileName);
		// Write out the dictionary file
		m_buildHeader->totalDataSize = m_available - m_start_compact;
		FILE *fp = fopen(szFileName, "wb");
		if (fp != NULL)
		{
			WriteToFile(fp, (char *) m_buildHeader, m_buildHeader->sizeTotal);
			WriteToFile(fp, m_start_compact, m_buildHeader->totalDataSize);
			
			gPhraseEngine->mHeader.totalDataSize = gPhraseEngine->m_available - gPhraseEngine->m_start_compact;
			WriteToFile(fp, (char*)&gPhraseEngine->mHeader, sizeof(PhraseHeader));
			WriteToFile(fp, gPhraseEngine->m_start_compact, gPhraseEngine->mHeader.totalDataSize);

			fclose(fp);
			bResult = TRUE;
		}
		gFirstNodeOfDicts[m_dictIdx] = (CompactNode *) ((GENPTR) m_start_compact + sizeof(GENPTR)); 

		//char *prefFileName = (char *) malloc((strlen(szFileName) + strlen (".pref") + 1));
		//addOn(szFileName,".pref",	prefFileName);
		//SPF1(TEXT("doDump called from compactToFile %s"), szFileName);
//		doDumpPreferences(FALSE,prefFileName, 1);
	//	free(prefFileName);
	}
	
	return bResult;
}

/////////////////////////////////////////////////////////////////////////////////////
// Loads a dictionary from disk to runtime compact form.
CompactNode *CCompactStore::fileToCompact(char *szFileName, BOOL forceFlag)
{
	DWORD  dwDataSize = 0;
	DWORD  totalSize = 0;
	int offset = 0;
//	m_eLang = findLanguage(szFileName);

	FILE *fp = fopen(szFileName, "rb");
	ShowInfo("fileToCompact: %s\n", szFileName);
	if (fp == NULL) 
	{
		ShowInfo("\n!!!TROUBLE!!!CCompactStore::fileToCompact: Failed to open dictionary file %s\n", szFileName);
		return NULL;
	}

	if (! readFromFile(fp, (char *) &totalSize, sizeof(totalSize))) 
	{
		ShowInfo("\n!!!TROUBLE!!!CCompactStore::fileToCompact: Failed to read totalSize(4Bytes) from dictionary file %s\n", szFileName);
		return NULL;
	}
	
	if (! (m_dictHeader = (DictHeader *) calloc(1, totalSize)))
	{
		ShowInfo("\n!!!TROUBLE!!!CCompactStore::fileToCompact: Failed to allocate %d from heap!\n", totalSize);
		return NULL;
	}

	m_dictHeader->sizeTotal = totalSize;
	int len = totalSize - sizeof(m_dictHeader->sizeTotal);
	if (! readFromFile(fp, ((char *) m_dictHeader) + sizeof(m_dictHeader->sizeTotal), len))
	{
		ShowInfo("\n!!!TROUBLE!!!CCompactStore::fileToCompact: Failed to read %d bytes from dictionary %s!\n", len);
		return NULL;
	}

	// now save off dictheader to be used for later if need to serialize back to disk:
	if (! (m_buildHeader = (DictHeader *) calloc(1, totalSize)))
	{
		ShowInfo("\n!!!TROUBLE!!!CCompactStore::fileToCompact: Failed to allocate %d from heap for buildHeader!\n", totalSize);
		return NULL;
	}
	else
		memcpy(m_buildHeader, m_dictHeader, totalSize);

	dwDataSize = m_buildHeader->totalDataSize;
	mTotalDictionarySize = m_buildHeader->totalDataSize + totalSize;
	ShowInfo("---fileToCompact:datasize = %d, totalsize=%d\n", dwDataSize, mTotalDictionarySize);
	if (!initialize(dwDataSize, FALSE))
	{
		ShowInfo("\n!!!TROUBLE!!!CCompactStore::fileToCompact: Failed to initialize dictionary with DataSize %d!\n", dwDataSize);
		return NULL;
	}

	if (! readFromFile(fp, (char *) m_start_compact, dwDataSize))
	{
		ShowInfo("\n!!!TROUBLE!!!CCompactStore::fileToCompact: Failed to read %d bytes from file !\n", dwDataSize);
		return NULL;
	}

	fclose(fp);
    fp = NULL;

	// do some version checking to make sure loaded dictionary matches this code:
	if(m_dictHeader->versionNumber < gSoftwareVersion)
	{
		ShowInfo("\n!!!TROUBLE!!!CCompactStore::fileToCompact: Dictionary version %d and Code version %d do NOT match!! Ignore this dictionary!\n", m_dictHeader->versionNumber, gSoftwareVersion);
		return NULL;
	}

//	m_dictHeader->tableSpaces = (TableSpace *) ((char *) m_dictHeader + sDiCtWrapHeaderSize);
	offset = sDiCtWrapHeaderSize + sTableSpacesSize;

	for (int i = 0; i < nTableSpaces; i++)
	{
		if (m_dictHeader->tableSpaces[i].sizeOfEntry)
			m_dictHeader->tableSpaces[i].tablePtr = (char *)m_dictHeader + offset;
		offset += m_dictHeader->tableSpaces[i].sizeOfEntry;
		offset = alignOnLong(offset);
	}

	ShowInfo("-- offset= %d , nTAbleSpaces %d \n",offset,	NTABLESPACES);
	if (m_dictHeader->tableSpaces[TBEVERBS].tablePtr)
	{
		TableSpace *tbsp = &m_dictHeader->tableSpaces[TBEVERBS]; 
		ShowInfo("deserializeEVerb called,  m_dictionaryheader %x sizeTotal %d ", m_dictHeader, m_dictHeader->sizeTotal); 
		deserializeEVerbs(tbsp->tablePtr, tbsp->sizeOfEntry);
	}

	if (m_dictHeader->tableSpaces[TBRULES].tablePtr)
	{
		TableSpace *tbsp = &m_dictHeader->tableSpaces[TBRULES]; 
		ShowInfo("deserializeRules called,  m_dictionaryheader %x sizeTotal %d \n", m_dictHeader, m_dictHeader->sizeTotal); 
		deserializeRules(tbsp->tablePtr, tbsp->sizeOfEntry);
	}

	if (* (uintptr_t *) m_start_compact != (uintptr_t) m_start_compact)
	{
		m_dictHeader->loadAddress = (long) m_start_compact;
		fixPointers( (long) m_start_compact - * (long *) m_start_compact);
	}

	setAllocatedFirstNode();
	//m_memOffset = m_start_compact;
	return (CompactNode *) ((GENPTR) m_start_compact + sizeof(GENPTR)); 
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////// END OF SERIALIZATION / DESERIALIZATION CODE /////////////////////////////////////////////////////

#ifdef WL_SDK
#if 0
void CCompactStore::printPreferences(MYWCHAR *afterTheseChars)
{
#if 0 //def _DEBUG
	int i = 0;
	int nLevels = 1;
	ExtPathNode *cNode = gWPathway[gWordProp->nPathNodes].cNode[m_dictIdx];

	ShowInfo((" node %c , pref %d , endpref %d \n"), cNode->a, cNode->pref, (isEndpoint(cNode) ?  getEndPreference(cNode) : 0) );

	while (afterTheseChars && afterTheseChars[i] != MYWCHAR('\0'))
	{
		prevNode = cNode;
		nLevels++;
		cNode = nextLetterNode(cNode, afterTheseChars[i]);
		if (cNode == NULL)
			return;

		ShowInfo((" node %c , pref %d , endpref %d \n"), (cNode->a), cNode->pref, (isEndpoint(cNode) ?  getEndPreference(cNode) : 0) );
		i++;
	}
#endif
}
////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CCompactStore::printProximities(MYWCHAR *afterTheseChars)
{
#if 0 //def _DEBUG
	CompactNode *cNode;
	int i = 0;
	char szBuf[30];
	char szPreBuf[200];
	CompactNode **nextPtrs;

	memset(szPreBuf,0,sizeof(szPreBuf));
	cNode = getCascadingNode(gWpSpace->pathway[gWordProp->nPathNodes].cNode, m_dictIdx);

	while (afterTheseChars && afterTheseChars[i] != MYWCHAR('\0'))
	{
		cNode = nextLetterNode(cNode, afterTheseChars[i]);
		if (cNode == NULL)
			return FALSE;
		ShowInfo(" %c %d(%d)"),cNode->a, cNode->pref, isEndpoint(cNode)? getEndPreference(cNode) : 0 );
		strcat(szPreBuf, szBuf);
		i++;
	}

	int cnt = cNode->Count;
	if (cnt == 0)
		return FALSE;

	nextPtrs = getFollowPtrs(cNode);
	CompactNode *myPointers[27];
	CompactNode *highestNode = (CompactNode *) queryPtrFieldAtOffset((char*)nextPtrs, 0);
	int nHighest = 0;

	for (i = 0; i < cnt; i++)
	{
		CompactNode *p = (CompactNode *) queryPtrFieldAtOffset((char*)nextPtrs, PTR_SIZE * i);
		myPointers[i] = p;
		if (p->pref > highestNode->pref)
			highestNode = p;
	}

	for (i = 0; i < cnt; i++)
	{
		if (myPointers[i]->pref == highestNode->pref)
			nHighest++;
	}

	if (nHighest < 2)
		return FALSE;

	for (i = 0; i < cnt; i++)
	{
		if (myPointers[i]->pref == highestNode->pref)
		{
			cNode = myPointers[i];
			OutputDebugString(szPreBuf);
			ShowInfo(" ## %c %d(%d)\n"), getCharacter(cNode), cNode->pref, isEndpoint(cNode) ? getEndPreference(cNode) : 0 );
		}
	}
#endif
	return TRUE;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
void CCompactStore::addWordsToWordList(CWordListSerializer *wordListObject, CompactNode *startNode)
{
	CompactNode *rootNode = startNode;
	CompactNode *dNode;

	if (startNode == NULL)
		rootNode = getAllocatedFirstNode();
	else 
		putCharInWordString(getCharacter(rootNode));
	
	if (getCode(rootNode) & CODE_ENDPOINT)
	{
		int endPointPref = getEndPreference(rootNode);
		wordListObject->addWord(getWordString(), endPointPref);
	}
	int cnt = getCount(rootNode);
	CompactNode **nextPtrs = getFollowPtrs(rootNode);
	for (int i = 0; i < cnt; i++)
	{
		dNode = (CompactNode *) queryPtrFieldAtOffset((char*)nextPtrs, PTR_SIZE * i);
		addWordsToWordList(wordListObject, dNode);
	}
	if (startNode != NULL)
		takeCharOffWordString();
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCompactStore::addWordsToWordList(CWordList *wordListObject, CompactNode *startNode, int fileRef)
{
	CompactNode *rootNode = startNode;
	CompactNode *dNode;

	if (startNode == NULL)
		rootNode = getAllocatedFirstNode();
	else 
		putCharInWordString(getCharacter(rootNode));

	if (getCode(rootNode) & CODE_ENDPOINT)
	{
		int endPointPref = getEndPreference(rootNode);
		// give occurences and preference the same preference values, just in case we decide to renormalize again
		if (fileRef != -1)
			wordListObject->addWordWithFileRef(getWordString(), endPointPref, fileRef);
		else
			wordListObject->UIaddWord(getWordString(), endPointPref);
	}

	int cnt = getCount(rootNode);

	CompactNode **nextPtrs = getFollowPtrs(rootNode);
	for (int i = 0; i < cnt; i++)
	{
		dNode = (CompactNode *) queryPtrFieldAtOffset((char*)nextPtrs, PTR_SIZE * i);
		if (isNodeDeleted(dNode) == TRUE)
			continue;
		addWordsToWordList(wordListObject, dNode, fileRef);
	}

	if (startNode != NULL)
		takeCharOffWordString();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void CCompactStore::removeWordsFromWordList(CWordList *wordListObject, CompactNode *startNode)
{
	CompactNode *rootNode = startNode;
	CompactNode *dNode;

	if (startNode == NULL)
		rootNode = getAllocatedFirstNode();
	else 
		putCharInWordString(getCharacter(rootNode));

	if (getCode(rootNode) & CODE_ENDPOINT)
	{
		int endPointPref = getEndPreference(rootNode);
		wordListObject->deleteWord(getWordString()); 
	}

	int cnt = getCount(rootNode);

	CompactNode **nextPtrs = getFollowPtrs(rootNode);
	for (int i = 0; i < cnt; i++)
	{
		dNode = (CompactNode *) queryPtrFieldAtOffset((char*)nextPtrs, PTR_SIZE * i);
		if (isNodeDeleted(dNode) == TRUE)
			continue;
		removeWordsFromWordList(wordListObject, dNode);
	}

	if (startNode != NULL) 
		takeCharOffWordString();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CCompactStore::printNodesWithSimilarPrefInfo(CompactNode *rootNode, FILE *fp)
{
#if 0
	DWORD dwWritten = 0;
	CompactNode *dNode = NULL;

	if (rootNode == NULL)
		rootNode = getAllocatedFirstNode();
	else 
		putCharIntoWordString(getCharacter(rootNode));

	if (rootNode->Code & CODE_ENDPOINT)
	{
		MYWCHAR mystring[100];
		int occurence = getOccurenceCountUsingDictionaryMap(rootNode);
		if (occurence)
		{
			int endPointPref = getEndPreference(rootNode);
			swprintf(mystring,100, L"cNode %x  %s pref %d occured %d", rootNode , getWordString(), endPointPref, occurence);

			WriteFile(hFile, (char *) mystring, mywcslen(mystring) * sizeof (WCHAR), &dwWritten, NULL);
			WriteFile(hFile,(LPCVOID) L"\x0d\x0a",4, &dwWritten, 0);
		}
	}

	int cnt = rootNode->Count;

	CompactNode **nextPtrs = getFollowPtrs(rootNode);
	for (int i = 0; i < cnt; i++)
	{
		dNode = (CompactNode *) queryPtrFieldAtOffset((char*)nextPtrs, PTR_SIZE * i);
		if (isNodeDeleted(dNode) == TRUE)
			continue;
		printNodesWithSimilarPrefInfo(dNode, hFile);
	}

	if (rootNode != NULL) 
		takeCharOffWordString();
#endif
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CCompactStore::doDumpPreferences(BOOL mapFlag, TCHAR *szPrefFileName, int bytesPerPreference)
{
	CompactNode *cp;
	CompactNode *nextcp;
	int cnt = 0;
	int allCode = 0;
	int nPreferences = 0;	
	BYTE *filledExtractp;
	BOOL   bResult = TRUE;

	if (m_generationOfPreferenceFileDisabled == TRUE)
		return  TRUE;

	SPF2(TEXT("doDump freeing up m_dictionaryMap %x m_preferenceExtract %x"), m_dictionaryMap, m_preferenceExtract);

	if (m_dictionaryMap && m_preferenceExtract)
	{
		SPF2(TEXT("doDump freeing up m_dictionaryMap %x m_preferenceExtract %x"), m_dictionaryMap, m_preferenceExtract);
		free( m_preferenceExtract);
		m_preferenceExtract = NULL;
		VirtualFree(m_dictionaryMap,0,MEM_RELEASE);
		m_dictionaryMap = NULL;
		return TRUE;
	}

		// the mapFlag and szPrefFileName are mutuallly exclusive !!!!!
	if ((mapFlag == TRUE && szPrefFileName) || (mapFlag == FALSE && ! szPrefFileName))
		return FALSE;

	if (mapFlag)
	{
		// allocate same size as dictionary to hold the mapping pointers
		m_dictionaryMap = (char *) VirtualAlloc((void *)0x0,m_mem_size,MEM_COMMIT,PAGE_READWRITE);
		if (! m_dictionaryMap)
			return FALSE;
	}

	if (m_start_compact == NULL)
		return TRUE;

	// first count the number of nodes
	cp = (CompactNode *) (m_start_compact + sizeof(UINT));
	SPF2(TEXT("doDump dumping from %x to %x "), cp, m_available);
	
	while ((UINT) cp < (UINT) m_available)
	{
		cnt = (cp->Count);
		allCode = (cp->Code);
		nPreferences++;
		if (allCode & CODE_ENDPOINT && cnt > 0)
			nPreferences++;
		nextcp = (CompactNode *) ((UINT) cp + spaceCalc(cp, PIN_WHOLE));
		cp = nextcp;
	}

	// allocate memory to hold the preferences and endpoint preferences
	m_preferenceExtract = (char *) malloc( nPreferences * bytesPerPreference);
	if (! m_preferenceExtract)
	{
		VirtualFree(m_dictionaryMap,0,MEM_RELEASE);
		m_dictionaryMap = NULL;
		return FALSE;
	}
	m_bytesPerPreference = bytesPerPreference;
	filledExtractp = (BYTE *) m_preferenceExtract;
	m_nPreferences = nPreferences;

	// now run through all the dictionary, dump preference and optional endpoint preference .
	// while processing build the m_dictionaryMap if it is required
	cp = (CompactNode *) (m_start_compact + sizeof(GENPTR));
	int n = 0;
	while ((UINT) cp < (UINT) m_available)
	{
		if (cp == (CompactNode *) (m_start_compact + sizeof(GENPTR)))
		{
			SPF7(TEXT("doDump filling m_preferenceExtract %x  (%d %d ) dictmap %x filename %s bytesperPref %d map flag %d"), 
				nPreferences , bytesPerPreference,
				m_dictionaryMap, szPrefFileName ? szPrefFileName : TEXT(""),bytesPerPreference , mapFlag);
		}

		cnt = getCount(cp);
		allCode = getCode(cp);
		nextcp = (CompactNode *) ((UINT) cp + spaceCalc(cp, PIN_WHOLE));

		// fill the map such that for this cp we know where the preference and endpointpref are located in the preference extraction
		if (mapFlag)
		{
			GENPTR p = (GENPTR) cp - (GENPTR) m_start_compact + (GENPTR) m_dictionaryMap;
			setValueAtOffset((char *) p, sizeof(UINT *), 0, (int) filledExtractp);
		}
		
		n++;

		*filledExtractp = cp->pref;
		filledExtractp += m_bytesPerPreference;

		if (allCode & CODE_ENDPOINT && cnt > 0) 
		{
			*filledExtractp = getEndPreference(cp);
			filledExtractp += m_bytesPerPreference;
		}
		cp = nextcp;
	}
	if ((n % 100) == 0)
	{
		SPF1(TEXT("doDump finished filling %x "), filledExtractp);
	}

	if (szPrefFileName && m_start_compact)
	{
		FILE *fp = fopen(szFileName, "rb");
		// Create a preference file, where the learned user preferences can be dumped in      
		if (fp == NULL)
		{
			free( m_preferenceExtract);
			m_preferenceExtract = NULL;
			m_bytesPerPreference = 0;
			return FALSE;
		}
		WriteToFile(fp, m_preferenceExtract,(UINT) filledExtractp - (UINT) m_preferenceExtract);
		fclose(fp);
		free(m_preferenceExtract);
		m_preferenceExtract = NULL;
		m_bytesPerPreference = 0;
		m_nPreferences = 0;
		return bResult;
	}
	// if file not specified then m_preferenceExtract and m_dictionaryMap are filled 
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CCompactStore::doLoadPreferences(char *szPrefFileName, int bytesPerPreference)
{
	CompactNode *cp;
	CompactNode *nextcp;
	int cnt = 0;
	int allCode = 0;
	BYTE endpointPref;
	BYTE *filledExtractp;
	FILE *fp = NULL;

	SPF1(TEXT("doLoadPreferences entered for %s"), szPrefFileName);

	// Open the preferences file     
	fp = fopen(szPrefFileName, "r+");
	if (fp==NULL) 
	{
		// file is not there, just ignore it
		return TRUE;
	}

	DWORD dwFileSize = getFileSize(fp);
	if (dwFileSize <= 0)
	{
		// the pref file is empty, one will be created if we unload the dictionary
		fclose(fp);
		return TRUE;
	}

	// allocate memory to hold the preferences and endpoint preferences
	m_preferenceExtract = (char *) malloc( dwFileSize);
	if (! m_preferenceExtract)
	{
		CloseHandle( hFile);
		ShowInfo(TEXT("Can't allocate memory to load preferences ")); 
		return FALSE;
	}

	m_bytesPerPreference = bytesPerPreference;
	m_nPreferences = dwFileSize / bytesPerPreference;

	filledExtractp = (BYTE *) m_preferenceExtract;

	if (! readFromFile(fp,(char *) m_preferenceExtract, dwFileSize)) 
	{
		free(m_preferenceExtract);
		CloseHandle(hFile);
		ShowInfo(TEXT("Trouble reading preferences from file")); 
		return FALSE;
	}

	void *id164bitp = (void *) ((char *) m_preferenceExtract + dwFileSize - sizeof(m_dictHeader->id164bit));
	if (memcmp((void *) m_dictHeader->id164bit, id164bitp, sizeof(m_dictHeader->id164bit)))
	{
		free(m_preferenceExtract);
		CloseHandle(hFile);
		DeleteFile(szPrefFileName);
		// apparently a new dictionary, the pref file should have been gone during installation
		ShowInfo(TEXT("Preference file doens't belong to dictionary")); 
		return TRUE;
	}

	// now run through all the dictionary, load preference and 
	// optional the endpoint preference .
	cp = (CompactNode *) (m_start_compact + sizeof(GENPTR));
	while ((GENPTR) cp < (GENPTR) m_available)
	{
		cnt = getCount(cp);
		allCode = getCode(cp);
		nextcp = (CompactNode *) ((GENPTR) cp + spaceCalc(cp, PIN_WHOLE));

		cp->pref = *filledExtractp;
		filledExtractp += m_bytesPerPreference;

		if (allCode & CODE_ENDPOINT && cnt > 0) 
		{
			endpointPref = *filledExtractp;
			setEndPreference(cp, endpointPref);
			filledExtractp += m_bytesPerPreference;
		}
		cp = nextcp;
	}

	free(m_preferenceExtract);
	m_preferenceExtract = NULL;
	m_bytesPerPreference = 0;
	m_nPreferences = 0;
	CloseHandle(hFile);
	return TRUE;
}
#endif

#endif // WL_SDK

