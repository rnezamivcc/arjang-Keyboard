package com.iknowu.miniapp;

interface IKnowUKeyboardInterface {
	/**
	 * Send a String of text to the keyboard,
	 * that will be posted to the input box upon being received
	 * @param param: The text you would like to put into the EditText field
	 */
	void sendText(String param, int before, int after, boolean stayAlive);
	
	/**
	 * Tell the keyboard that your RemoteViews object needs updating
	 * @param rm: Your updated RemoteViews object that will be displayed
	 */
	void updateView( in RemoteViews rm, int anim );
	
	/**
	 * Tell the keyboard that you want to post some text to the ClipBoard
	 * @param param: The text you would like to clip
	 */
	void clip( String param );
	
	/**
	 * Tell the keyboard that you are done and would like it to close your connection
	 * as well as the mini-app drawer.
	 */
	void close();
	
	/**
	 * Delete a specified number of characters from the text.
	 * @param before: the number of characters to delete before the current cursor position
	 * @param afterCursor: the number of characters to delete after the current cursor position
	 */
	 void deleteChars(int before, int after);
}