//
// Author: July 2000	Peter Knaven
//		 : July 2012    Reza Nezami: Total restructuring!!!
//
#ifdef WL_SDK

#include "StdAfx.h"

#include "wordlist.h"
#include "compactstore.h"
#include <stdlib.h>
#include <string.h>
#include "utility.h"


void CWordListSerializer::setWordListSize(int nWords)
{
	if(nWords < 0)
		nWords = 500;
	m_WordListSize = nWords * sizeof(WordEntry);
	m_WordList = (WordEntry *) calloc(1, m_WordListSize);
	m_filledEntries = 0;
	m_nWords = nWords;
}

CWordListSerializer::CWordListSerializer(int buffersize, FILE* file): m_WordList(NULL), m_HFile(file), m_WordListSize(0), m_nWords(0), m_filledEntries(0)
{
	setWordListSize(buffersize);
}

CWordListSerializer::~CWordListSerializer()
{
	free((char*)m_WordList);
}

void CWordListSerializer::printList(FILE* hFile)
{
	if (m_WordList == NULL)
		return;

	if(hFile != NULL)
		m_HFile = hFile;

	if(m_HFile == NULL)
	{
		printf("No valid file to write to! Exit!\n");
		return;
	}

	WordEntry *wp = m_WordList;

	for (int i = 0; i < m_filledEntries; i++, wp++)
	{
		//fwprintf(wBuf, 200, L"%s %d \x0d", wp->word, wp->preference);

		fwprintf(m_HFile, L"%s\x20%d\x000d\x000a", wp->word, wp->preference);
	}
	m_filledEntries = 0;
}

int CWordListSerializer::addWord(WCHAR *newWord, int preference)
{
	if (m_filledEntries >= ( m_nWords - 2 ))
	{
		printList();
	}
	assert((mywcslen(newWord) < MAX_WORD_LEN));
	mywcsncpy(m_WordList[m_filledEntries].word, newWord, MAX_WORD_LEN);
	m_WordList[m_filledEntries++].preference = preference;
	return m_filledEntries;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
CWordList::CWordList()
{
	m_filledEntries = 0;
	m_WordListCharP = NULL;
	m_WordList = 0;
	m_WordListSize = 0;
	m_EndOfWordList = 0;
	m_nCharacters = 0;
	m_nWords = 0;

	increaseWordListSize(100, 100 * MAX_WORD_LEN);
}

CWordList::CWordList(int nWords, int nNewCharacters)
{
	m_filledEntries = 0;
	m_WordListCharP = NULL;
	m_WordList = 0;
	m_WordListSize = 0;;
	m_EndOfWordList = 0;
	m_nCharacters = 0;
	m_nWords = 0;

	increaseWordListSize(nWords, nNewCharacters);
}

void CWordList::increaseWordListSize(int nWords, int nNewCharacters)
{
	if (nWords < m_nWords && nNewCharacters < m_nCharacters)
		return;

	WordListEntry *oldWordList = m_WordList;
	int oldFilledEntries = m_filledEntries;

	m_WordListSize = nNewCharacters * sizeof(MYWCHAR) // all the characters
					+ nWords * sizeof(MYWCHAR) // all the NULL characters
					+ nWords * sizeof(WordListEntry) // the list itself
					+ sizeof(WordListEntry); // last entry null terminated

	m_WordList = (WordListEntry *) calloc(1, m_WordListSize);
	m_EndOfWordList = (GENPTR) m_WordList + m_WordListSize;
	m_filledEntries = 0;
	m_nCharacters = nNewCharacters;
	m_nWords = nWords;
	m_WordListCharP = (WCHAR *) ((GENPTR) m_WordList + (nWords + 1)* sizeof(WordListEntry));

	if (oldWordList)
	{
		// copy all the old entries to the newly allocated space
		for (int i = 0; i < oldFilledEntries; i++)
		{
			memcpy(&m_WordList[i], &oldWordList[i], sizeof(WordListEntry));
			addText(oldWordList[i].word);
			m_filledEntries++;
		}
		free(oldWordList);
	}
}

CWordList::~CWordList()
{
	destroyWordList();
}

void CWordList::destroyWordList()
{
	free( m_WordList);
	m_filledEntries = 0;
	m_WordListCharP = NULL;
	m_WordList = 0;
	m_WordListSize = 0;
	m_EndOfWordList = 0;
	m_nCharacters = 0;
	m_nWords = 0;
}

int CWordList::addWordWithFileRef(MYWCHAR *newWord, int preference, int fileRef)
{
/*	unsigned short fileRefs[NFILEREFS];
	memset(fileRefs, 0, sizeof(fileRefs));
	fileRefs[0] = fileRef;
*/
	// check if the word already exists in the new word list
	if (findWordListIndex(newWord) >= 0)
		return -1;

	return insertWord(newWord, 1, preference, /*fileRefs,*/ WL_NORMAL);
}

int CWordList::UIaddWord(MYWCHAR *newWord, int preference)
{
	// check if the word already exists in the new word list
	if (findWordListIndex(newWord) >= 0)
		return -1;

	return insertWord(newWord, 1, preference, WL_TOUCHED | WL_USER_ADDED_NEW_WORD_OR_ASSIGNED_PREFERENCE);
}

int CWordList::UIchangeWord(int wordListIndex, MYWCHAR *changedWord,int changedPreference)
{
	if (wordListIndex < 0 || wordListIndex >= m_filledEntries)
		return -1;

	// the occurence doesn't matter anymore, even on a changed word we need an assigned preference
	// otherwise we have to keep it the same
	WordListEntry *wlp = &m_WordList[wordListIndex];
	if (mywcscmp(wlp->word, changedWord) == 0)
	{
		if (changedPreference != -1 && wlp->preference != changedPreference)
		{
			wlp->preference = changedPreference;
			wlp->state = (eWLState)(WL_TOUCHED | WL_USER_ADDED_NEW_WORD_OR_ASSIGNED_PREFERENCE);
		}
		return wordListIndex;
	}

	UIdeleteWord(wordListIndex, FALSE);
	return UIaddWord(changedWord, changedPreference);
}

BOOL CWordList::UIdeleteWord(int wordListIndex, BOOL permanentFlag) 
{
	if (wordListIndex < 0 || wordListIndex >= m_filledEntries)
		return FALSE;

	WordListEntry *wlp = &m_WordList[wordListIndex];

	if (permanentFlag == TRUE)
		wlp->state = (eWLState)(WL_TOUCHED | WL_PERMANENT_DELETED_SESSION); 
	else
		wlp->state = (eWLState)(WL_TOUCHED | WL_NOT_PERMANENT_DELETED_SESSION); 
	return TRUE;
}

int CWordList::findWordListIndex(MYWCHAR *word) 
{
	if (m_WordList == NULL)
		return -1;

	WordListEntry *wlp = m_WordList;

	for (int i = 0; i < m_filledEntries; i++, wlp++)
	{
		if (mywcscmp(wlp->word, word) == 0)
		{
			return i;
		}
	}
	return -1;
}
//////////////////////////////////////////////////////////////////////////////////////
int CWordList::insertWord(WCHAR *newWord, int occurence, int preference, int state)
{
	if (m_WordList == NULL || m_filledEntries >= ( m_nWords - 2 ) ||
		m_EndOfWordList <= (int) (m_WordListCharP + mywcslen(newWord) + 1))
	{
		increaseWordListSize(m_nWords + 400, m_nCharacters + 400 * MAX_WORD_LEN);
	}

	int wordListIndex = m_filledEntries;

	m_WordList[wordListIndex].occurence  = occurence;
	m_WordList[wordListIndex].preference = preference;
	m_WordList[wordListIndex].state = (eWLState)state;
	addText(newWord);
	m_filledEntries++;
	return wordListIndex;
}

void CWordList::addText(WCHAR *newWord)
{
	m_WordList[m_filledEntries].word = m_WordListCharP;
	mywcscpy(m_WordListCharP, newWord);
	m_WordListCharP += mywcslen(m_WordListCharP) + 1;
}

BOOL CWordList::deleteWord(WCHAR *newWord)
{
	int idx;

	// check if the word exists in the new word list
	if ((idx = findWordListIndex(newWord)) < 0)
		return false;

	return UIdeleteWord(idx, TRUE);
}


BOOL CWordList::getListEntryAtIndex(int index, WordListEntry *wlp)
{
	if (index < 0 || index >= m_filledEntries)
		return FALSE;

	memcpy(wlp, &m_WordList[index], sizeof(WordListEntry));
	return TRUE;
}

void CWordList::printList(FILE * hFile)
{
	char asciBuf[400];
	size_t dwWritten;

	if (m_WordList == NULL)
		return ;

	WordListEntry *wlp = m_WordList;

	for (int i = 0; i < m_filledEntries; i++, wlp++)
	{
		_snprintf(asciBuf, 400, "%s %d", toA(wlp->word), wlp->preference);
		dwWritten = WriteToFile(hFile, asciBuf, 400);
		dwWritten = WriteToFile(hFile, "\x0d\x0a",4);
	}
}

void CWordList::takeOverPreferences(CWordList *srcWordList)
{
	WordListEntry srcEntry;
	WordListEntry *wlp = m_WordList;

	for (int i = 0; i < m_filledEntries; i++, wlp++)
	{
		if (srcWordList)
		{
			int idx = srcWordList->findWordListIndex(wlp->word);
			if (srcWordList->getListEntryAtIndex(idx, &srcEntry) == TRUE)
				wlp->preference = srcEntry.preference;
		}
		else
		{
			wlp->preference = 0;
		}
	}
}

void CWordList::putNonZeroPrefWordsIntoWordList(CWordList *newWordsDesktopListObject)
{
	WordListEntry *wlp = m_WordList;
	for (int i = 0; i < m_filledEntries; i++, wlp++)
	{
		if (wlp->preference)
			newWordsDesktopListObject->UIaddWord(wlp->word, wlp->preference);
	}
}

#endif