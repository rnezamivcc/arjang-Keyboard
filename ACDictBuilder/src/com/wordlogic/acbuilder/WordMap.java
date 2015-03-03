package com.wordlogic.acbuilder;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;

public class WordMap {
	private HashMap<String,Word> mWords;
	private ArrayList<Integer> hashes;
	private HashMap<Integer,Word> hashmap;
	private ArrayList<Twogram> twogramlist;
	
	public WordMap(HashMap<String,Word> words) {
		mWords = words;
		hashmap = new HashMap<Integer,Word>();
		hashes = new ArrayList<Integer>(words.size());
			
		for(Word word : words.values()) {
			hashes.add(word.computeHash());
			hashmap.put(word.computeHash(),word);
		}
			
		Collections.sort(hashes);
	}
	
	public byte[] writeWordHashes() {
		byte[] ret = new byte[hashes.size()*4];
		ByteBuffer bb = ByteBuffer.wrap(ret);
		int c;
		
		bb.order(FileBuilder.byteOrder);
		
		bb.position(0);
		
		for(c = 0;c < hashes.size();c++) {
			hashmap.get(hashes.get(c)).hashoffset = c;
			bb.putInt(hashes.get(c));
		}
		
		return ret;
	}
	
	public void buildTwogramList() {
		int tgoffset = 0;
		
		twogramlist = new ArrayList<Twogram>();
		
		for(Word word : mWords.values()) {
			word.tgoffset = tgoffset;
			
			twogramlist.addAll(word.twograms);
			
			tgoffset += word.twograms.size();
		}
	}
	
	public byte[] writeTwoGramHeader() {
		ByteBuffer bb;
		int c;
		Word word;
		byte[] ret;
		
		ret = new byte[hashes.size()*8];
		
		bb = ByteBuffer.wrap(ret);
		
		bb.order(FileBuilder.byteOrder);
		
		bb.position(0);
		
		for(c = 0;c < hashes.size();c++) {
			word = hashmap.get(hashes.get(c));
			bb.putInt(word.tgoffset);
			bb.putInt(word.twograms.size());
		}
		
		return ret;
	}
	
	public byte[] writeTwograms() {
		ByteBuffer bb;
		int c;
		byte[] ret;
		
		ret = new byte[twogramlist.size()*12];
		
		bb = ByteBuffer.wrap(ret);
		
		bb.order(FileBuilder.byteOrder);
		
		bb.position(0);
		
		for(c = 0;c < twogramlist.size();c++) {
			bb.putInt(twogramlist.get(c).w1.computeHash());
			bb.putInt(twogramlist.get(c).w2.computeHash());
			bb.putInt(twogramlist.get(c).freq);
		}
		
		return ret;
	}
	
	public byte[] writeProbs() {
		byte[] ret;
		int c;
		Word word;
		int newbyte;
		
		ret = new byte[hashes.size()];
		
		for(c = 0;c < hashes.size();c++) {
			word = hashmap.get(hashes.get(c));
			
			newbyte = word.prob;
			
			assert(newbyte <= 63 && newbyte >= 0);
			
			if(word.accronym) {
				newbyte += 64;
			}
			
			if(word.propernoun) {
				newbyte += 128;
			}
			
			ret[c] = (byte)newbyte;
		}
		
		return ret;
	}
}