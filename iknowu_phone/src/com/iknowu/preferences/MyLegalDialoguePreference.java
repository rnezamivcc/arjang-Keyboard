package com.iknowu.preferences;

import android.content.Context;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.preference.DialogPreference;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;

import com.iknowu.IKnowUKeyboardService;

/**
 * this class needs to be here, even though it is empty! In order to properly inflate a dialog
 * preference from the preferences screen
 */
public class MyLegalDialoguePreference extends DialogPreference {
    // TODO - This for testing/development purposes only - set false in final release
    private static final boolean ENABLE_SWIPE_TOGGLE = true;

	public MyLegalDialoguePreference(Context context, AttributeSet attrs) {
		super(context, attrs);
	}

    @Override
    protected View onCreateView(ViewGroup parent) {
        View view = super.onCreateView(parent);
        if (ENABLE_SWIPE_TOGGLE) {
            SharedPreferences sp = this.getSharedPreferences();
            boolean swipeEnabled = sp.getBoolean(IKnowUKeyboardService.PREF_SWIPE_ENABLED, false);
            Log.i("MyLogs", "MyLegalDialoguePreference : swipeEnabled = " + swipeEnabled);
            String s = this.getDialogMessage().toString();
            this.setDialogMessage(s + (swipeEnabled ? "[on]" : "[off]"));
        }
        return view;
    }

    @Override
    protected void onDialogClosed(boolean positiveResult) {
        if (ENABLE_SWIPE_TOGGLE) {
            SharedPreferences sp = this.getSharedPreferences();
            boolean swipeEnabled = sp.getBoolean(IKnowUKeyboardService.PREF_SWIPE_ENABLED, false);
            Editor edit = sp.edit();
            edit.putBoolean(IKnowUKeyboardService.PREF_SWIPE_ENABLED, !swipeEnabled);
            edit.commit();
        }
        super.onDialogClosed(positiveResult);
    }
}
