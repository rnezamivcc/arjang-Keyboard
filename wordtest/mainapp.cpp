

#include "stdafx.h"
#include "testdictmanager.h"
#include "wordpath.h"
#include <conio.h>
#include "autocorrect.h"
//#include <cctype>
#include <direct.h>

bool ANOT_SP_CR_TAB(char letter) 
{
	return ((letter != ' ') && (letter != '\n') && (letter != '\t') && (letter != '\r') && (letter != 0));
}

int gPosition = 0;
char *getnextword(char *line)
{
	static char sWord[64];
	memset(sWord, 0, 64);
	int currPos = gPosition;
	while(ANOT_SP_CR_TAB(line[currPos]) && (currPos - gPosition) < MAX_WORD_LEN)
		currPos++;
	strncpy(sWord, &line[gPosition], (currPos - gPosition));
	gPosition = currPos+1;
	return sWord;
}
///////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
	MultiLetterAdvanceAr *gnextWordsAr;
	if( argc > 1 && strncmp(argv[1], "-runtest", 8) == 0 )
	{
		CTestDictManager::runTest();
	}
	else  //running in interactive mode. Take dictionary file from command line
	{
		printf("wordtest is command line testing interface to wordlogic prediction engine. \n");
		printf("usage: wordtest -runtest runs internal test and exits.\n");
		printf("       wordtest runs in interactive mode. \n \n");
	//	getchar();

		CTestDictManager *testDict;
		if(argc > 1)
		{
			char buffer[255];           
			// Get the current working directory: 
			if(argc == 2) // this means only dictionary file name has been provided, so use current working dir:
			{
				if( (_getcwd( buffer, 255 )) == NULL )
					perror( "_getcwd error" );
				strcat(buffer, "\\");
			}
			else // means argv[2] is working path:
				strcpy(buffer, argv[2]);
			assert(buffer[0] != NUL);
			strcat(buffer, "\\dictionary\\");
			printf( "Current working directory: %s\n", buffer);
			char* inputfilename = addOn(buffer, ".dict", argv[1]);
			if(FileExists(inputfilename))
				testDict = CTestDictManager::createTestDict( argv[2], argv[1]);
			else
				testDict = CTestDictManager::createTestDict( NULL, argv[1]);
		
			if(!testDict)
			{
				printf("ERROR!! couldn't open %s file!\n", inputfilename);
				return 1;
			}
		}
		else
			testDict = CTestDictManager::createTestDict();


		BOOL startSentence = true;
		BOOL controlState = false;
		BOOL bAdded = FALSE;
		BOOL bDeleted  = FALSE;
		MYWCHAR **nextWords = NULL;
		MYWCHAR *printP = new MYWCHAR[2];
		int *myposAr = NULL;
		MYWCHAR *currword;
		MYWCHAR testchar = NUL;
		MYWCHAR runningPhrase[MAX_WORD_LEN];
	//	MYWCHAR *runningPhraseP;
		BYTE nBackouts = 0;
		while(testchar != HTAB)
		{
			if(mywcslen(runningPhrase) > (MAX_WORD_LEN - 10))
			{
				memset(runningPhrase, 0, MAX_WORD_LEN * sizeof(MYWCHAR));
			//	runningPhraseP = runningPhrase;
			}
			nBackouts = 0;
	
			//testchar = getwchar();
			testchar = _getwch();
			if(testchar == LF || testchar == NUL)
				continue;
			if(testchar == (WCHAR)'~')  // this turns on control state, so we can type in numbers as input not as choosing prediction words!
			{
				controlState = !controlState;
				if(controlState)
					printf("\n Now accepts non alphabetic characters as input!\n");
				else
					printf(" Back to only alphabetic characters!\n");
				continue;
			}

			printP[0] = testchar;
			printP[1] = NUL;
			unsigned len = 0;
		//	MYWCHAR **nextWords;
			if(isdigit(testchar) && !controlState)
			{
				MYWCHAR nextWord[MAX_WORD_LEN];
				int num = testchar - '0';
				if(num >= 0 && num < 5 && gnextWordsAr && gnextWordsAr->nextWords[num])
				{
					mywcscpy(nextWord, gnextWordsAr->nextWords[num]);
					printf("\n @AvanceWord on %d, %s\n", num, toA(nextWord));
					nBackouts = testDict->advanceWord(nextWord, &printP, gnextWordsAr);
				}
				else if(num>=5 && num < 10 && gnextWordsAr && gnextWordsAr->nextWords[num-5])
				{
					mywcscpy(nextWord, gnextWordsAr->nextWords[num-5]);
					printf("\n @AdvanceWordComplete on %d, %s\n", num-5, toA(nextWord));
					nBackouts = testDict->advanceWordComplete(nextWord, &printP, gnextWordsAr);
				}
				currword = printP;
			}
			else if(testchar == BS)
			{
				printf("\n @BackspaceLetter:");
				gnextWordsAr = testDict->m_dictManager->backspaceLetter(currword);
				currword = getCurrentWord(len);
			//	runningPhraseP = runningPhrase;
				nBackouts = 0;
				//_putwch(testchar);
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
			}

			printf("\ncurrword=#%s#\n", toA(currword));
			while(nBackouts--)
				_putwch(BS);
			printf("\n @Nextwords:");
			for (int i=0; i< gnextWordsAr->nActualNexts; i++)
			{
				printf("--%d: #%s#,", i, toA(gnextWordsAr->nextWords[i]));
			}
			printf("\n");

			printf("\n @currword=#%s#\n", toA(currword));
			
			if(mywcslen(printP) > 1)
			{
			//	_putwch(BS);
				_cputws(printP);
			}
			else
				printf("%s", toA(currword));
		}

		testDict->reset();
		delete testDict;
	}

	return 0;
}