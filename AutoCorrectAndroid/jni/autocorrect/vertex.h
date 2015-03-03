#ifndef VERTEX
#define VERTEX

#include <jni.h>
#include <android/log.h>

#include <vector>
#include <climits>

#include "acMatch.h"

using namespace std;

namespace std {
	int getNChildren(const char* buf,unsigned int offset);
	unsigned int getChild(const char* buf,unsigned int offset,int c);

	void computeRow(const char* buf,unsigned int offset,const char* search,int slen,int* prow,vector<acMatch>* matches,string path,int depth,int maxcost);
};

#endif
