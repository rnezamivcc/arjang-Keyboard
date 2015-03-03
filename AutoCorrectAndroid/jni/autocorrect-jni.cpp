#include <jni.h>
#include <android/log.h>

#include <algorithm>
#include <string>
#include <cctype>

#include "autocorrect/wordhash.h"
#include "autocorrect/vertex.h"
#include "autocorrect/acMatch.h"

using namespace std;

#define LOG_TAG "autocorrect-jni"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)

extern "C" {
	JNIEXPORT jboolean Java_com_iknowu_autocorrect_AutoCorrect_isWord(JNIEnv* env,jclass cls,jstring str,jobject bufd) {
		const char* s;
		char* swork;
		unsigned int hash;
		unsigned int* hashes;
		unsigned char* buf;
		unsigned int* bufi;
		unsigned int nhashes;
		pair<unsigned int*,unsigned int*> bounds;
		char out[20];
		char* p;

		s = env->GetStringUTFChars(str,NULL);

		if(env->ExceptionCheck()) { return JNI_FALSE; }

		swork = new char[strlen(s)+1];
		strcpy(swork,s);

		p = swork;

		while(*p != 0) {
			*p = (char)tolower(*p);
			p++;
		}

		hash = wordHash(swork);

		delete[] swork;

		env->ReleaseStringUTFChars(str,s);
		if(env->ExceptionCheck()) { return JNI_FALSE; }

		buf = (unsigned char*)env->GetDirectBufferAddress(bufd);
		if(env->ExceptionCheck()) { return JNI_FALSE; }
		bufi = (unsigned int*)buf;

		nhashes = bufi[6];

		p = (char*)bufi + bufi[2];

		hashes = (unsigned int*)p;

		bounds = equal_range(hashes,hashes+nhashes,hash);

		if(*bounds.first == hash) {
			return JNI_TRUE;
		} else {
			return JNI_FALSE;
		}
	}

	void recursiveVPrint(char* buf,unsigned int offset,string s) {
		char ch = buf[offset];
		int nchildren = buf[offset+1];
		int c;
		unsigned int* op = (unsigned int*)(buf+offset+2);
		string ns;

		if(ch != 0) { ns = s + ch; }
		else { ns = s; }

		LOGI(ns.c_str());

		for(c = 0;c < nchildren;c++) {
			recursiveVPrint(buf,op[c],ns);
		}
	}

	void recursiveVPrintB(char* buf,unsigned int offset,string s,unsigned int* hashes,int nhashes,unsigned char* probs) {
		char ch = buf[offset];
		int nchildren = getNChildren(buf,offset);
		int c;
		string ns;
		unsigned int co;
		unsigned int hash;
		int p;

		if(ch != 0) { ns = s + ch; }
		else { ns = s; }

		hash = wordHash(ns.c_str());

		p = getProb(hash,hashes,nhashes,probs);

		if(p > 0 && ((p & 128) == 128)) {
		LOGI("%s p = %d",ns.c_str(),p);
		}

		for(c = 0;c < nchildren;c++) {
			co = getChild(buf,offset,c);
			recursiveVPrintB(buf,co,ns,hashes,nhashes,probs);
		}
	}

	void VPrint(char* buf,unsigned int offset) {
		int nchildren;
		int c;
		unsigned int* ip;

		nchildren = buf[offset+1];

		LOGI("vertex at offset %d",offset);
		LOGI("character = %c",buf[offset]);
		LOGI("has %d children",nchildren);

		ip = (unsigned int*)(buf + offset + 2);

		for(c = 0;c < nchildren;c++) {
			LOGI("offset %d",ip[c]);
		}
	}

	JNIEXPORT void Java_com_iknowu_autocorrect_AutoCorrect_walkGraph(JNIEnv* env,jclass cls,jobject dbuf) {
		unsigned int* bufi;
		string s;
		char* graphbuf;
		unsigned int* hashes;
		unsigned int nhashes;
		unsigned char* probs;
		char* p;

		bufi = (unsigned int*)env->GetDirectBufferAddress(dbuf);
		if(env->ExceptionCheck() || bufi == NULL) { return; }

		graphbuf = ((char*)bufi) + bufi[3];

		nhashes = bufi[6];
		p = (char*)bufi + bufi[2];
		hashes = (unsigned int*)p;

		p = (char*)bufi + bufi[4];
		probs = (unsigned char*)p;

		recursiveVPrintB(graphbuf,0,s,hashes,nhashes,probs);
	}

	JNIEXPORT void Java_com_iknowu_autocorrect_AutoCorrect_twoGramInfo(JNIEnv* env,jclass cls,jstring str,jobject dbuf) {
		const char* s;
		unsigned int* bufi;
		char* p;

		unsigned int* twograms;
		unsigned int ntwograms;
		unsigned short* twogramis;
		unsigned int* hashes;
		unsigned int nhashes;
		unsigned int hash;

		int hindex;

		int tgindex;
		int tgcount;

		s = env->GetStringUTFChars(str,NULL);
		if(env->ExceptionCheck()) { return; }

		bufi = (unsigned int*)env->GetDirectBufferAddress(dbuf);
		if(env->ExceptionCheck() || bufi == NULL) { return; }

		// read offset data from the dictionary file so we can get the graph, hash table, and probability pointers
		nhashes = bufi[6];
		p = (char*)bufi + bufi[2];
		hashes = (unsigned int*)p;

		/*p = (char*)bufi + bufi[3];
		graph = p;

		p = (char*)bufi + bufi[4];
		prob = (unsigned char*)p;*/

		ntwograms = bufi[9];
		p = (char*)bufi + bufi[7];
		twograms = (unsigned int*)p;

		p = (char*)bufi + bufi[8];
		twogramis = (unsigned short*)p;

		hash = wordHash(s);

		hindex = getIndex(hash,hashes,nhashes);

		LOGI("hash index = %d",hindex);

		tgindex = twogramis[hindex*2];
		tgcount = twogramis[1+(getIndex(hash,hashes,nhashes)*2)];

		LOGI("for the word %s there are %d twograms starting with index %d",s,tgcount,tgindex);

		env->ReleaseStringUTFChars(str,s);
		if(env->ExceptionCheck()) { return; }
	}

	JNIEXPORT void Java_com_iknowu_autocorrect_AutoCorrect_correct(JNIEnv* env,jclass cls,jstring pre,jstring str,jint l,jobject rs,jobject dbuf,jint msort) {
		const char* s;
		const char* stemp;
		char* spre;
		char* slower;
		jclass clsRS;
		jmethodID midAdd;
		int* strow;
		int slen;
		int c, cb;
		int nchildren;
		unsigned int co;
		char* p;
		int tgindex;

		char* graph;
		unsigned int* hashes;
		unsigned int nhashes;
		unsigned char* prob;

		unsigned int* twograms;
		unsigned int ntwograms;
		unsigned short* twogramis;

		vector<acMatch> matches;
		vector<acMatch> realwords;
		int d;
		pair<unsigned int*,unsigned int*> bounds;
		unsigned int hash;
		jstring smatch;
		char sa[40];
		char sb[40];
		unsigned int ha, hb;
		char ws[81];
		int hoff;
		unsigned int* bufi;

		bool upcase;

		const char* surrounds = "\"\"''<>()[]{}";
		int wrap = -1;

		string cppstr;

		s = env->GetStringUTFChars(str,NULL);
		if(env->ExceptionCheck()) { return; }

		bufi = (unsigned int*)env->GetDirectBufferAddress(dbuf);
		if(env->ExceptionCheck() || bufi == NULL) { return; }

		// read offset data from the dictionary file so we can get the graph, hash table, and probability pointers
		nhashes = bufi[6];
		p = (char*)bufi + bufi[2];
		hashes = (unsigned int*)p;

		p = (char*)bufi + bufi[3];
		graph = p;

		p = (char*)bufi + bufi[4];
		prob = (unsigned char*)p;

		ntwograms = bufi[9];
		p = (char*)bufi + bufi[7];
		twograms = (unsigned int*)p;

		p = (char*)bufi + bufi[8];
		twogramis = (unsigned short*)p;

		stemp = NULL;

		if(pre != NULL) {
			stemp = env->GetStringUTFChars(pre,NULL);
			if(env->ExceptionCheck()) { stemp = NULL; }
		} else {
			stemp = NULL;
		}

		unsigned int prehash;
		unsigned int tgindw;

		unsigned int tgcount;

		if(stemp != NULL) {
			spre = (char*)calloc(strlen(stemp)+1,1);

			for(c = 0;c < strlen(stemp);c++) {
				spre[c] = (char)tolower(stemp[c]);
			}

			env->ReleaseStringUTFChars(pre,stemp);
			if(env->ExceptionCheck()) { return; }

			prehash = wordHash(spre);
			free(spre);

			tgindex = twogramis[(getIndex(prehash,hashes,nhashes)*2)];
			tgcount = twogramis[1+(getIndex(prehash,hashes,nhashes)*2)];
		} else {
			tgindex = -1;
			tgcount = 0;
		}


		for(c = 0;c < strlen(surrounds)/2;c++) {
			if(s[0] == surrounds[2*c] && s[strlen(s)-1] == surrounds[(2*c)+1]) {
				wrap = c;
				break;
			}
		}

		if(wrap == -1) {
			slower = new char[strlen(s)+1];

			if(slower == NULL) { return; }

			strcpy(slower,s);
		} else {
			slower = new char[strlen(s)];

			if(slower == NULL) { return; }

			strcpy(slower,s+1);
			slower[strlen(s)-2] = 0;
		}

		slen = strlen(slower);

		p = slower;

		while(*p != 0) {
			*p = (char)tolower(*p);
			p++;
		}

		strow = new int[slen+1];

		if(strow == NULL) { return; }

		for(c = 0;c < slen+1;c++) {
			strow[c] = c;
		}

		nchildren = getNChildren(graph,0);

		for(c = 0;c < nchildren;c++) {
			co = getChild(graph,0,c);

			computeRow(graph,co,slower,slen,strow,&matches,string(),1,2);
		}

		delete[] strow;

		upcase = (slower[0] != s[0]);

		for(c = 0;c < matches.size();c++) {
			hash = wordHash(matches[c].match.c_str());

			bounds = equal_range(hashes,hashes+nhashes,hash);

			if(bounds.first != hashes+nhashes) {
				if(*bounds.first == hash) {
					if(matches[c].match[0] == *s) {
						matches[c].fletter = 1;
					} else {
						matches[c].fletter = 0;
					}

					// this is causing errors
					/*d = matches[c].match.length() - slen;

					if(d > 0) {
						matches[c].score -= d;
					}*/

					matches[c].ngram = 0;

					if(tgindex != -1) {
						for(cb = 0;cb < tgcount;cb++) {
							if(twograms[tgindex+cb] == hash) {
								matches[c].ngram = (float)(tgcount - cb) / (float)tgcount;
								break;
							}
						}
					}

					hoff = bounds.first - hashes;

					matches[c].prob = prob[hoff] & 127;

					if(matches[c].prob == 0) { continue; }

					if(((prob[hoff] & 128) == 128) || upcase) {
						matches[c].match[0] = (char)toupper(matches[c].match[0]);
					}

					if(wrap != -1) {	// put the quotes or brackets back in if they were removed
						matches[c].match = surrounds[2*wrap] + matches[c].match + surrounds[(2*wrap)+1];
					}

					realwords.push_back(matches[c]);
				}
			}
		}

		int pra, prb;
		acMatch spm;

		// find missing space auto-correct code
		if(slen < 80) {
			for(c = 1;c < slen;c++) {
				memset(sa,0,40);
				memset(sb,0,40);

				for(cb = 0;cb < c;cb++) {
					sa[cb] = slower[cb];
				}

				ha = wordHash(sa);
				pra = getProb(ha,hashes,nhashes,prob);

				if(pra < 1) { continue; }

				for(cb = c;cb < strlen(s);cb++) {
					sb[cb-c] = slower[cb];
				}

				hb = wordHash(sb);
				prb = getProb(hb,hashes,nhashes,prob);

				if(prb > 0 && pra > 0) {
					if(upcase || ((pra & 128) == 128)) {
						sa[0] = (char)toupper(sa[0]);
					}

					if((prb && 128) == 128) {
						sb[0] = (char)toupper(sb[0]);
					}

					memset(ws,0,81);

					strcat(ws,sa);
					strcat(ws," ");
					strcat(ws,sb);

					if(wrap == -1) {
						cppstr = string(ws);
					} else {	// put the quotes or brackets back in if they were removed
						cppstr = surrounds[2*wrap] + string(ws) + surrounds[(2*wrap)+1];
					}

					spm = acMatch(cppstr,1);

					spm.ngram = 0;

					if(tgindex != -1) {
						for(cb = 0;cb < tgcount;cb++) {
							if(twograms[tgindex+cb] == ha) {
								spm.ngram = (float)(tgcount - cb) / (float)tgcount;
								break;
							}
						}
					}

					spm.prob = ((pra & 127) + (prb & 127)) / 2;

					realwords.push_back(spm);
				}
			}
		}

		env->ReleaseStringUTFChars(str,s);
		if(env->ExceptionCheck()) { return; }

		if(realwords.size() > 1) {
			switch(msort) {
			case 1:
				sort(realwords.begin(),realwords.end(),&acMatch::sortAdjusted);
				break;
			case 2:
				sort(realwords.begin(),realwords.end(),&acMatch::sortAscScoreDescPrio);
				break;
			default:
			case 3:
				sort(realwords.begin(),realwords.end(),&acMatch::sortAdjustedNgramsFirst);
				break;
			case 4:
				sort(realwords.begin(),realwords.end(),&acMatch::sortDescNgramAscScoreDescPrio);
				break;
			}
		}

		if(realwords.size() > l) {
			realwords.erase(realwords.begin()+l,realwords.end());
		}

		clsRS = env->GetObjectClass(rs);
		if(env->ExceptionCheck()) { return; }

		midAdd = env->GetMethodID(clsRS,"add","(Ljava/lang/String;IIIF)V");
		if(env->ExceptionCheck()) { return; }

		for(c = 0;c < realwords.size();c++) {
			smatch = env->NewStringUTF(realwords[c].match.c_str());

			if(env->ExceptionCheck()) { continue; }

			env->CallVoidMethod(rs,midAdd,smatch,realwords[c].score,realwords[c].prob,realwords[c].fletter,realwords[c].ngram);

			if(env->ExceptionCheck()) { continue; }
		}
	}
}
