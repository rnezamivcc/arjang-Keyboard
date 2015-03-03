package com.iknowu.miniapp;

import android.app.Service;
import android.content.Intent;
import android.os.Binder;
import android.os.IBinder;
import android.os.Message;
import android.os.RemoteException;
import android.util.Log;
import android.widget.RemoteViews;

import com.iknowu.IKnowUKeyboardService;

public class MiniAppMessageReceiver extends Service {
	
	private static final String TAG = "MiniAppMessageReceiver";
	
	public static final String LOCAL_BIND = "com.iknowu.miniapp.LOCAL_BIND";
	
	public RemoteViews updatedView;
	public int animNum;
	public String clipText;
	
	public static final String CLIP_KEY = "com.iknowu.CLIP_KEY";
	public static final String ANIM_KEY = "com.iknowu.ANIM_KEY";
	
	public boolean isBound;
	
	private IKnowUKeyboardService inputService;
	
	@Override
    public IBinder onBind( Intent intent ) {
		Log.d(TAG, "On bind action = "+intent.getAction());
		if (intent.getAction().equals(LOCAL_BIND)) {
			return localBinder;
		} else {
			return binder;
		}
    }
	
	@Override
	public boolean onUnbind(Intent intent) {
		Log.d(TAG, "On unbind action = "+intent.getAction());
		this.isBound = false;
		return super.onUnbind(intent);
	}
	
	public class LocalBinder extends Binder {
		public MiniAppMessageReceiver getService() {
			return MiniAppMessageReceiver.this;
		}
	}
	
	// This is the object that receives interactions from clients.  See
    // RemoteService for a more complete example.
    private final IBinder localBinder = new LocalBinder();
	
	private final IKnowUKeyboardInterface.Stub binder = new IKnowUKeyboardInterface.Stub() {
		public void sendText(String param, int before, int after, boolean stayAlive) throws RemoteException {
			sendTextToEditor(param, before, after, stayAlive);
		}

		@Override
		public void updateView(RemoteViews rm, int anim) throws RemoteException {
            //TODO:
			updateRemoteView( rm, anim );
		}

		@Override
		public void clip(String param) throws RemoteException {
            //TODO:
			copyToClipboard(param);
		}

		@Override
		public void close() throws RemoteException {
            //TODO:
			closeMiniApp();
		}

		@Override
		public void deleteChars(int before, int after)
				throws RemoteException {
			deleteCharacters(before, after);
		}
	};
	
	private void deleteCharacters(int before, int after) {
		if (this.inputService != null) {
			this.inputService.deleteChars(before, after);
			/*
			Message msg = inputService.miniAppHandler.obtainMessage(IKnowUKeyboardService.MSG_DELETE_CHARS);
			msg.arg1 = before;
			msg.arg2 = after;
			inputService.miniAppHandler.sendMessage(msg);
			*/
		}
	}
	
	private void closeMiniApp() {
		if (this.inputService != null) {
			Message msg = inputService.miniAppHandler.obtainMessage(IKnowUKeyboardService.MSG_CLOSE_MINIAPP);
			inputService.miniAppHandler.sendMessage(msg);
		}
	}
	
	private void copyToClipboard(String text) {
		Log.v(TAG, "Sending clip text message");
		
		this.clipText = text;
		
		if (this.inputService != null) {
			Message msg = inputService.miniAppHandler.obtainMessage(IKnowUKeyboardService.MSG_CLIP_TEXT);
			inputService.miniAppHandler.sendMessage(msg);
		}
		//Intent i = new Intent(IKnowUKeyboardService.CLIP_TEXT);
		//i.putExtra(CLIP_KEY, text);
		//startService(i);
	}
	
	private void updateRemoteView( RemoteViews rm, int anim ) {
		updatedView = rm;
		animNum = anim;
		
		Log.v(TAG, "updating remote view");
		
		if (inputService != null) {
			Message msg = inputService.miniAppHandler.obtainMessage(IKnowUKeyboardService.MSG_UPDATE_MINIAPP_VIEW);
			inputService.miniAppHandler.sendMessage(msg);
		}
		
		//Intent i = new Intent(IKnowUKeyboardService.UPDATE_MINI_APP);
		//i.putExtra(ANIM_KEY, anim);
		
		//startService(i);
	}
	
	private void sendTextToEditor(String text, int before, int after, boolean stayAlive) {
		IKnowUKeyboardService.log(Log.VERBOSE, TAG, "SendText = "+text+", inputService = "+this.inputService);
		if (this.inputService != null) {
			this.inputService.sendTextToEditor(text, before, after, stayAlive);
		}
	}
	
	public void setKeyboardService(IKnowUKeyboardService service) {
		this.inputService = service;
	}
}
