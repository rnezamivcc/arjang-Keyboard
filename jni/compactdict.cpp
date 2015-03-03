// <copyright file="compactdict.cpp" company="WordLogic">
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
// <summary>Provides upper level interface to a dictionary at runtime. It provides interface between compactstore and dictionary class.</summary>

#include "stdafx.h"
#include "compactdict.h"
#include "wordpunct.h"
#include "wordpathext.h"
#include "compatibility.h"
#include "compactstore.h"
#include "searchResults.h"
#include "wordpath.h"

BYTE makeFlags(BYTE layer, BYTE flavor) { return (layer & 0x0f) | (flavor << 4);}

CCompactDict::CCompactDict(CCompactStore *compactStore, int dictIdx, eLanguage elang): m_compactStore(compactStore), m_dictIdx(dictIdx), m_eLang(elang)
{
	memset(m_assembledWord, 0, sizeof(m_assembledWord));
	m_assembledWordPtr = &m_assembledWord[0];
}

// take the everb endings out, by terminating the nodes just below the header
/////////////////////////////////////////////////////////////////////////////////////////////
void CCompactDict::takeOutTheEVerb(int nPath, BYTE *pref)
{
	ExtPathNode *extPathNode = gWPathway[nPath].dictNode[m_dictIdx];
	if (extPathNode == NULL)
		return;

	ExtCNode *extCNodes = extPathNode->extCNodes;

	// for now pinpoint the Everb and Terminate this, in future usee the breadcrumcache, 
	// revaluate branches as the everb root and base might be more than one node apart !!
	for (int i = 0; i < extPathNode->nExtCNodes && i < NEVERBCASES; i++)
	{
		pref[i] = extCNodes[i].cNode->pref;
		m_compactStore->setPreference(extCNodes[i].cNode, TERMINATED_PREFERENCE);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
void CCompactDict::putTheEVerbBackIn(int nPathNodes, BYTE *originalPref)
{
	ExtPathNode *extPathNode = gWPathway[nPathNodes].dictNode[m_dictIdx];
	if (extPathNode == NULL)
		return;

	ExtCNode *extCNodes = extPathNode->extCNodes;
	
	// only process this for one item for now !!!!!!, for now pinpoint the Everb and Terminate this
	// in future usee the breadcrumcache , revaluate branches as the everb root and base might be more than one node apart !!
	for (int i = 0; i < extPathNode->nExtCNodes && i < NEVERBCASES; i++)
		m_compactStore->setPreference(extCNodes[i].cNode, originalPref[i]);
}
////////////////////////////////////////////////////////////////////////////////
// this is the node we are sitting on, we have to fill the next one
////////////////////////////////////////////////////////////////////////////////
BOOL CCompactDict::advancePathWithLetter(MYWCHAR letter, int nPath)
{
	bool ret = false;
	ExtPathNode *extPathNode = gWPathway[nPath].dictNode[m_dictIdx]; 
	WLBreakIf(extPathNode == EXT_NODE_NOTSET, "!!ERROR!advancePathWithLetter: dictNode is not set yet!\n");
	ExtCNode *extCNodes = extPathNode->extCNodes;
	BOOL isNextNodeEndNode = false;
	for (BYTE k = 0; k < extPathNode->nExtCNodes; k++)
		ret |= fillNextPathNodeBC(nPath+1, &extCNodes[k], letter,extCNodes[k].layer(), &isNextNodeEndNode);

	return ret;
}
//////////////////////////////////////////////////////////////////////////////
// looking at end node of current working path, returns the preference of a given letter as next letter. 
// If it doesn't exist, returns 0.
//Notes: Only base layer (0) is considered.
BYTE CCompactDict::getLetterPref(MYWCHAR letter)
{
	BYTE ret = 0;
	ExtPathNode *extPathNode = gWPathway[gWordProp->nPathNodes].dictNode[m_dictIdx]; 
	if(extPathNode == EXT_NODE_NOTSET)
		return 0;

	ExtCNode *extCNodes = extPathNode->extCNodes;
//	for (BYTE k = 0; k < extPathNode->nExtCNodes; k++)
	{
		CompactNode *node = m_compactStore->nextLetterNode(extCNodes[0].cNode, letter);
		ret = node ? node->pref : 0;
	}
	return ret;
}
//////////////////////////////////////////////////////////////////////////////
// this is the node we are sitting on, we have to fill the other one
//////////////////////////////////////////////////////////////////////////////
BOOL CCompactDict::fillNextPathNode(MYWCHAR letter, int nPath, USHORT *layers, USHORT *layerEnds)
{
	BOOL ret = false;
	ExtPathNode *extPathNode = gWPathway[nPath].dictNode[m_dictIdx];
	ShowInfoIf(extPathNode == EXT_NODE_NOTSET, "!!WARNING!CCompactDict::fillNextPathNode: path at position %d not set yet!?\n", nPath);
	if(extPathNode != EXT_NODE_NOTSET)
	{
		ExtCNode *extCNodes = extPathNode->extCNodes;
		for (int k = 0; k < extPathNode->nExtCNodes; k++)
		{
			BOOL isNextNodeEndNode = false;
			BYTE layer = extCNodes[k].layer();
			if(fillNextPathNodeBC(nPath+1, &extCNodes[k], letter, k, &isNextNodeEndNode))
			{
				turnOnBit16(layers, layer);
				if(isNextNodeEndNode)
					turnOnBit16(layerEnds, layer);
				ret = true;
			}
		}
	}
	return ret;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CCompactDict::fillNextPathNodeBC(	int nNextPath, ExtCNode *extcNode, MYWCHAR letter, BYTE prev, BOOL *isEndNode)
{
	CompactNode **succNodes = nextNodeLetterCases(extcNode->cNode, letter);
	if(succNodes[0])
	{
		extcNode->next1 = topUpPathNode(nNextPath, succNodes[0], prev, extcNode->flags);
		*isEndNode = m_compactStore->isEndpoint(succNodes[0]);
	}
	if(succNodes[1])
	{
		extcNode->next2 = topUpPathNode(nNextPath, succNodes[1], prev, makeFlags(extcNode->layer(),extcNode->flavor()+1));
		*isEndNode = *isEndNode || m_compactStore->isEndpoint(succNodes[1]);
	}
	
	return (succNodes[0]!=NULL);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// This is the main method for growing pathnode of a word or phrase in progress. 
// This either sets the new node in path, or extends existing one!
// Returns location in the node's extended path, i.e. either the default(i.e. no extention) which is 0, or location in the extended path.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CCompactDict::topUpPathNode(int nPath, CompactNode *succNode, BYTE prev, BYTE flags)
{		
	ExtPathNode *extPathNode = gWPathway[nPath].dictNode[m_dictIdx];
	if (extPathNode == EXT_NODE_NOTSET) // node is not initialized yet. Initialze it and return
	{
		convertCNodeToPathNode(&extPathNode, succNode, gWorkingWordIdx, prev, flags);
		gWPathway[nPath].dictNode[m_dictIdx] = extPathNode;
		return 0;
	}

	int nOldEntries = extPathNode->nExtCNodes;
	for (int i = 0; i < nOldEntries; i++) //  make sure the entry doesn't already exist.
	{
		WLBreakIf ((extPathNode->extCNodes[i].cNode == succNode && extPathNode->extCNodes[i].flags == flags), "!!ERROR!topUpPathNode: already extcnode exists in the pathnode!\n")
	}
	
	// now build new extpathnode, copy the old one over, fill up the new ExtCNode, and finally delete the current extpathnode.
	int sizeToAllocate = sizeof(ExtPathNode) + nOldEntries* sizeof(ExtCNode);
	ExtPathNode *newExtPathNode = (ExtPathNode *) getWorkingHeap(gWorkingWordIdx)->allocate(sizeToAllocate);
	memcpy(newExtPathNode, extPathNode, sizeof(ExtPathNode)); // copy the old extPathNode over, extCNodes[0] entry is included !!
	if (nOldEntries > 1) // copy any old but valid extCNodes over, zero entry is already copied, but will be recopies. No harm done!
		memcpy(newExtPathNode->extCNodes, extPathNode->extCNodes, nOldEntries*sizeof(ExtCNode));
	
	// fill the new node 	
	convertCNodeIntoExtCNode(&newExtPathNode->extCNodes[nOldEntries], succNode, prev, flags);
	newExtPathNode->nExtCNodes++;
	WLBreakIf(!(BelongToWorkingHeap(gWorkingWordIdx, (char*)gWPathway[nPath].dictNode[m_dictIdx])), 
			"!!ERROR!  topUpPathNode: wrong memory heap been selected: gWorkingWordIdx=%d\n", gWorkingWordIdx);
	gWPathway[nPath].dictNode[m_dictIdx] = newExtPathNode;
	gWordProp->maxNumExtNodes = max(gWordProp->maxNumExtNodes, newExtPathNode->nExtCNodes);
	return nOldEntries;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// given a letter returns list of next 2 most probable compactnodes in this dictionary coming after
// the current given compactnode. Note: We may need to extend this to returning 3 nodes for languages
// with lots of possible flavors for each letter.
////////////////////////////////////////////////////////////////////////////////////////////////////
CompactNode **CCompactDict::nextNodeLetterCases(CompactNode *cNode, MYWCHAR letter)
{
	static CompactNode *nextNodes[2];
	nextNodes[0]=nextNodes[1]=NULL; 
	MYWCHAR *trychars = GetCharCases(letter, m_eLang);
	BYTE prefs[2] = {0,0};
	for (int i = 0; trychars[i]; i++)
	{
		CompactNode *next = NULL;

		if(i<2 && trychars[i] != letter && gWordProp->nPathNodes>0 )
			continue;
		else
			next = m_compactStore->nextLetterNode(cNode, trychars[i]);

		if(next && next->pref > 0)
		{
			if(next->pref >= prefs[0])
			{
				nextNodes[1] = nextNodes[0];
				nextNodes[0] = next;
				prefs[1] = prefs[0];
				prefs[0] = next->pref;
			}
			else if(next->pref >= prefs[1])
			{
				nextNodes[1] = next;
				prefs[1] = next->pref;
			}
		}
	}

	return nextNodes;
}

/////////////////////////////////////////////////////////////////////////////////////
BOOL CCompactDict::isChunk(int node)
{
	ExtPathNode *extPathNode = gWPathway[node].dictNode[m_dictIdx]; 
	if (extPathNode == EXT_NODE_NOTSET)
		return FALSE;

	ExtCNode *extCNodes = extPathNode->extCNodes;
	for (int k = 0; k < extPathNode->nExtCNodes; k++)
	{
		if (m_compactStore->isChunk(extCNodes[k].cNode))			
			return TRUE;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////
BOOL CCompactDict::isEndpoint(int node)
{
	ExtPathNode *extPathNode = gWPathway[node].dictNode[m_dictIdx]; 
	if (extPathNode == EXT_NODE_NOTSET)
		return FALSE;

	ExtCNode *extCNodes = extPathNode->extCNodes;
	for (int k = 0; k < extPathNode->nExtCNodes; k++)
	{
		if (m_compactStore->isEndpoint(extCNodes[k].cNode))			
			return TRUE;
	}
	return FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CCompactDict::isLayerEndpoint(CompactNode **nodePath, BYTE *layer, BOOL *chunkFlagP)
{
	WLBreakIf(layer==NULL, "!!ERROR! isEndpoint: layer is null!\n");
	*layer = 0;
	int thisnode = gWordProp->nPathNodes;
	ExtPathNode *extPathNode = gWPathway[thisnode].dictNode[m_dictIdx]; 
	if (extPathNode == EXT_NODE_NOTSET)
		return FALSE;

	ExtCNode *extCNodes = extPathNode->extCNodes;
	for (int k = 0; k < extPathNode->nExtCNodes; k++)
	{
		if (m_compactStore->isEndpoint(extCNodes[k].cNode))
		{
			*layer = extCNodes[k].layer();
			if (nodePath) 
			{
				fillNodePath(nodePath, gWPathway, m_dictIdx, &extCNodes[k]); // fill the node path by collecting all pathNodes !!
			}

			if (chunkFlagP)			
				*chunkFlagP = m_compactStore->isChunk(extCNodes[k].cNode);
			return TRUE;
		}
	}
	return FALSE;
}

// NOTE: This func assumes nPath >= 1 !!!!
/////////////////////////////////////////////////////////////////////////////////////////////
void CCompactDict::putFirstNodeInOnPathwayNode(int nPath)
{
	WLBreakIf(nPath < 1, "!!ERROR!! putFirstNodeInOnPathwayNode: nPath <= 1\n");
	ExtPathNode *extPathNode = gWPathway[nPath].dictNode[m_dictIdx];
	int prev = DEF_SUCCESSOR_PARENT;
	if(extPathNode != EXT_NODE_NOTSET)
	{
		int idx = extPathNode->nExtCNodes-1;
		CompactNode *node = extPathNode->extCNodes[idx].cNode;
		while(idx > 0 && !m_compactStore->isEndpoint(node))
			node = extPathNode->extCNodes[--idx].cNode;
		WLBreakIf(false==m_compactStore->isEndpoint(node), "!!ERROR! putFirstNodeInOnPathwayNode: path at %d should have an end point?! what happened?\n", nPath);
		prev = extPathNode->extCNodes[idx].prev;
		gWordProp->layerEndPref[gWordProp->maxLayerId-1] = max(gWordProp->layerEndPref[gWordProp->maxLayerId-1], m_compactStore->getEndPreference(node));
	}
		
	topUpPathNode(nPath, gFirstNodeOfDicts[m_dictIdx], prev, gWordProp->maxLayerId);
}
	
/////////////////////////////////////////////////////////////////////////////////////////////
void CCompactDict::backspaceNode()
{
	WLBreakIf (gWordProp->nNullMoves > 0, "!!Error! backspacenode: gWordProp->nNullMoves > 0!\n");
	gWPathway[gWordProp->nPathNodes].dictNode[m_dictIdx] = EXT_NODE_NOTSET;
}

/////////////////////////////////////////////////////////////////////////////////////////////
int CCompactDict::findEVerbHeader(BYTE *eVerbVarIdx)
{
	for (int nTrailChars = 1, nNodes = gWordProp->nPathNodes-1; nNodes >= 1; nNodes--, nTrailChars++)
	{
		ExtPathNode *extNode = gWPathway[nNodes].dictNode[m_dictIdx];
		if (extNode != EXT_NODE_NOTSET) 
		{
			int nEntries = extNode->nExtCNodes;
			for (int i = 0; i < nEntries; i++)
			{
				int allCode = extNode->extCNodes[i].cNode->Code;
				if ((allCode & CODE_EVERB_ROOT) == CODE_EVERB_ROOT)
				{
					*eVerbVarIdx = m_compactStore->getEVerbVarIdx(extNode->extCNodes[i].cNode);
					return nTrailChars;
				}
			}
		}
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
BOOL CCompactDict::isNodeAnEVerbEnding(int *nTrailingChars, BYTE *eVerbVarIdx)
{
	ExtPathNode *extPathNode = gWPathway[gWordProp->nPathNodes].dictNode[m_dictIdx];
	if(extPathNode == EXT_NODE_NOTSET)
		return FALSE;

	int nEntries = extPathNode->nExtCNodes;
	for (int i = 0; i < nEntries; i++)
	{
		if (m_compactStore->isNodeAnEVerbEnding(extPathNode->extCNodes[i].cNode))
		{
			*nTrailingChars = findEVerbHeader(eVerbVarIdx);
			return TRUE;
		}
	}
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CCompactDict::getPreferredLetters(PrefLetterNode *allPrefLetters, int nMaxLettersToRetrieve, int *nFilledLettersP, BOOL auCharsOnly)
{
	int nOverallFound = 0;
	int nFoundInTotal = 0;
	CompactNode *prefPtrs[MAX_FOLLOWING_LETTERS];
	MYWCHAR      prefLetters[MAX_FOLLOWING_LETTERS];

	if(gWordProp->nPathNodes == 0)
	{
		ShowInfo("!!WARNING!!! getPreferredLetters! nPathNodes==0 ?" );
		return 0;
	}
	ExtPathNode *extPathNode = gWPathway[gWordProp->nPathNodes].dictNode[m_dictIdx];
	if (extPathNode == NULL) 
		return 0;  // no valid node, so no preferred letters

	ExtCNode *extCNodes = extPathNode->extCNodes;
	for (int i = 0; i < extPathNode->nExtCNodes; i++) 
	{
		if (matchesFirstNodes(extCNodes[i].cNode))
		{
			if (gWordProp->nPathNodes > 0 || nMaxLettersToRetrieve < MAX_FOLLOWING_LETTERS)
				continue;
		}

		int nFound = m_compactStore->getInternalPreferredLetters(extCNodes[i].cNode, prefLetters, prefPtrs, nMaxLettersToRetrieve, auCharsOnly);

		for (int k=0; k < nFound && prefLetters[k]; k++)
		{
			// find the correct entry in the allPrefLetters array and update its statistics
			int allPrefIdx = getOrCreateAllPrefLettersIdx(allPrefLetters, MAX_FOLLOWING_LETTERS, prefLetters[k]);
			if (allPrefIdx < 0)
				continue;

			nOverallFound = max(nOverallFound, allPrefIdx + 1);
			//Minkyu:2014.06.09
			//Commented out because got assert here if type space after one letter, then backspacing.
			//Ex)"Go t ".
			//assert( m_compactStore->nextLetterNode(extCNodes[i].cNode, prefLetters[k]) == prefPtrs[k]);
			//allPrefLetters[allPrefIdx].posWordChunks += getNumWords(prefPtrs[k]);
			//allPrefLetters[allPrefIdx].pref = max(allPrefLetters[allPrefIdx].pref, prefPtrs[k]->pref);
			if(extCNodes[i].cNode)
			{
				allPrefLetters[allPrefIdx].posWordChunks += getNumWords(prefPtrs[k]);
				allPrefLetters[allPrefIdx].pref = max(allPrefLetters[allPrefIdx].pref, prefPtrs[k]->pref);
			}
		}
		nFoundInTotal += nFound;
	}
	(*nFilledLettersP) = nOverallFound;
	return nFoundInTotal;
}

/////////////////////////////////////////////////////////////////////////////////////////////
CompactNode *CCompactDict::buildAssembledPath(CompactNode *cNode, MYWCHAR *wordpart)
{
	int   wordLen = mywcslen(wordpart);
	CompactNode *succ1Node = cNode;

	for (int i = 0 ; i < wordLen; i++)
	{
		succ1Node = m_compactStore->nextLetterNode(succ1Node, wordpart[i]);
		if (succ1Node == NULL)
			return NULL;
		putNodeInAssembledPath(succ1Node);
	}
	return succ1Node;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCompactDict::collectFiveStrongestEVerbs(CompactNode *cNode, BYTE eVerbVarIdx, SearchPathSelect *prefEVerb)
{
	PrefLetterCNode prefCNodes[MAX_EVERB_ENDINGS];
	EVerbDefinitionR *eVerbs = m_compactStore->getEVerbs();

	memset(prefCNodes, 0 , sizeof(prefCNodes));
	MYWCHAR **variations = (MYWCHAR **) queryValueAtOffset((char *) &(eVerbs[eVerbVarIdx].variationsOnEVerbs), sizeof(int *), 0);
	
	int j = 0;
	int varioationsFound = 0;
	while (j < MAX_EVERB_ENDINGS)
	{
		MYWCHAR *variationsj = (MYWCHAR *) queryValueAtOffset((char *) variations, sizeof(int *), j * sizeof(int *));
		if (variationsj == 0)
			break;

		CompactNode *lastNode = buildAssembledPath(cNode, variationsj);

		// only when we stored it as a word we will honour the variation
		if (lastNode && (lastNode->Code & CODE_ENDPOINT))
		{
			prefCNodes[varioationsFound].pref = m_compactStore->getEndPreference(lastNode);
			prefCNodes[varioationsFound].cNode = lastNode;
			prefCNodes[varioationsFound++].variation = variationsj;
		}
		clearAssembledPath();
		j++;
	}

	prefCNodeSort(prefCNodes, varioationsFound);
	MYWCHAR *currWord = gWordProp->charsBuf + 1;
	for (int i = 0; i < NWORDPREFERENCES && prefCNodes[i].variation ; i++)
	{
		mywcsncpy(prefEVerb[i].text, currWord, gWordProp->nPathNodes);
		mywcscat(prefEVerb[i].text, prefCNodes[i].variation);
		prefEVerb[i].cNode = prefCNodes[i].cNode;
		prefEVerb[i].textlen = mywcslen(prefEVerb[i].text);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
void CCompactDict::collectEVerbs(int layer, BYTE eVerbVarIdx)
{
	int nRetrieved = 0;
	SearchPathSelect everbResults[NWORDPREFERENCES];

	ExtPathNode *extPathNode = gWPathway[gWordProp->nPathNodes].dictNode[m_dictIdx];
	if (extPathNode == NULL) 
	{
		ShowInfo("!!WARNING!! collectEVerbs: why node is invalid!?\n");
		return; // the node we are sitting on is invalid or nor initialized, no next words
	}
	gCurSearchLayer = layer;
	ExtCNode *extCNodes = extPathNode->extCNodes;
	for (int k = 0; k < extPathNode->nExtCNodes; k++)
	{
		if (extCNodes[k].layer() == layer)
		{
			if (matchesFirstNodes(extCNodes[k].cNode))
				return;

			// collect all the possible everb variations, and sort them on strength
			// take the five strongest and add them to the overall search results.
			memset(everbResults, 0 , sizeof(everbResults));

			collectFiveStrongestEVerbs(extCNodes[k].cNode, eVerbVarIdx,  everbResults);
			while (nRetrieved < NWORDPREFERENCES && everbResults[nRetrieved].textlen > 0)
			{
				addSearchResult(everbResults[nRetrieved].cNode, everbResults[nRetrieved].cNode->pref, everbResults[nRetrieved].text, everbResults[nRetrieved].textlen, FALSE, m_compactStore, m_dictIdx);
				nRetrieved++;
			}
		}
	}
}
/////////////////////////////////////////////////////////////////////////////
ExtCNode *CCompactDict::getLayerNode(int path, BYTE extIdx)
{
	ExtPathNode *extPathNode = gWPathway[path].dictNode[m_dictIdx];
	WLBreakIf (extPathNode == NULL, "!!ERROR!!getLayerNode: extPathNode == NULL!\n");
	WLBreakIf (extPathNode->nExtCNodes <=extIdx, "!!ERROR!!getLayerNode: extPathNode->nExtCNodes <=extIdx!\n");
	return &extPathNode->extCNodes[extIdx];
}

/////////////////////////////////////////////////////////////////////////////////////////////
void CCompactDict::constructWordPath(ExtCNode *excnode)
{
	WLBreakIf(!(excnode && excnode->cNode), "!!ERROR!!constructWordPath error!");
	memset(m_assembledWord, 0, sizeof(m_assembledWord));
	int offset = gWordProp->layerStartPos[excnode->layer()];
	int searchRootLen = (int)(gWordProp->charsBufP - &gWordProp->charsBuf[offset]);
	WLBreakIf(gWordProp->nPathNodes<1 || searchRootLen<0, "!!ERROR!!constructWordPath: pathlengh too small!\n");
	m_assembledWord[searchRootLen] = excnode->cNode->Letter;
	ExtCNode *curExNode = excnode;
	for(int i= searchRootLen-1; i>=0; i--)
	{
		curExNode = getLayerNode(i+offset, curExNode->prev); 
		WLBreakIf (curExNode == NULL, "!!ERROR!!constructWordPath: error at curExNode == NULL!\n");
		m_assembledWord[i] = curExNode->cNode->Letter;
	}
	m_assembledWordPtr = m_assembledWord + searchRootLen+1;
}

/////////////////////////////////////////////////////////////////////////////////////////////
void CCompactDict::nextWords(eSearchMode searchMode,  int layer)
{
	ExtPathNode *extPathNode = gWPathway[gWordProp->nPathNodes].dictNode[m_dictIdx];
	if (!extPathNode)
		return;			// the node we are sitting on is invalid or nor initialized, no next words
	
	gCurSearchLayer = layer;
	ExtCNode *extCNodes = extPathNode->extCNodes;
	int MaxPreferredWordPerCNode = extPathNode->nExtCNodes > 1? NWORDPREFERENCES : 2*NWORDPREFERENCES;
	for (int k = 0; k < extPathNode->nExtCNodes; k++)
	{
		int nRetrieved = 0;
		if (extCNodes[k].layer() == layer && gFirstNodeOfDicts[m_dictIdx] != extCNodes[k].cNode)
		{
			constructWordPath(&extCNodes[k]);
			while (nRetrieved < MaxPreferredWordPerCNode)
			{
				if (nextPreferredWord(extCNodes[k].cNode, searchMode)) 
					nRetrieved++;
				else
					break;
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
BOOL CCompactDict::nextPreferredWord(CompactNode *node, eSearchMode searchMode)
{
	CompactNode **oprefNodes = m_compactStore->next2PrefNodes(node);
	CompactNode *prefNodes[2] = {oprefNodes[0], oprefNodes[1]};

	if (prefNodes[0])
	{
		int code = node->Code;
		dropBreadCrumb(node, m_dictIdx, m_compactStore);
		if((code & CODE_ENDPOINT) && gWordProp->maxLayerId>0 && gWordProp->nPathNodes>gWordProp->layerEndPos[0]) // first save the very current latest layer word as typed, as a prediction. It is useful for continuous typing.
		{
			BYTE endPref = m_compactStore->getEndPreference(node);
			if(endPref > 0)
			{
				addSearchResult(node, max(endPref, node->pref),  m_assembledWord, (int)(m_assembledWordPtr - m_assembledWord), FALSE, m_compactStore, m_dictIdx);
				m_compactStore->setEndPreference(node, 0);
			}
		}
		*m_assembledWordPtr++ = prefNodes[0]->Letter; 
		putNodeInAssembledPath(prefNodes[0]);
		bool ret = findMostPreferredWord(prefNodes[0], searchMode);
		m_compactStore->reEvaluateBranches(node, (code & CODE_ENDPOINT), prefNodes[0], prefNodes[1]); 
		*(--m_assembledWordPtr) = NUL;
		takeNodeFromAssembledPath();
		return ret;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////
BOOL CCompactDict::findMostPreferredWord(CompactNode *node, eSearchMode searchMode)
{
	BYTE endPref = 0;
	int code = node->Code;
	dropBreadCrumb(node, m_dictIdx, m_compactStore);
	CompactNode **oprefNodes = m_compactStore->next2PrefNodes(node);
	CompactNode *prefNodes[2] = {oprefNodes[0], oprefNodes[1]};
	
	if (code & CODE_ENDPOINT)
	{
		endPref = m_compactStore->getEndPreference(node);
		if (endPref)
		{
			addSearchResult(node, max(endPref, node->pref),  m_assembledWord, (int)(m_assembledWordPtr - &m_assembledWord[0]), FALSE, m_compactStore, m_dictIdx);
			if (!prefNodes[0])  // no more descendants, so present this as the result
			{   
				m_compactStore->setPreference(node, TERMINATED_PREFERENCE);
			}
			else // if ( endPref>=prefNodes[0]->pref || (searchMode==eWideSearch && (code&CODE_SUFFIX)) )
			{
				if (searchMode==eDeepSearch) 
				{
					m_compactStore->setEndPreference(node, 0);
					m_compactStore->reEvaluateBranches(node, 0, prefNodes[0], prefNodes[1]); 
				}
				else
					m_compactStore->setPreference(node, TERMINATED_PREFERENCE);
			}
			return TRUE;
		}
	}

	if (prefNodes[0])
	{
		*m_assembledWordPtr++ = prefNodes[0]->Letter; 
		putNodeInAssembledPath(prefNodes[0]);
		bool ret = findMostPreferredWord(prefNodes[0], searchMode);
		m_compactStore->reEvaluateBranches(node, endPref, prefNodes[0], prefNodes[1]); 
		*(--m_assembledWordPtr) = NUL;
		takeNodeFromAssembledPath();
		return ret;
	}
	return FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////////
int CCompactDict::findTop10WordsInSubtree(CompactNode *root)
{
	clearBreadCrumbCache();  // save the dynamically changed preferences in the bread crumbs and restore the originals
	PWEresult.reset();
	while(PWEresult.nWordsFound < 10)
	{
		if(nextPreferredWordEndNode(root, eWideSearch) == false)
			break;
	}
	if(PWEresult.nWordsFound < 10)
		nextPreferredWordEndNode(root, eDeepSearch);
	restoreBreadCrumbs();
	return PWEresult.nWordsFound;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Note: maxWord2Find must be <= NumSearchWordsPerDict because of PWEresult content!
int CCompactDict::preferredWordEndNodes(CompactNode *node, eSearchMode searchMode, int maxWord2Find)
{
	while(PWEresult.nWordsFound < maxWord2Find)
	{
		if(!nextPreferredWordEndNode(node, searchMode))
			break;
	}
	return PWEresult.nWordsFound;
}

/////////////////////////////////////////////////////////////////////////////////////////////
bool CCompactDict::nextPreferredWordEndNode(CompactNode *node, eSearchMode searchMode)
{
	//BYTE endPref = 0;
	CompactNode **oprefNodes = m_compactStore->next2PrefNodes(node);
	CompactNode *prefNodes[2] = {oprefNodes[0], oprefNodes[1]};
	
	if(prefNodes[0])
	{
		int code = node->Code;
		dropBreadCrumb(node, m_dictIdx, m_compactStore);
		if (code & CODE_ENDPOINT)
		{
			BYTE endPref = m_compactStore->getEndPreference(node);
			if (endPref > 0  && node->pref != TERMINATED_PREFERENCE)
			{
				PWEresult.addNode(node, max(endPref, node->pref));
				m_compactStore->setEndPreference(node, 0);
			}
		}
		bool ret = findMostPrefWordEndNode(prefNodes[0], searchMode);
		m_compactStore->reEvaluateBranches(node, (code & CODE_ENDPOINT), prefNodes[0], prefNodes[1]); 
		return ret;
	}
	return false;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
bool CCompactDict::findMostPrefWordEndNode(CompactNode *node, eSearchMode searchMode)
{
	BYTE endPref = 0;
	int code = node->Code;
	dropBreadCrumb(node, m_dictIdx, m_compactStore);
	CompactNode **oprefNodes = m_compactStore->next2PrefNodes(node);
	CompactNode *prefNodes[2] = {oprefNodes[0], oprefNodes[1]};

	if (code & CODE_ENDPOINT)
	{
		endPref = m_compactStore->getEndPreference(node);
		if (endPref)
		{
			PWEresult.addNode(node, max(endPref, node->pref));
			if (!prefNodes[0])  // no more descendants, so present this as the result
			{   
				m_compactStore->setPreference(node, TERMINATED_PREFERENCE);
			}
			else // if ( endPref>=prefNodes[0]->pref || (searchMode==eWideSearch && (code&CODE_SUFFIX)) )
			{
				if (searchMode==eDeepSearch) 
				{
					m_compactStore->setEndPreference(node, 0);
					m_compactStore->reEvaluateBranches(node, 0, prefNodes[0], prefNodes[1]); 
				}
				else
					m_compactStore->setPreference(node, TERMINATED_PREFERENCE);
			}
			return true;
		}
	}

	if (prefNodes[0])
	{
		*m_assembledWordPtr++ = prefNodes[0]->Letter; 
		putNodeInAssembledPath(prefNodes[0]);
		bool ret = findMostPrefWordEndNode(prefNodes[0], searchMode);
		m_compactStore->reEvaluateBranches(node, endPref, prefNodes[0], prefNodes[1]); 
		return ret;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Determines if the current word is part of another word and itself is not a word!
/////////////////////////////////////////////////////////////////////////////////////////////
BOOL CCompactDict::partOfOtherWords(BOOL wordByItself)
{
	ExtPathNode *extPathNode = gWPathway[gWordProp->nPathNodes].dictNode[m_dictIdx];
	if (extPathNode == NULL)
		return FALSE;

	ExtCNode *extCNodes = extPathNode->extCNodes;

	if (gWordProp->nNullMoves > 0)
		DebugBreak((char*)"!!!partOfOtherWords nNull > 0 breaks");

	for (int k = 0; k < extPathNode->nExtCNodes; k++)
	{
		if(wordByItself || !m_compactStore->IsDeadEnd(extCNodes[k].cNode))
			return TRUE;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////
CompactNode *CCompactDict::retrieveEndNodeForString(MYWCHAR *word, bool isEndpoint)
{
	if(!word || word[0] == NUL || SP_CR_TAB(word[0]))
		return NULL;

	CompactNode *cNode = gFirstNodeOfDicts[m_dictIdx];
	return fitInDictionary(word, cNode, isEndpoint);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This function checks to see if a word fits in a dictionary, as a word or a chunk, starting from a starting node. 
CompactNode * CCompactDict::fitInDictionary(MYWCHAR *word,CompactNode *cNode, bool isEndpoint)
{
	if (!word)
		return NULL;
	else if(!word[0])
	{
		if(isEndpoint && !m_compactStore->isEndpoint(cNode)) // known in this dictionary as a word or a chunk
			return NULL;
		return cNode;
	}

	CompactNode *nextNode = m_compactStore->nextLetterNode(cNode,word[0]);
	if (nextNode != NULL)
		return fitInDictionary(&word[1], nextNode, isEndpoint);
	/*
	MYWCHAR letter = word[0];
	CompactNode **succNodes = nextNodeLetterCases(cNode, letter);
	
	if (!succNodes[0])
		return NULL;
	else 
	{
		if (succNodes[0]->a == word[0])
			return fitInDictionary(&word[1], succNodes[0]);
		else if (succNodes[1] && succNodes[1]->a == word[0])
			return fitInDictionary(&word[1], succNodes[1]); 
	}
	*/
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
int CCompactDict::getNumPossibleWords(CompactNode *node)
{
	if(node==NULL)
		node = m_compactStore->getAllocatedFirstNode();
	int possCnt = 0;
	int cnt = 0;
	int i;
	CompactNode **nextPtrs = m_compactStore->getFollowPtrs(node);
	cnt = node->Count;

	//Minkyu: 2013.09.13
	//Change to cnt  > 0, not cnt =0 to fix for checking chunk.
	if ((node->Code & CODE_ENDPOINT) && cnt > 0)
		possCnt++;

	for (i = 0; i < cnt; i++)
	{
		CompactNode *p = (CompactNode *) m_compactStore->queryPtrFieldAtOffset((char*)nextPtrs,  PTR_SIZE * i);
		possCnt += getNumPossibleWords(p);
	}
	return possCnt;
}

/////////////////////////////////////////////////////////////////////////////////////////////
int CCompactDict::getNumWords(CompactNode *node)
{
	if(node==NULL)
		return 0;
	int possCnt = 0;
	CompactNode **nextPtrs = m_compactStore->getFollowPtrs(node);
	int cnt = node->Count;
	
	if (node->Code & CODE_ENDPOINT)
		possCnt++;

	for (int i = 0; i < cnt; i++)
	{
		CompactNode *p = (CompactNode *) m_compactStore->queryPtrFieldAtOffset((char*)nextPtrs, PTR_SIZE * i);
		possCnt += getNumWords(p);
	}
	return possCnt;
}

/////////////////////////////////////////////////////////////////////////////////////////////
void CCompactDict::learnUserPreferences(CompactNode *currentNode, CompactNode *selNode)
{
	selNode->pref = min(selNode->pref+1, MAXIMUM_PREFERENCE);
	if (selNode->pref > MEDIUM_PREFERENCE && selNode->pref < PUSHDOWN_PREFERENCE)
	{
		if (m_compactStore->isChunk(currentNode))
		{	
			BYTE pref = m_compactStore->getEndPreference(currentNode);
			if (pref)
				m_compactStore->setEndPreference(currentNode, pref-1); // decrease the likelihood of this word to end
		}

		CompactNode **nextPtrs = m_compactStore->getFollowPtrs(currentNode);
		CompactNode *lowestNode = selNode;

		int cnt = currentNode->Count;
		for (int i = 0; i < cnt; i++)
		{
			CompactNode *p = (CompactNode *) m_compactStore->queryPtrFieldAtOffset((char*)nextPtrs, PTR_SIZE*i);
			if ((p->pref && p->pref < lowestNode->pref) || (p  != lowestNode && p->pref == lowestNode->pref))
				lowestNode = p;
		}
		if (lowestNode == selNode)
			return;
		lowestNode->pref = max(0, lowestNode->pref-1);
	}
}

/////////////////////////////////////////////////////////////////////
#ifdef WL_SDK
void CCompactDict::printAllWords(CompactNode *node, int count)
{
	return;
	static int sCount= 0;
	if(count == -1)
		sCount = 0;
	if(node==NULL)
		return;
	int cnt = node->Count;
	int i;
	CompactNode **nextPtrs = m_compactStore->getFollowPtrs(node);

	if (node->Code & CODE_ENDPOINT)
		printAssembledPath(sCount++);

	for (i = 0; i < cnt; i++)
	{
		CompactNode *p = (CompactNode *) m_compactStore->queryPtrFieldAtOffset((char*)nextPtrs, PTR_SIZE * i);
		putNodeInAssembledPath(p);

		if (p->Code & CODE_ENDPOINT && p->Count == 0)
			printAssembledPath(sCount++);
		else
			printAllWords(p, 0);

		takeNodeFromAssembledPath();;
	}
}
///////////////////////////////////////////////////////////////////
BOOL CCompactDict::doCountWord(MYWCHAR *wordPart)
{
	int i = 0;	// runs through the input
	CompactNode *cNode = gWPathway[gWordProp->nPathNodes].dictNode[m_dictIdx]->extCNodes[0].cNode;

	ShowInfo("doCountWord checking on #%s# ", toA(wordPart));
	while (cNode && wordPart && wordPart[i] != NUL)
	{
		cNode = m_compactStore->nextLetterNode(cNode, wordPart[i++]);
	}
	return i > 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
void CCompactDict::printPrefLetters(CompactNode *currentNode)
{
	int i;
	int cnt = currentNode->Count;
	CompactNode **nextPtrs;
	MYWCHAR prefLetters[11];
	CompactNode *prefPtrs[11];
	int nLettersToRetrieve = 10;

	memset(prefLetters, 0 , sizeof(prefLetters));
	nextPtrs = m_compactStore->getFollowPtrs(currentNode);

	for (i = 0; i < cnt; i++)
	{
		CompactNode *p = (CompactNode *) m_compactStore->queryPtrFieldAtOffset((char*)nextPtrs, PTR_SIZE * i);
		ShowInfo((" char %c has pref %d \n"), p->Letter, p->pref);
	}
	m_compactStore->getInternalPreferredLetters(currentNode, prefLetters, prefPtrs, nLettersToRetrieve, FALSE);
	ShowInfo((" prefletters are  %s\n"), prefLetters);
}
#endif
