package com.iknowu.sidelayout;

import android.content.Context;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;

import com.iknowu.IKnowUKeyboardService;
import com.iknowu.KeyboardContainerView;
import com.iknowu.R;

/**
 * Created by Justin on 25/10/13.
 *
 */
public class SideScreen extends LinearLayout {

    protected Context context;

    protected IKnowUKeyboardService keyboardService;
    protected KeyboardContainerView keyboardContainerView;
    protected SideRelativeLayout sideRelativeLayout;

    protected SidePopup popupWindow;

    protected SideTab tab;
    protected View contentView;
    protected boolean isLefty;
    protected boolean isSelected;

    protected int index;

    protected boolean inPopup;

    public SideScreen( Context ctx ) {
        super(ctx);
        this.context = ctx;
        //this.setParams();
    }

    public SideScreen(Context context, AttributeSet attrs) {
        super(context, attrs);
        this.context = context;
        //this.setParams();
    }

    public SideScreen(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        this.context = context;
        //this.setParams();
    }

    public void setParams() {
        this.setOrientation(HORIZONTAL);

        RelativeLayout.LayoutParams params = new RelativeLayout.LayoutParams(RelativeLayout.LayoutParams.MATCH_PARENT, RelativeLayout.LayoutParams.MATCH_PARENT);
        if (this.isLefty){
            params.addRule(RelativeLayout.ALIGN_PARENT_RIGHT);
            params.addRule(RelativeLayout.ALIGN_PARENT_TOP);
        } else {
            params.addRule(RelativeLayout.ALIGN_PARENT_LEFT);
            params.addRule(RelativeLayout.ALIGN_PARENT_TOP);
        }
        this.setLayoutParams(params);

        if (this.contentView != null) {
            LinearLayout.LayoutParams linparams = (LayoutParams) this.contentView.getLayoutParams();
            linparams.width = params.width;
            linparams.height = params.height;
            this.contentView.setLayoutParams(linparams);
        }
    }

    public void init(IKnowUKeyboardService serv, KeyboardContainerView kbcv, SideRelativeLayout sfl, boolean isLefty, int index) {
        this.isSelected = false;
        this.keyboardService = serv;
        this.keyboardContainerView = kbcv;
        this.sideRelativeLayout = sfl;
        this.isLefty = isLefty;

        this.index = index;

        this.setParams();
    }

    @Override
    public boolean onInterceptTouchEvent(MotionEvent ev) {

        //IKnowUKeyboardService.log(Log.VERBOSE, "SideScreen.onTouchEvent()", "action = "+ev.getAction());

        return super.onInterceptTouchEvent(ev);
    }

    /**
     * Handle open/close state of side screen.
     * @param isOpen the state flag
     */
    public void setOpenState(boolean isOpen) {
        // Do nothing
    }

    public void setIndex( int idx ) {
        this.index = idx;
    }

    public int getIndex() {
        return this.index;
    }

    public void setPopupWindow(SidePopup popup) {
        this.popupWindow = popup;
        this.inPopup = true;
        this.tab.setInPopup(true);
    }

    public void dismissPopupWindow() {
        this.inPopup = false;
        this.tab.setInPopup(false);
        if (this.popupWindow != null) {
            this.popupWindow.dismiss();
            this.popupWindow = null;
        }
    }

    public boolean isInPopup() {
        return this.inPopup;
    }

    public void movePopup(int xpos, int ypos) {
        if (this.popupWindow != null) {
            this.popupWindow.update(xpos, ypos, this.popupWindow.getWidth(), this.popupWindow.getHeight());
        }
    }

    public KeyboardContainerView getKeyboardContainerView() {
        return this.keyboardContainerView;
    }

    public void tabClickedInKeyboard() {
        this.toggleSelected();
        this.bringToFront();
        this.sideRelativeLayout.selectTab(this);
    }

    public void tabClickedInPopup() {
        this.sideRelativeLayout.moveScreenToKB(this);
    }

    public void toggleSelected() {
        this.isSelected = !this.isSelected;
        disableEnableScreen(this.isSelected);
        this.tab.setSelected(this.isSelected);
    }

    public void setNewLayoutParams(ViewGroup.LayoutParams params) {
        if (this.contentView != null) {
            LinearLayout.LayoutParams linparams = (LayoutParams) this.contentView.getLayoutParams();
            linparams.width = params.width - this.tab.getMeasuredWidth();
            linparams.height = params.height;
            this.contentView.setLayoutParams(linparams);
        }
        this.setLayoutParams(params);
    }

    public void tabLongClicked() {
        int width = (int) (this.keyboardContainerView.myWidth * KeyboardContainerView.SIDE_MAX_PERCENT) - this.tab.getWidth();

        LayoutParams params = (LayoutParams) this.contentView.getLayoutParams();
        params.width = width;

        this.contentView.setLayoutParams(params);

        this.sideRelativeLayout.moveScreenToPopup(this, width + this.tab.getWidth(), this.getHeight());
    }

    public void setTabTopMargin(int margin) {
        //IKnowUKeyboardService.log(Log.VERBOSE, "SideScreen.setTabTopMargin()", "margin = "+margin);
        LinearLayout.LayoutParams params = (LayoutParams) this.tab.getLayoutParams();
        params.topMargin = margin;
        this.tab.setLayoutParams(params);
    }

    public void setTabActivated(boolean activated) {
        this.tab.setActivated(activated);
    }

    public void startTabAnimation() {
        Animation anim = AnimationUtils.loadAnimation(this.context, R.anim.mini_app_up_down);
        this.tab.startAnimation(anim);
    }

    public void resizeContentView(int width) {
        ViewGroup.LayoutParams params = this.contentView.getLayoutParams();
        params.width = width;
        this.contentView.setLayoutParams(params);
    }

    /**
     * Enable/disable screen.
     * @param enable enable/disable flag
     */
    public void disableEnableScreen(boolean enable) {
        // Do nothing
    }

    /**
     * Enable/disable child controls in view group.
     * @param enable enable/disable flag
     * @param vg the view group
     */
    public void disableEnableChildControls(boolean enable, ViewGroup vg) {
        for (int i = 0; i < vg.getChildCount(); i++){
            View child = vg.getChildAt(i);
            child.setEnabled(enable);
            child.setVisibility((enable) ? View.VISIBLE : View.INVISIBLE);
            if (child instanceof ViewGroup){
                disableEnableChildControls(enable, (ViewGroup)child);
            }
        }
    }
}
