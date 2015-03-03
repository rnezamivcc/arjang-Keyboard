#ifdef WIN32
#include <string.h>
#include <stdio.h>
//#include <unistd.h>
#include <Windows.h>
#include <stdlib.h>
#include <jni.h>
//#include <android/log.h>
#include "dictmanager.h"
#include "userWordCache.h"
#include "userWordCacheOffline.h"
#include "dictionary.h"

#define LENGTHOF(array) (sizeof(array)/sizeof((array)[0]))
//#define WLLOGTrace(...) __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "WLLIBTRACE", __VA_ARGS__)

extern "C" {

static CDictManager *sDictManager = NULL;
bool setDictManager(char *rootPath)
{
	callLog("\nsetDictManager:: setting up with root "); callLog(rootPath); callLog("\n");
	if (!sDictManager)
	{
		sDictManager = new CDictManager();
		if (!sDictManager->Create(NULL, rootPath))
		{
			callLog("!!ERROR!! setDictManager:: failed initializaing! now it's just a dumb keyboard!\n");
			delete sDictManager;
			sDictManager = NULL;
		}
	}
	return sDictManager != NULL;
}
jint fillNextWordsInfo(JNIEnv* env, MultiLetterAdvanceAr *nextWordsAr, jobject nextInfo, MYWCHAR *rootWord);
jint fillNextPhraseInfo(JNIEnv* env, PhraseAr *inPhrases, jobject nextInfo, MYWCHAR *rootWord, int FromWhere);
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
jstring createjstring(JNIEnv* env, MYWCHAR *text)
{
//	char *textA = convWTOA(text);
//	jstring jstr = env->NewStringUTF(textA);
//	delete textA;

	int len = mywcslen(text);
	jstring jstr = env->NewString((jchar *) text, len );

	if (jstr == NULL)
	{
		jclass clazz = env->FindClass("java/lang/OutOfMemoryError");
		env->ThrowNew(clazz,NULL);
	}
	return jstr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JNIEXPORT jboolean Java_com_iknowu_PredictionEngine_ntvInitialize(JNIEnv* env, jobject javaThis, jstring wordPart)
{
	int wordPartLen = env->GetStringLength(wordPart);
	MYWCHAR wordPartW[100];
	env->GetStringRegion(wordPart, 0, wordPartLen, (jchar *) wordPartW);
	wordPartW[wordPartLen]=0;
	callLog("\nntvInitialize: setting up Engine at root "); callLog(toA(wordPartW)); callLog("\n");
	return setDictManager(toA(wordPartW));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
JNIEXPORT jboolean Java_com_iknowu_PredictionEngine_ntvResetDictionaryConfiguration(JNIEnv* env, jobject javaThis)
{
	return sDictManager && sDictManager->resetConfiguration();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
JNIEXPORT void Java_com_iknowu_PredictionEngine_ntvReset(JNIEnv* env, jobject javaThis, jboolean cleanHistory)
{
	if (sDictManager)
	{
		if(cleanHistory)
		{
			callLog("\nntvReset full\n"); 
			sDictManager->fullReset();
		}
		else
		{
			callLog("\nntvReset\n"); 
			sDictManager->reset();
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
JNIEXPORT void Java_com_iknowu_PredictionEngine_ntvLearnFromFile(JNIEnv* env, jobject javaThis, jstring jsFilePath)
{
	//Minkyu: 2013.09.23
	callLog("Start!!ntvLearnFromFile");
	if (sDictManager)
	{
		int nLen = env->GetStringLength(jsFilePath);
		MYWCHAR wPath[1024];
		memset(wPath, 0 , sizeof(wPath));
		env->GetStringRegion(jsFilePath, 0, nLen, (jchar *) wPath);
		wPath[nLen]=0;

		const char *szPath = toA(wPath);

		if(szPath == NULL)
		{
			callLog( "!!!Error! ntvLearnFromFile: Filepath is NULL!\n");
			return;
		}

		ShowInfo("ntvLearn File path = %s\n", szPath);
		sDictManager->ProcessLearningOffline(szPath);	
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
JNIEXPORT jint Java_com_iknowu_PredictionEngine_ntvBackspaceLetter(JNIEnv* env, jobject javaThis, jobject nextInfo)
{
	int nPosWords = 0;
   callLog("\ninside tvBackspaceLetter");
	if(sDictManager)
	{
	    MYWCHAR rootWord[MAX_WORD_LEN];
		memset(rootWord, 0 , sizeof(rootWord));
		MultiLetterAdvanceAr *nextWordsAr = sDictManager->backspaceLetter(rootWord);


		ShowInfo("--ntvBackspaceLetter rootword1:(%s)\n",rootWord);
		int nPosWords = fillNextWordsInfo(env, nextWordsAr, nextInfo, rootWord);
		ShowInfo("--ntvBackspaceLetter rootword2:(%s)\n",rootWord);

		//Minkyu:2013.11.15
		PhraseAr *phraseAr = sDictManager->ProcessPhrasePrediction(rootWord, true);
		int nPhraseWords = fillNextPhraseInfo(env, phraseAr, nextInfo, rootWord,-1);
		if (nPhraseWords < 0)
		{
			printf("!!!Error! tvBackspaceLetter: GetObjectClass or createjstring failed!\n");
			return(0);
		}
		//////////////////////////////////////////////////////////////////

		if (nPosWords < 0)
		{
			printf("!!!Error! tvBackspaceLetter: GetObjectClass or createjstring failed!\n");
			return(0);
		}

		ShowInfo("--ntvBackspaceLetter: num of nextwords=%d\n", nPosWords);
	}
	return nPosWords;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
JNIEXPORT jboolean Java_com_iknowu_PredictionEngine_ntvIsChunk(JNIEnv* env, jobject javaThis, jstring wordPart)
{
   if (sDictManager) 
   {
	   int wordPartLen = env->GetStringLength(wordPart);
	   MYWCHAR wordPartW[MAX_WORD_LEN];
	   env->GetStringRegion(wordPart, 0, wordPartLen, (jchar *) wordPartW);
	   wordPartW[wordPartLen]=0;

	   return sDictManager->isChunk(wordPartW);
   }
   return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
JNIEXPORT jint Java_com_iknowu_PredictionEngine_ntvNumWordsStartingWith(JNIEnv* env, jobject javaThis, jstring wordPart)
{
   if (sDictManager) 
   {
	   int wordPartLen = env->GetStringLength(wordPart);
	   MYWCHAR wordPartW[MAX_WORD_LEN];
	   env->GetStringRegion(wordPart, 0, wordPartLen, (jchar *) wordPartW);
	   wordPartW[wordPartLen]=0;

	   return sDictManager->getNumWordsStartingWith(wordPartW);
   }
   return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JNIEXPORT jint Java_com_iknowu_PredictionEngine_ntvAdvanceMultiLetters(JNIEnv* env, jobject javaThis, 
																	   jstring letters, jbyteArray inprefs, jobject nextInfo, bool begin)
{
   callLog("\ninside ntvAdvanceMultiLetters");
   int nPosBS = 0;

   if (sDictManager) 
   {
		MYWCHAR nextletters[4]; // max 4 letters we process at this time
		int lettersLen = env->GetStringLength(letters);
		ShowInfo("inside ntvAdvanceMultiLetters: lettersLen(%d)\n",lettersLen);

		WLBreakIf(lettersLen > 4, "!!ERROR! in ntvAdvanceMultiLetters: lettersLen (%d) > 8 !\n", lettersLen);
		env->GetStringRegion(letters, 0, lettersLen, (jchar *) nextletters);
		nextletters[lettersLen]=NUL;

	    byte *inPrefs = (byte*)env->GetByteArrayElements(inprefs, NULL);
	   if(inPrefs == NULL)
	   {
		   callLog("!!ntvAdvanceMultiLetters::Could not read prefs array! return!");
		   return 0;
	   }
	   
		MYWCHAR rootWord[MAX_WORD_LEN];
		memset(rootWord, 0, sizeof(rootWord));

		MYWCHAR *printP=NULL;
	    MultiLetterAdvanceAr *nextWordsAr = sDictManager->advanceMultiLetters(nextletters, inPrefs, rootWord, &printP, begin);

		nPosBS = nextWordsAr->nBackSpace;
		int nPosWords = fillNextWordsInfo(env, nextWordsAr, nextInfo, rootWord);

		//Minkyu:2013.11.15
		PhraseAr *phraseAr = sDictManager->ProcessPhrasePrediction(rootWord, false, nextletters);
		int nPhraseWords = fillNextPhraseInfo(env, phraseAr, nextInfo, rootWord, 0);
		if (nPhraseWords < 0)
		{
			printf("!!!Error! ntvAdvanceMultiLetters: GetObjectClass or createjstring failed!\n");
			return(0);
		}

		if (nPosWords < 0)
		{
			printf("!!!Error! ntvAdvanceMultiLetters: GetObjectClass or createjstring failed!\n");
			return(0);
		}

		ShowInfo("inside ntvAdvanceMultiLetters: returned %d words for letters #%s#: at root #%s#, nPosBS(%d):\n", nPosWords, toA(nextletters), toA(rootWord), nPosBS);
		for(int i=0; i<nPosWords; i++)
			ShowInfo("--ntvAdvanceMultiLetters (%d):%s,", i, toA(nextWordsAr->nextWords[i]));

	//	ShowInfo("--ntvAdvanceMultiLetters: rootWord: #%s#, num of next words=%d\n", toA(rootWord), nPosWords);
	}
	return nPosBS;
}
////////////////////////////////////////////////////////////////////////////////////////////////
JNIEXPORT jint Java_com_iknowu_PredictionEngine_ntvSetHistory(JNIEnv* env, jobject javaThis, jstring words, bool backspace)
{
	if (sDictManager) 
	{
		MYWCHAR historyWords[MAX_WORD_LEN]; 

		int historyLength = env->GetStringLength(words);
		ShowInfo("inside ntvSetHistory: historyLength(%d)\n",historyLength);

		env->GetStringRegion(words, 0, historyLength, (jchar *) historyWords);
		historyWords[historyLength]=NUL;

		ShowInfo("inside ntvSetHistory: historyWords(%s)\n",toA(historyWords));
		sDictManager->SetHistoryFromJNI(historyWords,backspace);
		
	}
}
////////////////////////////////////////////////////////////////////////////////////
JNIEXPORT jboolean Java_com_iknowu_PredictionEngine_ntvGetNounResult(JNIEnv* env, jobject javaThis, jstring wordPart)
{
   if (sDictManager) 
   {
	   int wordPartLen = env->GetStringLength(wordPart);
	   MYWCHAR wordPartW[MAX_WORD_LEN];

	   env->GetStringRegion(wordPart, 0, wordPartLen, (jchar *) wordPartW);
	   wordPartW[wordPartLen]=0;

	   ShowInfo("inside ntvGetNounResult:(%s)\n",toA(wordPartW));
	   return sDictManager->GetNounResult(wordPartW);
   }
   return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
JNIEXPORT jint Java_com_iknowu_PredictionEngine_ntvAdvanceLetter(JNIEnv* env, jobject javaThis, 
																	 jchar letter, jobject info, jobject nextInfo)
{
   jclass clazz;
   jfieldID fid;

   callLog("\ninside ntvAdvanceLetter");
    int nPosBS = 0;
  if (sDictManager) 
   {
		//callLog("inside ntvAdvanceLetter: CTestDictManager::sTestDictMgr");	   
		clazz = env->GetObjectClass(info);
		if (clazz == 0 )
		{
			printf("GetObjectClass returned 0\n");
			return JNI_FALSE;
		}
		fid = env->GetFieldID(clazz,"beginSentence","Z");
		BOOL bStartSentence = env->GetBooleanField(info, fid);
		fid = env->GetFieldID(clazz,"literalFlag","Z");
		BOOL bLiteralFlag = env->GetBooleanField(info, fid);
		MYWCHAR str[2] = { letter, NUL };
		BYTE prefs[1] = { 1 };

		MYWCHAR rootWord[MAX_WORD_LEN];
		MYWCHAR *printP;
		MultiLetterAdvanceAr *nextWordsAr = sDictManager->advanceMultiLetters( str, prefs, rootWord, &printP, bStartSentence, bLiteralFlag);
		nPosBS = nextWordsAr->nBackSpace;
		ShowInfo("inside ntvAdvanceLetter: returned %d words for letter #%c#: at root #%s#, nPosBS(%d):\n", nextWordsAr->nActualNexts, char2A(letter), toA(rootWord), nPosBS);

		int nPosWords = fillNextWordsInfo(env, nextWordsAr, nextInfo, rootWord);
		if (nPosWords < 0)
		{
			printf("!!!Error! ntvAdvanceLetter: GetObjectClass or createjstring failed!\n");
			return(0);
		}

		//Minkyu:2013.11.15
		PhraseAr *phraseAr=NULL;
		int nPhraseWords=0;
		if(!isPunctuation(letter))
		{
			ShowInfo("inside ntvAdvanceLetter: not Punctuation\n\n");
			phraseAr = sDictManager->ProcessPhrasePrediction(rootWord);

		}
		else
		{
			ShowInfo("inside ntvAdvanceLetter: isPunctuation\n\n");
			phraseAr = sDictManager->ProcessPhrasePrediction(NULL);
		}

		nPhraseWords =fillNextPhraseInfo(env, phraseAr, nextInfo, rootWord, 1);

		if (nPhraseWords < 0)
		{
			printf("!!!Error! ntvAdvanceLetter: GetObjectClass or createjstring failed!\n");
			return(0);
		}
			
	   //ShowInfo("inside ntvAdvanceLetter: sTetDictMgr returned for letter %c: %s\n", letter, toA(printP));

	   fid = env->GetFieldID(clazz,"printBuffer","Ljava/lang/String;");
	   jstring printBuffer = createjstring(env, printP);
	   env->SetObjectField(info, fid, printBuffer);
	   if (printBuffer)
	   {
		   env->SetObjectField(info, fid, printBuffer);
		   return nPosBS;
	   }
	}
	return nPosBS;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
JNIEXPORT jint Java_com_iknowu_PredictionEngine_ntvAdvanceWord(JNIEnv* env, jobject javaThis, jstring wordPart, 
															   jobject wInfo, jobject nextInfo, jboolean bComplete, jboolean replace)
{
   callLog("\ninside ntvAdvanceWord");

   if (sDictManager) 
   {
	   MYWCHAR *printPartW = NULL;

	   int wordPartLen = env->GetStringLength(wordPart);

		MYWCHAR wordPartW[MAX_WORD_LEN];
	   env->GetStringRegion(wordPart, 0, wordPartLen, (jchar *)wordPartW);
	   wordPartW[wordPartLen]=0;

	   ShowInfo("inside ntvAdvanceWord: wordPart=#%s#, len=%d\n", toA(wordPartW), wordPartLen);
	   MultiLetterAdvanceAr *nextWordsAr=NULL;

	   int numOfLettersBacout = sDictManager->advanceWord(wordPartW, &printPartW, nextWordsAr, bComplete, replace);
	   ShowInfo("inside ntvAdvanceWord: return from sDictManager->advanceWord: numOfLettersBacout= %d, nextWordsAr->nActualNexts=%d\n", numOfLettersBacout, nextWordsAr->nActualNexts);
	   ShowInfo("inside ntvAdvanceWord: bComplete= %d, replace=%d\n", bComplete, replace);

	   	// now record next words:
		MYWCHAR rootWord[MAX_WORD_LEN];
	   int nPosWords = fillNextWordsInfo(env, nextWordsAr, nextInfo, rootWord);

	  
		//Minkyu:2013.11.15
		PhraseAr *phraseAr = sDictManager->ProcessPhrasePrediction(wordPartW);
		int nPhraseWords = fillNextPhraseInfo(env, phraseAr, nextInfo, rootWord, 2);

		if (nPhraseWords < 0)
		{
			printf("!!!Error! ntvAdvanceWord: GetObjectClass or createjstring failed!\n");
			return(0);
		}
		//////////////////////////////////////////////////////////////////

		if (nPosWords < 0)
		{
			printf("!!!Error! ntvAdvanceWord: GetObjectClass or createjstring failed!\n");
			return(0);
		}

	   jclass clazz = env->GetObjectClass(wInfo);
	   if (clazz == 0 )
	   {
			callLog("--!!ERROR! ntvAdvanceWord GetObjectClass returned 0\n");
			return 0;
	   }

	   jstring printBuffer = createjstring(env, printPartW);
	   if (printBuffer == NULL)
		   return 0;
	   
	   jfieldID fid = env->GetFieldID(clazz,"printBuffer","Ljava/lang/String;");
	   env->SetObjectField(wInfo, fid, printBuffer);

	   return numOfLettersBacout;
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
JNIEXPORT jstring Java_com_iknowu_PredictionEngine_ntvNextLetters(JNIEnv* env, jobject javaThis, jintArray posArr)
{
   callLog("\ninside ntvNextLetters");
	if (sDictManager)
	{
		int *myposAr = NULL;
		MYWCHAR *prefLetters = sDictManager->nextLetters(&myposAr);
		jstring jprefletters = createjstring(env, prefLetters);

		int nLetters= mywcslen(prefLetters);
	    env->SetIntArrayRegion(posArr, 0, nLetters, (jint *)myposAr);
		return jprefletters;
	}
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
JNIEXPORT jint Java_com_iknowu_PredictionEngine_ntvGetDictPriority(JNIEnv* env, jobject javaThis, jint idx)
{
   ShowInfo("inside ntvGetDictPriority for idx=%s", idx);
	if (sDictManager)
	{
		DictionaryConfigurationEntry *dcen = sDictManager->GetOrderedDict(idx);
		if(dcen!= NULL)
			return dcen->priority;
	}

	return -1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
JNIEXPORT jint Java_com_iknowu_PredictionEngine_ntvGetDictVersion(JNIEnv* env, jobject javaThis, jint idx)
{
   ShowInfo("inside ntvGetDictVersion for idx=%s", idx);
	if (sDictManager)
	{
		return sDictManager->getDictVersionNumber(idx);
	}

	return -1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
JNIEXPORT jint Java_com_iknowu_PredictionEngine_ntvGetDictBuild(JNIEnv* env, jobject javaThis, jint idx)
{
   ShowInfo("inside ntvGetDictBuild for idx=%s", idx);
	if (sDictManager)
	{
		return sDictManager->getDictBuildNumber(idx);
	}

	return -1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
JNIEXPORT jboolean Java_com_iknowu_PredictionEngine_ntvSetDictSetting(JNIEnv* env, jobject javaThis, jint langidx, jint priority, jboolean enabled)
{
   ShowInfo("inside ntvSetDictSetting: langidx=%d, priority=%d, enabled=%d\n", langidx, priority, enabled);
	if (sDictManager)
	{
		jboolean res = sDictManager->SetOrderedDict(langidx, priority, enabled);
		if(res)
		{
			return JNI_TRUE;
		}
	}
	ShowInfo("\n");
	return JNI_FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
JNIEXPORT jboolean Java_com_iknowu_PredictionEngine_ntvGetDictInfo(JNIEnv* env, jobject javaThis, jint idx, jobject info)
{
   callLog("\nInside ntvGetDictInfo: ");
	if (sDictManager)
	{
		jclass clazz;
		jfieldID fid;
		jmethodID mid;

		DictionaryConfigurationEntry *dcen = sDictManager->GetOrderedDict(idx);
		if(dcen == NULL)
		{
			callLog("--reached end of dictionary list\n");
			return(JNI_FALSE);
		}

		clazz = env->GetObjectClass(info);
		if (clazz == 0 )
		{
			ShowInfo("--!!GetObjectClass returned 0 for ntvGetDictInfo. \n");
			return(JNI_FALSE);
		}

		fid = env->GetFieldID(clazz,"listIdx", "I");
		env->SetIntField(info, fid, dcen->priority);
		ShowInfoIf((dcen->priority<0 || dcen->priority >  sDictManager->getNumExistingDictionaries()), "ERROR! priority out of order!\n");
		fid = env->GetFieldID(clazz,"langIdx", "I");
		env->SetIntField(info, fid, dcen->langIdx);
	   
		fid = env->GetFieldID(clazz,"bEnabled","Z");
		env->SetBooleanField(info,fid,dcen->enabled);
	   
		const char* name = Dictionary::GetDictName(dcen->langIdx);
		MYWCHAR *wname = toW(name);
		jstring printBuffer = createjstring(env, wname);
		if (printBuffer == NULL)
		{
			ShowInfo("--failed creat jstring for dictionary name: %s!!\n", name);
			return (JNI_FALSE);
		}
		fid = env->GetFieldID(clazz,"name","Ljava/lang/String;");
		env->SetObjectField(info, fid, printBuffer);
		//ShowInfo("---returning dict: [%d]:%s==%s, priority=%d, enabled=%d\n", dcen->langIdx, name, toA(wname), dcen->priority, dcen->enabled);
	}

	return JNI_TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
JNIEXPORT jstring Java_com_iknowu_PredictionEngine_ntvGetRootText(JNIEnv* env, jobject javaThis)
{
    callLog("\ninside ntvGetRootText");
	if (sDictManager)
	{
		MYWCHAR *rootText = sDictManager->gatherRootEnding();
		jstring jrootText = createjstring(env, rootText);
		return jrootText;
	}
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
JNIEXPORT jstring Java_com_iknowu_PredictionEngine_ntvUndoLetterOrWord(JNIEnv* env, jobject javaThis)
{
    callLog("\ninside ntvUndoLetterOrWord");
	if (sDictManager) 
	{
		MYWCHAR *rootText = sDictManager->undoLetterOrWord();
		jstring jrootText = createjstring(env, rootText);
		return jrootText;
	}
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
JNIEXPORT jint Java_com_iknowu_PredictionEngine_ntvAutoAddedWords(JNIEnv* env, jobject javaThis, jobject addedInfo)
{
	int numWordsAdded = 0;
    callLog("\ninside ntvAutoAddedWords");
	if (sDictManager)
	{
		jclass clazz;
		jfieldID fid;
		jmethodID mid;

		BYTE totalNumNewWords = 0;
		numWordsAdded = sDictManager->mngrGetWordsAddedSinceLastCallAndReset(&totalNumNewWords);
		int newwordstart = totalNumNewWords - numWordsAdded;
		ShowInfo("ntvAutoAddedWords: %d words added!\n", numWordsAdded);
		if(numWordsAdded ==0)
			return 0;

		clazz = env->GetObjectClass(addedInfo);
		if (clazz == 0 )
		{
			printf("!!GetObjectClass returned 0 for ntvAutoAddedWords. \n");
			return(0);
		}

		fid = env->GetFieldID(clazz,"nPosWords", "I");
		env->SetIntField(addedInfo, fid, numWordsAdded);

		jobjectArray jaddedWordsAr;
		jaddedWordsAr = (jobjectArray)env->NewObjectArray(numWordsAdded, env->FindClass("java/lang/String"), env->NewStringUTF(""));
		for (int i=0; i<numWordsAdded; i++)
		{
			ShowInfo("ntvAutoAddedWords: add word %s, idx = %d\n", toA(sFlushableWords[newwordstart+i]->text), (newwordstart+i));
			jstring jaddedword = createjstring(env, sFlushableWords[newwordstart+i]->text);
			if (jaddedword == NULL)
			{
				return (0);
			}
	        env->SetObjectArrayElement(jaddedWordsAr,i,jaddedword);
		}

        fid = env->GetFieldID(clazz,"addedWordsAr","[Ljava/lang/String;");
		env->SetObjectField(addedInfo,fid,jaddedWordsAr);
	}
	return numWordsAdded;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JNIEXPORT jint Java_com_iknowu_PredictionEngine_ntvListWords(JNIEnv* env, jobject javaThis, jint dictId, jobject wordList)
{
   callLog("\ninside ntvListWords");
   ShowInfo("ntvListWords: dictId=%d\n", dictId);
	int nPosWords = 0;
	if (sDictManager) 
	{
		MYWCHAR *listWords = sDictManager->getWordList(dictId, &nPosWords);
		jclass clazz = env->GetObjectClass(wordList);
		if (clazz == 0 )
		{
			printf("GetObjectClass returned 0\n");
			return(0);
		}

		jfieldID fid = env->GetFieldID(clazz,"nPosWords", "I");
		env->SetIntField(wordList, fid, nPosWords);

		jobjectArray jlistWordsAr = (jobjectArray)env->NewObjectArray(nPosWords, env->FindClass("java/lang/String"), env->NewStringUTF(""));
		for (int i=0; i<nPosWords; i++)
		{
			MYWCHAR *thisword = &listWords[i*MAX_WORD_LEN];
			ShowInfo("--nextword = %s\n", toA(thisword));
			jstring jnextword = createjstring(env, thisword);
			if (jnextword == NULL)
			{
				ShowInfo("--ntvListWords: !!error! couldn't creat jstring for word %s!\n", toA(&listWords[i]));
				return (0);
			}
	        env->SetObjectArrayElement(jlistWordsAr, i, jnextword);
		}

        fid = env->GetFieldID(clazz,"nextWordsAr", "[Ljava/lang/String;");
		env->SetObjectField(wordList, fid, jlistWordsAr);
	}
	return nPosWords;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// helper function for filling nextWords list for export!
// Used internally, not called from outside!
////////////////////////////////////////////////////////////////////////////////////////////
jint fillNextWordsInfo(JNIEnv* env, MultiLetterAdvanceAr *nextWordsAr, jobject nextInfo, MYWCHAR *rootWord)
{
	jclass clazz = env->GetObjectClass(nextInfo);
	if (clazz == 0 )
	{
		printf("!!!Error! fillNextWordsInfo: GetObjectClass returned 0\n");
		return(-1);
	}
		
	int nPosWords = nextWordsAr->nActualNexts;		
	// write number of next words in nextInfo:
	jfieldID fid = env->GetFieldID(clazz,"nPosWords", "I");
	env->SetIntField(nextInfo, fid, nPosWords);

	// write actual next words in next words array:
	jobjectArray jnextWordsAr = (jobjectArray)env->NewObjectArray(nPosWords, env->FindClass("java/lang/String"), env->NewStringUTF(""));
	for (int i=0; i<nPosWords; i++)
	{
		WLBreakIf(nextWordsAr->nextWords[i]==NULL, "!!ERROR! fillNextWordsInfo: nextWord is null: %d(%s), ", i, toA(nextWordsAr->nextWords[i]));
		jstring jnextword = createjstring(env, nextWordsAr->nextWords[i]);
		ShowInfo("--fillNextWordsInfo: nextWord is: (%s)\n",toA(nextWordsAr->nextWords[i]));
		if (jnextword == NULL)
			return (-1);
	
	    env->SetObjectArrayElement(jnextWordsAr, i, jnextword);
	}

	// set the array into string arry object in nextInfo:
    fid = env->GetFieldID(clazz,"nextWordsAr", "[Ljava/lang/String;");
	env->SetObjectField(nextInfo, fid, jnextWordsAr);

	// set the root word in nextInfo object
	fid = env->GetFieldID(clazz,"rootWord","Ljava/lang/String;");
	jstring jrootWord = createjstring(env, rootWord);
	env->SetObjectField(nextInfo, fid, jrootWord);
	ShowInfo("--fillNextWordsInfo: returnerd nPosWords=%d\n", nPosWords);

	return nPosWords;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
jint fillNextPhraseInfo(JNIEnv* env, PhraseAr *inPhrases, jobject nextInfo, MYWCHAR *rootWord, int FromWhere)
{
	int count = inPhrases->count;
	int nn = mywcslen(rootWord);
	ShowInfo("--fillNextPhraseInfo: (%s)[%d] FromWhere[%d]\n", toA(rootWord), nn, FromWhere);

	jclass clazz = env->GetObjectClass(nextInfo);
	if (clazz == 0 )
	{
		printf("!!!Error! fillNextPhraseInfo: GetObjectClass returned 0\n");
		return(-1);
	}

	ShowInfo("--fillNextPhraseInfo: returnerd nPhraseWords=%d\n", count);
	jfieldID fid2 = env->GetFieldID(clazz,"nPhraseWords", "I");
	env->SetIntField(nextInfo, fid2, count);

	jobjectArray phraseAr = (jobjectArray)env->NewObjectArray(count, env->FindClass("java/lang/String"), env->NewStringUTF(""));
	for (int i=0; i < count; i++)
	{
		WLBreakIf(inPhrases->phrases[i]==NULL, "!!ERROR! fillNextPhraseInfo: PhraseAr is empty at (%d), ", i);
		ShowInfo("--fillNextPhraseInfo: returnerd Phrase=%s\n", toA(inPhrases->phrases[i]));

		jstring jnextword = createjstring(env, inPhrases->phrases[i]);
		if (jnextword == NULL)
			return (-1);
	
	    env->SetObjectArrayElement(phraseAr, i, jnextword);
	}

	fid2 = env->GetFieldID(clazz,"phraseAr", "[Ljava/lang/String;");
	env->SetObjectField(nextInfo, fid2, phraseAr);

	return count;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JNIEXPORT jint Java_com_iknowu_PredictionEngine_ntvNextWords(JNIEnv* env, jobject javaThis, jobject nextInfo)
{
	int nPosWords = 0;
    callLog("\ninside ntvNextWords");
	if (sDictManager) 
	{
		jfieldID fid;
		jmethodID mid;

		MYWCHAR rootWord[MAX_WORD_LEN];
		MultiLetterAdvanceAr *nextWordsAr = sDictManager->multiNextWords(rootWord);
		
		nPosWords = fillNextWordsInfo(env, nextWordsAr, nextInfo, rootWord);
		if (nPosWords < 0)
		{
			printf("!!!Error! ntvNextWords: GetObjectClass or createjstring failed!\n");
			return(0);
		}

		ShowInfo("--ntvNextWords: return rootWord: #%s#, nPosWords(%d)\n", toA(rootWord), nPosWords);
	}
	return nPosWords;
}

//////////////////////////////////////////////////////////////////////////////////////////////
JNIEXPORT jint Java_com_iknowu_PredictionEngine_ntvStartingWords(JNIEnv* env, jobject javaThis, jobject nextInfo)
{
	int nPosWords = 0;
	if (sDictManager) 
	{
		jfieldID fid;
		jmethodID mid;

		MYWCHAR rootWord[MAX_WORD_LEN];
		MultiLetterAdvanceAr *nextWordsAr = sDictManager->UpdateForStartWords();
		
		nPosWords = fillNextWordsInfo(env, nextWordsAr, nextInfo, rootWord);
		if (nPosWords < 0)
		{
			printf("!!!Error! ntvStartingWords: GetObjectClass or createjstring failed!\n");
			return(0);
		}
	}
	return nPosWords;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
JNIEXPORT jboolean Java_com_iknowu_PredictionEngine_ntvWordCanbeAdded(JNIEnv* env, jobject javaThis)
{
    callLog("\ninside ntvWordCanbeAdded");
	if(sDictManager)
		return sDictManager->CurrentWordCanbeAdded();
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JNIEXPORT jboolean Java_com_iknowu_PredictionEngine_ntvAddWord(JNIEnv* env, jobject javaThis, jstring newWordPart, jint dictId)
{
   callLog("\ninside ntvAddWord");
	if (sDictManager)
	{
		int wordPartLen = env->GetStringLength(newWordPart);
		if (wordPartLen <= MAX_WORD_LEN) 
		{
			MYWCHAR wordPartW[MAX_WORD_LEN];
			env->GetStringRegion(newWordPart, 0, wordPartLen, (jchar *) wordPartW);
			wordPartW[wordPartLen]=NUL;
			ShowInfo("ntvAddWord: adding #%s#\n", toA(wordPartW));
            int dictIdx = 0;
			return sDictManager->addWord(wordPartW, 0, &dictIdx)!=NULL;
		}
	}
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JNIEXPORT jint Java_com_iknowu_PredictionEngine_ntvEraseLastWord(JNIEnv* env, jobject javaThis, jobject nextInfo)
{
   callLog("\ninside ntvEraseLastWord");
	int ret = 0;
	if (sDictManager)
	{
		MYWCHAR rootWord[MAX_WORD_LEN];
		MultiLetterAdvanceAr *nextWordsAr = sDictManager->eraseLastWord(rootWord, ret);
		
		int nPosWords = fillNextWordsInfo(env, nextWordsAr, nextInfo, rootWord);
		if (nPosWords < 0)
		{
			printf("!!!Error! ntvEraseLastWord: GetObjectClass or createjstring failed!\n");
			return(0);
		}
	}
	return ret;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JNIEXPORT jboolean Java_com_iknowu_PredictionEngine_ntvDeleteWord(JNIEnv* env, jobject javaThis, jstring newWordPart)
{
   callLog("\ninside ntvDeleteWord");
	if (sDictManager)
	{
		   int wordPartLen = env->GetStringLength(newWordPart);
		   if (wordPartLen <= MAX_WORD_LEN)
		   {
			   MYWCHAR wordPartW[MAX_WORD_LEN];
			   env->GetStringRegion(newWordPart, 0, wordPartLen, (jchar *) wordPartW);
			   wordPartW[wordPartLen]=NUL;
			   return sDictManager->deleteWord(wordPartW);
		   }
	}
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JNIEXPORT void Java_com_iknowu_PredictionEngine_ntvSetAutoLearn(JNIEnv* env, jobject javaThis, jboolean bAutoLearn)
{
	if (sDictManager)
	{
		  sDictManager->setAutoLearn(bAutoLearn);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JNIEXPORT void Java_com_iknowu_PredictionEngine_ntvSetSpacelessTyping(JNIEnv* env, jobject javaThis, jboolean bSpaceless)
{
	if (sDictManager)
	{
		  sDictManager->setSpacelessTyping(bSpaceless);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*#ifdef DEBUG
void callLog(const char *s)
{
	__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "WLLIBLog [%s]", s);
	printf(s);

}
#else
void callLog(const char *s) {}
#endif */

}

#endif

