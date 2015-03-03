package com.iknowu.miniapp;

import android.content.Context;
import android.content.res.Resources.NotFoundException;
import android.content.res.XmlResourceParser;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Path;
import android.graphics.Point;
import android.graphics.Rect;
import android.util.AttributeSet;
import android.util.Log;
import android.view.Gravity;
import android.view.MotionEvent;
import android.view.View;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.widget.LinearLayout;

import com.iknowu.IKnowUKeyboardService;
import com.iknowu.KeyboardLinearLayout;
import com.iknowu.R;
import com.iknowu.util.Theme;
import com.iknowu.scroll.IKnowUScroller;

public class MiniAppActionBar extends LinearLayout {
	
	private static final String TAG = "MiniAppActionBar";
	private static final String TAG_THEME_ITEM = "item";
	
	public int childDimens = 30;
	
	private Context context;
	private MiniAppScreen miniScreen;
    private KeyboardLinearLayout keyboardLinearLayout;
	
	private int[] parentPos;
	private boolean dragStarted;
	private int initialState;

    private int topColor;
    private int bottomColor;
	
	private IKnowUScroller ikScroller;

    private int prevHighlightedChildIndex;

    private boolean mUseMarker;
    private Paint mPaint;
    private Rect mDrawingRect;
    private Path mPath;
    private Marker mMarker;
    private int mMarkerHeight = 10;
    private Point mPointA;
    private Point mPointB;
    private Point mPointC;

    class Marker {
        int x = -1;
        int y = -1;
    }
	
	public MiniAppActionBar(Context context) {
		super(context);
		this.context = context;
	}
	
	public MiniAppActionBar(Context context, AttributeSet attrs) {
		super(context, attrs);
		this.context = context;
	}
	
	/*
	public MiniAppActionBar(Context context, AttributeSet attrs, int defStyle) {
		super(context, attrs, defStyle);
		this.context = context;
	}
	*/
	
	public void init(int themeId, int height, int childHeight, boolean useMarker) {
		this.parentPos = new int[2];
		
		this.childDimens = childHeight;

        this.mUseMarker = useMarker;

        if (this.mUseMarker) {
            height += mMarkerHeight;
        }
		LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, height);
		this.setLayoutParams(params);
		this.setPadding(15, 2, 15, 2);
		
		//this.setTheme(themeId);
        this.setBackgroundColor(Theme.BACKGROUND_COLOR);

        if (this.mUseMarker) {
            //this.topColor = this.bottomColor = 0xFF82C33C;
            this.topColor = this.bottomColor = 0xFF2C86C7;
        }
		/*GradientDrawable gd = new GradientDrawable(GradientDrawable.Orientation.TOP_BOTTOM, new int[] {this.topColor,this.bottomColor});
	    gd.setCornerRadius(0f);*/
		
		//Drawable bg = this.context.getResources().getDrawable(R.drawable.sliding_bar);
		//this.setBackgroundDrawable(gd);
		this.setGravity(Gravity.CENTER_VERTICAL);
		this.setOrientation(LinearLayout.HORIZONTAL);

        mPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
        mPaint.setStyle(Paint.Style.FILL);

        mDrawingRect = new Rect();
        mPath = new Path();
        mMarker = new Marker();
        mPointA = new Point();
        mPointB = new Point();
        mPointC = new Point();
	}
	
	public void setTheme(int themeId) {
		XmlResourceParser parser;
		try {
			parser = this.context.getResources().getXml(themeId);
		} catch (NotFoundException nfe) {
			parser = this.context.getResources().getXml(IKnowUKeyboardService.DEFAULT_KEYBOARD_THEME_ID);
		}

        //IKnowUKeyboardService.log(Log.VERBOSE, "MiniAppActionBar.setTheme()", "themeId = "+themeId);

        try {
            int event;
            while ((event = parser.next()) != XmlResourceParser.END_DOCUMENT) {
                if (event == XmlResourceParser.START_TAG) {
                    String tag = parser.getName();
                    if (TAG_THEME_ITEM.equals(tag)) {
                        String attrName = parser.getAttributeValue(null, "name");
                        
                        if (attrName.equals("backgroundColor")) {
                            this.setBackgroundColor( parser.getAttributeIntValue(null, "value", 0xFF4f4f4f) );
                        	this.bottomColor = parser.getAttributeIntValue(null, "value", 0xFF4f4f4f);
                        	this.topColor = parser.getAttributeIntValue(null, "value", 0xFFd3d3d3);
                        } else if (attrName.equals("backgroundColor")) {
                            this.setBackgroundColor( parser.getAttributeIntValue(null, "value", 0xFF4f4f4f) );
                        	this.topColor = parser.getAttributeIntValue(null, "value", 0xFFd3d3d3);
                        }
                   }
                } else if (event == XmlResourceParser.END_TAG) {}
            }

            this.invalidate();
        } catch (Exception e) {
        	IKnowUKeyboardService.sendErrorMessage(e);
        }
	}

    public void highlightItem(int index) {
        this.prevHighlightedChildIndex = index;
        View child = this.getChildAt(this.prevHighlightedChildIndex);
        IKnowUKeyboardService.log(Log.VERBOSE, "MiniActionBar.highlightItem()", "index = "+this.prevHighlightedChildIndex+", child = "+child);
        if (child != null) {
            child.setBackgroundColor(0xFF2259a6);
            this.mMarker.x = child.getLeft();
            this.mMarker.y = child.getRight();
            this.invalidate();
        }
    }

    public void unhighlightItem() {
        View child = this.getChildAt(this.prevHighlightedChildIndex);
        if (child != null) {
            child.setBackgroundColor(0xffffffff);
            clearMarker();
        }
    }

    public void setListenForSpacePush() {
        this.setOnClickListener( new OnClickListener() {
            @Override
            public void onClick(View v) {
                clicked();
            }
        });
    }

    public void clicked() {
        this.keyboardLinearLayout.getKeyboardView().onReleaseKey(
                this.keyboardLinearLayout.getKeyboardView().getKeyboard().getKeyFromPrimaryCode(32)
        );
    }
	
	/*public void highlightBG() {
		GradientDrawable gd = new GradientDrawable(GradientDrawable.Orientation.TOP_BOTTOM, new int[] {this.topColor, 0xAA33B5E5});
	    gd.setCornerRadius(0f);
		this.setBackgroundDrawable(gd);
	}
	
	public void unHighlightBG() {
		GradientDrawable gd = new GradientDrawable(GradientDrawable.Orientation.TOP_BOTTOM, new int[] {this.topColor,this.bottomColor});
	    gd.setCornerRadius(0f);
		this.setBackgroundDrawable(gd);
	}*/
	
	public void setIKnowUScroller(IKnowUScroller scroll) {
		this.ikScroller = scroll;
	}

    public void startAnimation() {
        Animation anim = AnimationUtils.loadAnimation(this.context, R.anim.mini_app_anim);
        this.startAnimation(anim);
    }

    public void clearMarker() {
        this.mMarker.x = -1;
        this.mMarker.y = -1;
        this.invalidate();
    }

    @Override
    public void removeAllViews() {
        super.removeAllViews();
        clearMarker();
    }

    /**
     * Provides custom drawing.
     * @param canvas the canvas on which the background will be drawn
     */
    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);

        if (this.mUseMarker) {
            final int restoreCount = canvas.save();

            getDrawingRect(mDrawingRect);

            mDrawingRect.top = mDrawingRect.bottom - 10;
            mPaint.setColor(Color.WHITE);
            canvas.drawRect(mDrawingRect, mPaint);

            if (mMarker.x != -1) {
                this.drawMarker(canvas, mMarker.x, mDrawingRect.top, (mMarker.y - mMarker.x));
            }

            canvas.restoreToCount(restoreCount);
        }
    }
	
	@Override
	public boolean onTouchEvent(MotionEvent event) {
		//this.ikScroller.setPagingEnabled(true);
		/*
		int action = event.getAction();
		
		int x = (int) (event.getX() - this.getPaddingLeft());
		int y = (int) (event.getY() - this.getPaddingTop());
		
		IKnowUKeyboardService.log(Log.VERBOSE, TAG, "onTouchEvent action = "+event.getAction()+", x = "+x+", y = "+y);
		switch (action) {
			case MotionEvent.ACTION_DOWN:
				//this.dragStarted = true;
				//this.initialState = this.miniScreen.getState();
				//this.miniScreen.setState(MiniAppScreen.STATE_ANIMATING);
			break;
			case MotionEvent.ACTION_MOVE:
				//if(this.dragStarted) {
				//	this.parent.getLocationOnScreen(this.parentPos);
				//	int ypos = (int) event.getRawY() - this.parentPos[1];
				//	Log.d(TAG, "dragging action bar, y = "+ypos);
				//	this.miniScreen.setPositionManual(ypos);
				//}
			break;
			case MotionEvent.ACTION_UP:
				for (int i = 0; i < this.getChildCount(); i++) {
					Rect rect = new Rect();
					View child = this.getChildAt(i);
					if (child.getLocalVisibleRect(rect) && rect.contains(x, y)) {
						this.miniScreen.getMiniAppManager().startMiniApp(child.getId());
						return true;
					}
				}
				
				//did not touch in a child close mini-app drawer
				if (this.miniScreen.getState() == MiniAppScreen.STATE_OPEN) {
					this.miniScreen.setState(MiniAppScreen.STATE_CLOSED);
				}
			break;
		}
		
		return true;
		*/
		return super.onTouchEvent(event);
	}
	
	public void setScreen(MiniAppScreen screen) {
		this.miniScreen = screen;
	}

    public void setKeyboardLinearLayout(KeyboardLinearLayout linLay) {
        this.keyboardLinearLayout = linLay;
    }

    /**
     * Draw the marker.
     */
    private void drawMarker(Canvas canvas, int x, int y, int w) {
        mPointA.set(x + w/2 - mMarkerHeight, y);
        mPointB.set(x + w/2, y + mMarkerHeight);
        mPointC.set(x + w/2 + mMarkerHeight, y);

        mPath.reset();
        mPath.moveTo(mPointB.x, mPointB.y);
        mPath.lineTo(mPointC.x, mPointC.y);
        mPath.lineTo(mPointA.x, mPointA.y);
        mPath.close();

        mPaint.setColor(this.bottomColor);
        canvas.drawPath(mPath, mPaint);
    }
}
