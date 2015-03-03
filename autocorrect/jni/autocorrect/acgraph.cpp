
#include "acgraph.h"

acGraph::acGraph(const unsigned char* graph) {
	data = graph;
	deaccent = NULL;
}

int acGraph::getBaseVertex() {
	return 0;
}

jchar acGraph::getCharacter(int vo) {
	const unsigned char* v;
	const jchar* chp;

	v = data+vo;

	chp = (const jchar*)v;

	return *chp;
}

int acGraph::getNChildren(int vo) {
	const unsigned char* v;
	const short* s;

	v = data+vo;

	s = (const short*)v;

	return s[1];
}

int acGraph::getChildByIndex(int vo, int i) {
	const unsigned char* v;
	const int* ps;

	v = data+vo;

	ps = (const int*)v;

	return ps[i+1];
}

int acGraph::getChildByChar(int vo, jchar ch) {
	int nchildren;
	int c;
	int co;

	// TODO: optimize with binary search

	nchildren = getNChildren(vo);

	for(c = 0;c < nchildren;c++) {
		co = getChildByIndex(vo,c);

		if(getCharacter(co) == ch) {
			return co;
		}
	}

	return -1;
}

void acGraph::computeRow(int vo,std::basic_string<jchar> search,int *prow,vector<acMatch> *matches,std::basic_string<jchar> path,int depth,int maxcost) {
	basic_string<jchar> newpath;
	int* row;
	int c;
	int co;
	int columns;
	int insertCost, deleteCost, replaceCost;
	int rmin = 100000;
	int cost = 0;

	if(depth > 15) {
		return;
	}

	columns = search.length() + 1;

	newpath = path + getCharacter(vo);

	if(getCharacter(vo) == (jchar)'\'') {
		for(c = 0;c < getNChildren(vo);c++) {
			co = getChildByIndex(vo,c);

			computeRow(co,search,prow,matches,newpath,depth+1,maxcost);
		}

		return;
	}

	row = new int[columns];

	if(depth > columns) {
		cost = prow[columns-1] + 1;

		if(cost <= maxcost) {
			matches->push_back(acMatch(newpath,cost));

			row[columns-1] = cost + 1;

			for(c = 0;c < getNChildren(vo);c++) {
				co = getChildByIndex(vo,c);

				computeRow(co,search,row,matches,newpath,depth+1,maxcost);
			}
		}
	} else {
		row[0] = prow[0] + 1;

		if(deaccent == NULL) {
			for(c = 1;c < columns;c++) {
				insertCost = row[c-1] + 1;
				deleteCost = prow[c] + 1;

				if(search[c-1] != getCharacter(vo)) {
					replaceCost = prow[c-1] + 1;
				} else {
					replaceCost = prow[c-1];
				}

				row[c] = min(insertCost,min(deleteCost,replaceCost));

				rmin = min(rmin,row[c]);
			}
		} else {
			for(c = 1;c < columns;c++) {
				insertCost = row[c-1] + 1;
				deleteCost = prow[c] + 1;

				if(deaccent->areEqual(search[c-1],getCharacter(vo))) {
					replaceCost = prow[c-1];
				} else {
					replaceCost = prow[c-1] + 1;
				}

				row[c] = min(insertCost,min(deleteCost,replaceCost));

				rmin = min(rmin,row[c]);
			}
		}

		if(row[columns-1] <= maxcost) {
			matches->push_back(acMatch(newpath,row[columns-1]));
		}

		if(rmin <= maxcost) {
			for(c = 0;c < getNChildren(vo);c++) {
				co = getChildByIndex(vo,c);

				computeRow(co,search,row,matches,newpath,depth+1,maxcost);
			}
		}
	}

	delete[] row;
}

void acGraph::setEnv(JNIEnv* jnienv) {
	env = jnienv;
}

void acGraph::logout(const char* s) {
	jstring str;
	jclass clsSystem;
	jfieldID fidOut;
	jobject out;
	jclass clsPrintStream;
	jmethodID midPrintLn;

	str = env->NewStringUTF(s);

	clsSystem = env->FindClass("java/lang/System");

	fidOut = env->GetStaticFieldID(clsSystem,"out","Ljava/io/PrintStream;");

	out = env->GetStaticObjectField(clsSystem,fidOut);

	clsPrintStream = env->GetObjectClass(out);

	midPrintLn = env->GetMethodID(clsPrintStream,"println","(Ljava/lang/String;)V");

	env->CallVoidMethod(out,midPrintLn,str);
}

void acGraph::setAccentFilter(accentFilter* ac) {
	deaccent = ac;
}