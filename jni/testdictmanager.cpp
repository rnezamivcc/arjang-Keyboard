

#include "stdAfx.h"
#include "testdictmanager.h"
#include "dictmanager.h"
#if !defined(_WINDOWS)
#include <android/log.h>
#endif

//#include "wordlist.h"
#include "dictionary.h"
#include "userWordCache.h"

CTestDictManager * CTestDictManager::sTestDictMgr = NULL;
MultiLetterAdvanceAr *gnextWordsAr;

int  CTestDictManager::getWordsAddedSinceLastCallAndReset(BYTE *cnt)
{ 
	if(!m_dictManager)
		return 0;
	return m_dictManager->mngrGetWordsAddedSinceLastCallAndReset(cnt); 
}

DictionaryConfigurationEntry *CTestDictManager::GetOrderedDict(int idx)
{
	if(!m_dictManager)
		return NULL;
	return m_dictManager->GetOrderedDict(idx);
}

bool CTestDictManager::SetOrderedDict(int langId, int priority, bool enabled)
{
	return m_dictManager && m_dictManager->SetOrderedDict(langId, priority, enabled);
}

MYWCHAR *CTestDictManager::nextLetters(int **posWordsAr) 
{ 
	if(!m_dictManager)
		return NULL;
	return m_dictManager->nextLetters(posWordsAr);
}

BOOL  CTestDictManager::resetConfiguration() 
{ 
	return 	m_dictManager && m_dictManager->resetConfiguration();
}

BOOL CTestDictManager::addWord(MYWCHAR *newWord, int pref, int dictIdx) 
{ 
	return 	m_dictManager && m_dictManager->addWord(newWord, pref, &dictIdx);
}
	
BOOL CTestDictManager::deleteWord(MYWCHAR *newWord) 
{ 
	return 	m_dictManager && m_dictManager->deleteWord(newWord);
}
	
void CTestDictManager::setAutoLearn(BOOL bAutoLearn) 
{ 
	if(!m_dictManager)
		return;
	m_dictManager->setAutoLearn(bAutoLearn);
}

BOOL CTestDictManager::isChunk(MYWCHAR *wordPart) 
{ 
	return m_dictManager && m_dictManager->isChunk(wordPart);
}

MYWCHAR *CTestDictManager::getRootText() 
{ 
	if(!m_dictManager)
		return NULL;
	return m_dictManager->gatherRootEnding();
}
/////////////////////////////////////////////////////////////////////////////////////////
MYWCHAR ** CTestDictManager::printNextWordsATC(char *ansiWordPart, char *afterTheseChars)
{
	unsigned nPosWords;
	if(!m_dictManager)
		return NULL;
		
	MYWCHAR rootWord[MAX_WORD_LEN];
//	memset(rootWord, 0, sizeof(rootWord));

	gnextWordsAr = m_dictManager->multiNextWords(rootWord); 
	ShowInfo("Next words after root=#%s#:\n", toA(getCurrentWord(nPosWords)));
	for (int i=0; gnextWordsAr->nextWords[i][0] != NUL; i++)
	{
		ShowInfo("--%d: #%s#,", i, toA(gnextWordsAr->nextWords[i]));
		printf("--%d: #%s#,", i, toA(gnextWordsAr->nextWords[i]));
	}
	ShowInfo("\n");
	printf("\n");
	return NULL;
}
///////////////////////////////////////////////////////////////////////////////////////////////
CTestDictManager* CTestDictManager::createTestDict(char *path, char *dictname)
{
	if (!sTestDictMgr)
	{
		sTestDictMgr = new CTestDictManager();
		if (!sTestDictMgr->Create(path, dictname))
		{
			sTestDictMgr->Destroy();
			return NULL;
		}
	}
	return sTestDictMgr;
}
//////////////////////////////////////////////////////////////////////////////
bool CTestDictManager::SetDictMgr()
{
	if (!createTestDict())
	{
		return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef _WINDOWS
#include <conio.h>
void CTestDictManager::runTest()
{
	BOOL startSentence = true;
	BOOL bAdded = FALSE;
	BOOL bDeleted  = FALSE;
	MYWCHAR *pkletters;
	MYWCHAR *printP;
	MYWCHAR *currword;
	MYWCHAR root[MAX_WORD_LEN];
	MYWCHAR *rootText = root;
	MYWCHAR *prefLetters;
	int *myposAr = NULL;
	unsigned count = 0;
	extern int gNumOfWords;
	MYWCHAR* listofwords;
	unsigned buildnum, versionnum;
	byte prefs_4[4]={1, 2, 1,1};
	byte prefs_3[3]={1, 2, 1};
	byte prefs_2[2]={1, 2};
	byte prefs_1[1] = { 1};
	unsigned nextCount;
	MYWCHAR word[64];
	if (!SetDictMgr())
	{
		ShowInfo("!!ERROR!! no dictionary loaded! so no prediction!!\n");
		return;
	}
//	 sTestDictMgr->setAutoLearn(true);
	startSentence = false;
	sTestDictMgr->m_dictManager->mSwiper.testPath(L"rtvuiughyt");
	sTestDictMgr->m_dictManager->mSwiper.processPathAndReset();
	sTestDictMgr->m_dictManager->mSwiper.testPath(L"mkoiuythgfder");
	sTestDictMgr->m_dictManager->mSwiper.processPathAndReset();

	//sTestDictMgr->advanceLetter('f', &printP, startSentence);
	//count = gnextWordsAr->nActualNexts;
	//for(int i=0; i<count; i++)
	//	ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));
	sTestDictMgr->advanceLetter('h', &printP, startSentence);
	sTestDictMgr->advanceLetter('o', &printP, startSentence);
	//for(int i=0;i <13000; i++)
	{

		currword = makeWord(L"at");
		sTestDictMgr->ProcessPhrasePrediction(currword, false, NULL);

		currword = makeWord(L"make");
		nextCount = sTestDictMgr->m_dictManager->advanceWord( currword, &printP, gnextWordsAr, true, false);
		count = gnextWordsAr->nActualNexts;
		for(int i=0; i<count; i++)
			ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));


		currword = makeWord(L"sure");
		nextCount = sTestDictMgr->m_dictManager->advanceWord( currword, &printP, gnextWordsAr, true, false);
		count = gnextWordsAr->nActualNexts;
		for(int i=0; i<count; i++)
			ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));

		currword = makeWord(L"the");
		nextCount = sTestDictMgr->m_dictManager->advanceWord( currword, &printP, gnextWordsAr, true, false);
		count = gnextWordsAr->nActualNexts;
		for(int i=0; i<count; i++)
			ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));

		currword = makeWord(L"me");
		nextCount = sTestDictMgr->m_dictManager->advanceWord( currword, &printP, gnextWordsAr, true, false);
		count = gnextWordsAr->nActualNexts;
		for(int i=0; i<count; i++)
			ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));
		int dictId;
		CompactNode *ret = sTestDictMgr->m_dictManager->retrieveEndNodeForString(toW("It"), &dictId, true);

		sTestDictMgr->addWord(toW("reza"), 64, 1);
		//sTestDictMgr->advanceLetter('e', &printP, startSentence);
		//sTestDictMgr->advanceLetter('n', &printP, startSentence);
		//sTestDictMgr->advanceLetter('t', &printP, startSentence);
		//sTestDictMgr->advanceLetter(' ', &printP, startSentence);

		sTestDictMgr->advanceLetter('y', &printP, startSentence);
		sTestDictMgr->advanceLetter('o', &printP, startSentence);
		sTestDictMgr->advanceLetter('u', &printP, startSentence);
		sTestDictMgr->advanceLetter(' ', &printP, startSentence);
		
		sTestDictMgr->advanceLetter('a', &printP, startSentence);
		sTestDictMgr->advanceLetter(' ', &printP, startSentence);

		sTestDictMgr->advanceLetter('n', &printP, startSentence);
		sTestDictMgr->advanceLetter('e', &printP, startSentence);
		sTestDictMgr->advanceLetter('w', &printP, startSentence);
		sTestDictMgr->advanceLetter(' ', &printP, startSentence);


		sTestDictMgr->advanceLetter('c', &printP, startSentence);
		sTestDictMgr->advanceLetter('o', &printP, startSentence);
		sTestDictMgr->advanceLetter('p', &printP, startSentence);
		sTestDictMgr->advanceLetter('y', &printP, startSentence);
		sTestDictMgr->advanceLetter(' ', &printP, startSentence);

		sTestDictMgr->advanceLetter('o', &printP, startSentence);
		sTestDictMgr->advanceLetter('f', &printP, startSentence);
		sTestDictMgr->advanceLetter(' ', &printP, startSentence);

		nextCount = sTestDictMgr->m_dictManager->advanceWord( currword, &printP, gnextWordsAr, true, false);
		count = gnextWordsAr->nActualNexts;
		for(int i=0; i<count; i++)
			ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));


		currword = makeWord(L"you");
		//sTestDictMgr->ProcessPhrasePrediction(currword);
		nextCount = sTestDictMgr->m_dictManager->advanceWord( currword, &printP, gnextWordsAr, true, false);
		count = gnextWordsAr->nActualNexts;
		for(int i=0; i<count; i++)
			ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));

		currword = makeWord(L"a");
		//sTestDictMgr->ProcessPhrasePrediction(currword);
		nextCount = sTestDictMgr->m_dictManager->advanceWord( currword, &printP, gnextWordsAr, true, false);
		count = gnextWordsAr->nActualNexts;
		for(int i=0; i<count; i++)
			ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));

		currword = makeWord(L"new");
		////sTestDictMgr->ProcessPhrasePrediction(currword);
		//nextCount = sTestDictMgr->m_dictManager->advanceWord( currword, &printP, gnextWordsAr, true, false);
		//count = gnextWordsAr->nActualNexts;
		//for(int i=0; i<count; i++)
		//	ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));

		//currword = makeWord(L"copy");
		////sTestDictMgr->ProcessPhrasePrediction(currword);
		//nextCount = sTestDictMgr->m_dictManager->advanceWord( currword, &printP, gnextWordsAr, true, false);
		//count = gnextWordsAr->nActualNexts;
		//for(int i=0; i<count; i++)
		//	ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));

		//currword = makeWord(L"of");
		////sTestDictMgr->ProcessPhrasePrediction(currword);
		//nextCount = sTestDictMgr->m_dictManager->advanceWord( currword, &printP, gnextWordsAr, true, false);
		//count = gnextWordsAr->nActualNexts;
		//for(int i=0; i<count; i++)
		//	ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));
	}

	currword = makeWord(L"bite");
	nextCount = sTestDictMgr->m_dictManager->advanceWord( currword, &printP, gnextWordsAr, true, false);
	count = gnextWordsAr->nActualNexts;
	for(int i=0; i<count; i++)
		ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));


	currword = makeWord(L"to");
	nextCount = sTestDictMgr->m_dictManager->advanceWord( currword, &printP, gnextWordsAr, true, false);
	count = gnextWordsAr->nActualNexts;
	for(int i=0; i<count; i++)
		ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));


	//sTestDictMgr->advanceLetter('t', &printP, startSentence);
	//count = gnextWordsAr->nActualNexts;
	//for(int i=0; i<count; i++)
	//	ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));

	//sTestDictMgr->advanceLetter('h', &printP, startSentence);
	//count = gnextWordsAr->nActualNexts;
	//for(int i=0; i<count; i++)
	//	ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));

	//sTestDictMgr->advanceLetter('i', &printP, startSentence);
	//count = gnextWordsAr->nActualNexts;
	//for(int i=0; i<count; i++)
	//	ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));



	//sTestDictMgr->backspaceLetter();
	//count = gnextWordsAr->nActualNexts;
	//for(int i=0; i<count; i++)
	//	ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));

	nextCount = sTestDictMgr->m_dictManager->advanceWord( currword, &printP, gnextWordsAr, true, false);
	count = gnextWordsAr->nActualNexts;
	for(int i=0; i<count; i++)
		ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));


	currword = makeWord(L"make");
	//sTestDictMgr->ProcessPhrasePrediction(currword);
	nextCount = sTestDictMgr->m_dictManager->advanceWord( currword, &printP, gnextWordsAr, true, false);
	count = gnextWordsAr->nActualNexts;
	for(int i=0; i<count; i++)
		ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));

	currword = makeWord(L"sent");
	nextCount = sTestDictMgr->m_dictManager->advanceWord( currword, &printP, gnextWordsAr, true, false);
	count = gnextWordsAr->nActualNexts;
	for(int i=0; i<count; i++)
		ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));

	//sTestDictMgr->advanceLetter('G', &printP, startSentence);
	//count = gnextWordsAr->nActualNexts;
	//for(int i=0; i<count; i++)
	//	ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));

	//sTestDictMgr->advanceLetter('o', &printP, startSentence);
	//count = gnextWordsAr->nActualNexts;
	//for(int i=0; i<count; i++)
	//	ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));
	////currword = makeWord(L"55");
	//nextCount = sTestDictMgr->m_dictManager->advanceWord( currword, &printP, gnextWordsAr, true, false);
	//count = gnextWordsAr->nActualNexts;
	//for(int i=0; i<count; i++)
	//	ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));

	//currword = makeWord(L"t");
	//nextCount = sTestDictMgr->m_dictManager->advanceWord( currword, &printP, gnextWordsAr, true, false);
	//count = gnextWordsAr->nActualNexts;
	//for(int i=0; i<count; i++)
	//	ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));

	//currword = makeWord(L"o");
	//nextCount = sTestDictMgr->m_dictManager->advanceWord( currword, &printP, gnextWordsAr, true, true);
	//count = gnextWordsAr->nActualNexts;
	//for(int i=0; i<count; i++)
	//	ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));

	//sTestDictMgr->advanceLetter('t', &printP, startSentence);
	//count = gnextWordsAr->nActualNexts;
	//for(int i=0; i<count; i++)
	//	ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));

	//sTestDictMgr->backspaceLetter();
	//sTestDictMgr->backspaceLetter();

	//return;


	//sTestDictMgr->advanceLetter('o', &printP, startSentence);
	//count = gnextWordsAr->nActualNexts;
	//for(int i=0; i<count; i++)
	//	ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));


	//sTestDictMgr->advanceLetter(' ', &printP, startSentence);
	//count = gnextWordsAr->nActualNexts;
	//for(int i=0; i<count; i++)
	//	ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));


	//sTestDictMgr->backspaceLetter();
	//sTestDictMgr->backspaceLetter();
	//sTestDictMgr->backspaceLetter();
	//sTestDictMgr->backspaceLetter();
	//sTestDictMgr->backspaceLetter();
	//sTestDictMgr->backspaceLetter();
	//return;



	//sTestDictMgr->backspaceLetter();
	//sTestDictMgr->backspaceLetter();
	//sTestDictMgr->backspaceLetter();
	//sTestDictMgr->backspaceLetter();
	//sTestDictMgr->backspaceLetter();




	//sTestDictMgr->advanceLetter(' ', &printP, startSentence);
	//count = gnextWordsAr->nActualNexts;
	//for(int i=0; i<count; i++)
	//	ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));

	//sTestDictMgr->backspaceLetter();
	//sTestDictMgr->backspaceLetter();
	//sTestDictMgr->backspaceLetter();

	////sTestDictMgr->advanceLetter('e', &printP, startSentence);
	////count = gnextWordsAr->nActualNexts;
	////for(int i=0; i<count; i++)
	////	ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));

	//sTestDictMgr->advanceLetter(' ', &printP, startSentence);
	//count = gnextWordsAr->nActualNexts;
	//for(int i=0; i<count; i++)
	//	ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));
	

	//return;


	//currword = makeWord(L"more");
	//nextCount = sTestDictMgr->m_dictManager->advanceWord( currword, &printP, gnextWordsAr, true, false);
	//count = gnextWordsAr->nActualNexts;
	//for(int i=0; i<count; i++)
	//	ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));

	//currword = makeWord(L"information");
	//nextCount = sTestDictMgr->m_dictManager->advanceWord( currword, &printP, gnextWordsAr, true, false);
	//count = gnextWordsAr->nActualNexts;
	//for(int i=0; i<count; i++)
	//	ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));
	//


	//currword = makeWord(L"just");
	//nextCount = sTestDictMgr->m_dictManager->advanceWord( currword, &printP, gnextWordsAr, true, false);
	//count = gnextWordsAr->nActualNexts;
	//for(int i=0; i<count; i++)
	//	ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));
	//currword = makeWord(L"keep");
	//nextCount = sTestDictMgr->m_dictManager->advanceWord( currword, &printP, gnextWordsAr, true, false);
	//count = gnextWordsAr->nActualNexts;
	//for(int i=0; i<count; i++)
	//	ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));

	//currword = makeWord(L"typing");
	//nextCount = sTestDictMgr->m_dictManager->advanceWord( currword, &printP, gnextWordsAr, true, false);
	//count = gnextWordsAr->nActualNexts;
	//for(int i=0; i<count; i++)
	//	ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));

	//currword = makeWord(L"pros");
	//nextCount = sTestDictMgr->m_dictManager->advanceWord( currword, &printP, gnextWordsAr, true, false);
	//count = gnextWordsAr->nActualNexts;
	//for(int i=0; i<count; i++)
	//	ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));
	//currword = makeWord(L"and");
	//nextCount = sTestDictMgr->m_dictManager->advanceWord( currword, &printP, gnextWordsAr, true, false);
	//count = gnextWordsAr->nActualNexts;
	//for(int i=0; i<count; i++)
	//	ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));

	//currword = makeWord(L"cons");
	//nextCount = sTestDictMgr->m_dictManager->advanceWord( currword, &printP, gnextWordsAr, true, false);
	//count = gnextWordsAr->nActualNexts;
	//for(int i=0; i<count; i++)
	//	ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));


	//currword = makeWord(L"come");
	//nextCount = sTestDictMgr->m_dictManager->advanceWord( currword, &printP, gnextWordsAr, true, false);
	//count = gnextWordsAr->nActualNexts;
	//for(int i=0; i<count; i++)
	//	ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));

	//currword = makeWord(L"up");
	//nextCount = sTestDictMgr->m_dictManager->advanceWord( currword, &printP, gnextWordsAr, true, false);
	//count = gnextWordsAr->nActualNexts;
	//for(int i=0; i<count; i++)
	//	ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));


	//currword = makeWord(L"with");
	//nextCount = sTestDictMgr->m_dictManager->advanceWord( currword, &printP, gnextWordsAr, true, false);
	//count = gnextWordsAr->nActualNexts;
	//for(int i=0; i<count; i++)
	//	ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));
	//return;




	//sTestDictMgr->advanceLetter('T', &printP, startSentence);
	//count = gnextWordsAr->nActualNexts;
	//for(int i=0; i<count; i++)
	//	ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));

	//sTestDictMgr->advanceLetter('e', &printP, startSentence);
	//count = gnextWordsAr->nActualNexts;
	//for(int i=0; i<count; i++)
	//	ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));

	//sTestDictMgr->advanceLetter('l', &printP, startSentence);
	//count = gnextWordsAr->nActualNexts;
	//for(int i=0; i<count; i++)
	//	ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));


	//sTestDictMgr->advanceLetter('l', &printP, startSentence);
	//count = gnextWordsAr->nActualNexts;
	//for(int i=0; i<count; i++)
	//	ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));


	//sTestDictMgr->advanceLetter(' ', &printP, startSentence);
	//count = gnextWordsAr->nActualNexts;
	//for(int i=0; i<count; i++)
	//	ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));

	//sTestDictMgr->backspaceLetter();
	//sTestDictMgr->backspaceLetter();

	//sTestDictMgr->advanceLetter('l', &printP, startSentence);
	//count = gnextWordsAr->nActualNexts;
	//for(int i=0; i<count; i++)
	//	ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));


	//sTestDictMgr->advanceLetter('l', &printP, startSentence);
	//count = gnextWordsAr->nActualNexts;
	//for(int i=0; i<count; i++)
	//	ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));

	//sTestDictMgr->advanceLetter(' ', &printP, startSentence);
	//count = gnextWordsAr->nActualNexts;
	//for(int i=0; i<count; i++)
	//	ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));

	//sTestDictMgr->advanceLetter('h', &printP, startSentence);
	//count = gnextWordsAr->nActualNexts;
	//for(int i=0; i<count; i++)
	//	ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));

	//sTestDictMgr->advanceLetter('i', &printP, startSentence);
	//count = gnextWordsAr->nActualNexts;
	//for(int i=0; i<count; i++)
	//	ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));

	//sTestDictMgr->advanceLetter('m', &printP, startSentence);
	//count = gnextWordsAr->nActualNexts;
	//for(int i=0; i<count; i++)
	//	ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));

	//sTestDictMgr->advanceLetter(' ', &printP, startSentence);
	//count = gnextWordsAr->nActualNexts;
	//for(int i=0; i<count; i++)
	//	ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));

	//sTestDictMgr->advanceLetter('k', &printP, startSentence);
	//count = gnextWordsAr->nActualNexts;
	//for(int i=0; i<count; i++)
	//	ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));

	//sTestDictMgr->advanceLetter('e', &printP, startSentence);
	//count = gnextWordsAr->nActualNexts;
	//for(int i=0; i<count; i++)
	//	ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));

	//sTestDictMgr->advanceLetter('e', &printP, startSentence);
	//count = gnextWordsAr->nActualNexts;
	//for(int i=0; i<count; i++)
	//	ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));

	//sTestDictMgr->advanceLetter('p', &printP, startSentence);
	//count = gnextWordsAr->nActualNexts;
	//for(int i=0; i<count; i++)
	//	ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));

	//sTestDictMgr->backspaceLetter();

	//sTestDictMgr->advanceLetter('p', &printP, startSentence);
	//count = gnextWordsAr->nActualNexts;
	//for(int i=0; i<count; i++)
	//	ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));


	//sTestDictMgr->advanceLetter(' ', &printP, startSentence);
	//count = gnextWordsAr->nActualNexts;
	//for(int i=0; i<count; i++)
	//	ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));

	//sTestDictMgr->advanceLetter('i', &printP, startSentence);
	//count = gnextWordsAr->nActualNexts;
	//for(int i=0; i<count; i++)
	//	ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));

	//sTestDictMgr->advanceLetter('n', &printP, startSentence);
	//count = gnextWordsAr->nActualNexts;
	//for(int i=0; i<count; i++)
	//	ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));

	//currword = makeWord(L"in");
	//nextCount = sTestDictMgr->m_dictManager->advanceWord( currword, &printP, gnextWordsAr, true, false);
	//count = gnextWordsAr->nActualNexts;
	//for(int i=0; i<count; i++)
	//	ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));


	//sTestDictMgr->advanceLetter(' ', &printP, startSentence);
	//count = gnextWordsAr->nActualNexts;
	//for(int i=0; i<count; i++)
	//	ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));


	//sTestDictMgr->backspaceLetter();
	//sTestDictMgr->backspaceLetter();
	//sTestDictMgr->backspaceLetter();
	//return;


	//	sTestDictMgr->advanceLetter('b', &printP, startSentence);
	//count = gnextWordsAr->nActualNexts;
	//for(int i=0; i<count; i++)
	//	ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));

	//sTestDictMgr->advanceLetter('i', &printP, startSentence);
	//count = gnextWordsAr->nActualNexts;
	//for(int i=0; i<count; i++)
	//	ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));

	//sTestDictMgr->advanceLetter('t', &printP, startSentence);
	//count = gnextWordsAr->nActualNexts;
	//for(int i=0; i<count; i++)
	//	ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));


	//	sTestDictMgr->advanceLetter('e', &printP, startSentence);
	//count = gnextWordsAr->nActualNexts;
	//for(int i=0; i<count; i++)
	//	ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));



	//sTestDictMgr->advanceLetter(' ', &printP, startSentence);
	//count = gnextWordsAr->nActualNexts;
	//for(int i=0; i<count; i++)
	//	ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));


	//sTestDictMgr->advanceLetter('t', &printP, startSentence);
	//count = gnextWordsAr->nActualNexts;
	//for(int i=0; i<count; i++)
	//	ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));

	//sTestDictMgr->advanceLetter('o', &printP, startSentence);
	//count = gnextWordsAr->nActualNexts;
	//for(int i=0; i<count; i++)
	//	ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));



	//sTestDictMgr->advanceLetter(' ', &printP, startSentence);
	//count = gnextWordsAr->nActualNexts;
	//for(int i=0; i<count; i++)
	//	ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));


	//sTestDictMgr->advanceLetter('i', &printP, startSentence);
	//count = gnextWordsAr->nActualNexts;
	//for(int i=0; i<count; i++)
	//	ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));
 	delete sTestDictMgr;
	sTestDictMgr = NULL;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void CTestDictManager::testWordAdvance(MYWCHAR *word)
{
	if(!m_dictManager)
		return;
	m_dictManager->reset();
	for (int i = 0; i < (int) mywcslen(word); i++)
		tMngrAdvanceLetter(word[i]);
	tMngrAdvanceLetter(SP);

	sTestDictMgr->printNextWordsATC( toA(word), "");
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void CTestDictManager::testAddWord(MYWCHAR *word, int pref)
{
	if(!m_dictManager)
		return;
	addWord(word, pref, 1);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void CTestDictManager::testCharAdvance(char c)
{
	if(!m_dictManager)
		return;
	tMngrAdvanceLetter(char2W(c));
	char thisword[2]; 
	thisword[0] = c;
	thisword[1] = 0;
	sTestDictMgr->printNextWordsATC( thisword, "");
}

#endif
//////////////////////////////////////////////////////////////////////////////////////////////////
CTestDictManager::CTestDictManager()
{
	m_dictManager = new CDictManager();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
CTestDictManager::~CTestDictManager()
{
	if (m_dictManager) 
	{
		delete m_dictManager;
		m_dictManager = NULL;
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CTestDictManager::Create(char *path, char* dictname)
{
	BOOL bCreate = FALSE;
	if (m_dictManager)
	{
		bCreate = m_dictManager->Create(dictname, path);
		if(bCreate)
		{
			
			////Minkyu:2013.09.23
			////For testing purpose
			//for(int g=0; g < 100;g++)
			//{
			//	m_dictManager->CreateFileCache("d:\\sms.txt");
			//}
			
		}

	}
		
	return bCreate;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void CTestDictManager::Destroy()
{
	if (m_dictManager) 
	{
		m_dictManager->Destroy();
		m_dictManager = NULL;
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CTestDictManager::tMngrAdvanceLetter(MYWCHAR letter)
{
	MYWCHAR *printP = NULL;
	return m_dictManager && m_dictManager->advanceLetter(letter, &printP);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CTestDictManager::advanceLetter(char letter, MYWCHAR **printPart,  BOOL startSentence)
{
	MYWCHAR *printP;
	MYWCHAR str[2] = { letter, NUL };
	BYTE prefs[1] = { 1 };
	MYWCHAR rootWord[MAX_WORD_LEN];
//	memset(rootWord, 0, sizeof(rootWord));
	gnextWordsAr = sTestDictMgr->m_dictManager->advanceMultiLetters( str, prefs, rootWord, &printP, false);
	return true; //(m_dictManager && m_dictManager->advanceLetter(char2W(letter), printPart,  startSentence));
}
//////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CTestDictManager::advanceLetter(MYWCHAR letter, MYWCHAR **printPart, BOOL startSentence)
{
	return (m_dictManager && m_dictManager->advanceLetter(letter, printPart, startSentence));
}
//////////////////////////////////////////////////////////////////////////////////////////////////
int CTestDictManager::getDictPriority(const char *name)
{
	if(!m_dictManager)
		return -1;
	return m_dictManager->getDictPriority(name);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
MYWCHAR * CTestDictManager::setWordInfo(BOOL *canBeAddedFlag, BOOL *canBeDeletedFlag, CompactNode **nodePath)
{
	if(!m_dictManager)
		return NULL;
	*canBeAddedFlag = m_dictManager->CurrentWordCanbeAdded(); 
	return getRootText();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CTestDictManager::advanceWord(MYWCHAR *wordPart, MYWCHAR **printPart, MultiLetterAdvanceAr *nexts)
{	
	callLog("advanceWord\n");
	if(!m_dictManager)
		return false;
	return m_dictManager->advanceWord(wordPart, printPart, nexts, false); 
}
//////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CTestDictManager::advanceWordComplete(MYWCHAR *wordPart, MYWCHAR **printPart, MultiLetterAdvanceAr *nexts)
{	
	if(!m_dictManager)
		return false;
	return m_dictManager->advanceWord(wordPart, printPart, nexts, true); 
}
//////////////////////////////////////////////////////////////////////////////////////////////////
MYWCHAR *CTestDictManager::undoLetterOrWord()
{
	if(!m_dictManager)
		return NULL;
	return m_dictManager->undoLetterOrWord();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void CTestDictManager::backspaceLetter()
{
	MYWCHAR rootWord[MAX_WORD_LEN];
	if(m_dictManager)
		gnextWordsAr = m_dictManager->backspaceLetter(rootWord);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void CTestDictManager::ProcessPhrasePrediction(MYWCHAR *inputWord, bool backspace, MYWCHAR* nextLetter)
{
	if(m_dictManager)
	{
		m_dictManager->ProcessPhrasePrediction(inputWord);
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////////
MYWCHAR **CTestDictManager::nextWords(int *nPosWordsPtr, MYWCHAR **rootWordP)
{
	if(!m_dictManager)
		return NULL;
		
	MYWCHAR rootWord[MAX_WORD_LEN];
	memset(rootWord, 0, sizeof(rootWord));
	return m_dictManager->GetNextWords(nPosWordsPtr, rootWord);
}
//////////////////////////////////////////////////////////////////////////////////////////////////

