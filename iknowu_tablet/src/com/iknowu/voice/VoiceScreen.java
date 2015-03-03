package com.iknowu.voice;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.graphics.Paint;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.speech.RecognitionListener;
import android.speech.RecognizerIntent;
import android.speech.SpeechRecognizer;
import android.util.AttributeSet;
import android.util.Log;
import android.util.TypedValue;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TextView;

import com.iknowu.IKnowUKeyboardService;
import com.iknowu.KeyboardContainerView;
import com.iknowu.R;
import com.iknowu.sidelayout.SideRelativeLayout;
import com.iknowu.sidelayout.SideScreen;
import com.iknowu.sidelayout.SideTab;
import com.iknowu.util.Theme;

import java.util.ArrayList;
import java.util.List;

/**
 * Created by Justin on 15/11/13.
 *
 */
public class VoiceScreen extends SideScreen implements View.OnClickListener {

    private static final String TAG = "VOICE INPUT LISTENER";

    private static final int COLUMN_WIDTH = 150;
    private static final int VOICE_BUTTON_ID = 98765;

    private boolean setupComplete;
    //private ScrollView myView;
    private Context context;
    private int width;
    private VoiceInputAdapter listAdapter;
    private ListView voiceInputList;

    private ImageView voiceStartButton;
    private TextView buttonText;

    private SpeechRecognizer speechRecognizer;
    private boolean voiceInputStarted;
    private ArrayList<String> resultList;

    private Paint paint;

    public VoiceScreen(Context context) {
        super(context);
        this.context = context;
        this.paint = new Paint();
    }

    public VoiceScreen(Context context, AttributeSet attrs) {
        super(context, attrs);
        this.context = context;
        this.paint = new Paint();
    }

    public VoiceScreen(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        this.context = context;
        this.paint = new Paint();
    }

    /*@Override
    public void onDraw(Canvas canvas) {
        super.onDraw(canvas);

        this.paint.setColor(Color.GREEN);
        this.paint.setStyle(Paint.Style.STROKE);
        this.paint.setStrokeWidth(5);

        canvas.drawRect(0, 0, this.getWidth(), this.getHeight(), paint);
    }*/

    @Override
    public void init(IKnowUKeyboardService serv, KeyboardContainerView kbcv, SideRelativeLayout sfl, boolean islefty, int index) {
        //this.ikScroller = scroll;

        super.init(serv, kbcv, sfl, islefty, index);

        LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT);
        LinearLayout linlay = new LinearLayout(this.context);
        linlay.setOrientation(LinearLayout.VERTICAL);
        linlay.setLayoutParams(params);
        linlay.setBackgroundColor(Theme.SIDE_BACKGROUND_COLOR);

        params = new LinearLayout.LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.WRAP_CONTENT);
        params.gravity = Gravity.CENTER;
        LinearLayout buttonlin = new LinearLayout(this.context);
        buttonlin.setOrientation(LinearLayout.HORIZONTAL);
        buttonlin.setLayoutParams(params);
        buttonlin.setGravity(Gravity.CENTER);
        buttonlin.setPadding(10,10,10,10);
        buttonlin.setId(VOICE_BUTTON_ID);
        buttonlin.setOnClickListener(this);
        buttonlin.setBackgroundColor(Theme.KEY_DARK_COLOR);

        params = new LinearLayout.LayoutParams(LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT);
        this.voiceStartButton = new ImageView(this.context);
        this.voiceStartButton.setLayoutParams(params);
        this.voiceStartButton.setImageResource(R.drawable.mic_icon_start);
        buttonlin.addView(this.voiceStartButton);

        params = new LinearLayout.LayoutParams(LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT);
        this.buttonText = new TextView(this.context);
        this.buttonText.setLayoutParams(params);
        this.buttonText.setText("Tap to start");
        this.buttonText.setTextSize(TypedValue.COMPLEX_UNIT_SP, 24);
        this.buttonText.setTextColor(Theme.KEY_TEXT_COLOR);
        buttonlin.addView(this.buttonText);

        this.voiceInputList = new ListView(this.context);
        this.voiceInputList.setBackgroundColor(Theme.SIDE_BACKGROUND_COLOR);
        params = new LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.MATCH_PARENT);
        this.voiceInputList.setLayoutParams(params);

        linlay.addView(buttonlin);
        linlay.addView(this.voiceInputList);

        /*this.backToKeyboardButton = (ImageView) this.findViewById(R.id.switch_to_keyboard_button);
        this.backToKeyboardButton.setOnClickListener(this);*/

        this.speechRecognizer = SpeechRecognizer.createSpeechRecognizer(this.context);
        this.speechRecognizer.setRecognitionListener(new Listener());

        this.tab = new SideTab(this.context);
        this.tab.init(this);
        this.tab.setImageResource(this.isLefty ? R.drawable.tab_voice_lefty : R.drawable.tab_voice);
        params = new LinearLayout.LayoutParams(LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT);
        this.tab.setLayoutParams(params);

        //this.setTabClickListeners();

        /*this.listView = new ListView(this.context);
        *//*this.listView.setColumnWidth(COLUMN_WIDTH);
        this.listView.setNumColumns(3);
        this.listView.setVerticalSpacing(5);
        this.listView.setHorizontalSpacing(5);
        this.listView.setStretchMode(GridView.STRETCH_COLUMN_WIDTH);*//*
        this.listView.setBackgroundColor(Theme.SIDE_BACKGROUND_COLOR);
        params = new LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.MATCH_PARENT);
        this.listView.setLayoutParams(params);
        //this.listView.setGravity(Gravity.CENTER);*/

        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(this.context);
        this.setupComplete = sp.getBoolean(IKnowUKeyboardService.PREF_SETUP_COMPLETE, false);

        this.contentView = linlay;

        if (this.isLefty) {
            this.initLefty();
        } else {
            this.initRighty();
        }

        disableEnableScreen(this.isSelected);
    }

    @Override
    public void disableEnableScreen(boolean enable) {
        super.disableEnableScreen(enable);
        this.contentView.setEnabled(enable);
        disableEnableChildControls(enable, (ViewGroup)this.contentView);
        this.contentView.setVisibility((enable) ? View.VISIBLE : View.INVISIBLE);
    }

    private void initRighty() {
        this.addView(this.tab);
        this.addView(this.contentView);
    }

    private void initLefty() {
        this.addView(this.contentView);
        this.addView(this.tab);
    }

    /**
     * Handle open/close state of voice screen.
     * @param isOpen the state flag
     */
    @Override
    public void setOpenState(boolean isOpen) {
        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(this.context);
        SharedPreferences.Editor edit = sp.edit();
        edit.putString(SideRelativeLayout.OPEN_SIDE_SCREEN, (isOpen) ? SideRelativeLayout.VOICE_SCREEN : "");
        edit.commit();
    }

    @Override
    public void onMeasure( int widthMeasureSpec, int heightMeasureSpec) {
        super.onMeasure(widthMeasureSpec, heightMeasureSpec);
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
        this.voiceStartButton.setImageResource(R.drawable.mic_icon_stop);
        this.buttonText.setText("Tap to stop");
        Intent intent = new Intent(RecognizerIntent.ACTION_RECOGNIZE_SPEECH);
        intent.putExtra(RecognizerIntent.EXTRA_LANGUAGE_MODEL, RecognizerIntent.LANGUAGE_MODEL_FREE_FORM);
        intent.putExtra(RecognizerIntent.EXTRA_CALLING_PACKAGE, "com.iknowu.voice");

        intent.putExtra(RecognizerIntent.EXTRA_MAX_RESULTS, 5);
        this.speechRecognizer.startListening(intent);
        this.voiceInputStarted = true;
    }

    public void stopVoiceInput(boolean fromUser) {
        this.voiceStartButton.setImageResource(R.drawable.mic_icon_start);
        this.buttonText.setText("Tap to start");
        if (fromUser)
            this.speechRecognizer.stopListening();
        this.voiceInputStarted = false;
    }

    @Override
    public void onClick(View v) {
        //Log.d("ONCLICK", "view = "+v);
        if (this.sideRelativeLayout.lastSelected.equals(this)) {
            if (v.getId() == VOICE_BUTTON_ID) {
                if (this.voiceInputStarted) {
                    this.stopVoiceInput(true);
                } else {
                    this.startVoiceInput();
                }
            }/* else if (v.getId() == R.id.switch_to_keyboard_button) {
                if (this.voiceInputStarted) {
                    this.stopVoiceInput(true);
                }
                //this.inputService.switchToKeyboard();
            }*/
        }
    }

    public class VoiceInputAdapter extends ArrayAdapter<String> {

        public VoiceInputAdapter(Context context, int textViewResourceId, List<String> objects) {
            super(context, textViewResourceId, objects);
        }

        public View getView(final int position, View convertView, ViewGroup parent) {
            SelectableListItem theView = new SelectableListItem(context);
            theView.setText(resultList.get(position));
            theView.setService(keyboardService);
            theView.setListAdapter(this);
            return theView;
        }
    }
}
