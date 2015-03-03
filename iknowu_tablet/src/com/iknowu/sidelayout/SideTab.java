package com.iknowu.sidelayout;

import android.content.Context;
import android.os.Handler;
import android.os.Message;
import android.util.AttributeSet;
import android.util.Log;
import android.view.MotionEvent;
import android.widget.ImageView;
import android.widget.LinearLayout;

import com.iknowu.IKnowUKeyboardService;
import com.iknowu.util.Theme;

/**
 * Created by Justin on 29/10/13.
 *
 */
public class SideTab extends ImageView {

    private static final int MSG_LONGPRESS = 0;
    private static final int MOVEMENT_THRESHOLD = 10;
    private static final int SLIDE_THRESHOLD = 3;

    private Context context;
    private SideScreen parentScreen;
    private boolean inPopup;

    private float lastTouchX;
    private float lastTouchY;

    private float currentTouchX;
    private float currentTouchY;

    private int movePosX;
    private int movePosY;

    private LinearLayout.LayoutParams layoutParams;

    private boolean didReposition;

    public SideTab(Context context) {
        super(context);
        this.context = context;
    }

    public SideTab(Context context, AttributeSet attrs) {
        super(context, attrs);
        this.context = context;
    }

    public SideTab(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        this.context = context;
    }

    public void init(SideScreen screen) {
        this.parentScreen = screen;
    }

    public void setInPopup(boolean pop) {
        this.inPopup = pop;
    }

    /**
     * Handler used for long-press events and other UI related delayed tasks
     */
    Handler handler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            //Log.d("HANDLER MSG", ""+msg);
            switch (msg.what) {
                case MSG_LONGPRESS:
                    doLongPress();
                    break;
            }
        }
    };

    @Override
    public boolean onTouchEvent(MotionEvent ev) {

        boolean ret = false;

        IKnowUKeyboardService.log(Log.DEBUG, "SideTab.onTouchEvent()", "touchInPopup = "+this.inPopup+", action = "+ev.getAction());

        if (this.inPopup) {
            ret = this.touchInPopup(ev);
        } else {
            ret = this.touchRegular(ev);
        }

        return ret;
    }

    public boolean touchRegular(MotionEvent ev) {

        int action = ev.getAction();

        this.currentTouchX = ev.getRawX();
        this.currentTouchY = ev.getRawY();

        switch (action) {
            case MotionEvent.ACTION_CANCEL:
                handler.removeMessages(MSG_LONGPRESS);
                break;
            case MotionEvent.ACTION_DOWN:

                this.lastTouchX = ev.getRawX();
                this.lastTouchY = ev.getRawY();

                Message msg = handler.obtainMessage(MSG_LONGPRESS);
                handler.sendMessageDelayed(msg, Theme.LONG_PRESS_TIMEOUT);

                break;
            case MotionEvent.ACTION_MOVE:

                if ( Math.abs(this.currentTouchX - this.lastTouchX) > MOVEMENT_THRESHOLD || Math.abs(this.currentTouchY - this.lastTouchY) > MOVEMENT_THRESHOLD ) {
                    handler.removeMessages(MSG_LONGPRESS);
                }

                this.lastTouchX = ev.getRawX();
                this.lastTouchY = ev.getRawY();

                break;
            case MotionEvent.ACTION_UP:

                handler.removeMessages(MSG_LONGPRESS);
                this.parentScreen.tabClickedInKeyboard();

                break;
        }

        return true;
    }

    public boolean touchInPopup(MotionEvent ev) {

        int action = ev.getAction();

        this.currentTouchX = ev.getRawX();
        this.currentTouchY = ev.getRawY();

        switch (action) {
            case MotionEvent.ACTION_CANCEL:
                handler.removeMessages(MSG_LONGPRESS);
                this.didReposition = false;
                break;
            case MotionEvent.ACTION_DOWN:
                this.lastTouchX = ev.getRawX();
                this.lastTouchY = ev.getRawY();
                break;
            case MotionEvent.ACTION_MOVE:

                if ( Math.abs(this.currentTouchX - this.lastTouchX) > SLIDE_THRESHOLD || Math.abs(this.currentTouchY - this.lastTouchY) > SLIDE_THRESHOLD ) {
                    this.didReposition = true;

                    this.layoutParams = (LinearLayout.LayoutParams) this.getLayoutParams();

                    IKnowUKeyboardService.log(Log.VERBOSE, "SideTab.touchInPopup()", "marginTop = "+ this.layoutParams.topMargin);

                    this.movePosX = (int) ( this.currentTouchX - (this.getWidth() / 2) );
                    this.movePosY = (int) ( this.currentTouchY - (this.getHeight() / 2) ) - this.layoutParams.topMargin;

                    this.parentScreen.movePopup( this.movePosX, this.movePosY );
                }

                this.lastTouchX = ev.getRawX();
                this.lastTouchY = ev.getRawY();

                break;
            case MotionEvent.ACTION_UP:

                if (this.didReposition) {

                } else {
                    this.returnToKeyboard();
                }

                this.didReposition = false;

                break;
        }

        return true;
    }

    public void returnToKeyboard() {
        this.inPopup = false;
        this.parentScreen.tabClickedInPopup();
    }

    public void doLongPress() {
        this.inPopup = true;
        this.parentScreen.tabLongClicked();
    }
}
