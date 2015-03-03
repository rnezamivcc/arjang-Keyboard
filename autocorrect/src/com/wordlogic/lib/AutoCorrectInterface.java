package com.wordlogic.lib;

public interface AutoCorrectInterface {
	/****
	 * Called when a correction is made by the auto-correct system with information about the word that was replaced, its position,
	 * and the new word.<br>
	 * 
	 * For example, consider the following corrections:
	 * <ul>
	 * <li>On a buffer that contains "twe" corrected to "the"<br>
	 *	correctionMade(2,-1,"twe","the");</li>
	 *
	 * <li>On a buffer that contains "whay " corrected to "what "<br>
	 * 	&#09;correctionMade(4,0,"whay","what");</li>
	 * 
	 * <li>On a buffer that contains "hoe are " corrected to "how are "<br>
	 *  &#09;correctionMade(7,4,"hoe","how");</li>
	 * </ul>
	 * 
	 * @param si			the start index for the replacement relative to the last character in the buffer
	 * @param ei			the end index for the replacement, the index of the character one after the replacement.
	 * @param previous		the word that was replaced
	 * @param replacement	the word that it was replaced with
	 * 
	 */
	public void correctionMade(int si,int ei,String previous,String replacement);
}