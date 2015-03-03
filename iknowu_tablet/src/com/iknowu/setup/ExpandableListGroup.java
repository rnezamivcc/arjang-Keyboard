package com.iknowu.setup;

import android.content.Context;
import android.util.AttributeSet;
import android.widget.LinearLayout;

import java.util.ArrayList;

public class ExpandableListGroup extends LinearLayout {
	
	private int position;
	private ArrayList<ExpandableListItem> children;
	
	public ExpandableListGroup(Context context) {
		super(context);
	}
	
	public ExpandableListGroup(Context context, AttributeSet attrs) {
		super(context, attrs);
	}
	
	public void setPosition(int pos) {
		this.position = pos;
	}
	
	public int getPosition() {
		return this.position;
	}
	
	public ArrayList<ExpandableListItem> getItems() {
        return this.children;
    }
	
    public void setItems(ArrayList<ExpandableListItem> items) {
        this.children = items;
    }
}
