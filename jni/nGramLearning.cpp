// <copyright file="nGramLearning.cpp" company="WordLogic Corporation">
// Copyright (c) 2000, 2014 All Right Reserved, http://www.wordlogic.com/
//
// THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
// KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// </copyright>
// <author>Minkyu Lee</author>
// <email>minkyulee17@yahoo.com</email>
// <date>2014-04-08</date>
// <summary>
//        </summary>

#include "StdAfx.h"
#include "nGramLearning.h"
#include "dictionary.h"
#include "dictmanager.h"
#include "parsedline.h"
#ifdef _WINDOWS
#include <io.h>
#else
#include <stdio.h>
#endif
#include <fcntl.h>

extern CDictManager *wpDictManager;

//////////////////////////////////////////////////////////////////////////////
// searches in the cache for a startWord node with given fields. If it finds it, 
// updates it, otherwise add a new startWord node to the cache.
void StartWordCache::addStartWord(CompactNode *node, USHORT pref)
{
	for(int i=0; i<MAX_START_WORD_CACHE_SIZE; i++)
	{
		if(startWords[i].endCNode == node)
		{
			startWords[i].startPref = max(startWords[i].startPref, pref);
			break;
		}
		if(startWords[i].endCNode == NULL)
		{
			startWords[i].endCNode = node;
			startWords[i].startPref = pref;
			count++;
			countSinceSorted++;
			countSinceLastSaved++;
			if(pref < startWords[lowestPrefIdx].startPref)
				lowestPrefIdx = i;
			if(countSinceSorted >( MAX_START_WORD_CACHE_SIZE/10))
			{
				qsort(startWords, count, sizeof(CompactNodeBlk), NodeCompare);
				countSinceSorted = 0;
			}
			if(countSinceLastSaved > MAX_START_WORD_CACHE_SIZE/5)
			{
				saveOutCachedStartingWords();
				countSinceLastSaved = 0;
			}
			break;
		}
	}

	// cache is too full! purge out last 1/8 of its capacity based on lowest prefs:
	if(count >= (MAX_START_WORD_CACHE_SIZE-1))
	{
		qsort(startWords, count, sizeof(CompactNodeBlk), NodeCompare);
		int num2Purge = MAX_START_WORD_CACHE_SIZE/8;
		for(int i = 0; i<num2Purge; i++)
		{
			startWords[count-1-i].endCNode = NULL;
			startWords[count-1-i].startPref = 0;
		}
		count = count - num2Purge;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void StartWordCache::deleteStartWord(CompactNode *node)
{
	int count = 0;
	for(int i=0; i<MAX_START_WORD_CACHE_SIZE; i++)
	{
		if(startWords[i].endCNode == NULL)
			break;

		if(startWords[i].endCNode == node)
		{
			startWords[i].endCNode = NULL;
			startWords[i].startPref = 0;
		}
		count++;
	}

	qsort(startWords, count, sizeof(CompactNodeBlk), NodeCompare);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int StartWordCache::NodeCompare(const void *a, const void *b)
{
	CompactNodeBlk *nodeA = (CompactNodeBlk *)a;
	CompactNodeBlk *nodeB = (CompactNodeBlk *)b;

	if(nodeA->startPref > nodeB->startPref) return -1;
	if(nodeA->startPref == nodeB->startPref) return 0;

	return 1;
}
////////////////////////////////////////////////////////////////////////////////////////////////
void  StartWordCache::saveOutCachedStartingWords()
{
	if(cacheFilename[0])
	{
		char* filename = createFullPathFileName(cacheFilename);
		ShowInfo("StartWordCache: cache file %s, gHistWordIdx=%d\n", filename,gHistWordIdx);
		FILE *fp = fopen(filename, "w");
		if(fp)
		{
			//BYTE count = 0;
			for (int i = 0; i < count; i++)
			{
				MYWCHAR *word = wpDictManager->retrieveStringForEndNode(startWords[i].endCNode);
				if(!isEmptyStr(word))
				{
					char *szPhrase = toA(word);
					fprintf(fp, "%s %d\n", szPhrase, startWords[i].startPref);
					ShowInfo("MK StartWordCache: word %s, startPref=%d\n", szPhrase, startWords[i].startPref);
				}

			}
			fclose(fp);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
void StartWordCache::loadStartWordsCacheFromFile(const char*filename)
{
	strcpy(cacheFilename, filename);
	char* fullPath = createFullPathFileName(filename);
	ShowInfo("StartWordCache::StartWordCache: loading cache file %s\n", fullPath);
	FILE *fp = fopen(fullPath, "r");
	if(!fp)
		return;

	//ShowInfo("StartWordCache Opened cache file: now try to load it!\n");
	char aLine[MAX_WORD_LEN];
	// reset getLine structures
	bool unicodeFlag = true;
	getLine(0, NULL, unicodeFlag);
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

	while (fgets( aLine, MAX_WORD_LEN, fp ) != NULL)
	{
		int lineLen  = (int)strlen(aLine);

		while (lineLen - 1 >= 0 && (aLine[lineLen-1] == '\n' || aLine[lineLen-1] == '\r' || aLine[lineLen-1]== ' '))
		{
			aLine[lineLen-1] = 0;
			lineLen--;
		}

		MYWCHAR * word = toW(aLine);
		MYWCHAR wcPref[MAX_WORD_LEN];
		memset(wcPref, 0, sizeof(wcPref));
		int index =0;
		for(int i=0; i < lineLen;i++)
		{
			if(isNumberChar(word[i]))
			{
				wcPref[index++] = word[i];
				word[i]= NUL;
			}
		}

		trimEndingSpaces(word);

		int nPref = atoi(toA(wcPref));
		if(nPref > 0)
		{
			int dictIdx = 0;
			CompactNode *endnode = wpDictManager->retrieveEndNodeForString(word, &dictIdx, true);
			if(endnode==NULL)
				continue;
			addStartWord(endnode, nPref);
			ShowInfo("MK startWordCache:%s, nPref:%d\n", toA(word), nPref);
		}
	}

	fclose(fp);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CompactNode **StartWordCache::getTop5StatWords(MYWCHAR *head)
{
	static CompactNode *sOutNodes[5];
	sOutNodes[0] = sOutNodes[1] = sOutNodes[2] = sOutNodes[3] = sOutNodes[4] = NULL;
	if(head == NULL)
	{
		if(countSinceSorted > 2)
		{
			qsort(startWords, count, sizeof(CompactNodeBlk), NodeCompare);
			countSinceSorted = 0;
		}

		for(int i=0; i < 5; i++)
		{
			if(startWords[i].endCNode)
			{
				sOutNodes[i] = startWords[i].endCNode;
				sOutNodes[i]->pref = startWords[i].startPref;
			}

		}
	}
	else
	{
		int foundMatch = 0;
		int cmpLen = mywcslen(head);
		for(int i=0; i<count; i++)
		{
			MYWCHAR *curWord = wpDictManager->retrieveStringForEndNode(startWords[i].endCNode);
			if(mywcsncmp(head, curWord, cmpLen)==0)
			{
				sOutNodes[foundMatch++] = startWords[i].endCNode;
				if(foundMatch>=5)
					break;
			}
		}
	}
	return sOutNodes;
}
/////////////////////////////////////////////////////////////////////////////////////////
USHORT StartWordCache::GetStartWordPref(CompactNode *node)
{
	for(int i=0; i < MAX_START_WORD_CACHE_SIZE; i++)
	{
		if(!startWords[i].endCNode)
			break;

		if(startWords[i].endCNode && startWords[i].endCNode == node)
			return startWords[i].startPref;	
	}

	return 0;
}
/////////////////////////////////////////////////////////////////////////////////////////
void StartWordCache::SetSavedStartWordPref(CompactNode* node, USHORT newPref)
{
	for(int i=0; i < MAX_START_WORD_CACHE_SIZE; i++)
	{
		if(!startWords[i].endCNode)
			break;

		if(startWords[i].endCNode && startWords[i].endCNode == node)
		{
			startWords[i].startPref = newPref;
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
NGramLearning::NGramLearning()
{
	reset();

	char* dictName = wpDictManager->getTopDictionaryName();
	WLBreakIf(dictName == NULL || isEmptyStr(toW(dictName)), "!!ERROR! NGramLearning cannot be created!\n");
	char startingWordPath[MAX_WORD_LEN];
	sprintf(startingWordPath, "%sStartWordsCache.txt", dictName);
	// load starting word cache
	mStartWordCache.loadStartWordsCacheFromFile(startingWordPath);
	
	char path3[MAX_WORD_LEN];
	char path4[MAX_WORD_LEN];
	for(int i=0; i <MAX_ALPHABET_SIZE; i++)
	{
		sprintf(path3, "%s3LearningCache%d.txt", dictName, i);
		m_3Cache[i].LoadGramLearningCacheFromFile(path3, N3Gram);

		sprintf(path4, "%s4LearningCache%d.txt", dictName, i);
		m_4Cache[i].LoadGramLearningCacheFromFile(path4, N4Gram);
	}
}
////////////////////////////////////////////////////////////////////////////////////////
void NGramLearning::reset()
{
	memset(m_3Cache, 0, sizeof(m_3Cache));
	memset(m_4Cache, 0, sizeof(m_4Cache));
}

///////////////////////////////////////////////////////////////////////
NGramLearning::~NGramLearning()
{
	mStartWordCache.saveOutCachedStartingWords();
}
///////////////////////////////////////////////////////////////////////
void NGramLearning::addStartWordToCache(CompactNode *endnode, USHORT pref)
{
	mStartWordCache.addStartWord(endnode, pref);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int NGramLearning::GetNGramLearningAlphabetIndex(CompactNode* node, MYWCHAR* word)
{
	int alphaInd = -1;
	if(node)
	{
		alphaInd = GetAlphabetIndex(node->Letter);
	}
	else
	{
		if(word && !isEmptyStr(word))
		{
			int len = mywcslen(word);
			alphaInd = GetAlphabetIndex(word[len-1]); //Get index from the last letter of the first word.
		}
	}

	if(alphaInd < 0 || alphaInd >= MAX_ALPHABET_SIZE)
	{
		//ShowInfo("MK GetNGramLearningAlphabetIndex return MAX\n");
		return MAX_ALPHABET_SIZE;
	}
	//WLBreakIf(alphaInd < 0 || alphaInd >= MAX_ALPHABET_SIZE,"!ERROR!! alphabet index(%d) error for (%s)!!\n", alphaInd);

	return alphaInd;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CompactNode* NGramLearning::CheckWordForCases(MYWCHAR* word, int* nodeCount)
{
	//if(isUpperCase(word[0]))
	//	word[0] = lwrCharacter(word[0]);
	//else
	//	word[0] = uprCharacter(word[0]);

	int dictIdx =0;
	//CompactNode* endnode = wpDictManager->retrieveEndNodeForString(word, &dictIdx, false);

	CompactNode* endnode = wpDictManager->QuadRetrieveCompactNode(word, true, &dictIdx);
	if(endnode)
		*nodeCount = *nodeCount+1;
	
	return endnode;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void NGramLearning::Learn2Gram(MYWCHAR word[MAX_TGRAPH_HISTORY_NUM][MAX_WORD_LEN])
{
	MYWCHAR actualWord[N2Gram][MAX_WORD_LEN];
	memset(actualWord, 0, sizeof(actualWord));
	int ind =1;
	for(int i = MAX_TGRAPH_HISTORY_NUM-1; i >=0; i--)
	{
		if(ind == N2Gram)
			break;

		if(!isEmptyStr(word[i]))
		{
			mywcscpy(actualWord[ind--], word[i]);
		}
	}

	if(ind > 0)
		return;

	int nodeCount =0;
	CompactNode* node[N2Gram];	
	MYWCHAR wordInput[MAX_WORD_LEN];

	bool bNonDict = false;
	for(int i=0; i < N2Gram;i++)
	{
		node[i] = NULL;
	
		mywcscpy(wordInput, actualWord[i]);
		//MakeLowerCase(wordInput);

		ReplaceiToI(actualWord[i]);
		ReplaceiToI(wordInput);

		int dictIdx = 0;
		CompactNode* endnode = wpDictManager->retrieveEndNodeForString(wordInput, &dictIdx, true);
		if(endnode)
		{
			node[i] = endnode;
			nodeCount++;
		}
		else
		{
			bool bPreviousUpper = isUpperCase(actualWord[i][0]);
			node[i] = CheckWordForCases(actualWord[i], &nodeCount);
			if(!node[i])
			{
				if(bPreviousUpper)
					actualWord[i][0] = uprCharacter(actualWord[i][0]);
				else
					actualWord[i][0] = lwrCharacter(actualWord[i][0]);

				bNonDict = true;
				break;
			}
		}
	}

	if(bNonDict && !isEmptyStr(actualWord[0]) && !isEmptyStr(actualWord[1]))
	{
		Add2LearningNoDictToCache(actualWord);
		return;
	}

	if(nodeCount == N2Gram)
	{
		//ShowInfo("MK Learn3Gram Set3Learning:(%d)\n", index);
		Add2LearningToCache(node, actualWord);	
	}

}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void NGramLearning::Learn3Gram(MYWCHAR word[MAX_TGRAPH_HISTORY_NUM][MAX_WORD_LEN], bool bulk)
{
	int nodeCount =0;
	CompactNode* node[N3Gram];	
	MYWCHAR wordInput[MAX_WORD_LEN];

	bool bNonDict = false;
	for(int i=0; i < N3Gram;i++)
	{
		node[i] = NULL;
		if(isEmptyStr(word[i]))
			break;
		
		mywcscpy(wordInput, word[i]);
		//MakeLowerCase(wordInput);

		ReplaceiToI(word[i]);
		ReplaceiToI(wordInput);

		int dictIdx = 0;
		CompactNode* endnode = wpDictManager->retrieveEndNodeForString(wordInput, &dictIdx, true);
		if(endnode)
		{
			node[i] = endnode;
			nodeCount++;
		}
		else
		{
			bool bPreviousUpper = isUpperCase(word[i][0]);
			node[i] = CheckWordForCases(word[i], &nodeCount);
			if(!node[i])
			{
				if(bPreviousUpper)
					word[i][0] = uprCharacter(word[i][0]);
				else
					word[i][0] = lwrCharacter(word[i][0]);

				bNonDict = true;
				break;
			}
		}
	}

	if(bNonDict)
		return;
	
	if(nodeCount == N3Gram)
	{
		//ShowInfo("MK Learn3Gram Set3Learning:(%d)\n", index);
		Add3LearningToCache(node, word, bulk);	
	}
	
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void NGramLearning::Add2LearningToCache(CompactNode* node[N2Gram], MYWCHAR word[MAX_TGRAPH_HISTORY_NUM][MAX_WORD_LEN])
{
	int alphaInd = GetNGramLearningAlphabetIndex(node[0], word[0]);
	if(alphaInd == MAX_ALPHABET_SIZE)
		return;

	m_2Cache[alphaInd].Add2Learning(node, word);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void NGramLearning::Add2LearningNoDictToCache(MYWCHAR word[MAX_TGRAPH_HISTORY_NUM][MAX_WORD_LEN])
{
	int alphaInd = GetNGramLearningAlphabetIndex(NULL, word[0]);
	if(alphaInd == MAX_ALPHABET_SIZE)
		return;

	m_2CacheNoDict[alphaInd].Add2LearningNoDict(word);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void NGramLearning::Add3LearningToCache(CompactNode* node[N3Gram], MYWCHAR word[MAX_TGRAPH_HISTORY_NUM][MAX_WORD_LEN], bool bulk)
{
	int alphaInd = GetNGramLearningAlphabetIndex(node[0], word[0]);
	if(alphaInd == MAX_ALPHABET_SIZE)
		return;

	m_3Cache[alphaInd].Add3Learning(node, word, 0, bulk);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void NGramLearning::Add4LearningToCache(CompactNode* node[N4Gram], MYWCHAR word[MAX_TGRAPH_HISTORY_NUM][MAX_WORD_LEN], bool bulk)
{
	int alphaInd = GetNGramLearningAlphabetIndex(node[0], word[0]);
	if(alphaInd == MAX_ALPHABET_SIZE)
		return;

	m_4Cache[alphaInd].Add4Learning(node, word, 0, bulk);
}	
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void NGramLearning::Learn4Gram(MYWCHAR word[MAX_TGRAPH_HISTORY_NUM][MAX_WORD_LEN], bool bulk)
{
	const int nMax = N4Gram;
	int nodeCount =0;
	CompactNode* node[nMax];
	MYWCHAR wordInput[MAX_WORD_LEN];
	bool bNonDict = false;

	for(int i=0; i < nMax;i++)
	{
		node[i] = NULL;
		if(isEmptyStr(word[i]))
			break;

		mywcscpy(wordInput, word[i]);
		//MakeLowerCase(wordInput);
		
		ReplaceiToI(word[i]);
		ReplaceiToI(wordInput);

		int dictIdx = 0;
		CompactNode* endnode = wpDictManager->retrieveEndNodeForString(wordInput, &dictIdx, true);
		if(endnode)
		{
			node[i] = endnode;
			nodeCount++;
		}	
		else
		{
			bool bPreviousUpper = isUpperCase(word[i][0]);
			node[i] = CheckWordForCases(word[i], &nodeCount);
			if(!node[i])
			{
				if(bPreviousUpper)
					word[i][0] = uprCharacter(word[i][0]);
				else
					word[i][0] = lwrCharacter(word[i][0]);

				bNonDict = true;
				break;
			}
		}
	}
	if(bNonDict)
		return;

	if(nodeCount == N4Gram)
	{
		//ShowInfo("MK Learn3Gram Set3Learning:(%d)\n", index);
		Add4LearningToCache(node, word, bulk);	
	}
	
	MYWCHAR tempWord[nMax][MAX_WORD_LEN];
	memset(tempWord,0,sizeof(tempWord));
	//for(int i=0; i < nMax-1;i++)
	//{
	//	mywcscpy(tempWord[i],word[i]);
	//}

	//Learn3Gram(tempWord);

	//if phrase is "A,B,C,D", need to learn "A,B,C,D" and "B,C,D". "A,B,C" has already been learned in previous Learn3Gram.
	int idx =0;
	for(int i=1; i < nMax;i++)
	{
		mywcscpy(tempWord[idx],word[i]);
		idx++;
	}
	Learn3Gram(tempWord, bulk);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int NGramLearning::Find2LearningWord(MYWCHAR secondWord[MAX_TGRAPH_SIZE][MAX_WORD_LEN], USHORT learnedPref[MAX_TGRAPH_SIZE],
		Dictionary* dict, CompactNode* p2GramStartNode, MYWCHAR* cmpWord, MYWCHAR* N2StartWord, bool bIgnorePref)
{
	if(!dict || !p2GramStartNode )
		return 0;

	int count = 0;
	int alphaInd = GetNGramLearningAlphabetIndex(p2GramStartNode, NULL);
	if(alphaInd == MAX_ALPHABET_SIZE)
		return 0;

	USHORT checkPref =0;
	if(!bIgnorePref)
		checkPref = MIN_NGRAM_LEARNING_FREQUENCY/7;

	for(int i=0; i <MAX_NGRAM_LEARNING;i++)
	{
		NGramLearningCache* p2Cache = &m_2Cache[alphaInd];

		if(!p2Cache || p2Cache->IsEmpty(i, N2Gram))
			break;
		
		if(p2Cache->st2Gram[i].p2GramNode[0] == p2GramStartNode  && p2Cache->st2Gram[i].p2GramNode[1] != NULL
			&& p2Cache->st2Gram[i].learnPref2 > checkPref)
		{
			for(int k=0; k < MAX_TGRAPH_SIZE;k++)
			{
				if(!isEmptyStr(secondWord[k]))
					continue;
				
				MYWCHAR temp[MAX_WORD_LEN];
				dict->getCompactStore()->retrieveWordFromLastNode(p2Cache->st2Gram[i].p2GramNode[1], temp);

				if(IsDuplicateWord(secondWord, temp))
					continue;

				bool bAllow = true;
				if(cmpWord)
				{
					for(int i=0; i < MAX_WORD_LEN && cmpWord[i]; i++)
					{
						if(cmpWord[i] != temp[i])
						{
							bAllow = false;
							break;
						}
					}
				}

				if(bAllow)
				{
					mywcscpy(secondWord[k], temp);
					learnedPref[k] = p2Cache->st2Gram[i].learnPref2;
					if(p2Cache->st2Gram[i].b2FirstUpperCase[1])
						secondWord[k][0] = uprCharacter(secondWord[k][0]);
										
					count++;

				}
				break;
			}
		}
	}

	return count;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int NGramLearning::Find2LearningNoDicWord(MYWCHAR* N2StartWord, MYWCHAR secondWord[MAX_TGRAPH_SIZE][MAX_WORD_LEN], USHORT learnedPref[MAX_TGRAPH_SIZE], 
										  MYWCHAR* cmpWord, bool bIgnorePref)
{
	int alphaInd = GetNGramLearningAlphabetIndex(NULL, N2StartWord);
	if(alphaInd == MAX_ALPHABET_SIZE)
		return 0;

	int count =0;
	for(int i=0; i <MAX_NGRAM_LEARNING;i++)
	{
		N2GramLearningNoDict* p2CacheNoDict = &m_2CacheNoDict[alphaInd];

		if(!p2CacheNoDict || p2CacheNoDict->IsEmptyNoDict(i))
			break;

		MYWCHAR* curRoot = p2CacheNoDict->st2GramNoDict[i].arr2GramString[0];
		MYWCHAR* nextWord = p2CacheNoDict->st2GramNoDict[i].arr2GramString[1];

		USHORT checkPref =0;
		if(!bIgnorePref)
			checkPref = MIN_NGRAM_LEARNING_FREQUENCY/7;

		if(mywcscmp(N2StartWord, curRoot) == 0
			&& p2CacheNoDict->st2GramNoDict[i].learnPref2 > checkPref)
		{
			for(int k=0; k < MAX_TGRAPH_SIZE;k++)
			{
				if(!isEmptyStr(secondWord[k]))
					continue;
				
				if(IsDuplicateWord(secondWord, nextWord))
					continue;

				bool bAllow = true;
				if(cmpWord)
				{
					for(int i=0; i < MAX_WORD_LEN && cmpWord[i]; i++)
					{
						if(cmpWord[i] != nextWord[i])
						{
							bAllow = false;
							break;
						}
					}
				}

				if(bAllow)
				{
					mywcscpy(secondWord[k], nextWord);
					learnedPref[k] = p2CacheNoDict->st2GramNoDict[i].learnPref2;									
					count++;
				}	
				break;
			}
		}
	}

	return count;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool NGramLearning::IsDuplicateWord(MYWCHAR arr[MAX_TGRAPH_SIZE][MAX_WORD_LEN], MYWCHAR* compWord)
{

	if(isEmptyStr(compWord))
		return false;

	MYWCHAR temp[MAX_WORD_LEN];
	MYWCHAR temp2[MAX_WORD_LEN];

	for(int k=0; k <NMWORDPREDICTIONS; k++)
	{
		if(isEmptyStr(arr[k]))
			break;
		
		mywcscpy(temp, arr[k]);
		trimEndingSpaces(temp, true);

		mywcscpy(temp2,compWord);
		trimEndingSpaces(temp2, true);
		if(mywcscmp(temp2,temp)==0)
			return true;	
	}

	return false;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int NGramLearning::Find3LearningWord(MYWCHAR thirdWord[MAX_TGRAPH_SIZE][MAX_WORD_LEN], USHORT learnedPref[MAX_TGRAPH_SIZE],
		Dictionary* dict, CompactNode* p3GramStartNode, CompactNode* p2GramStartNode)
{
	if(!dict || !p3GramStartNode || !p2GramStartNode )
		return 0;

	int count = 0;
	int alphaInd = GetNGramLearningAlphabetIndex(p3GramStartNode, NULL);
	if(alphaInd == MAX_ALPHABET_SIZE)
		return 0;

	for(int i=0; i <MAX_NGRAM_LEARNING;i++)
	{
		NGramLearningCache* p3Cache = &m_3Cache[alphaInd];

		if(!p3Cache || p3Cache->IsEmpty(i, N3Gram))
			break;
		
		if(p3Cache->st3Gram[i].p3GramNode[0] == p3GramStartNode && p3Cache->st3Gram[i].p3GramNode[1] == p2GramStartNode && p3Cache->st3Gram[i].p3GramNode[2] != NULL
			&& p3Cache->st3Gram[i].learnPref3 > MIN_NGRAM_LEARNING_FREQUENCY)
		{
			for(int k=0; k < MAX_TGRAPH_SIZE;k++)
			{

				if(isEmptyStr(thirdWord[k]))
				{
					dict->getCompactStore()->retrieveWordFromLastNode(p3Cache->st3Gram[i].p3GramNode[2],thirdWord[k]);
					learnedPref[k] = p3Cache->st3Gram[i].learnPref3;
					if(p3Cache->st3Gram[i].b3FirstUpperCase[2])
						thirdWord[k][0] = uprCharacter(thirdWord[k][0]);
										
					count++;
					break;
				}
			}
		}
	}

	return count;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int NGramLearning::Find3LearningPhrase(MYWCHAR* newInput, CompactNode* startNode, Dictionary* dict, PhraseWords* stWords)
{
	if(!newInput || isEmptyStr(newInput) || !dict || !startNode)
		return 0;

	int count =0;
	int alphaInd = GetNGramLearningAlphabetIndex(startNode, NULL);
	if(alphaInd == MAX_ALPHABET_SIZE)
		return 0;

	for(int i=0; i <MAX_NGRAM_LEARNING;i++)
	{
		NGramLearningCache* p3Cache = &m_3Cache[alphaInd];
		if(!p3Cache || p3Cache->IsEmpty(i, N3Gram))
			break;
		
		//ShowInfo("MK Learning Phrase3:%hu\n", p3Learning->learnPref);
		if(p3Cache->st3Gram[i].p3GramNode[0] == startNode && p3Cache->st3Gram[i].learnPref3 > MIN_NGRAM_LEARNING_FREQUENCY)
		{
			for(int k=0; k < MAX_PHRASE_ALLOWED;k++)
			{
				if(isEmptyStr(stWords->ar1Gram[k]) && p3Cache->st3Gram[i].p3GramNode[1] && p3Cache->st3Gram[i].p3GramNode[2])
				{
					int len2 = dict->getCompactStore()->retrieveWordFromLastNode(p3Cache->st3Gram[i].p3GramNode[1], stWords->ar2Gram[k]);
					int len3 = dict->getCompactStore()->retrieveWordFromLastNode(p3Cache->st3Gram[i].p3GramNode[2], stWords->ar3Gram[k]);
					if(p3Cache->st3Gram[i].b3FirstUpperCase[1] && len2 > 0)
						stWords->ar2Gram[k][0] = uprCharacter(stWords->ar2Gram[k][0]);
					if(p3Cache->st3Gram[i].b3FirstUpperCase[2] && len3 > 0)
						stWords->ar3Gram[k][0] = uprCharacter(stWords->ar3Gram[k][0]);

					if(len2 > 0 && len3 > 0)
					{
						mywcscpy(stWords->ar1Gram[k], newInput);
						//ShowInfo("MK Learning Phrase3:(%s) (%s) (%s)\n", toA(stWords->_1Gram[k]), toA(stWords->_2Gram[k]), toA(stWords->_3Gram[k]));
						stWords->phrasePref[k] = p3Cache->st3Gram[i].learnPref3; 
						count++;
					}
					else
					{
						stWords->ResetThis(k);
					}
					break;
						
				}
			}
		}
	}

	return count;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int NGramLearning::Find4LearningWord(MYWCHAR fourthWord[MAX_TGRAPH_SIZE][MAX_WORD_LEN], USHORT learnedPref[MAX_TGRAPH_SIZE],
		Dictionary* dict, CompactNode* p4GramStartNode, CompactNode* p3GramStartNode, CompactNode* p2GramStartNode)
{
	if(!dict || !p4GramStartNode || !p3GramStartNode || !p2GramStartNode )
		return 0;

	int count =0;
	int alphaInd = GetNGramLearningAlphabetIndex(p4GramStartNode, NULL);
	if(alphaInd == MAX_ALPHABET_SIZE)
		return 0;

	for(int i=0; i <MAX_NGRAM_LEARNING;i++)
	{
		NGramLearningCache* p4Cache = &m_4Cache[alphaInd];

		if(!p4Cache || p4Cache->IsEmpty(i, N4Gram))
			break;
		
		if(p4Cache->st4Gram[i].p4GramNode[0] == p4GramStartNode && p4Cache->st4Gram[i].p4GramNode[1] == p3GramStartNode && p4Cache->st4Gram[i].p4GramNode[2] == p2GramStartNode
			&& p4Cache->st4Gram[i].p4GramNode[3] != NULL && p4Cache->st4Gram[i].learnPref4 > MIN_NGRAM_LEARNING_FREQUENCY)
		{
			for(int k=0; k < MAX_TGRAPH_SIZE;k++)
			{
				if(isEmptyStr(fourthWord[k]))
				{
					dict->getCompactStore()->retrieveWordFromLastNode(p4Cache->st4Gram[i].p4GramNode[3],fourthWord[k]);
					learnedPref[k] = p4Cache->st4Gram[i].learnPref4;
					if(p4Cache->st4Gram[i].b4FirstUpperCase[3])
						fourthWord[k][0] = uprCharacter(fourthWord[k][0]);
					
					count++;
					break;
				}
			}
		}
	}

	return count;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int NGramLearning::Find4LearningPhrase(MYWCHAR* newInput, CompactNode* startNode, Dictionary* dict, PhraseWords* stWords)
{
	if(!newInput || isEmptyStr(newInput) || !dict || !startNode)
		return 0;

	int count =0;
	int alphaInd = GetNGramLearningAlphabetIndex(startNode, NULL);
	if(alphaInd == MAX_ALPHABET_SIZE)
		return 0;

	for(int i=0; i <MAX_NGRAM_LEARNING;i++)
	{
		NGramLearningCache* p4Cache = &m_4Cache[alphaInd];
		if(!p4Cache || p4Cache->IsEmpty(i, N4Gram))
			break;
		
		//ShowInfo("MK Learning Phrase4:%hu\n", p4Learning->learnPref);
		if(p4Cache->st4Gram[i].p4GramNode[0] == startNode && p4Cache->st4Gram[i].learnPref4 > MIN_NGRAM_LEARNING_FREQUENCY)
		{
			for(int k=0; k < MAX_PHRASE_ALLOWED;k++)
			{
				if(isEmptyStr(stWords->ar1Gram[k]) && p4Cache->st4Gram[i].p4GramNode[1] && p4Cache->st4Gram[i].p4GramNode[2]
				&& p4Cache->st4Gram[i].p4GramNode[3])
				{
					int len2 = dict->getCompactStore()->retrieveWordFromLastNode(p4Cache->st4Gram[i].p4GramNode[1], stWords->ar2Gram[k]);
					int len3 = dict->getCompactStore()->retrieveWordFromLastNode(p4Cache->st4Gram[i].p4GramNode[2], stWords->ar3Gram[k]);
					int len4 = dict->getCompactStore()->retrieveWordFromLastNode(p4Cache->st4Gram[i].p4GramNode[3], stWords->ar4Gram[k]);
					if(p4Cache->st4Gram[i].b4FirstUpperCase[1] && len2 > 0)
						stWords->ar2Gram[k][0] = uprCharacter(stWords->ar2Gram[k][0]);
					if(p4Cache->st4Gram[i].b4FirstUpperCase[2] && len3 > 0)
						stWords->ar3Gram[k][0] = uprCharacter(stWords->ar3Gram[k][0]);
					if(p4Cache->st4Gram[i].b4FirstUpperCase[3] && len4 > 0)
						stWords->ar4Gram[k][0] = uprCharacter(stWords->ar4Gram[k][0]);

					if(len2 > 0 && len3 > 0 && len4 > 0)
					{
						mywcscpy(stWords->ar1Gram[k], newInput);
						//ShowInfo("MK Learning Phrase4---:%hu\n", p4Learning->learnPref);
						stWords->phrasePref[k] = p4Cache->st4Gram[i].learnPref4; 
						count++;
					}
					else
					{
						stWords->ResetThis(k);
					}
					break;
				}
			}
		}
	}
	return count;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void NGramLearningCache::Add2Learning(CompactNode* node[N2Gram], MYWCHAR word[MAX_TGRAPH_HISTORY_NUM][MAX_WORD_LEN])
{
	USHORT curPref = 0;
	for(int i=0; i<MAX_NGRAM_LEARNING; i++)
	{
		if(IsSameNodes2(i, node))
		{
			st2Gram[i].learnPref2 = min(st2Gram[i].learnPref2+LEARNED_PHRASE_PREF, MAX_NGRAM_LEARNING_FREQUENCY);
			CopyUpperCase(word, N2Gram, i);
			break;
		}
		if(IsEmpty(i, N2Gram))
		{
			SetNodes2(i, node);
			CopyUpperCase(word, N2Gram, i);

			curPref = min(st2Gram[i].learnPref2+LEARNED_PHRASE_PREF, MAX_NGRAM_LEARNING_FREQUENCY);

			st2Gram[i].learnPref2 = curPref;

			count2++;
			countSinceSorted2++;

			if(curPref < st2Gram[lowestPrefIdx2].learnPref2)
				lowestPrefIdx2 = i;

			if(countSinceSorted2 >( MAX_NGRAM_LEARNING/10))
			{
				qsort(st2Gram, count2, sizeof(N2GramLearningBlk), N2GramLearningCompare);
				countSinceSorted2 = 0;
			}
			break;
		}
	}

	// cache is too full! purge out last 1/8 of its capacity based on lowest prefs:
	if(count2 >= (MAX_NGRAM_LEARNING-1))
	{
		qsort(st2Gram, count2, sizeof(N2GramLearningBlk), N2GramLearningCompare);
		int num2Purge = MAX_NGRAM_LEARNING/8;
		for(int i = 0; i<num2Purge; i++)
		{
			int cnt = count2-1-i;
			Reset(cnt, N2Gram);

		}
		count2 = count2 - num2Purge;
	}
}	



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void NGramLearningCache::Add3Learning(CompactNode* node[N3Gram], MYWCHAR word[MAX_TGRAPH_HISTORY_NUM][MAX_WORD_LEN], 
									  USHORT prefFromFile, bool bBulkLoading)
{
	USHORT addPref = 0;
	if(!bBulkLoading)
		addPref = LEARNED_PHRASE_PREF;
	else
		addPref = BULKLOADING_PHRASE_PREF;


	USHORT curPref = 0;
	for(int i=0; i<MAX_NGRAM_LEARNING; i++)
	{
		if(IsSameNodes3(i, node))
		{
			st3Gram[i].learnPref3 = min(st3Gram[i].learnPref3+LEARNED_PHRASE_PREF, MAX_NGRAM_LEARNING_FREQUENCY);
			CopyUpperCase(word, N3Gram, i);
			break;
		}
		if(IsEmpty(i, N3Gram))
		{
			SetNodes3(i, node);
			if(prefFromFile ==0)
				curPref = min(st3Gram[i].learnPref3+addPref, MAX_NGRAM_LEARNING_FREQUENCY);
			else
				curPref = prefFromFile;
				 
			st3Gram[i].learnPref3 = curPref;

			CopyUpperCase(word, N3Gram, i);

			count3++;
			countSinceSorted3++;
			countSinceLastSaved3++;

			if(curPref < st3Gram[lowestPrefIdx3].learnPref3)
				lowestPrefIdx3 = i;

			if(countSinceSorted3 >( MAX_NGRAM_LEARNING/10))
			{
				qsort(st3Gram, count3, sizeof(N3GramLearningBlk), N3GramLearningCompare);
				countSinceSorted3 = 0;
			}
			if(countSinceLastSaved3 > MAX_NGRAM_LEARNING/5)
			{
				SaveOutCachedGramLearning(N3Gram, bBulkLoading);
				countSinceLastSaved3 = 0;
			}
			break;
		}
	}

	//cache is too full! purge out last 1/8 of its capacity based on lowest prefs:
	if(count3 >= (MAX_NGRAM_LEARNING-1))
	{
		qsort(st3Gram, count3, sizeof(N3GramLearningBlk), N3GramLearningCompare);
		int num2Purge = MAX_NGRAM_LEARNING/8;
		for(int i = 0; i<num2Purge; i++)
		{
			int cnt = count3-1-i;
			Reset(cnt, N3Gram);

		}
		count3 = count3 - num2Purge;
	}
}	
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void NGramLearningCache::Add4Learning(CompactNode* node[N4Gram], MYWCHAR word[MAX_TGRAPH_HISTORY_NUM][MAX_WORD_LEN], 
									  USHORT prefFromFile,  bool bBulkLoading)
{
	USHORT addPref = 0;
	if(!bBulkLoading)
		addPref = LEARNED_PHRASE_PREF;
	else
		addPref = BULKLOADING_PHRASE_PREF;

	USHORT curPref = 0;
	for(int i=0; i<MAX_NGRAM_LEARNING; i++)
	{
		if(IsSameNodes4(i, node))
		{
			st4Gram[i].learnPref4 = min(st4Gram[i].learnPref4+LEARNED_PHRASE_PREF, MAX_NGRAM_LEARNING_FREQUENCY);
			CopyUpperCase(word, N4Gram, i);
			break;
		}
		if(IsEmpty(i, N4Gram))
		{
			SetNodes4(i, node);
			if(prefFromFile == 0)
				curPref = min(st4Gram[i].learnPref4+addPref, MAX_NGRAM_LEARNING_FREQUENCY);
			else
				curPref = prefFromFile;

			st4Gram[i].learnPref4 = curPref;

			CopyUpperCase(word, N4Gram, i);

			count4++;
			countSinceSorted4++;
			countSinceLastSaved4++;

			if(curPref < st4Gram[lowestPrefIdx4].learnPref4)
				lowestPrefIdx4 = i;

			if(countSinceSorted4 >( MAX_NGRAM_LEARNING/10))
			{
				qsort(st4Gram, count4, sizeof(N4GramLearningBlk), N4GramLearningCompare);
				countSinceSorted4 = 0;
			}
			if(countSinceLastSaved4 > MAX_NGRAM_LEARNING/5)
			{
				SaveOutCachedGramLearning(N4Gram, bBulkLoading);
				countSinceLastSaved4 = 0;
			}
			break;
		}
	}

	// cache is too full! purge out last 1/8 of its capacity based on lowest prefs:
	if(count4 >= (MAX_NGRAM_LEARNING-1))
	{
		qsort(st4Gram, count4, sizeof(N4GramLearningBlk), N4GramLearningCompare);
		int num2Purge = MAX_NGRAM_LEARNING/8;
		for(int i = 0; i<num2Purge; i++)
		{
			int cnt = count4-1-i;
			Reset(cnt, N4Gram);

		}
		count4 = count4 - num2Purge;
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int NGramLearningCache::N2GramLearningCompare(const void *a, const void *b)
{
	N2GramLearningBlk *nodeA = (N2GramLearningBlk *)a;
	N2GramLearningBlk *nodeB = (N2GramLearningBlk *)b;

	if(nodeA->learnPref2 > nodeB->learnPref2) return -1;
	if(nodeA->learnPref2 == nodeB->learnPref2) return 0;

	return 1;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int NGramLearningCache::N3GramLearningCompare(const void *a, const void *b)
{
	N3GramLearningBlk *nodeA = (N3GramLearningBlk *)a;
	N3GramLearningBlk *nodeB = (N3GramLearningBlk *)b;

	if(nodeA->learnPref3 > nodeB->learnPref3) return -1;
	if(nodeA->learnPref3 == nodeB->learnPref3) return 0;

	return 1;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int NGramLearningCache::N4GramLearningCompare(const void *a, const void *b)
{
	N4GramLearningBlk *nodeA = (N4GramLearningBlk *)a;
	N4GramLearningBlk *nodeB = (N4GramLearningBlk *)b;

	if(nodeA->learnPref4 > nodeB->learnPref4) return -1;
	if(nodeA->learnPref4 == nodeB->learnPref4) return 0;

	return 1;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool NGramLearningCache::IsEmpty(int i, int type) 
{
	WLBreakIf(i >= MAX_NGRAM_LEARNING, "!!ERROR out of range for NGramLearning!!\n");
	if(type == N2Gram)
	{
		for(int k=0; k <N2Gram; k++)
		{
			if(st2Gram[i].p2GramNode[k])
				return false;	
		}
	}
	else if(type == N3Gram)
	{
		for(int k=0; k <N3Gram; k++)
		{
			if(st3Gram[i].p3GramNode[k])
				return false;	
		}
	}
	else if(type == N4Gram)
	{
		for(int k=0; k <N4Gram; k++)
		{
			if(st4Gram[i].p4GramNode[k])
				return false;	
		}
	}

	return true;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool NGramLearningCache::IsSameNodes2(int i, CompactNode* node[N2Gram])
{
	WLBreakIf(i >= MAX_NGRAM_LEARNING, "!!ERROR out of range for N2GramLearning!!\n");
	for(int k=0; k <N2Gram; k++)
	{
		if(!st2Gram[i].p2GramNode[k] || st2Gram[i].p2GramNode[k] != node[k])
			return false;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool NGramLearningCache::IsSameNodes3(int i, CompactNode* node[N3Gram])
{
	WLBreakIf(i >= MAX_NGRAM_LEARNING, "!!ERROR out of range for N3GramLearning!!\n");
	for(int k=0; k <N3Gram; k++)
	{
		if(!st3Gram[i].p3GramNode[k] || st3Gram[i].p3GramNode[k] != node[k])
			return false;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool NGramLearningCache::IsSameNodes4(int i, CompactNode* node[N4Gram])
{
	WLBreakIf(i >= MAX_NGRAM_LEARNING, "!!ERROR out of range for N4GramLearning!!\n");
	for(int k=0; k <N4Gram; k++)
	{
		if(!st4Gram[i].p4GramNode[k] || st4Gram[i].p4GramNode[k] != node[k])
			return false;
	}

	return true;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void NGramLearningCache::SetNodes2(int i, CompactNode* node[N2Gram])
{
	WLBreakIf(i >= MAX_NGRAM_LEARNING, "!!ERROR out of range for N2GramLearning!!\n");
	for(int k=0; k <N2Gram; k++)
		st2Gram[i].p2GramNode[k] =  node[k];	
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void NGramLearningCache::SetNodes3(int i, CompactNode* node[N3Gram])
{
	WLBreakIf(i >= MAX_NGRAM_LEARNING, "!!ERROR out of range for N3GramLearning!!\n");
	for(int k=0; k <N3Gram; k++)
		st3Gram[i].p3GramNode[k] =  node[k];	
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void NGramLearningCache::SetNodes4(int i, CompactNode* node[N4Gram])
{
	WLBreakIf(i >= MAX_NGRAM_LEARNING, "!!ERROR out of range for N4GramLearning!!\n");
	for(int k=0; k <N4Gram; k++)
		st4Gram[i].p4GramNode[k] =  node[k];	
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void NGramLearningCache::SetFirstUpperCase(int i, int k, bool bUpper, int type)
{
	WLBreakIf(i >= MAX_NGRAM_LEARNING, "!!ERROR out of range for NGramLearning!!\n");
	if(type == N2Gram)
		st2Gram[i].b2FirstUpperCase[k] =  bUpper;	
	else if(type == N3Gram)
		st3Gram[i].b3FirstUpperCase[k] =  bUpper;	
	else if(type == N4Gram)
		st4Gram[i].b4FirstUpperCase[k] =  bUpper;	
}
////////////////////////////////////////////////////////////////////////////////////////////////
void NGramLearningCache::CopyUpperCase(MYWCHAR word[MAX_TGRAPH_HISTORY_NUM][MAX_WORD_LEN], int type, int index)
{
	for(int k=0; k < type;k++)
	{
		if(isUpperCase(word[k][0]))
		{
			SetFirstUpperCase(index, k, true, type);	
		}
		else
		{
			SetFirstUpperCase(index, k, false, type);	
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////
void NGramLearningCache::Reset(int i, int type)
{
	if(type == N2Gram)
	{
		for(int k=0; k < N2Gram; k++)
		{
			st2Gram[i].p2GramNode[k] = NULL;
			st2Gram[i].b2FirstUpperCase[k] = false;
			st2Gram[i].learnPref2 =0;
		}

	}
	else if(type == N3Gram)
	{
		for(int k=0; k < N3Gram; k++)
		{
			st3Gram[i].p3GramNode[k] = NULL;
			st3Gram[i].b3FirstUpperCase[k] = false;
			st3Gram[i].learnPref3 =0;
		}

	}
	else if(type == N4Gram)
	{
		for(int k=0; k < N4Gram; k++)
		{
			st4Gram[i].p4GramNode[k] = NULL;
			st4Gram[i].b4FirstUpperCase[k] = false;
			st4Gram[i].learnPref4 =0;
		}

	}
}

////////////////////////////////////////////////////////////////////////////////////////////////
void  NGramLearningCache::SaveOutCachedGramLearning(int type, bool bBulkLoading)
{
	char* filename = NULL;
	if(CacheFilename3[0] && type == N3Gram)
	{
		filename = createFullPathFileName(CacheFilename3);
	}
	else if(CacheFilename4[0] && type == N4Gram)
	{
		filename = createFullPathFileName(CacheFilename4);
	}

	FILE *fp = fopen(filename, "w");
	if(!fp)
	{
		ShowInfo("MK cannot open cache file:%s\n", filename);
		return;
	}


	MYWCHAR phrase[MAX_PHRASE_LEN];

	if(type == N3Gram)
	{
		for (int i = 0; i < count3; i++)
		{
			memset(phrase, 0, sizeof(phrase));
			for(int k=0; k < N3Gram; k++)
			{
				CompactNode* node = st3Gram[i].p3GramNode[k];
				if(node)
				{
					MYWCHAR *word = wpDictManager->retrieveStringForEndNode(node);
					if(word && !isEmptyStr(word))
					{
						mywcscat(phrase, word);
						AddEndSpace(phrase);
					}
				}
			}

			trimEndingSpaces(phrase);
			char *szPhrase = toA(phrase);
			//ShowInfo("MK SaveOutCache3:%s %d\n", szPhrase, st3Gram[i].learnPref3);
			fprintf(fp, "%s %d\n", szPhrase, st3Gram[i].learnPref3);

			if(bBulkLoading && st3Gram[i].learnPref3 == 0)
			{
				st3Gram[i].learnPref3 =BULKLOADING_PHRASE_PREF;
			}
		}
	}
	else if(type == N4Gram)
	{
		for (int i = 0; i < count4; i++)
		{
			memset(phrase, 0, sizeof(phrase));
			for(int k=0; k < N4Gram; k++)
			{
				CompactNode* node = st4Gram[i].p4GramNode[k];
				if(node)
				{
					MYWCHAR *word = wpDictManager->retrieveStringForEndNode(node);
					if(word && !isEmptyStr(word))
					{
						mywcscat(phrase, word);
						AddEndSpace(phrase);
					}
				}
			}

			trimEndingSpaces(phrase);
			char *szPhrase = toA(phrase);
			//ShowInfo("MK SaveOutCache4:%s %d\n", szPhrase, st4Gram[i].learnPref4);
			fprintf(fp, "%s %d\n", szPhrase, st4Gram[i].learnPref4);

			if(bBulkLoading && st4Gram[i].learnPref4 == 0)
			{
				st4Gram[i].learnPref4 =BULKLOADING_PHRASE_PREF;
			}
		}
	}

	fclose(fp);
	
	
}
////////////////////////////////////////////////////////////////////////////////////////////////
bool NGramLearningCache::LoadGramLearningCacheFromFile(const char*filename, int type)
{
	char* fullPath = NULL;

	if(type == N3Gram)
		strcpy(CacheFilename3, filename);
	else if(type == N4Gram)
		strcpy(CacheFilename4, filename);
	
	fullPath = createFullPathFileName(filename);
	
	FILE *fp = fopen(fullPath, "r");
	if(!fp)
		return false;

	//ShowInfo("MK Learning Opened cache file:%s\n", fullPath);;
	char aLine[MAX_WORD_LEN];
	// reset getLine structures
	bool unicodeFlag = true;
	getLine(0, NULL, unicodeFlag);
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

	
	while (fgets( aLine, MAX_WORD_LEN, fp ) != NULL)
	{
		int lineLen  = (int)strlen(aLine);

		while (lineLen - 1 >= 0 && (aLine[lineLen-1] == '\n' || aLine[lineLen-1] == '\r' || aLine[lineLen-1]== ' '))
		{
			aLine[lineLen-1] = 0;
			lineLen--;
		}

		MYWCHAR * phrase = toW(aLine);
		MYWCHAR wcPref[MAX_WORD_LEN];
		memset(wcPref, 0, sizeof(wcPref));
		int index =0;
		for(int i=0; i < lineLen;i++)
		{
			if(isNumberChar(phrase[i]))
			{
				wcPref[index++] = phrase[i];
				phrase[i]= NUL;
			}
		}

		trimEndingSpaces(phrase);

		int nPref = atoi(toA(wcPref));
		if(nPref > 0)
		{
			AddLearningFromFile(phrase, nPref);
			//ShowInfo("MK Learning ScanCache:%s\n", aLine);
		}

	}


	//ShowInfo("MK Learning Closed cache file:%s\n", fullPath);
	fclose(fp);

	return true;
	
}
////////////////////////////////////////////////////////////////////////////////////////////////
void NGramLearningCache::AddLearningFromFile(MYWCHAR* inputLine, USHORT learnPref)
{
	MYWCHAR word[MAX_TGRAPH_HISTORY_NUM][MAX_WORD_LEN];
	CompactNode* node[N4Gram];
	memset(word, 0, sizeof(word));
	memset(node, 0, sizeof(node));

	int index =0;
	int sp =0;
	for(int i=0; i < MAX_PHRASE_LEN*2 && inputLine[i]; i++)
	{
		if(sp >= MAX_TGRAPH_HISTORY_NUM)
			break;

		if(inputLine[i] == SP && !isNumberChar(inputLine[i]))
		{
			index =0;
			sp++;
			continue;
		}

		if(!isNumberChar(inputLine[i]))
			word[sp][index++] = inputLine[i];

	}
	int dictIdx = 0;
	int alphaInd = -1;
	for(int i=0; i <N4Gram; i++)
	{		
		CompactNode *endnode = wpDictManager->retrieveEndNodeForString(word[i], &dictIdx, true);
		if(!endnode && i < N3Gram)
			return;
		
		if(endnode)
			node[i] = endnode;	

		if(i==0)
			alphaInd = wpDictManager->GetNGramLearning()->GetNGramLearningAlphabetIndex(node[0], word[0]);
	}

	if(alphaInd > 0)
	{
		if(node[N4Gram-1])
		{
			for(int i =0; i <N4Gram; i++)
			{
				if(node[i] == NULL)
					return;
			}
			Add4Learning(node, word, learnPref, false);
		}
		else
		{
			CompactNode* node3[N3Gram];
			for(int i =0; i <N3Gram; i++)
			{
				if(node[i] == NULL)
					return;

				node3[i] = node[i];
			}
			Add3Learning(node3, word, learnPref, false); 
		}
	}		
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool N2GramLearningNoDict::IsEmptyNoDict(int i) 
{
	for(int k=0; k <N2Gram; k++)
	{
		if(!isEmptyStr(st2GramNoDict[i].arr2GramString[k]))
			return false;	
	}

	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void N2GramLearningNoDict::Add2LearningNoDict(MYWCHAR word[MAX_TGRAPH_HISTORY_NUM][MAX_WORD_LEN])
{
	USHORT curPref = 0;
	for(int i=0; i<MAX_NGRAM_LEARNING; i++)
	{
		if(IsSameNodes2NoDict(i, word))
		{
			st2GramNoDict[i].learnPref2 = min(st2GramNoDict[i].learnPref2+LEARNED_PHRASE_PREF, MAX_NGRAM_LEARNING_FREQUENCY);
			break;
		}
		if(IsEmptyNoDict(i))
		{
			SetNodes2NoDict(i, word);
			curPref = min(st2GramNoDict[i].learnPref2+LEARNED_PHRASE_PREF, MAX_NGRAM_LEARNING_FREQUENCY);

			st2GramNoDict[i].learnPref2 = curPref;

			NoDictCount2++;
			NoDictCountSinceSorted2++;

			if(curPref < st2GramNoDict[NoDictLowestPrefIdx2].learnPref2)
				NoDictLowestPrefIdx2 = i;

			if(NoDictCountSinceSorted2 >( MAX_NGRAM_LEARNING/10))
			{
				SortNoDict2();
				NoDictCountSinceSorted2 = 0;
			}
			break;
		}
	}

	// cache is too full! purge out last 1/8 of its capacity based on lowest prefs:
	if(NoDictCount2 >= (MAX_NGRAM_LEARNING-1))
	{
		SortNoDict2();
		int num2Purge = MAX_NGRAM_LEARNING/8;
		for(int i = 0; i<num2Purge; i++)
		{
			int cnt = NoDictCount2-1-i;
			ResetNoDict(cnt);

		}
		NoDictCount2 = NoDictCount2 - num2Purge;
	}
}	
void N2GramLearningNoDict::SortNoDict2()
{

	bool bFlag = true;   
	USHORT nLearnedPref = 0;
	N2GramLearningNoDictBlk temp[MAX_NGRAM_LEARNING];

	for(int i = 1; (i <= MAX_NGRAM_LEARNING) && bFlag; i++)
	{
		bFlag = false;
		for (int j=0; j < (MAX_NGRAM_LEARNING -1); j++)
		{
			if(isEmptyStr(st2GramNoDict[j].arr2GramString[0]))
				break;

			if (st2GramNoDict[j+1].learnPref2 > st2GramNoDict[j].learnPref2)     
			{   
				nLearnedPref = st2GramNoDict[j].learnPref2;
	
				mywcscpy(temp[j].arr2GramString[0], st2GramNoDict[j].arr2GramString[0]);
				mywcscpy(temp[j].arr2GramString[1], st2GramNoDict[j].arr2GramString[1]);

				st2GramNoDict[j].learnPref2 = st2GramNoDict[j+1].learnPref2;
				st2GramNoDict[j+1].learnPref2 = nLearnedPref;

				mywcscpy(st2GramNoDict[j].arr2GramString[0], st2GramNoDict[j+1].arr2GramString[0]);
				mywcscpy(st2GramNoDict[j].arr2GramString[1], st2GramNoDict[j+1].arr2GramString[1]);
	
				mywcscpy(st2GramNoDict[j+1].arr2GramString[0], temp[j].arr2GramString[0]);
				mywcscpy(st2GramNoDict[j+1].arr2GramString[1], temp[j].arr2GramString[1]);
							
				bFlag = true;             
			}
		}
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool N2GramLearningNoDict::IsSameNodes2NoDict(int i, MYWCHAR arrWord[N2Gram][MAX_WORD_LEN])
{
	WLBreakIf(i >= MAX_NGRAM_LEARNING, "!!ERROR out of range for N2GramLearning!!\n");
	for(int k=0; k <N2Gram; k++)
	{
		if(isEmptyStr(st2GramNoDict[i].arr2GramString[k]) || mywcscmp(arrWord[k], st2GramNoDict[i].arr2GramString[k]) != 0 )
			return false;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void N2GramLearningNoDict::SetNodes2NoDict(int i, MYWCHAR arrWord[N2Gram][MAX_WORD_LEN])
{
	WLBreakIf(i >= MAX_NGRAM_LEARNING, "!!ERROR out of range for N2GramLearning!!\n");
	for(int k=0; k <N2Gram; k++)
		mywcscpy(st2GramNoDict[i].arr2GramString[k], arrWord[k]);	
}
////////////////////////////////////////////////////////////////////////////////////////////////
void N2GramLearningNoDict::ResetNoDict(int i)
{
	for(int k=0; k < N2Gram; k++)
	{
		memset(st2GramNoDict[i].arr2GramString[k], 0, sizeof(st2GramNoDict[i].arr2GramString[k]));
		st2GramNoDict[i].learnPref2 =0;
	}

}