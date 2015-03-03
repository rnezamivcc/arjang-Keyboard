#ifndef WORDHASH
#define WORDHASH

#include <algorithm>

using namespace std;

namespace std {

unsigned int wordHash(const char* str);
bool present(unsigned int hash,unsigned int* buf,unsigned int n);
int getProb(unsigned int hash,unsigned int* hashes,unsigned int nhashes,unsigned char* probs);
int getIndex(unsigned int hash,unsigned int* hashes,unsigned int nhashes);

};

#endif
