
#ifndef DICTSYNCER_H
#define DICTSYNCER_H
#include "utility.h"
#if 0  // code disabled for now: reza: 23 May 2013
//#include "compactstore.h"
//#include "dicttree.h"
//#include "dictionary.h"
//#include "dictmanager.h"
#include "utility.h"

#define MAX_SYNCERDICTS 10

typedef struct fileIdEntry 
{
	char *textFileName;
	int fileId;
	struct fileIdEntry *next;
} FileIdEntry;

class CWordList;
class Dictionary;
class CDictionaryTree;
class CDictManager;

class CDictSyncer
{
  
public:
    CDictSyncer();
    ~CDictSyncer();

	BOOL  addExistingDictionary(char *dictFileName);
	BOOL  addDeletedWordsDictionary(char *dictFileName);
	BOOL  replaceExistingDictionaryFilter(char *oldDictFileName, char *newDictFileName);

	void  findNewWords(char *szTextFileName, BOOL articleFlag);
	void  findNewWords(char *szTextFileName,  BOOL articleFlag, BOOL originalPrefFlag, BOOL unicodeFlag);

	BOOL  addNewWordInTree(MYWCHAR *word, char *outlookSource);
	void  syncTreeToWordListObject();
	void  syncWordListObjectToTree();
	int   getFileId(TCHAR *szTextFileName);
	const char *getFileName(int fileId);

	CWordList *getWordListObject();
	BOOL  generateDictionary(char *dictFileName);
	BOOL  generateSessionDictionary(char *dictFileName);

	BOOL  mergeDictionaries(char *outDictFileName);
	void addWordsToWordList(char *dictFileName);
	void printTree(char *szFileName);

private:
	
	int		findOrAllocateFileId(TCHAR *szTextFileName);
	void	closeAllDictionaries();
	BOOL	closeExistingDictionaryFilter(int mDictIdx);
	BOOL setExistingDictionaryFilter(int mDictIdx, char *dictFileName, Dictionary *existingDictionary);
	Dictionary *openExistingDictionaryFilter(char *dictFileName, int dictIdx);
	BOOL    dsyncerGenerateDictionary(char *dictFileName, BOOL finalCall);
	void  dsyncFindNewWords(char *szTextFileName, BOOL articleFlag, BOOL originalPrefFlag, BOOL unicodeFlag);
	
	CDictionaryTree *m_finalNewWordsTree;
	CWordList		*m_wordListObject;
	CDictManager    *m_dictManager;

	FileIdEntry *m_fileIdsList;
	int m_fileId;

	int				m_dicts;
	struct 
	{
		char *dictFileName;
		Dictionary *existingDictionary;
		BOOL   deleteTheseWords;
	} m_dictList[MAX_SYNCERDICTS];
};
#endif
#endif   //DICTSYNCER_H
