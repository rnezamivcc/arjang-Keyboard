package com.iknowu.preferences;

import android.content.Context;
import android.content.DialogInterface;
import android.preference.DialogPreference;
import android.util.AttributeSet;
import android.util.Log;
import android.view.Gravity;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.SeekBar;
import android.widget.TextView;

import com.iknowu.IKnowUKeyboardService;

public class SeekBarPreference extends DialogPreference implements
		SeekBar.OnSeekBarChangeListener {

    private static final String TAG = "SeekBarPreference";

	private static final String androidns = "http://schemas.android.com/apk/res/android";
	private static final String iknowuns = "http://schemas.android.com/apk/res/com.iknowu";
	private int minVal = 0;

	private SeekBar mSeekBar;
	private TextView mSplashText, mValueText;
	private Context mContext;

	private String mDialogMessage, mSuffix;
	private int mDefault, mMax, mValue = 0;
	
	private int valueSet;

	public SeekBarPreference(Context context, AttributeSet attrs) {
		super(context, attrs);
		mContext = context;
		
		int dialogres = attrs.getAttributeResourceValue(androidns, "dialogMessage", -1);
		if (dialogres >= 0) {
			this.mDialogMessage = context.getResources().getString(dialogres);
		} else {
			this.mDialogMessage = attrs.getAttributeValue(androidns, "dialogMessage");
		}

		mSuffix = attrs.getAttributeValue(androidns, "text");
		mDefault = attrs.getAttributeIntValue(androidns, "defaultValue", 0);
		mMax = attrs.getAttributeIntValue(androidns, "max", 100);
		minVal = attrs.getAttributeIntValue(iknowuns, "min", 0);

        IKnowUKeyboardService.log(Log.VERBOSE, TAG, "constructor() --> mMax = "+mMax+", minVal = "+minVal+", default = "+mDefault);
	}

	@Override
	protected View onCreateDialogView() {
		
		LinearLayout.LayoutParams params;
		LinearLayout layout = new LinearLayout(mContext);
		layout.setOrientation(LinearLayout.VERTICAL);
		layout.setPadding(6, 6, 6, 6);

		mSplashText = new TextView(mContext);
		if (mDialogMessage != null) {
            mSplashText.setText(mDialogMessage);
        }
		layout.addView(mSplashText);

		mValueText = new TextView(mContext);
		mValueText.setGravity(Gravity.CENTER_HORIZONTAL);
		mValueText.setTextSize(32);
		params = new LinearLayout.LayoutParams(
				LinearLayout.LayoutParams.MATCH_PARENT,
				LinearLayout.LayoutParams.WRAP_CONTENT);
		layout.addView(mValueText, params);

		mSeekBar = new SeekBar(mContext);
		mSeekBar.setOnSeekBarChangeListener(this);
		layout.addView(mSeekBar, new LinearLayout.LayoutParams(
				LinearLayout.LayoutParams.MATCH_PARENT,
				LinearLayout.LayoutParams.WRAP_CONTENT));

		if (shouldPersist()) {
            mValue = getPersistedInt(mDefault);
        }

        if (mValue < minVal) {
            mValue = mDefault;
        }
        else if (mValue > (minVal + mMax)) {
            mValue = mDefault;
        }

		mSeekBar.setMax(mMax - minVal);
		//mSeekBar.setProgress(mValue);
		return layout;
	}
	
	@Override
	public void onClick(DialogInterface dialog, int which) {
		IKnowUKeyboardService.log(Log.VERBOSE, "Seek bar on click", "which = "+which);
		
		if (which == DialogInterface.BUTTON_POSITIVE) {
			if (shouldPersist()) {
                persistInt(this.valueSet + minVal);
            }
		}
	}

	@Override
	protected void onBindDialogView(View v) {
		super.onBindDialogView(v);
		//Log.d("SEEK BAR", "BIND DIALOGUE VIEW");
		mSeekBar.setMax(mMax);
		mSeekBar.setProgress(mValue - minVal);
	}

	public void onProgressChanged(SeekBar seek, int value, boolean fromTouch) {
		//Log.d("SEEK BAR", "ON PROGRESS CHANGED");
		//if (fromTouch) {
		String t = String.valueOf(value + minVal);
		//Log.d("Value = ", "" + value);
		mValueText.setText(mSuffix == null ? t : t.concat(mSuffix));
		
		this.valueSet = value;
		
		callChangeListener(value + minVal);
	}

	public void onStartTrackingTouch(SeekBar seek) {
	}

	public void onStopTrackingTouch(SeekBar seek) {
	}

	public void setMax(int max) {
		mMax = max;
	}

	public int getMax() {
		return mMax;
	}

	public void setProgress(int progress) {
		mValue = progress;
		if (mSeekBar != null)
			mSeekBar.setProgress(progress);
	}

	public int getProgress() {
		return mValue;
	}
}
