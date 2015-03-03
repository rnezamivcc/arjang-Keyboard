
#include "autocorrect.h"

#define MaxBufferSize 1000

#define debug 0

AutoCorrect::AutoCorrect(const char* filename,const char* personalfilename,JNIEnv* jnienv) {
	ifstream acdfile;
	int* header;
	jclass clsIllegalArg;
	int* blocksizes;
	char** blocks;
	int nblocks;
	int c;
	char* err;
	int read;
	char buf[10];
	int nextblock;

	env = jnienv;

	acdfile.open(filename,ios::binary|ios::in);

	header = new int[4];

	acdfile.read((char*)header,sizeof(int)*4);

	clsIllegalArg = env->FindClass("java/lang/IllegalArgumentException");

	if(header[0] != 0x04030201) {
		// TODO: write code to read files of other endianesses.
		err = new char[250];

		sprintf(err,"AutoCorrect dictionary file (%s) has wrong endianess: 0x%X",filename,header[0]);

		env->ThrowNew(clsIllegalArg,err);

		delete[] err;
		return;
	}

	if(header[1] != 5) {
		env->ThrowNew(clsIllegalArg,"This code is written for version 5 auto-correct dictionary files, but another version has been loaded");
	}

	nblocks = header[2];

	blocks = new char*[nblocks];
	blocksizes = new int[nblocks];

	for(c = 0;c < nblocks;c++) {
		acdfile.read((char*)&(blocksizes[c]),sizeof(int));

		blocks[c] = new char[blocksizes[c]];

		acdfile.read(blocks[c],blocksizes[c]);

		read = blocksizes[c] % 4;

		if(read != 0) {
		acdfile.read((char*)buf,4-read); // read DWORD alignment padding from stream
		}
	}

	acdfile.close();

	graph = new acGraph((const unsigned char*)blocks[0]);

	nhashes = blocksizes[1] / 4;

	hashes = (const int*)blocks[1];

	if((header[3] & 1) == 1) {
		tgindex = (tgHeader*)blocks[2];

		twograms = (twogram*)blocks[3];

		ntwograms = blocksizes[3]/12;
	
		hasngrams = true;

		probs = blocks[4];

		nextblock = 5;
	} else {
		hasngrams = false;

		probs = blocks[2];

		nextblock = 3;
	}

	if((header[3] & 2) == 2) {
		deaccent = new accentFilter((jchar*)(blocks[nextblock]),(jchar*)(blocks[nextblock+1]),blocksizes[nextblock]/2);
		
		nextblock += 2;
	} else {
		deaccent = NULL;
	}

	personalDict = new personaldict(personalfilename,this);

	graph->setAccentFilter(deaccent);

	keylist = NULL;
	spacedist = NULL;
	distmatrix = NULL;

	twogramoverridethreshold = 15;
	correctvalidwords = true;
	correctwordcase = true;
}

void AutoCorrect::setEnv(JNIEnv *jnienv,jobject obj) {
	env = jnienv;
	jobj = obj;
	jclass clsPrintStream;
	jfieldID fidOut;
	jclass clsSystem;

	if(deaccent != NULL) {
	deaccent->env = jnienv;
	}

	personalDict->env = jnienv;

	clsCharacter = env->FindClass("java/lang/Character");
	clsSystem = env->FindClass("java/lang/System");

	fidOut = env->GetStaticFieldID(clsSystem,"out","Ljava/io/PrintStream;");

	SystemOut = env->GetStaticObjectField(clsSystem,fidOut);

	clsPrintStream = env->GetObjectClass(SystemOut);

	midPrintLn = env->GetMethodID(clsPrintStream,"println","(Ljava/lang/String;)V");

	clsCharacter = env->FindClass("java/lang/Character");
	midToLowerCase = env->GetStaticMethodID(clsCharacter,"toLowerCase","(C)C");
	midIsUpperCase = env->GetStaticMethodID(clsCharacter,"isUpperCase","(C)Z");
	midToUpperCase = env->GetStaticMethodID(clsCharacter,"toUpperCase","(C)C");
	midIsLowerCase = env->GetStaticMethodID(clsCharacter,"isLowerCase","(C)Z");
}

void AutoCorrect::addChar(jchar ch) {
	char buf[200];

	buffer.push_back(acchar(ch));

	sprintf(buf,"adding character %c",(char)ch);

	//javalog(buf);


	if(buffer.size() > MaxBufferSize) {
		buffer.pop_front();
	}

	bufferUpdated();
}

void AutoCorrect::addChars(const jchar* chars,int n) {
	int c;
	
	if(n >= MaxBufferSize) {
		setBeforeCursor(chars,n);
	} else {
		for(c = 0;c < n;c++) {
			buffer.push_back(chars[c]);
		}

		while(buffer.size() > MaxBufferSize) {
			buffer.pop_front();
		}
	}

	bufferUpdated();
}

void AutoCorrect::addPrediction(const jchar* chars,int n) {
	int c;

	if(n >= MaxBufferSize) {
		setBeforeCursor(chars,n);
	} else {
		for(c = 0;c < n;c++) {
			buffer.push_back(chars[c]);
		}

		while(buffer.size() > MaxBufferSize) {
			buffer.pop_front();
		}
	}

	// mark every character in the buffer as correct when a prediction is added
	for(c = 0;c < buffer.size();c++) {
		buffer[c].status = 1;
	}
}

void AutoCorrect::backspace(int n) {
	int c;
	deque<jchar>::iterator it;
	bool lastsep;
	int nwords;
	jclass clsAutoCorrect;
	jfieldID fidCallback;
	jobject objCallback;
	jclass clsInterface;
	jmethodID midBufferLow;

	for(c = 0;c < n && !buffer.empty();c++) {
		buffer.pop_back();
	}

	if(!bufferUpdated()) { // if no correction has been made, we'll consider caling bufferLow in the AutoCorrectInterface
		// count the words in the auto-correct buffer
		
		if(buffer.size() > 2) {
			nwords = 0;
			lastsep = isSeparator(buffer.back().ch);

			for(c = buffer.size()-2;c >= 0;c--) {
				if(isSeparator(buffer[c].ch)) {
					lastsep = true;
				} else {
					if(lastsep) {
						nwords++;
					}

					lastsep = false;
				}
			}
		} else {
			nwords = 1; // or possibly 0
		}

		/*if(nwords < 3) {
			clsAutoCorrect = env->FindClass("com/wordlogic/lib/AutoCorrect");
			if(env->ExceptionCheck()) {
				logout("exception in autocorrect.cpp at line %d",__LINE__);
				env->ExceptionDescribe();
			}

			fidCallback = env->GetFieldID(clsAutoCorrect,"callback","Lcom/wordlogic/lib/AutoCorrectInterface;");
			if(env->ExceptionCheck()) {
				logout("exception in autocorrect.cpp at line %d",__LINE__);
				env->ExceptionDescribe();
			}

			objCallback = env->GetObjectField(jobj,fidCallback);
			if(env->ExceptionCheck()) {
				logout("exception in AutoCorrect::backspace() at call to GetObjectField(jobj,fidCallback)");
				return;
			}

			if(objCallback == NULL) {
				return;
			}

			clsInterface = env->GetObjectClass(objCallback);
			if(env->ExceptionCheck()) {
				logout("exception in autocorrect.cpp at line %d",__LINE__);
				env->ExceptionDescribe();
				return;
			}

			midBufferLow = env->GetMethodID(clsInterface,"bufferLow","(I)V");
			if(env->ExceptionCheck()) {
				logout("exception in autocorrect.cpp at line %d",__LINE__);
				env->ExceptionDescribe();
				return;
			}

			env->CallVoidMethod(objCallback,midBufferLow,(jint)buffer.size());
		}*/
	}
}

void AutoCorrect::setBeforeCursor(const jchar *chars, int n) {
	int o, c;

	logout("setBeforeCursor() called");
	
	buffer.clear();

	if(n > MaxBufferSize) {
		o = n - MaxBufferSize;
	} else {
		o = 0;
	}

	for(c = o;c < n;c++) {
		buffer.push_back(chars[c]);
	}
}

void AutoCorrect::setAfterCursor(const jchar* chars,int n) {
	int o, c;
	
	aftercursor.clear();

	if(n > MaxBufferSize) {
		o = n - MaxBufferSize;
	} else {
		o = 0;
	}

	for(c = o;c < n;c++) {
		aftercursor.push_back(chars[c]);
	}
}

bool AutoCorrect::bufferUpdated() {
	int c;
	int lastchari, firstchari;
	jchar* word;
	jchar* wordo;
	int wordhash;
	int wordprob;
	jchar* priorword;
	jchar* prior2word;
	int wordlen, priorlen, prior2len;
	int priori, prior2i;
	vector<acMatch> corrections;
	int maxcost;
	int priorhash;
	int separators;
	bool correctionmade = false;
	bool uppercaseword;
	int wi;

	if(buffer.empty()) {
		logout("buffer empty");
		return false;
	} else {
		logout("buffer contents on next line:");
		logBuffer();
		logBufferStatus();
	}

	// if the last character is a space, then we take this as a cue to perform auto-correct
	if(isSeparator(buffer.back().ch)) {
		c = buffer.size() - 2;

		separators = 1;

		while(c >= 0 && isSeparator(buffer[c].ch)) {
			c--;
			separators++;
		}

		lastchari = c+1;
		firstchari = 0;

		while(c >= 0) {
			if(isSeparator(buffer[c].ch)) {
				firstchari = c+1;
				break;
			}

			c--;
		}

		wordlen = lastchari - firstchari;

		if(wordlen > 0) {
			wordo = new jchar[wordlen];
			word = new jchar[wordlen];

			for(c = firstchari;c < lastchari;c++) {
				wordo[c-firstchari] = buffer[c].ch;
			}

			uppercaseword = isUpper(wordo[0]);

			for(c = 0;c < wordlen;c++) {
				word[c] = toLower(wordo[c]);
			}

			char* out;

			int psi, pei;
			int p2si, p2ei;

			priori = -1;
			priorlen = 0;
			prior2i = -1;
			prior2len = 0;

			if(firstchari > 0 && hasngrams) { // look at words previous to the one being corrected, but don't bother if we don't have any 2-gram data
				getLastWord(0,firstchari-1,&psi,&pei);

				if((pei - psi) > 0) {
					priorlen = pei - psi;

					priorword = new jchar[priorlen];

					for(c = 0;c < priorlen;c++) {
						priorword[c] = toLower(buffer[psi+c].ch);
					}

					out = (char*)calloc(200,1);

					sprintf(out,"previous word range = [%d,%d]",psi,pei);

					logout(out);

					sprintf(out,"previous word = '%s'",jstoc(basic_string<jchar>(priorword,priorlen)));

					logout(out);

					priori = getHashIndex(priorword,priorlen,&priorhash);

					sprintf(out,"previous word index = %d",priori);

					logout(out);

					if(psi > 1) {

					getLastWord(0,psi-1,&p2si,&p2ei);

					sprintf(out,"p2si = %d, p2ei = %d",p2si,p2ei);

					logout(out);

					if((p2ei - p2si) > 0) {
						prior2len = p2ei - p2si;

						prior2word = new jchar[prior2len];

						for(c = 0;c < prior2len;c++) {
						prior2word[c] = toLower(buffer[p2si+c].ch);
						}

						prior2i = getHashIndex(prior2word,prior2len);
					}

					}
				}
			}

			if(isWord(word,wordlen,&wordprob,&wordhash,&wi) == 0) {
				if(!determinedCorrect(firstchari,lastchari)) {
				if(wordlen <= 3) {
					maxcost = 1;
				} else if(wordlen <= 5){
					maxcost = 2;
				} else {
					maxcost = 3;
				}

				if(wordlen > 1) {

				getCorrections(word,wordlen,&corrections,priori,maxcost);

				getMissedSpaces(word,wordlen,&corrections,priori);

				if(priorlen == 0) { priorhash = 0; }
				personalDict->getCorrections(word,wordlen,&corrections,priorhash,maxcost);

				addKeyboardDistance(word,wordlen,&corrections);

				sort(corrections.begin(),corrections.end(),&acMatch::sortAdjScore);

				printCorrections(&corrections);

				if(corrections.size() > 0) {
					correctionmade = true;

					makeCorrection(firstchari,lastchari,&(corrections[0]),uppercaseword,0);
				}

				}

				goto donecorrections;
				}
			} else {
				if(wi == -1) { // the last word entered is a valid personal word
					personalDict->words[wordhash].prob += (float)0.1;
				}
			}

			acMatch* match;

			if(!determinedCorrect(firstchari,lastchari)) {

				// special case to correct proper nouns which have been written in all lower-case
				if((wordprob & 128) == 128 && isLower(buffer[firstchari].ch)) {
					logout("lower-case proper noun found");

					match = new acMatch(basic_string<jchar>(word,wordlen),0);
					match->accronym = false;
					match->propernoun = true;

					correctionmade = true;
					makeCorrection(firstchari,lastchari,match,uppercaseword,3);

					delete match;

					goto donecorrections;
				}

				// special case to correct accronyms which have been written with one or more character lower-case
				if((wordprob & 64) == 64 && !containsNoLowerCase(wordo,wordlen)) {
					logout("lower-case accronym found");

					match = new acMatch(basic_string<jchar>(word,wordlen),0);
					match->accronym = true;
					match->propernoun = false;

					correctionmade = true;
					makeCorrection(firstchari,lastchari,match,uppercaseword,4);

					delete match;

					goto donecorrections;
				}

			}
			
			if(priorlen != 0) {
				if(priori == -1) {
					correctprevious:
					if(determinedCorrect(psi,pei)) {
						goto donecorrections;
					}

					// only correct an already valid word based on a 2-gram with the next word if there is no 2-gram for the previous word.
					if(prior2i == -1) {
						// this and the else if condition below are actually supposed to result in the exact same thing
						// Unfortunately, GCC isn't as clever with short-circuiting as Visual C++
						// getTwogramFreq(tgindex[prior2i],priorhash) == 0 || prior2i == -1 works in VC++ but not GCC
						goto ln392;
					} else if(getTwogramFreq(tgindex[prior2i],priorhash) == 0) {
ln392:					if(prior2i != -1) {
						sprintf(out,"no matching 2-gram for 2nd word back = '%s'",jstoc(basic_string<jchar>(prior2word,prior2len)));

						logout(out);
						}

						corrections.clear();

						getCorrections(priorword,priorlen,&corrections,-1,1);

						addForwardTwogramProb(&corrections,wordhash);

						sort(corrections.begin(),corrections.end(),&acMatch::sortNgramOnly);

						printCorrections(&corrections);

						if(corrections.size() > 0) {
							if(corrections[0].tgfreq > twogramoverridethreshold) {
								logout("entered");

								out = (char*)calloc(200,1);

								sprintf(out,"entered with %d from %s",corrections[0].tgfreq,jstoc(corrections[0].chars));

								logout(out);

								correctionmade = true;
								makeCorrection(psi,pei,&(corrections[0]),uppercaseword,1);
							}
						}
					}
				} else if(getTwogramFreq(tgindex[priori],wordhash) == 0) {
					// if there is a previous word and this word is not part of a known 2-gram with it, look for a better match with a levenshtein distance of 1.

					if(determinedCorrect(firstchari,lastchari)) {
						goto correctprevious;
					}

					getCorrections(word,wordlen,&corrections,priori,1);

					sort(corrections.begin(),corrections.end(),&acMatch::sortNgramOnly);

					if(corrections.size() > 0 && corrections[0].tgfreq > twogramoverridethreshold) {
						sprintf(out,"replacing %d,%d with %s",firstchari,lastchari,jstoc(corrections[0].chars));

						logout(out);

						correctionmade = true;
						makeCorrection(firstchari,lastchari,&(corrections[0]),uppercaseword,1);
					} else {
						goto correctprevious;
					}
				}
			} // end if for isWord / priorlen != 0

donecorrections:

			delete[] word;
			delete[] wordo;

			if(priorlen != 0) {
				if(!correctionmade) {
					personalDict->add2gram(wordhash,priorhash);
				}

				delete[] priorword;
			}

			if(prior2len != 0) {
				delete[] prior2word;
			}
		}
	}

	return correctionmade;
}

bool AutoCorrect::determinedCorrect(int fi,int ei) {
	int c;

	for(c = fi;c < ei;c++) {
		if(buffer[c].status == 1) { return true; }
	}

	return false;
}

bool AutoCorrect::isSeparator(jchar ch) {
	if(ch == (jchar)' ' || ch == (jchar)'\n' || ch == (jchar)'\r') {
		return true;
	} else if(ch == (jchar)'"' || ch == (jchar)'(' || ch == (jchar)')') {
		return true;
	} else if(ch == (jchar)'.' || ch == (jchar)'?' || ch == (jchar)'!' || ch == (jchar)',') {
		return true;
	} else {
		return false;
	}
}

/*****
* isWord
*	Determines if the specified word exists in the currently loaded auto-correct dictionary.
*	This is the first step in determining whether or not correction should occur.
*
*	chars	-	the UTF-16 characters in the word
*	len		-	the number of characters in the array <i>chars</i>
*
*	Jonathon Simister (December, 2012)
******/
bool AutoCorrect::isWord(const jchar* chars,int len) {
	return isWord(chars,len,NULL,NULL,NULL) != 0;
}

/*****
* isWord
*	Determines if the specified word exists in the currently loaded auto-correct dictionary.
*	If it does exist, some basic information about the word is returned which can be found
*	with an O(1) lookup using the index value found with the O(log n) hash lookup.
*
*	chars	-	the UTF-16 characters in the word
*	len		-	the number of characters in the array <i>chars</i>
*	prob	-	a pointer to an integer to be set with the probability value
*	hp		-	a pointer to an integer to be set with the hash value for this word
*	hi		-	a pointer to an integer to be set with the index value for the hash
*					Note: if the word is a personal word then this is set to -1
*
*	Jonathon Simister (December, 2012)
******/
int AutoCorrect::isWord(const jchar* chars,int len,int* prob,int* hp,int* hi) {
	int hash;
	pair<const int*,const int*> bounds;
	int offset;
	personalword pword;
	
	hash = Hash::wordHash(chars,len);

	bounds = equal_range(hashes,hashes+nhashes,hash);

	if(hp != NULL) {
		*hp = hash;
	}

	if(*bounds.first == hash) {
		offset = bounds.first - hashes;

		if(prob != NULL) {
		*prob = probs[offset];
		}		

		if(hi != NULL) {
		*hi = bounds.first - hashes;
		}

		return 1;
	} else {
		if(personalDict->words.count(hash) == 1) {
			pword = personalDict->words[hash];

			if(prob != NULL) {
			*prob = (int)pword.prob;
			}

			if(hi != NULL) {
			*hi = -1;
			}

			return 2;
		} else {
			return 0;
		}
	}
}

/*****
* toLower
*	converts a UTF-16 character to its lower-case form (if one exists) using Java's implementation of unicode.
*	This function is really just a C++ wrapper for Java's Character.toLowerCase() method.
*
*	ch	-	the character to be made lower-case
*
*	Jonathon Simister (December, 2012)
******/
jchar AutoCorrect::toLower(jchar ch) {
	return env->CallStaticCharMethod(clsCharacter,midToLowerCase,ch);
}

vector<acMatch>* AutoCorrect::getPredictions(int lim) {
	vector<acMatch>* ret;
	int si, ei;
	int psi, pei;
	jchar* word;
	jchar* previous;
	int wordlen;
	int previouslen = 0;
	int c;
	int maxcost;
	int priori;
	bool inputUpperCase;
	int priorhash;

	ret = new vector<acMatch>();

	if(buffer.size() < 1) {
		return ret;
	} else if(isSeparator(buffer.back().ch)) {
		return ret;
	}

	getLastWord(0,buffer.size()-1,&si,&ei);

	wordlen = ei - si;

	if(wordlen < 2) {
		// we're no longer returning correction predictions for 1 letter words
		return ret;
	}

	word = new jchar[wordlen];

	if(wordlen <= 3) {
		maxcost = 1;
	} else if(wordlen <= 5){
		maxcost = 2;
	} else {
		maxcost = 3;
	}

	inputUpperCase = isUpper(buffer[si].ch);

	for(c = 0;c < wordlen;c++) {
		word[c] = toLower(buffer[c+si].ch);
	}

	if(si > 1) {
		getLastWord(0,si-1,&psi,&pei);

		previouslen = pei - psi;
	}

	if(previouslen > 0) {
		previous = new jchar[previouslen];

		for(c = 0;c < previouslen;c++) {
			previous[c] = toLower(buffer[c+psi].ch);
		}

		priori = getHashIndex(previous,previouslen,&priorhash);
	} else {
		priori = -1;
	}

	getCorrections(word,wordlen,ret,priori,maxcost);

	getMissedSpaces(word,wordlen,ret,priori);

	personalDict->getCorrections(word,wordlen,ret,priorhash,maxcost);

	addKeyboardDistance(word,wordlen,ret);

	sort(ret->begin(),ret->end(),&acMatch::sortAdjScore);

	delete[] word;

	if(previouslen > 0) {
		delete[] previous;
	}

	if(ret->size() > lim) {
		ret->erase(ret->begin()+lim,ret->end());
	}

	if(inputUpperCase) {
		for(c = 0;c < ret->size();c++) {
			(*ret)[c].chars[0] = toUpper((*ret)[c].chars[0]);
		}
	}

	logout("gotPredictions() results");
	printCorrections(ret);

	return ret;
}

jchar AutoCorrect::toUpper(jchar ch) {
	return env->CallStaticCharMethod(clsCharacter,midToUpperCase,ch);
}

bool AutoCorrect::isLower(jchar ch) {
	return env->CallStaticBooleanMethod(clsCharacter,midIsLowerCase,ch);
}

bool AutoCorrect::isUpper(jchar ch) {
	return env->CallStaticBooleanMethod(clsCharacter,midIsUpperCase,ch);
}

bool AutoCorrect::containsNoLowerCase(const jchar* chars,int len) {
	int c;
	jmethodID midIsLowerCase = env->GetStaticMethodID(clsCharacter,"isLowerCase","(C)Z");

	for(c = 0;c < len;c++) {
		if(env->CallStaticBooleanMethod(clsCharacter,midIsLowerCase,chars[c])) {
			return false;
		}
	}

	return true;
}

/*****
* filterMatches
*	filters a tentative list of matches produced by the Levenshtein distance algorithm by matches found in the graph
*	which are not actual words using a hash look-up. This function also adds probability and part
*	of speech data to the matches.
*
*	matches - a pointer to a vector containing the tentative matches to be filtered
*	filtered - a pointer to a vector which will be filled with only the real words from <i>matches</i>
*	returns - the number of real words found in <i>matches</i>
*
*	Jonathon Simister (December 14, 2012)
******/
int AutoCorrect::filterMatches(vector<acMatch>* matches,vector<acMatch>* filtered) {
	int c;
	int n = 0;
	int prob;
	int hash;
	int hashi;

	for(c = 0;c < matches->size();c++) {
		if(isWord((*matches)[c].chars.c_str(),(*matches)[c].chars.length(),&prob,&hash,&hashi)) {
			(*matches)[c].prob = (prob & 63);
			(*matches)[c].accronym = ((prob & 64) == 64);
			(*matches)[c].propernoun = ((prob & 128) == 128);
			(*matches)[c].firstwordhash = hash;
			(*matches)[c].tgfreq = 0;
			(*matches)[c].hashindex = hashi;

			filtered->push_back((*matches)[c]);
			n++;
		}
	}

	return n;
}

void AutoCorrect::getMissedSpaces(const jchar* s,int slen, vector<acMatch>* corrections,int previousi) {
	int fwl;
	int c, cc;
	vector<acMatch> firstwords;
	vector<acMatch> ret;
	acMatch first;
	int prob, hashi, hash;

	for(fwl = 1;fwl < slen;fwl++) {
		firstwords.clear();

		if(fwl <= 5) {
			if(isWord(s,fwl,&prob,&hash,&hashi)) {
				first = acMatch(basic_string<jchar>(s,fwl),0);
				first.prob = prob;
				first.hashindex = hashi;
				first.firstwordhash = hash;

				if(previousi != -1) {
				first.tgfreq = getTwogramFreq(tgindex[previousi],hash);
				}

				if(personalDict != NULL) {
				first.learnedtg = personalDict->get2gramfreq(getHashForIndex(previousi),hash);
				}

				firstwords.push_back(first);
			} else {
				continue;
			}
		} else {
			getCorrections(s,fwl,&firstwords,previousi,1);
		}

		for(c = 0;c < firstwords.size();c++) {
			ret.clear();

			if(slen-fwl <= 5) {
				getCorrections(s+fwl,slen-fwl,&ret,firstwords[c].hashindex,0);
			} else {
				getCorrections(s+fwl,slen-fwl,&ret,firstwords[c].hashindex,1);
			}

			for(cc = 0;cc < ret.size();cc++) {
				ret[cc].levscore = 1 + ret[cc].levscore + firstwords[c].levscore;
				ret[cc].tgfreq = (ret[cc].tgfreq + firstwords[c].tgfreq) / 2;
				ret[cc].propernoun = firstwords[c].propernoun;
				ret[cc].learnedtg = (ret[cc].learnedtg + firstwords[c].learnedtg) / 2;
				ret[cc].prob = (ret[cc].prob + firstwords[c].prob) / 2;
				ret[cc].chars = firstwords[c].chars + (jchar)' ' + ret[cc].chars;

				filterMatches(&ret,corrections);
			}
		}
	}
}

/*****
* getCorrections
*	Performs a preliminary search for corrections for a specific block of characters and stores the results in a vector.
*
*	s		-	the UTF-16 characters in the text to be corrected
*	slen	-	the number of characters in the array <i>s</i>
*	corrections - a pointer to a vector which is to be filled with possible corrections
*	previousi -	the ordinal number of the word preceeding the text which is to be corrected, used for 2-gram based correction
*	maxcost - the maximum allowable Levenshtein distance between the text and possible corrections
*
*	Jonathon Simister (December, 2012)
******/
void AutoCorrect::getCorrections(const jchar* s,int slen,vector<acMatch>* corrections,int previousi,int maxcost) {
	int c;
	jchar* slower;
	int* strow;
	int co;
	int nchildren;
	vector<acMatch> matches;
	int previoushash;

	slower = new jchar[slen];

	if(slower == NULL) {
		logout("slower == NULL");
		return;
	}

	if(corrections == NULL) {
		logout("corrections == NULL");
		return;
	}

	for(c = 0;c < slen;c++) {
		slower[c] = toLower(s[c]);
	}
	
	basic_string<jchar> search(slower,slen);

	delete[] slower;

	strow = new int[slen+1];

	for(c = 0;c < slen+1;c++) {
		strow[c] = c;
	}

	graph->setEnv(env);

	nchildren = graph->getNChildren(0);

	for(c = 0;c < nchildren;c++) {
		co = graph->getChildByIndex(0,c);

		graph->computeRow(co,search,strow,&matches,basic_string<jchar>(),1,maxcost);
	}

	delete[] strow;

	filterMatches(&matches,corrections);

	if(previousi != -1 && hasngrams) {
	addTwogramProb(corrections,twograms+tgindex[previousi].offset,tgindex[previousi].n);
	}

	previoushash = getHashForIndex(previousi);

	addLearnedTwogram(corrections,previoushash);
}

/*void AutoCorrect::javalog(const char* s) {
	jstring str;
	jmethodID midLog;
	jclass clsAutoCorrect;

	str = env->NewStringUTF(s);

	clsAutoCorrect = env->GetObjectClass(jobj);

	midLog = env->GetMethodID(clsAutoCorrect,"log","(Ljava/lang/String;)V");

	env->CallVoidMethod(jobj,midLog,str);
}*/

void AutoCorrect::logout(const char* s) {
#if debug
	jstring str;

	str = env->NewStringUTF(s);

	env->CallVoidMethod(SystemOut,midPrintLn,str);

	env->DeleteLocalRef(str);
#endif
}

// deprecated
void AutoCorrect::logout(const char* p,int d) {
#if debug
	char* s;

	s = (char*)calloc(200,1);

	sprintf(s,p,d);

	logout(s);

	free(s);
#endif
}

void AutoCorrect::logBufferStatus() {
	int c;
	char* log;
	int n;

	log = (char*)calloc(buffer.size()+1,1);

	for(c = 0;c < buffer.size();c++) {
		if(buffer[c].status == 1) {
			log[c] = 'C';
		} else {
			log[c] = '?';
		}
	}

	logout(log);

	free(log);
}

void AutoCorrect::logBuffer() {
	int c;
	char* log;
	int n;

	log = (char*)calloc(buffer.size()+1,1);

	for(c = 0;c < buffer.size();c++) {
		log[c] = (char)buffer[c].ch;
	}

	logout(log);

	free(log);
}

/*****
* makeCorrection
*	Makes a correction to the buffer and the attached program through a call to the correctionMade method in the AutoCorrectInterface
*
*	si				-	the start index of the characters to be replaced
*	ei				-	the end index of the characters to be replaced
*	replacement		-	the match which is to be used for correction
*	inputUpperCase	-	true if the input string, to be correction, started with an upper-case character
*	type			-	the type of correction to be made (0 lev, 1 n-gram replace of valid word, 3&4 case corrections)
*
*	Jonathon Simister (December, 2012)
******/
void AutoCorrect::makeCorrection(int si, int ei, acMatch *replacement,bool inputUpperCase,int type) {
	jstring rep;
	jstring prev;
	int replen, prevlen;
	jchar* repchars;
	jchar* prevchars;
	int c;
	jclass clsAutoCorrect;
	jfieldID fidCallback;
	jobject objCallback;
	jclass clsInterface;
	jmethodID midCorrectionMade;
	int retsi;
	int previoussize;

	if((type == 2 || type == 3) && !correctwordcase) {
		return;
	}

	if(type == 1 && !correctvalidwords) {
		return;
	}

	previoussize = buffer.size();

	replen = replacement->chars.length();
	prevlen = ei-si;
	repchars = new jchar[replen];
	prevchars = new jchar[prevlen];

	for(c = 0;c < replen;c++) {
		repchars[c] = replacement->chars[c];
	}

	if(replacement->accronym) {
		for(c = 0;c < replen;c++) {
			repchars[c] = toUpper(repchars[c]);
		}
	} else if(replacement->propernoun || inputUpperCase) {
		repchars[0] = toUpper(repchars[0]);
	}

	for(c = 0;c < prevlen;c++) {
		prevchars[c] = buffer[si+c].ch;
	}

	deque<acchar>::iterator it, it2, rit;

	it2 = bufferIterator(ei);

	it2 = bufferInsertChars(it2,repchars,replen);

	if(type != 0) {
		rit = it2;

		for(c = 0;c < replen;c++) {
			rit->status = 2; // mark the characters as coming from a non-Levenshtein correction
			rit++;
		}
	}

	it = bufferIterator(si);

	buffer.erase(it,it2);

	logBuffer();

	retsi = previoussize - (si+1);
	//retei = previoussize - (ei+1);

	rep = env->NewString(repchars,replen);
	prev = env->NewString(prevchars,prevlen);
	if(env->ExceptionCheck()) {
		logout("exception in autocorrect.cpp at line %d",__LINE__);
		env->ExceptionDescribe();
	}

	clsAutoCorrect = env->FindClass("com/wordlogic/lib/AutoCorrect");
	if(env->ExceptionCheck()) {
		logout("exception in autocorrect.cpp at line %d",__LINE__);
		env->ExceptionDescribe();
	}

	fidCallback = env->GetFieldID(clsAutoCorrect,"callback","Lcom/wordlogic/lib/AutoCorrectInterface;");
	if(env->ExceptionCheck()) {
		logout("exception in autocorrect.cpp at line %d",__LINE__);
		env->ExceptionDescribe();
	}

	objCallback = env->GetObjectField(jobj,fidCallback);
	if(env->ExceptionCheck()) {
		logout("exception in AutoCorrect::makeCorrection() at call to GetObjectField(jobj,fidCallback)");
		return;
	}

	if(objCallback == NULL) {
		return;
	}

	clsInterface = env->GetObjectClass(objCallback);
	if(env->ExceptionCheck()) {
		logout("exception in autocorrect.cpp at line %d",__LINE__);
		env->ExceptionDescribe();
		return;
	}

	midCorrectionMade = env->GetMethodID(clsInterface,"correctionMade","(ILjava/lang/String;Ljava/lang/String;)V");
	if(env->ExceptionCheck()) {
		logout("exception in autocorrect.cpp at line %d",__LINE__);
		env->ExceptionDescribe();
		return;
	}

	env->CallVoidMethod(objCallback,midCorrectionMade,retsi,prev,rep);
}

deque<acchar>::iterator AutoCorrect::bufferInsertChars(deque<acchar>::iterator it,const jchar* chars,int len) {
	int c;

	for(c = len-1;c >= 0;c--) {
		it = buffer.insert(it,acchar(chars[c]));
	}

	return it;
}

/*****
* correctionPending
*	Determine of the word currently being typed would occur if it were ended in its current state.
*
*	Jonathon Simister (December, 2012)
******/
bool AutoCorrect::correctionPending() {
	int ws,we;
	jchar* word;
	int wordlen;
	int c;
	bool ret;

	logout("correctionPending() called");
	logBuffer();
	logBufferStatus();

	if(buffer.empty()) { return false; }

	if(isSeparator(buffer.back().ch)) { return false; }

	getLastWord(0,buffer.size()-1,&ws,&we);

	wordlen = we-ws;

	if(wordlen < 2) { return false; }

	word = new jchar[wordlen];

	for(c = 0;c < wordlen;c++) {
		word[c] = toLower(buffer[c+ws].ch);
	}

	ret = isWord(word,wordlen);

	delete[] word;

	return !ret;
}

void AutoCorrect::reverseCorrection(int si,jstring autocorrect,jstring original) {
	const jchar* ochars;
	int olen;
	int alen;
	int nsi;
	int c;
	jclass clsNullPointerException;

	if(autocorrect == NULL) {
		clsNullPointerException = env->FindClass("java/lang/NullPointerException");

		env->ThrowNew(clsNullPointerException,"autocorrect string arugment for reverseCorrection must not be NULL");
		return;
	}

	if(original == NULL) {
		clsNullPointerException = env->FindClass("java/lang/NullPointerException");

		env->ThrowNew(clsNullPointerException,"original string arugment for reverseCorrection must not be NULL");
		return;
	}

	olen = env->GetStringLength(original);
	alen = env->GetStringLength(autocorrect);
	ochars = env->GetStringChars(original,NULL);

	nsi = buffer.size() - si - 1;

	///
	deque<acchar>::iterator it, it2, it3, ait;

	it2 = bufferIterator(nsi+alen); // iterator to one past the last character of the string inserted by auto-correct

	it3 = bufferInsertChars(it2,ochars,olen); // inserts the original characters back and gets an iterator to the first of them
	
	it = it3;

	for(c = 0;c < olen;c++) {
		it->status = 1;
		it++;
	}
	
	it = bufferIterator(nsi); // gets the iterator to the first of the characters inserted by auto-correct
	ait = it;

	bool nonlev = false;

	for(c = 0;c < alen;c++) {
		ait++;
		if((ait->status & 2) == 2) {
			nonlev = true;
			break;
		}
	}

	if(!nonlev) {
		personalDict->addword(ochars,olen,1);
	}

	buffer.erase(it,it3); // erases the characters inserted by auto-correct

	env->ReleaseStringChars(original,ochars);

	logBuffer();
	logBufferStatus();
}

/*****
* getLastWord
*	Find the last full word in the character buffer.
*
*	starti	-	the start index for the buffer (typically 0)
*	endi	-	the end index for the buffer (either len-1 or some lower number (before the last word) to search for (for example) the second last word.
*	fs		-	a pointer to an integer which will be set to the index of the first character of the word found
*	fe		-	a pointer to an integer which will be set to the index of the first character *after* the word found (lenfound = fe-fs)
*
*	Note: Unlike some other functions which accept pointers in this code, it is NOT acceptable to pass a NULL-pointer to either fs or fe. After all,
*	what is the point of searching for a string if you're not going to use the result?
*
*	Jonathon Simister (December, 2012)
******/
void AutoCorrect::getLastWord(int starti, int endi, int *fs, int *fe) {
	int c;

	c = endi;

	while(isSeparator(buffer[c].ch)) {
		if(c == starti) {
			*fe = starti;
			*fs = starti;
			return;
		}
		
		c--;
	}

	*fe = c+1;

	while(true) {
		if(c == starti) {
			*fs = c;
			break;
		} else if(isSeparator(buffer[c].ch)) {
			*fs = c+1;
			break;
		} else {
			c--;
		}
	}
}

/*****
* getHashIndex
*	Performs a binary search for the hash of a string of characters and then returns the index found or -1 if nothing was found.
*	This index can then be used, for example, to lookup the probability of a word.
*
*	chars	-	the characters to search for (jchar*)
*	len		-	the number of characters pointed to by <i>chars</i>
*
*	Jonathon Simister (December, 2012)
******/
int AutoCorrect::getHashIndex(const jchar* chars,int len) {
	return getHashIndex(chars,len,NULL);
}

void AutoCorrect::getSecondCorrection(int si,const jchar* ochars,int olen,const jchar* achars,int alen,jchar** cchars,int* clen) {
	vector<acMatch> corrections;
	jchar* retchars;
	int maxcost;

	if(olen <= 3) {
		maxcost = 1;
	} else if(olen <= 5){
		maxcost = 2;
	} else {
		maxcost = 3;
	}

	getCorrections(ochars,olen,&corrections,-1,maxcost);

	addKeyboardDistance(ochars,olen,&corrections);

	sort(corrections.begin(),corrections.end(),&acMatch::sortAdjScore);

	if(corrections.size() >= 2) {
		retchars = new jchar[corrections[1].chars.length()];

		memcpy(retchars,corrections[1].chars.c_str(),sizeof(jchar)*corrections[1].chars.length());

		*clen = corrections[1].chars.length();
		*cchars = retchars;
	} else {
		*cchars = NULL;
		*clen = 0;
	}
}

/*****
* getHashIndex
*	Performs a binary search for the hash of a string of characters and then returns the index found or -1 if nothing was found.
*	This index can then be used, for example, to lookup the probability of a word.
*
*	chars	-	the characters to search for (jchar*)
*	len		-	the number of characters pointed to by <i>chars</i>
*	hp		-	a pointer to an integer which will be set to hash that was computed if specified
*
*	Jonathon Simister (December, 2012)
******/
int AutoCorrect::getHashIndex(const jchar* chars,int len,int* hp) {
	int hash;
	pair<const int*,const int*> bounds;
	int offset;
	
	hash = Hash::wordHash(chars,len);

	bounds = equal_range(hashes,hashes+nhashes,hash);

	if(hp != NULL) {
		*hp = hash;
	}

	if(*bounds.first == hash) {
		offset = bounds.first - hashes;

		return offset;
	} else {
		return -1;
	}
}

int AutoCorrect::getHashForIndex(int hi) {
	return hashes[hi];
}

void AutoCorrect::addTwogramProb(vector<acMatch> *matches, twogram *twograms, int ntgs) {
	unsigned int c;
	int cc;

	if(twograms == NULL || !hasngrams) { return; }

	for(c = 0;c < matches->size();c++) {
		for(cc = 0;cc < ntgs;cc++) {
			if(twograms[cc].h2 == (*matches)[c].firstwordhash) {
				(*matches)[c].tgfreq = twograms[cc].freq;
			}
		}
	}
}

void AutoCorrect::addLearnedTwogram(vector<acMatch>* matches,int previoushash) {
	unsigned int c;

	for(c = 0;c < matches->size();c++) {
		(*matches)[c].learnedtg = personalDict->get2gramfreq(previoushash,(*matches)[c].firstwordhash);
	}
}

int AutoCorrect::getTwogramFreq(tgHeader twogram,int hash) {
	int c;

	if(!hasngrams) { return 0; }

	for(c = twogram.offset;c < twogram.offset+twogram.n;c++) {
		if(twograms[c].h2 == hash) {
			return twograms[c].freq;
		}
	}

	return 0;
}

/*****
* addForwardTwogramProb
*	Adds 2-gram probability information to a list of matches based on the next word.
*	That is to say, for all matches M in the vector <i>matches</i> the twogram score of M
*	will be set to the frequency value for the 2-gram for the words "$M $S" where S is specified
*	by its hash.
*
*	matches	-	a pointer to a vector of matches (acMatch object)
*	hash	-	hash of the second word
*
*	Jonathon Simister (December, 2012)
******/
void AutoCorrect::addForwardTwogramProb(vector<acMatch>* matches,int hash) {
	unsigned int c;
	int cc;
	int matchhash;

	if(!hasngrams) { return; }

	for(c = 0;c < matches->size();c++) {
		matchhash = Hash::wordHash((*matches)[c].chars.c_str(),(*matches)[c].chars.length());

		for(cc = 0;cc < ntwograms;cc++) {
			if(twograms[cc].h2 == hash && twograms[cc].h1 == matchhash) {
				(*matches)[c].tgfreq = twograms[cc].freq;
			}
		}
	}
}

void AutoCorrect::printCorrections(vector<acMatch>* corrections) {
#if debug
	char* out;
	unsigned int c;
	acMatch* cmatch;
	
	out = (char*)calloc(200,1);

	sprintf(out,"corrections.size() = %d",corrections->size());

	logout(out);

	for(c = 0;c < min((int)corrections->size(),10);c++) {
		out = (char*)calloc(200,1);

		cmatch = &(*corrections)[c];

		sprintf(out,"%s (prob = %d,levscore = %d,twogram = %d,keydist = %f,adj = %f,pn = %d,acr = %d)",jstoc((*corrections)[c].chars),(*corrections)[c].prob,
			(*corrections)[c].levscore,(*corrections)[c].tgfreq,(*corrections)[c].keydist,(*corrections)[c].adjscore,
			(int)((*corrections)[c].propernoun),(int)((*corrections)[c].accronym));

		logout(out);

		sprintf(out,"ltg = %d",(*corrections)[c].learnedtg);

		logout(out);

		free(out);
	}

#endif
}

void AutoCorrect::loadKeyPositions(jchar* kl,float* dm,float* sd,int n) {
	if(keylist != NULL) {
		delete[] keylist;
	}
	if(distmatrix != NULL) {
		delete[] distmatrix;
	}
	if(spacedist != NULL) {
		delete[] spacedist;
	}
	
	keylist = kl;
	distmatrix = dm;
	spacedist = sd;
	nkeys = n;
}

/*****
* keyLev
*	A modified version of Levenshtein distance which gives a lower (1/3) weight to edits where a character has
*	been substitued for another character which it is close (less than 40% of the maximum key separation) to
*	on the keyboard.
*
*	Jonathon Simister (December, 2012)
******/
float AutoCorrect::keyLev(const jchar* s1,int len1,const jchar* s2,int len2) {
	float deleteCost;
	float insertCost;
	float replaceCost;
	int i, j;
	int c;
	jchar ch1, ch2;
    float** matrix;
	float ret;
	float dist;

	matrix = new float*[len1+1];

	for(c = 0;c < len1+1;c++) {
		matrix[c] = new float[len2+1];
		matrix[c][0] = (float)c;
	}

	for(c = 0;c < len2+1;c++) {
		matrix[0][c] = (float)c;
	}

    for (i = 1; i <= len1; i++) {
        ch1 = s1[i-1];
        for (j = 1; j <= len2; j++) {
            ch2 = s2[j-1];
            if(ch1 == ch2) {
            	matrix[i][j] = matrix[i-1][j-1];
            } else {
            	dist = keyDist(ch1,ch2);

            	if(dist == 0) {
            		matrix[i][j] = matrix[i-1][j-1];
            	} else {
            		deleteCost = matrix[i-1][j] + 1;
            		insertCost = matrix[i][j-1] + 1;

            		if(dist < 0.2) {
            			replaceCost = matrix[i-1][j-1] + (float)0.3333333;
            		} else if(dist < 0.4)  {
            			replaceCost = matrix[i-1][j-1] + (float)0.6666666;
            		} else {
            			replaceCost = matrix[i-1][j-1] + 1;
            		}

            		matrix[i][j] = min(deleteCost,min(insertCost,replaceCost));
            	}
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

/*****
* keyDist
*	Determines the distance between two keys, relative to other keys on the keyboard, within the range [0,1].
*
*	Jonathon Simister (December, 2012)
******/
float AutoCorrect::keyDist(jchar ch1, jchar ch2) {
	pair<jchar*,jchar*> bounds;
	int i1, i2;
	int c, i;
	int rowlen;

	if(deaccent != NULL) {
		ch1 = deaccent->removeAccent(ch1);
		ch2 = deaccent->removeAccent(ch2);
	}

	// if one of the characters is a space then lookup the other character in the keylist and then return the value at its index in the spacedist array
	if(ch1 == (jchar)' ') {
		if(spacedist != NULL) {
			bounds = equal_range(keylist,keylist+nkeys,ch2);

			if(*bounds.first == ch2) {
				i2 = bounds.first - keylist;

				return spacedist[i2];
			} else {
				return 1;
			}
		} else {
			return 1;
		}
	}

	if(ch2 == (jchar)' ') {
		if(spacedist != NULL) {
			bounds = equal_range(keylist,keylist+nkeys,ch1);

			if(*bounds.first == ch1) {
				i1 = bounds.first - keylist;

				return spacedist[i1];
			} else {
				return 1;
			}
		} else {
			return 1;
		}
	}

	bounds = equal_range(keylist,keylist+nkeys,ch1);

	if(*bounds.first == ch1) {
		i1 = bounds.first - keylist;
	} else {
		return 1;
	}

	bounds = equal_range(keylist,keylist+nkeys,ch2);

	if(*bounds.first == ch2) {
		i2 = bounds.first - keylist;
	} else {
		return 1;
	}

	if(i1 == i2) { return 0; }
	if(i1 > i2) { swap(i1,i2); }

	// the following code determines the 1-dimensional location on the packed distance matrix for a key-pair
	// I believe there is a more efficient, non-trivial way to find this, but it's not a major priority right now.
	rowlen = nkeys-1;
	i = 0;

	for(c = 0;c < i1;c++) {
		i += rowlen--;
	}

	i += ((i2 - i1) - 1);

	return distmatrix[i];
}

void AutoCorrect::addKeyboardDistance(const jchar *s, int slen, std::vector<acMatch> *matches) {
	int c;

	if(keylist == NULL || distmatrix == NULL) { return; }

	for(c = 0;c < matches->size();c++) {
		(*matches)[c].keydist = keyLev(s,slen,(*matches)[c].chars.c_str(),(*matches)[c].chars.length());

		(*matches)[c].updateAdjustedScore();
	}
}

AutoCorrect::~AutoCorrect() {
	delete personalDict;
}

void AutoCorrect::save() {
	personalDict->save();
}

deque<acchar>::iterator AutoCorrect::bufferIterator(int pos) {
	deque<acchar>::iterator it = buffer.begin();

	while(pos > 0) {
		it++;
		pos--;
	}

	return it;
}
