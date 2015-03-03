package com.iknowu;

import android.content.Context;
import android.view.MotionEvent;
import android.widget.HorizontalScrollView;

/**
 * A subclass of {@link HorizontalScrollView} that determines whether or not to pass any {@link MotionEvent}s
 * to it's underlying {@link SuggestionsLinearLayout}
 * 
 * @author Justin
 *
 */
public class SuggestionScrollView {
	
	private float downX;
	private Context context;
    private SuggestionsLinearLayout suggestionLin;
	
	public SuggestionScrollView(Context context) {
        this.context = context;
	}

    public void resizeComponents(int width) {
        /*LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.WRAP_CONTENT);
        this.setLayoutParams(params);*/
    }

	private void init() {
        //this.suggestionLin = new SuggestionsLinearLayout(this.context, this);
    }
	private int getHeight() {
        return this.suggestionLin.getHeight();
    }

	public boolean onTouchEvent(MotionEvent event) {
		//return super.onTouchEvent(event);
        return true;
	}

	public boolean onInterceptTouchEvent(MotionEvent event) {
		
		final int action = event.getAction();
		
		if (action == MotionEvent.ACTION_DOWN) {
			this.downX = event.getX();
		} else if (action == MotionEvent.ACTION_MOVE) {
			if ( Math.abs(event.getX() - this.downX) < 10 ) {
				return false;
			}
		}
		
		//return super.onInterceptTouchEvent(event);
        return true;
	}
}
