// <copyright file="dictconfig.cpp" company="WordLogic Corporation">
// Copyright (c) 2000, 2013 All Right Reserved, http://www.wordlogic.com/
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
// <summary>Provides support for initializing the engine, loading and updating dictionaries an paths.</summary>
//

#include "StdAfx.h"
#include <stdio.h>
#include "dictmanager.h"
#include "dictionary.h"
#include "compactstore.h"
#include "wordpath.h"
#include "userWordCache.h"
#include "userWordCacheOffline.h"
#include "parsedline.h"
#include "phraseEngine.h"
#include "T-Graph.h"
#include "autocorrect.h"

#ifdef _WINDOWS
#include "direntw.h"
#else
#include <dirent.h>
#endif

CPPExternOpen

char *g_installPath = NULL;
BOOL sConfigurationRead = false;
char g_currentVersion[24] = "3.0";

BYTE sortDictionariesBasedOnPriorities(DictionaryConfigurationEntry *configDict, int size);
USHORT CDictManager::m_nExistingDictionaries = 0;  // number of existing dictionaries in /dictionary folder. 

WLHeap CDictManager::sWHHeap[NWORDS];
WLHeap CDictManager::sWPHeap[NCURWORDS];

#ifdef _WINDOWS
char *gLocalInstallPath = "."; // default base location for code and data, used for pc.
#else
char *gLocalInstallPath = (char*)"/sdcard/wordlogic";
#endif
const char *g_dictionarySubDir = "/dictionary";

void wipeoutBreadCrumbs();
void allocateSearchStorage();
void releaseSearchStorage();
////////////////////////////////////////////////////////////////////////////////
WLHeap *getHistHeap(int heapIdx)
{
    WLBreakIf(!(heapIdx>=0 && heapIdx<NWORDS), "!!ERROR! getHistHeap: index(%d) out of range (0, %d)!!\n", heapIdx, NWORDS);
    return &CDictManager::sWHHeap[heapIdx];
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
bool BelongToHistHeap(int heapIdx, char *address)
{
    WLBreakIf(!(heapIdx>=0 && heapIdx<NWORDS), "!!ERROR! BelongToHistHeap: for index(%d) address (%x)out of range!!\n", heapIdx, address);
    return CDictManager::sWHHeap[heapIdx].belongs(address);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
WLHeap *getWorkingHeap(int heapIdx)
{
    WLBreakIf(!(heapIdx>=0 && heapIdx<NCURWORDS), "!!ERROR! getWorkingHeap: index(%d) out of range (0, %d)!!\n", heapIdx, NCURWORDS);
    return &CDictManager::sWPHeap[heapIdx];
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
bool BelongToWorkingHeap(int heapIdx, char *address)
{
    WLBreakIf(!(heapIdx>=0 && heapIdx<NCURWORDS), "!!ERROR! BelongToWorkingHeap: for index(%d) address (%x)out of range!!\n", heapIdx, address);
    return CDictManager::sWPHeap[heapIdx].belongs(address);
}
#ifdef __APPLE__
int GetConfigEntry(DictionaryConfigurationEntry *configDictList, char *dictNameP)
{
    //	ShowInfo("GetConfigEntry: numExistingDict=%d, dictNameP=%s\n", CDictManager::getNumExistingDictionaries(), dictNameP);
    for(int i=0; i<CDictManager::getNumExistingDictionaries(); i++)
    {
        char *curname = Dictionary::GetDictName(configDictList[i].langIdx);
        //	ShowInfo("--curname=%s for langIdx=%d\n", curname, configDictList[i].langIdx);
        if( strcmp(dictNameP, curname) == 0)
            return i;
    }
    return -1;
}

void saveConfigFile(DictionaryConfigurationEntry *configDictList, const char *filename, int nDictEntries)
{
    FILE *stream;
    ShowInfo("saveConfigFile: %s \n", filename);
    if ( (stream = fopen(filename, "w" )) != NULL )
    {
        fprintf(stream,"#dictionary priority en/disabled \n");
        for (int i=0; i < nDictEntries; i++)
            fprintf(stream,"%s %d %d\n", Dictionary::GetDictName(configDictList[i].langIdx), configDictList[i].priority, configDictList[i].enabled);
        fclose( stream );
    }
}

char *createFullPathFileName(const char *fileName, char *ext)
{
    ShowInfo("createFullPathFileName: fileName=#%s#, gHistWordIdx=%d: Extension: %s\n", fileName, gHistWordIdx, ext);
    static char sFullPathName[1024];
    memset(sFullPathName, 0, 1024);
    if (g_installPath == NULL)
    {
        ShowInfo("!!ERROR!!readDirectoryConfiguration need to be called first!!\n");
        return NULL;
    }
    
    sprintf(sFullPathName, ("%s/%s"), g_szStartDirectory, fileName);
    
    // add extention if any, and if it is not already there!
    if(ext != NULL)
    {
        int full_len = (int)strlen(sFullPathName);
        int ext_len = (int)strlen(ext);
        if((full_len + ext_len) < 1024 && !strstr(sFullPathName, ext))
            strcat(sFullPathName, ext);
    }
    
    return sFullPathName;
}

BYTE sortDictionariesBasedOnPriorities(DictionaryConfigurationEntry *configDict, int nActiveDicts)
{
    ShowInfo("sortDictionariesBasedOnPriorities: nActiveDicts = %d\n", nActiveDicts);
    DictionaryConfigurationEntry priorityRanking[MAX_NUM_DICTS]; // = (DictionaryConfigurationEntry *)calloc( 1, arrsize);
    bool visited[MAX_NUM_DICTS];
    for(int i=0; i<MAX_NUM_DICTS; i++) visited[i]= false;
    
    // scan fist for the occurence of the personal dictionary, if it exists
    // it will have the highest priority 0 no matter what. It will also appear in the
    // list first and users cannot change its priority
    BYTE curPriority = 0;
    for (int k=0; k < CDictManager::getNumExistingDictionaries(); k++)
    {
        if (configDict[k].langIdx == eLang_PERSONAL)
        {
            priorityRanking[0] =configDict[k];
            priorityRanking[0].priority = curPriority;
            priorityRanking[0].dictIdxWeight = max(1, (int)curPriority);
            curPriority++;
            visited[k] = true;
            break;
        }
    }
    
    // do it for the rest of the dictionaries
    for (int k=0; k < CDictManager::getNumExistingDictionaries(); k++)
    {
        BYTE highestPriority = 0xfe;
        int highestIdx = -1;
        
        for (int n = 0; n < CDictManager::getNumExistingDictionaries() ; n++)
        {
            if (configDict[n].priority  < highestPriority && !visited[n] && configDict[n].priority > 0 && configDict[n].enabled)
            {
                highestPriority = configDict[n].priority;
                highestIdx = n;
            }
        }
        
        if (highestIdx >= 0)
        {
            priorityRanking[curPriority] = configDict[highestIdx];
            priorityRanking[curPriority].priority = curPriority;
            priorityRanking[curPriority].dictIdxWeight = max(1, (int)curPriority);
            visited[highestIdx] = true; // scratch this entry out
            curPriority++;
        }
        else
            break;
    }
    
    CDictManager::SetNumOrderedEnabledDictionaries(curPriority);
    
    // now copy the rest of dictionaries, which are not enabled, to priority ranking:
    for(int i=0; i<CDictManager::getNumExistingDictionaries(); i++)
    {
        bool found = false;
        for(int j=0; j<curPriority; j++)
        {
            if(priorityRanking[j].langIdx == configDict[i].langIdx)
            {
                configDict[i].priority = curPriority;
                found = true;
                break;
            }
        }
        if(!found)
            priorityRanking[curPriority++] = configDict[i];
    }
    
    memcpy(&configDict[0], &priorityRanking[0], sizeof(DictionaryConfigurationEntry)*CDictManager::getNumExistingDictionaries());
#if 0 //_DEBUG
    ShowInfo("\n");
    for(int i=0; i < CDictManager::getNumExistingDictionaries(); i++)
    {
        ShowInfo("AfterSort: %d: langIdx=%d, prio=%d, name=%s\n", i, configDict[i].langIdx, configDict[i].priority, Dictionary::GetDictName(configDict[i].langIdx));
    }
#endif
    return curPriority;
}

int readDirectoryConfiguration(DictionaryConfigurationEntry *configDictList, int listSize)
{
    ShowInfo("ReadDirectoryConfiguration: setting up install path and reading content of dictionary folder: sConfigurationRead=%d, g_installPath=%s\n", sConfigurationRead, g_installPath);
    if(!sConfigurationRead)
    {
        if(configDictList == NULL)
        {
            listSize = (MAX_NUM_DICTS*3);
            configDictList = (DictionaryConfigurationEntry *) calloc(listSize, sizeof(DictionaryConfigurationEntry));
        }
    }
    
    ShowInfo("readDirectoryConfiguration: m_installPath %s \n", g_installPath);
    ShowInfo("readDirectoryConfiguration: g_szStartDirectory %s \n", g_szStartDirectory);
    ShowInfo("readDirectoryConfiguration: m_currentVersion %s \n", g_currentVersion);
    
    //first set slot 0 for personal
    int slot = 0;
    configDictList[slot].priority = 0;
    configDictList[slot].langIdx = eLang_PERSONAL;
    configDictList[slot].enabled = TRUE;  // enabled by default. Can be disabled from setting
    configDictList[slot].existingDictionary = NULL;
    slot++;
    
#ifdef _WINDOWS
#define FILENAMELEN ent->d_namlen
#else
#define FILENAMELEN (strlen(ent->d_name))
#endif
    
    // now read what's in dictionary folder
    //BOOL bListFull = slot >= listSize;
    DIR *dir = opendir(g_szStartDirectory);
    ShowInfo("Parsing dictionary folder %s for dictionaries! %d\n", g_szStartDirectory, dir!= NULL);
    if(dir != NULL)
    {
        // TODO - This code doesn't work on ios -- needs fixing
        /*struct dirent *ent = readdir(dir);
        while(! bListFull && (ent != NULL))
        {
            if(	ent->d_type == DT_REG && FILENAMELEN > strlen(LDAT)		&&
               !strcmp(ent->d_name + FILENAMELEN - strlen(LDAT), LDAT)	&&
               strcmp(ent->d_name, "personal.ldat") != 0 )
            {
                eLanguage langIdx = Dictionary::SetDictName(ent->d_name);
                if(langIdx == eLang_DEFAULT)
                    ShowInfo("--dictionary %s is not in our list! We load it as loose!\n", ent->d_name);
                configDictList[slot].priority = slot;
                configDictList[slot].langIdx = langIdx;
                configDictList[slot].existingDictionary = NULL;
                configDictList[slot].enabled = TRUE; // enable them all by default! to avoid case of single dict like german not being enabled!
                ShowInfo("--Dictionary %s available! Added it to list! langIdx=%d, priority=%d\n", ent->d_name, configDictList[slot].langIdx, configDictList[slot].priority);
                slot++;
                bListFull = (slot >= listSize);
            }
            ent = readdir(dir);
        }*/
        
        // Load the english dictionary
        char fullPath[300];
        strcpy(fullPath, g_szStartDirectory);
#ifdef __LP64__
        strcat(fullPath, "/english64.ldat");
#else
        strcat(fullPath, "/english32.ldat");
#endif
        
        FILE *fp = fopen(fullPath, "rb");
        if (fp != NULL)
        {
            eLanguage langIdx = Dictionary::SetDictName((char *)fullPath);
            if(langIdx == eLang_DEFAULT)
                ShowInfo("--dictionary %s is not in our list! We load it as loose!\n", fullPath);
            configDictList[slot].priority = slot;
            configDictList[slot].langIdx = langIdx;
            configDictList[slot].existingDictionary = NULL;
            configDictList[slot].enabled = TRUE; // enable them all by default! to avoid case of single dict like german not being enabled!
            ShowInfo("--Dictionary %s available! Added it to list! langIdx=%d, priority=%d\n", fullPath, configDictList[slot].langIdx, configDictList[slot].priority);
            slot++;
            fclose(fp);
        }
        
        closedir(dir);
    }
    else
    {
        ShowInfo("ERROR!! Couldn't open folder %s ??\n", g_szStartDirectory);
        return 0;
    }
    
    sConfigurationRead = true;
    return slot;
}

void createUrlConfigFilename(char *userFilenamePath, char *szUserFilename)
{
    strcpy(userFilenamePath, g_szStartDirectory);
    //mywcscat(userFilenamePath, "\\");
    strcat(userFilenamePath, szUserFilename);
}
    
CPPExternClose
#endif

CDictManager *wpDictManager = NULL;
PhraseEngineR *gPhraseEngine = NULL; // global phrase engine pointer.

///////////////////////////////////////////////////////////////////////////////////////////////////////
CDictManager::CDictManager() 
{
	memset(g_szStartDirectory, 0, sizeof(g_szStartDirectory));
	memset(gFirstNodeOfDicts, 0, MAX_NUM_DICTS * sizeof(CompactNode *));
	wpDictManager = this;
	m_eVerbRetrievalActive = FALSE;
	m_noDictionariesLoaded = TRUE;
	m_autoLearnActive = TRUE;
	m_spacelessTyping = TRUE;
	m_nOrderedDicts = 0;
	//m_personalDictIdx = INVALID_DICT_IDX;
	mUserWordCache = NULL; 
	m_pUserWordCacheOffline = NULL;
	m_TGraph = NULL;
#ifdef WL_SDK
	mPhraseEngine = NULL;
#endif
	gPhraseEngine = NULL;
	m_NGramLearning = NULL;
	mTopDictionary = NULL;
	mAutoCorrect = NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////
CDictManager::~CDictManager() 
{
	Destroy();
}
///////////////////////////////////////////////////////////////////////////////////////////////
dictionaryConfigurationEntry::~dictionaryConfigurationEntry() 
{ 
	delete existingDictionary; 
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Dictionary *CDictManager::createFromScratch(int nDictIdx, char *fileName) 
{
	char *fullPathFileName = createFullPathFileName(fileName, LDAT);
	eLanguage elang = Dictionary::SetDictName(fileName);
	Dictionary *newDict = new Dictionary(elang, nDictIdx);

	ShowInfo("CDictManager::createFromScratch %s with elang=%d \n", fullPathFileName, elang);

	if (newDict->CreateFromScratch() == FALSE)
	{
		newDict->Destroy(FALSE);
		delete newDict;
		return NULL;
	}
	return newDict;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CDictManager::Create(char *dictname, char *root) 
{	
	Dictionary::SetupDictionaryNames();
	for(int i=0; i<NWORDS; i++)
		sWHHeap[i].set(NWORDS* 1024 * MAX_NUM_DICTS);
	for(int i=0; i<NCURWORDS; i++)
		sWPHeap[i].set(NCURWORDS* 1024 * MAX_NUM_DICTS); 
	
	if(dictname != NULL) // this means funciton is not called from app, but rather from tools. So avoid alot of non-core setups!
	{
		if(root==NULL)
			root = gLocalInstallPath;
		static char sRootDirPath[MAX_PATH];
		g_installPath = sRootDirPath;
		strcpy(g_installPath, root);
		strcpy(g_szStartDirectory, g_installPath);
		strcat(g_szStartDirectory, g_dictionarySubDir);
		sConfigurationRead = true;
		ShowInfo("CDictManger::Create entered: g_szStartDirectory=%s\n", g_szStartDirectory);
		eLanguage elang = Dictionary::SetDictName(dictname);
		if(elang == eLang_DEFAULT)
			ShowInfo("!!WARNING!!CDictManager::Create!!dictionary %s is not in our list! We load it as loose!\n", dictname);
		m_nExistingDictionaries = 1;
		m_orderedDictList[0].priority = 1;
		m_orderedDictList[0].langIdx = elang;
		m_orderedDictList[0].existingDictionary = NULL;
		m_orderedDictList[0].enabled = TRUE; 

		bool ret = false;
		if(FileExists(dictname))
			ret = openDictionary(dictname);
		else
			ret = openDictionary(createFullPathFileName(dictname, LDAT));
		if(!ret)
			ret = openDictionary(createFullPathFileName(dictname, (char*)""));
		
		if(!ret)
		{
			printf("!Failed to Open dictionary %s\n", dictname);
			return false;
		}
	   
		m_noDictionariesLoaded = false;
		return true;
	}

	g_installPath = root ? root : gLocalInstallPath;
	strcpy(g_szStartDirectory, g_installPath);
	strcat(g_szStartDirectory, g_dictionarySubDir);
	sConfigurationRead = false;
	ShowInfo("CDictManger::Create entered: g_szStartDirectory=%s\n", g_szStartDirectory);

	if (!fillActiveConfiguration()) 
	{
		callLog("\nERROR!:fillActiveConfiguration failed!!\n\n");
		return false;
	}

	if (!activateConfiguration()) 
	{
		ShowInfo("!!Create: activateConfiguration failed!\n");
		return false;
	}
		
	ShowInfo("--Now load UserWordCache: \n");
	allocateSearchStorage();
	fullInitializeWordPaths();
	mAutoCorrect = new AutoCorrect(this);
	ShowInfo("--created mAutoCorrect!\n");
	m_TGraph = new TGraph(mTopDictionary);
	ShowInfo("--created m_TGraph!\n");

	return true;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void CDictManager::mngrCloseAllOpenDictionaries(BOOL justReleaseFlag) 
{
	for (int j = 0 ; j < m_nOrderedDicts; j++) 
	{
		if (m_orderedDictList[j].existingDictionary)
			closeExistingDictionary(j, justReleaseFlag);
	}
	m_nOrderedDicts = 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////
void CDictManager::Destroy() 
{
	ResetTGraph(true);
	mngrCloseAllOpenDictionaries( FALSE);
	g_installPath = NULL;
	releaseSearchStorage();
	CloseLogFile();
	mTopDictionary = NULL;

	delete mUserWordCache;
	mUserWordCache = NULL;

	delete m_pUserWordCacheOffline;
	m_pUserWordCacheOffline = NULL;

	delete mAutoCorrect;
	mAutoCorrect = NULL;

	if(gPhraseEngine)
	{	
		delete gPhraseEngine;
		gPhraseEngine = NULL;
	}

#ifdef WL_SDK
	delete mPhraseEngine;
	mPhraseEngine = NULL;
#endif
}
///////////////////////////////////////////////////////////////////////////////////////////////
char *CDictManager::retrieveWord(char *line, int *nLengthP)
{
	int sofw = 0;
	int lengthOfWord = 0;
	int sIdx;

	*nLengthP = 0;
	// skip leading spaces
	while (SP_CR_TAB(line[sofw]) || line[sofw] == LF)
	{
		sofw++;
	}

	if (line[sofw])
	{
		// scan through the word until hit space, end of line, or return
		sIdx = sofw;
		while (line[sIdx] && NOT_SP_CR_TAB(line[sIdx]) && line[sIdx] != LF) 
		{
			lengthOfWord++;
			sIdx = sofw+lengthOfWord;
		}

		if (lengthOfWord > 0)
		{
			if (line[sIdx])
			{
				// put a string terminator in 
				line[sIdx] = '\0';
			}
			*nLengthP = lengthOfWord;
			return &line[sofw];
		}
	}
	return NULL;
}
///////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __APPLE__
int GetConfigEntry(DictionaryConfigurationEntry *configDictList, char *dictNameP)
{
//	ShowInfo("GetConfigEntry: numExistingDict=%d, dictNameP=%s\n", CDictManager::getNumExistingDictionaries(), dictNameP);
	for(int i=0; i<CDictManager::getNumExistingDictionaries(); i++)
	{
		char *curname = Dictionary::GetDictName(configDictList[i].langIdx);
	//	ShowInfo("--curname=%s for langIdx=%d\n", curname, configDictList[i].langIdx);
		if( strcmp(dictNameP, curname) == 0)
			return i;
	}
	return -1;
}
#endif
///////////////////////////////////////////////////////////////////////////////////////////////
int CDictManager::readUserConfiguration(DictionaryConfigurationEntry *folderDictList, int listSize, BOOL bEnabledOnly)
{
	FILE *stream;
	char myline[200];
	int nLength = 0;

	char *dictNameP = NULL;
	char *priorityP = NULL;
	char *enabledP = NULL;
	DWORD priorityVal = 0;
	bool enabledVal = false;
	int lineNo = 0;
	int curEntry = 0;

	// first disable all dictionaries
	int count = 0;
	while(count < listSize)
		folderDictList[count++].enabled = false;

	const char* dictConfigFilename = GetUserInitFilename();
	ShowInfo("!CDictManager::readUserConfiguration: opening setting file %s \n", dictConfigFilename);
	if ((stream = fopen(dictConfigFilename, "r" )) != NULL )
	{
		while ( (curEntry < listSize) && fgets( myline, 200, stream ) != NULL)
		{
			lineNo++;
			int lineLen  = (int)strlen(myline);
			while (lineLen - 1 >= 0 && (myline[lineLen-1] == '\n' || myline[lineLen-1] == '\r' || myline[lineLen-1]== ' '))
			{
				myline[lineLen-1] = 0;
				lineLen--;
			}
			if(myline[0] == 0)
				break;
			if (myline[0] == '#' || strncmp(myline, "//", 2) == 0)
				continue;

			// split the line up in 3 parts, filename priority enabled/disabled
			dictNameP = retrieveWord(myline, &nLength);
			char *ldictNamep = mytolower(dictNameP, nLength);
			if(strstr(dictNameP, "null")!=0) // if somehow corrupt dictname got into setting ignore it!
				continue;

			if (dictNameP)
				priorityP = retrieveWord(dictNameP+nLength+1, &nLength);
			if (priorityP)
				enabledP = retrieveWord(priorityP+nLength+1, &nLength);

			if (ldictNamep && priorityP && enabledP)
			{
				priorityVal = atoi(priorityP);
				enabledVal =  atoi(enabledP) > 0;
			}

			curEntry = GetConfigEntry(folderDictList, ldictNamep);
			if(curEntry < 0 || curEntry >= listSize)
			{
				ShowInfo("!Warning! dictionary %s does not exist in dictionary folder on this device! or dictionary list capacity %d if full! Ignore it!\n", dictNameP, curEntry);
				continue;
			}
			//if(strstr(ldictNamep, "personal") && enabledVal > 0)
			//	m_personalDictIdx = 0; // personal dictionary disabled,
			folderDictList[curEntry].priority = priorityVal;
			folderDictList[curEntry].enabled = enabledVal;
			folderDictList[curEntry].dictIdxWeight = max(priorityVal, (DWORD)1);
			folderDictList[curEntry].existingDictionary = NULL;
		}
		fclose( stream );
	}

	BYTE numActiveEntries = sortDictionariesBasedOnPriorities(folderDictList, listSize);
	//ShowInfo("--readUserConfiguration finished m_personalDictIdx = %d\n", m_personalDictIdx);
	return numActiveEntries;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CDictManager::mngrGetEnabledDictionaries(DictionaryConfigurationEntry *configDictList, int enabledListSize)
{
	ShowInfo(("CDictManager::mngrGetEnabledDictionaries entered \n"));
	m_nExistingDictionaries = 0;
	// first parse the directory for existing dictionaries.
	if ((m_nExistingDictionaries=readDirectoryConfiguration(configDictList, enabledListSize)) <= 0) // this sets up directory paths
	{
		ShowInfo("mngrGetEnabledDictionaries:readDirectoryConfiguration failed! abort!\n");
		return FALSE;
	}
	ShowInfo("--mngrGetEnabledDictionaries: m_nExistingDictionaries=%d\n", m_nExistingDictionaries);
	// create default dictconfig dictionary if nothing is there
	createDefaultDictionaryConfig(configDictList);
	return readUserConfiguration(configDictList, enabledListSize, FALSE);
}
///////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __APPLE__
void saveConfigFile(DictionaryConfigurationEntry *configDictList, const char *filename, int nDictEntries)
{
	FILE *stream;
	ShowInfo("saveConfigFile: %s \n", filename);
	if ( (stream = fopen(filename, "w" )) != NULL )
	{
		fprintf(stream,"#dictionary priority en/disabled \n");
		for (int i=0; i < nDictEntries; i++)
			fprintf(stream,"%s %d %d\n", Dictionary::GetDictName(configDictList[i].langIdx), configDictList[i].priority, configDictList[i].enabled);
		fclose( stream );
	}
}
#endif
///////////////////////////////////////////////////////////////////////////////////////////////
Dictionary * CDictManager::getDictionaryEnabled(int dictIdx)
{
	Dictionary *existDict = m_orderedDictList[dictIdx].existingDictionary;
	if (existDict && m_orderedDictList[dictIdx].enabled)
		return existDict;
	return NULL;
}
///////////////////////////////////////////////////////////////////////////////////////////////

int CDictManager::setNGramFrom2Grams(CompactNode *end1, CompactNode *end2, MYWCHAR* cmpWord, int minNGram)
{
	return gPhraseEngine->setNGrams(end1, end2, cmpWord, minNGram);
}
/////////////////////////////////////////////////////////////////////////////////////////
int CDictManager::setNGramFrom2Grams(MYWCHAR *word1, MYWCHAR *word2, MYWCHAR* cmpWord, int minNGram)
{
	if(word1 == NULL)
		return 0;
	int dictIdx = 0;
	CompactNode* node1 = wpDictManager->retrieveEndNodeForString(word1, &dictIdx, false);
	if(!node1)
		return 0;
	CompactNode* node2 = NULL;
	if(word2 != NULL)
		node2 = wpDictManager->retrieveEndNodeForString(word2, &dictIdx, false);

	return gPhraseEngine->setNGrams(node1, node2, cmpWord, minNGram);
}

///////////////////////////////////////////////////////////////////////////////////////////////
// return the name of the top dictionary name in list. It avoid personal dict!
char *CDictManager::getTopDictionaryName()
{
	return Dictionary::GetDictName(m_orderedDictList[1].langIdx); // 0 is personal dictionary, so we avoid it!
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// loadUnified means dictionary and phrase file are combined into one! so here we only load the dictionary part!
BOOL CDictManager::openExistingDictionary(int idx) 
{
	ShowInfo(("CDictManager::openExistingDictionary  %s \n"), Dictionary::GetDictName(m_orderedDictList[idx].langIdx));
	m_orderedDictList[idx].existingDictionary =  openExistingDictionary(idx, m_orderedDictList[idx].langIdx);
	bool ret = m_orderedDictList[idx].existingDictionary != NULL;
	m_orderedDictList[idx].enabled = ret;
	return m_orderedDictList[idx].enabled;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Dictionary *CDictManager::openExistingDictionary(int idx, eLanguage elang) 
{
	char *fullPathFileName = createFullPathFileName(Dictionary::GetDictName(elang), LDAT);

	if(!fileAvailable(fullPathFileName))
	{
		ShowInfo("WARNING:openExistingDictionary: %s doesn't exist! Ignore it!\n", fullPathFileName);
		return NULL;
	}
	ShowInfo("openExistingDictionary: opening %s \n", fullPathFileName);
	Dictionary *dict = new Dictionary(elang, idx);
	ShowInfoIf(!dict, "...!!!Failed!!! instantiating the dictionary object@!! Ignore it!!");

	if (dict && !dict->Load(fullPathFileName, TRUE)) 
	{
		delete dict;
		dict = NULL;
	}

	return dict;
}

//////////////////////////////////////////////////////////////////////////////////////////
void CDictManager::LoadCompactPhraseFile(char* filePath)
{
	ShowInfo("Open phrase file: %s\n", filePath);
	FILE* fp = fopen(filePath, "rb");      
	if(!fp)     
	{ 
		ShowInfo("!!!!WARNING!! No File: %s exists!\n", filePath);
		return;    
	}

	if(gPhraseEngine)
	{	
		delete gPhraseEngine;
		gPhraseEngine = NULL;
	}

	gPhraseEngine = new PhraseEngineR();
	gPhraseEngine->init(fp);
	gPhraseEngine->mRoot = gPhraseEngine->loadCompact(fp);

	fclose(fp);
}

#ifdef _WINDOWS
#ifdef WL_SDK
//////////////////////////////////////////////////////////////////////////
void CDictManager::InitializeNGramNode(char* dictTextPath)
{
	ShowInfo("Open phrase file: %s\n", dictTextPath);
	FILE* fp = fopen(dictTextPath, "rb");      
	if(!fp)     
	{ 
		ShowInfo("No Dictionary ngram File: %s\n", dictTextPath);
		return;    
	}

	if(!mPhraseEngine)
		mPhraseEngine = new PhraseEngine();

	mPhraseEngine->init(fp);

	NGramMultiNode stNode;
	int phraseCount = 0;
	int totalPhraseCount = mPhraseEngine->getMaxNumPhraseCount();
	while(fread(&stNode, sizeof(NGramMultiNode), 1, fp) && phraseCount < totalPhraseCount)
	{
		mPhraseEngine->addPhrase(&stNode);
		++phraseCount;
	}

	fclose(fp);
}
#endif
#endif

/////////////////////////////////////////////////////////////////////////
MYWCHAR **NGramMultiNode::getStr(CCompactStore *cstore, int &len)
{
	static MYWCHAR thePhrase[4][MAX_WORD_LEN];
	static MYWCHAR *sPhraseP[4];
	len = wordCount();
	WLBreakIf(len > 4, "!!ERROR! NGramMultiNode::getStr(): we should not have 4 grams or more!\n");
	sPhraseP[3] = NULL;
	for(int i=0; i<len; i++)
	{
		MYWCHAR *str = cstore->getWordFromNode(endNodes[i]);
		mywcscpy(thePhrase[i], str);
		sPhraseP[i] = thePhrase[i];
	}
	sPhraseP[len] = NULL;

	return sPhraseP;
}

/////////////////////////////////////////////////////////////////////////
CompactNode *NGramMultiNode::getEndNode()
{
	int i = 0;
	CompactNode *endnode = endNodes[i++];
	while(endNodes[i])
		endnode = endNodes[i++];
	return endnode;
}
///////////////////////////////////////////////////////////////
void NGramMultiNode::set(NGramMultiNode *cp)
{
	for(int i=0; i<MAX_TGRAPH_HISTORY_NUM; i++)
	{
		endNodes[i] = cp->endNodes[i];
	}
	pref = cp->pref;
}
//////////////////////////////////////////////////////////////
USHORT NGramMultiNode::wordCount()
{
	USHORT count = 0;
	while(count <= MAX_TGRAPH_HISTORY_NUM && endNodes[count]!=NULL)
		count++;
	return count;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void CDictManager::BuildTGraph(MYWCHAR* curWord)
{
	WLBreakIf(m_TGraph == NULL, "!!ERROR! CDictManager::BuildTGraph: TGraph not created!!\n");
	m_TGraph->reset();

	MYWCHAR* curHistory = m_History.ChangeCurrentHistory();
	m_TGraph->set(curHistory, curWord, &m_History);
}

/////////////////////////////////////////////////////////////////////////////////////////////
BOOL CDictManager::openDictionary(char *fullpathname, int listIdx)
{
	ShowInfo("CDictManager::openDictionary: index(%d): fullpath = %s\n", listIdx, fullpathname);
	Dictionary *dict = new Dictionary(Dictionary::SetDictName(fullpathname), listIdx);
	if (dict) 
	{
		if (dict->Load(fullpathname, TRUE, true))
		{
			m_orderedDictList[listIdx].existingDictionary = dict;
			m_orderedDictList[listIdx].priority = 1;
			m_orderedDictList[listIdx].enabled = true;
			m_nOrderedDicts = listIdx+1;
			return TRUE;
		}
		else
		{
			ShowInfo("...failed to load dict(%s)!! ignore it!!\n", fullpathname);
			delete dict;
		}
	}
	m_orderedDictList[listIdx].enabled = false;
	return FALSE;
}
//////////////////////////////////////////////////////////////////////////////////////////
BOOL CDictManager::closeExistingDictionary(int idx, BOOL justReleaseFlag) 
{
	ShowInfo(("CDictManager::closeExisting dictionary %s \n"), Dictionary::GetDictName(m_orderedDictList[idx].langIdx));
	m_orderedDictList[idx].existingDictionary->Destroy(justReleaseFlag);
	m_orderedDictList[idx].existingDictionary = NULL;
	m_orderedDictList[idx].enabled = false;
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CDictManager::GetDictId(const char* name)
{
	for(int i=0; i< m_nExistingDictionaries; i++)
	{
		const char* dictname = Dictionary::GetDictName(m_orderedDictList[i].langIdx);
		if(strstr(name, dictname) != NULL)
			return i;
	}
	return -1;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
int CDictManager::getDictPriority(const char *name)
{
	int dictIdx = GetDictId(name);
	if(dictIdx >= 0 && m_orderedDictList[dictIdx].enabled)
		return m_orderedDictList[dictIdx].priority;
	ShowInfo("CDictManager::getDictPriority: dict %s: dictId=%d, enabled=%x\n", name, dictIdx, getDictionaryEnabled(dictIdx));
	return -1;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CDictManager::getDictBuildNumber(int dictIdx)
{
	WLBreakIf((dictIdx < 0 || dictIdx >= m_nExistingDictionaries), "!!ERROR! getDictBuildNumber: dictIdx not valid!\n");
	if(m_orderedDictList[dictIdx].existingDictionary)
		return m_orderedDictList[dictIdx].existingDictionary->getBuildNumber();
	return -1;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CDictManager::getDictVersionNumber(int dictIdx)
{
	WLBreakIf((dictIdx < 0 || dictIdx >= m_nExistingDictionaries), "!!ERROR! getDictVersionNumber: dictIdx not valid!\n");
	if(m_orderedDictList[dictIdx].existingDictionary)
		return m_orderedDictList[dictIdx].existingDictionary->getVersionNumber();
	return -1;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CDictManager::createDefaultDictionaryConfig(DictionaryConfigurationEntry *enabledRegistryDictList)
{
	FILE *stream = NULL;
	
	//makeUserIniFilename(dictConfigFilename, MAX_PATH);
	const char* dictConfigFilename = GetUserInitFilename();
	ShowInfo("createDefaultDictionaryConfig:: opening user init file %s \n", dictConfigFilename);
	if ((stream = fopen(dictConfigFilename, "r" )) == NULL )
	{
		ShowInfo(".....file doesn't exist. Create a default!\n");
		saveConfigFile(enabledRegistryDictList, dictConfigFilename, m_nExistingDictionaries);
	}
	else
		fclose( stream );
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This can be called two occasions, first of course when initializing
// Second when the user changed options so that the dictionaries have been 
// changed priority of the dictionaries
BOOL CDictManager::fillActiveConfiguration()
{
	ShowInfo(("CDictManager::fillActiveConfiguration entered \n"));

	// we can fill up as much as we like as long as we don't use MAX_NUM_DICTS dictionaries at once
	DictionaryConfigurationEntry configDictList [MAX_NUM_DICTS];
	int numEnabled = mngrGetEnabledDictionaries(configDictList, MAX_NUM_DICTS);

	if (m_nExistingDictionaries <= 0 || m_nExistingDictionaries!= numEnabled)
	{
		ShowInfo("!!Warning!!fillActiveConfiguration: no dictionary found present! or nothing active! skip setting up dictionaries! (nExisti)%d= (nEnabled)%d\n", m_nExistingDictionaries, numEnabled);
		return FALSE;
	}

	memcpy(m_orderedDictList, configDictList , m_nExistingDictionaries * sizeof(DictionaryConfigurationEntry));	
	return TRUE;
}
///////////////////////////////////////////////////////////////////////////////////////////////
void CDictManager::reset() 
{ 
	goToNextWord((char*)"CDictManager::reset", true); 
}
///////////////////////////////////////////////////////////////////////////////////////////////
void CDictManager::fullReset() 
{
	fullInitializeWordPaths();
	sWordPosInPhrase = 0;
	sEndofSentenceReached = true;
}

///////////////////////////////////////////////////////////////////////////////////////////////
bool CDictManager::resetConfiguration()
{
	ShowInfo("CDictManager::ResetConiguration called: m_nExistingDictionaries=%d\n", m_nExistingDictionaries);
	saveConfigFile(m_orderedDictList, GetUserInitFilename(), m_nExistingDictionaries);

	// wipe all the dictionaries out of the keyboard memory and start loading what is needed
	mngrCloseAllOpenDictionaries(TRUE);
	//m_personalDictIdx = INVALID_DICT_IDX;
	sConfigurationRead = false;
	if (!fillActiveConfiguration())
	{
		ShowInfo("--!!!!resetConfiguration:fillActiveConfiguration failed! m_nOrderedDicts=%d\n", m_nOrderedDicts);
		return FALSE;
	}

	if (!activateConfiguration())
	{
		ShowInfo("--!!!resetConfiguration:activateConfiguration failed!\n");
		return FALSE;
	}

	allocateSearchStorage();
	fullInitializeWordPaths();
	
	wipeoutBreadCrumbs();
	//ShowInfo("---CDictManager::ResetConiguration m_noDictionariesLoaded = %d, m_personalDictIdx = %d\n", m_noDictionariesLoaded, m_personalDictIdx);
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// loads enabled dictionaries
bool CDictManager::activateConfiguration()
{
	bool oneSuccessfullLoad = FALSE;
	ShowInfo("CDictManager::activeConfiguration entered: m_nOrderedDicts = %d\n", m_nOrderedDicts);

	eLanguage elang = eLang_NOTSET;
	for (int i = 0 ; i < m_nOrderedDicts; i++)
	{
		ShowInfo(("--Dictmanager::assessing [%i]: lang=%d, prio=%d, enab=%d, name=%s, existing=%d \n"),i, m_orderedDictList[i].langIdx,
			m_orderedDictList[i].priority, m_orderedDictList[i].enabled, Dictionary::GetDictName(m_orderedDictList[i].langIdx), m_orderedDictList[i].existingDictionary != NULL);
		WLBreakIf (m_orderedDictList[i].existingDictionary, "---!!!ERROR!! activateConfiguration::why is this dictionary still open!?\n");

		char *fullPathFileName = NULL;
		if (m_orderedDictList[i].enabled)
		{
			oneSuccessfullLoad |= openExistingDictionary(i);

			if(elang == eLang_NOTSET && (m_orderedDictList[i].langIdx== eLang_ENGLISH || m_orderedDictList[i].langIdx== eLang_KOREAN 
				||  m_orderedDictList[i].langIdx== eLang_SPANISH || m_orderedDictList[i].langIdx== eLang_RUSSIAN || m_orderedDictList[i].langIdx== eLang_FRENCH))
			{
				if(!PhraseEngineR::isInitialized())
				{
					elang = m_orderedDictList[i].langIdx;
					fullPathFileName = createFullPathFileName(Dictionary::GetDictName(elang), LDAT);
					LoadCompactPhraseFile(fullPathFileName);

#ifdef MINEWIN32 		//testing: this crashes in release in testbed! seems some data still not initialed! 
						//turn this into unit testing per dict for each feature! asap!
					{	
						allocateSearchStorage();

						int phraseCount = setNGramFrom2Grams(L"I", NULL); // testing phrase prediction
						phraseCount = gPhraseEngine->setNGramsBasedOnPartGram(L"ho"); // testing phrase from part of word prediction
						ShowInfo("All phrases starting with #all#: %d\n", phraseCount);
						int len = 0;
						for(int k=0; k<phraseCount; k++)
						{
							PhraseNode *phNode =  &PhraseEngineR::gPhrases[k];
							MYWCHAR **words = phNode->getStr(len);
							ShowInfo("Phrase: %d: %s %s %s, pref=%d\n", k, toA(words[0]), toA(words[1]), toA(words[2]), phNode->pref);
						}

						// testing phrase to phrase prediction
						phraseCount = setNGramFrom2Grams(L"the", NULL);
						for(int i=0; i<phraseCount; i++)
						{
							PhraseNode *phNode =  &PhraseEngineR::gPhrases[i];
							MYWCHAR** words = phNode->getStr(len);
							ShowInfoIf(len>0,"Phrase:len(%d): %s %s %s, pref=%d\n", len, toA(words[0]), toA(words[1]), toA(words[2]), phNode->pref);
							int nextPhCount = 0;
							PhraseNode *nextPhrase = phNode->getNextPhrase(nextPhCount++);
							while(nextPhrase)
							{
								words = nextPhrase->getStr(len);
								ShowInfoIf(len>0,"NextPhrase:len(%d): %s %s %s, pref=%d\n", len, toA(words[0]), toA(words[1]), toA(words[2]), nextPhrase->pref);
								nextPhrase = phNode->getNextPhrase(nextPhCount++);
							}
						}
					}
#endif
				//	LoadLearnedNGrams(fullPathFileName, N3Gram);
				//	LoadLearnedNGrams(fullPathFileName, N4Gram);
				}
			}
		}
	}
	
	if (!oneSuccessfullLoad)
	{
		ShowInfo(("!!!WARNING! activateConfiguration:Cannot load any dictionaries!, #dicts %d, installPath %s byebye!! \n"), m_nOrderedDicts, g_installPath);
		m_noDictionariesLoaded = true;
	}
	else
		m_noDictionariesLoaded = false;

#ifdef _DEBUG
	if (oneSuccessfullLoad) 
	{
		for (int s = 0 ; s < m_nOrderedDicts; s++) 
		{
			ShowInfo(("--langIdx:%d, name %s ,  %s , enabled:%d, prio:%d \n"), m_orderedDictList[s].langIdx, Dictionary::GetDictName(m_orderedDictList[s].langIdx),
				(m_orderedDictList[s].existingDictionary ? "opened" : "closed"), m_orderedDictList[s].enabled, m_orderedDictList[s].priority);
		}
	}
#endif

	setTopDictionary();
	if(mUserWordCache)
	{
		delete mUserWordCache;
		mUserWordCache = NULL;
	}

	if(m_pUserWordCacheOffline)
	{
		delete m_pUserWordCacheOffline;
		m_pUserWordCacheOffline = NULL;
	}
		
	mUserWordCache = new UserWordCache();
	m_pUserWordCacheOffline = new UserWordCacheOffline();

	ResetTGraph(true);
	m_NGramLearning = new NGramLearning();
	m_History.NGramHistoryReset();

	return oneSuccessfullLoad;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
DictionaryConfigurationEntry *CDictManager::GetOrderedDict(int idx) 
{ 
	//ShowInfo("GetOrderedDict: m_nOrderedDicts=%d, idx=%d\n", m_nOrderedDicts, idx);
	if(idx<0 || idx >= m_nExistingDictionaries) 
		return NULL; 
	return &m_orderedDictList[idx]; 
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CDictManager::SetOrderedDict(int langidx, int priority, bool enabled)
{
	ShowInfo("CDictManager::SetOrderedDict: langidx=%d, prio=%d, enabled=%d\n", langidx, priority, enabled);
	if(langidx<0 || langidx>=eLang_COUNT) 
	{
		ShowInfo("SetOrderedDict: ERROR! langidx %d not valid: CDictManager::SetOrderedDict.\n",langidx);
		return false; 
	}

	int idx = -1;
	int i = 0;
	for(; i<m_nExistingDictionaries; i++)
	{
		if(m_orderedDictList[i].langIdx == langidx)
		{
			idx = i;
			break;
		}
	}
	if(idx == -1)
		idx = i;
	if(idx >= MAX_NUM_DICTS)
	{
		ShowInfo("!!WARNING!!SetOrderedDict: Setting out of range of active dictionary counts! Ignore it!!\n");
		return false;
	}

	m_orderedDictList[idx].priority = priority;
	m_orderedDictList[idx].enabled = enabled;
	ShowInfo(("--idx:%d, [%d]= %s, enabled:%d, priority:%d \n"), idx, m_orderedDictList[idx].langIdx, Dictionary::GetDictName(m_orderedDictList[idx].langIdx),
		m_orderedDictList[idx].enabled, m_orderedDictList[idx].priority);
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
#ifndef __APPLE__
char *createFullPathFileName(const char *fileName, char *ext) 
{
	ShowInfo("createFullPathFileName: fileName=#%s#, gHistWordIdx=%d: Extension: %s\n", fileName, gHistWordIdx, ext);
	static char sFullPathName[1024];
	memset(sFullPathName, 0, 1024);
	if (g_installPath == NULL)
	{
		ShowInfo("!!ERROR!!readDirectoryConfiguration need to be called first!!\n");
		return NULL;
	}

	sprintf(sFullPathName, ("%s/%s"), g_szStartDirectory, fileName);

	// add extention if any, and if it is not already there!
	if(ext != NULL)
	{
		int full_len = strlen(sFullPathName);
		int ext_len = strlen(ext);
		if((full_len + ext_len) < 1024 && !strstr(sFullPathName, ext))
			strcat(sFullPathName, ext);
	}

	return sFullPathName;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE sortDictionariesBasedOnPriorities(DictionaryConfigurationEntry *configDict, int nActiveDicts)
{
	ShowInfo("sortDictionariesBasedOnPriorities: nActiveDicts = %d\n", nActiveDicts);
	DictionaryConfigurationEntry priorityRanking[MAX_NUM_DICTS]; // = (DictionaryConfigurationEntry *)calloc( 1, arrsize);
	bool visited[MAX_NUM_DICTS];
	for(int i=0; i<MAX_NUM_DICTS; i++) visited[i]= false;

	// scan fist for the occurence of the personal dictionary, if it exists
	// it will have the highest priority 0 no matter what. It will also appear in the 
	// list first and users cannot change its priority
	BYTE curPriority = 0;
	for (int k=0; k < CDictManager::getNumExistingDictionaries(); k++)
	{
		if (configDict[k].langIdx == eLang_PERSONAL) 
		{
			priorityRanking[0] =configDict[k];
			priorityRanking[0].priority = curPriority;
			priorityRanking[0].dictIdxWeight = max(1, curPriority);
			curPriority++;
			visited[k] = true;
			break;
		}
	}

	// do it for the rest of the dictionaries
	for (int k=0; k < CDictManager::getNumExistingDictionaries(); k++)
	{
		BYTE highestPriority = 0xfe;
		int highestIdx = -1;

		for (int n = 0; n < CDictManager::getNumExistingDictionaries() ; n++)
		{
			if (configDict[n].priority  < highestPriority && !visited[n] && configDict[n].priority > 0 && configDict[n].enabled)
			{
				highestPriority = configDict[n].priority;
				highestIdx = n;
			}
		}

		if (highestIdx >= 0)
		{
			priorityRanking[curPriority] = configDict[highestIdx];
			priorityRanking[curPriority].priority = curPriority;
			priorityRanking[curPriority].dictIdxWeight = max(1, curPriority);
			visited[highestIdx] = true; // scratch this entry out
			curPriority++;
		}
		else
			break;
	}
	
	CDictManager::SetNumOrderedEnabledDictionaries(curPriority);

	// now copy the rest of dictionaries, which are not enabled, to priority ranking:
	for(int i=0; i<CDictManager::getNumExistingDictionaries(); i++)
	{
		bool found = false;
		for(int j=0; j<curPriority; j++)
		{
			if(priorityRanking[j].langIdx == configDict[i].langIdx)
			{
				configDict[i].priority = curPriority;
				found = true;
				break;
			}
		}
		if(!found)
			priorityRanking[curPriority++] = configDict[i];
	}

	memcpy(&configDict[0], &priorityRanking[0], sizeof(DictionaryConfigurationEntry)*CDictManager::getNumExistingDictionaries());
#if 0 //_DEBUG
	ShowInfo("\n");
	for(int i=0; i < CDictManager::getNumExistingDictionaries(); i++)
	{
		ShowInfo("AfterSort: %d: langIdx=%d, prio=%d, name=%s\n", i, configDict[i].langIdx, configDict[i].priority, Dictionary::GetDictName(configDict[i].langIdx));
	}
#endif
	return curPriority;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// sets up directory path and parses it for existing dictionaries
int readDirectoryConfiguration(DictionaryConfigurationEntry *configDictList, int listSize)
{
	ShowInfo("ReadDirectoryConfiguration: setting up install path and reading content of dictionary folder: sConfigurationRead=%d, g_installPath=%s\n", sConfigurationRead, g_installPath);
	if(!sConfigurationRead)
	{
		if(configDictList == NULL)
		{
			listSize = (MAX_NUM_DICTS*3);
			configDictList = (DictionaryConfigurationEntry *) calloc(listSize, sizeof(DictionaryConfigurationEntry));
		}
	}

	ShowInfo("readDirectoryConfiguration: m_installPath %s \n", g_installPath);
	ShowInfo("readDirectoryConfiguration: g_szStartDirectory %s \n", g_szStartDirectory);
	ShowInfo("readDirectoryConfiguration: m_currentVersion %s \n", g_currentVersion);

	//first set slot 0 for personal
	int slot = 0;
	configDictList[slot].priority = 0;
	configDictList[slot].langIdx = eLang_PERSONAL;
	configDictList[slot].enabled = TRUE;  // enabled by default. Can be disabled from setting
	configDictList[slot].existingDictionary = NULL;
	slot++;

#ifdef _WINDOWS
#define FILENAMELEN ent->d_namlen
#else
#define FILENAMELEN (strlen(ent->d_name))
#endif
	///////////////////////////////////////////////////////////////
	////////////////////////////// try this method on iOS to see if it works for iOS:
	/*#include <dirent.h>
	struct dirent **namelist;

    int n;

    n = scandir(g_szStartDirectory, &namelist, 0, alphasort); 

    if (n < 0) 
        perror("scandir"); 
    else { 

        while(n--) { 
            printf("%s\n", namelist[n]->d_name); 
            free(namelist[n]); 
        } 

        free(namelist); 
    } 
	*/
	//////////////////////////////////////////////////////////////////
	// now read what's in dictionary folder
	BOOL bListFull = slot >= listSize;
	DIR *dir = opendir(g_szStartDirectory);
	ShowInfo("Parsing dictionary folder %s for dictionaries! %d\n", g_szStartDirectory, dir!= NULL);
	if(dir != NULL)
	{
		struct dirent *ent = readdir(dir);
		while(! bListFull && (ent != NULL))
		{
			if(	ent->d_type == DT_REG && FILENAMELEN > strlen(LDAT)		&& 
				!strcmp(ent->d_name + FILENAMELEN - strlen(LDAT), LDAT)	&&
				strcmp(ent->d_name, "personal.ldat") != 0 )
			{
					eLanguage langIdx = Dictionary::SetDictName(ent->d_name);
					if(langIdx == eLang_DEFAULT)
						ShowInfo("--dictionary %s is not in our list! We load it as loose!\n", ent->d_name);
					configDictList[slot].priority = slot;
					configDictList[slot].langIdx = langIdx;
					configDictList[slot].existingDictionary = NULL;
					configDictList[slot].enabled = TRUE; // enable them all by default! to avoid case of single dict like german not being enabled!
					ShowInfo("--Dictionary %s available! Added it to list! langIdx=%d, priority=%d\n", ent->d_name, configDictList[slot].langIdx, configDictList[slot].priority);
					slot++;
					bListFull = (slot >= listSize);
			}
			ent = readdir(dir);
		}
		closedir(dir);
	}
	else
	{
		ShowInfo("ERROR!! Couldn't open folder %s ??\n", g_szStartDirectory);
		return 0;
	}

	sConfigurationRead = true;
	return slot;
}
//////////////////////////////////////////////////////////////////////////////////////////////
void createUrlConfigFilename(char *userFilenamePath, char *szUserFilename)
{
	strcpy(userFilenamePath, g_szStartDirectory);
	//mywcscat(userFilenamePath, "\\");
	strcat(userFilenamePath, szUserFilename);
}
#endif
/////////////////////////////////////////////////////////////////////////////////////////////////
const char *CDictManager::GetUserInitFilename()
{
	static char sUserInitFilename[MAX_PATH];
	static bool sInitialized = false;
	if(!sInitialized)
	{
		memset(sUserInitFilename, 0, MAX_PATH);
		strcpy(sUserInitFilename, g_szStartDirectory);
		strcat(sUserInitFilename, "/settings.wlkb");
		sInitialized = true;
	}
	return sUserInitFilename;
}



