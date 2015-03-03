package com.iknowu.voice;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.speech.RecognitionListener;
import android.speech.RecognizerIntent;
import android.speech.SpeechRecognizer;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ListView;

import com.iknowu.IKnowUKeyboardService;
import com.iknowu.R;

import java.util.ArrayList;
import java.util.List;

public class VoiceInputLinearLayout extends LinearLayout implements OnClickListener {
	
	private static final String TAG = "VOICE INPUT LISTENER";
	private Context context;
	private IKnowUKeyboardService inputService;
	private VoiceInputAdapter listAdapter;
	private ListView voiceInputList;
	private ImageView button;
	private ImageView backToKeyboardButton;
	private SpeechRecognizer speechRecognizer;
	private boolean voiceInputStarted;
	private ArrayList<String> resultList;
	
	public VoiceInputLinearLayout(Context context) {
		super(context);
		this.context = context;
	}
	
	public VoiceInputLinearLayout(Context context, AttributeSet attrs) {
		super(context, attrs);
		this.context = context;
	}
	
	/*
	public VoiceInputLinearLayout(Context context, AttributeSet attrs, int defStyle) {
		super(context, attrs, defStyle);
		this.context = context;
	}*/
	
	public void init() {
		this.setBackgroundColor(0xff000000);
		this.voiceInputList = (ListView) this.findViewById(R.id.voice_input_list);
		
		this.button = (ImageView) this.findViewById(R.id.voice_input_button);
		this.button.setOnClickListener(this);
		
		/*this.backToKeyboardButton = (ImageView) this.findViewById(R.id.switch_to_keyboard_button);
		this.backToKeyboardButton.setOnClickListener(this);*/
		
		this.speechRecognizer = SpeechRecognizer.createSpeechRecognizer(this.context);
		this.speechRecognizer.setRecognitionListener(new Listener());
	}
	
	public void setInputService(IKnowUKeyboardService service) {
		this.inputService = service;
	}
	
	class Listener implements RecognitionListener {
		public void onReadyForSpeech(Bundle params) {
			//Log.d(TAG, "onReadyForSpeech");
		}

		public void onBeginningOfSpeech() {
			//Log.d(TAG, "onBeginningOfSpeech");
		}

		public void onRmsChanged(float rmsdB) {
			//Log.d(TAG, "onRmsChanged");
		}

		public void onBufferReceived(byte[] buffer) {
			//Log.d(TAG, "onBufferReceived");
		}

		public void onEndOfSpeech() {
			IKnowUKeyboardService.log(Log.DEBUG, TAG, "onEndofSpeech");
		}

		public void onError(int error) {
			IKnowUKeyboardService.log(Log.DEBUG, TAG, "error " + error);
			//mText.setText("error " + error);
		}

		public void onResults(Bundle results) {
			IKnowUKeyboardService.log(Log.DEBUG, TAG, "onResults " + results);
			resultList = results.getStringArrayList(SpeechRecognizer.RESULTS_RECOGNITION);
			
			listAdapter = new VoiceInputAdapter(context, R.layout.selectable_list_item, resultList);
			voiceInputList.setAdapter(listAdapter);
			
			stopVoiceInput(false);
		}

		public void onPartialResults(Bundle partialResults) {
			IKnowUKeyboardService.log(Log.DEBUG, TAG, "onPartialResults");
		}

		public void onEvent(int eventType, Bundle params) {
			//Log.d(TAG, "onEvent " + eventType);
		}
	}
	
	public void startVoiceInput() {
		this.button.setImageResource(R.drawable.mic_icon_stop);
		Intent intent = new Intent(RecognizerIntent.ACTION_RECOGNIZE_SPEECH);
		intent.putExtra(RecognizerIntent.EXTRA_LANGUAGE_MODEL, RecognizerIntent.LANGUAGE_MODEL_FREE_FORM);
		intent.putExtra(RecognizerIntent.EXTRA_CALLING_PACKAGE, "com.iknowu.voice");
		
		intent.putExtra(RecognizerIntent.EXTRA_MAX_RESULTS, 5);
		this.speechRecognizer.startListening(intent);
		this.voiceInputStarted = true;
	}
	
	public void stopVoiceInput(boolean fromUser) {
		this.button.setImageResource(R.drawable.mic_icon_start);
		if (fromUser)
			this.speechRecognizer.stopListening();
		this.voiceInputStarted = false;
	}

	@Override
	public void onClick(View v) {
		//Log.d("ONCLICK", "view = "+v);
		if (v.getId() == R.id.voice_input_button) {
			if (this.voiceInputStarted) {
				this.stopVoiceInput(true);
			} else {
				this.startVoiceInput();
			}
		}/* else if (v.getId() == R.id.switch_to_keyboard_button) {
			if (this.voiceInputStarted) {
				this.stopVoiceInput(true);
			}
			this.inputService.switchToKeyboard();
		}*/
	}
	
	private class VoiceInputAdapter extends ArrayAdapter<String> {
		
		public VoiceInputAdapter(Context context, int textViewResourceId, List<String> objects) {
			super(context, textViewResourceId, objects);
		}

		public View getView(final int position, View convertView, ViewGroup parent) {
			SelectableListItem theView = new SelectableListItem(context);
			theView.setText(resultList.get(position));
			theView.setService(inputService);
			return theView;
		}
	}
}
