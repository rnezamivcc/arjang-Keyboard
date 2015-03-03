//
// Author: July 2000	Peter Knaven
//
#ifdef WL_SDK

#ifndef WORDLIST_H
#define WORDLIST_H

#include <stdio.h>
#include "wltypes.h"

enum eWLState{
WL_NORMAL										=			0x1,
WL_USER_ADDED_NEW_WORD_OR_ASSIGNED_PREFERENCE	=			0x2,
WL_NOT_PERMANENT_DELETED_SESSION				=			0x4,
WL_PERMANENT_DELETED_SESSION					=			0x8,
WL_TOUCHED										=			0x80000000,
WL_ALL_ENTRIES									=			0xffffffff
};

typedef struct WordListEntry 
{
	MYWCHAR *word;
	int   preference;
	int   occurence;
	eWLState   state;	// normal new word, permanently deleted, not permanent deleted, user-added word 
} WordListEntry;

class CWordListSerializer
{
	typedef struct WordEntry
	{
		MYWCHAR word[MAX_WORD_LEN];
		short preference;
	} WordEntry;

public:
	CWordListSerializer(int bufferzize = 500, FILE *hFile = NULL);
	~CWordListSerializer();
	int addWord(MYWCHAR *newWord, int preference);
	void printList(FILE* hFile = NULL);
	int getNumberOfEntries();
	void setWordListSize(int nWords);

private:
	WordEntry *m_WordList;
	FILE *   m_HFile;
	int		m_filledEntries;
	int		m_WordListSize;
	int		m_nWords;
	
	friend  class CCompactStore;
};

class CWordList
{
public:
    CWordList();
    CWordList(int nNewWords, int nNewCharacters);
    ~CWordList();

	int  UIaddWord(MYWCHAR *newWord, int preference);
	int  UIchangeWord(int wordListIndex, MYWCHAR *changedWord, int changedPreference);
	BOOL UIdeleteWord(int wordListIndex, BOOL permanentFlag);
	int  addWordWithFileRef(MYWCHAR *newWord, int preference, int fileRef);
	BOOL deleteWord(MYWCHAR *newWord);

	int	 findWordListIndex(MYWCHAR *word);

	void increaseWordListSize(int nWords, int nNewCharacters);
	inline WordListEntry *getWordList() {return m_WordList;}
	void addText(MYWCHAR *newWord);
	void destroyWordList();
	inline int getNumberOfEntries() {return m_filledEntries;}
	void printList(FILE *fp);
	void takeOverPreferences(CWordList *srcWordList);
	void putNonZeroPrefWordsIntoWordList(CWordList *newWordsDesktopListObject);
	BOOL getListEntryAtIndex(int index, WordListEntry *wp);

private:

	int insertWord(MYWCHAR *newWord, int occurence, int preference, int state);

	int m_filledEntries;
	MYWCHAR *m_WordListCharP;
	WordListEntry *m_WordList;
	int 	m_WordListSize;
	GENPTR	m_EndOfWordList;
	int		m_nCharacters;
	int		m_nWords;

	friend  class CDictionaryTree;
	friend  class CCompactStore;
};

#endif  // WL_SDK
#endif
