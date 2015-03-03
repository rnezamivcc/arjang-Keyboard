
#ifndef PARSEDLINE_H
#define PARSEDLINE_H

#include "utility.h"

#ifdef WL_SDK
BOOL getWord(FILE* hFile, MYWCHAR *lpWOneWord, int *nCharsP, BOOL unicodeFlag);
DWORD CompleteWord (MYWCHAR *myBuffer, DWORD nCharsInBuffer,  MYWCHAR **lpWordToProcess);
void disinfectWord(MYWCHAR *infectedWord, MYWCHAR *desinfectedWord, int infectedLen);
#endif

DWORD CompleteLine(MYWCHAR *myWBuffer,DWORD nWCharsInBuffer,MYWCHAR *lpWLineToProcess);
bool getLine(FILE* hFile, MYWCHAR *lpWOneLine, BOOL unicodeFlag);

enum eState
{
	STATE_NOTVALID = -1,
	STATE_NOTSET = 0,
	STATE_WORD = 1,
	STATE_PREFERENCE = 2,
	STATE_PERCENTAGE = 3,
	STATE_IGNORED = 4,
	STATE_DESCRIPTION = 5,
	STATE_OCCURANCE = 6,
	MAX_STATE = 20
};

class CParsedLine
{
public:
    CParsedLine(char *);
	CParsedLine(char *parsingSpec, int thresholdPreference, MYWCHAR sepChar);
    ~CParsedLine();
	
#ifdef WL_SDK
	void ScanDictLine(WCHAR *);
#endif
	void ScanCache(WCHAR *);
	int  GetWordSpaceNum();

	unsigned   m_pref;
	unsigned   m_occurance;
	float m_percentage;
	bool  m_bCanChunk;
	BYTE   m_StartPref;

	MYWCHAR m_word [ MAX_WORD_LEN ];
	MYWCHAR m_description [ MAX_WORD_LEN ];
	eState  m_state_reached[MAX_STATE];
	MYWCHAR m_leadChars[MAX_WORD_LEN];
	BOOL m_bListSpecified;

private:
	void initializeStateMachine(char *parsingSpec);

	MYWCHAR m_seperator;
	unsigned   m_thresholdPref;
	unsigned   m_defaultPref;
	unsigned   m_defaultOccurance;

	MYWCHAR m_line [ MAX_WORD_LEN ];
};

#endif   //PARSEDLINE_H
