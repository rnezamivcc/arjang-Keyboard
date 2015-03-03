package com.iknowu.cloud;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

import com.iknowu.IKnowUKeyboardService;
import com.iknowu.PredictionEngine;

import org.json.JSONException;
import org.json.JSONObject;

import java.util.Iterator;

/**
 * Receive a push broadcast from the parse server.
 * 
 * @author Justin Desjardins
 *
 */
public class PushReceiver extends BroadcastReceiver {
	private static final String TAG = "PushReceiver";
	private PredictionEngine predictionEngine;
	
	/**
	 * Upon receiving a push from the parse server. Parse the json string provided by parse to and perform any required functions
	 */
	@Override
	public void onReceive(Context context, Intent intent) {
		try {
			predictionEngine = IKnowUKeyboardService.getPredictionEngine();
			
			String action = intent.getAction();
			String channel = intent.getExtras().getString("com.parse.Channel");
			JSONObject json = new JSONObject(intent.getExtras().getString("com.parse.Data"));
			
			IKnowUKeyboardService.log(Log.DEBUG, TAG, "received action " + action + " on channel " + channel + " with extras:");
			
			IKnowUKeyboardService.log(Log.DEBUG, TAG, "PARSE PUSH JSON OBJECT = " + json.toString());
			
			Iterator itr = json.keys();
			
			while (itr.hasNext()) {
				String key = (String) itr.next();
				if (action.equals("com.keyboard.ADD_WORD")) {
					if (key.substring(0, 4).equals("word")) {
						IKnowUKeyboardService.log(Log.DEBUG, TAG, "ADD WORD = " + json.getString(key));
						if (this.predictionEngine != null) {
							this.predictionEngine.addWord(json.getString(key), 1);
						}
					}
				} else if (action.equals("com.keyboard.DELETE_WORD")) {
					if (key.substring(0, 4).equals("word")) {
						if (this.predictionEngine != null) {
							this.predictionEngine.deleteWord(json.getString(key));
						}
					}
				}
				IKnowUKeyboardService.log(Log.DEBUG, TAG, "..." + key + " => " + json.getString(key));
			}
		} catch (JSONException e) {
			IKnowUKeyboardService.log(Log.ERROR, TAG, "JSONException: " + e.getMessage());
			IKnowUKeyboardService.sendErrorMessage(e);
		}
	}
}