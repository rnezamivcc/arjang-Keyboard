package com.iknowu.miniapp;

import android.widget.RemoteViews;

interface IKnowUMiniAppInterface {
    /**
     * Called by the keyboard when it needs to display the Mini-apps view
     * this is usually when a user has clicked on the apps icon in the action bar.
     */
	RemoteViews getView(String packageName, String param, String category);

	RemoteViews getSmallIcon();
	RemoteViews getLargeIcon();

	/**
	 * Called by the keyboard when it is about to close the Mini-app
	 * you can rely on this function to be called to do any finilizations
	 */
	void onFinishConnection();
}