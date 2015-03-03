package com.wordlogic.lib;


public class AutoCorrect {
	public AutoCorrect() {}
	
	public native void replaceBuffers(String beforeCursor,String afterCursor);
	
	public native void reverseCorrection(int si,String autocorrect,String rep);
	
	
	/**
	 * Default is (true,true)
	 */
//	public native void setSettings(boolean correctvalid,boolean correctcase);
	
	/**
	 * Determine the second auto-correct prediction for a word after the first set of predictions has been made
	 * 
	 * @param si
	 * @param original
	 * @param first
	 * @return
	 */
//	public native String getSecondCorrection(int si,String original,String firstautocorrect);
	
//	public native void saveUpdates();
	
	/**
	 * Load a keyboard layout into the auto-correct system which will be used to assist with the correction of errors
	 * caused by near-misses in key-presses (eg. typing thw instead of the on a QWERTY keyboard).
	 * 
	 * @param keypos
	 */
	public native void loadKeyboardLayout(KeyPosition[] keypos);
	
}