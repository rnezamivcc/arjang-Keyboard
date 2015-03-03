package com.iknowu.popup;

import android.content.Context;
import android.content.res.Resources;
import android.graphics.drawable.Drawable;

import com.iknowu.R;

/**
 * A key that will be presented in a {@link PopupKeyboardView}
 * 
 * @author Justin Desjardins
 *
 */
public class  PopupKey {
	/** 
     * All the key codes (unicode or custom code) that this key could generate, zero'th 
     * being the most important.
     */
    public int[] codes;
	
	/** Label to display */
    public CharSequence label;
    
    /** Icon to display instead of a label. Icon takes precedence over a label */
    public Drawable icon;
    /** Width of the key, not including the gap */
    public int width;
    /** Height of the key, not including the gap */
    public int height;
    /** The horizontal gap before this key */
    public int gap;
    /** X coordinate of the key in the keyboard layout */
    public int x;
    /** Y coordinate of the key in the keyboard layout */
    public int y;
    /** The current pressed state of this key */
    public boolean pressed;
    /** Text to output when pressed. This can be multiple characters, like ".com" */
    public CharSequence text;
    
    public static final int PAD_SIDES = 3;
    
    public static final double DEFAULT_WIDTH_PERCENTAGE = 0.70;
    public static final double DEFAULT_VERTICLE_GAP = 0;
    
    public static final int MAX_WIDTH = 100;
    
    public PopupKey(Context context, int maxWidth) {
    	Resources r = context.getResources();
    	this.width = (int) (DEFAULT_WIDTH_PERCENTAGE * maxWidth) + (2 * PAD_SIDES);
    	if (this.width > MAX_WIDTH) this.width = MAX_WIDTH;
    	//Log.d("KEY WIDTH =", ""+this.width);
    	this.height = r.getDimensionPixelSize(R.dimen.key_height);
    	this.codes = new int[] {};
    	this.gap = 0;
    	this.x = 0;
    	this.y = 0;
    }
    
    /**
     * Calculate the width of this key
     * @param maxWidth the maximum width that it can be
     * @return the calculated width
     */
    public static int calcKeyWidth(int maxWidth) {
    	int width = (int) (DEFAULT_WIDTH_PERCENTAGE * maxWidth) + (2 * PAD_SIDES);
    	if (width > MAX_WIDTH) width = MAX_WIDTH;
    	return width;
    }
    
    /**
     * Calculate the height of this key
     * 
     * @param ctx the context to retrieve application resources from
     * @return the calculated height of the key
     */
    public static int calcKeyHeight(Context ctx) {
    	Resources r = ctx.getResources();
    	return r.getDimensionPixelSize(R.dimen.key_height);
    }
}
