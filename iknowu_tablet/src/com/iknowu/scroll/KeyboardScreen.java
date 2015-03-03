package com.iknowu.scroll;


import android.content.Context;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.LinearLayout;

import com.iknowu.IKnowUKeyboard;
import com.iknowu.IKnowUKeyboardService;
import com.iknowu.IKnowUKeyboardView;
import com.iknowu.R;
import com.iknowu.SuggestionScrollView;
import com.iknowu.SuggestionsLinearLayout;
import com.iknowu.miniapp.MiniAppActionBar;

public class KeyboardScreen implements Screen {
	
	public static final int MODE_PHONE = 0;
	public static final int MODE_TABLET_FULL = 1;
	public static final int MODE_TABLET_SPLIT = 2;
	public static final int MODE_TABLET_THUMB = 3;
	
	public static final int QWERTY = 0;
	public static final int QWERTY_SPANISH = 1;
	public static final int AZERTY = 2;
	public static final int QWERTZ = 3;
	public static final int QZERTY = 4;
	public static final int RUSSIAN = 5;
	public static final int KOREAN = 6;
	
	public static final int COMPRESSED = 30;
    public static final int FEATURE = 31;
    public static final int KEYBOARD_PICKER = 33;
    public static final int POPUP_MENU = 34;
	
	public static final int NUMERIC = 50;
	public static final int SYMBOLS = 51;
	public static final int SYMBOLS_2 = 52;
	public static final int EXTRA_LETTERS = 53;
	public static final int SMILEY = 54;
	public static final int NAVIGATION = 55;
	
	private int currentMode;
	private int currentLayout;
	private LinearLayout myView;
	private Context context;
	private int width;
	private IKnowUKeyboardView inputView;
	private SuggestionsLinearLayout candidateView;
	private IKnowUScroller ikuScroller;
	private SuggestionScrollView suggScrollView;
	private MiniAppActionBar actionBar;
	
	public KeyboardScreen(Context ctx, IKnowUScroller scroller, int width) {
		this.context = ctx;
		this.width = width;
		this.ikuScroller = scroller;
		this.currentLayout = QWERTY; //default
	}
	
	public void init(IKnowUKeyboardService serv, int themeId, int layout) {
		LayoutInflater layinf = (LayoutInflater) this.context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
		
		this.myView = (LinearLayout) layinf.inflate(layout, null);
		this.inputView = (IKnowUKeyboardView) this.myView.findViewById(R.id.IKnowUKeyboardView);
		//IKnowUKeyboardService.log(Log.VERBOSE, "KBScreen", "inputView = "+inputView);

        this.inputView.setKeyboardService(serv);
        this.inputView.processAttributes();

		/*this.candidateView = (SuggestionsLinearLayout) this.myView.findViewById(R.id.CandView);
		this.suggScrollView = (SuggestionScrollView) this.myView.findViewById(R.id.sug_scroll_view);
		this.suggScrollView.setScroller(ikuScroller);

        this.candidateView.setKeyboardService(serv);
        this.candidateView.init(themeId);

		this.inputView.setSuggestionsView(this.candidateView);*/
		
		this.actionBar = (MiniAppActionBar) this.myView.findViewById(R.id.MiniActionBar);
		int actionHeight  = this.context.getResources().getDimensionPixelSize(R.dimen.small_action_bar_height);
		int childHeight = this.context.getResources().getDimensionPixelSize(R.dimen.small_action_bar_child_height);
		this.actionBar.init(IKnowUKeyboardService.DEFAULT_KEYBOARD_THEME_ID, actionHeight, childHeight, false);
		this.actionBar.setIKnowUScroller(this.ikuScroller);

        if (!IKnowUKeyboardService.MINIAPP_ON) this.actionBar.setVisibility(View.GONE);
		IKnowUKeyboardService.log(Log.VERBOSE, "KBScreen", "miniActionBar = "+this.actionBar);

        this.inputView.clearNextKeyHighlighting();
        this.inputView.establishScreenLocations();
	}

    public void doConfigurationChanged() {
        if (inputView != null) {
            inputView.resetScreenLocations();
            inputView.mPopupKeyboardActive = false;
            inputView.hidePreview();
            inputView.stopDeleteEntireWordFromEditor(); // runs on the main keyboards
        }
        if (candidateView != null) {
            candidateView.resetScreenLocations();
        }
    }
	
	public MiniAppActionBar getActionBar() {
		return this.actionBar;
	}
	
	@Override
	public View getView() {
		return this.myView;
	}
	
	public void setCurrentMode(int mode) {
		this.currentMode = mode;
	}
	
	public IKnowUKeyboardView getKeyboardView() {
		return this.inputView;
	}
	
	public SuggestionsLinearLayout getCandidateView() {
		return this.inputView.getSuggestionsView();
	}
	
	public IKnowUKeyboard getKeyboard() {
		return /*(IKnowUKeyboard)*/ this.inputView.getKeyboard();
	}

    public void onFinishInput() {
        this.inputView.clearNextKeyHighlighting();
    }

    public void doKeyHighlighting(boolean clearHighlight) {
        if (clearHighlight) this.inputView.clearNextKeyHighlighting();
        else this.inputView.prepareNextKeyHighlighting();
        this.inputView.invalidate();
    }

	public void setKeyboard() {
		this.setKeyboard(this.currentLayout);
	}
	
	public void setKeyboard(int kblayout) {
		IKnowUKeyboardService.log(Log.VERBOSE, "KBScreen", "Set Keyboard Layout to = "+kblayout+", yoffset = "+this.inputView.getSuggestionsView().getHeight());
		this.currentLayout = kblayout;
		IKnowUKeyboard kb = null;
		switch (this.currentMode) {
			case MODE_PHONE:
				kb = this.setPhoneKeyboard(kblayout, this.inputView.getSuggestionsView().getHeight());
			break;
			default:
				kb = this.setPhoneKeyboard(kblayout, this.inputView.getSuggestionsView().getHeight());
			break;
		}

		this.inputView.setKeyboard(kb);

        this.inputView.invalidate();
        //this.candidateView.invalidate();
        this.actionBar.invalidate();
	}
	
	private IKnowUKeyboard setPhoneKeyboard(int kblayout, int yoffset) {
		IKnowUKeyboard kb = null;
	/*	switch (kblayout) {
			case QWERTY:
				kb = new IKnowUKeyboard(this.context, R.xml.kbenglish, yoffset);
			break;
			case QWERTY_SPANISH:
				kb = new IKnowUKeyboard(this.context, R.xml.phone_qwerty_spanish, yoffset);
			break;
			case AZERTY:
				kb = new IKnowUKeyboard(this.context, R.xml.phone_azerty, yoffset);
			break;
			case QWERTZ:
				kb = new IKnowUKeyboard(this.context, R.xml.phone_qwertz, yoffset);
			break;
			case QZERTY:
				kb = new IKnowUKeyboard(this.context, R.xml.phone_qzerty, yoffset);
			break;
			case RUSSIAN:
				kb = new IKnowUKeyboard(this.context, R.xml.kb_rus, yoffset);
			break;
			case KOREAN:
				kb = new IKnowUKeyboard(this.context, R.xml.phone_korean, yoffset);
			break;
			case NUMERIC:
				kb = new IKnowUKeyboard(this.context, R.xml.kb123, yoffset);
			break;
			case SYMBOLS:
				kb = new IKnowUKeyboard(this.context, R.xml.kbsymbols, yoffset);
			break;
			case SYMBOLS_2:
				kb = new IKnowUKeyboard(this.context, R.xml.kbsymbols_shift, yoffset);
			break;
			case EXTRA_LETTERS:
				kb = new IKnowUKeyboard(this.context, R.xml.kbextraletters, yoffset);
			break;
			case SMILEY:
				kb = new IKnowUKeyboard(this.context, R.xml.kbsmileys, yoffset);
			break;
			case NAVIGATION:
				kb = new IKnowUKeyboard(this.context, R.xml.kbnavigation, yoffset);
			break;
			case COMPRESSED:
				kb = new IKnowUKeyboard(this.context, R.xml.compressed_kb, yoffset);
			break;
            case FEATURE:
                kb = new IKnowUKeyboard(this.context, R.xml.feature_phone, yoffset);
                break;
            case KEYBOARD_PICKER:
                kb = new IKnowUKeyboard(this.context, R.xml.keyboard_picker, yoffset);
                break;
            case POPUP_MENU:
                kb = new IKnowUKeyboard(this.context, R.xml.popup_menu, yoffset);
                break;
			default:
				kb = new IKnowUKeyboard(this.context, R.xml.kbenglish, yoffset);
			break;
		}
		*/
		return kb;
	}
}
