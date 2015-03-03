#if 0  // code disabled for now: Reza: 23 May 2013
#include "StdAfx.h"
#include "dictsyncer.h"
#include "dicttree.h"
#include "dictmanager.h"
#include "wordpath.h"
#include "dictionary.h"

CDictSyncer::CDictSyncer()
{
	m_dicts = 0;
	m_fileIdsList = NULL;
	m_fileId = UI_FILE_REFERENCE;
	findOrAllocateFileId("User");

	m_finalNewWordsTree = new CDictionaryTree();
	m_wordListObject	= new CWordList();

	// needed for the creation of the log file
	m_dictManager = new CDictManager();
	m_dictManager->fillActiveConfiguration();
}

CDictSyncer::~CDictSyncer()
{
	FileIdEntry *fip;
	FileIdEntry *nextFip;

	for (fip = m_fileIdsList; fip != NULL; fip = nextFip)
	{
		free(fip->textFileName);
		nextFip = fip->next;
		free(fip);
	}
	if (m_finalNewWordsTree)
	{
//		m_finalNewWordsTree->printTree(TEXT("d:\\wordlogic\\treefinal.txt"));
		delete m_finalNewWordsTree;
	}
	if (m_wordListObject)
		delete m_wordListObject;

	closeAllDictionaries();

	if (m_dictManager)
		delete m_dictManager;
}

BOOL CDictSyncer::addDeletedWordsDictionary(char *dictFileName)
{
	BOOL ret = addExistingDictionary(dictFileName);
	if (ret == TRUE)
		m_dictList[m_dicts-1].deleteTheseWords = TRUE;
	return ret;
}

BOOL CDictSyncer::addExistingDictionary(char *dictFileName)
{
	if (m_dicts >= (MAX_SYNCERDICTS - 1))
		return FALSE; 

	Dictionary *existingDictionary = openExistingDictionaryFilter(dictFileName, m_dicts);
	if (!existingDictionary)
		return FALSE;

	if (m_dicts == 0)
	{
		// dump the preference and dictionaryMap 
		// use the part of the preference dump as counters
		existingDictionary->CreatePreferencesMap(sizeof(int)); 
	}

	setExistingDictionaryFilter(m_dicts, dictFileName, existingDictionary);
	
	m_dicts++;

	return TRUE;
}

BOOL CDictSyncer::setExistingDictionaryFilter(int mDictIdx, char *dictFileName, Dictionary *existingDictionary)
{
	char *p = (char *) malloc(strlen(dictFileName) + 1);
	if (p == NULL)
		return FALSE;

	strcpy(p, dictFileName);
	m_dictList[mDictIdx].dictFileName = p;
	m_dictList[mDictIdx].existingDictionary = existingDictionary;
	m_dictList[mDictIdx].deleteTheseWords = FALSE;
	return TRUE;
}

Dictionary *CDictSyncer::openExistingDictionaryFilter(char *dictFileName, int dictIdx)
{
	Dictionary *existingDictionary = new Dictionary(Dictionary::SetDictName(dictFileName), dictIdx);
	if (! existingDictionary->Create(dictFileName, TRUE))
	{
		delete existingDictionary;
		return NULL;
	}
	return existingDictionary;
}

BOOL CDictSyncer::closeExistingDictionaryFilter(int mDictIdx)
{
	if (m_dictList[mDictIdx].existingDictionary)
	{
		m_dictList[mDictIdx].existingDictionary->Destroy();
		delete m_dictList[mDictIdx].existingDictionary;
		m_dictList[mDictIdx].existingDictionary = NULL;
		if (m_dictList[mDictIdx].dictFileName)
		{
			free(m_dictList[mDictIdx].dictFileName);
			m_dictList[mDictIdx].dictFileName = NULL;
		}
	}
	return TRUE;
}

BOOL CDictSyncer::replaceExistingDictionaryFilter(char *oldDictFileName, char *newDictFileName)
{
	if (m_dicts == 0 || m_dicts >= (MAX_SYNCERDICTS - 1))
		return FALSE; 

	for (int i = 1; i < m_dicts; i++)
	{
		if (_stricmp(m_dictList[i].dictFileName, oldDictFileName) == 0)
		{
			// replace the old dictionary file with a new dictionary file
			closeExistingDictionaryFilter(i);

			Dictionary *existingDictionary = openExistingDictionaryFilter(newDictFileName,i);
			if (!existingDictionary)
				return FALSE;

			setExistingDictionaryFilter(i, newDictFileName, existingDictionary);

			return TRUE;
		}
	}
	return FALSE;
}

int CDictSyncer::getFileId(char *szTextFileName)
{
	FileIdEntry *fip;

	for (fip = m_fileIdsList; fip != NULL; fip = fip->next)
	{
		if (_stricmp(fip->textFileName, szTextFileName) == 0)
			return fip->fileId;
	}
	return -1;
}

const char *CDictSyncer::getFileName(int fileId)
{
	FileIdEntry *fip;

	for (fip = m_fileIdsList; fip != NULL; fip = fip->next)
	{
		if (fip->fileId == fileId)
			return fip->textFileName;
	}
	return NULL;
}

int CDictSyncer::findOrAllocateFileId(char *szTextFileName)
{
	int fileId = getFileId(szTextFileName);

	if (fileId < 0)
	{
		FileIdEntry *fip = (FileIdEntry *) malloc(sizeof(FileIdEntry));

		fip->fileId = m_fileId++;
		fip->textFileName = (char *) malloc(strlen(szTextFileName)+1);
		strcpy(fip->textFileName,szTextFileName);
		fip->next = m_fileIdsList;
		m_fileIdsList = fip;
		return fip->fileId;
	}
	return fileId;
}

void CDictSyncer::findNewWords(char *szTextFileName, BOOL articleFlag)
{
	dsyncFindNewWords(szTextFileName, articleFlag, FALSE, FALSE);
}

void CDictSyncer::findNewWords(char *szTextFileName,  BOOL articleFlag,  BOOL originalPrefFlag, BOOL unicodeFlag)
{
	dsyncFindNewWords(szTextFileName, articleFlag, originalPrefFlag, unicodeFlag);
}

void CDictSyncer::dsyncFindNewWords(char *szTextFileName, BOOL articleFlag, BOOL originalPrefFlag, BOOL unicodeFlag)
{
	int fileId = findOrAllocateFileId(szTextFileName);
	CWordList *wordListObject;
	Dictionary *referenceDictionary = NULL; 

	if (m_dicts == 0)
	{
		if (articleFlag == TRUE) {
			m_finalNewWordsTree->processArticle(szTextFileName, NULL,fileId, unicodeFlag); 
		} else {
			m_finalNewWordsTree->processDictFile(szTextFileName,"spd",WORDSIDE_DICTIONARY, 
									NULL,fileId, 0, FALSE, unicodeFlag); 
		}
	} else 
	{
		referenceDictionary = m_dictList[0].existingDictionary;

		for (int i = 0 ; i < m_dicts; i++)
		{
			CDictionaryTree *newWordsTree = new CDictionaryTree();

			if (i == 0)
			{
				// process against the base dictionary to find new words
				if (articleFlag == TRUE) {
//					newWordsTree->processArticle(szTextFileName, m_dictList[i].existingDictionary,
//						fileId, unicodeFlag); 
					newWordsTree->processArticleLanguage(szTextFileName, m_dictList[i].existingDictionary,
						NULL, fileId, unicodeFlag); 
				} else {
					newWordsTree->processDictFile(szTextFileName,"spd",WORDSIDE_DICTIONARY,
								m_dictList[i].existingDictionary, fileId, 0, FALSE, unicodeFlag); 
				}
			}
			else
			{
				newWordsTree->processWordList(wordListObject, m_dictList[i].existingDictionary); 
				delete wordListObject;
			}

			wordListObject = new CWordList();
			newWordsTree->generateListOfNewWords(wordListObject,NULL, WL_ALL_ENTRIES);
			delete newWordsTree;
		}
		// add the new words to the final new words tree
		m_finalNewWordsTree->mergeWordListIntoTree(wordListObject, TREE_RULES);


		if (originalPrefFlag == FALSE)
		{
			if (referenceDictionary)
			{
//				referenceDictionary->printSimilarPreferencesMap(TEXT("d:\\similarpref.txt")); 
		//		referenceDictionary->calcAvgSimilarPrefsForOccurences(); 
//				m_finalNewWordsTree->printTree(TEXT("d:\\treebassign.txt"));
		//		m_finalNewWordsTree->assignSimilarPreferences(referenceDictionary);
//				m_finalNewWordsTree->printTree(TEXT("d:\\treeaassign.txt"));
			}
			else
			{
				// we don't have any reference results so normalize in the normal way
				m_finalNewWordsTree->normalizePreferences();
			}
		}

//		m_finalNewWordsTree->printTree(TEXT("d:\\wordlogic\\treeprocess1.txt"));
		delete wordListObject;
	}
}

void  CDictSyncer::syncTreeToWordListObject()
{
	m_finalNewWordsTree->mergeTreeIntoWordList(m_wordListObject,NULL);
}

void  CDictSyncer::syncWordListObjectToTree()
{
	m_finalNewWordsTree->mergeWordListIntoTree(m_wordListObject,WORDLIST_RULES); 
}

void CDictSyncer::closeAllDictionaries()
{
	for (int i = 0 ; i < m_dicts; i++)
	{
		closeExistingDictionaryFilter(i);
	}

	m_dicts = 0;
}

BOOL  CDictSyncer::generateSessionDictionary(char *dictFileName)
{
	return dsyncerGenerateDictionary(dictFileName, FALSE);
}

BOOL  CDictSyncer::generateDictionary(char *dictFileName)
{
	return dsyncerGenerateDictionary(dictFileName, TRUE);
}

BOOL  CDictSyncer::dsyncerGenerateDictionary(char *dictFileName, BOOL finalCall)
{
	BOOL ret = FALSE;

	if (finalCall)
		closeAllDictionaries();

	CWordList *wantedListObject = new CWordList();

	// first remove the unwanted entries by selecting the wanted entries
	m_finalNewWordsTree->generateListOfNewWords(wantedListObject,NULL, 
		WL_NORMAL | WL_USER_ADDED_NEW_WORD_OR_ASSIGNED_PREFERENCE);

	if (finalCall) 
	{
		delete m_finalNewWordsTree;
		m_finalNewWordsTree = NULL;
	}

	CDictionaryTree *wantedWordsTree = new CDictionaryTree();
	wantedWordsTree = new CDictionaryTree();
	wantedWordsTree->processWordList(wantedListObject, NULL); 
	delete wantedListObject;

	wantedWordsTree->collectSuffixes();
	wantedWordsTree->countWords(NULL);
//	wantedWordsTree->printTree(TEXT("d:\\usertree.txt"));

	if (wantedWordsTree->compactTree(NULL)) 
	{
		ret = wantedWordsTree->compactToFile(dictFileName);
		delete wantedWordsTree;
	}
	else
		free(dictFileName);

	return ret;
}

BOOL  CDictSyncer::addNewWordInTree(MYWCHAR *word, char *outlookSource)
{
	int fileId = findOrAllocateFileId(outlookSource);

	for (int i = 0 ; i < m_dicts; i++)
	{
		if (m_dictList[i].existingDictionary->countWord(word))
			return FALSE;

	}
	m_finalNewWordsTree->storeDictionaryWord(word, MEDIUM_PREFERENCE, 
						L"", L"", FALSE, NULL, WORDSIDE_DICTIONARY,fileId);
	return TRUE;
}

CWordList *  CDictSyncer::getWordListObject()
{
	return m_wordListObject;
}

void CDictSyncer::addWordsToWordList(char *dictFileName)
{
	for (int i = 0; i < m_dicts; i++)
	{
		if (_stricmp(m_dictList[i].dictFileName, dictFileName) == 0)
		{
			int fileId = findOrAllocateFileId(dictFileName);
			m_dictList[i].existingDictionary->addWordsToWordList(m_wordListObject, fileId);
			return;
		}
	}
}

void CDictSyncer::printTree(char *szFileName)
{
	m_finalNewWordsTree->printTree(szFileName);
}

BOOL CDictSyncer::mergeDictionaries(char *outDictFileName)
{
	CWordList *wantedListObject = new CWordList();
	CWordList *newWordsDesktopListObject = new CWordList();
	CWordList *personalWordListObject = new CWordList();

	// the treatment of the personal and newpersonal dictionary is fixed for now
	// first make sure the new personal dictionary takes over the learned user preferences
	// after that, add new words from the device to it and subtract the deleted words
	// from the device.

	// to take over the learned preferences the following algorithm is used.
	// 1. zeroize all words in newpersonal which exist in personal dictionary.
	// 2. pick up the non-zeroized pieces and put them into a newwords.desktop list.
	// 3. takeover all the preferences from personal.dict into newpersonal.dict
	// 4. reapply the preferences from the newwords.desktop list. 
	
	int personalWordLen = strlen("personal");
	int newPersonalWordLen = personalWordLen;
	Dictionary *personalDictionary = NULL;
	Dictionary *newPersonalDictionary = NULL;

	for (int i = 0 ; i < m_dicts; i++)
	{
		int dictFileNameLen = strlen(m_dictList[i].dictFileName);
		if (dictFileNameLen >= newPersonalWordLen)
		{
			char *p = &m_dictList[i].dictFileName[dictFileNameLen-newPersonalWordLen];
		//	if (stricmp(p, NEWPERSONAL_DICTIONARY) == 0) {
		//		newPersonalDictionary = m_dictList[i].existingDictionary;
			//	m_dictList[i].existingDictionary = NULL;
			//	continue;
		//	}
		}
		if (dictFileNameLen >= personalWordLen)
		{
			char *p = &m_dictList[i].dictFileName[dictFileNameLen-personalWordLen];
			if (strstr(p, "personal") == 0) 
			{
				personalDictionary = m_dictList[i].existingDictionary;
				m_dictList[i].existingDictionary = NULL;
			}
		}
	}

	// any deleted words in the personal dictionary are in the deletedwords.dict
	if (personalDictionary)
		personalDictionary->addWordsToWordList(wantedListObject);

	if (newPersonalDictionary)
		newPersonalDictionary->addWordsToWordList(wantedListObject);

	wantedListObject->putNonZeroPrefWordsIntoWordList(newWordsDesktopListObject);

	delete wantedListObject;
	wantedListObject = new CWordList();

	if (newPersonalDictionary)
		newPersonalDictionary->addWordsToWordList(wantedListObject);
	else if (personalDictionary)
		personalDictionary->addWordsToWordList(wantedListObject);

	if (personalDictionary)
		personalDictionary->addWordsToWordList(personalWordListObject);

	// first take the old preferences and put then the new preferernces on top of it
	// copy the preferences over from the personalWordList
	wantedListObject->takeOverPreferences(personalWordListObject);
	// copy the preferences over from the newWordsDesktopList
	wantedListObject->takeOverPreferences(newWordsDesktopListObject);

	// now deal with the newwords.dev and deletedwords.dev dictionaries
	int i;
	for (i = 0 ; i < m_dicts; i++)
	{
		if (m_dictList[i].existingDictionary && m_dictList[i].deleteTheseWords == FALSE)
			m_dictList[i].existingDictionary->addWordsToWordList(wantedListObject);
	}

	for (i = 0 ; i < m_dicts; i++)
	{
		if (m_dictList[i].existingDictionary && m_dictList[i].deleteTheseWords == TRUE)
			m_dictList[i].existingDictionary->removeWordsFromWordList(wantedListObject);
	}

	m_finalNewWordsTree = new CDictionaryTree();
	m_finalNewWordsTree->mergeWordListIntoTree(wantedListObject,WORDLIST_RULES); 

	delete wantedListObject;
	delete newWordsDesktopListObject;
	delete personalWordListObject;

	return generateDictionary(outDictFileName);
}
#endif