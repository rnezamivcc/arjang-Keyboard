// <copyright file="cookedToEnglish.cpp" company="WordLogic Corporation">
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
// <summary>Provides support for building final processed dicitonary based on cooked word list.</summary>
//

#include "StdAfx.h"
//#include "dictsyncer.h"
#include "everbserializer.h"
#include "treatmentruleserializer.h"
#include "compactstore.h"

#define COMPACTDICT

MYWCHAR *englishVariationsOnEVerbs1[] = 
{
	 L"ing"
	,L"ous"
	,L"ity"
	,L"um"
	,L"ation"
	,L"al"
	,L"ative"
	,L"able"
	,L"ance"
	,L"ment"
	,NULL
};

MYWCHAR *englishVariationsOnEVerbs2[] = 
{
	 L"sion"
	,L"ssion"
	,L"ing"
	,NULL
};

EVerbDefinition EnglishEVerbs[] = 
{
	 { L"e", englishVariationsOnEVerbs1,  NUL,  NULL, NULL  } // on any verb
	,{ L"de", englishVariationsOnEVerbs2, NUL,  NULL, NULL  } // on any verb
	, { {NUL }, NULL, NUL, NULL, NULL } // on any verb
};

MYWCHAR EnglishBondingChars[] = L"";

//triggerChar replacementStr maxNullMovements terminatedByOneSpace onHardLetter	        correctPrediction
//     triggerStr  minNullMovements     wholeWord			onSoftLetter    correctDisplay
LangTreatmentRule EnglishTreatmentRules[] =  
{
    // change i into I
	{ SP,	L"i",	L"I",	0, 0, 1, 1}
   ,{ '\'', L"i\'",	L"I\'",	0, 0, 1, 0}
   ,{ SP  , L"im",  L"I\'m",0, 0, 1, 1}
   , { SP , L"ive", L"I\'ve",0, 0, 1, 1}
   , { SP , L"ill", L"I\'ll",0, 0, 1, 1}
    // put the right double quotation to the end of previous word
 //  ,{ L" ",	L" \"",	L"\"",	1, 1, 0,1,FALSE,TRUE}
   , { NUL,	NULL,	NULL,	0, 0, 0,0}
};

#if !defined(_WIN64)
char *buildLoc = ".\\32\\"; 
char *pathldat = "english32.txt";
#else
char *buildLoc = ".\\64\\"; 
char *pathldat = "english64.txt";
#endif

void checkRes(BOOL res, char *filename)
{
	if(res == FALSE)
	{
		printf("!!ERROR!! processing %s!\n", filename);
		exit(1);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
void convertTextToDict(char* filename, bool unicode)
{
	int nSerializedBytes = 0;
	eLanguage langId = findLanguage(filename);
	CDictionaryTree *dictionaryTree =  new CDictionaryTree(langId);
	EVerbSerializer *eVerbSerializer = new EVerbSerializer();
	TreatmentRuleSerializer *treatmentRuleSerializer = new TreatmentRuleSerializer();


	StartWord *startWords = dictionaryTree->processDictFile(filename, "spi",  WORDSIDE_DICTIONARY, NULL,1,0, FALSE, unicode);

	char* phraseFileName = "englishPhrase.txt";
	char* phrase2PhraseFile = "englishP2P.txt";
	dictionaryTree->processAddNewWordFromPhraseFile(phraseFileName, "spi", unicode, false);
	dictionaryTree->processAddNewWordFromPhraseFile(phrase2PhraseFile, "spi", unicode, true);

	char* posTagFileName = "englishPOS.txt";
	dictionaryTree->processPoSTag(posTagFileName, "spi", unicode);
	
	dictionaryTree->collectSuffixes();
	dictionaryTree->locateEVerbs(EnglishEVerbs, TRUE);
	void *serializedData = eVerbSerializer->initialize(EnglishEVerbs, &nSerializedBytes);
	dictionaryTree->setSerializedEVerbsData(serializedData, nSerializedBytes);
	serializedData = treatmentRuleSerializer->initialize(EnglishTreatmentRules, &nSerializedBytes);
	dictionaryTree->setSerializedRuleData(serializedData, nSerializedBytes);
	dictionaryTree->setBondingChars(EnglishBondingChars);

	dictionaryTree->normalizePreferences();
	dictionaryTree->countWords(NULL);

	if (dictionaryTree->compactTree(NULL))
	{
		int numPhrasesProcessed = 0;
	//	dictionaryTree->linkNextsInCompactTree(NULL);
	//	dictionaryTree->linkObjectsInCompactTree(NULL);
	   // first, process phrase file:
		unsigned numPhrases = CDictionaryTree::countNumLinesInFile(phraseFileName);
		
		bool endOfFileReached = false;
		unsigned phrasePageCount = 0;
		while (!endOfFileReached && dictionaryTree->mPhraseHeader->phrasesCount < numPhrases) // max 10 times MAX_PHRASE_ARRAY for phraseEngine memory array is allocated!!
		{
			endOfFileReached = !dictionaryTree->processPhraseFile(phraseFileName, "spi", unicode, numPhrasesProcessed);
			dictionaryTree->setupPhraseEngine(numPhrases, numPhrasesProcessed, phrasePageCount++);
			if(numPhrasesProcessed == 0)
				break;
		}
		
		// process follow-up phrases:
		dictionaryTree->processPhraseFollowupFile(phrase2PhraseFile, "spi", unicode);

		// second, process starting words 1-grame phrases:
		dictionaryTree->processStartingWordsList(startWords);

		// now compact the phrase tree
		dictionaryTree->compactPhraseEngine();
		
		// write out to ldat file:
		char* outputfilename = addOn(buildLoc, LDAT, filename);
		printf("Write output to %s\n", outputfilename);
		dictionaryTree->compactToLDatFile(outputfilename);
	}

	delete startWords;
	delete treatmentRuleSerializer;
	delete eVerbSerializer;
	delete dictionaryTree;
}

////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
	char filename[255] = "english";

	bool unicodeflag = true;
	char *inputfilename = filename;
	if(argc >  1)
	{
		inputfilename = argv[1];
		if(argc > 2)
			unicodeflag = argv[2];
	}
	printf( "Processing %s\n", filename);
	convertTextToDict(mytolower("english.txt"), unicodeflag);
	return 0;
}



