package com.iknowu.popup;

import java.util.ArrayList;

/**
 * Contains a list of keys to be displayed in a {@link PopupKeyboardView} as
 * well as a default character to be displayed and the character on the main keyboard
 * that will trigger this popup
 * 
 * @author Justin
 *
 */
public class PopupKeyboard {
	
	private String theChar;
	private ArrayList<String> keyChars;
	private String defaultChar;
	
	public PopupKeyboard( String chr ) {
		this.theChar = chr;
		this.keyChars = new ArrayList<String>();
	}
	
	/**
	 * Add a key to this keyboard
	 * @param chr that character of the key
	 * @param isDefault if it is the default character or not
	 */
	public void addKeyChar( String chr, boolean isDefault ) {
		this.keyChars.add(chr);
		
		if (isDefault) this.defaultChar = chr;
	}
	
	/**
	 * Get the characters of this keyboard
	 * 
	 * @return the list of characters as a String
	 */
	public ArrayList<String> getKeyChars() {
		return this.keyChars;
	}
	
	/**
	 * Get the main character that will trigger this keyboard
	 * 
	 * @return the main character
	 */
	public String getMainChar() {
		return this.theChar;
	}
	
	/**
	 * Get the default character of this keyboard
	 * 
	 * @return the default character
	 */
	public String getDefaultChar() {
		return this.defaultChar;
	}
}
