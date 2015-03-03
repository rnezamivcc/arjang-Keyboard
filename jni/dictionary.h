#ifndef DICTIONARY_H
#define DICTIONARY_H

#include "dictrun.h"
#include "compactstore.h"

#define MAX_DICT_NAME_LENGTH    24
#define TREE_TXT	"tree.txt"
#define NGRAM	".ngram"

class CWordList;
class CCompactDict;
class Dictionary
{
public:

	Dictionary(eLanguage elang, int dictIdx);
	~Dictionary();

	BOOL	advanceLetter(MYWCHAR letter, MYWCHAR *printChunk);

	BOOL	fillNextPathNode(MYWCHAR letter, USHORT *layers, USHORT *layerEnds);
	BYTE getLetterPref(MYWCHAR letter); 
	void	putFirstNodeInOnPathwayNode(int pathlen);
	CompactNode *retrieveEndNodeForString(MYWCHAR *word, bool isEndpoint = false);
	int getPossibleNumWords(CompactNode *endNode);

	BOOL	advanceWord(MYWCHAR *wordPart, MYWCHAR *printChunk);
	BOOL	advanceWordComplete(MYWCHAR *wordPart, MYWCHAR *printChunk);
	void    backspaceLetter();

	int		nextLetters(PrefLetterNode *allPrefLetters, int nMaxLettersToRetrieve, int *nFilledLettersP, BOOL auCharsOnly);
	void	nextWords(eSearchMode searchMode, WordValue wordValue, int layerId);

	void 	collectEVerbs(WordValue wordValue, int layerId, BYTE eVerbVarIdx);

	LangTreatmentRule *getTreatmentRules();

	BOOL	isNodeAnEVerbEnding(int *nTrailingChars, BYTE *eVerbVarIdx);
	void	takeOutTheEVerb(int nPath, BYTE *pref);
	void	putTheEVerbBackIn(int nPath, BYTE *originalPref);

	BOOL	isChunk(int dictNode);
	BOOL	isEndpoint(int node);
	BOOL	isLayerEndpoint(CompactNode **nodePath, BYTE *layer);
//	BOOL	isFakeChunk();
	BOOL	partOfOtherWords(BOOL wordByItself);

	BOOL	Load(char *fullPathFileName, BOOL forceFlag, BOOL savepath = false);
	BOOL	CreateFromScratch();
	void	flushOutToDisk();

	void	Destroy(BOOL justReleaseFlag = FALSE);

	void	resetConfiguration(eLanguage langIdx);

	BOOL    usedUpEditableSpace();

#ifdef WL_SDK
	BOOL	countWord(MYWCHAR *wordPart);
	void	CreatePreferencesMap(int bytesPerPreference);
	void	printPreferences(MYWCHAR *afterTheseChars);
	//void	addWordsToWordList(CWordList *wordListObject);
//	void	addWordsToWordList(CWordList *wordListObject, int fileRef);
//	void	removeWordsFromWordList(CWordList *wordListObject);
#endif

	inline CCompactStore *getCompactStore() { return m_compactStore; }
	inline CCompactDict *getCompactDict() { return m_compactDict; }
	CompactNode*	addWord(MYWCHAR *word, int preference);
	BOOL	deleteWord(MYWCHAR *word, CompactNode *endnode);
	
	int findNextWords(CompactNode *node, WordsArray *nextWords, int num=0);
	
	int GetDictIndex() { return m_dictIdx; }
	BYTE getEndPreference(CompactNode* node) { return m_compactStore->getEndPreference(node); }
	eLanguage GetDictLanguage() { return m_eLang; }
	int getBuildNumber() { return m_compactStore->getBuildNumber(); }
	int getVersionNumber() { return m_compactStore->getVersionNumber(); }

	static char * GetDictName(char *name);
	static int    GetDictIdx(char *name);
	static char * GetDictName(int langId);
	static eLanguage   SetDictName(char *name);
	static void SetupDictionaryNames();

private:
	static char sDictNames[eLang_COUNT][MAX_DICT_NAME_LENGTH];
	char			*m_pathname; // used for debugging and testing. It is not used in release runtime.
	eLanguage		m_eLang; 
	CCompactStore	*m_compactStore;
	CCompactDict	*m_compactDict;
	int				m_dictIdx;  // index into m_orderedDictList. This can change as priority of the dictionary changes by user.
};

#endif   //DICTIONARY_H

