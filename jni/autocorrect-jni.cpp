// this file defines all of the JNI functions. 
#if !defined(_WINDOWS) &&  !defined(WIN32)

#include <jni.h>
#include <android/log.h>
#include "wltypes.h"
#include <algorithm>
#include <map>

#include "autocorrect/geometry.h"
#include "dictmanager.h"
#include "autocorrect.h"
#include "utility.h"

#ifndef WLLOGTrace
#ifdef DEBUG
#define WLLOGTrace(...) __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "WL_AC_TRACE", __VA_ARGS__)
#else
#define WLLOGTrace(...)
#endif
#endif

//#ifdef __cplusplus
extern "C" {
//#endif
extern CDictManager* gDictManager;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
jstring createjstring(JNIEnv* env, MYWCHAR *text);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
JNIEXPORT void JNICALL Java_com_wordlogic_lib_AutoCorrect_setBeforeCursor(JNIEnv* env, jobject obj, jstring s) 
{
	int nchars = env->GetStringLength(s);
	MYWCHAR chars[MAX_WORD_LEN];
	env->GetStringRegion(s, 0, nchars, (jchar *) chars);
	chars[nchars]=NUL;
	ShowInfo("JNI_AutoCorrect_setBeforeCursor: chars = (%s)\n", toA(chars));
	
	gDictManager->getAutocorrect()->setBeforeCursor(chars, nchars);
}

//////////////////////////////////////////////////////////////////////////////////////
JNIEXPORT void JNICALL Java_com_wordlogic_lib_AutoCorrect_replaceBuffers(JNIEnv* env, jobject obj, jstring beforeCursor, jstring afterCursor) 
{
	ShowInfo("JNI_AutoCorrect_replaceBuffers:\n");
	
	if(beforeCursor == NULL) 
		gDictManager->getAutocorrect()->setBeforeCursor(NULL, 0);
	else 
	{
		int nchars = env->GetStringLength(beforeCursor);
		MYWCHAR chars[MAX_WORD_LEN];
		env->GetStringRegion(beforeCursor, 0, nchars, (jchar *) chars);
		chars[nchars]=NUL;
		ShowInfo("--JNI_AutoCorrect_replaceBuffers: beforeCursor(%s)\n", toA(chars));
		gDictManager->getAutocorrect()->setBeforeCursor(chars, nchars);
	}

/*	if(afterCursor == NULL) 
		autocorrect->setAfterCursor(NULL,0);
	else 
	{
		int ncharsa = env->GetStringLength(afterCursor);
		MYWCHAR achars[MAX_WORD_LEN];
		env->GetStringRegion(afterCursor, 0, ncharsa, (jchar *) achars);
		achars[ncharsa]=NUL;
		ShowInfo("--JNI_AutoCorrect_replaceBuffers: afterCursor(%s)\n", toA(achars));

		autocorrect->setAfterCursor(achars, ncharsa);
	} */
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JNIEXPORT void JNICALL Java_com_wordlogic_lib_AutoCorrect_reverseCorrection(JNIEnv* env, jobject obj, jint si, jstring corrected, jstring replacement)
{
	ShowInfo("JNI_AutoCorrect_reverseCorrection:si=%d\n", si);
	WLBreakIf(replacement == NULL, "!!ERROR! JNI_AutoCorrect_reverseCorrection replacement string must not be NULL");
	WLBreakIf(corrected == NULL, "!!ERROR! JNI_AutoCorrect_reverseCorrection corrected string  must not be NULL");

	int rlen = env->GetStringLength(replacement);
	MYWCHAR rchars[MAX_WORD_LEN];
	env->GetStringRegion(replacement, 0, rlen, (jchar *) rchars);
	rchars[rlen]=NUL;
	ShowInfo("--JNI_AutoCorrect_reverseCorrection: replacement=(%s)\n", toA(rchars));

	int corlen = env->GetStringLength(corrected);
	gDictManager->getAutocorrect()->reverseCorrection(si, rlen, corlen, rchars);
}

/////////////////////////////////////////////////////////////////////////////////////////////
/*JNIEXPORT jstring JNICALL Java_com_wordlogic_lib_AutoCorrect_getSecondCorrection(JNIEnv* env, jobject obj, jint si, jstring original, jstring firstcorrect)
{
	ShowInfo("JNI_AutoCorrect_getSecondCorrection:si=%d\n", si);
	
	int olen = env->GetStringLength(original);
	MYWCHAR ochars[MAX_WORD_LEN];
	env->GetStringRegion(original, 0, olen, (jchar *) ochars);
	ochars[olen]=NUL;

	int alen = env->GetStringLength(firstcorrect);
	MYWCHAR achars[MAX_WORD_LEN];
	env->GetStringRegion(firstcorrect, 0, alen, (jchar *) achars);
	achars[alen]=NUL;

	jchar* cchars;
	int clen;
	gDictManager->getAutocorrect()->getSecondCorrection(si, ochars, olen, achars, alen, &cchars, &clen);

	if(clen == 0) 
		return NULL;

	return env->NewString(cchars, clen);
}
*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JNIEXPORT void JNICALL Java_com_wordlogic_lib_AutoCorrect_loadKeyboardLayout(JNIEnv* env,jobject obj,jobjectArray keys) 
{
    ShowInfo("JNI_AutoCorrect_loadKeyboardLayout");
	int nkeys;
	int c, cb, d, s;
	jobject key;
	jclass clsKeyPosition;
	jclass clsIllegalState;
	jfieldID fidX1, fidY1, fidX2, fidY2;
	jfieldID fidCh;
	map<jchar,bool> keymap;
	keyposition* ckeys;
	jchar ch;
	int npairs;
	float* distmatrix;
	float* spacedist;
	jchar* keylist;
	float ax, ay, bx, by;
	float maxdist;

	clsIllegalState = env->FindClass("java/lang/IllegalStateException");

	nkeys = env->GetArrayLength(keys);

	clsKeyPosition = env->FindClass("com/wordlogic/lib/KeyPosition");

	if(env->ExceptionCheck()) {
		env->ThrowNew(clsIllegalState,"class com.wordlogic.lib.KeyPosition could not be loaded");
		return;
	}

	ckeys = new keyposition[nkeys];

	fidX1 = env->GetFieldID(clsKeyPosition,"x1","F");
	fidY1 = env->GetFieldID(clsKeyPosition,"y1","F");
	fidX2 = env->GetFieldID(clsKeyPosition,"x2","F");
	fidY2 = env->GetFieldID(clsKeyPosition,"y2","F");
	fidCh = env->GetFieldID(clsKeyPosition,"ch","C");

	for(c = 0;c < nkeys;c++) {
		key = env->GetObjectArrayElement(keys,c);

		ckeys[c].x1 = env->GetFloatField(key,fidX1);
		ckeys[c].y1 = env->GetFloatField(key,fidY1);
		ckeys[c].x2 = env->GetFloatField(key,fidX2);
		ckeys[c].y2 = env->GetFloatField(key,fidY2);
		ch = env->GetCharField(key,fidCh);
		ckeys[c].ch = ch;

		env->DeleteLocalRef(key);

		if(keymap.count(ch) != 0) {
			env->ThrowNew(clsIllegalState,"duplicate key characters passed in keyPos");
			return;
		}

		keymap[ch] = true;
	}

	sort(ckeys,ckeys+nkeys);

	if(ckeys[0].ch == (jchar)' ') {
		npairs = ((nkeys-1) * (nkeys-2)) / 2;
		s = 1;
	} else {
		npairs = (nkeys * (nkeys-1)) / 2;
		s = 0;
	}

	distmatrix = new float[npairs];
	d = 0;

	for(c = s;c < nkeys;c++) {
		ax = (ckeys[c].x1 + ckeys[c].x2) / 2;
		ay = (ckeys[c].y1 + ckeys[c].y2) / 2;

		for(cb = c+1;cb < nkeys;cb++) {
			bx = (ckeys[cb].x1 + ckeys[cb].x2) / 2;
			by = (ckeys[cb].y1 + ckeys[cb].y2) / 2;

			distmatrix[d] = std::hypot(ax-bx,ay-by);
			d++;
		}
	}

	maxdist = 0;

	for(d = 0;d < npairs;d++) {
		if(distmatrix[d] > maxdist) { maxdist = distmatrix[d]; }
	}

	if(maxdist == 0) {
		env->ThrowNew(clsIllegalState,"The maximum distance between any key pair is 0");
		return;
	}

	for(d = 0; d < npairs; d++) {
		distmatrix[d] = distmatrix[d] / maxdist;
	}

	if(s == 1) {
		spacedist = new float[nkeys-1];

		for(c = 1;c < nkeys;c++) {
			ax = (ckeys[c].x1 + ckeys[c].x2) / 2;
			ay = (ckeys[c].y1 + ckeys[c].y2) / 2;

			spacedist[c-1] = rectPointDist(ax,ay,ckeys[0].x1,ckeys[0].y1,ckeys[0].x2,ckeys[0].y2) / maxdist;
		}
	} else {
		spacedist = NULL;
	}

	keylist = new jchar[nkeys-s];

	for(c = 0;c < nkeys-s;c++) {
		keylist[c] = ckeys[c-s].ch;
	}

	delete[] ckeys;

	gDictManager->getAutocorrect()->loadKeyPositions(keylist,distmatrix,spacedist,nkeys-s);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
JNIEXPORT void JNICALL Java_com_wordlogic_lib_AutoCorrect_setSettings(JNIEnv* env,jobject obj,jint ngramthresh,jboolean correctvalid,jboolean correctcase) 
{
	ShowInfo("JNI_AutoCorrect_setSettings:ngramthresh=%d,  correctvalid=%d, correctcase=%d\n", ngramthresh, correctvalid, correctcase);
//	autocorrect->twogramoverridethreshold = ngramthresh;
	gDictManager->getAutocorrect()->correctvalidwords = correctvalid;
	gDictManager->getAutocorrect()->correctwordcase = correctcase;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
JNIEXPORT jboolean JNICALL Java_com_wordlogic_lib_AutoCorrect_correctionPending(JNIEnv* env,jobject obj) 
{
	ShowInfo("JNI_AutoCorrect_correctionPending called");
	return gDictManager->getAutocorrect()->correctionPending();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
JNIEXPORT jobjectArray JNICALL Java_com_wordlogic_lib_AutoCorrect_getPendingCorrections(JNIEnv* env, jobject obj, jint lim) 
{
	ShowInfo("JNI_AutoCorrect_getPendingCorrections:max = %d\n", lim);

	CorrectionList * list = gDictManager->getAutocorrect()->getPredictions(lim);
	if(list==NULL || list->fillCount==0) {
		ShowInfo("-JNI_AutoCorrect_getPendingCorrections: correction list is empty!\n");
		return NULL;
	}

	jobjectArray jaddedWordsAr;
	int numWordsAdded = list->fillCount;
	jaddedWordsAr = (jobjectArray)env->NewObjectArray(numWordsAdded, env->FindClass("java/lang/String"), env->NewStringUTF(""));
	if(env->ExceptionCheck()) {
		env->ExceptionDescribe();
		return NULL;
	}
	for(int i = 0; i < numWordsAdded; i++) 
	{
		ShowInfo("-JNI_AutoCorrect_getPendingCorrectons: add correction %s,\n", toA(list->corrections[i].word));
		jstring jaddedword = createjstring(env, list->corrections[i].word);
		if (jaddedword == NULL)
		{
			ShowInfo("--!!ERROR!!JNI_AutoCorrect_getPendingCorrections:failed adding jstring (%s)\n", toA(list->corrections[i].word));
			return (0);
		}
	    env->SetObjectArrayElement(jaddedWordsAr, i, jaddedword);
	}

	return jaddedWordsAr;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
/** testing functions **/
JNIEXPORT jfloat JNICALL Java_com_wordlogic_lib_AutoCorrect_testKeyDist(JNIEnv* env,jobject obj,jchar ch1,jchar ch2) 
{
	ShowInfo("JNI_AutoCorrect_testKeyDist:\n");
	return gDictManager->getAutocorrect()->keyDist(ch1,ch2);
}

}
////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif