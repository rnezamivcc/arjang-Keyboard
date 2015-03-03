package com.iknowu.animation;

import android.content.Context;
import android.os.Handler;
import android.os.Message;
import android.util.AttributeSet;
import android.util.Log;
import android.widget.FrameLayout;

import com.iknowu.IKnowUKeyboardService;

/**
 * Created by Justin on 15/07/13.
 *
 */
public class AnimatedFrameLayout extends FrameLayout {

    private static final int LOOP_TICK = 25; //time in millis for each loop

    private static final int MSG_LOOP_TICK = 0;

    private long lastTick;

    private Handler handler = new Handler() {

        @Override
        public void handleMessage(Message msg) {

            switch (msg.what) {
                case MSG_LOOP_TICK:
                    onUpdate();
                    break;
            }
        }
    };

    public AnimatedFrameLayout(Context ctx) {
        super(ctx);
    }

    public AnimatedFrameLayout(Context context, AttributeSet attribs) {
        super(context, attribs);
    }

    public void init(Animation anim) {
        for (int i=0; i < this.getChildCount(); i++) {
            if (this.getChildAt(i) instanceof AnimatedImageView) {
                AnimatedImageView child = (AnimatedImageView) this.getChildAt(i);
                if (child != null) {
                    child.setAnimation(anim);
                }
            }
        }
    }

    public void start() {
        this.lastTick = System.currentTimeMillis();
        Message msg = handler.obtainMessage(MSG_LOOP_TICK);
        this.handler.sendMessageDelayed(msg, LOOP_TICK);
    }

    public void stop() {
        this.handler.removeMessages(MSG_LOOP_TICK);
    }

    private void onUpdate() {

        long elapsed = System.currentTimeMillis() - this.lastTick;

        IKnowUKeyboardService.log(Log.VERBOSE, "AFL onUpdate", "timeElapsed = "+elapsed);

        boolean shouldStop = false;

        for (int i=0; i < this.getChildCount(); i++) {
            if (this.getChildAt(i) instanceof AnimatedImageView) {
                AnimatedImageView child = (AnimatedImageView) this.getChildAt(i);
                if (child != null) {
                    child.onUpdate(elapsed);

                    if (child.getViewAnimation().getCurrentStepIndex() > child.getViewAnimation().getSteps().size()) shouldStop = true;
                }
            }
        }

        if (shouldStop) this.stop();
        else this.start();
    }
}
