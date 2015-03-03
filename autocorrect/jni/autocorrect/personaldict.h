#ifndef PERSONALDICT_H
#define PERSONALDICT_H

#if defined (WIN32)
#include "../jni.h"
#else
#include <jni.h>
#endif

#include <map>
#include <algorithm>
#include <fstream>
#include <vector>

class personaldict;

#include "hash.h"
#include "..\autocorrect.h"
#include "personalword.h"

using namespace std;

class personaldict {
public:
	personaldict(const char* fname,AutoCorrect* ac);
	~personaldict();

	void add2gram(int h1,int h2);
	int get2gramfreq(int h1,int h2);

	void addword(const jchar* s,int n,float p);
	void deleteword(const jchar* s,int n);
	vector<personalword>* listwords();

	int levenshtein(const jchar* s1,int len1,const jchar* s2,int len2);
	void getCorrections(const jchar* s,int slen,vector<acMatch>* corrections,int previoushash,int maxcost);

	void logout(const char* s);

	JNIEnv* env;

	friend class AutoCorrect;
	
	void save();

private:
	char* filename;
	int wordsentered;
	map<pair<int,int>,pair<int,unsigned int> > ngrams;
	map<int,personalword> words;
	AutoCorrect* autocorrect;
};

#endif