// <copyright file="userWordCache.cpp" company="WordLogic">
// Copyright (c) 2000, 2013 WordLogic Corporation All Right Reserved, http://www.wordlogic.com/
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
// <summary>This file contains class UserWordCache which is the structure responsile for caching recently used words and providing interface for searching them.</summary>

#include "StdAfx.h"
#include "userWordCache.h"
#include "wordpath.h"
#include "dictmanager.h"
#include "compactstore.h"
#include "compactdict.h"
#include "wordpunct.h"
#include "parsedline.h"

#ifdef _WINDOWS
#include <io.h>
#else
#include <stdio.h>
#endif
#include <fcntl.h>
/****************************************************************************************************
 * UsedWord cache class. This class is used for various user related words usage. This includes,
 * but not restricted to new words management, user frequently used words and phrases management, graduating
 * words to active dictionary management. This class is supposed to only keep around 200 or something elements, 
 * so be very efficient in terms of performance and space as longs as capacity stays low.
 *****************************************************************************************************/
extern CDictManager *wpDictManager;
BYTE  UserWordCache::mFlush_freq_threshold = 2;
WLArray UserWordCache::sCacheAr;
WLArray  UserWordCache::sQueueAr;
WordEntry * sFlushableWords[sMaxUpdatesBeforeSaveOut];
static MYWCHAR *sLearnedPredictions[MAX_NUM_PREDICTIONS+1];
static MYWCHAR *sLearnedNextWords[MAX_NUM_PREDICTIONS+1];
BYTE UserWordCache::sTotalWordAddedSinceLastCheck = 0; // counts the number of flushable new words currently in buffer ready to be flushed to personal dict.
//MYWCHAR UserWordCache::sFlushedNewWords[MAX_NUM_FLUSH_PER_CALL][24];
static BYTE sFlushableNewWordsCount = 0;
USHORT UserWordCache::sLatestAccess;

static char sCacheFileName[1024];// = "cacheBK.txt";

void constructCurrentCacheFilename()
{
	char *dictname = wpDictManager->getTopDictionaryName();
	WLBreakIf(dictname == NULL || isEmptyStr(toW(dictname)), "!!ERROR! cache file name cannot be created!\n");
	strcpy(sCacheFileName, dictname);
	strcat(sCacheFileName, "Cache.txt");

}

/////////////////////////////////////////////////////////////////////////////////////////////
UserWordCache::UserWordCache()
{
	mMax = MRUWORDSINCACHEMAX;
	mNumWordsInCache = 0;
	sLatestAccess = 0;
	for (int i = 0; i < QUEUESIZE; i++)
		mWordQueues[i] = NULL;

	int cachewordLen = sizeof(WordEntry);
	sCacheAr.set(mMax, cachewordLen);
	sQueueAr.set(2*(QUEUESIZE+mMax), max(sizeof(CacheQueue), sizeof(QueueEntry)));
	mLastWord = NULL;
	mMRUQueue = new(sQueueAr.next()) CacheQueue(false);
	// load cache back up if it exists
	constructCurrentCacheFilename();

	char* filename = createFullPathFileName(sCacheFileName);
	ShowInfo("UserWordCache: loading cache file %s\n", filename);
	FILE *fp = fopen(filename, "r");
	if(fp)
	{
		ShowInfo("UserWordCache Opened cache file: now try to load it!\n");
		MYWCHAR aLine[MAX_WORD_LEN];
		// reset getLine structures
		bool unicodeFlag = true;
		getLine(0, NULL, unicodeFlag);
		CParsedLine   *lineParser = new CParsedLine((char*)"spo", 0, SP);
		eEndianNess endianflag = eLITTLE_ENDIAN32;
		if(unicodeFlag) // this is a unicode file. Try to read first 2 byte for BOM (Byte Order Marker)
		{
			byte bom[2];
			fread(bom, 1, 2, fp);
			if(bom[0] == 0xff && bom[1] == 0xfe)
				endianflag = eLITTLE_ENDIAN32;
			else if(bom[0] == 0xfe && bom[1] == 0xff)
				endianflag = eBIG_ENDIAN32;
			else // none of the above. So it doesn't have BOM, go back 2 byte on file pointer
				fseek(fp, 0, SEEK_SET);
		}

		while (getLine(fp,  aLine, unicodeFlag))
		{
		//	ShowInfo("MK UserWordCache Processing line: %s \n", toA(aLine));
			lineParser->ScanCache(aLine);
		//	ShowInfo("MK UserWordCache add cache: %s %d  %d\n", toA(lineParser->m_word), lineParser->m_pref, lineParser->m_occurance);
			loadWord(lineParser->m_word, lineParser->m_pref, 0, mywcslen(lineParser->m_word));
			//ProcessCacheLine(szWOneLine);
		}
		fclose(fp);
		if(lineParser)
			delete lineParser;
	}
	else
		ShowInfo("\n!!WARNING!! UserWordCache: cache file does not exist in dictionary folder!\n\n");

//	InitializeCurrentCaches(filename);	
}
//////////////////////////////////////////////////////////////////////////////////////////////
void UserWordCache::InitializeCurrentCaches(char* filename)
{
	FILE *fp = fopen(filename, "r");
	char myline[MAX_WORD_LEN];
	memset(myline,0,sizeof(myline));

	if ((fp = fopen(filename, "r" )))
	{
	 
		while (fgets( myline, MAX_WORD_LEN, fp ) != NULL)
		{
			//ShowInfo("\n!!UserWordCache myLine: %s\n", myline);
			int lineLen  = (int)strlen(myline);
			CacheWords stWords;

			while (lineLen - 1 >= 0 && (myline[lineLen-1] == '\n' || myline[lineLen-1] == '\r' || myline[lineLen-1]== ' '))
			{
				myline[lineLen-1] = 0;
				lineLen--;
			}
			
			char *token = strtok(myline, " ");
			MYWCHAR* pData = NULL;

			if(token)
			{
				pData  = makeWord(toW(token));
				if(!isNumber(pData))
				{
					mywcscpy(stWords.word,pData);
				}
			}
			
			bool bGetPref = false;
			while (token) 
			{
				token = strtok(NULL, " ");
				if(token)
				{
					pData  = makeWord(toW(token));
					if(isNumber(pData))
					{
						char* szData = toA(pData);
						if(!bGetPref)
						{
							stWords.pref = atoi(szData);
							bGetPref = true;
						}
						else
						{
							stWords.freq = atoi(szData);
						}	
					}
				}
			}

			int len = mywcslen(stWords.word);
			if(len > 0)
			{
				loadWord(stWords.word,stWords.pref,stWords.freq, len);
			}
		}

		fclose(fp);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////
void UserWordCache::ProcessCacheLine(MYWCHAR *line)
{
	int len = mywcslen(line);
	if(SP_CR_TAB(line[len-1]))
		line[len-1] = NUL;

	ShowInfo("ProcessCacheLine: add line #%s# to cache\n", toA(line));
	if(line && line[0] != NUL && line[0] != '#')
	{
		trimEndingSpaces(line);
		if(containsSPs((MYWCHAR*)line) > 0) // so its content is like "word pref" at least!
		{
			MYWCHAR word[36];
			int pref = 0;
			int latestAccess = 0;
			int curSP =0;
			int prevSP =0;
			///////// extract word
			while(NOT_SP_CR_TAB(line[curSP])) 
				curSP++;
			mywcsncpy(word, &line[prevSP], curSP-prevSP);
			///////// extract pref
			prevSP =++curSP;
			while(NOT_SP_CR_TAB(line[curSP])) 
				curSP++;
			MYWCHAR tmpSt[16];
			mywcsncpy(tmpSt, &line[prevSP], curSP-prevSP);
			pref = atoi(toA(tmpSt));
			///////// extract latestAccess
			prevSP =++curSP;
			while(line[curSP] && NOT_SP_CR_TAB(line[curSP])) 
				curSP++;
			MYWCHAR tmpSt1[16];
			mywcsncpy(tmpSt1, &line[prevSP], curSP-prevSP);
			latestAccess = atoi(toA(tmpSt1));
			ShowInfo("--word=%s, pref=%s, access=%s\n", toA(word), toA(tmpSt), toA(tmpSt1));
			//swscanf((const wchar_t*)line, L"%s %d %d", word, &pref, &latestAccess);
			loadWord(word, pref, latestAccess, mywcslen((MYWCHAR*)word));
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
void UserWordCache::loadWord(MYWCHAR *word, BYTE pref, USHORT access, int len)
{
	if(mNumWordsInCache == mMax)
	{
		ShowInfo("!!WARNING! UserWordCache::loadWord cache array is full while loading cache file at max = %d line, so ignore rest of file!\n", mMax);
		return;
	}
//	if(mywcscmp(word, toW("the"))==0)
//		printf("test is the!\n");
//	ShowInfo( "putword #%s# pref=%d access=%d , initialLoad=%d\n", toA(word), pref, access, initialLoad );
	CacheQueue *wordQueue = pickWordQueue(word[0], TRUE);
	QueueEntry *wordQueueEntry = wordQueue->findEntry(word, len);

	sLatestAccess = max(access, sLatestAccess);
	if (wordQueueEntry)
	{
		// entry exists meaning it also exists in the mru queue, touch the entry so that it updates its position in the mMRUQueue
		QueueEntry *mru_entry = wordQueueEntry->getQueueEntryValue();
		WLBreakIf(!mru_entry , "!!ERROR! loadWord: wordentry for lookup word #%s# is NULL!\n", toA(word));
		WordEntry *we = mru_entry->getQueueEntryValue()->getWordEntryValue();		
		WLBreakIf(!we || we->text[0] != word[0], "!!ERROR! loadWord: lookup word #%s# is not the same as #%s#!\n", toA(we->text), toA(word));
		assert(we->bufferIdx != 0xff); // investigate this! this happens aftern detaching this word previously!
	} 
	else 
	{
		int dictIdx = 0;  // we don't want already existing words in any dict make it back in cache! so avoid cache bloat!
		if(wpDictManager->retrieveEndNodeForString(word, &dictIdx, true))
			return;
		WordEntry *wep = allocateWordEntry(word, pref, sLatestAccess, len, true);
		mNumWordsInCache++;
		QueueEntry *mruqe = new(sQueueAr.next()) QueueEntry();
		wordQueueEntry = new(sQueueAr.next()) QueueEntry();

		wordQueueEntry->setValues(mruqe, (void *) wep);
		mruqe->setValues(wordQueueEntry, (void *) wordQueue);

		mMRUQueue->insertEntry(mruqe);
		wordQueue->insertEntry(wordQueueEntry);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
// find at most 5 most probable word completion prediction based on input text. Predictions are
// based on learned user behavior recorded in cache. A preference for each predictions is
// computed and returned. Also the total number of predictions is returned too. 
/////////////////////////////////////////////////////////////////////////////////////////////
MYWCHAR** UserWordCache::findPredictionsStartingWith(MYWCHAR *text, USHORT *outputprefs, int *count)
{
	*count = 0;
	CacheQueue *ltrq = pickWordQueue(text[0], FALSE);
	if (ltrq) 
	{  //ltrq->printMatchingWords(text);
		memset(sLearnedPredictions, 0, sizeof(sLearnedPredictions));
		*count = ltrq->findPredictionsStartingWith(text, outputprefs);
		return sLearnedPredictions;
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////
bool UserWordCache::CacheQueue::ContainInPredictions(WCHAR* text)
{
	for(int i=0; i< MAX_NUM_PREDICTIONS; i++)
	{
		if(!sLearnedPredictions[i] || !text || text[0] == NUL)
			break;
		if(mywcscmp(sLearnedPredictions[i], text)==0)
			return true;
	}
	return false;

}

//////////////////////////////////////////////////////////////////////////////////////////
//finds next most frequent word entry in this wordqueue. This function only applies to wordqueues!
/////////////////////////////////////////////////////////////////////////////////////////////////////
WordEntry * UserWordCache::CacheQueue::getNextMostFrquentEntry(BYTE topFreq)
{
	assert(mIsLtrQueue==true);
	QueueEntry *qe = mFirst;
	while ( qe != NULL) 
	{
		WordEntry *wep = qe->getWordEntryValue();
		WLBreakIf(!wep, "!!ERROR! CacheQueue::getNextMostFrquentEntry: wep is null!\n");
		if(wep->freq > topFreq)
		{
			if(!ContainInPredictions(wep->text))
				return wep;
		}
		qe = qe->mNext;
	}
	return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////
// searches the cache to give a list of top words which follow the input word.
// output:  wchar* array is filled with possible next word candidates.
/////////////////////////////////////////////////////////////////////////////////////////////
MYWCHAR **UserWordCache::GetPossibleFollowWords(MYWCHAR *word, int len, int *count)
{
	int num = 0;
	QueueEntry* wqe = findWordEntry(word, len);
	if(wqe)
	{
		memset(sLearnedNextWords, 0, sizeof(sLearnedNextWords));
		WordEntry *wep = wqe->getWordEntryValue();
		for(int i=0; i<MAX_NUM_PREDICTIONS; i++)
		{
			if(wep->nextWords[i].idx != 0xFF)
			{
				WordEntry *follow = (WordEntry *)sCacheAr.getElementAt(wep->nextWords[i].idx);
				if(follow==NULL || follow->text[0]==NUL)
				{
					ShowInfo("!!WARNING! UserWordCache::GetPossibleFollowWords: follow word after (%s) is empty!\n", toA(word));
					wep->nextWords[i].idx = 0xFF;
				}
				else
			    {
					sLearnedNextWords[num++] = follow->text;
					ShowInfo("--GetPossibleFollowWords: next word:[%d] #%s#\n", num-1, toA(follow->text));
				}
			}
		}
		*count = num;
		return sLearnedNextWords;
	}
	*count = 0;
	return NULL;
}

// find the corresponding word queue out of the list based on starting letter given! 
// If not created yet, bcreate directs the function to create the queue.
/////////////////////////////////////////////////////////////////////////////////////////////
UserWordCache::CacheQueue *UserWordCache::pickWordQueue(MYWCHAR letter, BOOL bCreate) 
{
	int queueIndex = 0;
	MYWCHAR firstLetter = letter;
	if (firstLetter >= (MYWCHAR)'A' && firstLetter <= (MYWCHAR)'Z')
		queueIndex = firstLetter - (MYWCHAR)'A';
	else if (firstLetter >= (MYWCHAR)'a' && firstLetter <= (MYWCHAR)'z')
		queueIndex = 26 + firstLetter - (MYWCHAR)'a';
	else if (firstLetter >= (MYWCHAR)'0' && firstLetter <= (MYWCHAR)'9')
		queueIndex = 52;
	else
		queueIndex = QUEUESIZE-1; // for other latin based languages this index represents a few lettes missing from english alphabet like diacritics in french

	if (mWordQueues[queueIndex] == NULL && bCreate)
	{
		mWordQueues[queueIndex] = new(sQueueAr.next()) CacheQueue(true);
	}
	return mWordQueues[queueIndex];
}

/////////////////////////////////////////////////////////////////////////////////////////////
UserWordCache::QueueEntry *UserWordCache::findWordEntry(MYWCHAR *word, int len)
{
	CacheQueue *q = pickWordQueue(word[0], FALSE);
	if(q)
		return q->findEntry(word, len);
	return NULL;
}
///////////////////////////////////////////////////////////////////////////////////////
void UserWordCache::RoundupAccessCounts()
{
	for (int i = 0; i < QUEUESIZE; i++)
	{
		if (mWordQueues[i])
		{
			QueueEntry *qe = mWordQueues[i]->getFirst();
			while ( qe != NULL) 
			{
				WordEntry *wep = qe->getWordEntryValue();
				if(wep)
					wep->accessed = wep->accessed << 1;
				qe = qe->mNext;
			}
		}
	}
	sLatestAccess = sLatestAccess << 1;
}

////////////////////////////////////////////////////////////////////////////////////
// Searches the cache for "matured" new words to be graduated to current active dictionary. 
// Once a word has been graduated, it still stays in cache. 
void UserWordCache::graduateToDictionaries()
{
	ShowInfo("graduateToDictionaries, gHistWordIdx=%d\n", gHistWordIdx);
	static NextWordInfo sNextWords[MAX_NUM_PREDICTIONS];
#if defined(DEBUG)
	CompactNode *cNode = wpDictManager->getCompactStore(1)->getAllocatedFirstNode();
	ShowInfo("graduateToDictionaries:Number of words Before = %d\n",wpDictManager->getCompactDict(1)->getNumWords(cNode));
#endif

	bool memorylayoutchanged = false;
	for (int i = 0; i < QUEUESIZE; i++)
	{
		if (mWordQueues[i])
		{
			QueueEntry *qe = mWordQueues[i]->getFirst();
			while ( qe != NULL) 
			{
				WordEntry *wep = qe->getWordEntryValue();
				WLBreakIf(!wep || wep->text==NULL, "!!ERROR! graduateToDictionaries: wep(%x) or wep->text is null!!\n", (unsigned long)wep);
				if( mywcslen(wep->text)<1)
				{
					ShowInfo("!!WARNING! graduateToDictionaries! wep->text(%s) is empty!? why \n", toA(wep->text));
					mMRUQueue->deleteEntry(qe->getQueueEntryValue());
					mWordQueues[i]->deleteEntry(qe);
					qe = qe->mNext;
					continue;
				}
				int dictIdx = -1;
				if(wep->freq >= sMaxFrequencyBeforeSaveToDicts)
				{
					CompactNode *endNode = NULL;
					// first add wep word to current active dictionary, if it is not already there:
					if(wep->state == eCacheNew)
					{
						endNode = wpDictManager->addWord(wep->text, wep->pref, &dictIdx);
						wep->state = eCacheRegular;
					}
					else
						endNode = wpDictManager->retrieveEndNodeForString(wep->text, &dictIdx, true);
					if(!endNode)
					{
						ShowInfo("!!WARNING! graduateToDictionaries: word #%s# is not new and not in any dict either! probably deleted! so ignore and totally delete for now!\n", toA(wep->text));
						mMRUQueue->deleteEntry(qe->getQueueEntryValue());
						mWordQueues[i]->deleteEntry(qe);
						qe = qe->mNext;
						continue;
					}
					// then make a list of next words for t wep word:
					int count = 0;
					for(int j=0; wep->nextWords[j].idx != 0xff && j<MAX_NUM_PREDICTIONS && count<MAX_NUM_PREDICTIONS; j++)
					{
						if(wep->nextWords[j].idx == 0xff)
							continue;
						WordEntry *follow = (WordEntry *)sCacheAr.getElementAt(wep->nextWords[j].idx);
						if(follow==NULL)
						{
							wep->nextWords[j].idx = 0xff;
							continue;
						}
						if(follow->state == eCacheNew && follow->freq > 1) // for now we use freq>1 for next words in cache to graduate to dictionary!
						{
							sNextWords[count].text = follow->text;
							wpDictManager->getCompactStore(dictIdx)->addWord(follow->text, LOW_PREFERENCE);
							sNextWords[count].pref = max(LOW_PREFERENCE, wep->nextWords[j].pref);
							follow->state = eCacheRegular;
							count++;
						}
					}

					if(count>0)
					{
						endNode = wpDictManager->retrieveEndNodeForString(wep->text, &dictIdx, true);
						WLBreakIf(!endNode, "!!ERROR! graduateToDictionaries: endNode is null before adding next for word #%s#\n", toA(wep->text));
						for(int l=0; l<count; l++)
						{
							sNextWords[l].endNode = wpDictManager->getCompactDict(dictIdx)->retrieveEndNodeForString(sNextWords[l].text, true);
							WLBreakIf(!sNextWords[l].endNode, "!!ERROR! graduateToDictionaries: next word #%s# is not in dict yet!\n", toA(sNextWords[l].text));
						}

					//	memorylayoutchanged |= wpDictManager->getCompactStore(dictIdx)->insertNnextPtrs(endNode, sNextWords, count);

					}	
				}	
				qe = qe->mNext;
			}
		}
	}

	if(memorylayoutchanged)
	{
		ShowInfo("MK from graduateToDictionaries\n");
		fullInitializeWordPaths(true);
	}
}

static int sUpdatesCountSinceLastUpdate = 0;
/////////////////////////////////////////////////////////////////////////////////////////////
void UserWordCache::putWord(MYWCHAR *word, BYTE pref, USHORT access, int len, bool bNewWord, bool stopPhrase)
{
	
	WLBreakIf(!word || word[0]==NUL, "!!ERROR! UserWordCache::putWord: adding empty word!!\n");
	ShowInfo( "UserWordCache::putword #%s# pref=%d access=%d , bNewWord=%d, stopPhrase=%d, gHistWordIdx=%d\n", toA(word), pref, access, bNewWord, stopPhrase, gHistWordIdx );
	CacheQueue *wordQueue = pickWordQueue(word[0], TRUE);
	QueueEntry *wordQueueEntry = wordQueue->findEntry(word, len);
	if(wordQueueEntry && wordQueueEntry == mLastWord)
	{
		if(stopPhrase)
			mLastWord = NULL;
		ShowInfo("!!WARNING: putWord: are you doing the same word again? We igore consecutive words in cache for now!(%s)\n", toA(word)); 
		return;
	}
	if(sUpdatesCountSinceLastUpdate++ > sMaxUpdatesBeforeSaveOut)
	{
		graduateToDictionaries();
		dumpOutToCacheFile();
		sUpdatesCountSinceLastUpdate = 0;
	}
	sLatestAccess = max(access, sLatestAccess);
	if(sLatestAccess >= USHRT_MAX-1)
		RoundupAccessCounts();
	sLatestAccess++;
	if (wordQueueEntry)
	{
		// entry exists meaning it also exists in the mru queue, touch the entry so that it updates its position in the mMRUQueue
		QueueEntry *mru_entry = wordQueueEntry->getQueueEntryValue();
		if(mru_entry)
		{
			WordEntry *we = mru_entry->getQueueEntryValue()->getWordEntryValue();		
			WLBreakIf(!we || we->text[0] != word[0], "!!ERROR! putWord: lookup word #%s# is not the same as #%s#!\n", toA(we->text), toA(word));
			WLBreakIf(we->bufferIdx == 0xff, "!!ERROR! putWord: we->bufferIdx == 0xff!!\n"); // investigate this! this happens aftern detaching this word previously!
			we->pref = max(we->pref, pref);
			mMRUQueue->touchEntry(mru_entry, sLatestAccess);

		}
		//WLBreakIf(!mru_entry , "!!ERROR! putWord: wordentry for lookup word #%s# is NULL!\n", toA(word));
		//WordEntry *we = mru_entry->getQueueEntryValue()->getWordEntryValue();		
		//WLBreakIf(!we || we->text[0] != word[0], "!!ERROR! putWord: lookup word #%s# is not the same as #%s#!\n", toA(we->text), toA(word));
		//WLBreakIf(we->bufferIdx == 0xff, "!!ERROR! putWord: we->bufferIdx == 0xff!!\n"); // investigate this! this happens aftern detaching this word previously!
		//we->pref = max(we->pref, pref);
		//mMRUQueue->touchEntry(mru_entry, sLatestAccess);
	} 
	else 
	{
		if (mNumWordsInCache == mMax) 
		{
			// purge MRUWORDSINCACHEMAX/10 entry from letter queues as well as the mru queue. the mru queue entry contains 
			// the reference to the letter queue and the letter queue entry, making it easy to recycle these as well
			for(int i = 0; i<MRUWORDSINCACHEMAX/10; i++)
			{
				//QueueEntry *mruqe = mMRUQueue->purgeEntry();
				//ShowInfo("!putWord! Purging #%s# \n", toA(mruqe->getQueueEntryValue()->getWordEntryValue()->text));
				//QueueEntry *wordqe = mruqe->getQueueEntryValue();
				//CacheQueue *wordQueue = mruqe->getQueueValue();
				//wordQueue->detachEntry(wordqe);
				//freeupWordEntry(wordqe);
			}
			mNumWordsInCache -= (MRUWORDSINCACHEMAX/10);
		}

		// now allocate new entry in proper wordqueue and update mruqueue too!
		WordEntry *wep = allocateWordEntry(word, pref, sLatestAccess, len, bNewWord);
		mNumWordsInCache++;
		QueueEntry *mruqEntry = new(sQueueAr.next()) QueueEntry();
		wordQueueEntry = new(sQueueAr.next()) QueueEntry();

		wordQueueEntry->setValues(mruqEntry, (void *)wep);
		mruqEntry->setValues(wordQueueEntry, (void *)wordQueue);

		mMRUQueue->insertEntry(mruqEntry);
		wordQueue->insertEntry(wordQueueEntry);
	}

	UpdateLastWord(wordQueueEntry, stopPhrase);

}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Sets next words for previous last word, then updates lastWord.
void UserWordCache::UpdateLastWord(QueueEntry *currWord, bool stopPhrase)
{
	if(!mLastWord || !mLastWord->getQueueValue() || !mLastWord->getQueueEntryValue())
	{
		mpreLastWord = mLastWord;
		mLastWord = currWord;
		return;
	}
	WLBreakIf(!currWord || !mLastWord, "!!ERROR! UserWordCache::UpdateLastWord: current(%x) or lastword(%x) not set!, gHistWordIdx=%d\n", currWord, mLastWord);
	WordEntry *curWep = currWord->getWordEntryValue();
	WordEntry *lastWep = mLastWord->getWordEntryValue();

	if(!curWep || !lastWep)
	{
		return;
	}

	WLBreakIf(!curWep->text || !lastWep->text, "!!ERROR! UserWordCache::UpdateLastWord: curWep(%s) or lastWep(%s) is null!\n", toA(curWep->text), toA(lastWep->text));
	ShowInfo("UserWordCache::UpdateLastWord: setting up last(%s)->next(%s), gHistWordIdx=%d\n", toA(lastWep->text), toA(curWep->text), gHistWordIdx);
	// first see if lastWord's nextwords already has a slot, if so update it,
	// otherwise if there is an empty slot in nextWords, take it!
	int updatedIdx = -1;
	for(int i=0; i<MAX_NUM_PREDICTIONS; i++)
	{
		if(lastWep->nextWords[i].idx == curWep->bufferIdx)
		{ // TODO: this is a simplistic method of incrementing pref! It needs to be nonlinear to slow down when the count goes up!
			lastWep->nextWords[i].pref = min(lastWep->nextWords[i].pref+1, MAXIMUM_PREFERENCE); 
			updatedIdx = i;
			break;
		}
		if(lastWep->nextWords[i].idx == 0xFF)
		{
			lastWep->nextWords[i].set(curWep->bufferIdx, LOW_PREFERENCE);
			updatedIdx = i;
			break;
		}
	}

	if(updatedIdx < 0)
	{ //if we are here, all nextWords are occuppied, so replace the lowest preference nextword with new one	
		lastWep->nextWords[MAX_NUM_PREDICTIONS-1].set(curWep->bufferIdx, LOW_PREFERENCE);
	}
	else
	{ // now sort the list to keep it in highest to lowest order!
		for(int i=updatedIdx; i>0; i--)
		{
			if(lastWep->nextWords[i].pref > lastWep->nextWords[i-1].pref)
				CacheNextWordInfo::swap(lastWep->nextWords[i], lastWep->nextWords[i-1]);
		}
	}

	if(stopPhrase)
		mLastWord = NULL;
	else
		mLastWord = currWord;
	ShowInfoIf(mLastWord, "--UpdateLastWord: return:lastword=#%s#..", toA(mLastWord->getWordEntryValue()->text));
	ShowInfoIf(!mLastWord, "--UpdateLastWord: return:lastword is NUll since phrase ended!");
}

/////////////////////////////////////////////////////////////////////////////////////////////
// depreciate importance of a word in cache by decreasing its frequency!
void UserWordCache::depreciate(MYWCHAR *word)
{
	trimEndingSpaces(word);
	CacheQueue *wordQueue = pickWordQueue(word[0], TRUE);
	if(!wordQueue)
		return;
	int len = mywcslen(word);
	QueueEntry *wordQueueEntry = wordQueue->findEntry(word, len);
	if(wordQueueEntry)
	{
		WordEntry *curWep = wordQueueEntry->getWordEntryValue();
		WLBreakIf(!curWep, "!!ERROR!UserWordCache::depreciate: curWep is null for word #%s# !\n", toA(word));
		curWep->freq--;
		if(curWep->freq<=0)
		{
			mMRUQueue->deleteEntry(wordQueueEntry->getQueueEntryValue()); // purging mRUQueue entry
			wordQueue->deleteEntry(wordQueueEntry); // deleting wordqueue entry
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
int UserWordCache::GetWordsAddedAndReset() 
{
	int ret = sTotalWordAddedSinceLastCheck;
	sTotalWordAddedSinceLastCheck = 0;
	return ret;
}

/////////////////////////////////////////////////////////////////////////////////////////////
void UserWordCache::Flush2Dict(BYTE *cnt)
{
	int dictIdx = 0;
	for(int j = 0; j < sFlushableNewWordsCount; j++)
	{
		wpDictManager->addWord(sFlushableWords[j]->text, sFlushableWords[j]->pref, &dictIdx);
	//	ShowInfoIf(sFlushableWords[j]->state == eCacheRegular, "!Warning! newword to flash to disk has wrong state!!\n");
	//	sFlushableWords[j]->state = eCacheRegular;
	}
	*cnt = 0;
	sFlushableNewWordsCount = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// flushes word in new word cache once it is at least half full. Returns the number of words being flushed,
// which means added to Personal dictionary.
int UserWordCache::flushLearnedWords()
{
	// first collect flushable words
	BYTE count = sFlushableNewWordsCount;
	for (int i = 0; i < QUEUESIZE; i++)
	{
		if (mWordQueues[i])
			mWordQueues[i]->findWordsReadyToFlush(&count);
	}

	// flush to current dict if flushbuffer is almost full!
	if(count >= MAX_NUM_FLUSH_PER_CALL - 2)
		Flush2Dict(&count);

	sTotalWordAddedSinceLastCheck = count - sFlushableNewWordsCount;
	return count;
}


/////////////////////////////////////////////////////////////////////////////////////////////
void UserWordCache::CacheQueue::findWordsReadyToFlush(BYTE *count)
{
	QueueEntry *qe = mFirst;
	while ( qe != NULL) 
	{
		WordEntry *wep = qe->getWordEntryValue();
		WLBreakIf(!wep || wep->text[0]==NUL, "!!ERROR! CacheQueue::findWordsReadyToFlush: wep(%x) or wep->text is null!\n", wep);
		if(wep->state == eCacheNew && wep->freq >= sMaxFrequencyBeforeSaveToDicts)
		{
			if((*count) >= (MAX_NUM_FLUSH_PER_CALL-1))
				UserWordCache::Flush2Dict(count);
			sFlushableWords[(*count)++] = wep;
		}
		qe = qe->mNext;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
void UserWordCache::SaveToCacheFile(BYTE *cnt, FILE *fp)
{
	for(int j = 0; j < *cnt; j++)
	{
		fwprintf(fp, L"%s %d %d\n", sFlushableWords[j]->text, sFlushableWords[j]->pref, sFlushableWords[j]->freq);
		sFlushableWords[j] = NULL;
	}
	*cnt = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
void UserWordCache::CacheQueue::findWordsToDump(BYTE *count, FILE *fp)
{
	WLBreakIf(gHistWordIdx > NWORDS, "findWordsToDump: gHistWordIdx=%d, count=%d: \n", gHistWordIdx, *count);
	QueueEntry *qe = mFirst;
	while ( qe != NULL) 
	{
		WordEntry *wep = qe->getWordEntryValue();
		if(wep)
		{
			if((*count) >= sMaxUpdatesBeforeSaveOut)
				UserWordCache::SaveToCacheFile(count, fp);
			sFlushableWords[(*count)++] = wep;
		}
		qe = qe->mNext;
	}
}

// dump out cache content to cache file for backup and saving.
//////////////////////////////////////////////////////////////////////////////////////////////////
void UserWordCache::dumpOutToCacheFile()
{
	char* filename = createFullPathFileName(sCacheFileName);
	ShowInfo("UserWordCache: cache file %s, gHistWordIdx=%d\n", filename,gHistWordIdx);
	FILE *fp = fopen(filename, "w");
	if(fp)
	{
#ifdef _WINDOWS
		_setmode(_fileno(fp), _O_WTEXT);
#endif
		BYTE count = 0;
		for (int i = 0; i < QUEUESIZE; i++)
		{
			if (mWordQueues[i])
				mWordQueues[i]->findWordsToDump(&count, fp);
		}
		if(count > 0)
			SaveToCacheFile(&count, fp);
		fclose(fp);
	}
}
////////////////////////////////////////////////////////////////////////////////
void UserWordCache::removeAll() 
{
	for (int i = 0; i < QUEUESIZE; i++)
	{
		if (mWordQueues[i])
			mWordQueues[i]->clean();
	}
	mMRUQueue->clean();
	mNumWordsInCache = 0;

}

///////////////////////////////////////////////////////////////////////////////
 UserWordCache::CacheQueue::CacheQueue(bool bIsLetterQueue) 
{
	mFirst = NULL;
	mLast = NULL;
	mIsLtrQueue = bIsLetterQueue;
}
////////////////////////////////////////////////////////////////////////////////
void  UserWordCache::CacheQueue::insertEntry(QueueEntry *qe) 
{
	if (mFirst == NULL)
	{
		mFirst = qe;
		mLast = qe;
	}
	else 
	{
		mLast->mNext = qe;
		qe->mPrev = mLast;
		mLast = qe;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
// detach yourself from the list
void  UserWordCache::CacheQueue::detachEntry(QueueEntry *entry) 
{
//	ShowInfoIf(!mIsLtrQueue, "CacheQueue::detachEntry for mruqueue entry=#%s#\n", toA(entry->getQueueEntryValue()->getWordEntryValue()->text));
//	ShowInfoIf(mIsLtrQueue, "CacheQueue::detachEntry for wordqueue entry=#%s#\n", toA(entry->getWordEntryValue()->text));
	if (entry == mFirst)
	{
		mFirst = entry->mNext;
	}
	if (mLast == entry)
	{
		mLast = entry->mPrev;
	}

	if (entry->mPrev) 
	{
		entry->mPrev->mNext = entry->mNext;
	}
	if (entry->mNext) 
	{
		entry->mNext->mPrev = entry->mPrev;
	}
	entry->mNext = entry->mPrev = NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Place entry at the end of linked list -- Most Recently Used
/////////////////////////////////////////////////////////////////////////////////////////////
int  UserWordCache::CacheQueue::touchEntry(QueueEntry *entry, USHORT access) 
{
	WLBreakIf(mIsLtrQueue, "!!ERROR! CacheQueue::touchEntry: This call should be done only for mruqueue, not word queues!!\n");
	if (mLast != entry) //first update it's position in mruqueue! i.e. bring it to the top of th queue if it is not already there!
	{
		detachEntry(entry);
		insertEntry(entry);
	}
	return touchWordEntry(entry->getQueueEntryValue()->getWordEntryValue(), access);
}

/*************************************************************************************
 * Purge least recently used word. Returnes the purged entry! SO BE Carefull not to use 
 * the return entry for long since it will be overwritten quickly! We can still use it for few
 * immediate calls after only because purging only sets the flag in memory manager slot to be free
 *, but does not clear the memory. So if we change memory manager for some reason later on, this
 * behavior SHOULD be corrected!!
 ****************************************************************************************/
UserWordCache::QueueEntry * UserWordCache::CacheQueue::purgeEntry()
{
	WLBreakIf(mIsLtrQueue, "!!ERROR! CacheQueue::purgeEntry: This call should be done only for mruqueue, not word queues!!\n");
	QueueEntry *entry = mFirst;

	if(!entry)
		return NULL;

	detachEntry(entry);
	sQueueAr.release((char*)entry);

	// now purge corresponding word queue entry:
	ShowInfo("!CacheQueue::purgeEntry! Purging #%s# \n", toA(entry->getQueueEntryValue()->getWordEntryValue()->text));
	QueueEntry *wordqe = entry->getQueueEntryValue();
	CacheQueue *wordQueue = entry->getQueueValue();

	wordQueue->detachEntry(wordqe);
	freeupWordEntry(wordqe);

	return entry;
}

// works only on letter queues
/////////////////////////////////////////////////////////////////////////////////////////////
UserWordCache::QueueEntry*  UserWordCache::CacheQueue::findEntry(MYWCHAR *word, int len)
{
	for (QueueEntry *qe = mFirst; qe != NULL; qe = qe->mNext)
	{
		WordEntry *wep = qe->getWordEntryValue();
		if (wep && wep->len==len && !mywcsncmp(wep->text, word, len))
			return qe;
	}
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////
void  UserWordCache::CacheQueue::deleteEntry(QueueEntry *qe) 
{
	ShowInfo("CacheQueue::deleteEntry() for word #%s#\n", toA(qe->getWordEntryValue()->text));

	detachEntry(qe);
	if (mIsLtrQueue) 
	{
		freeupWordEntry(qe);
	}
	else
		sQueueAr.release((char*)qe);

}
////////////////////////////////////////////////////////////////////////////////////////////
void  UserWordCache::CacheQueue::clean() 
{
	QueueEntry *nextqe = NULL;
	for (QueueEntry *qe = mFirst; qe != NULL;) 
	{
		nextqe = qe->mNext;
		deleteEntry(qe);
		qe = nextqe;
	}
	mFirst = NULL;
	mLast = NULL;
}

/*
 WCHAR ** CacheQueue::findPredictionsStartingWith(WCHAR *word, BOOL *bChunkFlagP, int nFreeSlots)
 {
	// static WCHAR *sLearnedPredictions[6];

	 int nFilled = 0;
	 int typedChars = mywcslen(word); // number of characters in
	 for (QueueEntry *qe = mFirst;  qe != NULL; qe = qe->getNext())
	 {
		 WordEntry *wep = qe->getWordEntryValue();

		 // needs to be longer than the typedChars otherwise no prediction
		 if (wep && wep->len > typedChars)
		 {
			 int predictAfterNChars = wep->len - wep->usage;
			 if (typedChars >= predictAfterNChars && mywcsncmp(wep->text,word, typedChars) == 0)
			 {
				 myLearnedPredictions[nFilled] = &wep->text[typedChars];
				 bChunkFlagP[nFilled] = wep->bChunkFlag;
				 nFilled++;
				 #ifdef _DEBUG
				 WCHAR myszBuf[400];
				 swprintf(myszBuf, 400, TEXT("learning word prediction  %s , matchword %s pref %d, usage %d\n"),
				 word, wep->text, wep->preference, wep->usage);
				 ShowInfo(myszBuf);
				 #endif
			 }
		 }
	 }
	 for (int i = nFilled; i<6; i++) {
	 myLearnedPredictions[i] = NULL;
	 bChunkFlagP[i] = FALSE;
	 }

	 return nFilled > 0 ? myLearnedPredictions : NULL;
 }
 */
// finds possible word predictions, based on input stem word, for this letter queue cache.
// A preference is computed based on word's frequency of use, its latest access, its length and its basic preference.
// returns number of found predictions.
/////////////////////////////////////////////////////////////////////////////////////////////
int UserWordCache::CacheQueue::findPredictionsStartingWith(MYWCHAR *word, USHORT *prefs) 
{
	int nFilled = 0;
	int nTypedChars = mywcslen(word); // number of characters in
	for (QueueEntry *qe = mFirst; qe != NULL; qe = qe->mNext) 
	{
		WordEntry *wep = qe->getWordEntryValue();
		WLBreakIf(!wep, "!!ERROR! CacheQueue::findPredictionsStartingWith ! wep is null for #%s#\n", toA(word));
		// needs to be longer than the typedChars otherwise no prediction
		if (wep->len > nTypedChars && mywcsncmp(wep->text, word, nTypedChars) == 0)
		{
			bool bInserted = false;
		//	ShowInfo("findPredictionsStartingWith: learned word suggestion: (%s, %d): ", toA(wep->text), wep->freq);
			USHORT accessDif = UserWordCache::sLatestAccess - wep->accessed;
			WLBreakIf(accessDif<0, "!!ERROR! findPredictionsStartingWith:accessDif < 0 %d < %d ??\n", UserWordCache::sLatestAccess, wep->accessed);
			USHORT totalPref = (wep->len-nTypedChars) + wep->pref +  float(wep->freq*2+MAXIMUM_PREFERENCE)/float(MAXIMUM_PREFERENCE) * 0x150  + 0x270/(accessDif+1);
			//ShowInfo("findPredictionsStartingWith: text=%s, totalPref=%d\n", toA(wep->text), totalPref);
			
			// insert in the proper slot, keeping sLearnedPredictions ordered
			for (int i = 0; i <= nFilled; i++) 
			{
				if (totalPref > prefs[i])
				{	
					for (int j = nFilled; j > i; j--) // shift every cell up before inserting
					{
						prefs[j] = prefs[j-1];
						sLearnedPredictions[j] = sLearnedPredictions[j-1];
					}

					prefs[i] = totalPref;
					sLearnedPredictions[i] = wep->text;
					bInserted = true;
					break;
				}
			}
			
			nFilled = min(nFilled+1, MAX_NUM_PREDICTIONS-1);
			if (!bInserted && nFilled < (MAX_NUM_PREDICTIONS-1)) 
			{
				prefs[nFilled] = totalPref;
				sLearnedPredictions[nFilled] = wep->text;
			}
		}
	}

	return nFilled;
}


#ifdef _DEBUG
// works only on letter queues
/////////////////////////////////////////////////////////////////////////////////////////////
void  UserWordCache::CacheQueue::printMatchingWords(MYWCHAR *word) 
{
	int wordLen = mywcslen(word);
	for (QueueEntry *qe = mFirst; qe != NULL; qe = qe->mNext) 
	{
		WordEntry *wep = qe->getWordEntryValue();
		if (wep && mywcsncmp(wep->text, word, wordLen) == 0) 
		{
			ShowInfo(("searchForMru word starting with %s, matchword %s, freq %d\n"), toA(word), toA(wep->text), wep->freq);
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////
void  UserWordCache::CacheQueue::printQueue(BOOL bMruQueue)
{
	int i = 0;
	QueueEntry *lqe;
	for (QueueEntry *qe = mFirst; qe != NULL; qe = qe->mNext)
	{
		if (bMruQueue)
			lqe = qe->getQueueEntryValue();
		else
			lqe = qe;

		WordEntry *wep = lqe->getWordEntryValue();
		ShowInfo("%d: word %s, usage %d\n", i, toA(wep->text), wep->freq);
		i++;
	}
}
#endif
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void wordEntry::reset()
{
	UserWordCache::sCacheAr.release(bufferIdx); // WARNING! this slot should not be used till end of this function!
	pref = freq = len = accessed = 0;
	bufferIdx = 0xFF;
	resetNextWords();
	text[0] = NUL;
}

// returns next empty slot in word array.
///////////////////////////////////////////////////////////////////////////////////////////////////////////
WordEntry *UserWordCache::allocateWordEntry(MYWCHAR *word, BYTE pref, USHORT access, int len, bool bNewWord) 
{
	WLBreakIf(mywcslen(word)>= MAX_WORD_LEN, "!!ERROR! UserWordCache::allocateWordEntry: word %s is longer than expected in cache!\n", toA(word));
	//int wordLen = (len + 1) * sizeof(MYWCHAR) + sizeof(WordEntry);
	int idx = 0;
	WordEntry *p = (WordEntry *) sCacheAr.getNext(&idx);
	WLBreakIf(p==NULL, "!!ERROR! UserWordCache::allocateWordEntry! sCacheAr ran out of memory!\n");
//	p->text = (MYWCHAR *) ((char*) p + sizeof(WordEntry));
	mywcsncpy(p->text, word, MAX_WORD_LEN);
	p->pref = pref;
	p->bufferIdx = idx;
	p->state = bNewWord;// ? eCacheNew : eCacheRegular;
	p->freq = 1;
	p->accessed = access;
	p->resetNextWords();
	p->len = len;
	return p;
}
/////////////////////////////////////////////////////////////////////////////////////////
int UserWordCache::touchWordEntry(WordEntry *p, USHORT access) 
{
	WLBreakIf(!p, "!!ERROR!  UserWordCache::touchWordEntry! p is null!\n");
	p->freq = min(p->freq+1, MAXIMUM_PREFERENCE);
	p->accessed = access;
	return p->freq;
}
////////////////////////////////////////////////////////////////////////////
void UserWordCache::unLearn(MYWCHAR *word)
{
	if(!word || word[0] == NUL)
		return;
	int nSP = trimEndingSpaces((MYWCHAR*)word);
	ShowInfo(" UserWordCache::unLearn() for word #%s#, nSP=%d\n", toA(word), nSP);

	int len = mywcslen(word);
	if(nSP > 0 || findPunctuationChoice(word[len-1]))
	{
		ShowInfo("!!UserWordCache::unLearn: word ended at sp or punctuation! so return!\n");
		return;
	}
	CacheQueue *wordQueue = pickWordQueue(word[0], FALSE);
	QueueEntry *wordQueueEntry = NULL;
	if(wordQueue)
		wordQueueEntry = wordQueue->findEntry(word, len);
	if (wordQueueEntry)
	{
		QueueEntry *mru_entry = wordQueueEntry->getQueueEntryValue();
		WLBreakIf(!mru_entry , "!!ERROR! unLearn: wordentry for lookup word #%s# is NULL!\n", toA(word));
		WordEntry *we = mru_entry->getQueueEntryValue()->getWordEntryValue();		
		WLBreakIf(!we || we->text[0] != word[0], "!!ERROR! unLearn: lookup word #%s# is not the same as #%s#!\n", toA(we->text), toA(word));
		WLBreakIf(we->bufferIdx == 0xff, "!!ERROR! unLearn: we->bufferIdx == 0xff!!\n"); 
		if(--we->freq <= 0)
		{
			ShowInfo(" UserWordCache::unLearn() --we->freq <= 0\n");
			mMRUQueue->deleteEntry(wordQueueEntry->getQueueEntryValue());
			wordQueue->deleteEntry(wordQueueEntry);
		}
		mLastWord = mpreLastWord;
	}
}
////////////////////////////////////////////////////////////////////////////////////////
void UserWordCache::freeupWordEntry(QueueEntry *wqe)
{
	WordEntry *wep = wqe->getWordEntryValue();
	if(wep)
	{
		ShowInfo("CacheQueue::freeupWordEntry() for word #%s#\n", toA(wep->text));
		wqe->setValues(NULL, NULL);
		wep->reset();
	//	sCacheAr.release((char*)wep); alrady done in wep->reset()!
	}
	sQueueAr.release((char*)wqe);
}

///////////////////////////////////////////////////////////////////////////////////////////////
// replaces an already existing word in the cache with a new one. It is crucial that it uses the same buffer slots!
void UserWordCache::replace(MYWCHAR *word, MYWCHAR *replace) 
{
	if(!word || word[0] == NUL)
		return;

	trimEndingSpaces((MYWCHAR*)word);
	trimEndingSpaces((MYWCHAR*)replace);

	ShowInfo(" UserWordCache::replace() #%s#  with #%s#\n", toA(word), toA(replace));

	int len = mywcslen(word);
	CacheQueue *wordQueue = pickWordQueue(word[0], FALSE);
	QueueEntry *wordQueueEntry = NULL;
	if(wordQueue)
	{
		wordQueueEntry = wordQueue->findEntry(word, len);
	}
	
	if (wordQueueEntry)
	{
		ShowInfo("MK UserWordCache::replace() #%s#  with #%s#\n", toA(word), toA(replace));

		QueueEntry *mru_entry = wordQueueEntry->getQueueEntryValue();
		WLBreakIf(!mru_entry , "!!ERROR! replace: wordentry for lookup word #%s# is NULL!\n", toA(word));
		WordEntry *we = mru_entry->getQueueEntryValue()->getWordEntryValue();	
		WLBreakIf(!we || we->text[0] != word[0], "!!ERROR! replace: lookup word #%s# is not the same as #%s#!\n", toA(we->text), toA(word));
		WLBreakIf(we->bufferIdx == 0xff, "!!ERROR! replace: we->bufferIdx == 0xff!!\n"); 
		// first search if the replacement is already in cache. If so, just delete word's entry!
		int len1 = mywcslen(replace);
		CacheQueue *wordQueue1 = pickWordQueue(replace[0], TRUE);
		QueueEntry *wordQueueEntry1 = wordQueue1->findEntry(replace, len1);
		if(wordQueueEntry1)
		{
			WordEntry *we1 = wordQueueEntry1->getWordEntryValue();
			WLBreakIf(!we1 || we1->text[0] != replace[0], "!!ERROR! replace: replace lookup word #%s# is not the same as #%s#!\n", toA(we1->text), toA(replace));
			if(--we1->freq <= 0)
			{
				mMRUQueue->deleteEntry(wordQueueEntry->getQueueEntryValue());
				wordQueue->deleteEntry(wordQueueEntry);
			}
			mLastWord = wordQueueEntry1;
		}
		else
		{
			mywcsncpy(we->text, replace, MAX_WORD_LEN);
			we->len = mywcslen(replace);
		}
	}

}
/////////////////////////////////////////////////////////////////////////////////////////////
UserWordCache::~UserWordCache()
{
	removeAll();
}
