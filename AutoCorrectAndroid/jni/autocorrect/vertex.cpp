#include "vertex.h"

#define LOG_TAG "vertex.h"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)

int std::getNChildren(const char* buf,unsigned int offset) {
	return buf[offset+1];
}

unsigned int std::getChild(const char* buf,unsigned offset,int c) {
	unsigned int* cp;

	cp = (unsigned int*)(buf + offset + 2);

	return cp[c];
}

void std::computeRow(const char* buf,unsigned int offset,const char* search,int slen,int* prow,vector<acMatch>* matches,string path,int depth,int maxcost) {
	string newpath;
	int* row;
	int columns = slen+1;
	unsigned int c;
	int insertCost, deleteCost, replaceCost;
	int rmin = 10000;
	unsigned int co;
	int cost;

	if(depth > 15) {
		return;
	}

	newpath = path + (char)buf[offset];

	row = new int[columns];

	if(depth > slen+1) {

		cost = prow[columns-1] + 1;

		if(cost <= maxcost) {
			matches->push_back(acMatch(newpath,cost));

			row[columns-1] = cost + 1;

			for(c = 0;c < getNChildren(buf,offset);c++) {
				co = getChild(buf,offset,c);

				computeRow(buf,co,search,slen,row,matches,newpath,depth+1,maxcost);
			}
		}
	} else {
		row[0] = prow[0] + 1;

			for(c = 1;c < columns;c++) {
				insertCost = row[c-1] + 1;
				deleteCost = prow[c] + 1;

				if(search[c-1] != buf[offset]) {
					replaceCost = prow[c-1] + 1;
				} else {
					replaceCost = prow[c-1];
				}

				row[c] = min(insertCost,min(deleteCost,replaceCost));

				rmin = min(rmin,row[c]);
			}

			if(row[columns-1] <= maxcost) {
				matches->push_back(acMatch(newpath,row[columns-1]));
			}

			if(rmin <= maxcost) {
				for(c = 0;c < getNChildren(buf,offset);c++) {
					co = getChild(buf,offset,c);

					computeRow(buf,co,search,slen,row,matches,newpath,depth+1,maxcost);
				}
			}
	}

	delete[] row;
}
