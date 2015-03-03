package com.iknowu.sidelayout;

import android.content.Context;
import android.util.AttributeSet;
import android.util.Log;
import android.view.MotionEvent;
import android.widget.ImageView;

import com.iknowu.IKnowUKeyboardService;
import com.iknowu.R;

/**
 * Created by Justin on 01/11/13.
 *
 */
public class ExpanderBar extends ImageView {

    public static final int DIRECTION_RIGHT = 0;
    public static final int DIRECTION_DOWN = 1;

    public static final int MOVEMENT_THRESHOLD = 0;

    public SidePopup sidePopup;
    public int direction;

    public float currentTouchX;
    public float currentTouchY;
    public float lastTouchX;
    public float lastTouchY;

    public boolean didResize;

    public ExpanderBar(Context context) {
        super(context);
    }

    public ExpanderBar(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public ExpanderBar(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
    }

    public void init(SidePopup popup, int direction) {

        this.sidePopup = popup;
        this.direction = direction;

        IKnowUKeyboardService.log(Log.DEBUG, "ExpanderBar.init()", "direction = "+direction);

        switch (this.direction) {
            case DIRECTION_RIGHT:
                this.setImageResource(R.drawable.expander_right_image);
                this.setBackgroundResource(R.drawable.button_expander_right);
                break;
            case DIRECTION_DOWN:
                this.setImageResource(R.drawable.expander_bottom_image);
                this.setBackgroundResource(R.drawable.button_expander_bottom);
                break;
        }
    }

    @Override
    public boolean onTouchEvent(MotionEvent ev) {

        boolean ret = false;

        switch (this.direction) {
            case DIRECTION_RIGHT:
                ret = this.touchRight(ev);
                break;
            case DIRECTION_DOWN:
                ret = this.touchDown(ev);
                break;
        }

        return ret;
    }

    public boolean touchRight(MotionEvent ev) {

        int action = ev.getAction();

        this.currentTouchX = ev.getRawX();
        this.currentTouchY = ev.getRawY();

        switch (action) {
            case MotionEvent.ACTION_CANCEL:
                this.setPressed(false);
                break;
            case MotionEvent.ACTION_DOWN:

                this.lastTouchX = ev.getRawX();
                this.lastTouchY = ev.getRawY();

                this.setPressed(true);

                break;
            case MotionEvent.ACTION_MOVE:

                if ( Math.abs(this.currentTouchX - this.lastTouchX) > MOVEMENT_THRESHOLD ) {
                    this.didResize = true;
                    this.sidePopup.adjustSize( (int)(this.currentTouchX - this.lastTouchX), 0 );
                }

                this.lastTouchX = ev.getRawX();
                this.lastTouchY = ev.getRawY();

                break;
            case MotionEvent.ACTION_UP:
                this.setPressed(false);
                if (this.didResize) {
                    //this.sidePopup.onFinishAdjustSize();
                }
                break;
        }

        return true;
    }

    public boolean touchDown(MotionEvent ev) {
        int action = ev.getAction();

        this.currentTouchX = ev.getRawX();
        this.currentTouchY = ev.getRawY();

        switch (action) {
            case MotionEvent.ACTION_CANCEL:
                this.setPressed(false);
                break;
            case MotionEvent.ACTION_DOWN:

                this.lastTouchX = ev.getRawX();
                this.lastTouchY = ev.getRawY();

                this.setPressed(true);

                break;
            case MotionEvent.ACTION_MOVE:

                if ( Math.abs(this.currentTouchY - this.lastTouchY) > MOVEMENT_THRESHOLD ) {
                    this.didResize = true;
                    this.sidePopup.adjustSize( 0, (int)(this.currentTouchY - this.lastTouchY) );
                }

                this.lastTouchX = ev.getRawX();
                this.lastTouchY = ev.getRawY();

                break;
            case MotionEvent.ACTION_UP:
                this.setPressed(false);
                if (this.didResize) {
                    //this.sidePopup.onFinishAdjustSize();
                }
                break;
        }
        return true;
    }
}
