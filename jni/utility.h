/**************************************************************************

   File:          Utility.h
   
   Description:   Utility definitions.

**************************************************************************/

#ifndef UTILITY_H
#define UTILITY_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __APPLE__
#include "__ios_log_print.h"
#endif

/// define for cpp extern cross platform
//#ifndef _WINDOWS
#ifdef __APPLE__
	#define CPPExternOpen		//extern "C" {
#else // __APPLE__
#define CPPExternOpen	//extern "C" {		
#endif // __APPLE__
//#else
//	#define CPPExternOpen
//#endif  // !WINDOWS

//#ifndef _WINDOWS
#ifdef __APPLE__
	#define CPPExternClose		//}
#else //__APPLE__
	#define CPPExternClose		//}					
#endif // __APPLE__
//#else
//	#define CPPExternClose
//#endif  // !WINDOWS

/**************************************************************************
   #include statements
**************************************************************************/
#if defined(_DEBUG) //|| defined(NDEBUG)
#ifndef DEBUG
#define DEBUG
#endif
#endif

#ifdef _DEBUG
#ifndef  _LOGGING
#define _LOGGING
#endif
#endif
	
#include <assert.h>

CPPExternOpen

#ifndef _WINDOWS
	#define _snprintf_s _snprintf

	#ifdef NDK_DEBUG
		#define _DEBUG
		#define DEBUG
	#endif
	#ifdef DEBUG
		#ifndef __APPLE__
			#include <android/log.h>
			#define printf(...) __android_log_print(ANDROID_LOG_DEBUG, "WLLIB_PrintF:", __VA_ARGS__);
			//#define assert(test) (ShowInfoIf(!(test), (__FILE__)))
		#endif
	#else
		#define print(...)
	//	#define assert(test)
	#endif
#endif // !_WINDOWS

#include "wltypes.h"
#include <time.h>

#define NSIZE    2048

extern char *g_szStartDirectory;

const BYTE MAX_NUM_PREDICTIONS = 5; // max number of predictions engine provides each time.

unsigned long djb2_hash(MYWCHAR *str);

char char2A(WCHAR wc);
MYWCHAR char2W(char c);

char *toA(WCHAR *word);
WCHAR *toW(const char *asciword);

void wToA(WCHAR *word, char *ansiWord, unsigned size);
void AToW(char *astr, WCHAR* wstr); // assumes wstr has been allocated already
MYWCHAR* makeWord(MYWCHAR *word);

#if defined(_LOGGING)
extern FILE *gLogFile;
void CloseLogFile();
#else
#define CloseLogFile()
#endif

unsigned myClamp(unsigned val, unsigned min, unsigned max, unsigned range);

typedef struct WordsArray
{
	MYWCHAR words[MAX_NUM_PREDICTIONS][MAX_WORD_LEN];
	MYWCHAR *nextWords[MAX_NUM_PREDICTIONS];
	void reset();
} WordsArray;

BOOL FileExists(char *filename);
eLanguage findLanguage(char *filename);
char *getDictionaryname(eLanguage elang);

//WCHAR *fillClipboardData(HWND hWnd, WCHAR *clipBuf, int *clipLenP);
size_t WriteToFile(FILE *fp, char *address, int size);
BOOL readFromFile(FILE *fp, char *address, int size);
int getFileSize(FILE *fp);
DWORD MyGetFileSize(char *filename);
intptr_t queryValueAtOffset(char *p, unsigned size, unsigned offset);
void setValueAtOffset(char *p, int size, unsigned offset, uintptr_t val);
bool wordContainsLetter(MYWCHAR *word, MYWCHAR letter);

char* addOn(char *pathname, char *suffix, char *pathnameWithSuffix);

bool isSeparator(MYWCHAR ch);

void myNCaseTranslation(WCHAR *text, int nChars, BOOL toUpperCase);
BOOL isNumber(WCHAR *word);
char *mytolower(char *str, int len = 0);
WCHAR switchCase(WCHAR c);
inline BOOL isLCSpecialCharacter(MYWCHAR c) {return ((c >= (MYWCHAR)0xdf && c <= (MYWCHAR)0xff) || c == 0x153);} // lower case french oe
inline BOOL isUCSpecialCharacter(MYWCHAR c) { return ((c >= (MYWCHAR)0xc0 && c <= (MYWCHAR)0xde) || c == 0x152);} // upper case french oe
BOOL isCharacter(WCHAR c);
inline BOOL isNumberChar(MYWCHAR c) 
{ 
	return (c >= (MYWCHAR)'0' && c <= (MYWCHAR)'9'); 
}
inline BOOL isEmptyStr(const MYWCHAR *str) { return (!str || str[0] == NUL); }
WCHAR uprCharacter(WCHAR c);
WCHAR lwrCharacter(WCHAR c);
void mywcslwr(WCHAR *word);
void mywcs2lower(WCHAR* dest, WCHAR *word, int len);
void mywcsupr(WCHAR *word);
void smartLowerCaseWithPunctuation(WCHAR *word);
BOOL isPunctuation(WCHAR c);
BOOL isNotText(WCHAR c);

BOOL isUpperCase(const WCHAR c);
BOOL isLowerCase(const WCHAR c);
BOOL isStrictlyLowerCase(const WCHAR *word);
BOOL isStrictlyUpperCase(const WCHAR *word);
BOOL isAllLowerCase(const WCHAR *word);
BOOL isAllUpperCase(const WCHAR *word);
BOOL isNamePrediction(WCHAR *word);
int	 myStrToInt(MYWCHAR *pNumbers);
void myReverseStr(MYWCHAR *s);
MYWCHAR* DeleteLastWordFromPhrase(MYWCHAR* phrase, bool bReturn = false);
void AddEndSpace(MYWCHAR* text);
void ReplaceiToI(MYWCHAR* text);
double Diffclock(clock_t clock1,clock_t clock2);

inline int Mywtoi(WCHAR *text)
{
#ifdef _WINDOWS
	wchar_t *stopwcs;
	return wcstol(text, &stopwcs, 10);
#else
	char c = char2A(*text);
	int val = atoi(&c);
	return val;
#endif
}

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#ifdef DEBUG
void DebugBreak(char*txt);
#define DebugBreakIf(cond, txt) (if(cond){ DebugBreak(txt); return; } )
#else
#define DebugBreak(s)
#define DebugBreakIf(cond, txt)
#endif

#ifndef max
	#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
	#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

int  mywcscmp(WCHAR *s1, WCHAR *s2);
int  mywcsncmp(WCHAR *s1, WCHAR *s2, int len);
WCHAR* mywcsstr(WCHAR *s1, WCHAR *s2);
int  mywcslen(const WCHAR *s);
int  mywcscpy(WCHAR *s1, WCHAR *s2);
int mywcscat(WCHAR *s1, WCHAR *s2);
void mywcsncat(WCHAR *s1, WCHAR *s2, int n);
void mywcsncpy(WCHAR *s1, WCHAR *s2, int n);
int  mywcsnicmp(WCHAR *s1, WCHAR *s2, int n);

void sortN(int *d, const int N);

void DeleteFile(char *fileName);
bool fileAvailable(char *dictNameP);
int mystricmp(char *a, char *b);
int trimEndingSpaces(MYWCHAR *text, bool bLower = false);
MYWCHAR* GetLastWordFromPhrase(MYWCHAR *text);
void MakeLowerCase(MYWCHAR *text);
void MakeUpperCase(MYWCHAR *text);
int GetSpaceCount(MYWCHAR *text);
int GetLastSPIndexInPhrase(MYWCHAR *phrase);
bool IsOrdinalNumbers(MYWCHAR* wordInput);
bool HasEndingSpace(MYWCHAR* wordInput);
bool IsThisEmail(MYWCHAR* wordInput);
bool IsThisEmail2(MYWCHAR* wordInput);
int GetAlphabetIndex(MYWCHAR letter);

#define LOG_TAG "WLLIB"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

eEndianNess testEndianNess();

inline int alignOnLong(int len)
{
	return (len + 3) & (unsigned)(~0x3);
}
 
#ifdef DEBUG
#define WLBreak(fmt, ... ) \
{ \
	char szBuf[400]; \
	_snprintf(szBuf, 400, fmt, ##__VA_ARGS__ ); \
	OutputDebugString( szBuf); \
	assert(0); \
} 
#define WLBreakIf( condition, fmt, ... ) \
{ \
	if((condition) != 0) { \
		char szBuf[400]; \
		_snprintf(szBuf, 400, fmt, ##__VA_ARGS__ ); \
		OutputDebugString( szBuf); \
		assert(0); \
	} \
} 
#define ShowInfo( fmt, ... ) \
	{ \
		char szBuf[400]; \
		_snprintf(szBuf, 400, fmt, ##__VA_ARGS__ ); \
		OutputDebugString( szBuf); \
	} 

#define ShowInfoIf( condition, fmt, ... ) \
{ \
	if((condition) != 0) { \
		char szBuf[400]; \
		_snprintf(szBuf, 400, fmt, ##__VA_ARGS__ ); \
		OutputDebugString( szBuf); \
	} \
} 
#else
#define WLBreak(fmt, ...)
#define WLBreakIf(condtion, fmt, ...)
#define ShowInfo( fmt, ... )
#define ShowInfo( fmt, ... ) \
//{ \
//	char szBuf[400]; \
//	_snprintf(szBuf, 400, fmt, ##__VA_ARGS__ ); \
//} 
#define ShowInfoIf( condtion, fmt, ... )
#endif

inline void turnOnBit(BYTE *flags, int nthBit) {assert (nthBit <= 7);	*flags |=  1 << nthBit;}
inline void turnOffBit(BYTE *flags, int nthBit){assert (nthBit <= 7); *flags &= ~(1 << nthBit);}
inline bool isBitTurnedOn(BYTE flags, int nthBit) {assert (nthBit <= 7); return (flags & (1 << nthBit)) != 0;}
inline void turnOnBit16(USHORT *flags, int nthBit) {assert (nthBit <= 15);	*flags |=  1 << nthBit;}
inline void turnOffBit16(USHORT *flags, int nthBit){assert (nthBit <= 15); *flags &= ~(1 << nthBit);}
inline bool isBitTurnedOn16(USHORT flags, int nthBit) {assert (nthBit <= 15); return (flags & (1 << nthBit)) != 0;}

#ifdef _LOGGING
void sprintfile(char *format, ...);
void dumpMemory(char *addr, int len);
#define SPF0(a)	sprintfile((a))
#else
#define SPF0(a)	
#endif

#ifndef __APPLE__
const int sNumCharsAdded = 3;
const int sNumDegreeAdded = 2;
//static MYWCHAR sPeriods[sNumCharsAdded] = {DOT, DOT, DOT};
//static MYWCHAR sSpaces[sNumCharsAdded] = {SP, SP, SP };
static MYWCHAR sDegree[sNumDegreeAdded] = {CELSIUS, FAHRENHEIT };
#else
const int sNumCharsAdded = 4;
const int sNumDegreeAdded = 3;
//static MYWCHAR sPeriods[sNumCharsAdded] = {DOT, DOT, DOT, NUL};
//static MYWCHAR sSpaces[sNumCharsAdded] = {SP, SP, SP, NUL };
static MYWCHAR sDegree[sNumDegreeAdded] = {CELSIUS, FAHRENHEIT, NUL };
#endif


#ifdef _WINDOWS
#define callLogA(s) OutputDebugStringA(s)
#define callLog(s) OutputDebugStringA(s)
#else
#define callLogA(s)  __android_log_print(ANDROID_LOG_DEBUG, "WKLBInfo", "%s", s)
extern "C" void callLog(const char *s);
#endif

#ifdef _WINDOWS
	#ifdef OutputDebugString
		#undef OutputDebugString
//		#define OutputDebugString OutputDebugStringA
inline void OutputDebugString(char *text) { /*printf(text); */OutputDebugStringA(text); }
	#endif
#else  // _WINDOWS
#ifdef DEBUG
#ifndef __APPLE__
	inline void OutputDebugString(char *text) { SPF0(text);  __android_log_print(ANDROID_LOG_DEBUG, "WKLBInfo", "%s", text);  }
#else
	inline void OutputDebugString(char *text) { SPF0(text); __ios_log_print("WKLBInfo [%s]", text); }; 
#endif
#else
#define OutputDebugString(s)
#endif
#endif

MYWCHAR *GetCharCases(MYWCHAR c, eLanguage lang); // returns all case and accented version of this character, as a null terminated array
MYWCHAR GetCharRowBase(MYWCHAR ch);

struct CompactNode;
struct NextWordInfo
{
	MYWCHAR *text;
	CompactNode *endNode;
	BYTE pref;
};

//////////////////////////// simple heap allocator ////////////////////////////////////////////
struct WLHeap
{
	unsigned heapSize;

	char* start;
	char* clearStart;
    char* end;
    char* max_end;
	WLHeap(): heapSize(0), start(NULL), end(NULL), max_end(NULL) {}
    bool set(unsigned size)
    {
		WLBreakIf(size<=0, "!!ERROR! WLHeap! error in buffer size! it is negative or zero!! (%d)!!\n", size);
		if(start)
		 free(start);
		heapSize = (size + 15) & (unsigned)(~0xf);
        start = (char*)calloc(1, size);
		WLBreakIf(start == NULL, "!!!ERROR! WLHeap::set: failed to allocate memory!\n");
        end = start;
        max_end = start + heapSize;
		clearStart = start;
		return start != NULL;
    }

	void reset()
	{
		assert(start);
		assert(clearStart);
		assert(clearStart >= start);
		end = clearStart;
		memset(clearStart, 0, heapSize-(clearStart-start));
	}
	
	void setClearStart()
	{
		clearStart = end;
	}

	bool belongs(char *address)
	{
		return (address >=start && address < max_end);
	}
    ~WLHeap()
    {
        free(start);
    }

	int available()
	{
		return (int)(max_end - end);
	}

    char* allocate(unsigned bytes)
    {
		bytes = (bytes + 3 ) & ( ~0x3 ); 
		char* output = end;
        end = end + bytes;
        WLBreakIf( end > max_end, "!!ERROR! WLHeap:allocate: out of memory!\n" );
        return output;
    }
};
#if defined(_WINDOWS)
//////////////////////////// virtual heap allocator ////////////////////////////////////////////
#include <winbase.h>
struct WLHeapVirtual
{
	unsigned heapSize;

	char* start;
	char* clearStart;
    char* end;
    char* max_end;

	unsigned winPageSize;
	unsigned pageSize;
	unsigned pageCount;
	unsigned headerSize;
	char *header;
	int currentCommitIdx;
	char *baseAddress;
	WLHeapVirtual(): heapSize(0), start(NULL), end(NULL), max_end(NULL) {}
    bool set(unsigned pagecount, unsigned pagesize, unsigned headersize)
    {
		SYSTEM_INFO sSysInfo;         // Useful information about the system
		GetSystemInfo(&sSysInfo);     // Initialize the structure.
		printf ("This computer has page size %d.\n", sSysInfo.dwPageSize);
		winPageSize = sSysInfo.dwPageSize;
		assert((winPageSize != 0) && ((winPageSize & (~winPageSize + 1)) == winPageSize)); // check winPageSize is power of 2!
		currentCommitIdx = -1;
		unsigned wpageAlignment =  winPageSize-1;
		headerSize = (headersize + wpageAlignment) & (unsigned)(~wpageAlignment);
		pageCount = pagecount;
		pageSize = (pagesize +wpageAlignment) & (unsigned)(~wpageAlignment);
		heapSize = pageSize * pageCount + headerSize;
        baseAddress = (char*)VirtualAlloc(NULL,   // System selects address
									heapSize, // Size of allocation
									MEM_RESERVE,          // Allocate reserved pages
									PAGE_NOACCESS);       // Protection = no access
		if (baseAddress == NULL )
		{
			printf("!!!!!!!!!!!!VirtualAlloc reserve failed.\n");
			return false;
		}
		header = (char*)VirtualAlloc( (LPVOID) baseAddress, // Next page to commit
							 headerSize,				// Page size, in bytes
							 MEM_COMMIT,			// Allocate a committed page
							 PAGE_READWRITE);		// Read/write access
        end = start = header;
       max_end = start + headerSize;
		return baseAddress != NULL;
    }

	bool commit(unsigned pageIdx)
	{
		if(pageIdx == currentCommitIdx)
			return true;
		char *laststart = start;
		start = pageSize * pageIdx + header + headerSize;
		start = (char*)VirtualAlloc( (LPVOID) start, // Next page to commit
							 pageSize,				// Page size, in bytes
							 MEM_COMMIT,			// Allocate a committed page
							 PAGE_READWRITE);		// Read/write access
		if(start == NULL && pageIdx > 0)
		{
			VirtualFree((LPVOID)laststart, pageSize, MEM_DECOMMIT); 
			start = (char*)VirtualAlloc( (LPVOID) start, // Next page to commit
							 pageSize,				// Page size, in bytes
							 MEM_COMMIT,			// Allocate a committed page
							 PAGE_READWRITE);		// Read/write access
		}
		assert(start != NULL);
		currentCommitIdx = pageIdx;
        max_end = start + pageSize;
		end = start;
		clearStart = start;
		return start != NULL;
	}

	void unCommit()
	{
		VirtualFree((LPVOID)start, pageSize, MEM_DECOMMIT);
		end = start = NULL;
		currentCommitIdx = -1;
	}
	void reset(unsigned curIdx, unsigned newIdx)
	{
		commit(newIdx);
		assert(start);
		assert(clearStart);
		assert(clearStart >= start);
		end = clearStart;
		memset(clearStart, 0, heapSize-(clearStart-start));
	}
	
	void setClearStart()
	{
		clearStart = end;
	}

	bool belongs(char *address)
	{
		return (address >=start && address < max_end);
	}
    ~WLHeapVirtual()
    {
		VirtualFree(   (LPVOID)baseAddress,  // Base address of block
                       0,             // Bytes of committed pages
                       MEM_RELEASE);  // Decommit the pages
     }

	int available()
	{
		return (int)(max_end - end);
	}

    char* allocate(unsigned bytes)
    {
		bytes = (bytes + 3 ) & ( ~0x3 ); 
		char* output = end;
        end = end + bytes;
        WLBreakIf( end > max_end, "!!ERROR! WLHeap:allocate: out of memory!\n" );
        return output;
    }
};
#endif

WLHeap *getHistHeap(int idx);
bool BelongToHistHeap(int heapIdx, char *address);
WLHeap *getWorkingHeap(int idx);
bool BelongToWorkingHeap(int heapIdx, char *address);
//////////////////////////////////////////////////////////////////////////////////////////////////////////
// this provides an static array container to be used for max number of elements < MAX_SHORT 
#define EMPTY 'E' // F means slot in array is full, E means slot is empty i.e. not taken!
#define FULL 'F'
struct WLArray
{
	unsigned numElements;
	USHORT elementSize;
	WLHeap heap;
	char **elements;
	char *flags; 
	unsigned curEmptyCount; //number of slots currently empty in array
	void set(unsigned num, unsigned size)
	{
		ShowInfo("Cache Array set: numElements=%d, elementSize=%d\n", num, size);
		numElements = (num + 15 ) &  (unsigned)(~0xf);
		elementSize = (size + 15) & (USHORT)(~0xf);
		unsigned arrayPsize = numElements*sizeof(char*);
		heap.set(elementSize*numElements + arrayPsize+numElements); // numElements is number of flags we need, so it is added to memory count!
		flags = heap.allocate(numElements);
		assert(flags!=NULL);
		char *arrayP = heap.allocate(arrayPsize);
		elements= (char**)arrayP;
		for(UINT i=0; i<numElements; i++)
		{
			elements[i] = heap.allocate(elementSize);
			elements[i][0] = NUL;
			flags[i] =  EMPTY;
		}
		curEmptyCount = numElements;
	}

	UINT memUsed() { return (numElements - curEmptyCount) * elementSize; }
	//////////////////////////////////////////////
	char *getElementAt(UINT idx)
	{
		WLBreakIf(idx < 0 || idx >= numElements, "!!ERROR! WLArray::getElementAt(%d): out of range idx!\n", idx);
		ShowInfoIf(flags[idx]==EMPTY, "!!WARNING! WLArray::getElementAt(%d): element is EMPTY!\n", idx);
		if(flags[idx]==EMPTY)
			return NULL;
		return elements[idx];
	}
	////////////////////////////////////////////
	char *getNext(int *idx)
	{
		for(UINT i=0; i<numElements; i++)
		{
			if(flags[i] == EMPTY)
			{
				flags[i] = FULL;
				*idx = i;
				curEmptyCount--;
				WLBreakIf(curEmptyCount<0, "!!ERROR! WLArray::getNext() runs out of empty slot!\n");
				return elements[i];
			}
		}
		ShowInfo("!!WARNING! WLArray::getNext: ran out of available spots in word cache array!\n");
		return NULL;
	}
	////////////////////////////////////////////////
	char *next()
	{
		for(UINT i=0; i<numElements; i++)
		{
			if(flags[i] == EMPTY)
			{
				flags[i] = FULL;
				curEmptyCount--;
				WLBreakIf(curEmptyCount<0, "!!ERROR! WLArray::next() runs out of empty slot!\n");
				return elements[i];
			}
		}
		ShowInfo("!!WARNING! WLArray::next: ran out of available spots in word cache array!\n");
		return NULL;
	}
	////////////////////////////////////////////////////
	void release(unsigned idx)
	{
		WLBreakIf(idx<0 || idx>=numElements, "!!ERROR! WLArray::release: idx %d doesn't belong to array!\n", idx);
		flags[idx] = EMPTY;
		curEmptyCount++;
		elements[idx][0] = NUL;
	}
	/////////////////////////////////////////////////////
	void release(char *element)
	{
		// note!! FixMe! we should be able to find i bellow without having to parse the bufer!
		WLBreakIf(!heap.belongs(element), "!!ERROR! WLArray::release: element #%s# doesn't belong to array!\n", element);
		for(UINT i=0; i<numElements; i++)
		{
			if(element==elements[i])
			{
				WLBreakIf(flags[i] != FULL, "!!ERROR! WLArray::release: arry slot %d is not full!\n", i);
				flags[i] = EMPTY;
			    elements[i][0] = NUL;
				curEmptyCount++;
				return;
			}
		}
		WLBreak("!!ERROR! WLArray::release: couldn't find array element for %s!\n", element);
	}
	//////////////////////////////////////////////////////
	void releaseAll()
	{
		for(UINT i=0; i<numElements; i++)
		{
			flags[i] = EMPTY;
			elements[i][0] = NUL;
		}
		curEmptyCount = numElements;
	}
};

#if defined(_WINDOWS)
struct WLVirtualArray
{
	unsigned numElements;
	unsigned numUsedElements;
	unsigned numPages;
	USHORT elementSize;
	unsigned pageSize;
	WLHeapVirtual heap;
	char **elements;
	char *flags; 
	unsigned curEmptyCount; //number of slots currently empty in array
	void set(unsigned numpages, unsigned pagesize, unsigned elementsize, unsigned pageIdx = 0)
	{
		ShowInfo("Cache Array set: numPages=%d, pagesize=%d elementSize=%d\n", numpages, pagesize, elementsize);
		if(heap.currentCommitIdx == -1) {
			numUsedElements = 0;
			numPages = numpages;
			pageSize = (pagesize + 15) & (unsigned)(~0xf);
			numElements = (numpages*pagesize + 15 ) &  (unsigned)(~0xf);
			elementSize = (elementsize + 15) & (USHORT)(~0xf);
			unsigned arrayPsize = numElements*sizeof(char*);
			unsigned flagsize = numElements *sizeof(char);
			flagsize = (flagsize+15) & (unsigned)(~0xf);
			heap.set(numPages, elementSize*pageSize, arrayPsize+flagsize);
			flags = heap.allocate(flagsize);
			assert(flags!=NULL);
			char *arrayP = heap.allocate(arrayPsize);
			elements= (char**)arrayP;
			for(UINT i=0; i<numElements; i++)
			{
				elements[i] =NULL;
				flags[i] =  EMPTY;
			}
			heap.commit(0);
		}
		
		unsigned pageoffset = pageIdx * pageSize;
		for(UINT i=0; i<pageSize; i++)
		{
			elements[pageoffset+i] = heap.allocate(elementSize);
			elements[pageoffset+i][0] = NUL;
		}
		curEmptyCount = numElements;
	}

	void setHeapPage(unsigned pageIdx)
	{
		heap.commit(pageIdx);
	}
	UINT memUsed() { return numUsedElements * elementSize; }
	//////////////////////////////////////////////
	char *getElementAt(UINT idx)
	{
		WLBreakIf(idx < 0 || idx >= numElements, "!!ERROR! WLArray::getElementAt(%d): out of range idx!\n", idx);
		ShowInfoIf(flags[idx]==EMPTY, "!!WARNING! WLArray::getElementAt(%d): element is EMPTY!\n", idx);
		if(flags[idx]==EMPTY)
			return NULL;
		return elements[idx];
	}
	////////////////////////////////////////////
	char *getNext(int *idx)
	{
		for(UINT i=0; i<pageSize; i++)
		{
			if(flags[i] == EMPTY)
			{
				flags[i] = FULL;
				*idx = i;
				curEmptyCount--;
				numUsedElements++;
				WLBreakIf(curEmptyCount<0, "!!ERROR! WLArray::getNext() runs out of empty slot!\n");
				return elements[i];
			}
		}
		ShowInfo("!!WARNING! WLArray::getNext: ran out of available spots in word cache array!\n");
		return NULL;
	}
	////////////////////////////////////////////////
	char *next()
	{
		for(UINT i=0; i<pageSize; i++)
		{
			if(flags[i] == EMPTY)
			{
				flags[i] = FULL;
				curEmptyCount--;
				numUsedElements++;
				WLBreakIf(curEmptyCount<0, "!!ERROR! WLArray::next() runs out of empty slot!\n");
				return elements[i];
			}
		}
		ShowInfo("!!WARNING! WLArray::next: ran out of available spots in word cache array!\n");
		return NULL;
	}
	////////////////////////////////////////////////////
	void release(unsigned idx)
	{
		WLBreakIf(idx<0 || idx>=numElements, "!!ERROR! WLArray::release: idx %d doesn't belong to array!\n", idx);
		flags[idx] = EMPTY;
		curEmptyCount++;
		numUsedElements++;
		elements[idx][0] = NUL;
	}
	/////////////////////////////////////////////////////
	void release(char *element)
	{
		// note!! FixMe! we should be able to find i bellow without having to parse the buffer!
		WLBreakIf(!heap.belongs(element), "!!ERROR! WLArray::release: element #%s# doesn't belong to array!\n", element);
		for(UINT i=0; i<numElements; i++)
		{
			if(element==elements[i])
			{
				WLBreakIf(flags[i] != FULL, "!!ERROR! WLArray::release: arry slot %d is not full!\n", i);
				flags[i] = EMPTY;
			    elements[i][0] = NUL;
				curEmptyCount++;
				numUsedElements++;
				return;
			}
		}
		WLBreak("!!ERROR! WLArray::release: couldn't find array element for %s!\n", element);
	}
	//////////////////////////////////////////////////////
	void releaseAll()
	{
		for(UINT i=0; i<numElements; i++)
		{
			flags[i] = EMPTY;
			elements[i][0] = NUL;
		}
		curEmptyCount = numElements;
		numUsedElements = 0;
		heap.unCommit();
	}
};
#endif

CPPExternClose

#endif   //UTILITY_H
