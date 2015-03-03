// <copyright file="wordpathext.cpp" company="WordLogic">
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
// <summary>We use extended word path nodes in our wordpath array. This file contains a set of uitilities for that.</summary>

#include "StdAfx.h"
#ifndef _WINDOWS
#include "wltypes.h"
#endif
#include "utility.h"

CPPExternOpen

#include "wordpathext.h"
#include "wordpath.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void convertCNodeIntoExtCNodeWithFlavor(ExtCNode *extCNode, CompactNode *cNode, BYTE prev, BYTE layer, BYTE flavor)
{
	extCNode->cNode = cNode;
	extCNode->prev = prev;
	extCNode->SetLayerFlavor(layer, flavor);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void convertCNodeIntoExtCNode(ExtCNode *extCNode, CompactNode *cNode, BYTE prev, BYTE flags)
{
	extCNode->cNode = cNode;
	extCNode->prev = prev;
	extCNode->flags = flags;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void convertCNodeToPathNode(ExtPathNode **myExtPathNode, CompactNode *cNode, BYTE wordIdx, BYTE prev, BYTE flags)
{
	WLBreakIf(!cNode, "!!ERROR! convertCNodeToPathNode: cNode==NULL ?!\n");
	if(!(*myExtPathNode))
		*myExtPathNode = (ExtPathNode *)getWorkingHeap(wordIdx)->allocate(sizeof(ExtPathNode));
	(*myExtPathNode)->nExtCNodes = 1;
	convertCNodeIntoExtCNode((*myExtPathNode)->extCNodes, cNode, prev, flags);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL fillNodePath(CompactNode **nodePath, NodeEntry *dnsp, int dictIdx, ExtCNode *extCNode)
{
	BYTE walkingParent = extCNode->prev;

	nodePath[gWordProp->nPathNodes] = extCNode->cNode;
	for (int i = gWordProp->nPathNodes-1; i >= 0; i--)
	{
		ExtPathNode *extPathNode = gWPathway[i].dictNode[dictIdx];
		if (extPathNode == NULL)
		{
			// we made apparently a jump in the past, jot down the CASC_NOT_INIT
			CompactNode *cNode = dnsp[i].dictNode[dictIdx]->extCNodes[0].cNode;
			nodePath[i] = cNode;
			walkingParent = DEF_SUCCESSOR_PARENT;
		}
		else
		{
			assert(walkingParent >= 0 && walkingParent <= extPathNode->nExtCNodes);
			ExtCNode *prevExtCNode = &extPathNode->extCNodes[walkingParent];
			nodePath[i] = prevExtCNode->cNode;
			walkingParent = prevExtCNode->prev;
		}
	}
	return TRUE;
}


CPPExternClose