package com.iknowu;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.SharedPreferences;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.drawable.Drawable;
import android.os.Handler;
import android.os.Message;
import android.preference.PreferenceManager;
import android.util.DisplayMetrics;
import android.util.Log;
import android.util.TypedValue;
import android.view.MotionEvent;
import com.iknowu.util.DimensionConverter;
import com.iknowu.util.Size;
import com.iknowu.util.Theme;
import java.util.ArrayList;

/**
 * This class is responsible for taking a list of predictions/corrections and presenting them to the user
 * 
 * @author Justin
 *
 */
@SuppressLint("WrongCall")
public class SuggestionsLinearLayout {
	
	private static final int MIN_HEIGHT = 20;

	private static final int ORDER_LEFT_TO_RIGHT = 0;
	private static final int ORDER_CENTER_OUT = 1;
	private static final int ORDER_RIGHT_TO_LEFT = 2;
	
	public static final String CORRECTED_KNOWN_WORD = "ci"; //the word a user typed and we replaced, it is known to the dictionaries
	public static final String CORRECTED_NEW_WORD = "cn";	//the word a user typed and we replaced, it is not known to the dictionaries
	public static final String CORRECTION = "c";			// a possible correction for the user to insert
	public static final String CORRECTION_PRED_ROOT = "cp";
	public static final String PREDICTION = "p";			// a possible prediction for the user to insert
	public static final String PREDICTION_ROOT = "pr";		// this a is the current word that is typed, and it is the root word for the predictions
	
	private static final String TAG_THEME_ITEM = "item";
	private static final String PREF_FONT_SIZE = "predFontHeight";

    public static final int MAX_SUGGESTIONS = 5;
	
	private static final long CUSTOM_LONG_PRESS_TIMEOUT = 150;
	private static final int MSG_LONGPRESS = 43;
	
	private static final int MOVEMENT_THRESHOLD = 10;
	
	public static final float X_THRESHOLD = 0.15f;
	public static final float Y_THRESHOLD = 0.4f;
	
	public static Drawable divider;

	private final int[][] orderCenterOut = { {0}, {0,1}, {1,0,2}, {1,0,2,3}, { 1, 0, 2, 3, 4 } };
	private final int[] orderLeftToRight = { 0, 1, 2, 3, 4 };
	private final int[] orderRightToLeft = { 4, 3, 2, 1, 0 };

	private int suggestionsOrder;

	private Context context;
	
	private float lastTouchX;
	private float lastTouchY;

	private int fontSize;
	
	private SuggestionScrollView parent;
	
	private int minItemWidth;
    private int minHeight;

	private IKnowUKeyboardService keyboardService;
	private int[] candidateViewLoc = new int[2];
	
	private String mUnknownWord;
	private boolean mUnknownWordToAdd;

	private int lastSelected;
	
	private long lastMoveTime;
    public boolean seenUpEvent;

    private int xpos;
    private int ypos;
    private int scrollx;
    private int scrolly;

    private int totalWidth;
    private int screenWidth;

    private int totalPhraseWidth;
    private int phraseScrollx;
    private int phraseScrolly;

    private boolean didScroll;

    private ArrayList<Suggestion> suggestions;
    private ArrayList<Suggestion> phraseSuggestions;

    private boolean isPhraseBarShowing;

    private IKnowUKeyboardView iKnowUKeyboardView;

    private Paint paint;

	/**
	 * {@link Handler} to receive long-press actions
	 */
	Handler mHandler = new Handler() {
		@Override
		public void handleMessage(Message msg) {
			// Log.d("HANDLER MSG", ""+msg);
			switch (msg.what) {
			case MSG_LONGPRESS:
				handleLongPress(msg.arg1);
				break;
			}
		}
	};
	
	
	//========================================================================================
	//CONTRUCTORS
	//========================================================================================
	public SuggestionsLinearLayout(Context context, IKnowUKeyboardView kbview) {
		//super(context);
		this.context = context;
        this.iKnowUKeyboardView = kbview;
		this.minItemWidth = this.context.getResources().getDisplayMetrics().widthPixels / 5;

        this.paint = new Paint();
        this.init();
	}
	
	/**
	 * Main initialization method for this class. Sets up all dimensional parameters as well as establishing it's child views
	 *
	 */
	public void init() {
		try {

            this.suggestions = new ArrayList<Suggestion>();
            this.phraseSuggestions = new ArrayList<Suggestion>();

			this.lastTouchX = 0;
			this.lastTouchY = 0;
			
			this.setTheme();
			
			/*this.parent = (SuggestionScrollView) this.getParent();
			this.parent.setSmoothScrollingEnabled(true);
			this.parent.setFadingEdgeLength(40);*/

			this.suggestionsOrder = ORDER_CENTER_OUT;
			//this.suggestionsOrder = ORDER_LEFT_TO_RIGHT;
			// this.suggestionsOrder = ORDER_RIGHT_TO_LEFT;
			
			SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(this.context);
			String fontsize = sp.getString(PREF_FONT_SIZE, "medium");

			if (fontsize.equals("small")) {
				this.fontSize = 18;
			} else if (fontsize.equals("medium")) {
				this.fontSize = 19;
			} else if (fontsize.equals("large")) {
				this.fontSize = 22;
			} else if (fontsize.equals("xlarge")) {
                this.fontSize = 25;
            } else {
				this.fontSize = 22;
			}
			this.fontSize = DimensionConverter.stringToDimensionPixelSize(this.fontSize + "sp", this.context.getResources().getDisplayMetrics());
			
			this.minHeight = this.fontSize;
			int padding = DimensionConverter.stringToDimensionPixelSize(Suggestion.PADDING_TOP_BOTTOM + "sp", this.context.getResources().getDisplayMetrics());
			
			minHeight += (padding * 2);
			
			if (this.context.getResources().getDisplayMetrics().densityDpi < DisplayMetrics.DENSITY_HIGH) {
				minHeight += 5;
			}  
            IKnowUKeyboardService.log(Log.VERBOSE, "SuggestionsLinearLayout.init()", "fontSize = "+this.fontSize);
			/*this.setLayoutParams(new FrameLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, minHeight));
			this.setMinimumWidth(this.keyboardService.displayWidth);
			this.setOrientation(LinearLayout.HORIZONTAL);*/
			
			/*
			divider = this.context.getResources().getDrawable(R.drawable.vertical_divider);
			Rect rect = new Rect(0,0,1,minHeight);
			divider.setBounds(rect);
			divider.setColorFilter( this.textColor, PorterDuff.Mode.SRC_ATOP );
			*/
			
			//setHorizontalFadingEdgeEnabled(true);
			
			//this.createChildren();
			//setHorizontalScrollBarEnabled(false);
		} catch (Exception e) {
			IKnowUKeyboardService.sendErrorMessage(e);
		}
	}
	
	/**
	 * Establish the child views, for displaying the predictions and corrections
	 */
	/*private void createChildren() {
		CorrectionSuggestion tview = new CorrectionSuggestion(this.context);
		tview.init(this.keyboardService, this, this.fontSize, this.textColor, this.selectedColor, this.minItemWidth);
		tview.setVisibility(View.GONE);
		this.addView(tview);
		//IKnowUKeyboardService.log(Log.ERROR, "SuggLinLay", "Adding child 1 = "+this.getChildAt(0));
		tview = new CorrectionSuggestion(this.context);
		tview.init(this.keyboardService, this, this.fontSize, this.textColor, this.selectedColor, this.minItemWidth);
		tview.setVisibility(View.GONE);
		this.addView(tview);
		//IKnowUKeyboardService.log(Log.ERROR, "SuggLinLay", "Adding child 2 = "+this.getChildAt(1));
		tview = new CorrectionSuggestion(this.context);
		tview.init(this.keyboardService, this, this.fontSize, this.textColor, this.selectedColor, this.minItemWidth);
		tview.setVisibility(View.GONE);
		this.addView(tview);
		//IKnowUKeyboardService.log(Log.ERROR, "SuggLinLay", "Adding child 3 = "+this.getChildAt(2));
		tview = new CorrectionSuggestion(this.context);
		tview.init(this.keyboardService, this, this.fontSize, this.textColor, this.selectedColor, this.minItemWidth);
		tview.setVisibility(View.GONE);
		this.addView(tview);
		//IKnowUKeyboardService.log(Log.ERROR, "SuggLinLay", "Adding child 4 = "+this.getChildAt(3));
		tview = new CorrectionSuggestion(this.context);
		tview.init(this.keyboardService, this, this.fontSize, this.textColor, this.selectedColor, this.minItemWidth);
		tview.setVisibility(View.GONE);
		this.addView(tview);
		//IKnowUKeyboardService.log(Log.ERROR, "SuggLinLay", "Adding child 5 = "+this.getChildAt(4));
	}*/

    public void resizeComponents(int width) {
//        FrameLayout.LayoutParams params = (FrameLayout.LayoutParams) this.getLayoutParams();
//        params.height = this.minHeight;
        this.totalWidth = 0;
        this.totalPhraseWidth = 0;
        this.minItemWidth = (width-20) / 5;
        this.screenWidth = width;
        for (int i = 0; i < this.suggestions.size(); i++) {
            Suggestion sugg = this.suggestions.get(i);
            sugg.setMinWidth(minItemWidth);
            sugg.setLayoutParameters();
            this.totalWidth += sugg.getWidth();
            //this.getChildAt(i).setMinimumWidth(minWidth);
        }

        int minPhraseWidth = (width-20) / 3;

        // MD: Added to allow more phrases to fit
        if ( this.phraseSuggestions.size() > 3 ) {
            minPhraseWidth = (width) / this.phraseSuggestions.size();
        }

        for (int i = 0; i < this.phraseSuggestions.size(); i++) {
            Suggestion sugg = this.phraseSuggestions.get(i);
            sugg.setMinWidth(minPhraseWidth);
            sugg.setLayoutParameters();
            this.totalPhraseWidth += sugg.getWidth();
            //this.getChildAt(i).setMinimumWidth(minWidth);
        }
    }
	
	/**
	 * Parse the theme file to be used for displaying this layout
	 *
	 */
	public void setTheme() {

	}

    public void setPosition(int x, int y) {
        this.xpos = x;
        this.ypos = y;
        this.scrollx = 0;
        this.scrolly = 0;
    }

    /**
     * Scroll to the specified position, this will snap to the position without any smooth scrolling
     * @param x
     * @param y
     */
    public void scrollTo(int x, int y) {

        if (this.totalWidth > this.screenWidth) {
            if (x > 0) {
                x = 0;
            }
            else if ( x < (this.screenWidth - this.totalWidth) ) {
                x = this.screenWidth - this.totalWidth;
            }

            IKnowUKeyboardService.log(Log.INFO, "SuggestionLinearLayout.scrollTo()", "x = "+x+ ",screenWidth = "+this.screenWidth+", totalWidth = "+totalWidth);

            this.scrollx = x;
            this.scrolly = y;
        }
    }

    /**
     * Scroll by the specified amounts, add/subtracts the values to the current scroll values
     * @param x
     * @param y
     */
    public void scrollBy(int x, int y) {

        if (this.totalWidth > this.screenWidth) {
            this.scrollx += x;
            this.scrolly += y;

            if (this.scrollx > 0) {
                this.scrollx = 0;
            }
            else if (this.scrollx < (this.screenWidth - this.totalWidth)) {
                this.scrollx = this.screenWidth - this.totalWidth;
            }
        }
    }

    /**
     * Scrolls to center if total width of suggestions is less than screen width.
     * If greater or equal, first suggestion is aligned with left side.
     */
    public void scrollToCenter() {
        this.scrollx = Math.max(0, (this.screenWidth - this.totalWidth) / 2);
    }

    /**
     * Scroll to the specified position, this will snap to the position without any smooth scrolling
     * @param x
     * @param y
     */
    public void scrollPhraseTo(int x, int y) {

        if (this.totalPhraseWidth > this.screenWidth) {
            if (x > 0) {
                x = 0;
            }
            else if ( x < (this.screenWidth - this.totalPhraseWidth) ) {
                x = this.screenWidth - this.totalPhraseWidth;
            }

            IKnowUKeyboardService.log(Log.INFO, "SuggestionLinearLayout.scrollTo()", "x = "+x+ ",screenWidth = "+this.screenWidth+", totalWidth = "+totalPhraseWidth);

            this.phraseScrollx = x;
            this.phraseScrolly = y;
        }
    }

    /**
     * Scroll by the specified amounts, add/subtracts the values to the current scroll values
     * @param x
     * @param y
     */
    public void scrollPhraseBy(int x, int y) {

        if (this.totalPhraseWidth > this.screenWidth) {
            this.phraseScrollx += x;
            this.phraseScrolly += y;

            if (this.phraseScrollx > 0) {
                this.phraseScrollx = 0;
            }
            else if (this.phraseScrollx < (this.screenWidth - this.totalPhraseWidth)) {
                this.phraseScrollx = this.screenWidth - this.totalPhraseWidth;
            }
        }
    }

    public void scrollPhraseToCenter() {
        this.phraseScrollx = (this.screenWidth - this.totalPhraseWidth) / 2;
        IKnowUKeyboardService.log(Log.VERBOSE, "scrollPhraseToCenter()", "phraseScrollX = "+this.phraseScrollx+", screenWidth = "+this.screenWidth+", totalPhraseWidth = "+totalPhraseWidth);
    }

    public int getHeight() {
        int height = 0;

        if (IKnowUKeyboardService.PHRASE_PREDICTION_ON) {
             height = this.minHeight * 2;
        } else {
            height = this.minHeight;
        }

        /*if (this.phraseSuggestions.size() > 0) {
            height += this.phraseSuggestions.get(0).getHeight();
        }*/
        return height;
    }

    public int getMinHeight() {
        return this.minHeight;
    }

    public void onMeasure() {
        this.screenWidth = Size.getScreenWidth(this.context);
        this.scrollToCenter();
    }

    @SuppressLint("WrongCall")
	public void onDraw(Canvas canvas) {
        //this.scrollTo( ( (this.screenWidth - this.totalWidth) / 2 ), 0);
        this.paint.setColor(Theme.CANDIDATE_BACKGROUND_COLOR);
        //if (this.isPhraseBarShowing) {
            canvas.drawRect(0f, 0f, this.iKnowUKeyboardView.getWidth(), this.getHeight(), this.paint);

        this.paint.setColor(Theme.KEY_COLOR);
            canvas.drawLine(0f, 30f, this.iKnowUKeyboardView.getWidth(), 30f, this.paint);
            canvas.drawLine(0f, 10f, this.iKnowUKeyboardView.getWidth(), 10f, this.paint);

        if (IKnowUKeyboardService.PHRASE_PREDICTION_ON) {
            // draw the second line
            canvas.drawLine(0f, 75f, this.iKnowUKeyboardView.getWidth(), 75f, this.paint);
            canvas.drawLine(0f, 55f, this.iKnowUKeyboardView.getWidth(), 55f, this.paint);
        }

        this.paint.setColor(Theme.CANDIDATE_BACKGROUND_COLOR);

        //} else {
        //    canvas.drawRect(0f, this.getHeight() / 2, this.iKnowUKeyboardView.getWidth(), this.getHeight(), this.paint);
        //}

        if (this.suggestions.size() > 0) {
            int[] positions = this.orderCenterOut[this.suggestions.size()-1];

            Suggestion prev = null;
            Suggestion sugg = null;
            int x = this.scrollx;
            for (int i=0; i < this.suggestions.size(); i++) {

                int pos = positions[i];
                sugg = this.suggestions.get(pos);

                if (prev != null) {
                    x += prev.getWidth();
                }
                if (IKnowUKeyboardService.PHRASE_PREDICTION_ON) {
                    sugg.setPosition(x, this.ypos+this.minHeight);
                } else {
                    sugg.setPosition(x, this.ypos);
                }
                sugg.onDraw(canvas);
                prev = sugg;
            }
        }

        if (IKnowUKeyboardService.PHRASE_PREDICTION_ON && this.phraseSuggestions.size() > 0) {

            Suggestion prev = null;
            Suggestion sugg = null;

            // need to determine the number which will fit
            boolean complete = false;
            int maxCount = 0;
            int width = this.iKnowUKeyboardView.getWidth();
            //final float densityMultiple = getContext().getResources().getDisplayMetrics().density;
            float totalSize = 0f;
            while (!complete && maxCount < this.phraseSuggestions.size()) {
                Suggestion s = phraseSuggestions.get(maxCount);
                totalSize += s.getWidth();



                if (totalSize > width) {
                    complete = true;
                    totalSize -= s.getWidth(); // last valid width
                } else {
                    maxCount++;
                }

            }

            if (maxCount > this.orderCenterOut.length) {
                maxCount = this.orderCenterOut.length;
            }
            int[] positions = this.orderCenterOut[maxCount-1];

            //int x = this.phraseScrollx;
            int x = (int) ((this.iKnowUKeyboardView.getWidth() - totalSize) / 2);

            for (int i=0; i < maxCount; i++) {

                int pos = positions[i];
                sugg = this.phraseSuggestions.get(pos);

                if (prev != null) {
                    x += prev.getWidth();
                }

                sugg.setPosition(x, this.ypos/*+this.minHeight*/);
                sugg.onDraw(canvas);
                prev = sugg;
            }
        }
    }

	/**
	 * Handle a long-press on one of the child views
	 * 
	 * @param index the index of the child that the long-press occurred on
	 */
	private void handleLongPress(int index) {
		try {
            IKnowUKeyboardService.log(Log.VERBOSE, CorrectionSuggestion.TAG, "handleLongPress()");
			this.lastSelected = index;
			Suggestion tview = this.suggestions.get(index);
			tview.setPressed(false);
            this.sendKeyClick();
			if (tview instanceof CorrectionSuggestion) {
				if (tview.isNewWord()) {
					String text = tview.getRawSuggestion();
					if (text != null) {
						tview.setLongPressOccurred();
						//sendKeyClick();
					}
				} else if (tview.isRoot()) {
					tview.setLongPressOccurred();
					this.keyboardService.manualCorrect(tview.getRawSuggestion(), false, tview.getCapsMode());
				}
			} else {
				if (tview.isRoot()) {
					this.keyboardService.showAllPredictions();
					tview.setLongPressOccurred();
				} else {
					String pred = tview.getRawSuggestion();
					if (pred != null) {
						tview.setLongPressOccurred();
						//sendKeyClick();
						this.keyboardService.pickSuggestionManually(pred, false, tview.getCapsMode());
					}
				}
			}
		} catch (Exception e) {
			IKnowUKeyboardService.sendErrorMessage(e);
		}
	}

    public boolean isInside(int x, int y) {
        //if (this.isPhraseBarShowing) {
            if ( /*xpos < x && x < (xpos + this) &&*/ ypos < y && y < (ypos + this.getHeight()) ) return true;
        //} else {
        //    if ( /*xpos < x && x < (xpos + this) &&*/ ypos < y && y < (ypos + this.minHeight) ) return true;
        //}
        return false;
    }

    private boolean insidePhrase(float x, float y) {
        if ( /*xpos < x && x < (xpos + this) &&*/ ypos < y && y < (ypos + (this.getHeight() /2)) ) return true;

        return false;
    }

    public boolean onTouchEvent(MotionEvent ev) {

        int action = ev.getAction();

        switch (action) {
            case MotionEvent.ACTION_CANCEL:
                this.sendEventToChildren(ev);
                break;
            case MotionEvent.ACTION_DOWN:
                this.lastTouchX = ev.getX();
                this.lastTouchY = ev.getY();
                this.sendEventToChildren(ev);
                break;
            case MotionEvent.ACTION_MOVE:
                int diffx = (int)(ev.getX() - this.lastTouchX);

                if (Math.abs(diffx) > MOVEMENT_THRESHOLD) {
                    this.cancelLongPressMessage();

                    if ( IKnowUKeyboardService.PHRASE_PREDICTION_ON && this.insidePhrase(ev.getX(), ev.getY()) ) {
                        this.scrollPhraseBy( diffx, 0 );
                    } else {
                        this.scrollBy( diffx, 0 );
                    }

                    this.lastTouchX = ev.getX();
                    this.lastTouchY = ev.getY();

                    this.didScroll = true;

                    this.unHighlightChildren();
                } else if (!this.didScroll) {
                    this.sendEventToChildren(ev);
                }

                break;
            case MotionEvent.ACTION_UP:
                if (!this.didScroll) {
                    this.sendEventToChildren(ev);
                }
                this.unHighlightChildren();
                this.didScroll = false;
                break;
        }


        return true;
    }

    private void unHighlightChildren() {
        for (Suggestion sugg : this.suggestions) {
            sugg.setPressed(false);
        }

        for (Suggestion sugg : this.phraseSuggestions) {
            sugg.setPressed(false);
        }
    }

    private void sendEventToChildren(MotionEvent ev) {
        boolean handled = false;

        for (Suggestion sugg : this.suggestions) {
            if (sugg.isInside((int)ev.getX(), (int)ev.getY())) {
                sugg.onTouch(ev);
                handled = true;
                break;
            } else {
                sugg.setPressed(false);
            }
        }

        //IKnowUKeyboardService.log(Log.DEBUG, "sendEventTOChildren()", "handled = "+handled);

        if (IKnowUKeyboardService.PHRASE_PREDICTION_ON && !handled) {
            for (Suggestion sugg : this.phraseSuggestions) {
                if (sugg.isInside((int)ev.getX(), (int)ev.getY())) {
                    //IKnowUKeyboardService.log(Log.DEBUG, "sendEventTOChildren()", "isInside = true");
                    sugg.onTouch(ev);
                    break;
                } else {
                    //IKnowUKeyboardService.log(Log.DEBUG, "sendEventTOChildren()", "isInside = "+false);
                    sugg.setPressed(false);
                }
            }
        }
    }
	
	/**
	 * Custom touch event handling for when events are being passed in from the compressed view keyboard
	 * in the form of move events
	 * 
	 * @param event the event that occurred, in our case right now, always a move event
	 * @param topOfKB the top offset of the keyboardview, since the event started on that view, the y coordinates will be negative when
	 * sliding up to the suggestion view. To offset this, the suggestion view will have to add the top of keyboardview to the y values
	 * @return the success of the event
	 *
	public boolean onTouchEvent(MotionEvent event, float topOfKB) {
		
		float x = event.getX();
		final float y = event.getY() + topOfKB;
		
		//final float diffx = Math.abs(this.lastTouchX - x);
		//final float diffy = Math.abs(this.lastTouchY - y);
		//IKnowUKeyboardService.log(Log.INFO, "Action move", "x = "+diffx+", y = "+diffy);
		//if ( diffx > MOVEMENT_THRESHOLD || diffy > MOVEMENT_THRESHOLD ) {
			
			mHandler.removeMessages(MSG_LONGPRESS);
			
			event.setLocation(x, y);
			//IKnowUKeyboardService.log(Log.VERBOSE, "Action move", "x = "+x+", y = "+y);
			
			final float xdist = x - lastTouchX;
			final float ydist = y - lastTouchY;
			
			final float dtime = (float) (System.currentTimeMillis() - this.lastMoveTime);
			float xVel = (float) (xdist / dtime);
			final float yVel = (float) (ydist / dtime);
			
			if (yVel < 0 && Math.abs(yVel) > Y_THRESHOLD && Math.abs(xVel) < X_THRESHOLD ) {
				x = this.lastTouchX;
				xVel = 0;
			}
			
			//Rect rect = new Rect();
			CorrectionSuggestion tv;
			for (int i = 0; i < this.getChildCount(); i++ ) {
				tv = (CorrectionSuggestion) this.getChildAt(i);
				//tv.getLocalVisibleRect(rect);
				if (tv.getVisibility() == View.VISIBLE) {
					//if the touch is on one of the suggestion views, then trigger a long press
					//IKnowUKeyboardService.log(Log.VERBOSE, "Rect", "left = "+tv.getLeft()+", right = "+tv.getRight()+", top = "+tv.getTop()+", bottom = "+tv.getBottom());
					if ( tv.isInside(x, y) ) {
						tv.setPressed(true);
						this.addWordByVelocity(i, xVel, yVel);
						//this.sendLongPressMessage(i);
					} else {
						tv.setPressed(false);
					}
				}
			}
		//}
		
		this.lastMoveTime = System.currentTimeMillis();
		this.lastTouchX = x;
		this.lastTouchY = y;
		return true;
		//return this.onTouchEvent(event);
	}
    */
	
	/**
	 * Try to add a prediction to the text, do this by checking the x and y velocities for a distinct swipe up gesture on a TextView
	 * 
	 * @param xvel the velocity on the x-axis
	 * @param yvel the velocity on the y-axis
	 */
	private void addWordByVelocity( int index, float xvel, float yvel ) {
		//if under the x threshold and over the y threshold then probably want to add this character
		//IKnowUKeyboardService.log(Log.VERBOSE, "trying to add xvel = "+xvel, "yvel = "+yvel);
		if (Math.abs(xvel) <= X_THRESHOLD && Math.abs(yvel) >= Y_THRESHOLD && index > -1 && this.lastSelected != index) {
			//this.composingWord += key.label;
			this.lastSelected = index;
			IKnowUKeyboardService.log(Log.INFO, "add Word, xvel = "+xvel + ", yvel = "+yvel, "sending word index = "+index);
			this.handleLongPress(index);
			//this.onReleaseKey(key);
			//this.showPopupAnimation(key);
		} else if ( Math.abs(xvel) >= X_THRESHOLD && this.lastSelected == index ) {
			this.lastSelected = -1;
		}
		//this.lastVelocity = velocity;
	}

    public void showPhraseBar() {
        this.isPhraseBarShowing = true;
        //only show if not already showing
        /*if (!this.isPhraseBarShowing) {
            this.isPhraseBarShowing = true;

            this.iKnowUKeyboardView.getKeyboard().offsetKeyYPos(this.minHeight);
            this.iKnowUKeyboardView.requestLayout();
        }*/
    }

    public void hidePhraseBar() {
        this.isPhraseBarShowing = false;
        //only hide if currently showing
        /*if (this.isPhraseBarShowing) {
            this.isPhraseBarShowing = false;

            this.iKnowUKeyboardView.getKeyboard().offsetKeyYPos(-this.minHeight);
            this.iKnowUKeyboardView.requestLayout();
        }*/
    }

	/**
	 * A connection back to the service to communicate with the text field
	 * 
	 * @param sb the {@link IKnowUKeyboardService} to be connected with
	 */
	public void setKeyboardService(IKnowUKeyboardService sb) {
		keyboardService = sb;
        this.screenWidth = Size.getScreenWidth(this.context);
	}

	public void resetScreenLocations() {
		candidateViewLoc[0] = candidateViewLoc[1] = 0;
	}
	
	/**
	 * Send a message to the handler to initiate a long-press
	 * 
	 * @param index the index of the view on which the long-press will occur
	 */
	public void sendLongPressMessage(int index, long timeout) {
        if (!mHandler.hasMessages(MSG_LONGPRESS)) {
            Message msg = mHandler.obtainMessage(MSG_LONGPRESS);
            msg.arg1 = index;
            mHandler.sendMessageDelayed(msg, timeout);
        }
	}
	
	/**
	 * Send a message to the handler to initiate a long-press
	 * 
	 * @param index the index of the view on which the long-press will occur
	 */
	public void sendLongPressMessage(int index) {
        if (!mHandler.hasMessages(MSG_LONGPRESS)) {
            //IKnowUKeyboardService.log(Log.VERBOSE, "sendLongPressMessage()", "obtaining message");
            IKnowUKeyboardService.log(Log.VERBOSE, CorrectionSuggestion.TAG, "sendLongPressMessage()");
            Message msg = mHandler.obtainMessage(MSG_LONGPRESS);
            msg.arg1 = index;
            mHandler.sendMessageDelayed(msg, Theme.LONG_PRESS_TIMEOUT);
        }
	}
	
	/**
	 * Cancel any long-press messages
	 */
	public void cancelLongPressMessage() {
        IKnowUKeyboardService.log(Log.VERBOSE, CorrectionSuggestion.TAG, "cancelLongPressMessage()");
		mHandler.removeMessages(MSG_LONGPRESS);
	}

	/**
	 * Trim whitespace at the end of a string
	 * 
	 * @param str string to be trimmed
	 * @return the trimmed string
	 */
	public static String trimTrailing(String str) {
		try {
			if (str == null)
				return null;
			int len = str.length();
			for (; len > 0; len--) {
				if (!Character.isWhitespace(str.charAt(len - 1)))
					break;
			}
			return str.substring(0, len);
		} catch (Exception e) {
			IKnowUKeyboardService.sendErrorMessage(e);
			return null;
		}
	} // end of trimTrailing
	
	/*@Override
	protected void onSizeChanged (int w, int h, int oldw, int oldh) {
		super.onSizeChanged(w, h, oldw, oldh);
		
		if (this.suggestionsOrder == ORDER_CENTER_OUT && this.getWidth() > this.keyboardService.displayWidth) {
			int width = (this.getWidth() - this.keyboardService.displayWidth) / 2;
			this.parent.smoothScrollTo(width, 0);
		}	
	}*/
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    public void addPredictionSuggestion(String suggestion, String rootText, int capsMode, int textMode, boolean isNewWord, boolean isRoot) 
    {
        if (suggestion != null && suggestion.length() > 0) 
        {
            PredictionSuggestion tview = new PredictionSuggestion(this.context);
            tview.init(this.keyboardService, this, this.fontSize, Theme.CANDIDATE_TEXT_COLOR, Theme.CANDIDATE_SELECTED_COLOR, this.minItemWidth);

            IKnowUKeyboardService.log(Log.VERBOSE, "addPredictionSuggestion()", "setting appropriate values for, predictionSugg = "+suggestion+", isRoot?"+isRoot);

            int index = this.suggestions.size();
            int capmode = capsMode;
            boolean isChunk = false;
            if (isRoot) {
                tview.setAsRoot();
            } else {	
                isChunk = keyboardService.getPredictionEngine().isChunk(suggestion);
                if (capsMode == PredictionSuggestion.CAPS_FIRST_LETTER && !isChunk )
                	capmode = CorrectionSuggestion.CAPS_NONE;
             }

            IKnowUKeyboardService.log(Log.VERBOSE, "addPredictionSuggestion", "isChunk? "+isChunk+", capmode="+capmode+",  suggestion = " + suggestion + "root = "+rootText);
            tview.setSuggestion(index, rootText, suggestion, capsMode, textMode);
            
            if ( this.suggestions.size() <= 0 )
                tview.setBold();

            this.totalWidth += tview.getWidth();
            IKnowUKeyboardService.log(Log.ERROR, "SuggLinLay", " totalWidth = "+this.totalWidth);
            /*if ( suggestions[this.orderCenterOut[i]][1].equals(CORRECTED_NEW_WORD) || suggestions[this.orderCenterOut[i]][1].equals(CORRECTED_KNOWN_WORD)) {
                tview.setBold();
            } else if (this.orderCenterOut[i] == 0) {
                tview.setBold(this.highestPriorityColor);
                this.highestPriorityIndex = Integer.valueOf(this.suggestionsCount);
            }*/

            this.suggestions.add(tview);
        } 
        else
            IKnowUKeyboardService.log(Log.ERROR, "SuggLinLay", " Pred TextView = NULL !!!!");
    }

    public void addCorrectionSuggestion(String suggestion, int capsMode, int textMode, boolean isNewWord, boolean isCorrectedWord, boolean isRoot) {
        if (suggestion != null && suggestion.length() > 0) {
            CorrectionSuggestion tview = new CorrectionSuggestion(this.context);
            tview.init(this.keyboardService, this, this.fontSize, Theme.CANDIDATE_TEXT_COLOR, Theme.CANDIDATE_SELECTED_COLOR, this.minItemWidth);

            IKnowUKeyboardService.log(Log.VERBOSE, "addCorrectionSuggestion()", "setting appropriate values for, correctionSugg = "+suggestion);

            int index = this.suggestions.size();

            if (isRoot) {
                //tview.setSuggestion(index, suggestion, "...", capsMode, textMode);
                tview.setSuggestion(index, suggestion, "   ", capsMode, textMode);
                tview.setAsRoot();
            } else if (isCorrectedWord && isNewWord){ //corrected word that we don't know already
                tview.setSuggestion(index, "+ ", suggestion, capsMode, textMode);
                tview.setAsNewWord();
                tview.setBold();
            } else if (isCorrectedWord){ //corrected word that we know already
                tview.setSuggestion(index, "", suggestion, capsMode, textMode);
                tview.setBold();
            } else {
                tview.setSuggestion(index, "", suggestion, capsMode, textMode);
            }

            if ( this.suggestions.size() <= 0 ) 
                tview.setBold();

            this.totalWidth += tview.getWidth();
            IKnowUKeyboardService.log(Log.ERROR, "SuggLinLay", " totalWidth = "+this.totalWidth);

            this.suggestions.add(tview);
        } 
        else
            IKnowUKeyboardService.log(Log.ERROR, "SuggLinLay", " Corr TextView = NULL !!!!");
    }
    
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    public void addPhraseSuggestion(String suggestion, String rootText, int capsMode, int textMode) 
    {
        if (suggestion != null && suggestion.length() > 0) 
        {
            PredictionSuggestion tview = new PredictionSuggestion(this.context);
            tview.init(this.keyboardService, this, this.fontSize, Theme.CANDIDATE_TEXT_COLOR, Theme.CANDIDATE_SELECTED_COLOR, this.minItemWidth);

            IKnowUKeyboardService.log(Log.VERBOSE, "addPredictionSuggestion()", "setting appropriate values for, predictionSugg = "+suggestion);

            int index = this.phraseSuggestions.size();
            /*String text = suggestion.substring(0, suggestion.length()-3);
            boolean isChunk = IKnowUKeyboardService.getPredictionEngine().isChunk(text);*/

            /*if (capsMode == PredictionSuggestion.CAPS_FIRST_LETTER *//*&& !isChunk*//* ) {
                IKnowUKeyboardService.log(Log.VERBOSE, "setSuggestionsArray()", "CAPS_FIRST_LETTER detected, but not a chunk! So show without caps -> suggestion = " + suggestion + "root = "+rootText);
                tview.setSuggestion(index, rootText, suggestion, CorrectionSuggestion.CAPS_NONE, textMode);
            } else {*/
            IKnowUKeyboardService.log(Log.VERBOSE, "SuggestionLineaLayout.addPhraseSuggestion()", "CAPS_FIRST_LETTER detected, it is a chunk! So show with caps -> suggestion = " + suggestion + "root = "+rootText);
            tview.setSuggestion(index, rootText, suggestion, capsMode, textMode);


            //this.totalWidth += tview.getWidth();
            //IKnowUKeyboardService.log(Log.ERROR, "SuggLinLay", " totalWidth = "+this.totalWidth);
            this.totalPhraseWidth += tview.getWidth();
            IKnowUKeyboardService.log(Log.ERROR, "SuggLinLay.addPhrasePrediction()", " totalPhraseWidth = "+this.totalPhraseWidth);
            /*if ( suggestions[this.orderCenterOut[i]][1].equals(CORRECTED_NEW_WORD) || suggestions[this.orderCenterOut[i]][1].equals(CORRECTED_KNOWN_WORD)) {
                tview.setBold();
            } else if (this.orderCenterOut[i] == 0) {
                tview.setBold(this.highestPriorityColor);
                this.highestPriorityIndex = Integer.valueOf(this.suggestionsCount);
            }*/

            if (this.phraseSuggestions.size() <= 0)
                this.showPhraseBar();
            
            this.phraseSuggestions.add(tview);
        } else {
            IKnowUKeyboardService.log(Log.ERROR, "SuggLinLay", " TextView = NULL !!!!");
        }
    }

    public String getSuggestionAt(int index) {
        Suggestion sugg = this.suggestions.get(index);
        if (sugg != null) {
            return sugg.getRawSuggestion();
        }
        return null;
    }

    /**
     * Returns true if suggestion is contained in suggestion list
     * @param suggestion the suggestion string
     * @return true if found; otherwise false
     */
    public boolean containsSuggestion(String suggestion) {
        String s1 = suggestion.trim().toLowerCase();
        for (Suggestion sugg : this.suggestions) {
            String s2 = sugg.getRawSuggestion().toLowerCase().trim();
            if (s2.equals(s1)) {
                return true;
            }
        }
        return false;
    }

	/**
	 * Get the highest priority item from the current list of items being displayed
	 * @return
	 */
	public String getHighestPriority() {
		Suggestion tview = (Suggestion) this.suggestions.get(0);
		if (tview != null) {
			return tview.getRawSuggestion();
		} else {
			return null;
		}
	}
	
	/**
	 * Clear the list of elements currently being displayed
	 */
	public void clear() {
		//this.removeAllViews();
        this.suggestions.clear();
        this.phraseSuggestions.clear();
        this.totalWidth = 0;
        this.totalPhraseWidth = 0;
        this.scrollTo(0,0);
        this.scrollPhraseTo(0,0);
	}
	
	/**
	 * Request a key click action to be played
	 */
	public void sendKeyClick() {
		keyboardService.audioAndClickFeedback(1);
	}
}
