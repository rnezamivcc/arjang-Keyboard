
#ifndef DICTMANAGER_H
#define DICTMANAGER_H

#include "utility.h"
#include "dictrun.h"
#include "nGramHistory.h"
#include "phraseEngine.h"
#include "nGramLearning.h"
#include "swipe.h"

class Dictionary;
class UserWordCache;
class UserWordCacheOffline;
class TGraph;
struct WordNode;
class AutoCorrect;

//////////////////////////////// NGRAM STUFF ///////////////////////////////////////////////////////////
typedef struct NGramMultiNode
{
	CompactNode* endNodes[MAX_TGRAPH_HISTORY_NUM + 1];
	UINT pref;
	bool startPhrase; // indicates this phrase can work as starting a sentence. It is currently used only for starting word setup!
	NGramMultiNode() { reset(); }
	void reset()
	{
		memset(this, 0, sizeof(NGramMultiNode));
	}
	
	MYWCHAR **getStr(CCompactStore *cstore, int &len);
	CompactNode *getEndNode();
	void set(NGramMultiNode *cp);
	USHORT wordCount();
	//unsigned long hash; // a hash generated from the phrase string to make comparison easier!
	WordNode *wordNodeP;
} NGramMultiNode;

extern BYTE sWordPosInPhrase;
extern bool sEndofSentenceReached;
///////////////////////////// END of NGRAM STUFF ///////////////////////////////////////////

typedef	struct dictionaryConfigurationEntry 
{
	eLanguage langIdx;
	Dictionary *existingDictionary;
	BYTE   priority;
	BYTE   dictIdxWeight;
	bool  enabled;
	dictionaryConfigurationEntry():langIdx(eLang_NOTSET), existingDictionary(NULL), priority(0), dictIdxWeight(0), enabled(false){}
	~dictionaryConfigurationEntry();
} DictionaryConfigurationEntry;

CPPExternOpen
int readDirectoryConfiguration(DictionaryConfigurationEntry *configDictList, int enabledListSize);
char *createFullPathFileName(const char *fileName, char* ext = NULL);    
CPPExternClose

class CCompactDict;
class CCompactStore;
struct CacheNextWordInfo;


struct MultiLetterAdvanceAr
{
	MYWCHAR nextWords[NMWORDPREDICTIONS][MAX_WORD_LEN];
	BYTE nShouldUpperCase[NMWORDPREDICTIONS];
	MYWCHAR rootWord[MAX_WORD_LEN];
	USHORT prefs[NMWORDPREDICTIONS];
	BYTE nActualNexts; // this the total number of next words, both predictions and corrections
	BYTE nBackSpace; // number of possible backspaces should be done. It is useful when inserting puctuation, and we need to backspace first.
	BYTE nCorrections; // number of corrections in nextWords.
	bool bCorrection[NMWORDPREDICTIONS]; //Indicates that which indexes are corrected for iOS. 
	void add(MYWCHAR *word, USHORT pref=MAX_PREFERENCE);
	void reset();
	void setActualNexts();
	void set(int num, MYWCHAR **nexts);
	
	MultiLetterAdvanceAr()
	{
		reset();
	}
};

/////////////////////////////////////////////////////////////////////////////////////
struct PhraseAr
{
	MYWCHAR phrases[MAX_PHRASE_ALLOWED][MAX_PHRASE_LEN];

	USHORT prefs[MAX_PHRASE_ALLOWED];
	BYTE count;

	void reset();
	void setActualNexts();
	void set(int num, MYWCHAR **phrases);

	PhraseAr()
	{
		reset();
	}
};
/////////////////////////////////////////////////////////////////////////////////////
struct NounAr
{
	MYWCHAR word[MAX_FOUND_PHRASES][MAX_WORD_LEN];
	int count;
	NounAr()
	{
		memset(this, 0, sizeof(NounAr));
	}
};
///////////////////////////////////////////////////////////////////////////////////////
class CDictManager
{
public:
    CDictManager();
    ~CDictManager();

	bool Create(char *dictname = NULL, char *root = NULL);
	void Destroy();

	inline static USHORT getNumExistingDictionaries() { return m_nExistingDictionaries; }
	MYWCHAR **GetNextWords(int *nPosWordsPtr, MYWCHAR *rootWord, bool doNext = true, MYWCHAR* letters = NULL);
	MultiLetterAdvanceAr *multiNextWords( MYWCHAR *rootWord, bool doNext=false, bool bUpdateTGraph = true);
	MultiLetterAdvanceAr *advanceMultiLetters(MYWCHAR *letters, BYTE *prefs,  MYWCHAR *rootWord, MYWCHAR **printPart=NULL, BOOL startSentence=false, BOOL literal=false);
	BOOL advanceLetter(MYWCHAR letter, MYWCHAR **printPart, BOOL startSentence = false);

	BYTE advanceWord(MYWCHAR *wordPart, MYWCHAR **printPart, MultiLetterAdvanceAr *&nextwords, bool wordComplete, bool replace=false);
	MYWCHAR *undoLetterOrWord();
	MultiLetterAdvanceAr *backspaceLetter(MYWCHAR *rootWord, bool doNext = true);
	MultiLetterAdvanceAr *eraseLastWord(MYWCHAR* rootWord,unsigned &numLetterBackedOut);

	int DoAutoCorrect(MYWCHAR *advanceLetters=NULL);

	MultiLetterAdvanceAr* advanceLetterSwipe(MYWCHAR* letters, bool complete=false);

	MYWCHAR *nextLetters(int **posWordsAr);
	BOOL isEVerbRetrievalActive() { return m_eVerbRetrievalActive; }
	BYTE isChunk(int node);
	BYTE isEndpoint(int node);
	BOOL isChunk(MYWCHAR *wordPart);
	void reset();
	void fullReset();
	
	CCompactDict *getCompactDict(int dictIdx);
	CCompactStore *getCompactStore(int dictIdx);
	inline static USHORT getNumberOfDictionaries() { return m_nOrderedDicts; }
	bool  resetConfiguration();

	USHORT getDictIdxWeight(int dictIdx) {return m_orderedDictList[dictIdx].dictIdxWeight;}
	char *getTopDictionaryName();
	void setTopDictionary() { mTopDictionary = getDictionaryEnabled(1); }
	eLanguage getTopDictionaryId() { return m_orderedDictList[1].langIdx; }
	Dictionary *getTopDictionary() { return  mTopDictionary; } // returns the top (but not personal!) dictionary

	BOOL CurrentWordCanbeAdded();
	CompactNode* addWord(MYWCHAR *newWord, int pref, int *dictIdx);
	BOOL deleteWord(MYWCHAR *deleteWord);

	//BOOL mngrDeleteWordFromPersonal(MYWCHAR *word);
	BYTE  isNodeAnEVerbEnding(int startDictIdx, int endDictIdx, int *nTrailChars, BYTE *eVerbVarIdx);
	MYWCHAR *gatherRootEnding(); // return root of current word. 
	USHORT  calcCascadingBasePreference(int dictIdx, WordValue wordValue, int layerId); 

	void setAutoLearn(BOOL bAutoLearn) { m_autoLearnActive = bAutoLearn; }
	void setSpacelessTyping(BOOL bAutoLearn) { m_spacelessTyping = bAutoLearn; }
	void printPreferences(MYWCHAR *afterTheseChars);
	bool learnCurrentWord(MYWCHAR* inputWord = NULL, bool endphrase=false);

	int getDictBuildNumber(int dictId);
	int getDictVersionNumber(int dictId);
	int getNumWordsStartingWith(MYWCHAR *word, int *dictId=NULL);

	CompactNode * retrieveEndNodeForString(MYWCHAR *word, int *dictIdx, bool isEndpoint = false);
	int isWordInDictionaries(const MYWCHAR *word, int &dictId);
	int isAWord(const MYWCHAR *word){
		int dictId;
		return isWordInDictionaries(word, dictId);
	}
	bool accentEqual(MYWCHAR ch1, MYWCHAR ch2);
	MYWCHAR removeAccent(MYWCHAR ch);

	Dictionary *mTopDictionary;

	Swipe mSwiper;

	MYWCHAR *retrieveStringForEndNode(CompactNode *endNode);
	int getWordForEndNode(CompactNode *node, MYWCHAR *outWord);
	MYWCHAR *getWordFromNode(CompactNode *node);
	int mngrGetWordsAddedSinceLastCallAndReset(BYTE *accum);
	//UserWordCache *getUserCache() { return mUserWordCache; }
	MYWCHAR *getWordList(int dictIdx, int *count);
	static DictionaryConfigurationEntry *GetOrderedDict(int idx); 
	bool SetOrderedDict(int langId, int priority, bool enabled);
	
	static WLHeap sWHHeap[NWORDS]; // words history heap
	static WLHeap sWPHeap[NCURWORDS];  // parallel words heap
	void ProcessLearningOffline(const char *filePath);

	PhraseAr * ProcessPhrasePrediction(MYWCHAR *inputWord, bool bFromBackSpacing = false, MYWCHAR* nextLetter = NULL);
	void		SetHistoryFromJNI(MYWCHAR	*inputWord, bool backspace = false);
	static BYTE sNumBackouts;

	static void SetNumOrderedEnabledDictionaries(int count) {m_nOrderedDicts = count; }
	int getDictPriority(const char *name);
	BOOL fillActiveConfiguration();
	void ConvertWordForChunk(MYWCHAR* wordInput);
	MultiLetterAdvanceAr * UpdateForStartWords();

	int setNGramFrom2Grams(CompactNode *end1, CompactNode *end2=NULL, MYWCHAR* cmpWord = NULL, int minNGram = 0);
	int setNGramFrom2Grams(MYWCHAR *word1, MYWCHAR *word2=NULL, MYWCHAR* cmpWord = NULL, int minNGram = 0);

	inline NGramLearning* GetNGramLearning() { return m_NGramLearning; }
	void SetProcessedArr(MultiLetterAdvanceAr arr) { mMultiLetterNexts.reset(); mMultiLetterNexts = arr; }
	NounAr* GetNounsFromPhrase(MYWCHAR* phrase);
	void LearnStartingWords(MYWCHAR *word = NULL);
	void LearnMultiGramWord(MYWCHAR* input = NULL);
	void ResetTGraphHistory() {m_History.NGramHistoryReset(); }
	CompactNode* QuadRetrieveCompactNode(MYWCHAR* word, bool CopyActualWord, int *retDictIdx);

	AutoCorrect *getAutocorrect() { return mAutoCorrect; };
	//Dictionary *getPersonalDictionary();
	
	Dictionary *getDictionaryEnabled(int dictIdx);

	void setMostPreferredWord(MYWCHAR *word, int pref);
	void sortMostPreferredWords();
	void resetMostPreferredWords() { 
		for (int i = 0; i < NWORDPREFERENCES; i++){
			mMostPreferredWordPrefs[i] = 0;
			mMostPreferredWords[i] = NULL;
		}
	}
	MYWCHAR **getMostPreferredWords(int &count) { 
		count = 0;
		while (mMostPreferredWords[count] && mMostPreferredWords[count][0] != NUL && count < NWORDPREFERENCES)
			count++;
		return mMostPreferredWords; 
	}

private:
	int GetDictId(const char* name);
	char *retrieveWord(char *line, int *nLengthP);
	static const char *GetUserInitFilename();

	AutoCorrect *mAutoCorrect;

	void fillFiveMostPreferredWords();
	int addNextAndLearnedPredictions(bool doNext, MYWCHAR* letters);
	SearchResultEntry * findLowestPrefInFinalFive(PredictionType lookingFor, int *finalFiveIdxP);
	bool  UpdateInFinalFive(MYWCHAR *text, USHORT cascPref);
	bool addUniqueToFinalFive(MYWCHAR *text, PredictionType predType, USHORT pref);
	BOOL replaceInFinalFive(MYWCHAR *text, PredictionType predType, USHORT pref);

	int readUserConfiguration(DictionaryConfigurationEntry *registryDictList, int listSize, BOOL bEnabledOnly);
	Dictionary *openExistingDictionary(int idx, eLanguage elang);
	void LoadCompactPhraseFile(char* filePath);

	BOOL openExistingDictionary(int idx);
	BOOL openDictionary(char *fullpathname, int listIdx=1);
	BOOL closeExistingDictionary(int idx, BOOL justReleaseFlag);
	BOOL partOfWordsInADictionary(int fIdx, int lIdx, int exceptIdx, BOOL wordByItself);
	BOOL partOfWordsInAnyDictionaries();	
	BOOL activateConfiguration();
	MYWCHAR** findNextWords(int *count);

	BOOL mngrDeleteWord(MYWCHAR *word, int dictIdx, CompactNode *nodePath);
	void updateMultiLetterNextWords(USHORT basePref, MYWCHAR *curLetter=NULL, bool complete=true, bool replace=true, bool bUpdateTGraph = true, bool bSpace=false);
	CompactNode* mngrAddWord(MYWCHAR *word, int preference, int dictIdx = 0);

	static DictionaryConfigurationEntry	m_orderedDictList[MAX_NUM_DICTS];
	void mngrNextWordsInDictionaries(int startDict,int endDict, eSearchMode searchMode, WordValue wordValue, int layerId);
	void mngrCollectEVerbsInDictionaries(int DictIdx, WordValue wordValue, int layerId, BYTE eVerbVarIdx);
	void mngrInternalNextWordsInDictionaries(int layerId);

	void mngrNextLettersInDictionaries();
	void mngrInternalNextWords(MYWCHAR *root);

	void doLanguageSpecificCorrections(MYWCHAR letter, MYWCHAR *printChunk);
	BYTE mngrFillNextNodeDictionaries(MYWCHAR letter, StepsType stepsType);
	BYTE getLetterPref(MYWCHAR letter); 
	BYTE getWordPref(BYTE endPointInDicts, BYTE path, BYTE layer);
	bool UpdateWordLayers(USHORT containingLayerEnds, USHORT containingLayers, BYTE endPointInDicts);
	void PurgeTopLayers(int topcontainingLayer);
	BYTE mngrAttemptFillNodeDictionaries(MYWCHAR letter, USHORT *containinglayers, USHORT *layerEnds);
	void mngrInternalBackSpaceLetterDictionaries(BOOL clearRankedResultsFlag);

	int  mngrIsWordOrChunkInDictionariesIdx(int dictStart, int dictEnd, CompactNode **nodePath, BYTE *layer);
	BOOL mngrAdvanceLetter(MYWCHAR letter, MYWCHAR *printChunk, BOOL startSentence = false);
	BYTE mngrAdvanceWordC(MYWCHAR *wordPart, MYWCHAR **printPart, MultiLetterAdvanceAr *&nextwords, 
		BOOL wordCompleted, bool replace, MYWCHAR* originalWordPart);

	void takeOutTheEVerb(int eVerbDictIdx, BYTE pref[]);
	void putTheEVerbBackIn(int eVerbDictIdx, BYTE originalPref[]);

	void mngrCloseAllOpenDictionaries(BOOL justReleaseFlag);

	Dictionary *createFromScratch(int nDictIdx, char *fileName = NULL);
	int mngrGetEnabledDictionaries(DictionaryConfigurationEntry *enabledRegistryDictList, int enabledListSize);
	BOOL createDefaultDictionaryConfig(DictionaryConfigurationEntry *enabledRegistryDictList);

	//Minkyu: 2013.11.14
	void BuildTGraph(MYWCHAR* curWord);
	void InitializeNGramNode(char* textFilePath);
	
	void ResetTGraph(bool bAll = false);
	MYWCHAR** ProcessingBackoutNum(MYWCHAR* wordPart, MYWCHAR*originalWordPart, MYWCHAR **printPart, BYTE* numOfLettersBacout);
	BYTE RecalculateBackoutNum(MYWCHAR* wordInput);
	BYTE RecalculateBackoutNumForNumbers(MYWCHAR* wordInput, BYTE backoutNum);
	BYTE RecalculateBackoutNumForEmails(MYWCHAR* wordInput, BYTE backoutNum);

	void UpdateForNumbers(MYWCHAR *letters, USHORT basePref);
	void UpdateForDegrees(USHORT basePref);
	void UpdateMultiLetterNextWordsCase(MYWCHAR *curLetter, bool bSpace, bool complete);
	void UpdateForEmails();
	void UpdateForStartWordsWithLetter(MYWCHAR* curLetter);
	void UpdateAfterNumbers(MYWCHAR* curLetter);
	void ChangeUpperForI(MYWCHAR* input, MYWCHAR*print);
	void ChangeSpecialUpperCase(MYWCHAR* input, MYWCHAR* original);

	MYWCHAR* DeleteStartingWords(MYWCHAR *wordPart);

	bool IsDuplicate(MultiLetterAdvanceAr*arr,MYWCHAR*startWord);
	int ConvertWordArr2Chunks(MultiLetterAdvanceAr *wordAr);

	void PrintPhrases(PhraseAr* arr, int startPos, char* szType);
	void ReplaceStartWordCache(MultiLetterAdvanceAr*arr, USHORT  basePref, MYWCHAR* curLetter);

	MYWCHAR* GetSwipePath(MYWCHAR* letters);

	USHORT mMostPreferredWordPrefs[NWORDPREFERENCES];
	MYWCHAR *mMostPreferredWords[NWORDPREFERENCES + 1];
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	static USHORT	m_nOrderedDicts;    // number of active, i.e. enabled dictionaries which exist in dictionary folder, which means <= m_nExistingDictionaries
	static USHORT m_nExistingDictionaries;
	//BYTE	m_personalDictIdx;
	
	// being called internally
	BOOL  mngrLearnWord(MYWCHAR *thisword, bool endphrase=false);
	MYWCHAR m_prefLetters[MAX_FOLLOWING_LETTERS+1];
	int   m_posWordChunks[MAX_FOLLOWING_LETTERS+1];
	BOOL m_eVerbRetrievalActive;
	MYWCHAR  m_eVerbPrefixStrWordPath[MAX_NUM_PREDICTIONS];

	BOOL m_noDictionariesLoaded;
	BOOL m_autoLearnActive;
	BOOL m_spacelessTyping;
	bool sDoFillFinalList;

	PhraseAr mPhraseAr;

	MultiLetterAdvanceAr mMultiLetterNexts;


#ifdef WL_SDK
	PhraseEngine *mPhraseEngine;
#endif

	UserWordCache *mUserWordCache;
	UserWordCacheOffline *m_pUserWordCacheOffline;
	TGraph	*m_TGraph;

	NGramHistory		m_History;
	NGramLearning*		m_NGramLearning;	
};

#endif   //DICTMANAGER_H
