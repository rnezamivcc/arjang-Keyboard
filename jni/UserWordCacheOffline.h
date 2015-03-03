
#ifndef _USERWORDCACHEOFFLINE_H_
#define _USERWORDCACHEOFFLINE_H_

#define NEW_WORD_PREF		10

#include "utility.h"

typedef struct OfflineWord
{
	MYWCHAR word[MAX_FOUND_PHRASES][MAX_WORD_LEN];
	bool	bEndPhrase[MAX_FOUND_PHRASES];
	OfflineWord()
	{
		memset(this, 0, sizeof(OfflineWord));
	}

} OfflineWord;

class UserWordCacheOffline
{
public:
	UserWordCacheOffline();
	~UserWordCacheOffline();

	bool ReadOfflineFile(const char *filePath, char *parsingSpec, BOOL unicodeFlag);

private:
	void TokenizePhrase(MYWCHAR* phrase);
	void LearningOffline(OfflineWord* stWord);

};

#endif