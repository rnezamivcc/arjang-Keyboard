#if !defined(_WINDOWS) && !defined(WIN32)
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <jni.h>
#include <android/log.h>
#include "dictmanager.h"
#include "userWordCache.h"
#include "userWordCacheOffline.h"
#include "dictionary.h"
#include "autocorrect.h"

#define LENGTHOF(array) (sizeof(array)/sizeof((array)[0]))
#ifdef DEBUG
#define WLLOGTrace(...) __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "WLLIBTRACE", __VA_ARGS__)
void callLog(const char *s)
{
__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "WLLIBLog [%s]", s);
}

#else
#define WLLOGTrace(...)
void callLog(const char *s) {}
#endif

extern "C" {

CDictManager *gDictManager = NULL;
bool setDictManager(char *rootPath)
{
	callLog("\nsetDictManager:: setting up with root "); callLog(rootPath); callLog("\n");
	if (!gDictManager)
	{
		gDictManager = new CDictManager();
		if (!gDictManager->Create(NULL, rootPath))
		{
			callLog("!!ERROR!! setDictManager:: failed initializaing! now it's just a dumb keyboard!\n");
			delete gDictManager;
			gDictManager = NULL;
		}
	}
	return gDictManager != NULL;
}

jint fillNextWordsInfo(JNIEnv* env, MultiLetterAdvanceAr *nextWordsAr, jobject nextInfo);
jint fillNextInfo(JNIEnv* env, MYWCHAR **nextWords, jobject nextInfo, int nPosWords);
jint fillNextPhraseInfo(JNIEnv* env, PhraseAr *inPhrases, jobject nextInfo, MYWCHAR *rootWord, int FromWhere);
jint fillNounInfo(JNIEnv* env, NounAr *inNouns, jobject nounInfo);
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
jstring createjstring(JNIEnv* env, MYWCHAR *text)
{
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
	return (jboolean)(setDictManager(toA(wordPartW)));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
JNIEXPORT jboolean Java_com_iknowu_PredictionEngine_ntvResetDictionaryConfiguration(JNIEnv* env, jobject javaThis)
{
	return gDictManager && gDictManager->resetConfiguration();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
JNIEXPORT void Java_com_iknowu_PredictionEngine_ntvReset(JNIEnv* env, jobject javaThis, jboolean cleanHistory)
{
	if (gDictManager)
	{
		if(cleanHistory)
		{
			callLog("\nntvReset full\n"); 
			gDictManager->fullReset();
		}
		else
		{
			callLog("\nntvReset\n"); 
			gDictManager->reset();
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
JNIEXPORT void Java_com_iknowu_PredictionEngine_ntvResetTGraphHistory(JNIEnv* env, jobject javaThis)
{
	if (gDictManager)
	{
		ShowInfo("MK ntvResetTGraphHistory\n");
		gDictManager->ResetTGraphHistory();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
JNIEXPORT void Java_com_iknowu_PredictionEngine_ntvLearnFromFile(JNIEnv* env, jobject javaThis, jstring jsFilePath)
{
	//Minkyu: 2013.09.23
	callLog("Start!!ntvLearnFromFile");
	if (gDictManager)
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

		//Testing
		//szPath = createFullPathFileName("sms.txt");
		ShowInfo("ntvLearn File path = %s\n", szPath);

		gDictManager->ProcessLearningOffline(szPath);	
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
JNIEXPORT jint Java_com_iknowu_PredictionEngine_ntvBackspaceLetter(JNIEnv* env, jobject javaThis, jobject nextInfo)
{
	int nPosWords = 0;
   callLog("\ninside tvBackspaceLetter");
	if(gDictManager)
	{
	    MYWCHAR rootWord[MAX_WORD_LEN];
		memset(rootWord, 0 , sizeof(rootWord));
		MultiLetterAdvanceAr *nextWordsAr = gDictManager->backspaceLetter(rootWord);
		if(!nextWordsAr)
			return 0;

		ShowInfo("--ntvBackspaceLetter rootword1:(%s)\n", toA(rootWord));
		int nPosWords = fillNextWordsInfo(env, nextWordsAr, nextInfo);

		//Minkyu:2013.11.15
		PhraseAr *phraseAr = gDictManager->ProcessPhrasePrediction(rootWord, true);
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
   if (gDictManager) 
   {
	   int wordPartLen = env->GetStringLength(wordPart);
	   MYWCHAR wordPartW[MAX_WORD_LEN];
	   env->GetStringRegion(wordPart, 0, wordPartLen, (jchar *) wordPartW);
	   wordPartW[wordPartLen]=0;

	   return gDictManager->isChunk(wordPartW);
   }
   return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
JNIEXPORT jint Java_com_iknowu_PredictionEngine_ntvNumWordsStartingWith(JNIEnv* env, jobject javaThis, jstring wordPart)
{
   if (gDictManager) 
   {
	   int wordPartLen = env->GetStringLength(wordPart);
	   MYWCHAR wordPartW[MAX_WORD_LEN];
	   env->GetStringRegion(wordPart, 0, wordPartLen, (jchar *) wordPartW);
	   wordPartW[wordPartLen]=0;

	   return gDictManager->getNumWordsStartingWith(wordPartW);
   }
   return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JNIEXPORT jint Java_com_iknowu_PredictionEngine_ntvAdvanceMultiLetters(JNIEnv* env, jobject javaThis, jstring letters,
																	   jbyteArray inprefs, jobject nextInfo, bool begin)
{
   callLog("\ninside ntvAdvanceMultiLetters");
   int nPosBS = 0;

   if (gDictManager) 
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
	
		MYWCHAR *printP=NULL;
	    MultiLetterAdvanceAr *nextWordsAr = gDictManager->advanceMultiLetters(nextletters, inPrefs, rootWord, &printP, begin);
		if(!nextWordsAr)
			return 0;

		nPosBS = nextWordsAr->nBackSpace;
		int nPosWords = fillNextWordsInfo(env, nextWordsAr, nextInfo);

		//Minkyu:2013.11.15
		PhraseAr *phraseAr = gDictManager->ProcessPhrasePrediction(rootWord, false, nextletters);
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

/////////////////////////////////////////////////////////////////////////////////////////////////////////
JNIEXPORT jint Java_com_iknowu_PredictionEngine_ntvAdvanceLetter(JNIEnv* env, jobject javaThis, 
																	 jchar letter, jobject info, jobject nextInfo)
{
   jclass clazz;
   jfieldID fid;

    int nPosBS = 0;
  if (gDictManager) 
   {
		callLog("inside ntvAdvanceLetter: CTestDictManager::sTestDictMgr");	   
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
		MultiLetterAdvanceAr *nextWordsAr = gDictManager->advanceMultiLetters( str, prefs, rootWord, &printP, bStartSentence, bLiteralFlag);
		if(!nextWordsAr)
			return 0;

		nPosBS = nextWordsAr->nBackSpace;
		ShowInfo("inside ntvAdvanceLetter: returned %d words for letter #%c#: at root #%s#, nPosBS(%d):\n", nextWordsAr->nActualNexts, char2A(letter), toA(rootWord), nPosBS);

		int nPosWords = fillNextWordsInfo(env, nextWordsAr, nextInfo);
		if (nPosWords < 0)
		{
			printf("!!!Error! ntvAdvanceLetter: GetObjectClass or createjstring failed!\n");
			return(0);
		}

		PhraseAr *phraseAr=NULL;
		int nPhraseWords=0;
		if(!isPunctuation(letter))
		{
			ShowInfo("inside ntvAdvanceLetter: not Punctuation\n\n");
			phraseAr = gDictManager->ProcessPhrasePrediction(rootWord);
		}
		else
		{
			ShowInfo("inside ntvAdvanceLetter: isPunctuation\n\n");
			phraseAr = gDictManager->ProcessPhrasePrediction(NULL);
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
   callLog("\n inside ntvAdvanceWord");
   if (gDictManager) 
   {
	   MYWCHAR *printPartW = NULL;
	   int wordPartLen = env->GetStringLength(wordPart);
	   MYWCHAR wordPartW[MAX_WORD_LEN];
	   env->GetStringRegion(wordPart, 0, wordPartLen, (jchar *)wordPartW);
	   wordPartW[wordPartLen]=0;

	   ShowInfo("-ntvAdvanceWord: wordPart=#%s#, len=%d, bComplete=%d, replace=%d\n",
			   	   	   	   	   	   	   	   	   toA(wordPartW), wordPartLen, bComplete, replace);
	   MultiLetterAdvanceAr *nextWordsAr=NULL;

	   int numBacout = gDictManager->advanceWord(wordPartW, &printPartW, nextWordsAr, bComplete, replace);
	   if(!nextWordsAr)
			return 0;

	   ShowInfo("-ntvAdvanceWord: return from gDictManager->advanceWord: numBacout= %d, nextWordsAr->nActualNexts=%d\n",
			   numBacout, nextWordsAr->nActualNexts);
	   ShowInfo("-ntvAdvanceWord: bComplete= %d, replace=%d\n", bComplete, replace);

	   	// now record next words:
		MYWCHAR rootWord[MAX_WORD_LEN];

		if (fillNextWordsInfo(env, nextWordsAr, nextInfo) < 0)
		{
			printf("!!!Error! ntvAdvanceWord: GetObjectClass or createjstring failed!\n");
			return(0);
		}
	  
		PhraseAr *phraseAr = gDictManager->ProcessPhrasePrediction(wordPartW);
		int nPhraseWords = fillNextPhraseInfo(env, phraseAr, nextInfo, rootWord, 2);

		if (nPhraseWords < 0)
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

	   return numBacout;
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////
JNIEXPORT void Java_com_iknowu_PredictionEngine_ntvPathSwipe(JNIEnv* env, jobject javaThis, jobjectArray path, jobject nextInfo)
{
	callLog("\ninside ntvPathSwipe");
	jfieldID fidX, fidY, fidCh, fidCount;
	jchar ch;

	if (gDictManager)
	{
		int nNodes = env->GetArrayLength(path);
	    jclass clsPathNode0 = env->FindClass("com/iknowu/swipe/Swipe");
    	if(clsPathNode0 == NULL) {
    		printf("!!!Error! class com.iknowu.swipe.Swipe could not be loaded! No Pathing!! Len=%d\n", nNodes);
    		return;
    	}
       jclass clsPathNode = env->FindClass("com/iknowu/swipe/Sw_Node");
    	if(clsPathNode==NULL /*env->ExceptionCheck()*/) {
    		printf("!!!Error! class com.iknowu.swipe.Swipe.Sw_Node could not be loaded! No Pathing!! Len=%d\n", nNodes);
    		return;
    	}

		fidX = env->GetFieldID(clsPathNode,"x","F");
		fidY = env->GetFieldID(clsPathNode,"y","F");
		fidCh = env->GetFieldID(clsPathNode,"val","C");
		fidCount = env->GetFieldID(clsPathNode,"count","I");
        printf("--ntvPathSwipe:adding %d nodes to swipe:\n", nNodes);
		for(int c=0; c<nNodes; c++){
			jobject node = env->GetObjectArrayElement(path, c);

			float x = env->GetFloatField(node,fidX);
			float y = env->GetFloatField(node,fidY);
			MYWCHAR val = (MYWCHAR)env->GetCharField(node,fidCh);
			int cnt = env->GetIntField(node,fidCount);
			printf("--ntvPathSwipe:addNode:%d:( %f, %f, %c, %d)\n", c, x, y, char2A(val), cnt);
			gDictManager->mSwiper.addNode(x,y,val, cnt);
		}
        printf("--ntvPathSwipe:processPathAndReset:");
		gDictManager->mSwiper.processPathAndReset();
		int count = 0;
	    printf("--ntvPathSwipe:getMostPreferredWords:");
		MYWCHAR **results = gDictManager->getMostPreferredWords(count);

		if (fillNextInfo(env, results, nextInfo, count) < 0)
		{
			printf("!!!Error!!! ntvPathSwipe: GetObjectClass or createjstring failed!\n");
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
JNIEXPORT jstring Java_com_iknowu_PredictionEngine_ntvNextLetters(JNIEnv* env, jobject javaThis, jintArray posArr)
{
   callLog("\ninside ntvNextLetters");
	if (gDictManager)
	{
		int *myposAr = NULL;
		MYWCHAR *prefLetters = gDictManager->nextLetters(&myposAr);
		jstring jprefletters = createjstring(env, prefLetters);

		int nLetters= mywcslen(prefLetters);
	    env->SetIntArrayRegion(posArr, 0, nLetters, (jint *)myposAr);
		return jprefletters;
	}
	return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////
JNIEXPORT jint Java_com_iknowu_PredictionEngine_ntvSetHistory(JNIEnv* env, jobject javaThis, jstring words, bool backspace)
{
	if (gDictManager) 
	{
		MYWCHAR historyWords[MAX_WORD_LEN]; 

		int historyLength = env->GetStringLength(words);
		ShowInfo("inside ntvSetHistory: historyLength(%d)\n",historyLength);

		env->GetStringRegion(words, 0, historyLength, (jchar *) historyWords);
		historyWords[historyLength]=NUL;

		ShowInfo("inside ntvSetHistory: historyWords(%s)\n",toA(historyWords));
		gDictManager->SetHistoryFromJNI(historyWords,backspace);
	}
}
////////////////////////////////////////////////////////////////////////////////////
JNIEXPORT jboolean Java_com_iknowu_PredictionEngine_ntvGetNounResult(JNIEnv* env, jobject javaThis, jobject nounInfo, jstring wordPart)
{
   if (gDictManager) 
   {
	   int wordPartLen = env->GetStringLength(wordPart);
	   MYWCHAR wordPartW[MAX_WORD_LEN];

	   env->GetStringRegion(wordPart, 0, wordPartLen, (jchar *) wordPartW);
	   wordPartW[wordPartLen]=0;

	   ShowInfo("inside ntvGetNounResult:(%s)\n",toA(wordPartW));
	   NounAr* arr = gDictManager->GetNounsFromPhrase(wordPartW);
	  
	   int nNouns = fillNounInfo(env, arr, nounInfo);
	   
	   return false;
   }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
JNIEXPORT jint Java_com_iknowu_PredictionEngine_ntvGetDictPriority(JNIEnv* env, jobject javaThis, jint idx)
{
   ShowInfo("inside ntvGetDictPriority for idx=%d", idx);
	if (gDictManager)
	{
		DictionaryConfigurationEntry *dcen = gDictManager->GetOrderedDict(idx);
		if(dcen!= NULL)
			return dcen->priority;
	}

	return -1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
JNIEXPORT jint Java_com_iknowu_PredictionEngine_ntvGetDictVersion(JNIEnv* env, jobject javaThis, jint idx)
{
   ShowInfo("inside ntvGetDictVersion for idx=%d", idx);
	if (gDictManager)
	{
		return gDictManager->getDictVersionNumber(idx);
	}

	return -1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
JNIEXPORT jint Java_com_iknowu_PredictionEngine_ntvGetDictBuild(JNIEnv* env, jobject javaThis, jint idx)
{
   ShowInfo("inside ntvGetDictBuild for idx=%d", idx);
	if (gDictManager)
	{
		return gDictManager->getDictBuildNumber(idx);
	}

	return -1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
JNIEXPORT jboolean Java_com_iknowu_PredictionEngine_ntvSetDictSetting(JNIEnv* env, jobject javaThis, jint langidx, jint priority, jboolean enabled)
{
   ShowInfo("inside ntvSetDictSetting: langidx=%d, priority=%d, enabled=%d\n", langidx, priority, enabled);
	if (gDictManager)
	{
		jboolean res = gDictManager->SetOrderedDict(langidx, priority, enabled);
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
	if (gDictManager)
	{
		jclass clazz;
		jfieldID fid;
		jmethodID mid;

		DictionaryConfigurationEntry *dcen = gDictManager->GetOrderedDict(idx);
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
		ShowInfoIf((dcen->priority<0 || dcen->priority >  gDictManager->getNumExistingDictionaries()), "ERROR! priority out of order!\n");
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
	if (gDictManager)
	{
		MYWCHAR *rootText = gDictManager->gatherRootEnding();
		jstring jrootText = createjstring(env, rootText);
		return jrootText;
	}
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
JNIEXPORT jstring Java_com_iknowu_PredictionEngine_ntvUndoLetterOrWord(JNIEnv* env, jobject javaThis)
{
    callLog("\ninside ntvUndoLetterOrWord");
	if (gDictManager) 
	{
		MYWCHAR *rootText = gDictManager->undoLetterOrWord();
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
	if (gDictManager)
	{
		jclass clazz;
		jfieldID fid;
		jmethodID mid;

		BYTE totalNumNewWords = 0;
		numWordsAdded = gDictManager->mngrGetWordsAddedSinceLastCallAndReset(&totalNumNewWords);
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
	if (gDictManager) 
	{
		MYWCHAR *listWords = gDictManager->getWordList(dictId, &nPosWords);
		jclass clazz = env->GetObjectClass(wordList);
		if (clazz == 0 )
		{
			printf("GetObjectClass returned 0\n");
			return(0);
		}

		jfieldID fid = env->GetFieldID(clazz,"nPredictions", "I");
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
jint fillNextInfo(JNIEnv* env, MYWCHAR **nextWords, jobject nextInfo, int nPosWords)
{
	if(!nextWords)
		return 0;

	jclass clazz = env->GetObjectClass(nextInfo);
	if (clazz == 0 )
	{
		printf("!!!Error! fillNextInfo: GetObjectClass returned 0\n");
		return(-1);
	}

	// write number of next words predictions and possible corrections in nextInfo:
	jfieldID fid = env->GetFieldID(clazz,"nPredictions", "I");
	env->SetIntField(nextInfo, fid, nPosWords);
	fid = env->GetFieldID(clazz,"nCorrections", "I");
	env->SetIntField(nextInfo, fid, 0);
	fid = env->GetFieldID(clazz,"rootWordExist", "I");
	env->SetIntField(nextInfo, fid, false);

	// write actual next words in next words array:
	jobjectArray jnextWordsAr = (jobjectArray)env->NewObjectArray(nPosWords, env->FindClass("java/lang/String"), env->NewStringUTF(""));
	for (int i=0; i<nPosWords; i++)
	{
		WLBreakIf(nextWords[i]==NULL || nextWords[i][0]==NUL, "!!ERROR! fillNextInfo: nextWord is null: %d(%s), ", i, toA(nextWords[i]));
		jstring jnextword = createjstring(env, nextWords[i]);
		ShowInfo("--fillNextInfo: nextWord is: (%s)\n",toA(nextWords[i]));
		if (jnextword == NULL)
			return (-1);

		env->SetObjectArrayElement(jnextWordsAr, i, jnextword);
	}

	// set the array into string arry object in nextInfo:
	fid = env->GetFieldID(clazz,"nextWordsAr", "[Ljava/lang/String;");
	env->SetObjectField(nextInfo, fid, jnextWordsAr);

	// set the root word in nextInfo object
	fid = env->GetFieldID(clazz,"rootWord", "Ljava/lang/String;");
//	jstring jrootWord = createjstring(env, rootWord);
	env->SetObjectField(nextInfo, fid, NULL);

	return nPosWords;
}
/////////////////////////////////////////////////////////////////////////////////////////////
// helper function for filling nextWords list for export!
// Used internally, not called from outside!
////////////////////////////////////////////////////////////////////////////////////////////
jint fillNextWordsInfo(JNIEnv* env, MultiLetterAdvanceAr *nextWordsAr, jobject nextInfo)
{
	if(!nextWordsAr)
		return 0;

	MYWCHAR rootWord[MAX_WORD_LEN];
	bool isWord = false;
	if(isCurWordNotInDictionaries())
		rootWord[0] = NUL;
	else
	{
		constructCurWord(rootWord);
		int dictIdx = 0;
		CompactNode * node = gDictManager->QuadRetrieveCompactNode(rootWord, true, &dictIdx);
		if(node)
			isWord = true;
	}

	jclass clazz = env->GetObjectClass(nextInfo);
	if (clazz == 0 )
	{
		printf("!!!Error! fillNextWordsInfo: GetObjectClass returned 0\n");
		return(-1);
	}
		
	int nPredictions = nextWordsAr->nActualNexts - nextWordsAr->nCorrections;
	int nCorrections = nextWordsAr->nCorrections;
	int nPosWords = nextWordsAr->nActualNexts;
	// write number of next words predictions and possible corrections in nextInfo:
	jfieldID fid = env->GetFieldID(clazz,"nPredictions", "I");
	env->SetIntField(nextInfo, fid, nPredictions);
	fid = env->GetFieldID(clazz,"nCorrections", "I");
	env->SetIntField(nextInfo, fid, nCorrections);
	fid = env->GetFieldID(clazz,"rootWordExist", "I");
	env->SetIntField(nextInfo, fid, isWord);

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
	fid = env->GetFieldID(clazz,"rootWord", "Ljava/lang/String;");
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
jint fillNounInfo(JNIEnv* env, NounAr *inNouns, jobject nounInfo)
{
	int count = inNouns->count;
	jclass clazz = env->GetObjectClass(nounInfo);
	if (clazz == 0 )
	{
		printf("!!!Error! fillNounInfo: GetObjectClass returned 0\n");
		return(-1);
	}

	jfieldID fid2 = env->GetFieldID(clazz,"nNounWords", "I");
	env->SetIntField(nounInfo, fid2, count);


	jobjectArray nounAr = (jobjectArray)env->NewObjectArray(count, env->FindClass("java/lang/String"), env->NewStringUTF(""));
	for (int i=0; i < count; i++)
	{
		WLBreakIf(inNouns->word[i]==NULL, "!!ERROR! fillNounInfo: NounAr is empty at (%d), ", i);
		ShowInfo("--fillNounInfo: returnerd words=%s\n", toA(inNouns->word[i]));

		jstring jnextword = createjstring(env, inNouns->word[i]);
		if (jnextword == NULL)
			return (-1);
	
	    env->SetObjectArrayElement(nounAr, i, jnextword);
	}

	fid2 = env->GetFieldID(clazz,"NounAr", "[Ljava/lang/String;");
	env->SetObjectField(nounInfo, fid2, nounAr);

}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JNIEXPORT jint Java_com_iknowu_PredictionEngine_ntvNextWords(JNIEnv* env, jobject javaThis, jobject nextInfo)
{
	int nPosWords = 0;
    callLog("\ninside ntvNextWords");
	if (gDictManager) 
	{
		jfieldID fid;
		jmethodID mid;

		MYWCHAR rootWord[MAX_WORD_LEN];
		MultiLetterAdvanceAr *nextWordsAr = gDictManager->multiNextWords(rootWord);
		if(!nextWordsAr)
			return 0;

		nPosWords = fillNextWordsInfo(env, nextWordsAr, nextInfo);
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
	if (gDictManager) 
	{
		jfieldID fid;
		jmethodID mid;
		callLog("\nntvStartingWords");
		MYWCHAR rootWord[MAX_WORD_LEN];
		MultiLetterAdvanceAr *nextWordsAr = gDictManager->UpdateForStartWords();
		if(!nextWordsAr)
			return 0;
		
		nPosWords = fillNextWordsInfo(env, nextWordsAr, nextInfo);
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
	if(gDictManager)
		return gDictManager->CurrentWordCanbeAdded();
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JNIEXPORT jboolean Java_com_iknowu_PredictionEngine_ntvAddWord(JNIEnv* env, jobject javaThis, jstring newWordPart, jint dictId)
{
   callLog("\ninside ntvAddWord");
	if (gDictManager)
	{
		int wordPartLen = env->GetStringLength(newWordPart);
		if (wordPartLen <= MAX_WORD_LEN) 
		{
			MYWCHAR wordPartW[MAX_WORD_LEN];
			env->GetStringRegion(newWordPart, 0, wordPartLen, (jchar *) wordPartW);
			wordPartW[wordPartLen]=NUL;
			ShowInfo("ntvAddWord: adding #%s#\n", toA(wordPartW));
            int dictIdx = 0;
			return gDictManager->addWord(wordPartW, 0, &dictIdx)!=NULL;
		}
	}
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JNIEXPORT jint Java_com_iknowu_PredictionEngine_ntvEraseLastWord(JNIEnv* env, jobject javaThis, jobject nextInfo)
{
   callLog("\ninside ntvEraseLastWord");
	unsigned ret = 0;
	if (gDictManager)
	{
		MYWCHAR rootWord[MAX_WORD_LEN];
		MultiLetterAdvanceAr *nextWordsAr = gDictManager->eraseLastWord(rootWord, ret);
		if(!nextWordsAr)
			return 0;

		int nPosWords = fillNextWordsInfo(env, nextWordsAr, nextInfo);
		if (nPosWords < 0)
		{
			printf("!!!Error! ntvEraseLastWord: GetObjectClass or createjstring failed!\n");
			return(0);
		}
	}
	return (jint)ret;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JNIEXPORT jboolean Java_com_iknowu_PredictionEngine_ntvDeleteWord(JNIEnv* env, jobject javaThis, jstring newWordPart)
{
   callLog("\ninside ntvDeleteWord");
	if (gDictManager)
	{
		   int wordPartLen = env->GetStringLength(newWordPart);
		   if (wordPartLen <= MAX_WORD_LEN)
		   {
			   MYWCHAR wordPartW[MAX_WORD_LEN];
			   env->GetStringRegion(newWordPart, 0, wordPartLen, (jchar *) wordPartW);
			   wordPartW[wordPartLen]=NUL;
			   return gDictManager->deleteWord(wordPartW);
		   }
	}
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JNIEXPORT void Java_com_iknowu_PredictionEngine_ntvSetAutoLearn(JNIEnv* env, jobject javaThis, jboolean bAutoLearn)
{
	if (gDictManager)
	{
		  gDictManager->setAutoLearn(bAutoLearn);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JNIEXPORT void Java_com_iknowu_PredictionEngine_ntvSetSpacelessTyping(JNIEnv* env, jobject javaThis, jboolean bSpaceless)
{
	if (gDictManager)
	{
		  gDictManager->setSpacelessTyping(bSpaceless);
	}
}

}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif

