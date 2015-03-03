#include "wordhash.h"

unsigned int std::wordHash(const char* str) {
	unsigned int hash = 5381;
    unsigned int c;

	while (c = *str++) {
		if((char)c == '\'') { hash++; continue; }

        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	}

    return hash;
}

bool std::present(unsigned int hash,unsigned int* buf,unsigned int n) {
	pair<unsigned int*,unsigned int*> bounds;

	bounds = equal_range(buf,buf+n,hash);

	if(bounds.first != buf+n) {
		if(*bounds.first == hash) {
			return true;
		}
	}

	return false;
}

int std::getProb(unsigned int hash,const unsigned int* hashes,unsigned int nhashes,const unsigned char* probs) {
	pair<const unsigned int*,const unsigned int*> bounds;

	bounds = equal_range(hashes,hashes+nhashes,hash);

		if(bounds.first != hashes+nhashes) {
			if(*bounds.first == hash) {
				return probs[bounds.first-hashes];
			}
		}

	return -1;
}

int std::getIndex(unsigned int hash,unsigned int* hashes,unsigned int nhashes) {
	pair<unsigned int*,unsigned int*> bounds;

		bounds = equal_range(hashes,hashes+nhashes,hash);

			if(bounds.first != hashes+nhashes) {
				if(*bounds.first == hash) {
					return bounds.first-hashes;
				}
			}

		return -1;
}
