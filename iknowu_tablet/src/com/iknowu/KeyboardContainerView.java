package com.iknowu;

import android.content.Context;
import android.content.res.Configuration;
import android.graphics.Canvas;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.widget.LinearLayout;

import com.iknowu.sidelayout.SideRelativeLayout;

//import android.view.ViewGroup;
//import android.widget.LinearLayout;

/**
 * Created by Justin on 25/09/13.
 *
 */
public class KeyboardContainerView extends LinearLayout {

    private static final float KEYBOARD_MIN_PERCENT = 0.65f;
    public static final float SIDE_MAX_PERCENT = 0.35f;

    private static final float ANIM_DURATION = 200.0f;

    private Context context;
    private KeyboardLinearLayout kbLinearLayout;
    private SideRelativeLayout sideRelativeLayout;
    private IKnowUKeyboardService kbService;

    public int myWidth;

    private float currentKeyboardPercent;
    private long animStartTime;

    public boolean isAnimating;
    private boolean isLefty;
    private boolean tabsOnLeft;

    public KeyboardContainerView(Context context) {
        super(context);
        this.context = context;
    }

    public KeyboardContainerView(Context context, AttributeSet attrs) {
        super(context, attrs);
        this.context = context;
    }

    public KeyboardContainerView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        this.context = context;
    }

    @Override
    public void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        super.onMeasure(widthMeasureSpec, heightMeasureSpec);

        this.myWidth = MeasureSpec.getSize(widthMeasureSpec);
        //IKnowUKeyboardService.log(Log.VERBOSE, "KeyboardContainerView.oMeasure()", "myWidth = "+myWidth);
        this.resizeComponents();
    }

    public void init(IKnowUKeyboardService serv, int kbLayoutId, int themeId, int kbMode, boolean islefty, boolean tabsLeft) {
        this.setBackgroundColor(0x00FFFFFF);

        //this.setOrientation(LinearLayout.HORIZONTAL);

        this.kbService = serv;
        this.isLefty = islefty;
        this.tabsOnLeft = tabsLeft;

        LayoutInflater li = (LayoutInflater) this.context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);

        this.kbLinearLayout = (KeyboardLinearLayout) li.inflate(kbLayoutId, null);
        assert this.kbLinearLayout != null;

        LinearLayout.LayoutParams params = new LayoutParams(0, LayoutParams.MATCH_PARENT, KEYBOARD_MIN_PERCENT);
        this.kbLinearLayout.setLayoutParams(params);
        this.kbLinearLayout.init(serv, this, themeId, kbMode, isLefty);

        this.sideRelativeLayout = new SideRelativeLayout(this.context);
        params = new LayoutParams(0, LayoutParams.MATCH_PARENT, SIDE_MAX_PERCENT);
        this.sideRelativeLayout.setLayoutParams(params);
        this.sideRelativeLayout.init(serv, this, this.tabsOnLeft);

        if (this.tabsOnLeft) {
            this.addView(this.sideRelativeLayout);
            this.addView(this.kbLinearLayout);
        } else {
            this.addView(this.kbLinearLayout);
            this.addView(this.sideRelativeLayout);
        }
    }

    public KeyboardLinearLayout getKbLinearLayout() {
        return this.kbLinearLayout;
    }

    public SideRelativeLayout getSideRelativeLayout() {
        return this.sideRelativeLayout;
    }

    public void onConfigurationChanged(Configuration newConfig) {
        this.kbLinearLayout.onConfigurationChanged(newConfig);
        this.sideRelativeLayout.onConfigurationChanged(newConfig);
    }

    public void onFinishInput() {
        this.kbLinearLayout.onFinishInput();
        this.sideRelativeLayout.onFinishInput();
    }

    @Override
    public void onDraw(Canvas canvas) {
        this.resizeComponents();
        super.onDraw(canvas);
    }

    /**
     * Restore the state of the side screen.
     */
    public void restoreSideScreen() {
        if (this.sideRelativeLayout != null) {
            this.sideRelativeLayout.restoreSideScreen();
        }
    }

    public void startAnimation() {
        this.isAnimating = true;
        this.animStartTime = System.currentTimeMillis();

        this.resizeComponents();
    }

    /**
     * Resize the underlying views to either show or hide the side panel.
     * If the animating flag has been set, then scale the view appropriately
     * given the time elapsed.
     */
    public void resizeComponents() {
        //IKnowUKeyboardService.log(Log.INFO, "KeyboardContainerView.resizeComponents()", "sideRelativeLayout.isOpen = "+this.sideRelativeLayout.isOpen() + ", isAnimating = "+isAnimating);

        if (this.tabsOnLeft) {
            this.resizeLeftHanded();
            //this.resizeRightHanded();
        } else {
            this.resizeRightHanded();
        }
    }

    private void resizeRightHanded() {
        //if currently animating
        if (this.isAnimating) {
            float perc = (System.currentTimeMillis() - this.animStartTime) / ANIM_DURATION;
            //if the time elapsed is not greater than the animation duration
            if (perc <= 1) {
                //sliding open
                if (this.sideRelativeLayout.isOpen()) {
                    float startWidth = this.myWidth - this.sideRelativeLayout.getToggleViewWidth(false);
                    float targetKbWidth = this.myWidth * KEYBOARD_MIN_PERCENT;
                    perc = 1 - perc;
                    int kbWidth = (int) (targetKbWidth + ( perc * (startWidth - targetKbWidth) ) );
                    if (kbWidth < targetKbWidth) {
                        kbWidth = (int) targetKbWidth;
                    }
                    //IKnowUKeyboardService.log(Log.VERBOSE, "KeyboardContainerView.resizeComponents()", "kbWidth = "+kbWidth);
                    this.kbLinearLayout.resizeComponents(kbWidth);
                    //sliding closed
                } else {
                    float startWidth = this.myWidth * KEYBOARD_MIN_PERCENT;
                    float targetKbWidth = this.myWidth - this.sideRelativeLayout.getToggleViewWidth(false);
                    int kbWidth = (int) (startWidth + ( perc * (targetKbWidth - startWidth) ) );
                    if (kbWidth > targetKbWidth) {
                        kbWidth = (int) targetKbWidth;
                    }
                    //IKnowUKeyboardService.log(Log.VERBOSE, "KeyboardContainerView.resizeComponents()", "kbWidth = "+kbWidth);
                    this.kbLinearLayout.resizeComponents(kbWidth);
                }
                this.invalidate();
                //time elapsed is greater than animation duration
                //show the views in their final positions;
            } else {
                this.isAnimating = false;

                if (this.sideRelativeLayout.isOpen()) {
                    int kbWidth = (int) (this.myWidth * KEYBOARD_MIN_PERCENT);
                    if (kbWidth != this.kbLinearLayout.getWidth()) {
                        this.kbLinearLayout.resizeComponents(kbWidth);
                    }
                } else {
                    int kbWidth = this.myWidth - this.sideRelativeLayout.getToggleViewWidth(false);
                    if (kbWidth != this.kbLinearLayout.getWidth()) {
                        //IKnowUKeyboardService.log(Log.INFO, "KeyboardContainerView.resizeComponents()", "kbWidth = "+kbWidth);
                        this.kbLinearLayout.resizeComponents(kbWidth);
                    }
                }
            }
            //not currently animating
        } else {
            if (this.sideRelativeLayout.isOpen()) {
                int kbWidth = (int) (this.myWidth * KEYBOARD_MIN_PERCENT);
                if (kbWidth != this.kbLinearLayout.getWidth()) {
                    this.kbLinearLayout.resizeComponents(kbWidth);
                }
            } else {
                int kbWidth = this.myWidth - this.sideRelativeLayout.getToggleViewWidth(false);
                if (kbWidth != this.kbLinearLayout.getWidth()) {
                    //IKnowUKeyboardService.log(Log.INFO, "KeyboardContainerView.resizeComponents()", "kbWidth = "+kbWidth);
                    this.kbLinearLayout.resizeComponents(kbWidth);
                }
            }
        }
    }

    private void resizeLeftHanded() {
        //if currently animating
        if (this.isAnimating) {
            float perc = (System.currentTimeMillis() - this.animStartTime) / ANIM_DURATION;
            //if the time elapsed is not greater than the animation duration
            if (perc <= 1) {
                //sliding open
                if (this.sideRelativeLayout.isOpen()) {
                    float startWidth = this.myWidth - this.sideRelativeLayout.getToggleViewWidth(true);
                    float targetKbWidth = this.myWidth * KEYBOARD_MIN_PERCENT;
                    perc = 1 - perc;
                    int kbWidth = (int) (targetKbWidth + ( perc * (startWidth - targetKbWidth) ) );
                    if (kbWidth < targetKbWidth) {
                        kbWidth = (int) targetKbWidth;
                    }
                    //IKnowUKeyboardService.log(Log.VERBOSE, "KeyboardContainerView.resizeComponents()", "kbWidth = "+kbWidth);
                    this.kbLinearLayout.resizeComponents(kbWidth);
                    this.sideRelativeLayout.resizeComponents(this.myWidth-kbWidth - this.sideRelativeLayout.getToggleViewWidth(true));
                //sliding closed
                } else {
                    float startWidth = this.myWidth * KEYBOARD_MIN_PERCENT;
                    float targetKbWidth = this.myWidth - this.sideRelativeLayout.getToggleViewWidth(false);
                    int kbWidth = (int) (startWidth + ( perc * (targetKbWidth - startWidth) ) );
                    if (kbWidth > targetKbWidth) {
                        kbWidth = (int) targetKbWidth;
                    }
                    //IKnowUKeyboardService.log(Log.VERBOSE, "KeyboardContainerView.resizeComponents()", "kbWidth = "+kbWidth);
                    this.kbLinearLayout.resizeComponents(kbWidth);
                    this.sideRelativeLayout.resizeComponents(this.myWidth-kbWidth - this.sideRelativeLayout.getToggleViewWidth(false) );
                }
                this.invalidate();
            //time elapsed is greater than animation duration
            //show the views in their final positions;
            } else {
                this.isAnimating = false;

                if (this.sideRelativeLayout.isOpen()) {
                    int kbWidth = (int) (this.myWidth * KEYBOARD_MIN_PERCENT);
                    if (kbWidth != this.kbLinearLayout.getWidth()) {
                        this.kbLinearLayout.resizeComponents(kbWidth);
                        this.sideRelativeLayout.resizeComponents(this.myWidth-kbWidth - this.sideRelativeLayout.getToggleViewWidth(true) );
                    }
                } else {
                    int kbWidth = this.myWidth - this.sideRelativeLayout.getToggleViewWidth(false);
                    if (kbWidth != this.kbLinearLayout.getWidth()) {
                        //IKnowUKeyboardService.log(Log.INFO, "KeyboardContainerView.resizeComponents()", "kbWidth = "+kbWidth);
                        this.kbLinearLayout.resizeComponents(kbWidth);
                        this.sideRelativeLayout.resizeComponents(0);
                    }
                }
            }
            //not currently animating
            this.invalidate();
        } else {
            if (this.sideRelativeLayout.isOpen()) {
                int kbWidth = (int) (this.myWidth * KEYBOARD_MIN_PERCENT);
                if (kbWidth != this.kbLinearLayout.getWidth()) {
                    this.kbLinearLayout.resizeComponents(kbWidth);
                    this.sideRelativeLayout.resizeComponents(this.myWidth-kbWidth - this.sideRelativeLayout.getToggleViewWidth(true) );
                }
            } else {
                int kbWidth = this.myWidth - this.sideRelativeLayout.getToggleViewWidth(false);
                if (kbWidth != this.kbLinearLayout.getWidth()) {
                    //IKnowUKeyboardService.log(Log.INFO, "KeyboardContainerView.resizeComponents()", "kbWidth = "+kbWidth);
                    this.kbLinearLayout.resizeComponents(kbWidth);
                    this.sideRelativeLayout.resizeComponents(0);
                }
            }
            this.invalidate();
        }
    }
}
