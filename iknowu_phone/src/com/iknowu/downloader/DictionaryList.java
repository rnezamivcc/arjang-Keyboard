package com.iknowu.downloader;

import android.content.Context;
import android.util.AttributeSet;
import android.widget.LinearLayout;

import com.iknowu.draganddrop.DragDropLinearLayout;

/**
 * An implementation of {@link DragDropLinearLayout} to set up a few custom parameters to be used when displaying the
 * items
 * 
 * @author Justin
 *
 */
public class DictionaryList extends DragDropLinearLayout {
	
	public DictionaryList(Context context) {
		super(context);
		this.init();
	}
	
	public DictionaryList(Context context, AttributeSet attrs) {
		super(context, attrs);
		this.init();
	}
	
	private void init() {
		LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.MATCH_PARENT);
		this.setLayoutParams(params);
		this.setOrientation(LinearLayout.VERTICAL);
		this.setPadding(0, 10, 0, 10);
	}
}
