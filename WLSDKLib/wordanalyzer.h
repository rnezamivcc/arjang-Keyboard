
#ifndef WORDANALYZE_H
#define WORDANALYZE_H

#include "utility.h"

class CWordAnalyzer
{
public:
    CWordAnalyzer();
    ~CWordAnalyzer();
    WCHAR *analyzeWord(MYWCHAR *wordSeen, int nChars);

    BOOL m_EndOfSentence;
	MYWCHAR m_StrippedWord[MAX_WORD_LEN * 2];
private:
};

#endif   //WORDANALYZE_H
