package com.iknowu.sidelayout;

import android.content.Context;
import android.content.SharedPreferences;
import android.graphics.Paint;
import android.preference.PreferenceManager;
import android.util.AttributeSet;
import android.view.Gravity;
import android.view.View;
import android.widget.FrameLayout;

import com.iknowu.IKnowUKeyboard;
import com.iknowu.IKnowUKeyboardService;
import com.iknowu.IKnowUKeyboardView;
import com.iknowu.KeyboardContainerView;
import com.iknowu.KeyboardLinearLayout;
import com.iknowu.R;
import com.iknowu.util.Theme;

/**
 * Created by Justin on 27/09/13.
 *
 */
public class SideKeyboardScreen extends SideScreen {

    private Context context;

    private IKnowUKeyboardView kbView;
    private int measuredWidth;

    private Paint paint;

    public SideKeyboardScreen(Context context) {
        super(context);
        this.context = context;
        this.paint = new Paint();
    }

    public SideKeyboardScreen(Context context, AttributeSet attrs) {
        super(context, attrs);
        this.context = context;
        this.paint = new Paint();
    }

    public SideKeyboardScreen(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        this.context = context;
        this.paint = new Paint();
    }

    @Override
    public void init(IKnowUKeyboardService serv, KeyboardContainerView kbcv, SideRelativeLayout sfl, boolean isLefty, int index) {

        super.init(serv, kbcv, sfl, isLefty, index);

        FrameLayout flay = new FrameLayout(this.context);
        LayoutParams params = new LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT);
        flay.setLayoutParams(params);
        flay.setBackgroundColor(Theme.SIDE_BACKGROUND_COLOR);

        this.tab = new SideTab(this.context);
        this.tab.init(this);
        this.tab.setImageResource(this.isLefty ? R.drawable.tab_kb_lefty : R.drawable.tab_kb);
        params = new LayoutParams(LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT);
        this.tab.setLayoutParams(params);

        this.kbView = new IKnowUKeyboardView(this.context);
        this.kbView.setKeyboardService(this.keyboardService);
        this.kbView.setAsSideView();
        this.kbView.processAttributes();
        FrameLayout.LayoutParams fparams = new FrameLayout.LayoutParams(LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT);
        fparams.gravity = Gravity.CENTER;
        this.kbView.setLayoutParams(fparams);
        this.setKeyboard(KeyboardLinearLayout.NUMERIC);

        flay.addView(this.kbView);
        this.contentView = flay;

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
        this.kbView.setEnabled(enable);
        this.kbView.setVisibility((enable) ? View.VISIBLE : View.INVISIBLE);
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
     * Saved the keyboard type in shared preferences.
     * @param kbType keyboard type
     */
    public void saveKeyboardTypePreference(int kbType) {
        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(this.context);
        SharedPreferences.Editor edit = sp.edit();
        edit.putInt(SideRelativeLayout.SIDE_KEYBOARD_SCREEN_TYPE, kbType);
        edit.commit();
    }

    /**
     * Handle open/close state of keyboard screen.
     * @param isOpen the state flag
     */
    @Override
    public void setOpenState(boolean isOpen) {
        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(this.context);
        SharedPreferences.Editor edit = sp.edit();
        edit.putString(SideRelativeLayout.OPEN_SIDE_SCREEN, (isOpen) ? SideRelativeLayout.SIDE_KEYBOARD_SCREEN : "");
        edit.commit();
    }

    @Override
    public void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        super.onMeasure(widthMeasureSpec, heightMeasureSpec);

        if ( this.kbView.getKeyboard() != null && this.getMeasuredWidth() != this.measuredWidth) {
            this.kbView.getKeyboard().resizeKeys(this.getMeasuredWidth()-this.tab.getMeasuredWidth());
        }

        this.measuredWidth = this.getMeasuredWidth();
    }

    public void setKeyboard(int kblayout) {
        IKnowUKeyboard kb = null;
        switch (kblayout) {
            case KeyboardLinearLayout.NUMERIC:
                kb = new IKnowUKeyboard(this.context, R.xml.tablet_side_numeric, 0);
                break;
            case KeyboardLinearLayout.SYMBOLS:
                kb = new IKnowUKeyboard(this.context, R.xml.tablet_side_symbols, 0);
                break;
            case KeyboardLinearLayout.SYMBOLS_2:
                kb = new IKnowUKeyboard(this.context, R.xml.tablet_side_symbols2, 0);
                break;
            case KeyboardLinearLayout.EXTRA_LETTERS:
                kb = new IKnowUKeyboard(this.context, R.xml.tablet_side_extra_letters, 0);
                break;
            case KeyboardLinearLayout.SMILEY:
                kb = new IKnowUKeyboard(this.context, R.xml.tablet_side_smiley, 0);
                break;
            case KeyboardLinearLayout.NAVIGATION:
                kb = new IKnowUKeyboard(this.context, R.xml.tablet_side_navigation, 0);
                break;
            default:
                kb = new IKnowUKeyboard(this.context, R.xml.tablet_side_numeric, 0);
                break;
        }

        kb.resizeKeys(this.getMeasuredWidth() - this.tab.getMeasuredWidth());
        this.kbView.setKeyboard(kb);

        this.kbView.invalidate();
    }
}
