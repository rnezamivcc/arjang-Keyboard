package com.iknowu.draganddrop;

import android.content.Context;
import android.util.AttributeSet;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.widget.CheckBox;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.Toast;

import com.iknowu.IKnowUKeyboardService;

import java.util.ArrayList;

/**
 * Encapsules all the functionality required to have a list of items that are drag and droppable.
 * 
 * 
 * @author Justin
 *
 */
public class DragDropLinearLayout extends LinearLayout {
	
	private static final int DROP_INDEX_INVALID = -1;
	private static final int DROP_INDEX_SAME_POSITION = -2;
	
	public boolean dragging;
	
	private float startY;
	private float startX;
	
	private View dragee;
	private int drageeIndex;
	
	public int disabledIndex;
	
	private int lastHovered;
	
	public RelativeLayout relLayout;
	public DragDropScrollView parent;
	
	private int topHeight;
	private Context context;
	
	public DragDropLinearLayout(Context context) {
		super(context);
		this.context = context;
	}
	
	public DragDropLinearLayout(Context context, AttributeSet attribs) {
		super(context, attribs);
		this.context = context;
	}
	
	/**
	 * Set the top offset to be used when determining location of items currently being dragged
	 * 
	 * @param height
	 */
	public void setTopHeight(int height) {
		this.topHeight = height;
	}
	
	
	@Override
	public boolean onTouchEvent(MotionEvent event) {
		try {
			int action = event.getAction();
			switch (action) {
				case MotionEvent.ACTION_DOWN:
					if(this.dragging) {
						return true;
					} else {
						return super.onTouchEvent(event);
					}
				case MotionEvent.ACTION_MOVE:
					if (this.dragging) {
						//if we haven't moved the child into the relative layout then do it now.
						if (this.relLayout.getChildCount() <= 0) {
							this.removeViewAt(this.drageeIndex);
							
							CheckBox cb = new CheckBox(this.getContext());
							cb.setVisibility(View.INVISIBLE);
							this.addView(cb, this.drageeIndex);
							
							this.relLayout.addView(this.dragee);
							//this.setRelativeLayoutParams(this.getLeft(), (int)event.getY());
						}
						
						if (event.getY() > this.parent.getBottom() - 150) {
							this.parent.scrollBy(0, 10);
						} else if (event.getY() < this.getTop() + 150) {
							this.parent.scrollBy(0, -10);
						}
						
						int pos[] = new int[2];
						this.parent.getLocationOnScreen(pos);
						int ypos = (int) event.getRawY() - pos[1];
						this.setRelativeLayoutParams(this.getLeft(), (int) event.getRawY());
						
						int currentHovered = this.getViewIndex(event.getRawX(), event.getRawY(), true);
						
						if (currentHovered != this.lastHovered) {
							if (this.lastHovered != DROP_INDEX_INVALID && this.lastHovered != this.drageeIndex) {
								DragDropItem dcb = (DragDropItem) this.getChildAt(this.lastHovered);
								dcb.unHighlight();
							}
							
							if (currentHovered != DROP_INDEX_INVALID && currentHovered != this.drageeIndex /*&& currentHovered != this.disabledIndex*/) {
								DragDropItem drag = (DragDropItem) this.dragee;
								DragDropItem dcb = (DragDropItem) this.getChildAt(currentHovered);
								//IKnowUKeyboardService.log(Log.DEBUG, "DD Action Up", "lastHovered = "+currentHovered+", dragMax = "+drag.getMaxPosition()+", hoverMax = "+dcb.getMaxPosition());
								if ( dcb.isDraggable() && currentHovered >= drag.getMaxPosition()-1 && this.drageeIndex >= dcb.getMaxPosition()-1 ) {
									dcb.highlight();
								} else {
									currentHovered = DROP_INDEX_INVALID;
								}
							}
							
							this.lastHovered = currentHovered;
						}
						
						return true;
					} else {
						return super.onTouchEvent(event);
					}
				case MotionEvent.ACTION_UP:
					if (this.dragging) {
						int newIndex = this.getViewIndex(event.getRawX(), event.getRawY(), true);
						//IKnowUKeyboardService.log("NEW INDEX =", ""+newIndex);
						if (this.lastHovered != this.drageeIndex && this.lastHovered != DROP_INDEX_INVALID) {
							DragDropItem drag = (DragDropItem) this.dragee;
							DragDropItem dcb = (DragDropItem) this.getChildAt(this.lastHovered);
							//IKnowUKeyboardService.log(Log.DEBUG, "DD Action Up", "lastHovered = "+lastHovered+", dragMax = "+drag.getMaxPosition()+", hoverMax = "+dcb.getMaxPosition());
							if ( dcb.isDraggable() && lastHovered >= drag.getMaxPosition()-1 && this.drageeIndex >= dcb.getMaxPosition()-1 ) {
								dcb.unHighlight();
							} else {
								newIndex = DROP_INDEX_INVALID;
							}
						}
						
						if (lastHovered < 0) {
							newIndex = DROP_INDEX_SAME_POSITION;
						}
						
						if (this.relLayout.getChildCount() > 0) {
							this.relLayout.removeViewAt(0);
							this.removeViewAt(this.drageeIndex);
						
							if (newIndex < 0) {
								this.addView(this.dragee, this.drageeIndex);
								DragDropItem dcb = (DragDropItem) this.dragee;
								dcb.onDropLayoutParams();
								if (newIndex == DROP_INDEX_INVALID) {
									Toast.makeText(context, "Invalid Position", Toast.LENGTH_SHORT).show();
								}
							} else {
								DragDropItem dcb;
								if (newIndex == 1) {
									dcb = (DragDropItem) this.getChildAt(newIndex);
									//dcb.notTop();
									dcb = (DragDropItem) this.dragee;
									//dcb.setAsTop();
								} else if (this.drageeIndex == 1) {
									dcb = (DragDropItem) this.dragee;
									//dcb.notTop();
									dcb = (DragDropItem) this.getChildAt(1);
									//dcb.setAsTop();
								}
								//get the current child at position, to be swapped into the old position
								int swapIndex = this.drageeIndex < newIndex ? newIndex - 1 : newIndex;
								int swapPos = this.drageeIndex < newIndex ? this.drageeIndex : this.drageeIndex - 1;
								//int swapIndex = newIndex - 1;
								IKnowUKeyboardService.log(Log.DEBUG, "DD Action Up", "dragIndex = "+this.drageeIndex+", newIndex = "+newIndex+", swapIndex = "+swapIndex);
								DragDropItem swap = (DragDropItem) this.getChildAt(swapIndex);
								this.removeViewAt(swapIndex);
								this.addView((View) swap, swapPos);
								
								this.addView(this.dragee, newIndex);
								dcb = (DragDropItem) this.dragee;
								dcb.onDropLayoutParams();
							}
						}
						this.dragee = null;
						this.dragging = false;
						return true;
					} else {
						return super.onTouchEvent(event);
					}
				default:
					return super.onTouchEvent(event);
			}
		} catch (Exception e) {
			IKnowUKeyboardService.sendErrorMessage(e);
			return super.onTouchEvent(event);
		}
	}
	
	@Override
	public boolean onInterceptTouchEvent(MotionEvent event) {
		try {
			int action = event.getAction();
			//Log.d("ACTION =", ""+action);
			switch (action) {
				case MotionEvent.ACTION_DOWN:
					this.startX = event.getRawX();
					this.startY = event.getRawY();
					
					this.drageeIndex = this.getViewIndex(startX, startY, false);
					this.dragee = this.getChildAt(this.drageeIndex);
					
					IKnowUKeyboardService.log(Log.VERBOSE, "DragDropLin", "drageeindex = "+this.drageeIndex);
					
					this.lastHovered = this.drageeIndex;
					
					if (this.dragee != null) {
						DragDropItem dcb = (DragDropItem) this.dragee;
						
						//if we can't move this one, reset!
						//otherwise return true and start taking over all touch events
						if (!dcb.isDraggable()) {
							this.dragee = null;
							this.drageeIndex = -1;
						} else {
							this.dragging = true;
							//this.dragee.setVisibility(View.INVISIBLE);
							return true;
						}
					}
					return false;
				case MotionEvent.ACTION_MOVE:
					//IKnowUKeyboardService.log(Log.INFO, "DragDropLin", "intercept onMove");
					//in case we get here for some reason
					if (this.dragee != null && this.dragging == true) {
						
						if (this.relLayout.getChildCount() <= 0) {
							//IKnowUKeyboardService.log(Log.INFO, "DragDropLin", "intercept onMove, setting child to rel layout");
							this.removeViewAt(this.drageeIndex);
							
							CheckBox cb = new CheckBox(this.getContext());
							cb.setVisibility(View.INVISIBLE);
							this.addView(cb, this.drageeIndex);
							
							this.relLayout.addView(this.dragee);
							this.setRelativeLayoutParams(this.getLeft(), (int)event.getRawY());
							return true;
						}
					}
					return false;
				case MotionEvent.ACTION_UP:
					return false;
				default:
					return false;
			}
		} catch (Exception e) {
			IKnowUKeyboardService.sendErrorMessage(e);
			return false;
		}
	}
	
	/**
	 * Set the layout parameters of the current item being dragged.
	 * 
	 * This essentially moves the view being dragged around the screen
	 * 
	 * @param x the x-coord for the dragged view
	 * @param y the y-coord for the dragged view
	 */
	private void setRelativeLayoutParams(int x, int y) {
		try {
			if (this.dragee != null) {
				y = y - (this.dragee.getHeight()/2) - this.topHeight;
			}
			RelativeLayout.LayoutParams params = new RelativeLayout.LayoutParams(RelativeLayout.LayoutParams.WRAP_CONTENT, RelativeLayout.LayoutParams.WRAP_CONTENT);
			params.setMargins(x, y, 0,0);
			this.relLayout.setLayoutParams(params);
		} catch (Exception e) {
			IKnowUKeyboardService.sendErrorMessage(e);
		}
	}
	
	/**
	 * Look for a child view at the specified coordinates
	 * @param x the x-position of the touch event
	 * @param y the y-position of the touch event
	 * @param topLevel whether or not to look deep into the childs hierarchy for hit detection
	 * @return the index of the detected view, or -1 if no detection
	 */
	private int getViewIndex(float x, float y, boolean topLevel) {
		try {
			int index = DROP_INDEX_INVALID;
			for (int i = 0; i < this.getChildCount(); i++) {
				if ( i != this.drageeIndex /* && i != this.disabledIndex */ ) {
					View lin = (View) this.getChildAt(i);
					DragDropItem ddi = (DragDropItem) lin;
					//View tview = lin.getChildAt(0);
					//RectF rect = new RectF(tview.getLeft(), tview.getTop()-25, tview.getRight(), tview.getBottom()+25);
					int[] ppos = new int[2];
					lin.getLocationOnScreen(ppos);
					
					int x2 = (int) (x);
					int y2 = (int) (y);
					
					//IKnowUKeyboardService.log(Log.VERBOSE, "DragDropLin", "left = "+lin.getLeft()+", top = "+lin.getTop()+", x = "+x2+", y = "+y2);
					//IKnowUKeyboardService.log(Log.INFO, "DragDropLin", "left = "+rect.left+", top = "+rect.top+", right = "+rect.right+", bottom = "+rect.bottom);
					if (ppos[0] < x2 && ppos[1] < y2 && (ppos[1]+lin.getHeight()) > y2 && (ppos[0]+lin.getWidth()) > x2) {
						//IKnowUKeyboardService.log(Log.VERBOSE, "DragDropLin", "hit detected inside container = "+i);
						//if we only need general hit detection for the overall layout
						if (topLevel) {
							index = i;
						} else {
							ArrayList<View> children = ddi.getChildren();
							int[] pos = new int[2];
							boolean hit = false;
							//IKnowUKeyboardService.log(Log.VERBOSE, "DragDropLin", "clickable child count = "+children.size());
							for (int k=0; k < children.size(); k++) {
								View child = children.get(k);
								pos = new int[2];
								
								child.getLocationOnScreen(pos);
								//IKnowUKeyboardService.log(Log.INFO, "Child", "X x Y = "+x2+" x "+y2+", posx = "+pos[0]+", posy = "+pos[1]+", width = "+child.getWidth()+", height = "+child.getHeight());
								if (pos[0] < x2 && pos[1] < y2 && (pos[1]+child.getHeight()) > y2 && (pos[0]+child.getWidth()) > x2) {
						            index = -1;
						            hit = true;
						            break;
						        }
							}
							//if a child was not hit, then the item is draggable
							if (!hit) {
								index = i;
							}
						}
						break;
					}
				}
			}
			return index;
		} catch (Exception e) {
			IKnowUKeyboardService.sendErrorMessage(e);
			return -1;
		}
	}
}