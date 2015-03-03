// this file defines all of the JNI functions. 

#if defined (WIN32)
#include "jni.h"
#define JNIEXPORT
#else
#include <jni.h>
#endif
#include "WordLogicDLL2.h"

static AutoCorrect* autocorrect;

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL Java_com_wordlogic_lib_AutoCorrect_init(JNIEnv* env,jclass cls,jstring filename,jstring personal) 
{
	const char* sfilename;
	const char* spersonal;
	
	sfilename = env->GetStringUTFChars(filename,NULL);
	spersonal = env->GetStringUTFChars(personal,NULL);

	autocorrect = new AutoCorrect(sfilename,spersonal,env);

	env->ReleaseStringUTFChars(filename,sfilename);
	env->ReleaseStringUTFChars(personal,spersonal);
}

JNIEXPORT void JNICALL Java_com_wordlogic_lib_AutoCorrect_addChar(JNIEnv* env,jobject obj,jchar ch) 
{
	autocorrect->setEnv(env,obj);
	autocorrect->addChar(ch);
}

JNIEXPORT void JNICALL Java_com_wordlogic_lib_AutoCorrect_backspace(JNIEnv* env,jobject obj,jint n) 
{
	autocorrect->setEnv(env,obj);
	autocorrect->backspace(n);
}

JNIEXPORT void JNICALL Java_com_wordlogic_lib_AutoCorrect_addString(JNIEnv* env,jobject obj,jstring s) 
{
	const jchar* chars;
	int nchars;

	nchars = env->GetStringLength(s);
	chars = env->GetStringChars(s,NULL);
	
	autocorrect->setEnv(env,obj);

	autocorrect->addChars(chars,nchars);

	env->ReleaseStringChars(s,chars);
}

JNIEXPORT void JNICALL Java_com_wordlogic_lib_AutoCorrect_addPrediction(JNIEnv* env,jobject obj,jstring s) 
{
	const jchar* chars;
	int nchars;

	nchars = env->GetStringLength(s);
	chars = env->GetStringChars(s,NULL);
	
	autocorrect->setEnv(env,obj);

	autocorrect->addPrediction(chars,nchars);

	env->ReleaseStringChars(s,chars);
}

JNIEXPORT void JNICALL Java_com_wordlogic_lib_AutoCorrect_replaceBuffers(JNIEnv* env,jobject obj,jstring beforeCursor,jstring afterCursor) 
{
	const jchar* chars;
	int nchars;
	
	autocorrect->setEnv(env,obj);

	if(beforeCursor == NULL) {
		autocorrect->setBeforeCursor(NULL,0);
	} else {
		nchars = env->GetStringLength(beforeCursor);
		chars = env->GetStringChars(beforeCursor,NULL);

		autocorrect->setBeforeCursor(chars,nchars);

		env->ReleaseStringChars(beforeCursor,chars);
	}

	if(afterCursor == NULL) {
		autocorrect->setAfterCursor(NULL,0);
	} else {
		nchars = env->GetStringLength(afterCursor);
		chars = env->GetStringChars(afterCursor,NULL);

		autocorrect->setAfterCursor(chars,nchars);

		env->ReleaseStringChars(afterCursor,chars);
	}
}

JNIEXPORT void JNICALL Java_com_wordlogic_lib_AutoCorrect_reverseCorrection(JNIEnv* env,jobject obj,jint si,jstring generated,jstring replacement)
{
	autocorrect->setEnv(env,obj);
	autocorrect->reverseCorrection(si,generated,replacement);
}

JNIEXPORT jstring JNICALL Java_com_wordlogic_lib_AutoCorrect_getSecondCorrection(JNIEnv* env,jobject obj,jint si,jstring original,jstring firstcorrect)
{
	const jchar* ochars;
	int olen;
	const jchar* achars;
	int alen;
	jchar* cchars;
	int clen;
	
	autocorrect->setEnv(env,obj);

	olen = env->GetStringLength(original);
	ochars = env->GetStringChars(original,NULL);

	alen = env->GetStringLength(firstcorrect);
	achars = env->GetStringChars(firstcorrect,NULL);

	autocorrect->getSecondCorrection(si,ochars,olen,achars,alen,&cchars,&clen);

	env->ReleaseStringChars(original,ochars);
	env->ReleaseStringChars(firstcorrect,achars);

	if(clen == 0) {
		return NULL;
	} else {
		return env->NewString(cchars,clen);
	}
}

JNIEXPORT jboolean JNICALL Java_com_wordlogic_lib_AutoCorrect_isWord(JNIEnv* env,jobject obj,jstring str) 
{
	bool ret;
	const jchar* chars;
	int n;

	n = env->GetStringLength(str);
	chars = env->GetStringChars(str,NULL);
	
	autocorrect->setEnv(env,obj);

	ret = autocorrect->isWord(chars,n);

	env->ReleaseStringChars(str,chars);

	return ret;
}

JNIEXPORT void JNICALL Java_com_wordlogic_lib_AutoCorrect_loadKeyboardLayout(JNIEnv* env,jobject obj,jobjectArray keys) 
{
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

	for(d = 0;d < npairs;d++) {
		distmatrix[d] = distmatrix[d] / maxdist;
	}

	if(s == 1) {
		spacedist = new float[nkeys-1];

		for(c = 1;c < nkeys;c++) {
			ax = (ckeys[c].x1 + ckeys[c].x2) / 2;
			ay = (ckeys[c].y1 + ckeys[c].y2) / 2;

			spacedist[c-1] = std::rectPointDist(ax,ay,ckeys[0].x1,ckeys[0].y1,ckeys[0].x2,ckeys[0].y2) / maxdist;
		}
	} else {
		spacedist = NULL;
	}

	keylist = new jchar[nkeys-s];

	for(c = 0;c < nkeys-s;c++) {
		keylist[c] = ckeys[c-s].ch;
	}

	delete[] ckeys;

	autocorrect->loadKeyPositions(keylist,distmatrix,spacedist,nkeys-s);
}

JNIEXPORT void JNICALL Java_com_wordlogic_lib_AutoCorrect_saveUpdates(JNIEnv* env,jobject obj) {
	autocorrect->setEnv(env,obj);

	autocorrect->save();
}

JNIEXPORT jstring JNICALL getSecondCorrection(int si,jstring original,jstring firstautocorrect) {
	return NULL;
}

JNIEXPORT jboolean JNICALL Java_com_wordlogic_lib_AutoCorrect_correctionPending(JNIEnv* env,jobject obj) {
	autocorrect->setEnv(env,obj);

	if(autocorrect->correctionPending()) {
		return JNI_TRUE;
	} else {
		return JNI_FALSE;
	}
}

JNIEXPORT void JNICALL Java_com_wordlogic_lib_AutoCorrect_addWord(JNIEnv* env,jobject obj,jstring str) 
{
	const jchar* chars;
	int n;

	n = env->GetStringLength(str);
	chars = env->GetStringChars(str,NULL);
	
	autocorrect->setEnv(env,obj);

	autocorrect->personalDict->addword(chars,n,1);

	env->ReleaseStringChars(str,chars);
}

JNIEXPORT void JNICALL Java_com_wordlogic_lib_AutoCorrect_deleteWord(JNIEnv* env,jobject obj,jstring str) 
{
	const jchar* chars;
	int n;

	n = env->GetStringLength(str);
	chars = env->GetStringChars(str,NULL);
	
	autocorrect->setEnv(env,obj);

	autocorrect->personalDict->deleteword(chars,n);

	env->ReleaseStringChars(str,chars);
}

JNIEXPORT jobjectArray JNICALL Java_com_wordlogic_lib_AutoCorrect_listWords(JNIEnv* env,jobject obj) 
{
	vector<personalword>* words;
	int c;
	jclass clsString;
	jobjectArray ret;
	jstring cstr;

	clsString = env->FindClass("java/lang/String");
	if(env->ExceptionCheck()) {
		env->ExceptionDescribe();
		return NULL;
	}

	autocorrect->setEnv(env,obj);

	words = autocorrect->personalDict->listwords();

	if(words->empty()) {
		delete words;
		return NULL;
	}

	ret = env->NewObjectArray(words->size(),clsString,NULL);
	if(env->ExceptionCheck()) {
		env->ExceptionDescribe();
		return NULL;
	}

	for(c = 0;c < words->size();c++) {
		cstr = env->NewString((*words)[c].getChars(),(*words)[c].getLength());
		if(env->ExceptionCheck()) {
			env->ExceptionDescribe();
			return NULL;
		}

		env->SetObjectArrayElement(ret,c,cstr);
		if(env->ExceptionCheck()) {
			env->ExceptionDescribe();
			return NULL;
		}

		env->DeleteLocalRef(cstr);
		if(env->ExceptionCheck()) {
			env->ExceptionDescribe();
			return NULL;
		}
	}

	delete words;

	return ret;
}

JNIEXPORT void JNICALL Java_com_wordlogic_lib_AutoCorrect_setSettings(JNIEnv* env,jobject obj,jint ngramthresh,jboolean correctvalid,jboolean correctcase) 
{
	autocorrect->twogramoverridethreshold = ngramthresh;
	autocorrect->correctvalidwords = correctvalid;
	autocorrect->correctwordcase = correctcase;
}

JNIEXPORT jobjectArray JNICALL Java_com_wordlogic_lib_AutoCorrect_getPendingCorrections(JNIEnv* env,jobject obj,jint lim) 
{
	vector<acMatch>* pred;
	jclass clsString;
	int c;
	jstring cstr;
	jobjectArray ret;

	clsString = env->FindClass("java/lang/String");
	if(env->ExceptionCheck()) {
		env->ExceptionDescribe();
		return NULL;
	}

	autocorrect->setEnv(env,obj);

	pred = autocorrect->getPredictions(lim);

	if(pred->empty()) {
		delete pred;
		return NULL;
	}

	ret = env->NewObjectArray(pred->size(),clsString,NULL);
	if(env->ExceptionCheck()) {
		env->ExceptionDescribe();
		return NULL;
	}

	for(c = 0;c < pred->size();c++) {
		cstr = env->NewString((*pred)[c].chars.c_str(),(*pred)[c].chars.length());
		if(env->ExceptionCheck()) {
			env->ExceptionDescribe();
			return NULL;
		}

		env->SetObjectArrayElement(ret,c,cstr);
		if(env->ExceptionCheck()) {
			env->ExceptionDescribe();
			return NULL;
		}

		env->DeleteLocalRef(cstr);
		if(env->ExceptionCheck()) {
			env->ExceptionDescribe();
			return NULL;
		}
	}

	delete pred;

	return ret;
}

}
