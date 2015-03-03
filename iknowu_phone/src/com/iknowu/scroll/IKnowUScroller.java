package com.iknowu.scroll;

import android.content.Context;
import android.graphics.Paint;
import android.graphics.Rect;
import android.os.Parcelable;
import android.support.v4.view.DirectionalViewPager;
import android.support.v4.view.PagerAdapter;
import android.support.v4.view.ViewPager.OnPageChangeListener;
import android.util.AttributeSet;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;

import com.iknowu.IKnowUKeyboard;
import com.iknowu.IKnowUKeyboardService;
import com.iknowu.IKnowUKeyboardView;
import com.iknowu.SuggestionsLinearLayout;
//import com.iknowu.miniapp.MiniAppDrawer;
import com.iknowu.miniapp.MiniAppScreen;

import java.util.ArrayList;

public class IKnowUScroller extends DirectionalViewPager implements OnPageChangeListener {
	
	private Context context;
	private ArrayList<Screen> screens;
	private IKnowUKeyboardView inputView;
	private SuggestionsLinearLayout candidateView;
	
	private ScrollerPagerAdapter scrollAdapter;
	
	private KeyboardScreen kbScreen;

    private MiniAppScreen miniAppScreen;
	private View primary;
	private boolean enabled;
	
	private float downy;
	private float lastx;
	
	private Rect rect;
	private Paint paint;
	
	private IKnowUKeyboardService ikkbService;
	private int currentPosition;
	
	public IKnowUScroller(Context context) {
		super(context);
		this.context = context;
		//init();
	}
	
	public IKnowUScroller(Context context, AttributeSet attrs) {
		super(context, attrs);
		this.context = context;
	}
	
	public void setIKnowUKeyboardService(IKnowUKeyboardService service) {
		this.ikkbService = service;
	}
	
	public void init(IKnowUKeyboardService serv, int kbLayoutId, int themeId) {
		try {
			this.paint = new Paint();
			//this.setPageMargin(10);
			this.setOrientation(DirectionalViewPager.VERTICAL);
			
			this.setOnPageChangeListener(this);
			
			DisplayMetrics dm = new DisplayMetrics();
			WindowManager window = (WindowManager) this.context.getSystemService(Context.WINDOW_SERVICE);
			window.getDefaultDisplay().getMetrics(dm);
			int width = dm.widthPixels;
			
			this.screens = new ArrayList<Screen>();
			
			SettingsScreen setScreen = new SettingsScreen(this.context, width);
			setScreen.init(this);
			//keyboard screen gets treated a little differently than the other ones
			this.kbScreen = new KeyboardScreen(this.context, this, width);
            this.kbScreen.init(serv, themeId, kbLayoutId);
			
			this.inputView = kbScreen.getKeyboardView();
		//	this.inputView.setScroller(this);
			this.candidateView = kbScreen.getCandidateView();

            if (IKnowUKeyboardService.MINIAPP_ON) {
             //   this.miniAppScreen = new MiniAppScreen(this.context, this, width);
              //  this.miniAppScreen.init(themeId);
            }

			this.screens.add(setScreen);
			this.screens.add(kbScreen);

            if (IKnowUKeyboardService.MINIAPP_ON) {
             //   this.screens.add(miniAppScreen);
            }
			
			this.scrollAdapter = new ScrollerPagerAdapter();
			this.setAdapter(this.scrollAdapter);
			this.setCurrentItem(1);
			this.currentPosition = 1;
		} catch (Exception e) {
			IKnowUKeyboardService.sendErrorMessage(e);
		}
	}

    public void onFinishInput() {
        this.kbScreen.onFinishInput();
    }
	
	public void scrollToKeyboard() {
		this.setCurrentItem(1);
	}
	
	@Override
    public boolean onTouchEvent(MotionEvent event) {
        if (this.enabled) {
        	return super.onTouchEvent(event);
        }
        return false;
    }

    @Override
    public boolean onInterceptTouchEvent(MotionEvent event) {
    	try {
	    	//IKnowUKeyboardService.log(Log.VERBOSE, "IKnowUScroller onInterceptTouch", "is enable ? "+this.enabled);
	    	//if paging is enable check to see if we should scroll
	        if (this.enabled) {
	        	final int action = event.getAction();
	        	//record the x position of a down event
	        	if (action == MotionEvent.ACTION_DOWN) {
	    			this.downy = event.getY();
	    			this.lastx = event.getX();
	    		} else if (action == MotionEvent.ACTION_MOVE) {
	    			this.lastx = event.getX();

                    if (IKnowUKeyboardService.MINIAPP_ON) {
                        //if the keyboard is the current screen and there are no mini-apps and trying to scroll to mini-app screen, prevent it
                     //   if (this.currentPosition == 1 && this.miniAppScreen.getCurrentMiniAppCount() <= 0 && this.downy - event.getY() > 0) 
                            return false;
                      
                    }

	    			//if we havent moved more than 15 pixels then return false
	    			if ( Math.abs(event.getY() - this.downy) < 15 ) {
	    				return false;
	    			}
	    		}
	        	//IKnowUKeyboardService.log(Log.INFO, "IKnowUScroller onInterceptTouch", "returning super event");
	            return super.onInterceptTouchEvent(event);
	        }
	        return false;
    	} catch (Exception e) {
    		IKnowUKeyboardService.sendErrorMessage(e);
    		return false;
    	}
    }
 
    public void setPagingEnabled(boolean enabled) {
    	//IKnowUKeyboardService.log(Log.VERBOSE, "Scroller", "Setting pager enabled to = "+enabled);
        this.enabled = enabled;
    }
	
	@Override
	protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
		int kbHeight = 0;
		int mActionHeight = 0;
		super.onMeasure(widthMeasureSpec, heightMeasureSpec);
		if (this.inputView != null) {
			kbHeight = this.inputView.getMeasuredHeight();
		}
		
		if (this.kbScreen.getActionBar() != null) {
			mActionHeight = this.kbScreen.getActionBar().getMeasuredHeight();
		}
		//IKnowUKeyboardService.log(Log.VERBOSE, "IKnowUScroller, kb height = "+kbHeight, "candHeight = "+candHeight);
		
		final int maxHeight = kbHeight + mActionHeight;
		
		//final int curHeight = MeasureSpec.getSize(heightMeasureSpec);
		IKnowUKeyboardService.log(Log.VERBOSE, "IKnowUScroller, measured height = "+kbHeight, "kb height = "+maxHeight);
		final int newHeight = MeasureSpec.makeMeasureSpec(maxHeight, MeasureSpec.EXACTLY);
		super.onMeasure(widthMeasureSpec, newHeight);
		//super.onMeasure(widthMeasureSpec, heightMeasureSpec);
	}

    public void doConfigurationChagned() {
        this.kbScreen.doConfigurationChanged();

        this.setOnTouchListener(null);
    }
	
	public KeyboardScreen getKeyboardScreen() {
		return this.kbScreen;
	}
	
	public IKnowUKeyboardView getInputView() {
		return this.inputView;
	}
	
	public SuggestionsLinearLayout getCandidateView() {
		return this.candidateView;
	}
	
	public IKnowUKeyboard getKeyboard() {
		return this.kbScreen.getKeyboard();
	}
	
	public void setKeyboardLayout(int layout) {
		this.kbScreen.setKeyboard(layout);
	}
	
	public void setKeyboardMode(int mode) {
		this.kbScreen.setCurrentMode(mode);
	}

    //TODO:
	//public MiniAppDrawer getMiniAppDrawer() {
    //    if (IKnowUKeyboardService.MINIAPP_ON) {
	//	    return this.miniAppScreen.getMiniAppDrawer();
    //    } else {
     //       return null;
    //    }
	//}
	public void setMiniAppTheme(int themeId) {
		//this.miniAppScreen.init(themeId);
		this.kbScreen.getActionBar().setTheme(themeId);
	}

	
	public void hideKeyboard() {
		this.ikkbService.handleClose();
	}

	public ScrollerPagerAdapter getAdapter() {
		return this.scrollAdapter;
	}
	
	public class ScrollerPagerAdapter extends PagerAdapter {

		@Override
		public int getCount() {
			return screens.size();
		}

		/**
		 * Create the page for the given position. The adapter is responsible
		 * for adding the view to the container given here, although it only
		 * must ensure this is done by the time it returns from
		 * {@link #finishUpdate(android.view.ViewGroup)}.
		 * 
		 * @param collection
		 *            The containing View in which the page will be shown.
		 * @param position
		 *            The page position to be instantiated.
		 * @return Returns an Object representing the new page. This does not
		 *         need to be a View, but can be some other container of the
		 *         page.
		 */
		@Override
		public Object instantiateItem(ViewGroup collection, int position) {
			final Screen screen = screens.get(position);
			collection.addView(screen.getView(), 0);
			
			return screen.getView();
		}

		/**
		 * Remove a page for the given position. The adapter is responsible for
		 * removing the view from its container, although it only must ensure
		 * this is done by the time it returns from
		 * {@link #finishUpdate(android.view.ViewGroup)}.
		 * 
		 * @param collection
		 *            The containing View from which the page will be removed.
		 * @param position
		 *            The page position to be removed.
		 * @param view
		 *            The same object that was returned by
		 *            {@link #instantiateItem(android.view.View, int)}.
		 */
		@Override
		public void destroyItem(ViewGroup collection, int position, Object view) {
			collection.removeView((View)view);
		}
		
		
		/**
		 * Determines whether a page View is associated with a specific key
		 * object as returned by instantiateItem(ViewGroup, int). This method is
		 * required for a PagerAdapter to function properly.
		 * 
		 * @param view
		 *            Page View to check for association with object
		 * @param object
		 *            Object to check for association with view
		 * @return
		 */
		@Override
		public boolean isViewFromObject(View view, Object object) {
			return (view == object);
		}

		/**
		 * Called when the a change in the shown pages has been completed. At
		 * this point you must ensure that all of the pages have actually been
		 * added or removed from the container as appropriate.
		 * 
		 * @param arg0
		 *            The containing View which is displaying this adapter's
		 *            page views.
		 */
		@Override
		public void finishUpdate(ViewGroup arg0) {
		}

		@Override
		public void restoreState(Parcelable arg0, ClassLoader arg1) {
		}

		@Override
		public Parcelable saveState() {
			return null;
		}

		@Override
		public void startUpdate(ViewGroup arg0) {
		}
	}
	
	@Override
	public void onPageScrolled(int position, float positionOffset, int positionOffsetPixels) {
		this.currentPosition = position;
	}

	@Override
	public void onPageScrollStateChanged(int state) {
	}

	@Override
	public void onPageSelected(int position) {
	}
}
