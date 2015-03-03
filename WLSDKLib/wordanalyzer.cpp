
#include "StdAfx.h"
#include "WordAnalyzer.h"


CWordAnalyzer::CWordAnalyzer()
{
	m_EndOfSentence = FALSE;
	m_StrippedWord[0] = NUL;
}


MYWCHAR *CWordAnalyzer::analyzeWord(MYWCHAR *lpWord, int nChars)
{
	int lastIdx = 0;

	if (*(lpWord + nChars - 1) == (MYWCHAR) '.'  && nChars > 1 &&
		*(lpWord + nChars - 2) >= (MYWCHAR) '0' && *(lpWord + nChars - 2) <= (MYWCHAR) 'z') 
	{
		m_EndOfSentence = TRUE;
		lastIdx = nChars - 2;	// strip the '.'
		if (lastIdx >= 0)
		{
			wcsncpy(m_StrippedWord,lpWord, lastIdx + 1);
			m_StrippedWord[lastIdx + 1] = NUL;
			return m_StrippedWord;
		}	
	}
	else if (nChars > 4 && *lpWord == (MYWCHAR) '`' && *(lpWord+1) == (MYWCHAR) '`' &&
			 *(lpWord+nChars-2) == (MYWCHAR) '\'' && *(lpWord+nChars-1) == (MYWCHAR) '\'')
	{
		// take out the `` and '' , for instance ``hunt-and-peck'' becomes hunt-and-peck
		wcsncpy(m_StrippedWord,lpWord+2, nChars - 4);
		m_StrippedWord[nChars - 4] = NUL;
		return m_StrippedWord;
	}
	else
	{
		lastIdx = nChars - 1;

		// strip punctuation from beginning
		//
		while ( nChars >= 0 && isPunctuation(*lpWord))
		{
			lpWord++;
			lastIdx--;
		}
		while (lastIdx >= 0 && isPunctuation(lpWord[lastIdx]))
			lastIdx--;

		if (lastIdx >= 0)
		{
			wcsncpy(m_StrippedWord,lpWord, lastIdx + 1);
			m_StrippedWord[lastIdx + 1] = (WCHAR) '\0';

			if (m_EndOfSentence)
			{
				int i;
				MYWCHAR wChars[MAX_WORD_LEN];
				MYWCHAR oldwChars[MAX_WORD_LEN];

				wcsncpy(wChars, m_StrippedWord,nChars);
				wChars[nChars] = NUL;
				wcscpy(oldwChars, wChars);
				oldwChars[nChars] = NUL;
				mywcslwr(wChars);
				
				for (i = 1; i < nChars; i++)
				{
					if (wChars[i] != oldwChars[i])
						break;
				}
				if (i >= nChars)
				{
//					wcstombs(m_StrippedWord, wChars,1);
					int k = 1; // DebugBreak here , weird !!
				}
			}
			m_EndOfSentence = FALSE;
			return m_StrippedWord;
		}
	}
	return NULL;
}


CWordAnalyzer::~CWordAnalyzer()
{
}


