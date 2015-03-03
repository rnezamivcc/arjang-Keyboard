#ifndef ACGRAPH_H
#define ACGRAPH_H

#if defined (WIN32)
#include "../jni.h"
#else
#include <jni.h>
#endif

#include <string>
#include <vector>

#include "acmatch.h"
#include "accentfilter.h"

using namespace std;

class acGraph {
public:
	acGraph(const unsigned char* graph);

	int getBaseVertex();

	jchar getCharacter(int vo);
	int getNChildren(int vo);
	int getChildByIndex(int vo,int i);
	int getChildByChar(int vo,jchar ch);

	void computeRow(int vo,basic_string<jchar> search,int* prow,vector<acMatch>* matches,basic_string<jchar> path,int depth,int maxcost);

	void setEnv(JNIEnv* jnienv);
	void setAccentFilter(accentFilter* af);
private:
	const unsigned char* data;

	accentFilter* deaccent;

	// debug helpers
	void logout(const char* s);

	JNIEnv* env;
};

#endif