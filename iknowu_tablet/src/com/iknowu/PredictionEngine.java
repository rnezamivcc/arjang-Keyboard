package com.iknowu;

import com.iknowu.swipe.Sw_Node;

import android.util.Log;

/**
 * Java wrapper class that provides access to the underlying library via JNI Native function calls
 * @author Justin
 *
 */
public class PredictionEngine {
	private final static String TAG = "PredEn_Logs";

	public class AdvanceLetterInfo {
		boolean beginSentence;
		boolean literalFlag;  // means treat the letter as literal and do not try to be smart about it! useful for say period in a url address and so on.
		String printBuffer;
		String rootBuffer;
	};

	public class AdvanceWordInfo {
		String printBuffer;
		String rootBuffer;
	};
	
	public class NextWordsInfo 
	{
		int nPredictions;
		int nCorrections;
		int nPhraseWords;
		int rootWordExist; 
		String nextWordsAr[];
		String rootWord;
		String phraseAr[];
		int nPosWords() { return (nPredictions + nCorrections); }
	}
	
	public class NounInfo 
	{
		int nNounWords;
		String NounAr[];
	}

	
	public class DictInfo {
		int listIdx;  // index in the list. It changes as priority in the list order changes.
		boolean bEnabled;
		int langIdx;  // this is a global constant index for a lang, for example English is always 1.
		int dataVersion; // version number related to dictionary content. 
		int codeVersion; // version number related to data structure of the content.
		String name;
	}
	public class AddedWordsInfo {
		int nPosWords;
		String addedWordsAr[];
	}

	private AdvanceLetterInfo m_advanceLetterInfo = new AdvanceLetterInfo();
	private AdvanceWordInfo   m_advanceWordInfo   = new AdvanceWordInfo();
	NextWordsInfo 	  		  m_nextWordsInfo 	  = new NextWordsInfo();
	private NextWordsInfo     personalDictInfo    = new NextWordsInfo();
	private AddedWordsInfo	  m_addedWordsInfo	  = new AddedWordsInfo();
	private DictInfo		  m_dictInfo		  = new DictInfo();
	private NounInfo		  m_NounInfo		  = new NounInfo();
	
   // load the library - name matches jni/Android.mk
	static {
		try {
			System.loadLibrary("wlpredcorrect");
		} catch (UnsatisfiedLinkError e) {
			Log.e("WLKB", "Unable to load native library wlpredcorrect\n" +e);
		}
	}
	
	private native String ntvNextLetters( int posWordsAr[]);
	private native int ntvAdvanceLetter(char c, AdvanceLetterInfo advanceLetterInfo, NextWordsInfo nexts);
	private native int ntvAdvanceMultiLetters(String letters, byte[] inprefs, NextWordsInfo nexts, boolean begin);
	private native boolean ntvBackspaceLetter( NextWordsInfo nexts);
	private native int ntvNextWords(NextWordsInfo nextWordsInfo);
	private native boolean ntvIsChunk(String candidatePred);
	private native int ntvAdvanceWord(String candidatePred, AdvanceWordInfo awInfo, NextWordsInfo nexts, boolean bWordComplete, boolean replace);
	
	private native void ntvPathSwipe(Sw_Node[] candidatePath, NextWordsInfo nexts);
	
	private native int ntvAutoAddedWords(AddedWordsInfo addedWordsInfo);
	
	private native int ntvListWords(int dictId, NextWordsInfo wordList);
	private native boolean ntvGetDictInfo(int index, DictInfo info);
	private native int ntvGetDictPriority(int idx);
	private native boolean ntvSetDictSetting(int idx, int priority, boolean enabled);

    private native int ntvEraseLastWord(NextWordsInfo nexts);

	private native String ntvUndoLetterOrWord();
	
	private native boolean ntvInitialize(String root);
	private native boolean ntvIsNewWord();
	private native boolean ntvResetDictionaryConfiguration();
	private native void   ntvReset(boolean cleanHistory);
	private native String ntvGetRootText();
	private native boolean ntvAddWord(String newWord, int dictIndex);
	private native boolean ntvDeleteWord(String word);
	private native void ntvSetAutoLearn(boolean bAutoLearn);
	private native void ntvSetSpacelessTyping(boolean spaclessOn);
	private native void ntvResetTGraphHistory();

    private native boolean ntvWordCanbeAdded();
	
	private native int ntvNumWordsStartingWith(String word);

    private native void ntvLearnFromFile(String path);

    private native int ntvSetHistory(String words, boolean backspace);
      
    private native int ntvStartingWords(NextWordsInfo nextWordsInfo);
    
    private native void ntvGetNounResult(NounInfo nounInfo, String words);

	public void reset(boolean cleanHistory) {
        Log.i(TAG, "PE.reset : cleanHistory = " + cleanHistory); // TODO - remove
        IKnowUKeyboardService.log(Log.VERBOSE, "PredEngine reset", "cleanHistory = "+cleanHistory);

        ntvReset(cleanHistory);
		for (int i=0; i< m_nextWordsInfo.nPosWords(); i++)
		{
			m_nextWordsInfo.nextWordsAr[i] = null;
		}
		for (int i=0; i< m_nextWordsInfo.nPhraseWords; i++) 
		{
			m_nextWordsInfo.phraseAr[i] = null;
		}
		
		m_nextWordsInfo.nPredictions = 0;
		m_nextWordsInfo.nCorrections = 0;
		m_nextWordsInfo.nPhraseWords =0;
		
		for(int i=0; i < m_NounInfo.nNounWords; i++)
		{
			m_NounInfo.NounAr[i] = null;
			
		}
		m_NounInfo.nNounWords = 0;
		
        SetStartingWords();
		
	}
	
	public void ResetTGraphHistory()
	{
        Log.i(TAG, "PE.ResetTGraphHistory : "); // TODO - remove
		ntvResetTGraphHistory();
	}

	public boolean initialize(String path) {
        Log.i(TAG, "PE.initialize : path = |" + path + "|"); // TODO - remove
		IKnowUKeyboardService.log(Log.VERBOSE, "PredEngine initialize", "path = "+path);
		//String root = "/sdcard/wordlogic";
		boolean result = ntvInitialize(path);
        SetStartingWords();
		return result;
	}
	
	public boolean resetDictionaryConfiguration() {
        Log.i(TAG, "PE.resetDictionaryConfiguration : "); // TODO - remove
		return ntvResetDictionaryConfiguration();
	}

	public int advanceLetter( char c, boolean beginSentence, boolean cancelSmartSpacing) {
        Log.i(TAG, "PE.advanceLetter : c = |" + c + "|, beginSentence = " + beginSentence + ", cancelSmartSpacing = " + cancelSmartSpacing); // TODO - remove
		m_advanceLetterInfo.beginSentence = beginSentence;
		m_advanceLetterInfo.literalFlag = cancelSmartSpacing; // change this to true for say period in url address and such.
		return ntvAdvanceLetter(c, m_advanceLetterInfo, m_nextWordsInfo);
	}

    public int advanceLetterMulti(String letters, byte[] inprefs, boolean begin) {
        Log.i(TAG, "PE.advanceLetterMulti : letters = |" + letters + "|, inprefs = " + inprefs + " begin = " + begin); // TODO - remove
        return ntvAdvanceMultiLetters(letters, inprefs, m_nextWordsInfo, begin);
    }

	public String nextLetters( int posWordsAr[]) {
        Log.i(TAG, "PE.nextLetters : posWordsAr = " + posWordsAr); // TODO - remove
        if (posWordsAr != null) {
            for (int i = 0; i < posWordsAr.length; i++) {
                Log.i(TAG, "PE.nextLetters : posWordsAr[" + i + "] = " + posWordsAr[i]); // TODO - remove
            }
        }
		String letters = ntvNextLetters(posWordsAr);
        Log.i(TAG, "PE.nextLetters : letters = " + letters); // TODO - remove
		return letters;
	}

	public int advanceWord( String candidatePred, boolean replace) {
        Log.i(TAG, "PE.advanceWord : candidatePred = |" + candidatePred + "| replace = " + replace); // TODO - remove
		int b = ntvAdvanceWord(candidatePred, m_advanceWordInfo, m_nextWordsInfo, false, replace);
		//Log.d("WLKB", "ntvAdvanceWord: backtrack=" +b);
		return b;
	}
    
	public int advanceLetterSwipe( Sw_Node[] nPath) {
        Log.i(TAG, "PE.advanceLetterSwipe : candidatePath = |" + IKnowUKeyboardView.swiper.printPath()); 
        ntvPathSwipe(nPath, m_nextWordsInfo);
		//Log.d("WLKB", "ntvAdvanceWord: backtrack=" +b);
		return 0;
	}

	public int advanceWordComplete( String candidatePred, boolean replace) {
        Log.i(TAG, "PE.advanceWordComplete : candidatePred = |" + candidatePred + "| replace = " + replace); 
		int b = ntvAdvanceWord(candidatePred, m_advanceWordInfo, m_nextWordsInfo, true, replace);
		//Log.w("WLKB", "AdvanceWordComplete returns int = "+b);
		return b;
	}
	
	public boolean backspaceLetter() {
        Log.i(TAG, "PE.backspaceLetter : "); 
		boolean b = ntvBackspaceLetter(m_nextWordsInfo);
		return b;
	}
	
	public boolean isChunk(String candidatePred) {
        Log.i(TAG, "PE.isChunk : candidatePred = |" + candidatePred + "|"); 
		boolean b = ntvIsChunk(candidatePred);
		return b;
	}

    public int eraseLastWord() {
        Log.i(TAG, "PE.eraseLastWord : "); // TODO - remove
        return ntvEraseLastWord(m_nextWordsInfo);
    }

    /*
	public int nextWords() {
		int nPosWords = ntvNextWords(m_nextWordsInfo);
		return nPosWords;
	}
	*/
    
    public int getNumNextWords() {
        Log.i(TAG, "PE.getNumNextWords : "); 
        if (m_nextWordsInfo.nextWordsAr != null) {
            IKnowUKeyboardService.log(Log.VERBOSE, "getNumNextWords()", " = "+m_nextWordsInfo.nextWordsAr.length);
            return m_nextWordsInfo.nextWordsAr.length;
        } else {
            return 0;
        }
    }
	
	public int addedWordsInfo() {
        Log.i(TAG, "PE.addedWordsInfo : "); 
		int nWords = ntvAutoAddedWords(m_addedWordsInfo);
		return nWords;
	}
	
	public boolean setDictSetting(int idx, int priority, boolean enabled) {
        Log.i(TAG, "PE.setDictSetting : idx = " + idx + " priority = " + priority + " enabled = " + enabled);
		return ntvSetDictSetting(idx, priority, enabled);
	}
	
	public boolean getDictInfo(int idx) {
        Log.i(TAG, "PE.getDictInfo : idx = " + idx); 
		return ntvGetDictInfo(idx, m_dictInfo);
	}
	
	/**
	 * Get the priority of the currently set dictionary
	 * @return the priority of the dictionary
	 */
	public int getDictPriority() {
        Log.i(TAG, "PE.getDictPriority : "); // TODO - remove
		return m_dictInfo.listIdx;
	}
	
	/**
	 * Get the index of the currently set dictionary
	 * @return the index of the dictionary
	 */
	public int getDictIndex() {
        Log.i(TAG, "PE.getDictIndex : "); // TODO - remove
		return m_dictInfo.langIdx;
	}
	
	/**
	 * Get the name of the current dictionary set into m_dictInfo
	 * @return the name of the dictionary
	 */
	public String getDictName() {
        Log.i(TAG, "PE.getDictName : "); // TODO - remove
		return m_dictInfo.name;
	}
	
	/**
	 * Get whether the currently set dictionary is enabled or disabled
	 * @return whether the dictionary is enabled or disabled
	 */
	public boolean getDictEnabled() {
        Log.i(TAG, "PE.getDictEnabled : "); // TODO - remove
		return m_dictInfo.bEnabled;
	}
	
	public int getDictDataVersion() {
        Log.i(TAG, "PE.getDictDataVersion : "); // TODO - remove
		return m_dictInfo.dataVersion;
	}
	
	public int getDictCodeVersion() {
        Log.i(TAG, "PE.getDictCodeVersion : "); // TODO - remove
		return m_dictInfo.codeVersion;
	}
	
	public String[] getAutoAddedWords() {
        Log.i(TAG, "PE.getAutoAddedWords : "); // TODO - remove
		return m_addedWordsInfo.addedWordsAr;
	}
	
	public String getWordPrediction(int pos) 
	{
        Log.i(TAG, "PE.getWordPrediction : pos = " + pos); // TODO - remove
		if (pos >= m_nextWordsInfo.nPosWords())
			return null;
		return m_nextWordsInfo.nextWordsAr[pos];
	}
	
	public String getPhrasePrediction(int pos) 
	{
        Log.i(TAG, "PE.getPhrasePrediction : pos = " + pos); // TODO - remove
		if (pos >= m_nextWordsInfo.nPhraseWords)
			return null;
		return m_nextWordsInfo.phraseAr[pos];
	}
	
	public String getNounResults(int pos) 
	{
        Log.i(TAG, "PE.getNounResults : pos = " + pos); // TODO - remove
		if (pos >= m_NounInfo.nNounWords)
			return null;
		return m_NounInfo.NounAr[pos];
	}

	public String getRootText() {
        Log.i(TAG, "PE.getRootText : "); // TODO - remove
		String rootText = ntvGetRootText();
		return rootText;
	}
	
	public String undoWordOrLetter() {
        Log.i(TAG, "PE.undoWordOrLetter : "); // TODO - remove
		return ntvUndoLetterOrWord();
	}

	public String[] getSuggestions() {
        Log.i(TAG, "PE.getSuggestions : "); // TODO - remove
        if (m_nextWordsInfo != null && m_nextWordsInfo.nextWordsAr != null) {
            for (int i = 0 ; i < m_nextWordsInfo.nextWordsAr.length; i++) {
                Log.i(TAG, "PE.getSuggestions : m_nextWordsInfo.nextWordsAr[" + i + "] = |" + m_nextWordsInfo.nextWordsAr[i] + "|"); // TODO - remove
            }
        } else {
            Log.i(TAG, "PE.getSuggestions : NO suggestions"); // TODO - remove
        }
		return m_nextWordsInfo.nextWordsAr;
	}

    public String[] getPhraseSuggestions() {
        Log.i(TAG, "PE.getPhraseSuggestions : "); // TODO - remove
        return m_nextWordsInfo.phraseAr;
    }
    
    public String[] getNounSuggestions() {
        Log.i(TAG, "PE.getNounSuggestions : "); // TODO - remove
        return m_NounInfo.NounAr;
    }
	
	public String getRootWordNextWords() {
        Log.i(TAG, "PE.getRootWordNextWords : "); // TODO - remove
		return m_nextWordsInfo.rootWord;
	}
	
	public String getAdvanceWordPrintBuffer(String originalS) {
        Log.i(TAG, "PE.getAdvanceWordPrintBuffer : originalS = |" + originalS + "|"); // TODO - remove
		if (m_advanceWordInfo.printBuffer.length() == 0) {
			return "emptyAdvWordPrint on #" + originalS + "#";
		}
		return m_advanceWordInfo.printBuffer;
	}

	public String getAdvanceLetterPrintBuffer(String originalS) {
        Log.i(TAG, "PE.getAdvanceLetterPrintBuffer : originalS = |" + originalS + "|"); // TODO - remove
		if (m_advanceLetterInfo.printBuffer != null &&
			m_advanceLetterInfo.printBuffer.length() == 0) {
//			return "empty AdvanceLetter on #" + originalS + "#";
			Log.d("WLKB", "empty AdvanceLetter on #" + originalS + "#");
			// investigate setting printBuffer to S (rather copy characters into printBuffer).
			return originalS;
		}
		return m_advanceLetterInfo.printBuffer;
	}
		
	public boolean canWordBeAdded()	{
        Log.i(TAG, "PE.canWordBeAdded : "); // TODO - remove
		return this.ntvWordCanbeAdded();
	}
	
	public boolean addWord(String newWord, int dictIndex)	{
        Log.i(TAG, "PE.addWord : newWord = |" + newWord + "| dictIndex" + dictIndex); // TODO - remove
		return ntvAddWord(newWord, dictIndex);
	}
	public boolean deleteWord(String word)	{
        Log.i(TAG, "PE.deleteWord : word = |" + word + "|"); // TODO - remove
		return ntvDeleteWord(word);
	}
	public void setAutoLearn(boolean bAutoLearn) {
        Log.i(TAG, "PE.setAutoLearn : bAutoLearn = " + bAutoLearn); // TODO - remove
		ntvSetAutoLearn(bAutoLearn);
	}
	
	public String[] getPersonalDictWords() {
        Log.i(TAG, "PE.getPersonalDictWords : "); // TODO - remove
		ntvListWords(0, personalDictInfo);
		return personalDictInfo.nextWordsAr;
	}
	
	public int getNumWordsStartingWith(String word) {
        Log.i(TAG, "PE.getNumWordsStartingWith : word = |" + word + "|"); // TODO - remove
		return ntvNumWordsStartingWith(word);
	}
	
	public void setSpacelessTyping(boolean spacelessOn) {
        Log.i(TAG, "PE.setSpacelessTyping : spacelessOn = " + spacelessOn); // TODO - remove
		ntvSetSpacelessTyping(spacelessOn);
	}

    public void learnFromFile(String path) {
        Log.i(TAG, "PE.learnFromFile : path = |" + path + "|"); // TODO - remove
        ntvLearnFromFile(path);
    }

    public void setHistory(String words, boolean backspace) {
        Log.i(TAG, "PE.setHistory : words = |" + words + "| backspace = " + backspace); // TODO - remove
        ntvSetHistory(words,backspace);
    }

    public int SetStartingWords()
    {
        Log.i(TAG, "PE.SetStartingWords : "); // TODO - remove
        int numWords = ntvStartingWords(m_nextWordsInfo);
    	return numWords;
    }
    
    public void SetNounResult(String word)
    {
        Log.i(TAG, "PE.SetNounResult : word = |" + word + "|"); // TODO - remove
    	ntvGetNounResult(m_NounInfo, word);
    }
}
