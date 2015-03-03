
#ifndef TESTDICTMANAGER_H
#define TESTDICTMANAGER_H

#include "wltypes.h"
#include "dictmanager.h"

struct CompactNode;


class CTestDictManager
{
  
public:
    CTestDictManager();
    ~CTestDictManager();

	BOOL Create(char *path=NULL, char* dictname = NULL);
	void Destroy();

	MYWCHAR **nextWords(int *nPosWordsPtr, MYWCHAR **rootWord);
	BOOL advanceLetter(char letter, MYWCHAR **printPart, BOOL startSentence);
	BOOL advanceLetter(MYWCHAR letter, MYWCHAR **printPart, BOOL startSentence);

	BYTE advanceWord(MYWCHAR *wordPart,MYWCHAR **printPart, MultiLetterAdvanceAr *nexts);
	BYTE advanceWordComplete(MYWCHAR *wordPart, MYWCHAR **printPart, MultiLetterAdvanceAr *nexts);

	MYWCHAR *undoLetterOrWord();
	void backspaceLetter();
	void ProcessPhrasePrediction(MYWCHAR *inputWord, bool backspace, MYWCHAR* nextLetter);
	MYWCHAR *nextLetters(int **posWordsAr);
	MYWCHAR *allNextLetters(int **posWordsAr);
	MYWCHAR *allNextLettersPhrase(int **posWordsAr);
	 MYWCHAR *getRootText();

	int getWordsAddedSinceLastCallAndReset(BYTE *cnt);

	int getDictPriority(const char *name);
	DictionaryConfigurationEntry *GetOrderedDict(int idx);
	bool SetOrderedDict(int langId, int priority, bool enabled);

	BOOL isChunk(MYWCHAR *wordPart);
	MYWCHAR *setWordInfo(BOOL *canBeAddedFlag, BOOL *canBeDeletedFlag, CompactNode **nodePath);
	void reset() {if(m_dictManager) m_dictManager->reset();}
	void fullReset() {if(m_dictManager)  m_dictManager->fullReset();}


	BOOL  resetConfiguration();
	BOOL addWord(MYWCHAR *newWord, int pref, int dictIdx);
	BOOL deleteWord(MYWCHAR *newWord);
	void setAutoLearn(BOOL bAutoLearn);

#ifdef RATECLIPBOARD
	void rateText(MYWCHAR *word, int clipLen, CRateList *rator);
#endif

	BOOL tMngrAdvanceLetter(MYWCHAR letter);

	void printPreferences(MYWCHAR *afterTheseChars) { m_dictManager->printPreferences(afterTheseChars);}
	void printNextWordsAtNode(CompactNode *cNode, int dictIdx) { /*	m_dictManager->printNextWordsAtNode(cNode, dictIdx); */}

	MYWCHAR ** printNextWordsATC(char *ansiWordPart, char *afterTheseChars);
	static CTestDictManager * createTestDict(char *path=NULL, char *dictname=NULL);
	static bool SetDictMgr();

#ifdef _WINDOWS
	static void runTest();
	void testAddWord(MYWCHAR *word, int preference);
	void testCharAdvance(char c);
	void testWordAdvance(MYWCHAR *word);
#endif

	static CTestDictManager * sTestDictMgr;
	CDictManager *m_dictManager;
};

#endif   //TESTDICTMANAGER_H
