package com.iknowu.autocorrect;

import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;

import android.content.Context;
import android.content.res.AssetManager;


public class AutoCorrect {
	
	ByteBuffer bufACD;
	
	public AutoCorrect(Context context) throws IOException {
		InputStream insACD;
		byte[] buf;
		AssetManager assetMgr = context.getAssets();
		int c;
		
		insACD = assetMgr.open("english.acd");
		
		buf = new byte[514864];
		
		insACD.read(buf);
		
		insACD.close();
		
		bufACD = ByteBuffer.allocateDirect(514864);
		bufACD.position(0);
		
		for(c = 0;c < 514864;c++) {
			bufACD.put(buf[c]);
		}
	}
	
	/**
	 * Info on auto-correct sorting methods:
	 * 1 - score adjusted sort
	 * 2 - sort first by levenshtein ascending, then by priority descending
	 * 3 - sort by adjusted score with ngrams up front
	 * 4 - sort first by ngram, then by rule #2
	 * 
	 */
	
	/**
	 * Auto-correct a string
	 * 
	 * @param s			the string to auto-correct
	 * @param l			the maximum number of results to return
	 * @param msort		the sorting method to be used (see above)
	 * @return
	 */
	public ResultSet correct(String s,int l,int msort) {
		ResultSet rs = new ResultSet();
		
		correct(null,s,l,rs,bufACD,msort);
		
		return rs;
	}
	
	/**
	 * Auto-correct a string
	 * 
	 * @param s			the string to auto-correct
	 * @param l			the maximum number of results to return
	 * @return
	 */
	public ResultSet correct(String s,int l) {
		ResultSet rs = new ResultSet();
		
		correct(null,s,l,rs,bufACD,3);
		
		return rs;
	}
	
	/**
	 * Auto-correct a string, taking into account the previous word
	 * 
	 * @param pre		the previous word
	 * @param s
	 * @param l
	 * @return
	 */
	public ResultSet correct(String pre,String s,int l) {
		ResultSet rs = new ResultSet();
		
		correct(pre,s,l,rs,bufACD,3);
		
		return rs;
	}
	
	/**
	 * 
	 * @param pre
	 * @param s
	 * @param l
	 * @param msort
	 * @return
	 */
	public ResultSet correct(String pre,String s,int l,int msort) {
		ResultSet rs = new ResultSet();
		
		correct(pre,s,l,rs,bufACD,msort);
		
		return rs;
	}
	
	/**
	 * Check if a word is in the auto-correct dictionary
	 * 
	 * @param s			the word to check for
	 * @return
	 */
	public boolean isWord(String s) {
		return isWord(s,bufACD);
	}
	
	/**
	 * Check if auto-correct should be applied to a word string.
	 * @param s
	 * @return
	 */
	public boolean shouldCorrect(String s) {
		if(s.matches("[_A-Za-z0-9-]+(\\.[_A-Za-z0-9-]+)*@[A-Za-z0-9]+(\\.[A-Za-z0-9]+)*(\\.[A-Za-z]{2,})")) { // e-mail
			return false;
		} else if(s.matches("[0-9-.]+")) { // number
			return false;
		} else if(isWord(s)) { // valid word
			return false;
		} else {
			return true;
		}
	}
	
	/**
	 * For debugging
	 */
	public void walk() {
		walkGraph(bufACD);
	}
	
	public void twoGramTest(String pre) {
		twoGramInfo(pre,bufACD);
	}
	
	private static native boolean isWord(String s,ByteBuffer dbuf);
	private static native void walkGraph(ByteBuffer dbuf);
	
	private static native void correct(String pre,String s,int l,ResultSet rs,ByteBuffer dbuf,int msort);
	private static native void twoGramInfo(String pre,ByteBuffer dbuf);
	
	static {
		System.loadLibrary("autocorrect-jni");
	}
}