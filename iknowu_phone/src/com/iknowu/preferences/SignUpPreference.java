package com.iknowu.preferences;

import android.content.Context;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.preference.DialogPreference;
import android.text.InputType;
import android.util.AttributeSet;
import android.view.View;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.TextView;

public class SignUpPreference extends DialogPreference {
	
	private static final String androidns="http://schemas.android.com/apk/res/android";
	
	private static final String PREF_KEY_FIRST = "firstSignUpValue";
	private static final String PREF_KEY_SECOND = "secondSignUpValue";
	
	private TextView firstInputLabelView, secondInputLabelView;
	private Context mContext;
	
	private EditText firstInput, secondInput;

	private String firstInputLabel, secondInputLabel;
	private String mDefault = "";
	
	private String firstVal, secondVal;
	
	public SignUpPreference(Context context, AttributeSet attrs) {
		super(context, attrs);
		mContext = context;
		
	    this.firstInputLabel = attrs.getAttributeValue(null,"firstInputLabel");
	    this.secondInputLabel = attrs.getAttributeValue(null,"secondInputLabel");
	    mDefault = attrs.getAttributeValue(androidns,"defaultValue");
	    
	    //this.setOnPreferenceChangeListener();
	}
	
	@Override
	protected View onCreateDialogView() {
		LinearLayout layout = new LinearLayout(mContext);
		layout.setOrientation(LinearLayout.VERTICAL);
		layout.setPadding(6, 6, 6, 6);

		firstInputLabelView = new TextView(mContext);
		if (this.firstInputLabel != null)
			firstInputLabelView.setText(this.firstInputLabel);
		layout.addView(this.firstInputLabelView);
		
		this.firstInput = new EditText(mContext);
		this.firstInput.setInputType(InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_VARIATION_EMAIL_ADDRESS);
		layout.addView(this.firstInput);
		
		secondInputLabelView = new TextView(mContext);
		if (this.secondInputLabel != null)
			secondInputLabelView.setText(this.secondInputLabel);
		layout.addView(this.secondInputLabelView);
		
		this.secondInput = new EditText(mContext);
		this.secondInput.setInputType(InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_VARIATION_PASSWORD);
		layout.addView(this.secondInput);
		
		if (shouldPersist()) {
			firstVal = getPersistedString(mDefault);
			secondVal = getPersistedString(mDefault);
		}
		
		return layout;
	}

	@Override
	protected void onBindDialogView(View v) {
		super.onBindDialogView(v);
		
		SharedPreferences sp = this.getSharedPreferences();
		
		firstVal = sp.getString(PREF_KEY_FIRST, mDefault);
		secondVal = sp.getString(PREF_KEY_SECOND, mDefault);
		
		this.firstInput.setText(this.firstVal);
		this.secondInput.setText(this.secondVal);
	}
	
	@Override
	protected void onDialogClosed(boolean positiveResult) {
	    super.onDialogClosed(positiveResult);
	    
	    if (positiveResult) {
	        Editor editor = getEditor();
	        editor.putString(PREF_KEY_FIRST, this.firstInput.getText().toString());
	        editor.putString(PREF_KEY_SECOND, this.secondInput.getText().toString());
	        editor.commit();
	    }
	}
}
