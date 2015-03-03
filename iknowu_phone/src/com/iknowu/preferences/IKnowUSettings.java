package com.iknowu.preferences;

import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.content.SharedPreferences.OnSharedPreferenceChangeListener;
import android.os.Bundle;
import android.preference.CheckBoxPreference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceFragment;
import android.preference.PreferenceScreen;
import android.util.Log;

import com.iknowu.IKnowUKeyboardService;
import com.iknowu.R;

import java.util.List;

public class IKnowUSettings extends PreferenceActivity implements OnSharedPreferenceChangeListener {

    public static final String ACTION_ASSISTANCE = "com.iknowu.settings.assistance";
    public static final String ACTION_FEEDBACK = "com.iknowu.settings.feedback";
    public static final String ACTION_CLOUD = "com.iknowu.settings.cloud";
    public static final String ACTION_PERSONAL = "com.iknowu.settings.personal";
    public static final String ACTION_LEGAL = "com.iknowu.settings.legal";
//    public static final String KEY_LIST_PREFERENCE = "listPref";
	private static final String PREF_AUTO_LEARN = "autolearn_on";
	private static final String PREF_IN_LINE_ADDITIONS = "inlineaddition_on";
	private static final String PREF_KEYBOARD_LAYOUT = "keyboard_layout";
    private static final String PREF_KEY = "english_ime_settings";

    private static SettingsFragmentOne settingsFragment;

    public static IKnowUSettings activity;

    @SuppressWarnings("deprecation")
	@Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        activity = this;
    }

    @Override
    public void onBuildHeaders(List<Header> target) {
        loadHeadersFromResource(R.xml.prefs_headers, target);
    }

    @SuppressWarnings("deprecation")
	@Override
    public void onResume() {
        super.onResume();

        // Set up a listener whenever a key changes
        /*this.settingsFragment.getPreferenceScreen().getSharedPreferences().registerOnSharedPreferenceChangeListener(this);

        Intent intent = this.getIntent();
        String action = intent.getAction();
        PreferenceScreen screen = (PreferenceScreen) findPreference(PREF_KEY);

        if (action != null) {
            if (action.equals(ACTION_ASSISTANCE)) {
                // simulate a click / call it!!
                screen.onItemClick( null, null, 0, 0 );
            } else if (action.equals(ACTION_FEEDBACK)) {
                // simulate a click / call it!!
                screen.onItemClick( null, null, 1, 0 );
            } else if (action.equals(ACTION_CLOUD)) {
                // simulate a click / call it!!
                screen.onItemClick( null, null, 2, 0 );
            } else if (action.equals(ACTION_PERSONAL)) {
                // simulate a click / call it!!
                screen.onItemClick( null, null, 3, 0 );
            } else if (action.equals(ACTION_LEGAL)) {
                // simulate a click / call it!!
                screen.onItemClick( null, null, 4, 0 );
            }
        }*/
    }

    @SuppressWarnings("deprecation")
	@Override
    public void onPause() {
        super.onPause();
        // Unregister the listener whenever a key changes
        //this.settingsFragment.getPreferenceScreen().getSharedPreferences().unregisterOnSharedPreferenceChangeListener(this);
    }

    @SuppressWarnings("deprecation")
	public void onSharedPreferenceChanged(SharedPreferences sharedPreferences, String key) {
    	
    	//if (key.equals(PREF_AUTO_LEARN)) {
        //	CheckBoxPreference autocbf = (CheckBoxPreference) getPreferenceScreen().findPreference(PREF_AUTO_LEARN);
        //	boolean isEnabled = autocbf.isChecked();
        //	if (isEnabled) {
        //		CheckBoxPreference cbf = (CheckBoxPreference) getPreferenceScreen().findPreference(PREF_IN_LINE_ADDITIONS);
        //		cbf.setChecked(false);
        //	}
        //	getPreferenceScreen().findPreference(PREF_IN_LINE_ADDITIONS).setEnabled(!isEnabled);

        IKnowUKeyboardService.log(Log.VERBOSE, "onSharedPreferenceChanged()", "key = "+key);

    	if (key.equals(PREF_KEYBOARD_LAYOUT) || key.equals(IKnowUKeyboardService.PREF_TABLET_LAYOUT) || key.equals(IKnowUKeyboardService.PREF_KEY_HEIGHT)) {
    		Editor edit = sharedPreferences.edit();
    		edit.putBoolean(IKnowUKeyboardService.PREF_KB_LAYOUT_CHANGED, true);
    		edit.commit();
    	} else if (key.equals(IKnowUKeyboardService.PREF_AUTOCORRECT_ON)) {
    		CheckBoxPreference autocbf = (CheckBoxPreference) settingsFragment.getPreferenceScreen().findPreference(IKnowUKeyboardService.PREF_AUTOCORRECT_ON);
        	boolean isEnabled = autocbf.isChecked();
    		if (isEnabled) {
        		CheckBoxPreference cbf = (CheckBoxPreference) settingsFragment.getPreferenceScreen().findPreference(IKnowUKeyboardService.PREF_AUTOCORRECT_INSERT);
                settingsFragment.getPreferenceScreen().findPreference(IKnowUKeyboardService.PREF_AUTOCORRECT_INSERT).setEnabled(true);
        		cbf.setChecked(true);
        	} else {
        		CheckBoxPreference cbf = (CheckBoxPreference) settingsFragment.getPreferenceScreen().findPreference(IKnowUKeyboardService.PREF_AUTOCORRECT_INSERT);
        		cbf.setChecked(false);
                settingsFragment.getPreferenceScreen().findPreference(IKnowUKeyboardService.PREF_AUTOCORRECT_INSERT).setEnabled(false);
        	}
    	}
    }

    public static class SettingsFragmentOne extends PreferenceFragment {
        @Override
        public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);

            settingsFragment = this;

            Bundle args = this.getArguments();
            int screen = args.getInt("headerKey");

            switch (screen) {
                case 0:
                    addPreferencesFromResource(R.xml.prefs_1);
                    break;
                case 1:
                    addPreferencesFromResource(R.xml.prefs_2);
                    break;
                case 2:
                    addPreferencesFromResource(R.xml.prefs_3);
                    break;
                case 3:
                    addPreferencesFromResource(R.xml.prefs_4);
                    break;
                case 4:
                    addPreferencesFromResource(R.xml.prefs_5);
                    break;
            }
            // Load the preferences from an XML resource
        }

        @Override
        public void onResume() {
            super.onResume();
            this.getPreferenceScreen().getSharedPreferences().registerOnSharedPreferenceChangeListener( activity );
        }

        @Override
        public void onPause() {
            super.onPause();
            this.getPreferenceScreen().getSharedPreferences().unregisterOnSharedPreferenceChangeListener( activity );
        }
    }
}
