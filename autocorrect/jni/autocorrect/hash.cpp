
#include "hash.h"

int Hash::wordHash(const jchar* chars,int len) {
	int c;
	int hash;
	
	hash = 5381;

	for(c = 0;c < len;c++) {
		if(chars[c] == (jchar)'\'') { hash++; continue; }

		hash = (hash << 5) + hash;
		hash += chars[c];
	}

	return hash;
}