package com.iknowu.draganddrop;

import android.content.Context;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.widget.ScrollView;

/**
 * Subclass of {@link ScrollView} that determines when to pass touch events down to the
 * {@link DragDropLinearLayout} or if it should take the Events for itself and scroll the view
 * 
 * @author Justin Desjardins
 *
 */
public class DragDropScrollView extends ScrollView {
	
	public boolean islongpressed;
	
	public DragDropScrollView(Context context) {
		super(context);
		this.init();
	}
	
	public DragDropScrollView(Context context, AttributeSet attrs) {
		super(context, attrs);
		this.init();
	}
	
	private void init() {
		
	}
	
	@Override
	public boolean onInterceptTouchEvent(MotionEvent event) {
		//Log.d("DD SCROLL VIEW", "TOUCH EVENT INTERCEPTED");
		
		final int action = event.getAction();
		
		if (action == MotionEvent.ACTION_MOVE) {
			return false;
		} else {
			return false;
		}
	}
}
