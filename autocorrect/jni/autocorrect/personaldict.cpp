#include "personaldict.h"

#define debug 0

#define wordfreq 10000

personaldict::personaldict(const char *fname,AutoCorrect* ac) {
	fstream file;
	int version;
	int n2grams;
	int c;
	int h1, h2, freq;
	pair<int,int> key;
	int nwords;
	int wordlen;
	float wordprob;
	jchar wordchars[100];
	int wordhash;
	int wordattr;

	filename = new char[strlen(fname)+1];
	strcpy(filename,fname);

	autocorrect = ac;

	file.open(fname,ios::binary|ios::in);

	if(file.is_open()) {
		file.read((char*)&version,sizeof(int));
	} else {
		version = 0;
	}

	if(version == 3) {
		file.read((char*)&n2grams,sizeof(int));
		file.read((char*)&wordsentered,sizeof(int));
		file.read((char*)&nwords,sizeof(int));

		for(c = 0;c < n2grams;c++) {
			file.read((char*)&h1,sizeof(int));
			file.read((char*)&h2,sizeof(int));
			file.read((char*)&freq,sizeof(int));

			key.first = h1;
			key.second = h2;

			ngrams[key].first = freq;
			ngrams[key].second = wordsentered;
		}

		for(c = 0;c < nwords;c++) {
			file.read((char*)&wordlen,sizeof(int));
			file.read((char*)&wordprob,sizeof(float));
			file.read((char*)&wordattr,sizeof(int));

			if(wordlen > 100) { break; }

			file.read((char*)wordchars,sizeof(jchar)*wordlen);
			
			wordhash = Hash::wordHash(wordchars,wordlen);

			words[wordhash] = personalword(wordchars,wordlen,wordprob);
			words[wordhash].attrs = wordattr;
		}
	} else {
		wordsentered = 0;
	}

	file.close();
}

personaldict::~personaldict() {
	delete[] filename;
}

void personaldict::save() {
	fstream file;
	int v = 3;
	int n2grams;
	map<pair<int,int>,pair<int,unsigned int> >::iterator it;
	map<int,personalword>::iterator wordit;
	int ha, hb;
	int freq;
	int lastupdate;
	int nwords;
	int wordlen;
	float wordprob;
	int wordattr;

	file.open(filename,ios::out|ios::binary|ios::trunc);

	for(it = ngrams.begin();it != ngrams.end();) {
		freq = it->second.first;
		lastupdate = it->second.second;

		freq -= (wordsentered - lastupdate) / wordfreq;

		if(freq <= 0) {
			ngrams.erase(it++);
		} else {
			++it;
		}
	}

	n2grams = ngrams.size();

	nwords = words.size();

	file.write((const char*)&v,sizeof(int));
	file.write((const char*)&n2grams,sizeof(int));
	file.write((const char*)&wordsentered,sizeof(int));
	file.write((const char*)&nwords,sizeof(int));

	for(it = ngrams.begin();it != ngrams.end();it++) {
		ha = it->first.first;
		hb = it->first.second;
		freq = it->second.first;

		file.write((const char*)&ha,sizeof(int));
		file.write((const char*)&hb,sizeof(int));
		file.write((const char*)&freq,sizeof(int));
	}

	for(wordit = words.begin();wordit != words.end();wordit++) {
		wordlen = wordit->second.getLength();
		wordprob = wordit->second.prob;
		wordattr = wordit->second.attrs;

		file.write((const char*)&wordlen,sizeof(int));
		file.write((const char*)&wordprob,sizeof(float));
		file.write((const char*)&wordattr,sizeof(int));
		file.write((const char*)wordit->second.getChars(),sizeof(jchar)*wordlen);			
	}

	file.close();
}

void personaldict::addword(const jchar* s,int n,float p) {
	int c;
	jchar* slower;
	int hash;
	map<int,personalword>::iterator it;

	slower = new jchar[n];

	for(c = 0;c < n;c++) {
		slower[c] = autocorrect->toLower(s[c]);
	}

	hash = Hash::wordHash(slower,n);

	if(words.count(hash) == 0) {
		words[hash] = personalword(slower,n,p);

		if(autocorrect->containsNoLowerCase(s,n)) {
			words[hash].attrs = 2;
		} else if(autocorrect->isUpper(s[0])) {
			words[hash].attrs = 1;
		}
	} else {
		words[hash].prob += p;
	}

	delete[] slower;
}

void personaldict::deleteword(const jchar* s,int n) {
	int c;
	jchar* slower;
	int hash;
	map<int,personalword>::iterator it;

	slower = new jchar[n];

	for(c = 0;c < n;c++) {
		slower[c] = autocorrect->toLower(s[c]);
	}

	hash = Hash::wordHash(slower,n);

	words.erase(hash);

	delete[] slower;
}

vector<personalword>* personaldict::listwords() {
	map<int,personalword>::iterator it;
	vector<personalword>* ret;

	ret = new vector<personalword>(words.size());

	for(it = words.begin();it != words.end();it++) {
		if(it->second.getLength() > 0) {
			ret->push_back(it->second);
		}
	}

	return ret;
}

void personaldict::add2gram(int h1,int h2) {
	pair<int,int> key;
	char* buf;
	pair<int,unsigned int>* value;

	key.first = h1;
	key.second = h2;

	buf = new char[100];

	if(ngrams.count(key) == 0) {
		value = &(ngrams[key]);

		value->first = 1;
		value->second = wordsentered;

		sprintf(buf,"creating new ltg entry for %d,%d",h1,h2);
		logout(buf);
	} else {
		value = &(ngrams[key]);

		value->first++;
		value->first -= (wordsentered - value->second) / wordfreq;
		if(value->first < 1) { value->first = 1; }
		value->second = wordsentered;

		sprintf(buf,"updated ltg entry for %d,%d to %d",h1,h2,value->first);
		logout(buf);
	}

	wordsentered += 2;

	delete[] buf;
}

void personaldict::logout(const char* s) {
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

int personaldict::get2gramfreq(int h1,int h2) {
	pair<int,int> key;
	pair<int,unsigned int>* value;
	int freq;

	key.first = h1;
	key.second = h2;

	if(ngrams.count(key) == 0) {
		return 0;
	} else {
		value = &(ngrams[key]);

		freq = value->first - ((wordsentered - value->second) / wordfreq);

		if(freq <= 0) {
			ngrams.erase(key);
		}

		return max(freq,0);
	}
}

int personaldict::levenshtein(const jchar* s1,int len1,const jchar* s2,int len2) {
	int deleteCost;
	int insertCost;
	int replaceCost;
	int i, j;
	int c;
	jchar ch1, ch2;
	int** matrix;
	int ret;

	matrix = new int*[len1+1];

	for(c = 0;c < len1+1;c++) {
		matrix[c] = new int[len2+1];
		matrix[c][0] = c;
	}

	for(c = 0;c < len2+1;c++) {
		matrix[0][c] = c;
	}

	for (i = 1; i <= len1; i++) {
		ch1 = s1[i-1];
		for (j = 1; j <= len2; j++) {
			ch2 = s2[j-1];
			if(ch1 == ch2) {
				matrix[i][j] = matrix[i-1][j-1];
			} else {
				deleteCost = matrix[i-1][j] + 1;
				insertCost = matrix[i][j-1] + 1;
				replaceCost = matrix[i-1][j-1] + 1;

				matrix[i][j] = min(deleteCost,min(insertCost,replaceCost));
			}
		}
	}

	ret = matrix[len1][len2];

	for(c = 0;c < len1+1;c++) {
		delete[] matrix[c];
	}

	delete[] matrix;

	return ret;
}

char* jstoc(basic_string<jchar> s) {
	char* ret = (char*)calloc(s.length()+1,1);
	unsigned int c;

	for(c = 0;c < s.length();c++) {
		ret[c] = (char)s[c];
	}

	ret[c] = 0;

	return ret;
}

void personaldict::getCorrections(const jchar* s,int slen,vector<acMatch>* corrections,int previoushash,int maxcost) {
	map<int,personalword>::iterator it;
	vector<personalword>* ret;
	personalword* word;
	int lev;
	acMatch match;
	char buf[200];

	logout("personalDict->getCorrections() called");

	for(it = words.begin();it != words.end();it++) {
		word = &(it->second);

		lev = levenshtein(s,slen,word->getChars(),word->getLength());

		if(lev <= maxcost) {
			match = word->generateMatch(lev);

			match.learnedtg = get2gramfreq(previoushash,word->getHash());

			corrections->push_back(match);

			sprintf(buf,"got learned word for correction %s with attrs = %d",jstoc(match.chars),word->attrs);

			logout(buf);
		}
	}
}