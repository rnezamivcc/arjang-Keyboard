#include "StdAfx.h"
#include "UserWordCacheOffline.h"
#include "dictmanager.h"
#include "parsedline.h"

extern CDictManager *wpDictManager;

UserWordCacheOffline::UserWordCacheOffline()
{

}

UserWordCacheOffline::~UserWordCacheOffline()
{

}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool UserWordCacheOffline::ReadOfflineFile(const char *filePath, char *parsingSpec, BOOL unicodeFlag)
{
	MYWCHAR szWOneLine[MAX_PHRASE_LEN*10];
	memset(szWOneLine,0,sizeof(szWOneLine));
  
	MYWCHAR sepChar = SP;
	FILE* fp = fopen(filePath, "r");      
	if(!fp)     
	{ 
		printf("!!!!Bye no file %s exists ProcessLearningOffline \n", filePath);
		return FALSE;    
	}
	
	// reset getLine structures
	getLine(0, NULL, unicodeFlag);
	if (strcmp(parsingSpec,"tabdelimited") == 0)
	{
		sepChar = HTAB;
		parsingSpec = (char *)"sp";
	}

	CParsedLine   *lineParser = new CParsedLine(parsingSpec, 0, sepChar);
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

	int count =1;
	while (getLine(fp,  szWOneLine, unicodeFlag) == TRUE)
	{	
		//ShowInfo("%d. %s\n",count,toA(szWOneLine));
		TokenizePhrase(szWOneLine);
		count++;
	}

	fclose(fp);
	delete lineParser;

   return true;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void UserWordCacheOffline::TokenizePhrase(MYWCHAR* phrase)
{
	OfflineWord stWord;

	int n = mywcslen(phrase);

	int index=0;
	int letterInd =0;
	for(int i=0; i < n; i++)
	{
		if(phrase[i] == SP)
		{
			letterInd =0;
			index++;
			continue;
		}
		stWord.word[index][letterInd] = phrase[i];
		stWord.word[index][letterInd] = lwrCharacter(stWord.word[index][letterInd]);
		letterInd++;
		if(isPunctuation(phrase[i]))
		{
			stWord.bEndPhrase[index] = true;
		}
	}
	
	LearningOffline(&stWord);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void UserWordCacheOffline::LearningOffline(OfflineWord* stWord)
{
	int index =0;
	MYWCHAR inputPhrase[MAX_TGRAPH_HISTORY_NUM][MAX_WORD_LEN];
	memset(inputPhrase,0,sizeof(inputPhrase));

	for(int i=0; i < MAX_FOUND_PHRASES;i++)
	{
		if(isEmptyStr(stWord->word[i]))
			break;
		if(index == MAX_TGRAPH_HISTORY_NUM)
		{
			for(int k=0; k < MAX_TGRAPH_HISTORY_NUM; k++)
			{
				if(k== MAX_TGRAPH_HISTORY_NUM-1)
					memset(inputPhrase[k],0,sizeof(inputPhrase[k]));
				else
					mywcscpy(inputPhrase[k], inputPhrase[k+1]);	
			}
			index--;
		}

		for(int k=0; k < MAX_WORD_LEN && stWord->word[i][k] != NUL;k++)
		{
			if(!isNumber(stWord->word[i]) && !isPunctuation(stWord->word[i][k]))
				inputPhrase[index][k] = stWord->word[i][k];
		}

	
		MYWCHAR* thisWord = inputPhrase[index];
		//bool bEnd = stWord->bEndPhrase[i];
		bool bNum = isNumber(thisWord);
		bool bEmptyStr = isEmptyStr(thisWord);
		ReplaceiToI(thisWord);

		//int dictIdx = 0;
		//CompactNode* endnode = wpDictManager->retrieveEndNodeForString(thisWord, &dictIdx, true);

		//if(!bEmptyStr && !bNum && !wpDictManager->learnCurrentWord(thisWord, bEnd))
		if(!bEmptyStr && !bNum)
		{	
			wpDictManager->learnCurrentWord(thisWord, true);
		}
		
		if(index+1 == N3Gram && !bNum && !bEmptyStr)
		{
			if(wpDictManager->GetNGramLearning())
				wpDictManager->GetNGramLearning()->Learn3Gram(inputPhrase, true);
			
		}

		if(index+1 == N4Gram && !bNum && !bEmptyStr)
		{
			if(wpDictManager->GetNGramLearning())
				wpDictManager->GetNGramLearning()->Learn4Gram(inputPhrase, true);
			
		}
		index++;

		if(stWord->bEndPhrase[i])
		{
			index =0;
			memset(inputPhrase,0,sizeof(inputPhrase));
		}
	}
}