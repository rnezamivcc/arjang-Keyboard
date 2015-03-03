

#include "stdafx.h"
#include "testdictmanager.h"
#include <conio.h>
//#include "utility.h"
#include "autocorrect.h"
//#include <time.h>
#include <direct.h>

//////////////////////////////////////////////////////
char* randomword() 
{
	char ret[200];	
	srand (time(NULL));;
	int l = 1 + (rand() % 5);

	for(int c = 0;c < l;c++) 
	{
		ret[c] = (char)('a' + (rand()%26));
	}

	return ret;
}
////////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) 
{
	unsigned int c, cb;
	int nsug;

	CTestDictManager *testDict =  CTestDictManager::createTestDict();
	assert(testDict);

//	char *fullPathFileName = createFullPathFileName("english.aac", NULL);
	AutoCorrect *autocorrect = testDict->m_dictManager->getAutocorrect();

	if(autocorrect == NULL) 
	{
		printf("!!Failed to initialize AutoCorrect! return\n");
		return 0;
	}
	
	if( argc > 1 && strncmp(argv[1], "-auto", 8) == 0 )
	{
		for(c = 0; c < 100000; c++) 
		{
			char * s= randomword();
			MYWCHAR *wWord = toW(s);
			int len = strlen(s);
			printf("main:Trying matches for #%s#\n", s);
			if(len < 2 || autocorrect->isWord(wWord))
				continue;
			for(int i = 0; i<len; i++)
			{
				autocorrect->addChar(wWord[i]);
				CorrectionList* preds = autocorrect->getPredictions(5);
				for(int i = 0; i < preds->fillCount>0; i++) 
					ShowInfo("main: add correction %s,\n", toA(preds->corrections[i].word));
			
			//	if(autocorrect->correctionPending())
				{
			//		autocorrect->addChars(toW(", "), 2);
				}
			}
			autocorrect->addChar(SP);
			CorrectionList* sug = autocorrect->getPredictions(5);
			printf("\nSuggested corrections:\n");
			for(int i = 0; i < sug->fillCount>0; i++) 
			{
				printf("%s, ", toA(sug->corrections[i].word));
			}

		}
		return 0;
	}
	
	//else // running manual test
	MYWCHAR testchar = NUL;
	BOOL startSentence = true;
	MYWCHAR runningPhrase[MAX_WORD_LEN];
	BYTE nBackouts = 0;
	BOOL controlState = false;
	MYWCHAR *printP = new MYWCHAR[2];

	while(testchar != HTAB) {
		if(mywcslen(runningPhrase) > (MAX_WORD_LEN - 10))
		{
			memset(runningPhrase, 0, MAX_WORD_LEN * sizeof(MYWCHAR));
		//	runningPhraseP = runningPhrase;
		}
		nBackouts = 0;

	//	testchar = getwchar();
		testchar = _getwch();
		if(testchar == LF || testchar == NUL)
			continue;

		if(testchar == (WCHAR)'~')  // this turns on control state, so we can type in numbers as input for choosing correction choices!
		{
			controlState = !controlState;
			if(controlState)
				printf("\n Now accepts non alphabetic characters as input!\n");
			else
				printf(" Back to only alphabetic characters!\n");
			continue;
		}

		MultiLetterAdvanceAr *gnextWordsAr;
		CorrectionList *gCList;
		MYWCHAR *currword;
		printP[0] = testchar;
		printP[1] = NUL;
		unsigned len = 0;
		if(isdigit(testchar) && controlState)
		{
			MYWCHAR nextWord[MAX_WORD_LEN];
			int num = testchar - '0';
			if(num >= 0 && num < 5 && gCList && num < gCList->fillCount && gnextWordsAr && gnextWordsAr->nextWords[num])
			{
				mywcscpy(nextWord, gCList->corrections[num].word);
				printf("\n @AvanceWord on %d, %s\n", num, toA(nextWord));
				nBackouts = testDict->advanceWord(nextWord, &printP, gnextWordsAr);
				autocorrect->reverseCorrection(3, mywcslen(nextWord), 4, nextWord);
			}
			else if(num>=5 && num < 10 && gnextWordsAr && gnextWordsAr->nextWords[num-5] && gCList && (num-5) < gCList->fillCount)
			{
				mywcscpy(nextWord, gnextWordsAr->nextWords[num-5]);
				printf("\n @AdvanceWordComplete on %d, %s\n", num-5, toA(nextWord));
				nBackouts = testDict->advanceWordComplete(nextWord, &printP, gnextWordsAr);
				autocorrect->addPrediction(nextWord, mywcslen(nextWord));
			}
			currword = printP;
		}
		else if(testchar == BS)
		{
			printf("\n @BackspaceLetter:");
			gnextWordsAr = testDict->m_dictManager->backspaceLetter(currword);
			currword = getCurrentWord(len);
			printf("current word is #%s#\n", toA(currword));
			autocorrect->backspace(1);

		//	runningPhraseP = runningPhrase;
			nBackouts = 0;
			//_putwch(testchar);

			gCList = autocorrect->getPredictions(5);
			gCList = autocorrect->getCorrections(currword);
			
			ShowInfo("\n: Corrections(after BS): (%d)\n", gCList->fillCount);
			for(int i = 0; i < gCList->fillCount; i++) 
				ShowInfo("%s, ", toA(gCList->corrections[i].word));
			ShowInfo("\n");
		}
		else
		{
			assert(testchar);
			printf("\n @AdvanceLetter on #%s#", toA(printP));
			MYWCHAR input[2] = { testchar, NUL};
			BYTE prefs[1] = {1};
			MYWCHAR root[MAX_WORD_LEN];
			gnextWordsAr = testDict->m_dictManager->advanceMultiLetters(input, prefs, root, &printP, startSentence);
			for(int i=0; i<gnextWordsAr->nActualNexts; i++)
				ShowInfo("(%d):%s,", i, toA(gnextWordsAr->nextWords[i]));
			//testDict->advanceLetter(testchar, &printP, startSentence);
			startSentence = false;
			//_cputws(printP);
		//	runningPhraseP = runningPhrase;
			nBackouts = 0;
			currword = getCurrentWord(len); 

			// do autocorrect now
			autocorrect->addChar(testchar);
			gCList = autocorrect->getPredictions(5);
			printf("\n: Corrections:(%s)\n", toA(currword));
			for(int i = 0; i < gCList->fillCount; i++) 
				printf("%s, ", toA(gCList->corrections[i].word));
			printf("\n");
			autocorrect->correctionPending();
			int test=2;
			if(test==1)
			//	autocorrect->reverseCorrection(3, 4, 4, L"milk");
				autocorrect->addPrediction(L"Li's", 4);
		}
	}
}