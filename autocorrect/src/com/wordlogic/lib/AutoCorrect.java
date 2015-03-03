package com.wordlogic.lib;

import java.io.File;

public class AutoCorrect {
	public AutoCorrectInterface callback;
	
	public AutoCorrect(File acdFile,File personalFile) {
		init(acdFile.getAbsolutePath(),personalFile.getAbsolutePath());
	}
	
	public native void addChar(char ch);
	public void backspace() {
		backspace(1);
	}
	public native void backspace(int nchars);
	public native void addString(String s);
	
	public native void setBeforeCursor(String s);
	
	public native void reverseCorrection(int si,int ei,String autocorrect,String rep);
	
	public native void saveUpdates();
	
	/**
	 * Load a keyboard layout into the auto-correct system which will be used to assist with the correction of errors
	 * caused by near-misses in key-presses (eg. typing thw instead of the on a QWERTY keyboard).
	 * 
	 * @param keypos
	 */
	public native void loadKeyboardLayout(KeyPosition[] keypos);
	
	private static native void init(String filename,String personal);
	
	public native float testKeyDist(char ch1,char ch2);
	
	static {
		System.loadLibrary("autocorrect");
	}
}