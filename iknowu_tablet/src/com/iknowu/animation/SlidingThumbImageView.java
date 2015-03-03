package com.iknowu.animation;

import android.content.Context;
import android.util.AttributeSet;
import android.util.Log;
import android.widget.FrameLayout;
import android.widget.ImageView;

import com.iknowu.IKnowUKeyboardService;

/**
 * Created by Justin on 16/07/13.
 *
 */
public class SlidingThumbImageView extends ImageView implements AnimatedImageView {

    private final static long ANIM_FRAME_DURATION = 100; //time in millis
    private static final int X_DELTA = 5;  //5 pixels at a time
    private static final int Y_DELTA = 10;  //10 pixels at a time

    private long timeSinceLastFrame;

    private Animation animation;
    private boolean doneMeasure;
    private int origWidth;
    private int origHeight;
    private int origPosX;
    private int origPosY;
    /*
    private int xMax;
    private int xMin;
    private int yMax;
    private int yMin;

    private int parentWidth;
    private int parentHeight;

    public boolean movingDown;
    *
    public ArrayList<AnimationStep> steps;
    public int currentStep;

    public boolean looping;
    */

    public SlidingThumbImageView(Context context) {
        super(context);
        this.doneMeasure = false;
    }

    public SlidingThumbImageView(Context context, AttributeSet attrs) {
        super(context, attrs);
        this.doneMeasure = false;
    }

    public SlidingThumbImageView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        this.doneMeasure = false;
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        super.onMeasure(widthMeasureSpec,heightMeasureSpec);

        if (!this.doneMeasure && this.getMeasuredWidth() > 0 && this.getMeasuredHeight() > 0) {
            this.doneMeasure = true;

            this.origWidth = this.getMeasuredWidth();
            this.origHeight = this.getMeasuredHeight();

             FrameLayout.LayoutParams params = (FrameLayout.LayoutParams) this.getLayoutParams();
            this.origPosX = params.leftMargin;
            this.origPosY = params.topMargin;

            IKnowUKeyboardService.log(Log.VERBOSE, "STIV onMeasure", "width = " +this.origWidth + ", height = "+this.origHeight);
        }
    }

    public void setAnimation(Animation anim) {
        this.animation = anim;

        /*
        FrameLayout.LayoutParams params = new FrameLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT);
        params.leftMargin = startx;
        params.topMargin = starty;
        this.setLayoutParams(params);
        /*
        this.parentHeight = pHeight;
        this.parentWidth = pWidth;

        int halfx = pWidth / 2;
        int halfy = pHeight / 2;

        this.xMin = halfx - 40;
        this.xMax = pWidth - 40;

        this.yMin = 20;
        this.yMax = pHeight - 20 - this.getHeight();

        IKnowUKeyboardService.log(Log.VERBOSE, "STIV init", "pWidth = "+pWidth+", pHeight = "+pHeight+", xMin = "+xMin+", xMax = "+xMax+", yMin = "+yMin+", yMax = "+yMax);
        IKnowUKeyboardService.log(Log.VERBOSE, "STIV init", "leftMargin = "+params.leftMargin+", topMargin = "+params.topMargin);
        */
    }

    public Animation getViewAnimation() {
        return this.animation;
    }

    private void reset() {
        FrameLayout.LayoutParams params = (FrameLayout.LayoutParams) this.getLayoutParams();

        final int prevLeft = params.leftMargin;
        final int prevTop = params.topMargin;

        params.leftMargin = this.origPosX;
        params.rightMargin = params.rightMargin + (prevLeft - this.origPosX);
        params.topMargin = this.origPosY;
        params.bottomMargin = params.bottomMargin + (prevTop - this.origPosY);

        params.width = this.origWidth;
        params.height = this.origHeight;

        this.setLayoutParams(params);
    }

    public void onUpdate(long elapsed) {
        this.timeSinceLastFrame += elapsed;

        IKnowUKeyboardService.log(Log.VERBOSE, "STIV onUpdate", "timeSinceLastFrame = " + this.timeSinceLastFrame);

        AnimationStep step = this.animation.getCurrentStep();

        if (this.timeSinceLastFrame >= step.frameDuration) {
            this.timeSinceLastFrame = 0;

            step.currentFrame++;
            if (step.currentFrame > step.totalFrames) this.animation.incrementStep();
            if (this.animation.getCurrentStepIndex() >= this.animation.getSteps().size() && this.animation.isLooping()) {
                this.animation.reset();
                this.reset();
                step = this.animation.getCurrentStep();
            } else if (this.animation.getCurrentStepIndex() < this.animation.getSteps().size()) {
                step = this.animation.getCurrentStep();
            } else {
                step = null;
            }

            IKnowUKeyboardService.log(Log.VERBOSE, "STIV onUpdate", "currentStepIndex = " + this.animation.getCurrentStepIndex());

            if (step != null) {
                switch (step.type) {
                    case AnimationStep.TYPE_TRANSLATE:
                        this.doTranslate(step);
                        break;
                    case AnimationStep.TYPE_SCALE:
                        this.doScale(step);
                        break;
                }
            }
        }
    }

    private void doTranslate(AnimationStep step) {

        FrameLayout.LayoutParams params = (FrameLayout.LayoutParams) this.getLayoutParams();

        final int xdelta = (step.translateStopX - step.translateStartX) / step.totalFrames;
        final int ydelta = (step.translateStopY - step.translateStartY) / step.totalFrames;

        final int xval = xdelta * step.currentFrame;
        final int yval = ydelta * step.currentFrame;

        final int prevLeft = params.leftMargin;
        final int prevTop = params.topMargin;
        params.leftMargin = step.translateStartX + xval;
        params.rightMargin = params.rightMargin + (prevLeft - params.leftMargin);
        params.topMargin = step.translateStartY + yval;
        params.bottomMargin = params.bottomMargin + (prevTop - params.topMargin);

        //IKnowUKeyboardService.log(Log.VERBOSE, "STIV doTranslate", "left = " + params.leftMargin+", right = "+params.rightMargin+", top = "+params.topMargin+", bottom = "+params.bottomMargin);

        this.setLayoutParams(params);
    }

    private void doScale(AnimationStep step) {
        final float xdelta = (step.scaleToX - step.scaleFromX) / step.totalFrames;
        final float ydelta = (step.scaleToY - step.scaleFromY) / step.totalFrames;

        final float xScale = step.currentFrame * xdelta;
        final float yScale = step.currentFrame * ydelta;

        IKnowUKeyboardService.log(Log.VERBOSE, "STIV doScale", "xdelta = "+xdelta+", ydelta = "+ydelta+", xScale = "+xScale+", yScale = "+yScale);

        final int width = (int) (this.origWidth * (step.scaleFromX + xScale));
        final int height = (int) (this.origHeight * (step.scaleFromY + yScale));

        FrameLayout.LayoutParams params = (FrameLayout.LayoutParams) this.getLayoutParams();

        IKnowUKeyboardService.log(Log.VERBOSE, "STIV doScale", "WIDTH = " + width+", HEIGHT = "+height);

        params.width = width;
        params.height = height;

        this.setLayoutParams(params);
    }
}