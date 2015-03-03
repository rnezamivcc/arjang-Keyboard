// <copyright file="utility.cpp" company="WordLogic">
// Copyright (c) 2001, 2013 All Right Reserved, http://www.wordlogic.com/
//
// THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
// KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// </copyright>
// <author>Reza Nezami</author>
// <email>rnezami@wordlogic.com</email>
// <date>2012-06-10</date>
// <summary>This contains mostly string utilities we need by various parts of the engine.</summary>


#include "StdAfx.h"
#include "dictrun.h"

CPPExternOpen

#include "compatibility.h"
#include "compactstore.h"
#include "utility.h"

#ifdef _WINDOWS
	#include <stdarg.h>
#else
	#include <stdlib.h>
	#include <ctype.h>
	#include <wchar.h>
	#include <unistd.h>
	#ifndef __APPLE__
		#include <android/log.h>
	#endif
#endif

#include "wordpunct.h"

char sRootDictPath[MAX_PATH];
char *g_szStartDirectory = sRootDictPath;


size_t WriteToFile(FILE *fp, char *address, int size) 
{
	return fwrite(address, size, 1, fp);
}

BOOL readFromFile(FILE *fp, char *address, int size) 
{
	int result = (int)fread(address, 1, size, fp);
	if (result != size)
		return FALSE;
	return TRUE;
}

unsigned myClamp(unsigned val, unsigned min, unsigned max, unsigned range)
{
	if(val<min)
		return 0;
	assert(val<=range);
	return (unsigned)((((float)val/(float)(range)) * (float)(max-min+1)));
}

 /*************************************************************************************
 djb2 hashing algorithm
this algorithm (k=33) was first reported by dan bernstein in comp.lang.c. another version of this algorithm (now favored by bernstein) 
uses xor: hash(i) = hash(i - 1) * 33 ^ str[i]; the magic of number 33 (why it works better than many other constants, prime or not) 
has never been adequately explained.
 *************************************************************************************/
unsigned long djb2_hash(MYWCHAR *str)
{
    unsigned long hash = 5381;
    int c;

    while (*str != NUL)
    {
		c = (int)*str++;
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	}   

    return hash;
}
///////////////////////////////////////////////////////////////////////////////////////

char char2A(const WCHAR wc)
{
	char ret[2];
	//wcstombs(&ret[0], &wc, 1);
	ret[0] = (char)wc;
	return ret[0];
}

MYWCHAR char2W(char c)
{
#ifdef _WINDOWS
	char asciword[1] = { c};
	wchar_t wc;
	mbtowc(&wc, &asciword[0], MB_CUR_MAX);
#else
	MYWCHAR wc = (MYWCHAR) c;
#endif
	return wc;
}

#if defined(DEBUG)
char *toA(MYWCHAR *word)
{
	static unsigned sNBuffersUsed = 0;
	static char printBuffers[10][MAX_WORD_LEN];

	if(!word || word[0] == NUL)
	{
		//	ShowInfo("!!!ERROR! null string passed to toA! ignore it and return empty string!\n");
		return (char *)"";
	}

	char *str = &printBuffers[sNBuffersUsed][0];
	//wcstombs(str, word, MAXBUF-1);
	size_t i = 0;
	for (i = 0; word[i] && i<199; i++) 
	{
		str[i] = (char)word[i];
	}
	str[i] = 0;

	sNBuffersUsed++;
	sNBuffersUsed %= 10;
	return str;
}
WCHAR *toW(const char *asciword)
{
	static WCHAR output[150];
	memset(output, 0, sizeof(output));
	size_t len = strlen(asciword);
	size_t i=0;
	for(; i<len; i++)
//		mbtowc(&output[i], &asciword[i], MB_CUR_MAX);
		output[i] = (WCHAR)asciword[i];
	output[i]=NUL;
	//mbstowcs(output, asciword, 150);

	return (WCHAR*)output;
}
#else
char *toA(WCHAR *word) { return (char*)NULL; }
WCHAR *toW(const char *asciword) { return (WCHAR*)NULL; }
#endif //DEBUG

void wToA(WCHAR *word, char *ansiWord, unsigned size) 
{
//	wcstombs(ansiWord, word, size);
	int i = 0;
	for (i = 0; word[i] && i < size; i++)
	{
		ansiWord[i] = word[i];
	}
	ansiWord[i] = 0;
}

void AToW(char *asciword, WCHAR* wstr)
{
	int len = (int)strlen(asciword);
	int i=0;
	for(; i<len; i++)
//		mbtowc(&output[i], &asciword[i], MB_CUR_MAX);
		wstr[i] = asciword[i];
	wstr[i]=NUL;
}

MYWCHAR* makeWord(MYWCHAR *word)
{
	static MYWCHAR curWord[MAX_WORD_LEN];
	mywcscpy(curWord, word);
	return curWord;
}

#if defined(_LOGGING)
FILE *gLogFile = NULL;

#include <time.h>

static void CreateLogFile()
{
	static bool sIgnoreLogging = false;
	if(sIgnoreLogging || gLogFile)
	{
//		callLogA("\n!!CreateLogFile : logfile already open!! Ignore!\n");
		return;
	}
	if(g_szStartDirectory==NULL || strlen(g_szStartDirectory)==0)
	{
#if defined(_WINDOWS)
		strcpy(g_szStartDirectory, "c:/code/wl_root/trunk/dictionary");
#else
		strcpy(g_szStartDirectory, "/sdcard/wordlogic/dictionary");
#endif
		callLogA("\n!!CreateLogFile : g_szStartDirectory not set yet!! set it to default!\n");
	}
	callLogA("\n!!CreateLogFile for first time!\n");

	time_t seconds = time(NULL);
	int picker = seconds % 3;
	char fullPathFileName[150];
	strcpy(fullPathFileName, g_szStartDirectory);
	if(picker ==0)
		strcat(fullPathFileName, "/dictlog0.txt");
	else if(picker == 1)
		strcat(fullPathFileName, "/dictlog1.txt");
	else if(picker == 2)
		strcat(fullPathFileName, "/dictlog2.txt");

	// Open the dictionary file     
	gLogFile = fopen(fullPathFileName, "wb");
	if(gLogFile)
	{	
		callLog("\n!Opened log file at "); 
		callLog(fullPathFileName); callLog("\n");
	}
	else
	{	
		callLogA("\n!!!!ERROR! failed to opened log file at "); 
		callLogA(fullPathFileName); callLogA("--Ignore Logging for now!!\n");
		sIgnoreLogging = true;
	}
}
////////////////////////////////////////////////////////////
void CloseLogFile() 
{ 
	if (gLogFile)
	{
		fclose(gLogFile);
		gLogFile = NULL;
	}
}
/////////////////////////////////////////////////////////
void sprintfile(char *format, ...)
{
	va_list marker;
	int iRet;
	static char pchBuf[400];

	va_start(marker, format);
	iRet = vsprintf(pchBuf, format, marker);
	va_end(marker);

	if (gLogFile == NULL)
		CreateLogFile();

	if (gLogFile != NULL)
	{
		fputs(pchBuf, gLogFile);
		fputs("\n", gLogFile);
		fflush(gLogFile);
	}
}
#endif // LOGGING
////////////////////////////////////////////////////////////////////////////////////////////////////////////
eEndianNess testEndianNess()
{
	eEndianNess ret = eLITTLE_ENDIAN32;
	unsigned value= 0x78006712;
	unsigned char *p = (unsigned char *) &value;
	unsigned width = sizeof(p);
	if (*p == 0x78)
	{
		if(width == 4)
			ret = eBIG_ENDIAN32;
		else
			ret = eBIG_ENDIAN64;
	}
	else if (*p == 0x12)
	{
		if(width == 4)
			ret = eLITTLE_ENDIAN32;
		else
			ret = eLITTLE_ENDIAN64;
	}
	return ret;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
intptr_t queryValueAtOffset(char *p, unsigned size, unsigned offset)
{
	intptr_t val = 0;
	unsigned char *tp = (unsigned char *) p + offset;

	switch (size)
	{
            
		case 1:
			val = *(BYTE *) (tp);
			break;
		case 2: 
		{
			if ((unsigned long) tp & 1)
			{
				val = *tp++;
				val |= (*tp << 8);
			} 
			else
				val = *(USHORT *) (tp);
			break;
		}
		case 3: 
		{
			val = *tp++;
			val |= (*tp++ << 8);
			val |= (*tp << 16);
			break;
		}
		case 4: 
		{
			if ((unsigned long) tp & 1)
			{
				val = *tp++;
				val |= (*tp++ << 8);
				val |= (*tp++ << 16);
				val |= (*tp << 24);
			} 
			else if ((unsigned long) tp & 2)
			{
				val = (unsigned int) *((USHORT *) tp);
				tp += 2;
				val |= (unsigned int) (*((USHORT *) tp)) << 16;
			} 
			else
				val = *(UINT *) tp;
			break;
		}
		case 8:
			assert(((unsigned long)tp & 0x1) == 0);
			if ((unsigned long)tp & 0x2) // starts on 2 byte boundary
			{
				val = (unsigned int)*((USHORT *)tp); // select the low order word of the long
				tp += 2;
				val |= (unsigned int)(*((USHORT *)tp)) << 16;
				tp += 2;
				val |= (unsigned int)(*((USHORT *)tp)) << 32;
				tp += 2;
				val |= (unsigned int)(*((USHORT *)tp)) << 48;
			}
			else if ((unsigned long)tp & 0x4) // starts on 4 byte boundary
			{
				val = (unsigned int)*((UINT *)tp);
				tp += 4;
				val |= (unsigned int)(*((UINT *)tp)) << 32;
			}
			else
				val = *(uintptr_t*)tp;
			break;
		default:
			DebugBreak((char *)"qfieldoff");
	}
	return val;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
void setValueAtOffset(char *p, int size, unsigned offset, uintptr_t val)
{
	char *tp = p + offset;

	switch (size) 
	{
		case 1:
			*(BYTE *) tp = val;
			break;
		case 2:
		{
			if ((unsigned long) tp & 1)
			{
				*tp = val & 0xff;
				*(tp + 1) = (val >> 8) & 0xff;
			} 
			else
				*(USHORT *) tp = val;
			break;
		}
		case 3:
		{
			WLBreakIf(val>>24 != 0, "!!!ERROR!! setValueAtOffset: value #%x# is bigger than 3 bytes!! so won't fit in 3 bytes slot!\n", val);
			*tp = val & 0xff;
			tp++;
			*tp = (val >> 8) & 0xff;
			tp++;
			*tp = (val >> 16) & 0xff;
			tp++;
			break;
		}
		case 4:
		{
			if ((unsigned long) tp & 0x1) // starts on a byte boundary
			{
				*tp = val & 0xff;
				tp++;
				*tp = (val >> 8) & 0xff;
				tp++;
				*tp = (val >> 16) & 0xff;
				tp++;
				*tp = (val >> 24) & 0xff;
			} 
			else if ((unsigned long) tp & 0x2) // starts on 2 byte boundary
			{
				USHORT *pShort = ((USHORT *) tp); // select the low order word of the long
				*pShort = (USHORT) val & 0xffff;
				pShort++; // select the high order word of the long
				*pShort = (USHORT)(val >> 16);
			} else
				*(UINT *) tp = val;
			break;
		}
		case 8:
			assert(((unsigned long)tp & 0x1) == 0);
			if ((unsigned long)tp & 0x2) // starts on 2 byte boundary
			{
				USHORT *pShort = ((USHORT *)tp); // select the low order word of the long
				*pShort = (USHORT)val & 0xffff;
				pShort++; // select the high order word of the long long
				*pShort = (USHORT)(val >> 16);
				pShort++; // select the high order word of the long long 
				*pShort = (USHORT)(val >> 32);
				pShort++; // select the high order word of the long long
				*pShort = (USHORT)(val >> 48);
			}
			else if ((unsigned long)tp & 0x4) // starts on 4 byte boundary
			{
				UINT *puInt = (UINT *)tp;
				*puInt = (UINT)val & 0xffffffff;
				puInt++;
				*puInt = (UINT)(val >> 32);
			}
			else
				*(uintptr_t*)tp = val;
			break;
		default:
			DebugBreak((char *)"sfieldoff");
	}
}
/////////////////////////////////////////////
void WordsArray::reset()
{
	memset(this, 0, sizeof(WordsArray));
	for(int i=0; i<MAX_NUM_PREDICTIONS; i++)
	{
		nextWords[i] = words[i];
	}

}
/////////////////////////////////////////////////////
bool wordContainsLetter(MYWCHAR *word, MYWCHAR letter)
{
	int len = mywcslen(word);
	for(int i=0; i<len; i++)
	{
		if(word[i] == letter)
			return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////
char* addOn(char *pathname, char *suffix, char *filename) 
{
	static char soutputfilename[MAX_WORD_LEN];
	strcpy(soutputfilename, pathname);
	strcat(soutputfilename, filename);
	char *endpnt = strstr(&soutputfilename[strlen(pathname)], ".");
	if(endpnt)
		strcpy(endpnt, suffix);
	else
		strcat(soutputfilename, suffix);
	return soutputfilename;
}
/////////////////////////////////////////////////////////////////////////////////////////////////
bool isSeparator(MYWCHAR ch) 
{
	if(ch == (MYWCHAR)' ' || ch == (MYWCHAR)'\n' || ch == (MYWCHAR)'\r') {
		return true;
	} else if(ch == (MYWCHAR)'"' || ch == (MYWCHAR)'(' || ch == (MYWCHAR)')') {
		return true;
	} else if(ch == (MYWCHAR)'.' || ch == (MYWCHAR)'?' || ch == (MYWCHAR)'!' || ch == (MYWCHAR)',') {
		return true;
	} else {
		return false;
	}
}
///////////////////////////////////////////////////////////////////
// obtain file size but don't affect the file position indicator
int getFileSize(FILE *fp) 
{
	int curPos = (int)ftell(fp);

	fseek(fp, 0, SEEK_END);
	int lSize = (int)ftell(fp);

//	rewind(fp);
	fseek(fp, curPos, SEEK_SET);
	return lSize;
}
////////////////////////////////////////////////////////////
#if defined(_LOGGING)
void dumpMemory(char *addr, int len)
{
	char szBuf[200];

	char *p = addr;
	for (int i = 0; i < len; i += 16 )
	{
		sprintf(szBuf, "%x :", p);
		int len = strlen(szBuf);
		for (int j = 0; j < 4; j++)
		{
			sprintf(&szBuf[len], " %x %x %x %x ", *p, *(p+1), *(p+2), *(p+3));
			len = strlen(szBuf);
			p += 4;
		}
		sprintfile("%s", szBuf);
	}
}
#endif
//////////////////////////////////////////////////////////////////////////////
void myNCaseTranslation(MYWCHAR *text, int nChars, BOOL toUpperCase) 
{
	MYWCHAR myWord[MAX_WORD_LEN + 1];

	if (nChars > MAX_WORD_LEN)
		nChars = MAX_WORD_LEN;

	mywcsncpy(myWord, text, nChars);
	myWord[nChars] = NUL;
	if (toUpperCase == TRUE)
		mywcsupr(myWord);
	else
		mywcslwr(myWord);
	mywcsncpy(text, myWord, nChars);
}
//////////////////////////////////////////////////////////////////////////
BOOL isNumber(MYWCHAR *word) 
{
	MYWCHAR *forgetLeadingZeroP = word;

	while (*forgetLeadingZeroP && *forgetLeadingZeroP == NUL)
		forgetLeadingZeroP++;

	BOOL ret = *forgetLeadingZeroP != NUL;
	while(*forgetLeadingZeroP != NUL && isNumberChar(forgetLeadingZeroP[0]))
		forgetLeadingZeroP++;

	return ret && (*forgetLeadingZeroP == NUL);
}

////////////////////////////////////////////////////////////////////////////
static char *LCspecialChars = (char *) "àáâãäåæçèéêëìíîïðñòóôõö\x00\x00ùúûýüþ";
static char *UCspecialChars = (char *) "ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ\x00\x00ÙÚÛÝÜÞ";

static MYWCHAR lwrSpecialCharacter(MYWCHAR c)
{
	if (c >= (MYWCHAR)0xc0 && c <= (MYWCHAR)0xde) 
	{
		int scidx = c - 0xc0;
		if (UCspecialChars[scidx] != 0)
			return (WCHAR) (scidx + 0xe0);
	}
	switch (c) 
	{
	case 0x152:
		return 0x153; // french oe
		break;
	default:
		break;
	}
	return c;
}
///////////////////////////////////////////////////////////////////////
static MYWCHAR uprSpecialCharacter(MYWCHAR c)
{
	if (c >= (MYWCHAR)0xe0 && c <= (MYWCHAR)0xfe) 
	{
		int scidx = c - (MYWCHAR)0xe0;
		if (LCspecialChars[scidx] != 0)
			return (WCHAR) (scidx + 0xc0);
	}
	switch (c) 
	{
	case 0x153:
		return 0x152; // french oe
		break;
	default:
		break;
	}
	return c;
}
////////////////////////////////////////////////////////
WCHAR uprCharacter(MYWCHAR c)
{
	if (c >= (MYWCHAR)'a' && c <= (MYWCHAR)'z')
		return  (WCHAR)'A' + ( c - (MYWCHAR)'a' );
	else
		return uprSpecialCharacter(c); 
}
///////////////////////////////////////////////////////////
WCHAR lwrCharacter(MYWCHAR c)
{
	if (c >= (MYWCHAR)'A' && c <= (MYWCHAR)'Z')
		return  (WCHAR) (MYWCHAR)'a' + ( c - (MYWCHAR)'A' );
	else
		return lwrSpecialCharacter(c); 
}
WCHAR switchCase(WCHAR c)
{
	if(isLowerCase(c))
		return uprCharacter(c);
	else if(isUpperCase(c))
		return lwrCharacter(c);
	return c;
}
//////////////////////////////////////////////////////////////
BOOL isUpperCase(const MYWCHAR c)
{
	return ((c >= (MYWCHAR)'A' && c <= (MYWCHAR)'Z') || isUCSpecialCharacter(c));
}
////////////////////////////////////////////////////////////////////////////
BOOL isLowerCase(const MYWCHAR c)
{
	return ((c >= (MYWCHAR)'a' && c <= (MYWCHAR)'z') || isLCSpecialCharacter(c));
}
////////////////////////////////////////////////////////////////////////////////
BOOL isCharacter(const MYWCHAR c)
{
	return (isLowerCase(c) || isUpperCase(c));
}
/////////////////////////////////////////////////////////////////////////////////
BOOL isStrictlyLowerCase(const MYWCHAR *word)
{
	const MYWCHAR *chp = word;
	int i = 0;
	while (*chp && i++ < MAX_WORD_LEN)
	{
		if (isLowerCase(*chp) == FALSE)
			return FALSE;
		chp++;
	}
	return TRUE;
}
/////////////////////////////////////////////////////////////////////////////
BOOL isStrictlyUpperCase(const MYWCHAR *word)
{
	const MYWCHAR *chp = word;
	int i = 0;
	while (*chp && i++ < MAX_WORD_LEN) 
	{
		if (isUpperCase(*chp) == FALSE)
			return FALSE;
		chp++;
	}
	return TRUE;
}
///////////////////////////////////////////////////////////////////////////
BOOL isAllLowerCase(const MYWCHAR *word) 
{
	if(word == NULL)
		return FALSE;
	const MYWCHAR *chp = word;
	int i = 0;
	while (*chp && i++ < MAX_WORD_LEN)
	{
		if (isUpperCase(*chp))
			return FALSE;
		chp++;
	}
	return TRUE;
}
//////////////////////////////////////////////////////////////////////
BOOL isAllUpperCase(const MYWCHAR *word)
{
	const MYWCHAR *chp = word;
	int i = 0;
	while (*chp && i++ < MAX_WORD_LEN) 
	{
		if (isLowerCase(*chp))
			return FALSE;
		chp++;
	}
	return TRUE;
}
/////////////////////////////////////////////////////////////////////
BOOL isNamePrediction(MYWCHAR *word)
{
	MYWCHAR *chp = word;
	int i=0;
	while (*chp && i++ < 199) 
	{
		if (*chp == SP && *(chp + 1) && isUpperCase(*(chp + 1)) == TRUE && *(chp + 2) && isLowerCase(*(chp + 2)) == TRUE)
			return TRUE;
		chp++;
	}
	return FALSE;
}
/////////////////////////////////////////////////////////////////
void mywcslwr(MYWCHAR *word)
{
	if(word == NULL)
		return;
	MYWCHAR *chp = word;
	//	wcslwr(word);
	int i=0;
	while (*chp && i++ < MAX_WORD_LEN) 
	{
		if (isUCSpecialCharacter(*chp))
			*chp = lwrSpecialCharacter(*chp);
		else
			*chp = tolower(*chp);
		chp++;
	}
}
/////////////////////////////////////////////////////////////////
void mywcs2lower(MYWCHAR* dest, MYWCHAR *word, int len)
{
//	ShowInfo("mywcs2lower: %s, %d\n", word, len);
	memset(dest, 0, len * sizeof(MYWCHAR));
	if(word == NULL)
		return;
	MYWCHAR *chp = word;
	MYWCHAR *destp = dest;
	int i=0;
	while(*chp && i++<len)
	{
		if(isUpperCase(*chp))
			*destp = lwrCharacter(*chp);
		else
			*destp = *chp;
		chp++;
		destp++;
	}
	*destp = NUL;
}
///////////////////////////////////////////////////////////////
void mywcsupr(MYWCHAR *word)
{
	if(word == NULL)
		return;
	WCHAR *chp = word;
	//	wcsupr(word);
	int i=0;
	while (*chp && i++ < MAX_WORD_LEN) 
	{
		if (isLCSpecialCharacter(*chp))
			*chp = uprSpecialCharacter(*chp);
		else
			*chp = toupper(*chp);
		chp++;
	}
}
////////////////////////////////////////////////////////////////
void smartLowerCaseWithPunctuation(MYWCHAR *word)
{
	int punctionIdx = -10000;
	MYWCHAR LCword[MAX_WORD_LEN*2];
	mywcscpy(LCword, word);

	//	wcslwr(LCword);
	for (int i = 0; word[i]; i++)
	{
		LCword[i] = tolower(LCword[i]);
		if (word[i] == LCword[i] && isPunctuation(word[i])) 
		{
			punctionIdx = i;
			continue;
		}
		if (LCword[i] != word[i] && punctionIdx != (i - 1))
			word[i] = LCword[i];
		else if (isUCSpecialCharacter(word[i]) && punctionIdx != (i - 1))
			word[i] = lwrSpecialCharacter(word[i]);
	}
}
///////////////////////////////////////////////////////////////////////
BOOL isPunctuation(MYWCHAR c)
{
	//this should not be punctuation...|| c == (MYWCHAR) '-' 

	if(c == APOSTROPHE || c == HASHTAG) //apostrophe  0x0027 
	{
		return FALSE;
	}

	if(findPunctuationChoice(c))
		return TRUE;
	if (  c == (MYWCHAR) '.' || c == (MYWCHAR) ';' || c == (MYWCHAR) ':' ||
		  c == (MYWCHAR) ',' || c == (MYWCHAR) '_' ||
		  c == (MYWCHAR) '+' || c == (MYWCHAR) '(' || c == (MYWCHAR) ')' ||
		  c == (MYWCHAR) '{' || c == (MYWCHAR) '}' || c == (MYWCHAR) '[' ||
		  c == (MYWCHAR) ']' || c == (MYWCHAR) ']' ||
		  c == (MYWCHAR) '"' || c == (MYWCHAR) '/' ||
		  c == (MYWCHAR) '?' || c == (MYWCHAR) '!' || 
		  c == (MYWCHAR) L'¿' || c == (MYWCHAR) L'¡' || // inverted question mark 0x00bf, inverted exclamation mark 0x00a1
		  c == (MYWCHAR) L'”'|| c == (MYWCHAR) L'“' || c == (MYWCHAR)L'„' || // german quotations 0x201d and 0x201e
		  c == (MYWCHAR) L'«' || c == (MYWCHAR) L'»' // spanish italian, french quotations
	)
		return TRUE;
	
	return FALSE;
}

////////////////////////////////////////////////////////////////////////
BOOL isNotText(WCHAR c)
{
	if(c==NUL || isPunctuation(c) || SP_CR_TAB(c))
		return TRUE;
	return FALSE;
}

////////////////////////////////////////////////////////////////////////
#ifdef DEBUG
void DebugBreak(char *text) 
{
	ShowInfo("!!ERROR!!! Segmentation error at %s happened!! On Windows this will crash, on Android just hope for the best!!\n", text);
#ifndef _WINDOWS
#ifndef __APPLE__
	__android_log_print(ANDROID_LOG_ERROR, "WLKB", "%s", text);
#endif
#endif
	assert(0);
}
#endif
///////////////////////////////////////////////////////////////////////
int mywcscmp(MYWCHAR *s1, MYWCHAR *s2) 
{
	if(!s1 || !s2)
		return -1;
	int len = mywcslen(s1);
	if(len != mywcslen(s2))
		return 1;
	WLBreakIf(len> 149, "!!ERROR! mywcscmp! Words #%s# or #%s# is too long!\n", toA(s1), toA(s2));
	int i = 0;
	for (i = 0; s1[i] && i<len; i++)
	{
		if (s1[i] != s2[i])
			return 1;
	}
	return 0;
}
/////////////////////////////////////////////////////////////////////////////
int mywcsnicmp(MYWCHAR *s1, MYWCHAR *s2, int n)
{
	if(!s1 || !s2)
		return -1;
	int i = 0;
	for (i = 0; s1[i]; i++)
	{
		short s1upper = toupper(s1[i]);
		if (s1[i] != s2[i] && s1upper != s2[i])
			return 1;
	}
	if (s2[i])
		return 1;
	return 0;
}
/////////////////////////////////////////////////////////////////////////////
int mywcscat(MYWCHAR *s1, MYWCHAR *s2)
{
	WLBreakIf(!s1 || !s2, "!mywcscat error!");
	int s1len = mywcslen(s1);
	return mywcscpy(&s1[s1len], s2);
}
/////////////////////////////////////////////////////////////////////////////
void mywcsncat(WCHAR *s1, WCHAR *s2, int n)
{
	WLBreakIf(!s1 || !s2, "!mywcsncat error!");
	int s1len = mywcslen(s1);
	mywcsncpy(&s1[s1len], s2, n);
}
/////////////////////////////////////////////////////////////////////////////
int mywcsncmp(MYWCHAR *s1, MYWCHAR *s2, int len) 
{
	if(!s1 || !s2)
		return -1;
	int i = 0;
	for (i = 0; s1[i] && i < len; i++)
	{
		if (s1[i] != s2[i])
			return 1;
	}
	if (i < len && s2[i])
		return 1;
	return 0;
}
/////////////////////////////////////////////////////////////////////////////
MYWCHAR* mywcsstr(MYWCHAR *s1, MYWCHAR *s2)
{
	if(!s1 || !s2)
		return NULL;
	int len = mywcslen(s1);
	int len1 = mywcslen(s2);
    for(int i=0; i<len; i++)
	{
		if(mywcsncmp(&s1[i], s2, len1)==0)
			return &s1[i];
	}
	return NULL;
}
/////////////////////////////////////////////////////////////////////////////
int mywcscpy(MYWCHAR *s1, MYWCHAR *s2)
{
	if(!s1 || !s2)
		return 0;
	int i = 0;
	for (i = 0; s2[i] && i<199; i++)
	{
		s1[i] = s2[i];
	}
	s1[i] = NUL;
	return i;
}
/////////////////////////////////////////////////////////////////////////////
void mywcsncpy(MYWCHAR *s1, MYWCHAR *s2, int n)
{
	if(!s1 || !s2 || n==0)
		return;
	for (int i = 0; i < n; i++)
		s1[i] = s2[i];
	s1[n] = NUL;
}
/////////////////////////////////////////////////////////////////////////////
int mywcslen(const MYWCHAR *s) 
{
	if(!s)
		return 0;
	
	int len = 0;
	while (s[len] && len < 199)
		len++;
	return len;
}
//////////quick  sorting for small array of N integers/////////////////////////////
void sortN(int *d, const int N)
{
	int i, j;
	for (i = 1; i < N; i++) {
		int tmp = d[i];
		for (j = i; j >= 1 && tmp < d[j - 1]; j--)
			d[j] = d[j - 1];
		d[j] = tmp;
	}
}
/////////////////////////////////////////////////////////////////////////////
void DeleteFile(char *fileName) 
{
#ifdef _WINDOWS
	_unlink(fileName);
#else
	unlink(fileName);
#endif
}
/////////////////////////////////////////////////////////////////////////////
bool fileAvailable(char *dictNameP)
{
	if(!dictNameP)
		return FALSE;

	FILE * hFile = fopen(dictNameP, "r");
//	ShowInfo("fileAvailable: %s ? %d \n", fullPathFileName, (hFile != NULL));
	return (hFile && !fclose(hFile));
}
/////////////////////////////////////////////////////////////////////////////
int mystricmp(char *a, char *b)
{
	if(!a || !b) return -1;
	size_t len = min(strlen(a), strlen(b));
	len = min(len, 149);
	int i = 0;
	for (i = 0; a[i] && i<len; ++i)
	{
		if (!b[i])
			return 1;
		if (tolower(a[i]) != tolower(b[i]))
			return 1;
	}
	if (b[i])
		return 1;
	return 0;
}
/////////////////////////////////////////////////////////////////////////////
int trimEndingSpaces(MYWCHAR *text, bool bLower)
{
	size_t len = mywcslen((const MYWCHAR *)text);
	int count = 0;
	while (len > 0) 
	{
		if (text[len-1] == SP || text[len-1]==HTAB) 
		{
			count++;
			text[--len] = NUL;
		}
		else
			break;
	}
	if(text[0]==SP || text[0] == HTAB)
	{
		text = &text[1];
		count++;
	}

	if(bLower)
		MakeLowerCase(text);

	return count;
}

////////////////////////////////////////////////////////////////////////////////
MYWCHAR* GetLastWordFromPhrase(MYWCHAR *text)
{
	if(HasEndingSpace(text))
	{
		MYWCHAR inputText[MAX_PHRASE_LEN];
		mywcscpy(inputText,text);
		trimEndingSpaces(inputText);
		text = inputText;
	}

	int idx = GetLastSPIndexInPhrase(text);
	if(idx == 0)
		return text;
	

	static MYWCHAR lastWord[MAX_WORD_LEN];
	mywcscpy(lastWord, &text[idx]);

	//ShowInfo("MK GetLastWordFromPhrase:(%s)\n\n",toA(lastWord));
	return lastWord;
}

/////////////////////////////////////////////////////////////////////////////
MYWCHAR* DeleteLastWordFromPhrase(MYWCHAR* phrase, bool bReturn)
{
	static MYWCHAR deletedWord[MAX_WORD_LEN];

	int len = mywcslen(phrase);
	int index =0;
	for(int i=len-1; i >=0; i--)
	{
		if(phrase[i] == SP)
			break;
	
		if(bReturn)
			deletedWord[index++] = phrase[i];

		phrase[i] = NUL;
	}
	deletedWord[index] = NUL;
	if(index > 0)
		myReverseStr(deletedWord);

	return deletedWord;
}
/////////////////////////////////////////////////////////////////////////////
void AddEndSpace(MYWCHAR* text)
{
	if(!text)
		return;

	int n = mywcslen(text);
	if(n == 0)
		return;

	if(text[n-1] == SP)// if last letter is space, DO NOT ADD END SPACE.
		return;

	text[n] = SP;
}
/////////////////////////////////////////////////////////////////////////////
void MakeLowerCase(MYWCHAR *text)
{
	int len = mywcslen((const MYWCHAR *)text);

	for(int i=0; i < len;i++)
	{
		if(isUpperCase(text[i]))
			text[i] =lwrCharacter(text[i]);	
	}
}

/////////////////////////////////////////////////////////////////////////////
void MakeUpperCase(MYWCHAR *text)
{
	int len = mywcslen((const MYWCHAR *)text);

	for(int i=0; i < len;i++)
	{
		if(isLowerCase(text[i]))
			text[i] =uprCharacter(text[i]);	
	}
}
/////////////////////////////////////////////////////////////////////////////
int GetSpaceCount(MYWCHAR *text)
{
	int spCount=0;

	int n = mywcslen(text);
	for(int i=0; i < n;i++)
	{
		if(text[i] == SP)
			spCount++;	
	}

	return spCount;
}

/////////////////////////////////////////////////////////////////////////////
// Used to construct the last word in the phrase.
// returns 0 if this phrase contains only one word, otherwise returns the index of beginning of last word.
/////////////////////////////////////////////////////////////////////////////
int GetLastSPIndexInPhrase(MYWCHAR *phrase)
{
	int n = mywcslen(phrase);
	while(n > 0 && SP_CR_TAB(phrase[n-1]))
	{	
		phrase[n-1] = NUL;
		n--;
	}
	
	while(n > 0 && SP_CR_TAB(phrase[0]))
	{	
		phrase = &phrase[1];
		n--;
	}

	int i = n-1;
	while(i>=0 && phrase[i]!=SP)
	{
		i--;
	}

    return i==0 ? 0 : i+1;
}

/////////////////////////////////////////////////////////////////////////////
bool IsOrdinalNumbers(MYWCHAR* wordInput)
{
	if(!wordInput)
		return false;
	
	int n = mywcslen(wordInput);
	if(n < 2)
		return false;
	
	bool bOrdinal = true;

	MYWCHAR letters[MAX_WORD_LEN];
	memset(letters,0,sizeof(letters));
	letters[0] = wordInput[n-2];
	letters[1] = wordInput[n-1];

	if(mywcscmp(letters, toW("st")) == 0 || mywcscmp(letters, toW("nd")) == 0 || mywcscmp(letters, toW("rd")) == 0
		|| mywcscmp(letters, toW("th")) == 0)
	{
		for(int i=0; i < n-2;i++)
		{
			if(!isNumberChar(wordInput[i]))
				bOrdinal = false;		
		}

	}
	else
	{
		bOrdinal = false;
	}

	//ShowInfo("MK IsOrdinalNumbers(%d)\n\n",bOrdinal);
	return bOrdinal;
}
/////////////////////////////////////////////////////////////////////////////
bool HasEndingSpace(MYWCHAR* wordInput)
{
	int n = mywcslen(wordInput);
	if(n > 0)
	{
		if(wordInput[n-1] == SP)
			return true;	
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////
bool IsThisEmail(MYWCHAR* wordInput)
{
	for(int i=0; i < MAX_WORD_LEN && wordInput[i]!=NUL; i++)
	{
		if(wordInput[i] == AT)
			return true;		
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////
bool IsThisEmail2(MYWCHAR* wordInput)
{
	int n = mywcslen(wordInput);
	int nCnt1 = 0, nCnt2 = 0;

	int lenAfterDOT = 0;

	bool bStartCount = false;
	for(int i=0; i < n; i++)
	{
		if(bStartCount)
		{
			if(wordInput[i] != NUL && wordInput[i] != DOT)
				lenAfterDOT++;
			

			if(wordInput[i] == DOT)
			{
				bStartCount = false;
				if(lenAfterDOT < 2)
					return false;	
			}
		}

		if(wordInput[i] == AT)
			nCnt1++;
		
		if(wordInput[i] == DOT)
		{
			bStartCount  = true;
			lenAfterDOT =0;
			nCnt2++;
		}
	}

	if(nCnt1 == 1 && nCnt2 >= 1 && lenAfterDOT > 1)//@yahoo.co.kr
		return true;
	
	return false;

}
/////////////////////////////////////////////////////////////////////////////
int	 myStrToInt(MYWCHAR *pNumbers)
{
	WLBreakIf(!pNumbers,"!!ERROR!! pNumbers is NULL!!!\n");
	int n = mywcslen(pNumbers);
	bool bNegative = false;
	int endPos = 0;
	if(pNumbers[0] == '-')
	{
		bNegative = true;
		endPos++;
	}
	int output = 0;
	//int index =0;
	int multiplier = 1;
	for(int i = n-1; i >= endPos; i--)
	{
		int num = pNumbers[i]-'0';
		output += (num * multiplier);
		multiplier *= 10;
	}
	
	if(bNegative)
		output = output * -1;
	
	return output;
}
/////////////////////////////////////////////////////////////////////////////////////
int GetAlphabetIndex(MYWCHAR letter)
{
	int num = -1;
	if(isNumberChar(letter))	
		return num = MAX_ALPHABET_SIZE-2;
	
	
	if(isLowerCase(letter))
	{
		if(letter >='a' || letter <='z')
			num= letter-'a';	
	}
	else
	{
		if(letter >= 'A' && letter <= 'Z')
			num= letter-'A';	
	}

	if(num >= MAX_ALPHABET_SIZE || num < 0) //non-English char..$ or René
		return MAX_ALPHABET_SIZE-1;
	
	return num;
}
/////////////////////////////////////////////////////////////////////////////
void myReverseStr(MYWCHAR *s) 
{
    MYWCHAR t, *d = &(s[mywcslen(s) - 1]);
    while (d > s) 
	{
        t = *s;
        *s++ = *d;
        *d-- = t;
    }
}
/////////////////////////////////////////////////////////////////////////////
void ReplaceiToI(MYWCHAR* text)
{
	if(mywcscmp(text, toW("i")) == 0)
		text[0] = uprCharacter(text[0]);

	for(int i=0; i < MAX_WORD_LEN && text[i]; i++)
	{
		if(text[i] == 'i' && text[i+1] && text[i+1] == APOSTROPHE)
			text[i] = uprCharacter(text[i]);
	}

}
/////////////////////////////////////////////////////////////////////////////
double Diffclock(clock_t clock1,clock_t clock2)
{
    double diffticks=clock1-clock2;
    double diffms=(diffticks)/(CLOCKS_PER_SEC/1000);
    return diffms;
}
/////////////////////////////////////////////////////////////////////////////
BOOL FileExists(char *filename)
{
	FILE * hFile = fopen(filename, "r");
	return (hFile && !fclose(hFile));
}
/////////////////////////////////////////////////////////////////////////////
DWORD MyGetFileSize(char *filename)
{
	DWORD dwFileSize = 0;

	FILE * hFile = fopen(filename, "r");
	if (hFile)
	{
		dwFileSize = getFileSize(hFile);
		fclose(hFile);
	}
	return dwFileSize;
}
/////////////////////////////////////////////////////////////////////////////
#define MWC MYWCHAR
MYWCHAR aRow[]={(MWC)'a', (MWC)'A', (MWC)'@',(MWC)0xE0,(MWC)0xE1,(MWC)0xE2,(MWC)0xE3,(MWC)0xE4,(MWC)0xE5,(MWC)0xE6,(MWC)0xC0, (MWC)0xC1,(MWC)0xC2,(MWC)0xC3,(MWC)0xC4,(MWC)0xC5,(MWC)0xC6,(MWC)0xAA, NUL};  // a, A, @, à, á, â, ã, ä, å, æ, À, Á, Â, Ã, Ä, Å, Æ, ª
MYWCHAR bRow[]={(MWC)'b', (MWC)'B', (MWC)0xDF, NUL};  // b, B, ß
MYWCHAR cRow[]={(MWC)'c', (MWC)'C', (MWC)0xE7, (MWC)0xC7, (MWC)0xA9, NUL};   // c, C, ç, Ç, ©
MYWCHAR dRow[]={(MWC)'d', (MWC)'D', NUL};  
MYWCHAR eRow[]={(MWC)'e', (MWC)'E', (MWC)0xE8, (MWC)0xE9, (MWC)0xEA, (MWC)0xEB, NUL};  // e, E, è, é, ê, ë
MYWCHAR fRow[]={(MWC)'f', (MWC)'F', NUL};  
MYWCHAR gRow[]={(MWC)'g', (MWC)'G', NUL};  
MYWCHAR hRow[]={(MWC)'h', (MWC)'H', NUL};  
MYWCHAR iRow[]={(MWC)'i', (MWC)'I', (MWC)0xCC, (MWC)0xCD, (MWC)0xCE, (MWC)0xCF, (MWC)0xEC, (MWC)0xED, (MWC)0xEE, (MWC)0xEF, NUL};  // i, I, Ì, Í, Î, Ï, ì, í, î, ï
MYWCHAR jRow[]={(MWC)'j', (MWC)'J', NUL};  
MYWCHAR kRow[]={(MWC)'k', (MWC)'K', NUL};  
MYWCHAR lRow[]={(MWC)'l', (MWC)'L', NUL};  // l, L, 
MYWCHAR mRow[]={(MWC)'m', (MWC)'M', (MWC)0xB5, NUL};  // m, M, µ
MYWCHAR nRow[]={(MWC)'n', (MWC)'N', (MWC)0xD1, (MWC)0xF1, NUL};  //n, N, Ñ, ñ
MYWCHAR oRow[]={(MWC)'o', (MWC)'O', (MWC)0xF2, (MWC)0xF3, (MWC)0xF4, (MWC)0xF5, (MWC)0xF6, (MWC)0xD2, (MWC)0xD3, (MWC)0xD4, (MWC)0xD5, (MWC)0xD6, (MWC)0xD8, (MWC)0xF8, (MWC)0x8C, (MWC)0x9C, NUL};  // o, O, ò, ó, ô, õ, ö, Ò, Ó, Ô, Õ, Ö, Ø, ø, Œ, œ
MYWCHAR pRow[]={(MWC)'p', (MWC)'P', NUL};  
MYWCHAR qRow[]={(MWC)'q', (MWC)'Q', NUL};  
MYWCHAR rRow[]={(MWC)'r', (MWC)'R', (MWC)0xAE, NUL};  // r, R, ®
MYWCHAR sRow[]={(MWC)'s', (MWC)'S', (MWC)0xDF, NUL};  // s, S, ß, 
MYWCHAR tRow[]={(MWC)'t', (MWC)'T', NUL};
MYWCHAR uRow[]={(MWC)'u', (MWC)'U', (MWC)0xF9, (MWC)0xFA, (MWC)0xFB, (MWC)0xFC, (MWC)0xD9, (MWC)0xDA, (MWC)0xDB, (MWC)0xDC, NUL};  // u, U, ù, ú, û, ü, Ù, Ú, Û, Ü
MYWCHAR vRow[]={(MWC)'v', (MWC)'V', NUL};  
MYWCHAR wRow[]={(MWC)'w', (MWC)'W', NUL};  
MYWCHAR xRow[]={(MWC)'x', (MWC)'X', NUL};  
MYWCHAR yRow[]={(MWC)'y', (MWC)'Y', (MWC)0xFD, (MWC)0xFF, (MWC)0xA5, NUL};  // y, Y, ý, ÿ, ¥
MYWCHAR zRow[]={(MWC)'z', (MWC)'Z', (MWC)0x8E, (MWC)0x9E, NUL}; // z, Z, Ž, ž
MYWCHAR *gAccentedChars[26] = {aRow,bRow,cRow,dRow,eRow,fRow,gRow,hRow,iRow,jRow,kRow,lRow,mRow,nRow,oRow,pRow,qRow,rRow,sRow,tRow,uRow,vRow,wRow,xRow,yRow,zRow};
///////////////////////////////////////////////////////////////////////////

// for each character and a language, returns a list of possible accented versions, starting with character itself
// returns list of possible variations of letter, mostly its different accent flavors and such.
MYWCHAR *GetCharCases(MYWCHAR c, eLanguage lang)
{
	static MYWCHAR defaultCharCases[3];
	MYWCHAR lc = lwrCharacter(c);
	// default case which is english
	defaultCharCases[0] = lc;
	defaultCharCases[1] = uprCharacter(c);
	if(defaultCharCases[1]==lc)
		defaultCharCases[1] = NUL;
	defaultCharCases[2] = NUL;

	switch(lang)
	{
		case eLang_ENGLISH:
		case eLang_BRITISH:
			return defaultCharCases;
		default:
			if( lc >= (MYWCHAR)'a' && lc <= (MYWCHAR)'z')
				return gAccentedChars[lc - (MYWCHAR)'a'];
	}

	 return defaultCharCases;
}
// returns accent free version of a char.
MYWCHAR GetCharRowBase(MYWCHAR ch)
{
	for(int i=0; i<26; i++)
	{
		MYWCHAR *row = gAccentedChars[i];
		for(int j=0; row[j]; j++)
			if(row[j] ==  ch)
				return row[0];
	}
	return ch;
}
/////////////////////////////////////////////////////////////////////////////
eLanguage findLanguage(char *filename)
{
	char *lowname = mytolower(filename);
	if(strstr(lowname, "french"))
		return eLang_FRENCH;
	else if(strstr(lowname, "german"))
		return eLang_GERMAN;
	else if(strstr(lowname, "portuguese"))
		return eLang_PORTUGUESE;
	else if(strstr(lowname, "spanish"))
		return eLang_SPANISH;
	else if(strstr(lowname, "italian"))
		return eLang_ITALIAN;
	else if(strstr(lowname, "englishuk"))
		return eLang_BRITISH;
	else if(strstr(lowname, "english"))
		return eLang_ENGLISH;
	else if(strstr(lowname, "medical"))
		return eLang_MEDICAL;
	else if(strstr(lowname, "korean"))
		return eLang_KOREAN;
	else if(strstr(lowname, "arabic"))
		return eLang_ARABIC;
	return eLang_DEFAULT;
}
/////////////////////////////////////////////////////////////////////////////
char *getDictionaryname(eLanguage elang)
{
	switch(elang)
	{
	case eLang_PERSONAL:
		return (char *)"personal";
	case eLang_ENGLISH:
		return (char *)"english";
	case eLang_FRENCH:
		return (char *)"french";
	case eLang_GERMAN:
		return (char *)"german";
	case eLang_PORTUGUESE:
		return (char *)"portuguese";
	case eLang_SPANISH:
		return (char *)"spanish";
	case eLang_ITALIAN:
		return (char *)"italian";
	case eLang_BRITISH:
		return (char *)"englishuk";
	case eLang_MEDICAL:
		return (char *)"medical";
	case eLang_KOREAN:
		return (char *)"korean";
	case eLang_ARABIC:
		return (char *)"arabic";
	case eLang_RUSSIAN:
		return (char *)"russian";
	case eLang_DEFAULT:
		return (char *)"default";
	default:
		return (char *)"notset";
	}
}
/////////////////////////////////////////////////////////////////////////////
char *mytolower(char *str, int span)
{
	static char lowerstr[MAX_WORD_LEN];
	if(!str)
		return NULL;
	int len = span > 0 ? span:(int)strlen(str);
	strncpy(lowerstr, str, len);
	for(int i=0; i<len; i++)
		lowerstr[i] = tolower(str[i]);
	lowerstr[len] = '\0';
	return lowerstr;
}
/////////////////////////////////////////////////////////////////////////////
// get rid of line feeds by repacking the characters
int stripOutLineFeedAndUnicodePunctuation(WCHAR *myw, int wLen)
{
	int j=0;
	for (int i=0 ; i  < wLen; i++)
	{
		if (myw[i] == (WCHAR) 0x0a)
			continue;
		switch(myw[i])
		{
		case 0x2013: // unicode en dash
			myw[i] = (WCHAR) '-';
			break;
		case 0x2014: // unicode en dash
			myw[i] = (WCHAR) '-';
			break;
		case 0x00ad: // soft hyphen, make a hard one out of it
			myw[i] = (WCHAR) '-';
			break;
		case 0x2018: // unicode left single quotation mark
			myw[i] = (WCHAR) '\'';
			break;
		case 0x2019: // unicode right single quotation mark
			myw[i] = (WCHAR) '\'';
			break;
		case 0x201c: // unicode left double quotation
			myw[i] = (WCHAR) '\"';
			break;
		case 0x201d: // unicode right double quotation
			myw[i] = (WCHAR) '\"';
			break;
		case 0x201e: // unicode double low-9 quotation mark
			myw[i] = (WCHAR) '\"';
			break;
		}     
		if (i != j) 
			myw[j] = myw[i];
		
		j++;
	}
	myw[j] = 0;
	return j;
}
/////////////////////////////////////////////////////////////////////////////
CPPExternClose
