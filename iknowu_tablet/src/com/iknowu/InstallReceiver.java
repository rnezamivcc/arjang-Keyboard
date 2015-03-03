package com.iknowu;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

import com.parse.Parse;
import com.parse.ParseObject;

import java.net.URLDecoder;

import com.iknowu.R;

/**
 * BroadcastReceiver that tracks when and how this application has been installed and logs this to the cloud
 * @author Justin
 *
 */
public class InstallReceiver extends BroadcastReceiver {

	@Override
	public void onReceive(Context context, Intent intent) {
		try {
			//initialize parse to connect with the application
			Parse.initialize(context, "j17LUua9VZ96CCEAP1QKimCoUuGlOfKRYnyLoqWv", "C7QJdvSFey3DwfZSCSqMaC2hjWFZwvQCr9ruiOoE");
			
			String referrer = intent.getStringExtra("referrer");
			if (referrer != null) {
				referrer = URLDecoder.decode(referrer);
			} else {
				referrer = "No Referrer";
			}
			
			if (referrer != null && referrer.length() > 0) {
				ParseObject pObject = new ParseObject("Tracking");
	        	pObject.put("iKnowUVersion", context.getResources().getString(R.string.version_name));
	        	pObject.put("Referrer", referrer);
	        	pObject.saveInBackground();
			} else {
				ParseObject pObject = new ParseObject("Tracking");
	        	pObject.put("iKnowUVersion", context.getResources().getString(R.string.version_name));
	        	pObject.put("Referrer", "no referrer!!!!");
	        	pObject.saveInBackground();
			}
		} catch (Exception e) {
			IKnowUKeyboardService.sendErrorMessage(e);
		}
	}
}
