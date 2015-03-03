package com.iknowu.voice;

import android.content.Context;
import android.util.AttributeSet;
import android.util.TypedValue;
import android.view.MotionEvent;
import android.view.inputmethod.InputConnection;
import android.widget.TextView;

import com.iknowu.IKnowUKeyboardService;
import com.iknowu.util.Theme;

public class SelectableListItem extends TextView {
	
	private IKnowUKeyboardService service;
    private VoiceScreen.VoiceInputAdapter adapter;

	public SelectableListItem(Context context) {
		super(context);
		this.init();
	}
	
	public SelectableListItem(Context context, AttributeSet attrs) {
		super(context, attrs);
		this.init();
	}
	
	public SelectableListItem(Context context, AttributeSet attrs, int defStyle) {
		super(context, attrs, defStyle);
		this.init();
	}
	
	private void init() {
		this.setTextColor(Theme.KEY_TEXT_COLOR);
		this.setTextSize(TypedValue.COMPLEX_UNIT_SP, 20);
		this.setPadding(10, 10, 10, 10);
        this.setBackgroundColor(Theme.KEY_COLOR);
	}
	
	public void setService(IKnowUKeyboardService serv) {
		this.service = serv;
	}

    public void setListAdapter(VoiceScreen.VoiceInputAdapter adapter) {
        this.adapter = adapter;
    }
	
	private void performTouchAction() {
		//Log.d("Performing Touch action", "going to submit text = "+this.getText());
		InputConnection ic = this.service.getCurrentInputConnection();
		ic.commitText(this.getText()+" ", 1);
        this.adapter.clear();
		//this.service.switchToVoiceInput();
	}
	
	@Override
	public boolean onTouchEvent(MotionEvent event) {
		//Log.d("On Touch Event", "Initiated");
		if (event.getAction() == MotionEvent.ACTION_UP) {
			this.performTouchAction();
		}
		return true;
	}
}
