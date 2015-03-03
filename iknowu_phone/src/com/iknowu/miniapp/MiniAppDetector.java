package com.iknowu.miniapp;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

import com.iknowu.IKnowUKeyboardService;

public class MiniAppDetector extends BroadcastReceiver {
	
	private static final String LOG_TAG = "MiniAppDetector";
	
	private IKnowUKeyboardService inputService;
	
	public MiniAppDetector() {
		super();
	}
	
	@Override
	public void onReceive(Context context, Intent intent) {
		Log.d( LOG_TAG, "onReceive: "+intent );
		this.checkForMiniapps();
	}
	
	private void checkForMiniapps() {
		this.inputService.checkForMiniApps();
	}
	
	public void setKeyboardService(IKnowUKeyboardService service) {
		this.inputService = service;
	}
}
