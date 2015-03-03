#ifndef _WLTYPES_H
#define _WLTYPES_H

#include <stddef.h>

//#ifndef NULL
//#ifdef __cplusplus
//#define NULL    0
//#else
//#define NULL    ((void *)0)
//#endif
//#endif

#ifdef MYDLL_EXPORT
#define OutputDebugStringA(s) printf(s)
#endif

#ifdef _WINDOWS
#undef WCHAR
typedef wchar_t WCHAR;
#define MYWCHAR WCHAR
#else
#ifdef __LP64__
typedef long intptr_t;
#else
#include <stdint.h>
#endif

typedef unsigned short WCHAR;    // wc,   16-bit UNICODE character
//typedef char16_t WCHAR;
#define MYWCHAR WCHAR
//#define MYWCHAR short
//#define TEXT(a)	(char *)(a)
#endif // _WINDOWS

#ifdef _WIN64
#undef NULL
#define NULL nullptr
#endif

//#ifndef _WINDOWS   // this is for Android and iOS
//typedef	int intptr_t;
//typedef unsigned uintptr_t;
//#endif 

#define BOOL 	bool
#define BYTE 	unsigned char
#define DWORD   unsigned int
#define UINT	unsigned int
#define USHORT  unsigned short
#define GENPTR	uintptr_t

#define INT		int
#ifdef TRUE
#undef TRUE
#undef FALSE
#endif
#define TRUE	true
#define FALSE	false

typedef unsigned char byte;
#define MB	(1024*1024)

// byte encoding and Platform configuration
enum  eEndianNess
{
	eLITTLE_ENDIAN32	= 0,
	eBIG_ENDIAN32		= 1,
	eLITTLE_ENDIAN64	= 2,
	eBIG_ENDIAN64		= 3
};


// some ascii char values needed 
#define HTAB	(MYWCHAR)'\t'	// 0x09 ascii horizontal tab
#define LF		(MYWCHAR)'\n'	// 0x0a ascii Line Feed
#define CR		(MYWCHAR)'\r'	// 0x0d carriage return
#define SP		(MYWCHAR) ' '	// space
#define BS		(MYWCHAR) '\b' // 0x08 backspace
#define NUL     (MYWCHAR) '\0' // 0x00 NUL
#define DOT		(MYWCHAR) '.'
#define COMMA   (MYWCHAR) ','
#define COLON	(MYWCHAR) ':'
#define APOSTROPHE (MYWCHAR)'\''
#define RIGHT_ARROW					(MYWCHAR)  0x2192
#define CELSIUS						(MYWCHAR)  0x2103
#define FAHRENHEIT					(MYWCHAR)  0x2109
//Minkyu:2014.01.20
//1.TILDE
//2.ASTERISK
//3.CARET
//4. AT
//THIS ORDER MUST BE KEPT IN DICTIONARY.txt 
//#define TILDE	(MYWCHAR)'~'   //indicator that this word can be used as noun.
#define ASTERISK (MYWCHAR)'*'  //indicator that this word cannot be a chunk 
#define CARET   (MYWCHAR)'^'	//indicator that this word has frequency as a starting word
#define AT		(MYWCHAR) '@'   //For Phrase predictions, indicator that this phrease has 2 gram phrase
#define HASHTAG (MYWCHAR) '#'

const MYWCHAR SEmptyStr[1] = {NUL};
#define EMPTYSTRING const_cast<MYWCHAR*>(&SEmptyStr[0])

#define NOT_SP_CR_TAB(letter) ((letter != SP) && (letter != CR) && (letter != HTAB) && (letter != LF))
#define SP_CR_TAB(letter) ((letter == SP) || (letter == CR) || (letter == HTAB) || (letter == LF))
#define CR_TAB_LF(letter) ((letter == CR) || (letter == HTAB) || (letter == LF))

#ifndef _WINDOWS
#define _snprintf snprintf
#endif

#ifndef MAX_PATH
#define MAX_PATH	260
#endif

#define DEBUG_TAG "WL_TAG_D"
#define MAX_WORD_LEN					64
#define MAX_TGRAPH_HISTORY_NUM			4
#define MAX_PHRASE_LEN					(MAX_WORD_LEN*MAX_TGRAPH_HISTORY_NUM)
#define MAX_PHRASE_ARRAY				1600		
#define MAX_NGRAM_LEARNING				300
#define MAX_FOUND_PHRASES				255
#define MAX_TGRAPH_SIZE					10

#define MAX_START_WORD_CACHE_SIZE		500

#define START_PREF_INCREMENT			8
#define LEARNED_START_PREF				10
#define MAX_START_PREF					0xFD
#define N1Gram							1
#define N2Gram							2
#define N3Gram							3
#define N4Gram							4
#define MAX_POSTAG_NAME_SIZE			4
#define MAX_ALPHABET_SIZE				28		//index 0~25 for alphabet, 26 is for the word that has a number at the end, 27 is for non-English alphabet.
#define MAX_PHRASE_ALLOWED				5

enum eLanguage
{
	eLang_NOTSET	=  -1,
	eLang_PERSONAL	=	0,
	eLang_ENGLISH	=	1,
	eLang_FRENCH	=	2,
	eLang_SPANISH	=	3,
	eLang_GERMAN	=	4,
	eLang_PORTUGUESE=	5,
	eLang_DUTCH		=	6,
	eLang_ITALIAN	=	7,
	eLang_AUSTRIA	=	8,
	eLang_BRITISH	=	9,
	eLang_KOREAN	=	10,
	eLang_MEDICAL	=	11,
	eLang_ARABIC	=	12,
	eLang_RUSSIAN	=	13,
	eLang_DEFAULT	=	14,  // used  for specialized languages like medical or such for now.
	eLang_COUNT		=	15
};



#endif
