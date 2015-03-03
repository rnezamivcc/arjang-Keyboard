package com.iknowu;

import android.content.Context;
import android.content.SharedPreferences;
import android.content.res.Configuration;
import android.preference.PreferenceManager;
import android.util.AttributeSet;
import android.util.Log;
import android.widget.LinearLayout;

import com.iknowu.miniapp.MiniAppActionBar;

/**
 * Created by Justin on 25/09/13.
 *
 */
public class KeyboardLinearLayout extends LinearLayout {

    public static final int MODE_TABLET_FULL = 0;
    public static final int MODE_TABLET_SPLIT = 2;
    public static final int MODE_TABLET_THUMB = 1;

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
    private Context context;
    //private int width;

    private KeyboardContainerView kbContainerView;

    private IKnowUKeyboardView inputView;
    private MiniAppActionBar actionBar;

    private boolean isLefty;

    public KeyboardLinearLayout(Context context) {
        super(context);

        this.context = context;
        this.currentLayout = QWERTY; //default
    }

    public KeyboardLinearLayout(Context context, AttributeSet attrs) {
        super(context, attrs);

        this.context = context;
        this.currentLayout = QWERTY; //default
    }

    public KeyboardLinearLayout(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);

        this.context = context;
        this.currentLayout = QWERTY; //default
    }

    public void init(IKnowUKeyboardService serv, KeyboardContainerView container, int themeId, int mode, boolean islefty) {
        this.kbContainerView = container;
        this.setCurrentMode(mode);

        this.isLefty = islefty;

        this.inputView = (IKnowUKeyboardView) this.findViewById(R.id.IKnowUKeyboardView);
        //IKnowUKeyboardService.log(Log.VERBOSE, "KBScreen", "inputView = "+inputView);

        this.inputView.setKeyboardService(serv);
        this.inputView.setKeyboardLinearLayout(this);
        this.inputView.setLeftyMode(islefty);
        this.inputView.processAttributes();

        /*this.candidateView = (SuggestionsLinearLayout) this.myView.findViewById(R.id.CandView);
		this.suggScrollView = (SuggestionScrollView) this.myView.findViewById(R.id.sug_scroll_view);
		this.suggScrollView.setScroller(ikuScroller);

        this.candidateView.setKeyboardService(serv);
        this.candidateView.init(themeId);

		this.inputView.setSuggestionsView(this.candidateView);*/

        this.actionBar = (MiniAppActionBar) this.findViewById(R.id.MiniActionBar);
        int actionHeight  = this.context.getResources().getDimensionPixelSize(R.dimen.small_action_bar_height);
        int childHeight = this.context.getResources().getDimensionPixelSize(R.dimen.small_action_bar_child_height);
        this.actionBar.init(0, actionHeight, childHeight, false);
        this.actionBar.setKeyboardLinearLayout(this);
        this.actionBar.setListenForSpacePush();
        //this.actionBar.setIKnowUScroller(this.ikuScroller);
        //IKnowUKeyboardService.log(Log.VERBOSE, "KBScreen", "miniActionBar = "+this.actionBar);

        this.inputView.clearNextKeyHighlighting();
        this.inputView.establishScreenLocations();
    }

    public void onFinishInput() {
        this.inputView.clearNextKeyHighlighting();
    }

    public void onConfigurationChanged(Configuration newConfig) {
        this.inputView.resetScreenLocations();
        this.inputView.mPopupKeyboardActive = false;
        this.inputView.hidePreview();
        this.inputView.stopDeleteEntireWordFromEditor(); // runs on the main keyboards

        this.inputView.getSuggestionsView().resetScreenLocations();
    }

    public void doKeyHighlighting(boolean clearHighlight) {
        if (clearHighlight) this.inputView.clearNextKeyHighlighting();
        else this.inputView.prepareNextKeyHighlighting();
        this.inputView.invalidate();
    }

    public MiniAppActionBar getActionBar() {
        return this.actionBar;
    }

    public void setCurrentMode(int mode) {
        this.currentMode = mode;

        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(this.context);
        SharedPreferences.Editor edit = sp.edit();
        edit.putString(IKnowUKeyboardService.PREF_TABLET_LAYOUT, this.currentMode+"");
        edit.commit();
    }

    public int getCurrentMode() {
        return this.currentMode;
    }

    public IKnowUKeyboardView getKeyboardView() {
        return this.inputView;
    }

    public SuggestionsLinearLayout getSuggestionView() {
        return this.inputView.getSuggestionsView();
    }

    public IKnowUKeyboard getKeyboard() {
        return /*(IKnowUKeyboard)*/ this.inputView.getKeyboard();
    }

    public void setKeyboard() {
        this.setKeyboard(this.currentLayout, false);
    }

    public void resizeComponents(int width) {
        LinearLayout.LayoutParams params = new LayoutParams(width, LayoutParams.WRAP_CONTENT);
        this.getKeyboard().resizeKeys(width);
        this.inputView.resizeComponents(width);
        this.inputView.getSuggestionsView().resizeComponents(width);
        //this.suggScrollView.resizeComponents(width);
        this.setLayoutParams(params);
    }

    public void setKeyboard(int kblayout, boolean changeMode) {
        //IKnowUKeyboardService.log(Log.VERBOSE, "KBScreen", "Set Keyboard Layout to = "+kblayout);
        if (changeMode) {
            switch (this.currentMode) {
                case MODE_TABLET_FULL:
                    this.setCurrentMode(MODE_TABLET_THUMB);
                    break;
                case MODE_TABLET_THUMB:
                    this.setCurrentMode(MODE_TABLET_FULL);
                    break;
            }
        }

        this.currentLayout = kblayout;
        IKnowUKeyboard kb = null;
        switch (this.currentMode) {
            case MODE_TABLET_FULL:
                kb = this.setTabletFullKeyboard(kblayout, this.inputView.getSuggestionsView().getHeight());
                break;
            case MODE_TABLET_THUMB:
                kb = this.setTabletThumbKeyboard(kblayout, this.inputView.getSuggestionsView().getHeight());
                break;
            default:
                kb = this.setTabletFullKeyboard(kblayout, this.inputView.getSuggestionsView().getHeight());
                break;
        }

        kb.resizeKeys(this.getMeasuredWidth());
        this.inputView.setKeyboard(kb);

        this.inputView.invalidate();
        //this.suggestionView.invalidate();
        this.actionBar.invalidate();
    }

    private IKnowUKeyboard setTabletFullKeyboard(int kblayout, int yoffset) {
        IKnowUKeyboard kb = null;
        switch (kblayout) {
            case QWERTY:
                kb = new IKnowUKeyboard(this.context, R.xml.tablet_full_english, yoffset);
                break;
            case QWERTY_SPANISH:
                kb = new IKnowUKeyboard(this.context, R.xml.tablet_full_qwerty_spanish, yoffset);
                break;
            case AZERTY:
                kb = new IKnowUKeyboard(this.context, R.xml.tablet_full_azerty, yoffset);
                break;
            case QWERTZ:
                kb = new IKnowUKeyboard(this.context, R.xml.tablet_full_qwertz, yoffset);
                break;
            case QZERTY:
                kb = new IKnowUKeyboard(this.context, R.xml.tablet_full_qzerty, yoffset);
                break;
            case RUSSIAN:
                kb = new IKnowUKeyboard(this.context, R.xml.tablet_full_rus, yoffset);
                break;
            case KOREAN:
                kb = new IKnowUKeyboard(this.context, R.xml.tablet_full_korean, yoffset);
                break;
            case NUMERIC:
                kb = new IKnowUKeyboard(this.context, R.xml.tablet_full_numeric, yoffset);
                break;
            case SYMBOLS:
                kb = new IKnowUKeyboard(this.context, R.xml.tablet_full_symbols, yoffset);
                break;
            case SYMBOLS_2:
                kb = new IKnowUKeyboard(this.context, R.xml.tablet_full_symbols2, yoffset);
                break;
            case EXTRA_LETTERS:
                kb = new IKnowUKeyboard(this.context, R.xml.tablet_full_extra_letters, yoffset);
                break;
            case SMILEY:
                kb = new IKnowUKeyboard(this.context, R.xml.tablet_full_smiley, yoffset);
                break;
            case NAVIGATION:
                kb = new IKnowUKeyboard(this.context, R.xml.tablet_full_navigation, yoffset);
                break;
            case COMPRESSED:
                kb = new IKnowUKeyboard(this.context, R.xml.compressed_kb_tablet, yoffset);
                break;
            case FEATURE:
                kb = new IKnowUKeyboard(this.context, R.xml.feature_phone, yoffset);
                break;
            case KEYBOARD_PICKER:
                kb = new IKnowUKeyboard(this.context, R.xml.keyboard_picker, yoffset);
                IKnowUKeyboardService.log(Log.VERBOSE, "KeyboardLinearLayout.setKeyboard()", "setting keyboard to keyboard_picker");
                break;
            case POPUP_MENU:
                kb = new IKnowUKeyboard(this.context, R.xml.popup_menu, yoffset);
                break;
            default:
                kb = new IKnowUKeyboard(this.context, R.xml.tablet_full_english, yoffset);
                break;
        }

        return kb;
    }

    private IKnowUKeyboard setTabletThumbKeyboard(int kblayout, int yoffset) {
        IKnowUKeyboard kb = null;
        switch (kblayout) {
            case QWERTY:
                kb = new IKnowUKeyboard(this.context, R.xml.tablet_thumb_english, yoffset);
                break;
            case QWERTY_SPANISH:
                kb = new IKnowUKeyboard(this.context, R.xml.tablet_thumb_qwerty_spanish, yoffset);
                break;
            case AZERTY:
                kb = new IKnowUKeyboard(this.context, R.xml.tablet_thumb_azerty, yoffset);
                break;
            case QWERTZ:
                kb = new IKnowUKeyboard(this.context, R.xml.tablet_thumb_qwertz, yoffset);
                break;
            case QZERTY:
                kb = new IKnowUKeyboard(this.context, R.xml.tablet_thumb_qzerty, yoffset);
                break;
            case RUSSIAN:
                kb = new IKnowUKeyboard(this.context, R.xml.tablet_full_rus, yoffset);
                break;
            case KOREAN:
                kb = new IKnowUKeyboard(this.context, R.xml.tablet_full_korean, yoffset);
                break;
            case NUMERIC:
                kb = new IKnowUKeyboard(this.context, R.xml.tablet_full_numeric, yoffset);
                break;
            case SYMBOLS:
                kb = new IKnowUKeyboard(this.context, R.xml.tablet_full_symbols, yoffset);
                break;
            case SYMBOLS_2:
                kb = new IKnowUKeyboard(this.context, R.xml.tablet_full_symbols2, yoffset);
                break;
            case EXTRA_LETTERS:
                kb = new IKnowUKeyboard(this.context, R.xml.tablet_full_extra_letters, yoffset);
                break;
            case SMILEY:
                kb = new IKnowUKeyboard(this.context, R.xml.tablet_full_smiley, yoffset);
                break;
            case NAVIGATION:
                kb = new IKnowUKeyboard(this.context, R.xml.tablet_full_navigation, yoffset);
                break;
            case COMPRESSED:
                kb = new IKnowUKeyboard(this.context, R.xml.compressed_kb_tablet, yoffset);
                break;
            case FEATURE:
                kb = new IKnowUKeyboard(this.context, R.xml.feature_phone, yoffset);
                break;
            case KEYBOARD_PICKER:
                kb = new IKnowUKeyboard(this.context, R.xml.keyboard_picker, yoffset);
                IKnowUKeyboardService.log(Log.VERBOSE, "KeyboardLinearLayout.setKeyboard()", "setting keyboard to keyboard_picker");
                break;
            case POPUP_MENU:
                kb = new IKnowUKeyboard(this.context, R.xml.popup_menu, yoffset);
                break;
            default:
                kb = new IKnowUKeyboard(this.context, R.xml.tablet_thumb_english, yoffset);
                break;
        }

        return kb;
    }
}
