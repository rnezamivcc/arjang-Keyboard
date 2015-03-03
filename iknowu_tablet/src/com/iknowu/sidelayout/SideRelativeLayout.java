package com.iknowu.sidelayout;

import android.content.Context;
import android.content.SharedPreferences;
import android.content.res.Configuration;
import android.graphics.Paint;
import android.preference.PreferenceManager;
import android.util.AttributeSet;
import android.util.Log;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.widget.ImageView;
import android.widget.RelativeLayout;

import com.iknowu.IKnowUKeyboardService;
import com.iknowu.KeyboardContainerView;
import com.iknowu.KeyboardLinearLayout;
import com.iknowu.R;
import com.iknowu.miniapp.MiniAppScreen;
import com.iknowu.util.Size;
import com.iknowu.util.Theme;
import com.iknowu.voice.VoiceScreen;

import java.util.ArrayList;

/**
 * Created by Justin on 25/09/13.
 *
 */
public class SideRelativeLayout extends RelativeLayout {

    private static final String TAG_THEME_ITEM = "item";

    public static final String OPEN_SIDE_SCREEN = "openSideScreen";
    public static final String SIDE_KEYBOARD_SCREEN = "sideKeyboardScreen";
    public static final String SIDE_KEYBOARD_SCREEN_TYPE = "sideKeyboardScreenType";
    public static final String SETTINGS_SCREEN = "settingsScreen";
    public static final String MINI_APP_SCREEN = "miniAppScreen";
    public static final String VOICE_SCREEN = "voiceScreen";
    public static final String ABBREVIATION_SCREEN = "abbreviationScreen";

    public static int TAB_INDEX_MENU_SCREEN;
    public static int TAB_INDEX_MINIAPP_SCREEN;
    public static int TAB_INDEX_SIDE_KEYBOARD;
    public static int TAB_INDEX_VOICE_SCREEN;
    public static int TAB_INDEX_ABBREVIATION_SCREEN;

    public static final int ID_TAB_HOLDER = 111111;
    public static final int ID_MENU_TAB = 222222;
    public static final int ID_REACH_TAB = 333333;
    public static final int ID_KB_TAB = 444444;
    public static final int ID_CONTENT_VIEW = 555555;

    public static final int TAB_HOLDER_WIDTH = 50;
    public static final int TAB_MARGIN_INCREMENT = 53;
    public static final int FADING_EDGE_WIDTH = 6;

    public static final int MARGIN_BETWEEN = 10;

    public static final int NEGATIVE_OVERLAP = -15;

    private KeyboardContainerView kbContainerView;
    private IKnowUKeyboardService keyboardService;

    private Context context;
    private ArrayList<SideScreen> screens;

    private SideKeyboardScreen sideKeyboardScreen;
    private SettingsScreen settingsScreen;
    private MiniAppScreen miniAppScreen;
    private VoiceScreen voiceScreen;
    private AbbreviationScreen abbreviationScreen;

    private ImageView fadingEdge;

    private boolean isOpen;
    private boolean isLefty;

    public SideScreen lastSelected;

    private Paint paint;

    //private ArrayList<SidePopup> popups;

    public SideRelativeLayout(Context context) {
        super(context);
        this.context = context;
        this.paint = new Paint();
    }

    public SideRelativeLayout(Context context, AttributeSet attrs) {
        super(context, attrs);
        this.context = context;
        this.paint = new Paint();
    }

    public SideRelativeLayout(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        this.context = context;
        this.paint = new Paint();
    }

    public void init(IKnowUKeyboardService serv, KeyboardContainerView container, boolean islefty) {

        //this.popups = new ArrayList<SidePopup>();

        this.keyboardService = serv;
        this.kbContainerView = container;

        this.isLefty = islefty;
        this.isOpen = false;

        this.setBackgroundColor(Theme.BACKGROUND_COLOR);

        this.createScreens();

        /*if (this.isLefty) {
            this.initLeftHanded();
        } else {
            this.initRightHanded();
        }*/

        //this.contentView.setBackgroundColor(Theme.SIDE_BACKGROUND_COLOR);
    }

    /*@Override
    public void onDraw(Canvas canvas) {
        super.onDraw(canvas);

        this.paint.setColor(Color.BLUE);
        this.paint.setStyle(Paint.Style.STROKE);
        this.paint.setStrokeWidth(2);

        canvas.drawRect(0, 0, this.getWidth(), this.getHeight(), paint);
    }*/

    /*private void initRightHanded() {
        this.tabHolder = new RelativeLayout(this.context);
        this.tabHolder.setId(ID_TAB_HOLDER);
        LayoutParams params = new LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.MATCH_PARENT);
        params.addRule(RelativeLayout.ALIGN_PARENT_LEFT);
        this.tabHolder.setLayoutParams(params);
        this.tabHolder.setBackgroundColor(Theme.BACKGROUND_COLOR);

        params = new LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT);
        params.addRule(RIGHT_OF, ID_TAB_HOLDER);
        params.addRule(ALIGN_PARENT_BOTTOM);
        this.contentView.setLayoutParams(params);

        //menu tab
        this.menuTab = new ImageView(this.context);
        this.menuTab.setId(ID_MENU_TAB);

        RelativeLayout.LayoutParams relparams = new RelativeLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT);
        relparams.addRule(RelativeLayout.ALIGN_PARENT_TOP);
        relparams.addRule(RelativeLayout.ALIGN_PARENT_LEFT);
        this.menuTab.setLayoutParams(relparams);
        this.menuTab.setBackgroundResource(R.drawable.tab_menu);
        //this.menuTab.setScaleType(ImageView.ScaleType.CENTER);
        this.menuTab.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                selectTab(menuTab);
            }
        });
        this.tabHolder.addView(this.menuTab);

        //reach tab
        this.reachTab = new ImageView(this.context);
        this.reachTab.setId(ID_REACH_TAB);
        relparams = new RelativeLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT);
        relparams.addRule(RelativeLayout.ALIGN_PARENT_LEFT);
        relparams.addRule(RelativeLayout.BELOW, ID_MENU_TAB);
        relparams.topMargin = NEGATIVE_OVERLAP;
        this.reachTab.setLayoutParams(relparams);
        this.reachTab.setBackgroundResource(R.drawable.tab_reach);
        //this.menuTab.setScaleType(ImageView.ScaleType.CENTER);
        this.reachTab.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                selectTab(reachTab);
            }
        });
        this.tabHolder.addView(this.reachTab);

        //keyboard tab
        this.keyboardTab = new ImageView(this.context);
        this.keyboardTab.setId(ID_KB_TAB);
        relparams = new RelativeLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT);
        relparams.addRule(RelativeLayout.ALIGN_PARENT_LEFT);
        relparams.addRule(RelativeLayout.BELOW, ID_REACH_TAB);
        relparams.topMargin = NEGATIVE_OVERLAP;
        this.keyboardTab.setLayoutParams(relparams);
        this.keyboardTab.setBackgroundResource(R.drawable.tab_kb);
        //this.menuTab.setScaleType(ImageView.ScaleType.CENTER);
        this.keyboardTab.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                selectTab(keyboardTab);
            }
        });
        this.tabHolder.addView(this.keyboardTab);

        this.fadingEdge = new ImageView(this.context);
        relparams = new RelativeLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.MATCH_PARENT);
        relparams.addRule(RelativeLayout.RIGHT_OF, ID_MENU_TAB);
        this.fadingEdge.setLayoutParams(relparams);
        this.fadingEdge.setBackgroundResource(R.drawable.fading_edge);

        this.tabHolder.addView(this.fadingEdge);
        this.tabHolder.bringChildToFront(this.reachTab);
        this.tabHolder.bringChildToFront(this.menuTab);

        this.addView(this.contentView);
        this.addView(this.tabHolder);
    }

    private void initLeftHanded() {

        this.contentView.setId(ID_CONTENT_VIEW);
        LayoutParams params = new LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT);
        //params.addRule(LEFT_OF, ID_TAB_HOLDER);
        params.addRule(ALIGN_PARENT_LEFT);
        params.addRule(ALIGN_PARENT_BOTTOM);
        this.contentView.setLayoutParams(params);

        this.tabHolder = new RelativeLayout(this.context);
        this.tabHolder.setId(ID_TAB_HOLDER);
        params = new LayoutParams(this.getToggleViewWidth(true), ViewGroup.LayoutParams.MATCH_PARENT);
        params.addRule(RIGHT_OF, ID_CONTENT_VIEW);
        //params.addRule(ALIGN_PARENT_RIGHT);
        this.tabHolder.setLayoutParams(params);
        this.tabHolder.setBackgroundColor(Theme.BACKGROUND_COLOR);

        //menu tab
        this.menuTab = new ImageView(this.context);
        this.menuTab.setId(ID_MENU_TAB);

        RelativeLayout.LayoutParams relparams = new RelativeLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT);
        relparams.addRule(RelativeLayout.ALIGN_PARENT_TOP);
        relparams.addRule(RelativeLayout.ALIGN_PARENT_RIGHT);
        this.menuTab.setLayoutParams(relparams);
        this.menuTab.setBackgroundResource(R.drawable.tab_menu_lefty);
        //this.menuTab.setScaleType(ImageView.ScaleType.CENTER);
        this.menuTab.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                selectTab(menuTab);
            }
        });
        this.tabHolder.addView(this.menuTab);

        //reach tab
        this.reachTab = new ImageView(this.context);
        this.reachTab.setId(ID_REACH_TAB);
        relparams = new RelativeLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT);
        relparams.addRule(RelativeLayout.ALIGN_PARENT_RIGHT);
        relparams.addRule(RelativeLayout.BELOW, ID_MENU_TAB);
        relparams.topMargin = NEGATIVE_OVERLAP;
        this.reachTab.setLayoutParams(relparams);
        this.reachTab.setBackgroundResource(R.drawable.tab_reach_lefty);
        //this.menuTab.setScaleType(ImageView.ScaleType.CENTER);
        this.reachTab.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                selectTab(reachTab);
            }
        });
        this.tabHolder.addView(this.reachTab);

        //keyboard tab
        this.keyboardTab = new ImageView(this.context);
        this.keyboardTab.setId(ID_KB_TAB);
        relparams = new RelativeLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT);
        relparams.addRule(RelativeLayout.ALIGN_PARENT_RIGHT);
        relparams.addRule(RelativeLayout.BELOW, ID_REACH_TAB);
        relparams.topMargin = NEGATIVE_OVERLAP;
        this.keyboardTab.setLayoutParams(relparams);
        this.keyboardTab.setBackgroundResource(R.drawable.tab_kb_lefty);
        //this.menuTab.setScaleType(ImageView.ScaleType.CENTER);
        this.keyboardTab.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                selectTab(keyboardTab);
            }
        });
        this.tabHolder.addView(this.keyboardTab);

        this.fadingEdge = new ImageView(this.context);
        relparams = new RelativeLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.MATCH_PARENT);
        relparams.addRule(RelativeLayout.LEFT_OF, ID_MENU_TAB);
        this.fadingEdge.setLayoutParams(relparams);
        this.fadingEdge.setBackgroundResource(R.drawable.fading_edge_left);

        this.tabHolder.addView(this.fadingEdge);
        this.tabHolder.bringChildToFront(this.reachTab);
        this.tabHolder.bringChildToFront(this.menuTab);

        this.addView(this.contentView);
        this.addView(this.tabHolder);
    }*/

    private void createScreens() {
        this.screens = new ArrayList<SideScreen>();

        int count = 0;

        this.settingsScreen = new SettingsScreen(this.context);
        this.settingsScreen.init(this.keyboardService, this.kbContainerView, this, this.isLefty, 0);
        this.settingsScreen.setTabTopMargin( this.screens.size() * TAB_MARGIN_INCREMENT );
        this.screens.add(this.settingsScreen);
        this.addView(this.settingsScreen);
        TAB_INDEX_MENU_SCREEN = count;
        count++;

        if (IKnowUKeyboardService.MINIAPP_ON) {
            this.miniAppScreen = new MiniAppScreen(this.context);
            this.miniAppScreen.init(this.keyboardService, this.kbContainerView, this, this.isLefty, 1);
            this.miniAppScreen.setTabTopMargin( this.screens.size() * TAB_MARGIN_INCREMENT );
            this.screens.add(this.miniAppScreen);
            this.addView(this.miniAppScreen);
            TAB_INDEX_MINIAPP_SCREEN = count;
            count++;
        }

        this.sideKeyboardScreen = new SideKeyboardScreen(this.context);
        this.sideKeyboardScreen.init(this.keyboardService, this.kbContainerView, this, this.isLefty, 2);
        this.sideKeyboardScreen.setTabTopMargin( this.screens.size() * TAB_MARGIN_INCREMENT );
        this.screens.add(this.sideKeyboardScreen);
        this.addView(this.sideKeyboardScreen);
        TAB_INDEX_SIDE_KEYBOARD = count;
        count++;

        this.voiceScreen = new VoiceScreen(this.context);
        this.voiceScreen.init(this.keyboardService, this.kbContainerView, this, this.isLefty, 0);
        this.voiceScreen.setTabTopMargin( this.screens.size() * TAB_MARGIN_INCREMENT );
        this.screens.add(this.voiceScreen);
        this.addView(this.voiceScreen);
        TAB_INDEX_VOICE_SCREEN = count;
        count++;

        this.abbreviationScreen = new AbbreviationScreen(this.context);
        this.abbreviationScreen.init(this.keyboardService, this.kbContainerView, this, this.isLefty, 0);
        this.abbreviationScreen.setTabTopMargin( this.screens.size() * TAB_MARGIN_INCREMENT );
        this.screens.add(this.abbreviationScreen);
        this.addView(this.abbreviationScreen);
        TAB_INDEX_ABBREVIATION_SCREEN = count;
    }

    /**
     * Restore the side screen to the state set by the user. The state of side screens is stored in the
     * shared preferences.
     */
    public void restoreSideScreen() {
        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(this.context);
        String openSideScreen = sp.getString(OPEN_SIDE_SCREEN, "");

        SideScreen sideScreen = null;
        if (openSideScreen.equals(SIDE_KEYBOARD_SCREEN)) {
            SideKeyboardScreen sideKeyboardScreen = getSideKeyboardScreen();

            // Restore the type of keyboard selected.
            int sideKeyboardType = sp.getInt(SIDE_KEYBOARD_SCREEN_TYPE, KeyboardLinearLayout.NUMERIC);
            sideKeyboardScreen.setKeyboard(sideKeyboardType);
            sideScreen = sideKeyboardScreen;

        } else if (openSideScreen.equals(SETTINGS_SCREEN)) {
            sideScreen = getSettingsScreen();

        } else if (openSideScreen.equals(MINI_APP_SCREEN)) {
            sideScreen = getMiniAppScreen();

        } else if (openSideScreen.equals(VOICE_SCREEN)) {
            sideScreen = getVoiceScreen();

        } else if (openSideScreen.equals(ABBREVIATION_SCREEN)) {
            sideScreen = getAbbreviationScreen();
        }

        if (sideScreen != null) {
            this.isOpen = true;
            if (this.lastSelected != null) {
                this.lastSelected.toggleSelected();
            }
            this.lastSelected = sideScreen;
            this.lastSelected.toggleSelected();
            this.lastSelected.bringToFront();
            this.requestLayout();
            this.invalidate();
        }
    }

    public void resizeComponents(int width) {
        if (this.isLefty) {
            //IKnowUKeyboardService.log(Log.VERBOSE, "SideRelativeLayout.resizeComponents()", "width = "+width+", toggleWidth = "+this.getToggleViewWidth(true));
            this.settingsScreen.resizeContentView(width);
            if (IKnowUKeyboardService.MINIAPP_ON)
                this.miniAppScreen.resizeContentView(width);
            this.sideKeyboardScreen.resizeContentView(width);
            this.voiceScreen.resizeContentView(width);
            this.abbreviationScreen.resizeContentView(width);

            //this.contentView.requestLayout();
            this.requestLayout();

            //IKnowUKeyboardService.log(Log.VERBOSE, "SideRelativeLayout.resizecomponents()", "contentView.width = "+this.contentView.getWidth());
            IKnowUKeyboardService.log(Log.INFO, "SideRelativeLayout.resizecomponents()", "height = "+this.getHeight());
        }
    }

    public MiniAppScreen getMiniAppScreen() {
        if (IKnowUKeyboardService.MINIAPP_ON) {
            return (MiniAppScreen) this.screens.get(TAB_INDEX_MINIAPP_SCREEN);
        } else return null;
    }

    public SideKeyboardScreen getSideKeyboardScreen() {
        return (SideKeyboardScreen) this.screens.get(TAB_INDEX_SIDE_KEYBOARD);
    }

    public VoiceScreen getVoiceScreen() {
        return (VoiceScreen) this.screens.get(TAB_INDEX_VOICE_SCREEN);
    }

    public SettingsScreen getSettingsScreen() {
        return (SettingsScreen) this.screens.get(TAB_INDEX_MENU_SCREEN);
    }

    public AbbreviationScreen getAbbreviationScreen() {
        return (AbbreviationScreen) this.screens.get(TAB_INDEX_ABBREVIATION_SCREEN);
    }

    /*public RelativeLayout getToggleView() {
        return this.tabHolder;
    }*/

    public int getToggleViewWidth(boolean inclusive) {
        if (inclusive) {
            return TAB_HOLDER_WIDTH + FADING_EDGE_WIDTH;
        } else {
            return TAB_HOLDER_WIDTH;
        }
    }

    public boolean isOpen() {
        return isOpen;
    }

    public void highlightReachBG() {
        if (IKnowUKeyboardService.MINIAPP_ON)
            this.miniAppScreen.setTabActivated(true);
    }

    public void unHighlightReachBG() {
        if (IKnowUKeyboardService.MINIAPP_ON)
            this.miniAppScreen.setTabActivated(false);
    }

    public void startAnimation() {
        if (IKnowUKeyboardService.MINIAPP_ON) {
            Animation anim = AnimationUtils.loadAnimation(this.context, R.anim.mini_app_up_down);
            this.miniAppScreen.startTabAnimation();
        }
    }

    public void onConfigurationChanged(Configuration newConfig) {
        this.onFinishInput();
    }

    public void onFinishInput() {

        if (this.settingsScreen != null && this.settingsScreen.isInPopup())
            this.moveScreenToKB(this.settingsScreen);
        if (this.miniAppScreen != null && this.miniAppScreen.isInPopup())
            this.moveScreenToKB(this.miniAppScreen);
        if (this.sideKeyboardScreen != null && this.sideKeyboardScreen.isInPopup())
            this.moveScreenToKB(this.sideKeyboardScreen);
        if (this.voiceScreen != null && this.voiceScreen.isInPopup())
            this.moveScreenToKB(this.voiceScreen);
        if (this.abbreviationScreen != null && this.abbreviationScreen.isInPopup())
            this.moveScreenToKB(this.abbreviationScreen);

        //this.popups.clear();
    }

    /*private void setContentViewParams( int wrapOrMatch) {
        if (this.isLefty) {
            LayoutParams params = new LayoutParams(this.getWidth() - this.getToggleViewWidth(true), wrapOrMatch);
            params.addRule(ALIGN_PARENT_LEFT);
            //params.addRule(LEFT_OF, ID_TAB_HOLDER);
            params.addRule(CENTER_VERTICAL);
            this.contentView.setId(ID_CONTENT_VIEW);
            this.contentView.setLayoutParams(params);

            params = new LayoutParams(this.getToggleViewWidth(true), ViewGroup.LayoutParams.MATCH_PARENT);
            params.addRule(RIGHT_OF, ID_CONTENT_VIEW);
            //params.addRule(ALIGN_PARENT_RIGHT);
            this.tabHolder.setLayoutParams(params);
        } else {
            LayoutParams params = new LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, wrapOrMatch);
            params.addRule(RIGHT_OF, ID_TAB_HOLDER);
            params.addRule(CENTER_VERTICAL);
            this.contentView.setLayoutParams(params);
        }
        this.contentView.requestLayout();
        this.requestLayout();
    }*/

    public void selectTab(SideScreen screen) {

        boolean needsToggle = false;
        if (this.isOpen()) {
            if ( this.lastSelected != null && !screen.equals(this.lastSelected) ) {
                this.lastSelected.toggleSelected();
            } else if ( this.lastSelected != null && screen.equals(this.lastSelected) ) {
                needsToggle = true;
            } else if ( this.lastSelected == null ) {
                needsToggle = true;
            }
        } else {
            needsToggle = true;
        }

        this.lastSelected = screen;

        if (needsToggle) {
            this.toggle();
        }

        this.lastSelected.setOpenState(this.isOpen);

        this.requestLayout();
        this.invalidate();
    }

    public void toggle() {
        this.isOpen = !this.isOpen;
        this.kbContainerView.startAnimation();
    }

    public void moveScreenToKB(SideScreen screen) {
        screen.dismissPopupWindow();

        screen.setParams();

        if (screen.index < this.getChildCount()) {
            this.addView(screen, screen.getIndex());
        } else {
            this.addView(screen);
        }
    }

    public void moveScreenToPopup(SideScreen screen, int width, int height) {
        int[] loc = new int[2];
        this.getLocationOnScreen(loc);

        int screenHeight = Size.getScreenHeight(this.context);
        int screenWidth = Size.getScreenWidth(this.context);

        int ypos = 50;
        int xpos = (screenWidth / 2) - (width / 2);

        this.removeView(screen);

        //IKnowUKeyboardService.log(Log.VERBOSE, "SideRelativeLayou.moveScreenToPopup()", "after screen width = "+screen.getWidth()+", height = "+screen.getHeight());
        SidePopup popup = new SidePopup(this.context, screen, width, height, false);
        popup.showPopupView(this, xpos, ypos);
        //this.popups.add(popup);

        if (this.isOpen() && this.getChildCount() <= 0) {
            this.toggle();
        }
    }
}
