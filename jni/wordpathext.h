#ifndef WORDPATHEXT_H
#define WORDPATHEXT_H

#include "dictrun.h"

CPPExternOpen
void convertCNodeIntoExtCNodeWithFlavor(ExtCNode *extCNode, CompactNode *cNode, BYTE prev, BYTE layer, BYTE flavor);
void convertCNodeIntoExtCNode(ExtCNode *extCNode, CompactNode *cNode, BYTE prev, BYTE flags);
void convertCNodeToPathNode(ExtPathNode **myExtPathNode, CompactNode *cNode, BYTE wordIdx, BYTE prev, BYTE flags);
BOOL fillNodePath(CompactNode **nodePath, NodeEntry *dnsp, int dictIdx, ExtCNode *extCNode);

CPPExternClose

#endif
