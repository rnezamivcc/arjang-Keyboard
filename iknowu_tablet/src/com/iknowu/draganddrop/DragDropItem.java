package com.iknowu.draganddrop;

import android.view.View;

import java.util.ArrayList;

/**
 * Interface to describe a graphical element that can be dragged and/or dropped
 * 
 * @author Justin Desjardins
 *
 */
public interface DragDropItem {
	
	/**
	 * Called when the item needs highlighting
	 */
	public void highlight();
	
	/**
	 * Called when the item should no longer be highlighted
	 */
	public void unHighlight();
	
	/**
	 * Determines whether the item can be dragged or not
	 * @return whether or not the item can be dragged
	 */
	public boolean isDraggable();
	
	/**
	 * Called when the item should set it's {@link LayoutParams} to incorporate its new drop position
	 */
	public void onDropLayoutParams();
	
	/**
	 * Get the maximum position that this item can be displayed at.
	 * The lower the number, the higher the position.
	 * 
	 * @return
	 */
	public int getMaxPosition();
	
	/**
	 * Get the children of this item that can and should receive {@link MotionEvent}s
	 * 
	 * @return
	 */
	public ArrayList<View> getChildren();
}
