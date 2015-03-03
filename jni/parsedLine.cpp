
#include "StdAfx.h"
#include "ParsedLine.h"

void replaceUnderscores(MYWCHAR *word);

BOOL ReadWCharsFromFile(FILE* hFile, MYWCHAR *wBufp, int nWCharsToRead, DWORD *nWReadp, BOOL unicodeFlag)
{
	int nBytesToRead = 0;
	DWORD nBytesRead = 0;
	static unsigned char sBuffer[2 * (NSIZE + 4)];

	nBytesToRead = nWCharsToRead;
	if (unicodeFlag == TRUE)
		nBytesToRead *= 2;

	nBytesRead = (DWORD)fread(sBuffer, 1, nBytesToRead, hFile);
	if (nBytesRead == 0) 
	{
		*nWReadp = nBytesRead / sizeof (MYWCHAR);
		return TRUE; // we're at the end of the file 
	}

	if (unicodeFlag == FALSE)
	{
		// convert everything to Unicode
		*nWReadp = nBytesRead;
		 
		DWORD n = 0;
		for (BYTE *chp = sBuffer; n < nBytesRead; n++, chp++)
		{
			if (*chp == 0x92) // replace all variants of apostrophe with the one in the ASCII table
				*chp = 0x27;
		}
		
#ifdef _WINDOWS
		mbstowcs(wBufp, (const char*)sBuffer, nBytesRead);
#else
		int i=0;
		for(; i<NSIZE; i++)
			wBufp[i] = (WCHAR)sBuffer[i];
		wBufp[i]=NUL;
#endif
	}
	else
	{
		*nWReadp = nBytesRead / sizeof (MYWCHAR);
		memcpy(wBufp, sBuffer, nBytesRead);
	}
	return TRUE;
}
/////////////////////////////////////////////////////////////////////////////////////
DWORD CompleteLine(MYWCHAR *myWBuffer,DWORD nWCharsInBuffer,MYWCHAR *lpWLineToProcess)
{
	if (lpWLineToProcess == NULL)
		return 0;

	MYWCHAR *curcharp = lpWLineToProcess;

	while (((long) curcharp) < ((long)(myWBuffer + nWCharsInBuffer)))
	{
		if (*curcharp != LF && *curcharp != CR)  // if it is not linefeed or carriage return
			curcharp++;
		else 
			return (DWORD) (curcharp - lpWLineToProcess);
	}
	return 0;
}

#ifdef WL_SDK
//////////////////////////////////////////////////////////////////////////////////////////
BOOL getWord(FILE* hFile, MYWCHAR *lpWOneWord, int *nCharsP, BOOL unicodeFlag)
{
	static MYWCHAR myWBuffer[NSIZE];
	static MYWCHAR *lpWWordToProcess = NULL;
	static MYWCHAR *lpWNextLine = NULL;
	static DWORD nWCharInBuffer = 0;

	DWORD nWCharsRead = 0;
	DWORD nWCharsToRead = 0;
	DWORD nWCharsInWordToProcess = 0;
	DWORD nWPrevious = 0;

	if (hFile == 0 && lpWOneWord == NULL)
	{
		// reset the static structures
		memset(myWBuffer,0,sizeof(myWBuffer));
		lpWWordToProcess = NULL;
		lpWNextLine = NULL;
		nWCharInBuffer = 0;
		return TRUE;
	}

	int maxlen = *nCharsP;
	*nCharsP = 0;

	while ((nWCharsInWordToProcess = CompleteWord(myWBuffer,nWCharInBuffer,&lpWWordToProcess)) == 0)
	{
		BOOL bResult;

		if (lpWWordToProcess && lpWWordToProcess < (myWBuffer + NSIZE))
		{
			// copy the remainder of the buffer to the beginning
			nWPrevious = wcslen(lpWWordToProcess);
			memcpy(myWBuffer, lpWWordToProcess, nWPrevious * sizeof(MYWCHAR));
		}
		nWCharsToRead = NSIZE - nWPrevious;

		bResult = ReadWCharsFromFile(hFile, (WCHAR *) myWBuffer + nWPrevious,
			nWCharsToRead , &nWCharsRead, unicodeFlag);

		// Check for end of file. 
		if (bResult &&  nWCharsRead == 0) 
		{ 
			return FALSE; // we're at the end of the file 
		}
		lpWWordToProcess = myWBuffer;
		nWCharInBuffer = nWCharsRead + nWPrevious;
	}

	if ((int)nWCharsInWordToProcess > maxlen)
	{
		// skip a word which is too big
		lpWWordToProcess += (nWCharsInWordToProcess + 1);
		*lpWOneWord = NUL;
		*nCharsP = 0;
		return TRUE;
	}

	memcpy(lpWOneWord,lpWWordToProcess,nWCharsInWordToProcess * sizeof(MYWCHAR));
	*(lpWOneWord + nWCharsInWordToProcess) = NUL;
	lpWWordToProcess += (nWCharsInWordToProcess + 1);

	*nCharsP = nWCharsInWordToProcess;
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
DWORD CompleteWord (MYWCHAR *myBuffer, DWORD nWCharsInBuffer,  MYWCHAR **pWordToProcess)
{
	if (*pWordToProcess == NULL)
		return 0;

	MYWCHAR *curcharp = *pWordToProcess;
	while (((int) curcharp) < ((int)(myBuffer + nWCharsInBuffer)))
	{
		if (NOT_SP_CR_TAB(*curcharp) && *curcharp != LF)
			curcharp++;
		else if (curcharp == *pWordToProcess)
		{
			curcharp++;
			*pWordToProcess = curcharp;
		}
		else
		{
			return (DWORD) (curcharp - *pWordToProcess);
		}
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////
void disinfectWord(MYWCHAR *infectedWord, MYWCHAR *desinfectedWord, int infectedLen)
{
	MYWCHAR *pBegin, *pEnd;
	int desinfectedLen = infectedLen;
	BOOL isPunctuation(MYWCHAR c);

	pBegin = infectedWord;
	pEnd   = infectedWord + infectedLen;
	while (pBegin < pEnd && isPunctuation(*pBegin))
	{
		pBegin++;
		desinfectedLen--;
	}

	pEnd--;
	while (pEnd >= pBegin && isPunctuation(*pEnd))
	{
		pEnd--;
		desinfectedLen--;
	}
	pEnd++;

	wcsncpy(desinfectedWord, infectedWord, desinfectedLen);
	desinfectedWord[desinfectedLen] = NUL;

	if ((desinfectedWord[0] >= (MYWCHAR)'A' && desinfectedWord[0] <= (MYWCHAR)'Z') || isUCSpecialCharacter(desinfectedWord[0]))
		mywcslwr(desinfectedWord);
}

///////////////////////////////////////////////////////////////////////////////
void CParsedLine::ScanDictLine(MYWCHAR *lpWLine)
{
	MYWCHAR *chp = NULL;
	MYWCHAR field[MAX_WORD_LEN];
	MYWCHAR *fieldp;
	int len;
	int state_nr = 0;

	memset(field, '\0', sizeof(field));
	chp = lpWLine;
	fieldp = field;
	m_description[0] = NUL;
	m_pref = m_defaultPref;
	m_percentage = 0;	
	m_word[0] = NUL;
	m_bCanChunk = true;

	m_StartPref = 0;

	state_nr = 0;

	int n =0;
	bool bNumber = true;
	while (1)
	{
	
		if (	*chp == NUL || *chp == LF ||  *chp == HTAB || *chp == CR
			|| (*chp == m_seperator && m_state_reached[state_nr] != STATE_DESCRIPTION && (field[0] != (MYWCHAR) '#' || field[1] != (MYWCHAR) '#')))
		{
			switch (m_state_reached[state_nr]) 
			{
			  case STATE_IGNORED:
					break;
			  case STATE_PREFERENCE:
				  n = mywcslen(field);
				  MYWCHAR temp[MAX_WORD_LEN];
				  memset(temp,0, sizeof(temp));

				  if(field[n-1] == CARET)
				  {
					  int index =0;
					  bool bStart = false;
					  for(int i=0; i < n; i++)
					  {
						   if(bStart && field[i] == CARET)
						  {
							  field[i]  = NUL;
							  break;
						  }

						  if(bStart)
						  {
							  temp[index] = field[i];
							  field[i]  = NUL;
							  index++;
						  }

						  if(!bStart && field[i] == CARET)
						  {
							  bStart = true;
							  field[i]  = NUL;
						  }
	 
					  }

					  m_StartPref = _wtoi(temp);

				  }

				  n = mywcslen(field);
				  if(field[n-1] == ASTERISK)
				  {
					  m_bCanChunk = false;
					  field[n-1] = NUL;
					  n = mywcslen(field);
				  }
				  else
				  {
					  m_bCanChunk = true;
				  }

				  n = mywcslen(field);

				  bNumber = true;
				  for(int i=0; i < n; i++)
				  {
					  if(!isNumberChar(field[i]))
					  {
						  bNumber = false;
						  break;
					  }
				  }

				  if (field[0] >= (MYWCHAR) '0' && field[0] <= (MYWCHAR) '9' && bNumber)
						m_pref = _wtoi(field);
				  else if (m_state_reached[state_nr -1] == STATE_WORD)
				  {
					  // append this field to previous field
					  wcscat(m_word, L" ");
					  wcscat(m_word, field);
					  len = wcslen(m_word);
					  state_nr--;
					  //m_bPhraseSpecified = TRUE;
				  }
				  else
				  {
		  			  printf("trouble in scanner on line %s\n", toA(lpWLine));
					  exit(7);
				  }
			  	  break;
			  case STATE_PERCENTAGE:
				  if (field[0] >= (MYWCHAR)'0' && field[0] <= (MYWCHAR)'9')
				  {
					m_percentage = atof(toA(field));
					m_pref = 1;
				  }
				  else if (m_state_reached[state_nr -1] == STATE_WORD)
				  {
					  // append this field to previous field
					  wcscat(m_word, L" ");
					  wcscat(m_word, field);
					  state_nr--;
					  //m_bPhraseSpecified = TRUE;
				  }
				  else 
				  {
  		  			  printf("trouble in scanner on line %s\n", toA(lpWLine));
					  exit(7);
				  }
			  	  break;
			  case STATE_WORD:
				  {
					  MYWCHAR *sfieldp = field;
					  MYWCHAR *endlistname;

					  if (wcsncmp(field, L"##", 2) == 0 && (endlistname = wcsstr(field + 2, L"##")))
					  {	
						  MYWCHAR *liststart = endlistname + 2;
						  MYWCHAR *listend = wcsstr(endlistname + 2, L"##");
						  if (!listend && listend > liststart)
						  {
		   		  			  printf("!!!No end of list detection on line %s\n", toA(lpWLine));
							  exit(3);
						  }

						//  if (listend > liststart)
							wcsncpy(m_leadChars, liststart, (int)(listend - liststart));
						//  else
						//		m_leadChars[0] = (MYWCHAR) '\0';

						  int nchars = (int) (endlistname - (field + 2));
						  wcsncpy(m_word, field+2, nchars);
						  *(m_word + nchars) = NUL;
						  m_bListSpecified = TRUE;
					  }
					  else
					  {
						  len = wcslen(sfieldp);
						  memcpy(m_word, sfieldp, len * sizeof(MYWCHAR));
						  *(m_word + len) = NUL;
						  m_bListSpecified = FALSE;
					  }
					  break;
				  }
			  case STATE_DESCRIPTION:
				  {
				  len = wcslen(field);
				  MYWCHAR *dst = m_description;
				  MYWCHAR *myp;

				  // trim off the blanks at the end of the word
				  for (myp = &field[len - 1]; *myp == SP; myp--)
					  *myp = NUL;

/*				  for (myp = field; *myp; myp++)
				  {
#ifdef UNDERSCORE 
					  if (*myp == SP)
						  *dst++ = L'_';
					  else
#endif
						  *dst++ = *myp;
				  }
*/
				  *dst = NUL;
				  break;
				  }
			  default :
				  if (m_state_reached[state_nr -1] == STATE_WORD)
				  {
					  // append this field to previous field
					  wcscat(m_word, L" ");
					  wcscat(m_word, field);
					  state_nr--;
					  //m_bPhraseSpecified = TRUE;
				  }
				  else
				  {
					  // make sure there are no spaces behind the last field and new-line
 					  printf("!!!bad line %s processed \n", toA(lpWLine));
					  exit(1);
				  }
				  break;
			}
			state_nr++;

			if (*chp == NUL  || *chp == LF || *chp == CR)
				break;

			while (*(chp+1) == m_seperator)
				chp++;

			memset(field, NUL,sizeof(field));
			fieldp = field;
		}
		else
			*fieldp++ = *chp;

		chp++;
		if ((*chp == NUL  || *chp == LF || *chp == CR) && (m_defaultPref != m_pref))
			break;
	}

	if (m_pref < m_thresholdPref)
	{
		// doesn't qualify because of the threshold, wipe it out
		m_word[0] = NUL;
		return;
	}

#ifndef UNDERSCORE
	replaceUnderscores(m_word);
	replaceUnderscores(m_description);
#endif
}
#endif // WL_SDK
////////////////////////////////////////////////////////////////////////
bool getLine(FILE* fp, MYWCHAR *lpWOneLine)
{
	return getLine(fp, lpWOneLine, FALSE);
}

////////////////////////////////////////////////////////////////////////////////////////////
bool getLine(FILE* fp, MYWCHAR *lpWOneLine, BOOL unicodeFlag)
{
	static MYWCHAR myWBuffer[NSIZE];
	static MYWCHAR *pLineToProcess = NULL;
	static MYWCHAR *lpWNextLine = NULL;
	static DWORD nWCharInBuffer = 0;
	DWORD nWBytesRead = 0;
	DWORD nWBytesToRead = 0;
	DWORD nWCharsToProcess = 0;
	DWORD nWPrevious = 0;

	if (!fp && !lpWOneLine)
	{
		pLineToProcess = 0;
		memset(myWBuffer,0,sizeof(myWBuffer));
		lpWNextLine = NULL;
		nWCharInBuffer = 0;
		nWCharsToProcess = 0;
		return false;
	}

	WCHAR *endofBuffer = myWBuffer + nWCharInBuffer;
	if (pLineToProcess)
	{
		while (*pLineToProcess && pLineToProcess < endofBuffer &&
			(*pLineToProcess == CR || *pLineToProcess == LF))
			pLineToProcess++;
	}

	if ((nWCharsToProcess = CompleteLine(myWBuffer, nWCharInBuffer, pLineToProcess)) == 0)
	{
		bool bResult;

		if (pLineToProcess && pLineToProcess < (myWBuffer + NSIZE))
		{
			nWPrevious = (DWORD)(endofBuffer - pLineToProcess); //mywcslen(pLineToProcess); // copy the remainder of the buffer to the beginning
			memcpy(myWBuffer, pLineToProcess, nWPrevious * sizeof(MYWCHAR));
		//	printf(" lstrlen buffer %d,  read max in %d \n", wcslen(myWBuffer), nWBytesToRead);
		}

		nWBytesToRead = NSIZE - nWPrevious;
		bResult = ReadWCharsFromFile(fp, (MYWCHAR *) myWBuffer + nWPrevious, nWBytesToRead , &nWBytesRead, unicodeFlag);

		// Check for end of file. 
		if (bResult &&  nWBytesRead == 0) 
		{ 
			return false; // we're at the end of the file 
		}

		pLineToProcess = myWBuffer;
		nWCharInBuffer = nWBytesRead + nWPrevious;

		if ((nWCharsToProcess = CompleteLine(myWBuffer,nWCharInBuffer, pLineToProcess)) == 0)
		{
			pLineToProcess++;
			if ((nWCharsToProcess = CompleteLine(myWBuffer,nWCharInBuffer,pLineToProcess)) == 0)
			{
				printf("!!!No line in a newly fresh buffer ");
				exit(1);
			}
		}
	}

	memcpy(lpWOneLine, pLineToProcess,nWCharsToProcess * sizeof(MYWCHAR));
	*(lpWOneLine + nWCharsToProcess) = NUL;
	pLineToProcess += (nWCharsToProcess + 1);
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
CParsedLine::CParsedLine(char *parsingSpec, int thresholdPref, MYWCHAR sepChar)
{
	m_seperator = sepChar;
	m_thresholdPref = thresholdPref; // process everything with a minimum of threshold Preference
	initializeStateMachine(parsingSpec);
	memset(m_word, 0, sizeof(m_word));
	memset(m_description, 0, sizeof(m_description));
	memset(m_leadChars, 0, sizeof(m_leadChars));
	m_bCanChunk = true;
	m_StartPref = 0;
}

///////////////////////////////////////////////////////////////////////////////////
CParsedLine::CParsedLine(char *parsingSpec)
{
	m_seperator = SP;
	m_thresholdPref = 0; // process everything
	m_defaultPref = 1;
	initializeStateMachine(parsingSpec);
	memset(m_word, 0, sizeof(m_word));
	memset(m_description, 0, sizeof(m_description));
	memset(m_leadChars, 0, sizeof(m_leadChars));
	m_bCanChunk = true;
	m_StartPref =0;
}
///////////////////////////////////////////////////////////////////////////////////
void CParsedLine::initializeStateMachine(char *parsingSpec)
{
	int state_nr = 0;

	for (int i = 0 ; i < MAX_STATE ; i++)
		m_state_reached[i] = STATE_NOTVALID;

	m_leadChars[0] = NUL;

	for (char *c = parsingSpec; *c; c++)
	{
		switch (*c) 
		{
		case 's':
			m_state_reached[state_nr++] = STATE_WORD;
			break;
		case 'p':
			m_state_reached[state_nr++] = STATE_PREFERENCE;
			m_defaultPref = 0;
			break;
		case 'o':
			m_state_reached[state_nr++] = STATE_OCCURANCE;
			m_defaultOccurance = 1;
			break;
		case 'f':
			m_state_reached[state_nr++] = STATE_PERCENTAGE;
			break;
		case 'i':
			m_state_reached[state_nr++] = STATE_IGNORED;
			break;
		case 'd':
			m_state_reached[state_nr++] = STATE_DESCRIPTION;
			break;
		default:
			printf("ERROR!!trouble initializing statemachine in line parser\n");
			exit(6);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CParsedLine::ScanCache(MYWCHAR *lpWLine)
{
	MYWCHAR *chp = NULL;
	MYWCHAR field[MAX_WORD_LEN];
	MYWCHAR *fieldp;
	int state_nr = 0;
    int len = 0;
	memset(field, '\0', sizeof(field));
	chp = lpWLine;
	fieldp = field;
	m_description[0] = NUL;
	m_pref = m_defaultPref;
	m_occurance = m_defaultOccurance;
	m_percentage = 0;
	m_word[0] = NUL;
//	if(chp[0] != (MYWCHAR) '#' || chp[1] != (MYWCHAR) '#') // skip  comment line
//		return;

	state_nr = 0;
	while (1)
	{
		if (	*chp == NUL || *chp == LF ||  *chp == HTAB || *chp == CR
			|| (*chp == m_seperator && m_state_reached[state_nr] != STATE_DESCRIPTION && (field[0] != (MYWCHAR) '#' || field[1] != (MYWCHAR) '#')) )
		{
			switch (m_state_reached[state_nr]) 
			{
			  case STATE_PREFERENCE:
				  if (field[0] >= (MYWCHAR)'0' && field[0] <= (MYWCHAR)'9')
					m_pref = atoi(toA(field));
				  else if (m_state_reached[state_nr -1] == STATE_WORD)
				  {
					  // append this field to previous field
					  mywcscat(m_word, (MYWCHAR*)L" ");
					  mywcscat(m_word, field);
					  state_nr--;
				  }
				  else
				  {
		  			  ShowInfo("trouble in scanner on line %s\n", toA(lpWLine));
					  exit(7);
				  }
			  	  break;
			  case STATE_OCCURANCE:
				  if (field[0] >= (MYWCHAR)'0' && field[0] <= (MYWCHAR)'9')
				  {
					m_occurance = atoi(toA(field));
				  }
				  else 
				  {
  		  			  ShowInfo("trouble in scanner on line %s\n", toA(lpWLine));
					  exit(7);
				  }
			  	  break;
			  case STATE_WORD:
				  {
					  MYWCHAR *sfieldp = field;
					  MYWCHAR *endlistname;

					  if (mywcsncmp(field, toW("##"), 2) == 0 && (endlistname = mywcsstr(field + 2, toW("##"))))
					  {	
						  MYWCHAR *liststart = endlistname + 2;
						  MYWCHAR *listend = mywcsstr(endlistname + 2, toW("##"));
						  if (!listend && listend > liststart)
						  {
		   		  			  ShowInfo("!!!No end of list detection on line %s\n", toA(lpWLine));
							  exit(3);
						  }

						  mywcsncpy(m_leadChars, liststart, (int)(listend - liststart));

						  int nchars = (int) (endlistname - (field + 2));
						  mywcsncpy(m_word, field+2, nchars);
						  *(m_word + nchars) = NUL;
						  m_bListSpecified = TRUE;
					  }
					  else
					  {
						  len = mywcslen(sfieldp);
						  memcpy(m_word, sfieldp, len * sizeof(MYWCHAR));
						  *(m_word + len) = NUL;
						  m_bListSpecified = FALSE;
					  }
					  break;
				  }
			  default :
				  if (m_state_reached[state_nr -1] == STATE_WORD)
				  {
					  // append this field to previous field
					  mywcscat(m_word, toW(" "));
					  mywcscat(m_word, field);
					  state_nr--;
				  }
				  else
				  {
					  // make sure there are no spaces behind the last field and new-line
 					  ShowInfo("!!!bad line %s processed \n", toA(lpWLine));
					  exit(1);
				  }
				  break;
			}
			state_nr++;

			if (*chp == NUL  || *chp == LF || *chp == CR)
				break;

			while (*(chp+1) == m_seperator)
				chp++;

			memset(field, NUL,sizeof(field));
			fieldp = field;
		}
		else
			*fieldp++ = *chp;

		chp++;
	}
}

/////////////////////////////////////////////////////////////
CParsedLine::~CParsedLine()
{
}
//////////////////////////////////////////////////////////////////
int CParsedLine::GetWordSpaceNum()
{
	int nTotalLength = mywcslen(m_word);
	int nSpace = 0;
	for(int i=0; i < nTotalLength; i++)
	{
		MYWCHAR letter = m_word[i];
		if(letter == SP)
		{
			nSpace++;
		}
	}

	return nSpace;
}

void replaceUnderscores(MYWCHAR *word)
{
	MYWCHAR *chp = word;

	// see whether it is an e-mail or web-site address, if so hands off
	if (mywcsstr(word,toW("@")) || mywcsstr(word,toW(".")) || mywcsstr(word,toW("/")))
		return;

	while (*chp)
	{
		if (*chp == (MYWCHAR) '_')
			*chp = (MYWCHAR) ' ';
		chp++;
	}
}
