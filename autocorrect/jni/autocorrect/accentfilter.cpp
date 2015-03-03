
#include "accentfilter.h"

accentFilter::accentFilter(jchar* aindex,jchar* avalues,int n) {
	index = aindex;
	values = avalues;
	naccents = n;
}

accentFilter::~accentFilter() {
	delete[] index;
	delete[] values;
}

/*****
* areEqual
*	Determines whether two characters are equal, without respect to their accents.
*	Short circuit logic is used wherever possible as this function needs to be very fast.
*	For languages with character sets that contain accents this function is used during
*	every Levenshtein distance character comparison.
*
*	TODO: all normal non-accented characters fall in a very small range as do accented characters. Look to see if something's in these ranges first to speed things up.
*	for example if ch1 is not found and ch2 is not in the normal value range then we can return false without doing a second binary search.
*
*	Similarly, if ch1 != ch2 and both are outside of the index range then we can return false.
*
*	ch1		-	the first character
*	ch2		-	the second character
*
*	Jonathon Simister (December, 2012)
******/
bool accentFilter::areEqual(jchar ch1, jchar ch2) {
	pair<jchar*,jchar*> bounds;
	int i1, i2;
	bool f1, f2;

	f1 = false;
	f2 = false;

	if(ch1 == ch2) { return true; }

	bounds = equal_range(index,index+naccents,ch1);

	if((bounds.first - bounds.second) != 0) { f1 = true; } 

	i1 = bounds.first - index;

	bounds = equal_range(index,index+naccents,ch2);

	if((bounds.first - bounds.second) != 0) { f2 = true; } 

	i2 = bounds.first - index;

	if(f1 && f2) {
		if(values[i1] == values[i2]) { return true; }
		else { return false; }
	} else if(f1) {
		if(values[i1] == ch2) { return true; }
		else { return false; }
	} else if(f2) {
		if(values[i2] == ch1) { return true; }
		else { return false; }
	} else { // !f1 and !f2 and we already know the keys are not equal
		return false;
	}
}

/*****
* removeAccent
*	Return the accent-free version of a character based on current language settings in the auto-correct dictionary.
*
*	Jonathon Simister (December, 2012)
******/
jchar accentFilter::removeAccent(jchar ch) {
	pair<jchar*,jchar*> bounds;
	int i;

	bounds = equal_range(index,index+naccents,ch);

	i = bounds.first - index;

	if((bounds.first - bounds.second) == 0) { return ch; }

	return values[i];
}

void accentFilter::logout(const char* s) {
	jstring str;
	jclass clsSystem;
	jfieldID fidOut;
	jobject out;
	jclass clsPrintStream;
	jmethodID midPrintLn;

#if debug
	str = env->NewStringUTF(s);

	clsSystem = env->FindClass("java/lang/System");

	fidOut = env->GetStaticFieldID(clsSystem,"out","Ljava/io/PrintStream;");

	out = env->GetStaticObjectField(clsSystem,fidOut);

	clsPrintStream = env->GetObjectClass(out);

	midPrintLn = env->GetMethodID(clsPrintStream,"println","(Ljava/lang/String;)V");

	env->CallVoidMethod(out,midPrintLn,str);
#endif
}