package com.iknowu.setup;

import android.content.Context;
import android.util.AttributeSet;
import android.widget.LinearLayout;

public class ExpandableListItem extends LinearLayout {
	
	private int position;
	private int groupPosition;
	
	public ExpandableListItem(Context context) {
		super(context);
	}
	
	public ExpandableListItem(Context context, AttributeSet attrs) {
		super(context, attrs);
	}
	
	public void setPosition(int pos) {
		this.position = pos;
	}
	
	public int getPosition() {
		return this.position;
	}
	
	public void setGroupPosition(int pos) {
		this.groupPosition = pos;
	}
	
	public int getGroupPosition() {
		return this.groupPosition;
	}
}
