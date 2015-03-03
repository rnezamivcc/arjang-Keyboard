package com.wordlogic.acbuilder;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;

public class Word {
	public final String chars;
	public int prob;
	public ArrayList<Twogram> twograms;
	public int hashoffset;
	public int tgoffset;
	private boolean hashed;
	private int hash;
	final boolean accronym;
	final boolean propernoun;
	int c;
	public static int longest = 0;
	public static String longestWord = null;
	
	public Word(String w,int p) {
		boolean alluppercase;
		
		chars = w.toLowerCase();
		prob = p;
		twograms = new ArrayList<Twogram>();
		hashed = false;
		
		alluppercase = true;
		
		for(c = 0;c < w.length();c++) {
			if(Character.isLowerCase(w.charAt(c))) {
				alluppercase = false;
				break;
			}
		}
		
		if(alluppercase) {
			accronym = true;
			propernoun = false;
		} else {
			accronym = false;
			if(Character.isUpperCase(w.charAt(0))) {
				propernoun = true;
			} else {
				propernoun = false;
			}
		}
		
		if(chars.length() > longest) {
			longest = chars.length();
			longestWord = chars;
		}
	}
	
	public int computeHash() {
		int c;
		
		if(hashed) { return hash; }
		
		hash = 5381;
		
		for(c = 0;c < chars.length();c++) {
			if(chars.charAt(c) == '\'') { hash++; continue; }
			
			hash = (hash << 5) + hash;
			hash += chars.charAt(c);
		}
		
		hashed = true;
		
		return hash;
	}
}