
#ifndef WORDPATH_H
#define WORDPATH_H

#include "dictrun.h"

CPPExternOpen

const BYTE MAXIMUM_PREFERENCE	 =  0xf0;
const BYTE LOW_PREFERENCE		 = 	0x30;
const BYTE MEDIUM_PREFERENCE	 = 	0x80;
const BYTE HIGH_PREFERENCE		 = 	0xa0;
const BYTE PUSHDOWN_PREFERENCE	 = 	0xc0;
const BYTE TERMINATED_PREFERENCE = 	0xfe;

class CCompactStore;
extern unsigned gHistWordIdx;
extern unsigned gWorkingWordIdx; 
extern bool gWorkingPathsAdvanced[NCURWORDS];
//extern BYTE gWorkingPathPrefs[NCURWORDS];// serves as preference for an active path. 0xff means not active

extern SearchResultEntry *gRankedSearchResults; 

///////////// running string structure support //////////////
void putCharInWordString(MYWCHAR c);
void takeCharOffWordString();
MYWCHAR *getWordString();
BYTE getStringLength();

/////////////////////////////////////////////////////////
extern WordPath gHistWordPaths[NWORDS]; // wordpaths for already entered words. We keep track of last NWORDS words.
extern WordPath gWorkingWordPaths[NCURWORDS]; // wordpaths for currently under construction words. 
extern CompactNode *gFirstNodeOfDicts[MAX_NUM_DICTS];

extern WordPath *gWPath;
extern NodeEntry *gWPathway;
extern WordProps *gWordProp;

void fullInitializeWordPaths(bool soft = false);

void goToNextWord(char *whoStr = NULL, bool lastWord = false, bool learn=true);
void resetWorkingWordPath(unsigned idx = gWorkingWordIdx);
void resetHistoryWordPath(unsigned idx);
bool deepCopyWorkingPath(WordPath &dest, WordPath &src, WLHeap *heap);
void goToWorkingWordPath(unsigned idx);
void resetAllWorkingWordPaths();
BYTE findNextAvailableWorkingPath();
int pruneWorkingPaths();

bool isCurWordNotInDictionaries();
int constructCurWord(MYWCHAR *word);
void getNLastCharacters(MYWCHAR *buf, int nChars);
MYWCHAR getCharAtPosition(unsigned n);
MYWCHAR *getCurrentWord(unsigned &len, bool stripped=false);

void BackToPreviousWord(bool doCopy = false);
BOOL BackToPreviousWordIfAny(bool doCopy = false);
BOOL BackToWorkingWord();

void initializePathwayNodes(WordPath *ppp, int nEntries);
void resetHisWordPath(int wordIdx);

void wipeOutNode(NodeEntry *dnsp, int thisNode);

int getWorkingWordLength(int idx = gWorkingWordIdx);
MYWCHAR *getWorkingWord(int idx);
void addToWordBuf(WordProps *wp, MYWCHAR userTypedLetter);
void removeFromWordBuf(WordProps *wordpunc);

//void deleteWordFromPersonalDictionary(MYWCHAR *word);
BOOL terminatedByOneSpace();
BOOL matchesFirstNodes(CompactNode *cNode);
void setUpWordPathPointers(bool inHistory);
void putLetterInWordpath(MYWCHAR letter, BYTE dictIdxFittedIn, PuncChoice *pc=NULL);

CompactNode *getNodePathAtPosForDict(int pos, int dictId, int layer=0);
bool startPathWithNode(CompactNode *node, int dictIdx);

inline bool CurrentWordNullTermintated(){return (*gWordProp->charsBufP!=SP && gWordProp->nNullMoves > 0);}
inline bool emptyWordPath() {return (gWordProp->nPathNodes == 0 && gWordProp->nNullMoves == 0);}
inline bool isWordUnderConstruction(){return ((gWordProp->nPathNodes > 0  || gWordProp->nNullMoves > 0));}

bool previousWordEndsWithSP();
bool firstCharAfterConnectionChar();
bool FirstMoveAfterSPTerminatedWord();

#ifdef _DEBUG
void printWordPaths(char *whenText);
void printWordPathInfo();
void printNodePath(int wpIdx, int dictIdx, int nNodes);
#else
//#define printWordPaths(text)
//void printWordPathInfo(){}
//void printNodePath(int wpIdx, int dictIdx, int nNodes){}
#endif

CPPExternClose

#endif
