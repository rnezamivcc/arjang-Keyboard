package com.iknowu.autocorrect;

public class Prediction {
	public final String word;
	/** Ordinal probability of this word in the dictionary [ 0 , AutoCorrect.wordCount() ) */
	public final int prob;
	/** Levenshtein distance between this word and the search text */
	public final int lev;
	/** Keyboard distance between this word and the search text */
	public final float kbd;
	/** 2-gram probability for this word based on the preceeding text */
	public final float ngram;
	/** True if the first letter of this word matches the search text, false otherwise */
	public final boolean fletter;
	
	public Prediction(String w,int p,int l,float k,float n,boolean f) {
		prob = p;
		lev = l;
		kbd = k;
		ngram = n;
		fletter = f;
		word = w;
	}
	
	public String toString() {
		return word + " (prob = "+Integer.toString(prob) + "; lev = "+Integer.toString(lev) + "; ngram = "+Float.toString(ngram)+")";
	}
}