package com.iknowu.preferences;

import android.content.Context;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.widget.RadioButton;

/**
 * This class is needed in order to give proper behaviour from the list. Without this
 * class overriding the ontouchevent method you could have multiple radio buttons selected
 * at the same time, by returning false, we always let the parent view handle the touchevent
 * 
 * @author Justin
 *
 */
public class ImageListRadioButton extends RadioButton {

	public ImageListRadioButton(Context context, AttributeSet attrs) {
		super(context, attrs);
	}
	
	//do nothing when this button gets touched, let the container view handle this
	@Override
	public boolean onTouchEvent(MotionEvent event) {
		return false;
	}
}
