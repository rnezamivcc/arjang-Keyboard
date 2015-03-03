package com.iknowu.animation;

/**
 * Created by Justin on 17/07/13.
 *
 * An AnimationStep is a class that holds a certain type of animation that will be performed on
 * an @link{AnimatedImageView}. It is only a step in the whole process of a complete @link{Animation},
 * which can consist of many AnimationSteps
 */
public class AnimationStep {

    public static final int TYPE_TRANSLATE = 0;
    public static final int TYPE_SCALE = 1;

    public int type;
    public long frameDuration;
    public long totalDuration;
    public int currentFrame;
    public int totalFrames;

    public int translateStartX;
    public int translateStopX;
    public int translateStartY;
    public int translateStopY;

    public float scaleFromX;
    public float scaleToX;
    public float scaleFromY;
    public float scaleToY;

    public AnimationStep(long frameDur, long totalDur) {
        this.frameDuration = frameDur;
        this.totalDuration = totalDur;
        this.currentFrame = 0;
        this.totalFrames = (int) (totalDur / frameDur);
    }

    public void setTranslation(int startx, int starty, int stopx, int stopy) {
        this.type = TYPE_TRANSLATE;

        this.translateStartX = startx;
        this.translateStartY = starty;
        this.translateStopX = stopx;
        this.translateStopY = stopy;
    }

    public void setScale(float fromx, float fromy, float tox, float toy) {
        this.type = TYPE_SCALE;

        this.scaleFromX = fromx;
        this.scaleFromY = fromy;
        this.scaleToX = tox;
        this.scaleToY = toy;
    }
}
