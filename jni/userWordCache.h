
#ifndef _USERWORDCACHE_H_
#define _USERWORDCACHE_H_

#define MRUWORDSINCACHEMAX 254
#define MAX_NUM_FLUSH_PER_CALL 5
#include "utility.h"

const BYTE	eCacheRegular = 0; // a regular word in cache.
const BYTE  eCacheNew = 1;     // a new word, meaning it has not yet been saved out to any active dictionary!


struct CacheWords
{

	MYWCHAR word[MAX_WORD_LEN];
	BYTE pref;
	BYTE freq;
	CacheWords()
	{
		memset(word,0,sizeof(word));
		pref = 0;
		freq = 0;
	}

};
struct CacheNextWordInfo
{
	BYTE idx;
	BYTE pref;
	void set(BYTE id=0xff, BYTE pre=0) 
	{ 
		idx=id; pref=pre; 
	}
	static void swap(CacheNextWordInfo &a, CacheNextWordInfo &b) { CacheNextWordInfo tmp = a; a = b; b = tmp;}
};

typedef struct wordEntry
{
	BYTE pref;
	BYTE freq; // how many times used while still in the cache
	BYTE bufferIdx;  // index of this word in underlying word buffer.
	BYTE len:6;	 // length of the text
	BYTE state:2; // default is eCacheRegular

	CacheNextWordInfo nextWords[MAX_NUM_PREDICTIONS];  // array of indices to the most frequent next 4 words! 
	void reset();
	void resetNextWords() { 
		for(int i=0; i<MAX_NUM_PREDICTIONS; i++)
			nextWords[i].idx = 0xff;
	}
	USHORT accessed; // an integer to represent the last access time of this word. The higher means the latest
	MYWCHAR text[MAX_WORD_LEN];
	wordEntry(): pref(0), freq(0), len(0),bufferIdx(0xFF), accessed(0)  {text[0] = NUL; }
} WordEntry;


static const int sMaxUpdatesBeforeSaveOut = MRUWORDSINCACHEMAX/4;  // after sMaxUpdatesBeforeSaveOut updates to cache, we write it out to a file for backing up!
extern WordEntry * sFlushableWords[sMaxUpdatesBeforeSaveOut];


static const int sMaxFrequencyBeforeSaveToDicts = 3;

class CDictManager;
class UserWordCache
{
	class CacheQueue;
	class QueueEntry
	{
	public:
		QueueEntry():mPrev(NULL), mNext(NULL), mqep(NULL), mvp(NULL) {}

		void setValues( QueueEntry *qEntryp, void *vp)
		{
			mqep = qEntryp; // pointer to mMRUQueue entry in word queue, and to word queue entry in mMRUQueue
			mvp = vp;
		}

		QueueEntry *getQueueEntryValue() { return mqep;}
		WordEntry *getWordEntryValue() { return (WordEntry *) mvp;}
		CacheQueue *getQueueValue() { return (CacheQueue *) mvp;}

		QueueEntry  *mPrev;
		QueueEntry	*mNext;
	private:

		QueueEntry *mqep;// mWordQueues: pointer to mMRUQueue entry; mMRUQueue: pointing to word queue entry in correspondng wordqueue
		void *mvp;		//  mWordQueues: pointing to wordEntry;      mMRUQueue: pointing to corresponding wordqueue in mWordQueues

	};

	class CacheQueue
	{
	public:
		CacheQueue(bool bLetterQueue);

		void insertEntry( QueueEntry *qe );
		void detachEntry( QueueEntry *entry );
		void deleteEntry(QueueEntry *qe);

		QueueEntry* findEntry(MYWCHAR *word, int len);
		int touchEntry( QueueEntry *entry, USHORT access);
		QueueEntry *purgeEntry();
		void clean();
		void findWordsReadyToFlush(BYTE *count);
		void findWordsToDump(BYTE *count, FILE *fp);
		bool ContainInPredictions(WCHAR* text);
		WordEntry* getNextMostFrquentEntry(BYTE topFreq=0);

		int findPredictionsStartingWith(MYWCHAR *word, USHORT *prefs);
#ifdef _DEBUG
		void printMatchingWords(MYWCHAR *word);
		void printQueue(BOOL bMruQueue);
#else
		void printMatchingWords(MYWCHAR *word){}
		void printQueue(BOOL bMruQueue){}
#endif
		QueueEntry *getFirst() { return mFirst;}
	private:
		QueueEntry	*mFirst; /* First entry is element which has been used least recently. */
		QueueEntry	*mLast; /* Last entry is element which has been used most recently. */
		bool	    mIsLtrQueue; // whether queue is used as ltrqueue or mruqueue
	};

public:
	UserWordCache();
	~UserWordCache();
	void loadWord( MYWCHAR *word, BYTE pref, USHORT access, int len);
	void putWord( MYWCHAR *word, BYTE pref, USHORT access, int len, bool bNewWord, bool stopPhrase);

	void unLearn(MYWCHAR *word);
	void replace(MYWCHAR *curword, MYWCHAR *replacement); // replaces an already existing word in the cache with a new one. It is crucial that it uses the same buffer slots!
	
	
	int flushLearnedWords(); // flushes some of the learned words to personal dictionary. Returns number of flushed ones.
	
	
	MYWCHAR** findPredictionsStartingWith(MYWCHAR *text, USHORT *prefs, int *count);
	MYWCHAR** GetPossibleFollowWords(MYWCHAR *word, int len, int *numFound);
	void depreciate(MYWCHAR *word);

	int GetWordsAddedAndReset();

	static WLArray sCacheAr;  // array heap to contain actual words strings
	
private:
	void InitializeCurrentCaches(char* filename);
	void ProcessCacheLine(MYWCHAR *line);

	void removeAll();
	static BYTE sTotalWordAddedSinceLastCheck;
	void RoundupAccessCounts(); // used when access counter sLatestAccess overflows!
	void graduateToDictionaries();
	void dumpOutToCacheFile();
	static WordEntry *allocateWordEntry(MYWCHAR *word, BYTE pref, USHORT access, int len, bool bNewWord);
	static void freeupWordEntry(QueueEntry *wqe);
	static int touchWordEntry(WordEntry *p, USHORT access);

	CacheQueue *pickWordQueue(MYWCHAR letter, BOOL bCreate);
	void removeWordFromLookupQueues( MYWCHAR *word );
	QueueEntry *findWordEntry(MYWCHAR *word, int len);

	
	static void Flush2Dict(BYTE *count);
	static void SaveToCacheFile(BYTE *count, FILE *fp);

	USHORT mMax; // max number of entries in the cache.
	static USHORT sLatestAccess; // latest count for accessing cache.
	const static USHORT QUEUESIZE = 54;  // this should cover all latin based languages. For other languages we may need to modify this class!
	/*Current number of objects in the cache.  */
	USHORT mNumWordsInCache;
	static BYTE mFlush_freq_threshold;

	
	static WLArray  sQueueAr;  // array heap to contain data structure needed for managing cache.

	CacheQueue *mMRUQueue;  // list of most recently used queue entries from mWordQueues. "Last" is the most recently used.
	CacheQueue *mWordQueues[QUEUESIZE];  // an array of linked list of word entries in cache. Each list correspond to one letter heading, 
	                                    //for instance all words starting with 'A' go under mWordQueues[0];
	QueueEntry *mLastWord; // used for 2-grams
	QueueEntry *mpreLastWord; // used for deleting current word history!

	void UpdateLastWord(QueueEntry *currWord, bool stopPhrase);
};


#endif
