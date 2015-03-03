package com.iknowu.sidelayout;

import android.content.Context;
import android.view.Gravity;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.PopupWindow;
import android.widget.RelativeLayout;

import com.iknowu.R;

import java.lang.reflect.Method;

/**
 * Created by Justin on 25/10/13.
 *
 */
public class SidePopup extends PopupWindow {

    public static final int PADDING = 8;
    public static final int MIN_WIDTH = 150;
    public static final int MIN_HEIGHT = 150;

    public static final int EXPANDER_RIGHT_WIDTH = 25;
    public static final int EXPANDER_RIGHT_HEIGHT = 50;
    public static final int EXPANDER_BOTTOM_WIDTH = 50;
    public static final int EXPANDER_BOTTOM_HEIGHT = 35;

    public static final int SCREEN_ID = 12345;

    public SideScreen screen;
    public SideRelativeLayout sideRelativeLayout;
    public Context context;

    public RelativeLayout contentLayout;

    public int width;
    public int height;

    public ExpanderBar rightExpander;
    public ExpanderBar bottomExpander;

    public SidePopup(Context context, SideScreen content, int width, int height, boolean focusable) {
        super(context);
        //super(content, ( width + (2 * PADDING) ), height, focusable);

        this.width = width + EXPANDER_RIGHT_WIDTH;
        this.height = height + EXPANDER_BOTTOM_HEIGHT;

        this.setWidth(this.width);
        this.setHeight(this.height);

        this.context = context;
        this.screen = content;
        this.screen.setId(SCREEN_ID);
        this.screen.setPopupWindow(this);

        this.contentLayout = new RelativeLayout(this.context);
        ViewGroup.LayoutParams params = new ViewGroup.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT);
        this.contentLayout.setLayoutParams(params);

        RelativeLayout.LayoutParams relparams = new RelativeLayout.LayoutParams(width, height);
        relparams.addRule(RelativeLayout.ALIGN_PARENT_TOP);
        relparams.addRule(RelativeLayout.ALIGN_PARENT_LEFT);
        this.screen.setLayoutParams(relparams);

        this.contentLayout.addView(this.screen);

        relparams = new RelativeLayout.LayoutParams( RelativeLayout.LayoutParams.WRAP_CONTENT, RelativeLayout.LayoutParams.WRAP_CONTENT );
        relparams.addRule(RelativeLayout.RIGHT_OF, SCREEN_ID);
        relparams.addRule(RelativeLayout.CENTER_VERTICAL);
        this.rightExpander = new ExpanderBar(this.context);
        this.rightExpander.setLayoutParams(relparams);
        this.rightExpander.init(this, ExpanderBar.DIRECTION_RIGHT);
        this.contentLayout.addView(this.rightExpander);

        relparams = new RelativeLayout.LayoutParams( RelativeLayout.LayoutParams.WRAP_CONTENT, RelativeLayout.LayoutParams.WRAP_CONTENT );
        relparams.addRule(RelativeLayout.BELOW, SCREEN_ID);
        relparams.addRule(RelativeLayout.CENTER_HORIZONTAL);
        this.bottomExpander = new ExpanderBar(this.context);
        this.bottomExpander.setLayoutParams(relparams);
        this.bottomExpander.init(this, ExpanderBar.DIRECTION_DOWN);
        this.contentLayout.addView(this.bottomExpander);

        this.setBackgroundDrawable(this.context.getResources().getDrawable(R.drawable.bg_side_popup));
        this.setWindowLayoutType();

        this.setContentView(this.contentLayout);
        this.setOutsideTouchable(false);
        this.setClippingEnabled(false);
        this.setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_ADJUST_NOTHING | WindowManager.LayoutParams.SOFT_INPUT_STATE_ALWAYS_VISIBLE);

        //this.setInputMethodMode(INPUT_METHOD_NEEDED);
    }

    public void setWindowLayoutType() {
        Method[] methods = PopupWindow.class.getMethods();
        for(Method m: methods){
            if(m.getName().equals("setWindowLayoutType")) {
                try{
                    m.invoke(this, WindowManager.LayoutParams.TYPE_SYSTEM_ALERT);
                }catch(Exception e){
                    e.printStackTrace();
                }
                break;
            }
        }
    }

    public void showPopupView(SideRelativeLayout sideRelativeLayout, int x, int y) {
        this.sideRelativeLayout = sideRelativeLayout;
        this.setAnimationStyle(R.style.SidePopupAnimation);
        this.showAtLocation(this.sideRelativeLayout, Gravity.NO_GRAVITY, x, y);
    }

    public void expandToBottom() {

    }

    public void expandToRight() {

    }

    public void adjustSize(int deltaWidth, int deltaHeight) {
        this.width += deltaWidth;
        this.height += deltaHeight;

        if (this.width < MIN_WIDTH) {
            this.width = MIN_WIDTH;
        }

        if (this.height < MIN_HEIGHT) {
            this.height = MIN_HEIGHT;
        }

        this.onFinishAdjustSize();
        this.update(this.width, this.height);
    }

    /**
     * Called when the dragging has stopped to tell the content view that it needs to resize itself.
     */
    public void onFinishAdjustSize() {
        RelativeLayout.LayoutParams params = (RelativeLayout.LayoutParams) this.screen.getLayoutParams();
        params.width = this.width - this.rightExpander.getMeasuredWidth();
        params.height = this.height - EXPANDER_BOTTOM_HEIGHT;
        this.screen.setNewLayoutParams(params);
    }

    @Override
    public void dismiss() {
        this.contentLayout.removeView(this.screen);
        super.dismiss();
    }

}
