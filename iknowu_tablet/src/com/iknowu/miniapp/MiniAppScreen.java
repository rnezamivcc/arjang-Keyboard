package com.iknowu.miniapp;

import android.content.Context;
import android.content.SharedPreferences;
import android.graphics.Paint;
import android.preference.PreferenceManager;
import android.util.AttributeSet;
import android.util.Log;
import android.view.Gravity;
import android.view.View;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.widget.FrameLayout;
import android.widget.FeedbackHorizontalScrollView;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;

import com.iknowu.IKnowUKeyboardService;
import com.iknowu.KeyboardContainerView;
import com.iknowu.R;
import com.iknowu.sidelayout.SideRelativeLayout;
import com.iknowu.sidelayout.SideScreen;
import com.iknowu.sidelayout.SideTab;
import com.iknowu.util.Theme;

public class MiniAppScreen extends SideScreen {
	
	private static final String TAG = "MiniAppScreen";
    private static final String TAG_THEME_ITEM = "item";
	
	public static final int STATE_CLOSED = 0;
	public static final int STATE_OPEN = 1;
	public static final int STATE_ANIMATING = 2;
	
	public static final int ANIM_NONE = -1;

	private MiniAppActionBar actionBar;
    private LinearLayout containerView;
	private LinearLayout contents;
	private MiniAppScrollView sview;
	
	private MiniAppManager mappManager;
	private int state;
	
	private boolean miniTakingTouchEvents;
	
	private long animationStart;

    private Paint paint;
	
	public MiniAppScreen(Context context) {
		super(context);
		//this.init(context);
        this.paint = new Paint();
	}
	
	public MiniAppScreen(Context context, AttributeSet attrs) {
		super(context, attrs);
		//this.init(context);
        this.paint = new Paint();
	}

    /*@Override
    public void onDraw(Canvas canvas) {
        super.onDraw(canvas);

        this.paint.setColor(Color.RED);
        this.paint.setStyle(Paint.Style.STROKE);
        this.paint.setStrokeWidth(3);

        canvas.drawRect(0, 0, this.getWidth(), this.getHeight(), paint);
    }*/
	
	public void init(IKnowUKeyboardService serv, KeyboardContainerView kbcv, SideRelativeLayout sfl, boolean isLefty, int index) {

        super.init(serv, kbcv, sfl, isLefty, index);

        this.tab = new SideTab(this.context);
        this.tab.init(this);
        this.tab.setImageResource(this.isLefty ? R.drawable.tab_reach_lefty : R.drawable.tab_reach);
        LayoutParams params = new LayoutParams(LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT);
        this.tab.setLayoutParams(params);

        //this.setTabClickListeners();

        this.containerView = new LinearLayout(this.context);
		this.containerView.setOrientation(LinearLayout.VERTICAL);
        //this.containerView.setBackgroundColor(Theme.SIDE_BACKGROUND_COLOR);
        this.containerView.setBackgroundColor(0xFFffffff);
        LinearLayout.LayoutParams lParams = new LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.MATCH_PARENT);
        this.containerView.setLayoutParams(lParams);

		this.actionBar = new MiniAppActionBar(this.context);
		int actionHeight  = this.context.getResources().getDimensionPixelSize(R.dimen.action_bar_height);
		int childHeight = this.context.getResources().getDimensionPixelSize(R.dimen.action_bar_child_height);
		this.actionBar.init(0,actionHeight, childHeight, true);
		this.actionBar.setScreen(this);
		//this.actionBar.setIKnowUScroller(this.ikScroller);
		//KeyboardLayoutContainer parent = (KeyboardLayoutContainer) this.getParent();
		//this.actionBar.setParent(parent);

        // Wrap the mini action bar with a horizontal scroll view.
        FeedbackHorizontalScrollView hsView = new FeedbackHorizontalScrollView(this.context);
        FrameLayout.LayoutParams fParams = new FrameLayout.LayoutParams(FrameLayout.LayoutParams.MATCH_PARENT, FrameLayout.LayoutParams.WRAP_CONTENT);
        hsView.setLayoutParams(fParams);
        //hsView.setBackgroundColor(Theme.BACKGROUND_COLOR);
        hsView.setBackgroundColor(0xFFffffff);
        hsView.setFillViewport(true);
        hsView.setHorizontalScrollBarEnabled(false);
        hsView.addView(this.actionBar);
        this.containerView.addView(hsView);

		this.state = STATE_CLOSED;
		//this.state = STATE_OPEN;
		
		this.sview = new MiniAppScrollView(this.context);
		//this.sview.setIKnowUScroller(this.ikScroller);

		/*GradientDrawable gd = new GradientDrawable(GradientDrawable.Orientation.TOP_BOTTOM, new int[] {0xFF2f2f2f,0xFF7f7f7f});
	    gd.setCornerRadius(0f);
	    this.setBackgroundDrawable(gd);*/
		//this.sview.setBackgroundColor(0xFF8c8c8c);
		
		this.contents = new LinearLayout(this.context);
		this.contents.setOrientation(LinearLayout.VERTICAL);
		this.contents.setGravity(Gravity.CENTER);
		FrameLayout.LayoutParams fparams = new FrameLayout.LayoutParams(FrameLayout.LayoutParams.MATCH_PARENT, FrameLayout.LayoutParams.MATCH_PARENT);
		this.contents.setLayoutParams(fparams);
		
		lParams = new LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.MATCH_PARENT);
		this.sview.addView(this.contents);
		this.sview.setFillViewport(true);
		this.sview.setLayoutParams(lParams);
		this.containerView.addView(this.sview);

        if (this.isLefty) {
            this.initLefty();
        } else {
            this.initRighty();
        }

        this.contentView = this.containerView;

        disableEnableScreen(this.isSelected);
    }

    @Override
    public void disableEnableScreen(boolean enable) {
        super.disableEnableScreen(enable);
        this.containerView.setEnabled(enable);
        disableEnableChildControls(enable, this.containerView);
        this.containerView.setVisibility((enable) ? View.VISIBLE : View.INVISIBLE);
    }

    /**
     * Handle open/close state of mini-app screen.
     * @param isOpen the state flag
     */
    @Override
    public void setOpenState(boolean isOpen) {
        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(this.context);
        SharedPreferences.Editor edit = sp.edit();
        edit.putString(SideRelativeLayout.OPEN_SIDE_SCREEN, (isOpen) ? SideRelativeLayout.MINI_APP_SCREEN : "");
        edit.commit();
    }

    private void initRighty() {
        this.addView(this.tab);
        this.addView(this.containerView);
    }

    private void initLefty() {
        this.addView(this.containerView);
        this.addView(this.tab);
    }
	
	public void setMiniAppManager(MiniAppManager manager) {
		this.mappManager = manager;
	}
	
	public MiniAppManager getMiniAppManager() {
		return this.mappManager;
	}
	
	@Override
	protected void onLayout (boolean changed, int l, int t, int r, int b) {
		//if (changed) {
		
		//int newt = this.setPositionAuto();
		int newt = 0;
		int newb = newt + this.actionBar.getMeasuredHeight();
		//}
		IKnowUKeyboardService.log(Log.VERBOSE, TAG, "!!!!!!!! ON LAYOUT TOP = "+t+", bottom = "+b+", changed = "+changed+" !!!!!!!!!!");
		//super.onLayout(changed, l, newt, r, newb);
		
		super.onLayout(changed, l, t, r, b);
	}
	
	/*
	 public boolean onTouchEvent(MotionEvent event) {
		IKnowUKeyboardService.log(Log.VERBOSE, TAG, "onTouchEvent action = "+event.getAction()+", x = "+event.getX()+", y = "+event.getY());
		
		int x = (int) (event.getX() - this.getPaddingLeft());
		int y = (int) (event.getY() - this.actionBar.getHeight() - this.getPaddingTop());
		
		boolean handled = false;
		
		//any touch on the action bar gets first crack at the event, this is because it controls which app is in view
		//and also when the drawer closes and opens
		Rect rect = new Rect();
		if (this.actionBar.getLocalVisibleRect(rect) && rect.contains((int)event.getX(), (int)event.getY())) {
			IKnowUKeyboardService.log(Log.VERBOSE, TAG, "rect.left = "+rect.left+", rect.top = "+rect.top+", rect.right = "+rect.right+", rect.bottom = "+rect.bottom);
			handled = this.actionBar.onTouchEvent(event);
			return true;
		}
		
		//mini apps get the chance to take over the touch event if they return true, otherwise the user, is likely trying to scroll the view
		if (!handled && this.state == STATE_OPEN) {
			handled = this.mappManager.onClick(event.getAction(), x, y);
			this.miniTakingTouchEvents = handled;
		}
		
		if (!handled) {
			this.sview.onTouchEvent(event);
		}
		
		return true;
	}
	
	/*
	@Override
	public boolean onInterceptTouchEvent(MotionEvent event) {
		IKnowUKeyboardService.log(Log.VERBOSE, TAG, "onInterceptTouchEvent action = "+event.getAction());
		if (event.getAction() == MotionEvent.ACTION_DOWN) {
			return true;
		}
		return false;
	}
	
	/*
	@Override
	public void onSizeChanged(int w, int h, int oldw, int oldh) {
		super.onSizeChanged(w, h, oldw, oldh);
		//this.setPositionAuto();
	}
	*/
	
	public int setPositionAuto() {
		RelativeLayout parent = (RelativeLayout) this.getParent();
		LinearLayout kbcont = (LinearLayout) parent.getChildAt(0);
		int height = kbcont.getMeasuredHeight();
		//Log.d(TAG, "height = "+height);
		
		RelativeLayout.LayoutParams params = new RelativeLayout.LayoutParams(RelativeLayout.LayoutParams.MATCH_PARENT, RelativeLayout.LayoutParams.WRAP_CONTENT);
		
		LinearLayout.LayoutParams lparams = new LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, height);
		this.sview.setLayoutParams(lparams);
		
		switch(this.state) {
			case STATE_CLOSED:
				params.setMargins(0, height, 0, 0);
				//Log.v(TAG, "Tmar = "+params.topMargin);
				//Log.v(TAG, "Bmar = "+params.bottomMargin);
				break;
			case STATE_OPEN:
				params.setMargins(0, 0, 0, 0);
				//Log.v(TAG, "tmar = "+params.topMargin);
				break;
			case STATE_ANIMATING:
				break;
		}
		this.setLayoutParams(params);
		return height;
	}
	
	public void setPositionManual(int ypos) {
		RelativeLayout.LayoutParams params = new RelativeLayout.LayoutParams(RelativeLayout.LayoutParams.MATCH_PARENT, RelativeLayout.LayoutParams.WRAP_CONTENT);
		params.setMargins(0, ypos, 0, 0);
		this.setLayoutParams(params);
	}
	
	public void setState(int state) {
		/*
		if (this.contents.getChildCount() <= 0) {
			this.state = STATE_CLOSED;
		} else {
			this.state = state;
		}
		/*
		if (state != STATE_ANIMATING) {
			this.setPositionAuto();
		}
		*/
	}
	
	public int getState() {
		return this.state;
	}
	
	public void setContentLayout(View v, int anim) {
		if (contents.getChildCount() > 0) {
			contents.removeViewAt(0);
		}
		
		Animation inputAnim = null;
		switch (anim) {
		case 0:
			inputAnim = AnimationUtils.loadAnimation(this.context, R.anim.slide_left);
			break;
		case 1:
			inputAnim = AnimationUtils.loadAnimation(this.context, R.anim.slide_right);
			break;
		case 2:
			inputAnim = AnimationUtils.loadAnimation(this.context, R.anim.slide_up);
			break;
		case 3:
			inputAnim = AnimationUtils.loadAnimation(this.context, R.anim.slide_down);
			break;
		case 4:
			inputAnim = AnimationUtils.loadAnimation(this.context, R.anim.zoom_in);
			break;
		case 5:
			inputAnim = AnimationUtils.loadAnimation(this.context, R.anim.zoom_out);
			break;
		}
		
		this.contents.addView(v);
		this.requestLayout();
		
		//this.ikScroller.getAdapter().notifyDataSetChanged();
		
		if (inputAnim != null) {
			v.startAnimation(inputAnim);
		}
	}
	
	public LinearLayout getContentLayout() {
		return this.contents;
	}
	
	public MiniAppActionBar getActionBar() {
		return this.actionBar;
	}
}
