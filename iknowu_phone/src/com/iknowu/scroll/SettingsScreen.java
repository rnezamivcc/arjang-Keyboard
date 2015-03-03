package com.iknowu.scroll;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.Uri;
import android.preference.PreferenceManager;
import android.support.v4.view.ViewPager.LayoutParams;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.LinearLayout;

import com.iknowu.IKnowUKeyboardService;
import com.iknowu.R;
import com.iknowu.dictionarymanager.Analyzer;
import com.iknowu.downloader.DownloadActivity;
import com.iknowu.preferences.IKnowUSettings;
import com.iknowu.setup.TutorialActivity;

public class SettingsScreen implements Screen {
	
	private boolean setupComplete;
	//private ScrollView myView;
	private LinearLayout myView;
	private Context context;
	private int width;
	
	private IKnowUScroller ikScroller;
	
	public SettingsScreen(Context ctx, int width) {
		this.context = ctx;
		this.width = width;
	}
	
	public void init(IKnowUScroller scroll) {
		
		this.ikScroller = scroll;
		
		SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(this.context);
		this.setupComplete = sp.getBoolean(IKnowUKeyboardService.PREF_SETUP_COMPLETE, false);
		
		LayoutInflater layinf = (LayoutInflater) this.context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
		ViewGroup.LayoutParams params = new ViewGroup.LayoutParams(this.width, LayoutParams.WRAP_CONTENT);
		
		this.myView = (LinearLayout) layinf.inflate(R.layout.settings_menu_layout, null);
		this.myView.setLayoutParams(params);
		
		LinearLayout helpLin = (LinearLayout) this.myView.findViewById(R.id.settings_help);
		helpLin.setBackgroundResource(R.drawable.menu_state_button);
        helpLin.setOnClickListener( new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                launchHelp();
            }
        });
	
		LinearLayout settingsLin = (LinearLayout) this.myView.findViewById(R.id.settings_settings);
        settingsLin.setBackgroundResource(R.drawable.menu_state_button);
		settingsLin.setOnClickListener( new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                launchSettings();
            }
        });
		
		LinearLayout dictDownloadLin = (LinearLayout) this.myView.findViewById(R.id.settings_dict_download);
        dictDownloadLin.setBackgroundResource(R.drawable.menu_state_button);
		dictDownloadLin.setOnClickListener( new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                launchDownloader();
            }
        });
	
		LinearLayout dictManageLin = (LinearLayout) this.myView.findViewById(R.id.settings_dict_manage);
        dictManageLin.setBackgroundResource(R.drawable.menu_state_button);
		dictManageLin.setOnClickListener( new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                launchDictionaryManager();
            }
        });
		
		LinearLayout dictLin = (LinearLayout) this.myView.findViewById(R.id.settings_hide);
		dictLin.setBackgroundResource(R.drawable.menu_state_button);
        dictLin.setOnClickListener( new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                ikScroller.hideKeyboard();
            }
        });
		
		LinearLayout webLin = (LinearLayout) this.myView.findViewById(R.id.settings_website);
		webLin.setBackgroundResource(R.drawable.menu_state_button);
        webLin.setOnClickListener( new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                launchWebsite();
            }
        });
       
    }
	
	@Override
	public View getView() {
		return this.myView;
	}
	
	private void launchSetup() {
		//handleClose();
		Intent intent = new Intent();
		intent.setClass(this.context, TutorialActivity.class);
		intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
		this.context.startActivity(intent);
	}

	/*
	 * Launch a new intent that will bring us to the website
	 */
	private void launchWebsite() {
		//handleClose();
		String url = "http://www.iknowu.net";
		Intent intent = new Intent(Intent.ACTION_VIEW);
		intent.setData(Uri.parse(url));
		intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
		this.context.startActivity(intent);
	}
	
	/*
	 * launch the settings activity
	 */
	private void launchSettings() {
		//if (this.setupComplete) {
			//handleClose();
			Intent intent = new Intent();
			intent.setClass(this.context, IKnowUSettings.class);
			intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
			this.context.startActivity(intent);
		//} else {
		//	launchSetup();
		//}
	}
	
	/*
	 * launch the dictionary manager activity
	 */
	private void launchDictionaryManager() {
		//handleClose();
		Intent intent = new Intent();
		intent.setClass(this.context, Analyzer.class);
		intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
		this.context.startActivity(intent);
	}
	
	public void launchAnalyzer() {
		//handleClose();
		Intent intent = new Intent();
		intent.setClass(this.context, Analyzer.class);
		intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
		this.context.startActivity(intent);
	}
	
	public void launchDownloader() {
		//handleClose();
		Intent intent = new Intent();
		intent.setClass(this.context, DownloadActivity.class);
		intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
		this.context.startActivity(intent);
	}
	
	/*
	 * show the help menu screen
	 */
	private void launchHelp() {

        Intent intent = new Intent();
        intent.setClass(this.context, TutorialActivity.class);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        this.context.startActivity(intent);
	}
}
