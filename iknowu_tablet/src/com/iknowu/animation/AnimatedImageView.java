package com.iknowu.animation;

/**
 * Created by Justin on 16/07/13.
 *
 */
public interface AnimatedImageView {
    public void onUpdate(long timeElapsed);
    public Animation getViewAnimation();
    public void setAnimation(Animation anim);
}
