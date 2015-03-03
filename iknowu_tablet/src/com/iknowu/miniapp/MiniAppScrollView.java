package com.iknowu.miniapp;

import android.content.Context;
import android.os.Build;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.widget.ScrollView;

import com.iknowu.scroll.IKnowUScroller;

public class MiniAppScrollView extends ScrollView {
	
	private IKnowUScroller ikScroller;
	
	public MiniAppScrollView(Context context) {
		super(context);
	}
	
	public MiniAppScrollView(Context context, AttributeSet attrs) {
		super(context, attrs);
	}
	
	public void setIKnowUScroller(IKnowUScroller scroll) {
		this.ikScroller = scroll;
	}

	@Override
	public boolean onInterceptTouchEvent(MotionEvent event) {

        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.HONEYCOMB) {
            // Fixes a bug that wouldn't allow the mini-app content to scroll.
            getParent().requestDisallowInterceptTouchEvent(true);
        }
		
		final boolean ret = super.onInterceptTouchEvent(event);
		
		/*if (event.getAction() == MotionEvent.ACTION_DOWN && ret) {
			this.ikScroller.setPagingEnabled(false);
		} else if (event.getAction() == MotionEvent.ACTION_MOVE && ret) {
			this.ikScroller.setPagingEnabled(false);
		} else if (event.getAction() == MotionEvent.ACTION_UP && ret) {
			this.ikScroller.setPagingEnabled(true);
		}*/
		return ret;
	}
}
