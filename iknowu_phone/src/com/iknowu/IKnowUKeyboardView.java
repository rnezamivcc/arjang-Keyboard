package com.iknowu;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.content.res.Resources;
import android.graphics.Canvas;
import android.graphics.LinearGradient;
import android.graphics.Paint;
import android.graphics.Path;
import android.graphics.PorterDuff;
import android.graphics.Rect;
import android.graphics.RectF;
import android.graphics.Region;
import android.graphics.Shader;
import android.graphics.Typeface;
import android.graphics.drawable.Drawable;
import android.os.Handler;
import android.os.Message;
import android.preference.PreferenceManager;
import android.util.AttributeSet;
import android.util.Log;
import android.util.TypedValue;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.inputmethod.InputConnection;
import android.widget.PopupWindow;

import com.iknowu.IKnowUKeyboard.Key;
import com.iknowu.asian.Hangul;
import com.iknowu.preview.PreviewTextView;
import com.iknowu.swipe.Swipe;
import com.iknowu.util.Theme;

import java.io.IOException;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;

import com.iknowu.R;

//import android.inputmethodservice.Keyboard;
//import android.inputmethodservice.Keyboard.Key;

/**
 * The KeyboardView extends a regular Android {@link View} instead of the Android KeyboardView.
 * 
 * This is to allow greater flexibility and incorporate the custom {@link IKnowUKeyboard} class that we have created.
 * 
 * This class handles all of the presentation for an XML defined {@link IKnowUKeyboard}.
 * As well as detecting all of the touch events and passing off the interpreted information to the {@link IKnowUKeyboardService}
 * 
 * @author Justin
 * 
 */
public class IKnowUKeyboardView extends View /* extends KeyboardView * implements KeyboardView.OnKeyboardActionListener */ {
	static final int KEYCODE_OPTIONS = -100;
	static final int NUMBER_OF_SEARCHROWS = 6;


	protected static final int THEME_GRADIENT_DIAGONAL = 0;
	protected static final int THEME_GRADIENT_VERTICAL = 1;
	protected static final int THEME_GRADIENT_HORIZONTAL = 2;
	
	protected static final int THEME_STYLE_REGULAR = 0;
	protected static final int THEME_STYLE_OUTLINE = 1;
	protected static final int THEME_STYLE_LINESBETWEEN = 2;
	
	protected static final int MOVEMENT_THRESHHOLD = 200;

    protected static final int SWIPE_MOVEMENT_THRESHOLD = 8;
    protected static final int PREFERENCE_THRESHOLD = 4;
	
	protected static final long PREVIEW_DELAY = 250;
	
	static public IKnowUKeyboardService keyboardService;
	private Context mContext;
	protected SuggestionsLinearLayout suggestionsView;
    protected KeyboardLinearLayout keyboardLinearLayout;
	
	//private int deadZoneKeyIndex = -1;
	private boolean mSearchMode = false;
	public int lastTouchX, lastTouchY;
	protected float touchStartX;
    protected float touchStartY;
	private int[] keyboardLoc = new int[2];
	
	public static final int MSG_LONGPAUSE = 40;
	public static final int MSG_DELETEWORDFROMEDITOR = 41;
	public static final int MSG_LONGPRESS = 43;
	public static final int MSG_SEARCH_BACKBUTTON = 44;
	public static final int MSG_SEARCH_SCROLL = 45;
	public static final int MSG_REMOVE_PREVIEW = 46;
	public static final int MSG_REMOVE_POPUP = 47;
    public static final int MSG_CHECK_CURSOR_UPDATES = 48;
    public static final int MSG_LONG_PRESS_KB_PICKER = 49;
    public static final int MSG_SEARCH_SPACEBUTTON = 50;

    public static final int UPDATE_INTERVAL = 500;
	
	public static final int TEXT_STYLE_REGULAR = 0;
	public static final int TEXT_STYLE_BIG = 1;
	public static final int TEXT_STYLE_BIG_OFFSET = 2;

    public static final int GESTURE_X_COORD_THRESHOLD = 100;
    public static final int GESTURE_Y_COORD_THRESHOLD = 100;
	
	private Paint paint;
	private boolean mLongPressDeleteWordsEngaged = false;  // repetition indicator
	
	private Drawable mKbSwitchingIcon = null;
	private Drawable mLogoSettingsIcon = null;
	private Drawable mBackspaceLongPressIcon = null;
	protected Drawable mShiftActive = null;
	protected Drawable mShiftLockActive = null;
	private Drawable enterLongPressIcon = null;

	public boolean mPopupKeyboardActive = false;	//true when this instance of iknowukeyboardview created a popup
	
	// color 5 keys maximum
	protected int[] m_oldColorKeyHighlights = new int[6];
	protected int[] m_newColorKeyHighlights = new int[6];
	protected int[] numberOfFollowers = new int[6];
	
	// the searchlist contains chunks and non-chunks
	// line up with number of rows in xml file kbsearchlist.xml
	//private boolean[] mPredictionIsChunk = new boolean[9];
	
	protected int mKeyTextSize = 1; //34;
	protected int mLabelTextSize = 1; //22; // longer than 1 character
	protected int mUprLabelTextSize = 1; //16;
	
	public int searchListTextSize;

   // private int mPadLeft = 1;
   // private int mPadRight = 1;
   // private int mPadTop = 1;
   // private int mPadBottom = 1;

    private static int KEY_PADDING_LR_FULL = 2;
    private static int KEY_PADDING_LR_THUMB = 2;
    private static int KEY_PADDING_TB_FULL = 2;
    private static int KEY_PADDING_TB_THUMB = 2;
    
    private int keyPaddingLR = 5;
    private int keyPaddingTB = 5;
    private int keyCharPaddingB = 5;
    private int clipPadding = 3;

    private int viewWidth;

    //drawing variables
    private RectF keyRect;
    protected boolean bKeyDarkBackground;
    protected float labelWidth;
    protected String labelText;
    protected RectF shadowRect;

	int mpkPadTextVertical = 5;
	int mpkPadUprTextRightEdgeIn = 5;
	int mpkPadUprTextTopEdgeBelow = 5;

	int mpkUprIconPadLeft = 8;
	int mpkUprIconPadTop = 8;
	int mpkUprIconPadRight = 8;
	int mpkUprIconPadBottom = 8;
	
	private float keyHeightScale;
	
	//static int counttt = 0;
	
	public SearchList searchList;			//a custom class to hold all of our search list items
	public float densityScale;					//passed on from the input service, allows us to draw things to correct sizes on screen

	private long animStartTime;					//track the start time for the search list animation
	private boolean fingerRaisedAfterSearchListExit;	//check to see if they have released there finger after exiting search mode
	
	public boolean releaseWithNoConsequence;	//a flag to tell the input service that it should ignore the following key release
	public boolean resizable;					//it this is true then when we measure this view we will know to resize the keys
	
	public boolean isattached;					//flag to tell whether or not this view has been attached to its window
	
	protected PreviewTextView previewText;
	private PopupWindow previewPopup;
	private int previewPopupX;
	private int previewPopupY;

	protected boolean cancelSwipeDetect;
	
	//private Keyboard current;
	
	private boolean drawFlash;
	public boolean onkey;
	
	private boolean miniAppsAvailable;
	private Rect miniAppRect;
	private LinearGradient miniAppGradient;
	
	public boolean tutorialMode;
	private TutorialView tutorialView;
	
	public IKnowUKeyboard keyboard;
	
	public Typeface font;
	private int textStyle;
	
	protected boolean touchDown;
	protected String nextLetters;

    protected boolean inKeyboardPicker;

    public boolean updatePredEngineOnRelease;
    public boolean ignoreShift;

    public boolean isSideView;
    public boolean isLefty;

	public static Swipe swiper;
	
    private boolean inTouch;
 //   private LinkedList<PossibleChar> possibleChars;
    private PossibleChar currentChar;
	
	/**
	 * Handler used for long-press events and other UI related delayed tasks
	 */
	Handler mHandler = new Handler() {
		@Override
		public void handleMessage(Message msg) {
			//Log.d("HANDLER MSG", ""+msg);
			switch (msg.what) {
			case MSG_LONGPAUSE:
				fireLongPause(msg.arg1);
				break;
			case MSG_DELETEWORDFROMEDITOR:
				deleteEntireWordFromEditor();
				break;
			case MSG_LONGPRESS:
				Key firstKey = findKeyByCoords(msg.arg1, msg.arg2);
				Key currentKey = findKeyByCoords(lastTouchX, lastTouchY);
				
				if (currentKey != null && firstKey != null && currentKey.codes[0] == firstKey.codes[0]) {
					handleLongPress(firstKey, msg.arg1, msg.arg2);
				}
				break;
			case MSG_SEARCH_BACKBUTTON:
				handleSearchBackButtonPressed(msg.arg1);
				break;
			case MSG_SEARCH_SCROLL:
				//searchList.updateScroll();
				break;
			case MSG_REMOVE_PREVIEW:
				if (!touchDown) {
					hidePreview();
				} else {
					this.removeMessages(MSG_REMOVE_PREVIEW);
					Message msg1 = this.obtainMessage(MSG_REMOVE_PREVIEW);
					this.sendMessageDelayed(msg1, PREVIEW_DELAY);
				}
				break;
			case MSG_REMOVE_POPUP:
				hidePreview();
				break;
            case MSG_CHECK_CURSOR_UPDATES:
                keyboardService.resetKeysAhead();
                break;
            case MSG_LONG_PRESS_KB_PICKER:
                Key hover = findKeyByCoords(msg.arg1, msg.arg2);
                Key lastKey = findKeyByCoords(lastTouchX, lastTouchY);
                if (lastKey != null && hover != null && lastKey.codes[0] == hover.codes[0]) {
                    handleLongPress(hover, msg.arg1, msg.arg2);
                }
                break;
            case MSG_SEARCH_SPACEBUTTON:
                handleSearchListSpace(msg.arg1);
                break;
			}
		}
	};
	
	//=================================================================================
	//CONTRUCTORS
	//=================================================================================
    public IKnowUKeyboardView(Context context) {
        super(context);
        mContext = context;
        //this.setPreviewEnabled(false);
        if (paint == null) 
        	paint = new Paint();

        if(swiper == null)
        	swiper = new Swipe();
        
        if(currentChar == null)
        	currentChar = new PossibleChar();
        
        this.setWillNotDraw(false);
        //this.setOnKeyboardActionListener(this);
    }

	public IKnowUKeyboardView(Context context, AttributeSet attrs) throws IOException {
		super(context, attrs);
		mContext = context;
		//this.setPreviewEnabled(false);
		if (paint == null) 
			paint = new Paint();
        if(swiper == null)
        	swiper = new Swipe();
        if(currentChar == null)
        	currentChar = new PossibleChar();
        

        this.setWillNotDraw(false);
		//this.setOnKeyboardActionListener(this);
	}

	public IKnowUKeyboardView(Context context, AttributeSet attrs, int defStyle) {
		super(context, attrs, defStyle);
		mContext = context;
		//this.setPreviewEnabled(false);
		if (paint == null) 
			paint = new Paint();
 
        if(swiper == null)
        	swiper = new Swipe();
        if(currentChar == null)
        	currentChar = new PossibleChar();
        
        this.setWillNotDraw(false);
		//this.setOnKeyboardActionListener(this);
	}
	
	/**
	 * Message caught to delete a complete word form the editor.
	 * Relays this message to the {@link IKnowUKeyboardService}
	 */
    public void deleteEntireWordFromEditor() {
    	mLongPressDeleteWordsEngaged = true;
		mHandler.removeMessages(MSG_DELETEWORDFROMEDITOR);
    	boolean keepgoing = keyboardService.deleteEntireWordFromEditor();
    	if (keepgoing) {
    		Message msg = mHandler.obtainMessage(MSG_DELETEWORDFROMEDITOR);
    		msg.arg1 = 0;
    		mHandler.sendMessageDelayed(msg, Theme.DELETE_WORD_DELAY_TIMEOUT);
    	}
    }
    
    /**
     * Check if the delte key is being long-pressed
     * @return
     */
    public boolean longPressDeleteEntireWordEngaged()
    {
    	return mLongPressDeleteWordsEngaged;
    }
    
    /**
     * Stop the delete entire word message in the {@link Handler}
     */
    public void stopDeleteEntireWordFromEditor()
    {
    	if (mLongPressDeleteWordsEngaged) {
	       	//Log.d("WLKB", "stopped delete entire word from editor");
	       	mLongPressDeleteWordsEngaged = false;
	       	if (mHandler != null) {
	       		mHandler.removeMessages(MSG_DELETEWORDFROMEDITOR);
	       	}
            if (keyboardService != null) {
                keyboardService.updatePredEngineHistoryLatin(false);
            }
    	}
    }
    
    /**
     * A long-press has occurred on the {@link SearchList}
     * @param keyIndex the index of the {@link SearchListItem} that was pressed
     */
	public void fireLongPause(int keyIndex) {
		SearchListItem item = this.searchList.fireLongPause(keyIndex);
		if (item != null) {
			switchToSearchListKeyboard(item.xCoord, item.yCoord, 0, item.getText());
		}
	}
	
	public int getKeyboardLoc() {
		return keyboardLoc[1];
	}

	public void resetScreenLocations() {
		keyboardLoc[0] = keyboardLoc[1] = 0;
	}
	/**
	 * Determine the View's location on screen
	 */
	public void establishScreenLocations() {
		if (keyboardLoc[1] == 0) {
			getLocationOnScreen(keyboardLoc);
		}
	}

	/**
	 * Set the {@link IKnowUKeyboardService} to be set
	 * @param sb
	 */
	public void setKeyboardService(IKnowUKeyboardService sb) {
		keyboardService = sb;
	}

    public void setKeyboardLinearLayout(KeyboardLinearLayout kbl) {
        this.keyboardLinearLayout = kbl;
    }

    public SuggestionsLinearLayout getSuggestionsView() {
        return this.suggestionsView;
    }
	
	/*public void setSuggestionsView(SuggestionsLinearLayout sugg) {
		this.suggestionsView = sugg;
	}*/
	
	/**
	 * Process all the necessary attributes for this keyboard, these are mostly all related to
	 * visual aspects of the keyboard
	 * This function must be called after softkeyboard has been set, otherwise we will get null pointer exceptions
	 * @param themeId the theme to load for the keyboard
	 */
	public void processAttributes( ) {
		try {
			Resources r = mContext.getResources();
	        
	        this.densityScale = getResources().getDisplayMetrics().densityDpi;

            IKnowUKeyboardService.log(Log.VERBOSE, "iKnowUKeyboardView.processAttributes()", "creating typeface, current font = "+this.font);

	        this.font = Theme.TYPEFACE;
	        
	        this.setTheme();
			
	        this.keyHeightScale = keyboardService.keyHeightDP;

            if (!keyboardService.mLandscape || keyboardService.tabletThumbView) {
                this.keyPaddingLR = (int) TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, KEY_PADDING_LR_THUMB, r.getDisplayMetrics());
            } else {
                this.keyPaddingLR = (int) TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, KEY_PADDING_LR_FULL, r.getDisplayMetrics());
            }
            this.keyPaddingTB = (int) TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, KEY_PADDING_TB_FULL, r.getDisplayMetrics());
            this.keyCharPaddingB = (int) TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, this.keyCharPaddingB, r.getDisplayMetrics());
	        this.clipPadding = (int) TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, this.clipPadding, r.getDisplayMetrics());
	        
	        mKbSwitchingIcon = r.getDrawable(R.drawable.kbswitchicon); // upr right corner
//	        mLogoSettingsIcon = r.getDrawable(R.drawable.wllogo_settings); // upr right corner
	        mLogoSettingsIcon = r.getDrawable(R.drawable.wlinfo_settings); // upr right corner
	        mBackspaceLongPressIcon = r.getDrawable(R.drawable.backspace_upr_icon);  // upr right corner
	        enterLongPressIcon = r.getDrawable(R.drawable.kb_pic_smiley1);  // upr right corner
	        
			mShiftActive = r.getDrawable(R.drawable.sym_keyboard_shift_1); 
			mShiftLockActive = r.getDrawable(R.drawable.sym_keyboard_shift_lock);
			
			SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(mContext);
			
			this.textStyle = Integer.parseInt(sp.getString(IKnowUKeyboardService.PREF_KB_FONT_STYLE, "" + TEXT_STYLE_BIG));

			if ( this.textStyle == TEXT_STYLE_REGULAR ) {
		        mKeyTextSize = r.getDimensionPixelSize(R.dimen.kbview_tablet_keytextsize);
		        mLabelTextSize = r.getDimensionPixelSize(R.dimen.kbview_tablet_labeltextsize);
			} else if ( this.textStyle == TEXT_STYLE_BIG || this.textStyle == TEXT_STYLE_BIG_OFFSET ) {
                mKeyTextSize = r.getDimensionPixelSize(R.dimen.kbview_tablet_keytextsize_big);
                mLabelTextSize = r.getDimensionPixelSize(R.dimen.kbview_tablet_labeltextsize);
			}
	        
	        mUprLabelTextSize = r.getDimensionPixelSize(R.dimen.kbview_tablet_uprlabeltextsize);
	        this.searchListTextSize = r.getDimensionPixelSize(R.dimen.searchlist_textsize);
	        
	        paint.setAntiAlias(true);
	        paint.setTypeface(this.font);

            mpkPadTextVertical = r.getDimensionPixelSize(R.dimen.kbview_key_pkpadtextvertical);
	        mpkPadUprTextRightEdgeIn = r.getDimensionPixelSize(R.dimen.kbview_key_tablet_pkpaduprtextrightedgein);
	        mpkPadUprTextTopEdgeBelow = r.getDimensionPixelSize(R.dimen.kbview_key_pkpaduprtexttopedgebelow);

			mpkUprIconPadLeft = r.getDimensionPixelSize(R.dimen.kbview_key_upriconpadleft);
			mpkUprIconPadTop = r.getDimensionPixelSize(R.dimen.kbview_key_upriconpadright);
			mpkUprIconPadRight = r.getDimensionPixelSize(R.dimen.kbview_key_upriconpadtop);
			mpkUprIconPadBottom = r.getDimensionPixelSize(R.dimen.kbview_key_upriconpadbottom);
			
			//Initialize variables to defaults
			this.searchList.searchBackButtonPressed = false;
            this.searchList.searchSpaceButtonPressed = false;
			this.fingerRaisedAfterSearchListExit = false;
			this.releaseWithNoConsequence = false;
			this.searchList.startText = "";
			this.resizable = false;
			
			int previewWidth = r.getDimensionPixelSize(R.dimen.preview_width);
			int previewHeight = r.getDimensionPixelSize(R.dimen.preview_height);
			
			this.previewPopup = new PopupWindow(mContext);
			this.previewPopup.setBackgroundDrawable(null);
			this.previewPopup.setWidth(previewWidth);
			this.previewPopup.setHeight(previewHeight);
			this.previewPopup.setTouchable(false);
			
			this.previewText = new PreviewTextView(mContext);
			this.previewText.init(Theme.PREVIEW_BACKGROUND_COLOR, Theme.PREVIEW_BORDER_COLOR, Theme.PREVIEW_TEXT_COLOR);
			this.previewPopup.setContentView(this.previewText);

            /*
             * Create candidate view, not actually a view anymore, will be custom drawn on the canvas
             */
            if (!this.isSideView) {
                this.suggestionsView = new SuggestionsLinearLayout(keyboardService, this);
                this.suggestionsView.setKeyboardService(keyboardService);
                this.suggestionsView.setPosition(0,0);
            }
			
			/**
			 * check the version of the app, is it doesn't match the one set in the preferences then show the "tutorial"
			 * or more appropriately the "what's new" dialog
			 */
			String version = sp.getString(IKnowUKeyboardService.PREF_VERSION_NAME, "");
			if ( !version.equals(this.mContext.getResources().getString(R.string.version_name)) ) {
				this.tutorialView = new TutorialView(this.mContext, this);
				this.tutorialMode = true;
				this.tutorialView.setStartTime(System.currentTimeMillis());
				
				Editor edit = sp.edit();
				edit.putString(IKnowUKeyboardService.PREF_VERSION_NAME, this.mContext.getResources().getString(R.string.version_name));
				edit.commit();
			}
		} catch (Exception e) {
			IKnowUKeyboardService.sendErrorMessage(e);
		}
	}
	
	/**
	 * Set key attributes, such as label widths, upper text, etc.
	 * 
	 * This is to optimize the drawing process.
	 */
	private void setKeyAttributes() {
		List<Key> keys = this.getKeyboard().getKeys();
		Iterator<Key> it = keys.iterator();

		while (it.hasNext()) {
			IKnowUKeyboard.Key key = it.next();
			
			if (key.label != null) {
				if (key.label.length() > 1 && key.codes.length < 2) {
					paint.setTextSize(mLabelTextSize);
				} else {
					paint.setTextSize(mKeyTextSize);
				}
				
				key.labelWidthLower = paint.measureText(key.label.toString());
                if (key.labelUpperCase != null) {
                    key.labelWidthUpper = paint.measureText(key.labelUpperCase.toString());
                } else {
                    key.labelWidthUpper = paint.measureText(key.label.toString().toUpperCase());
                }
				
				key.upperText = keyboardService.popupManager.getDefaultChar(key.label.toString());
				key.upperTextCaps = keyboardService.popupManager.getDefaultChar(key.label.toString().toUpperCase());

                paint.setTextSize(this.mUprLabelTextSize);

                if (key.upperText != null && key.upperTextCaps != null) {
                    key.upperTextWidth = paint.measureText(key.upperText);
                    key.upperTextWidthCaps = paint.measureText(key.upperTextCaps.toUpperCase());
                }
			}
		}// end paint keys loop
	}
	
	/**
	 * Set the theme of this keyboard
	 * @param themeId the theme to load
	 */
	public void setTheme() {
        /*if (this.isSideView) {
            this.setBackgroundColor(Theme.SIDE_BACKGROUND_COLOR);
        } else {
            this.setBackgroundColor(Theme.BACKGROUND_COLOR);
        }*/
        
        this.searchList = new SearchList(this, this.densityScale, keyboardService.mLandscape,
                Theme.SEARCH_ITEM_TEXT_COLOR, Theme.SEARCH_ITEM_COLOR, Theme.SEARCH_ITEM_PRESSED_COLOR, Theme.SEARCH_ITEM_BACK_COLOR);
	}

    public void setAsSideView() {
        this.isSideView = true;
    }

    public void setLeftyMode(boolean islefty) {
        this.isLefty = islefty;
    }
	
	/**
	 * Paint a key to the canvas
	 * @param key the key to be painted
	 * @param canvas the canvas to paint to
	 * @param colorRef any color to overlay the key, used for key highlighting etc.
	 * @param bCenterText whether or no the text should be centered
	 */
	private void paintKey(Key key, Canvas canvas, int colorRef, boolean bCenterText) {
		try {
//			int padLeft = 4; // 2;
//			int padRight = 1; // 3;
//			int padTop = 8; // 6;
//			int padBottom = 5; // 3;
			bKeyDarkBackground = false;

			if (key instanceof IKnowUKeyboard.Key /* IKnowUKeyboard.LatinKey */ ) {
				//IKnowUKeyboard.LatinKey lKey = (IKnowUKeyboard.LatinKey) key;
				bKeyDarkBackground = key.getDarkBackground();
			}
			
			//keyRect = new RectF( (key.x + mPadLeft), (key.y + mPadTop), (key.x + key.width - mPadLeft - mPadRight), (key.y + key.height - mPadBottom) );
			keyRect = new RectF( (key.x + keyPaddingLR), (key.y + keyPaddingTB), (key.x + key.width - (keyPaddingLR * 2)), (key.y + key.height - keyPaddingTB) );
			//IKnowUKeyboardService.log(Log.VERBOSE, "KbView paint key", "left = "+keyRect.left+", top = "+keyRect.top+", right = "+keyRect.right+", bottom = "+keyRect.bottom);
			/*
			 * Key drawing logic
			 * (Now incorporates themes)
			 * 
			 * Three basic key styles for now:
			 * 
			 * - Lines between = keys are represented by a background color, separated by a line on either side
			 * 
			 * - Outline = keys are draw with and empty space in the middle and only an outline shown
			 * 
			 * - Regular = keys are drawn using a standard rounded rectangle filled with the appropriate color
			 */
			switch(Theme.KEY_STYLE) {
			/*
			 * Style for regular
			 */
			case IKnowUKeyboardView.THEME_STYLE_REGULAR:
				paint.setStyle(Paint.Style.FILL);
				if (key.pressed) {
					paint.setColor(Theme.KEY_PRESSED_COLOR);
					canvas.drawRoundRect(keyRect, Theme.CORNER_RADIUS_X, Theme.CORNER_RADIUS_Y, paint);
					//if (!this.mPopupKeyboardActive && !this.mSearchMode)
					//	this.showPreview(key);
				} else {
					this.shadowRect = new RectF( (keyRect.left+2), (keyRect.top+2), (keyRect.right+2), (keyRect.bottom+2) );
					paint.setColor(Theme.KEY_SHADOW_COLOR);
					canvas.drawRoundRect(shadowRect, Theme.CORNER_RADIUS_X, Theme.CORNER_RADIUS_Y, paint);
					if (bKeyDarkBackground) {
						paint.setColor(Theme.KEY_DARK_COLOR);
						/*
						if (this.useGradient) {
							switch (this.gradientDirection) {
							case IKnowUKeyboardView.THEME_GRADIENT_DIAGONAL:
								mPaint.setShader(new LinearGradient(keyRect.left, keyRect.top, keyRect.right, keyRect.bottom, this.keyPressedColor, this.keyDarkColor, Shader.TileMode.REPEAT));
								break;
							case IKnowUKeyboardView.THEME_GRADIENT_VERTICAL:
								mPaint.setShader(new LinearGradient(keyRect.left, keyRect.top, keyRect.left, keyRect.bottom, this.keyPressedColor, this.keyDarkColor, Shader.TileMode.REPEAT));
								break;
							case IKnowUKeyboardView.THEME_GRADIENT_HORIZONTAL:
								mPaint.setShader(new LinearGradient(keyRect.left, keyRect.top, keyRect.right, keyRect.top, this.keyPressedColor, this.keyDarkColor, Shader.TileMode.REPEAT));
								break;
							}
						}
						*/
					} else {
						paint.setColor(Theme.KEY_COLOR);
						if (Theme.USE_GRADIENT) {
							switch (Theme.GRADIENT_DIRECTION) {
							case IKnowUKeyboardView.THEME_GRADIENT_DIAGONAL:
								paint.setShader(new LinearGradient(keyRect.left, keyRect.top, keyRect.right, keyRect.bottom, Theme.KEY_PRESSED_COLOR, Theme.KEY_COLOR, Shader.TileMode.REPEAT));
								break;
							case IKnowUKeyboardView.THEME_GRADIENT_VERTICAL:
								paint.setShader(new LinearGradient(keyRect.left, keyRect.top, keyRect.left, keyRect.bottom, Theme.KEY_PRESSED_COLOR, Theme.KEY_COLOR, Shader.TileMode.REPEAT));
								break;
							case IKnowUKeyboardView.THEME_GRADIENT_HORIZONTAL:
								paint.setShader(new LinearGradient(keyRect.left, keyRect.top, keyRect.right, keyRect.top, Theme.KEY_PRESSED_COLOR, Theme.KEY_COLOR, Shader.TileMode.REPEAT));
								break;
							}
						}
					}
					canvas.drawRoundRect(keyRect, Theme.CORNER_RADIUS_X, Theme.CORNER_RADIUS_Y, paint);
				}
				break;
			/*
			 * Styling for outline
			 */
			case IKnowUKeyboardView.THEME_STYLE_OUTLINE:
				paint.setStyle(Paint.Style.STROKE);
				paint.setStrokeWidth(Theme.BORDER_STROKE);
				if (key.pressed) {
					paint.setColor(Theme.KEY_PRESSED_COLOR);
					canvas.drawRoundRect(keyRect, Theme.CORNER_RADIUS_X, Theme.CORNER_RADIUS_Y, paint);
					//if (!this.mPopupKeyboardActive && !this.mSearchMode)
					//	this.showPreview(key);
				} else {
					paint.setStyle(Paint.Style.FILL);
					this.shadowRect = new RectF( keyRect.left, keyRect.top, keyRect.right, keyRect.bottom );
					paint.setColor(Theme.KEY_SHADOW_COLOR);
					canvas.drawRoundRect(shadowRect, Theme.CORNER_RADIUS_X, Theme.CORNER_RADIUS_Y, paint);
					paint.setStyle(Paint.Style.STROKE);
					if (bKeyDarkBackground) {
						paint.setColor(Theme.KEY_DARK_COLOR);
                        if (Theme.USE_GRADIENT) {
                            switch (Theme.GRADIENT_DIRECTION) {
							case IKnowUKeyboardView.THEME_GRADIENT_DIAGONAL:
								paint.setShader(new LinearGradient(keyRect.left, keyRect.top - 3, keyRect.right, keyRect.bottom + 3, Theme.KEY_PRESSED_COLOR, Theme.KEY_COLOR, Shader.TileMode.REPEAT));
								break;
							case IKnowUKeyboardView.THEME_GRADIENT_VERTICAL:
								paint.setShader(new LinearGradient(keyRect.left, keyRect.top - 3, keyRect.left, keyRect.bottom + 3, Theme.KEY_PRESSED_COLOR, Theme.KEY_COLOR, Shader.TileMode.REPEAT));
								break;
							case IKnowUKeyboardView.THEME_GRADIENT_HORIZONTAL:
								paint.setShader(new LinearGradient(keyRect.left - 3, keyRect.top - 3, keyRect.right + 3, keyRect.top - 3, Theme.KEY_PRESSED_COLOR, Theme.KEY_COLOR, Shader.TileMode.REPEAT));
								break;
							}
						}
					} else {
						paint.setColor(Theme.KEY_COLOR);
                        if (Theme.USE_GRADIENT) {
                            switch (Theme.GRADIENT_DIRECTION) {
							case IKnowUKeyboardView.THEME_GRADIENT_DIAGONAL:
								paint.setShader(new LinearGradient(keyRect.left, keyRect.top - 3, keyRect.right, keyRect.bottom + 3, Theme.KEY_PRESSED_COLOR, Theme.KEY_COLOR, Shader.TileMode.REPEAT));
								break;
							case IKnowUKeyboardView.THEME_GRADIENT_VERTICAL:
								paint.setShader(new LinearGradient(keyRect.left, keyRect.top - 3, keyRect.left, keyRect.bottom + 3, Theme.KEY_PRESSED_COLOR, Theme.KEY_COLOR, Shader.TileMode.REPEAT));
								break;
							case IKnowUKeyboardView.THEME_GRADIENT_HORIZONTAL:
								paint.setShader(new LinearGradient(keyRect.left - 3, keyRect.top - 3, keyRect.right + 3, keyRect.top - 3, Theme.KEY_PRESSED_COLOR, Theme.KEY_COLOR, Shader.TileMode.REPEAT));
								break;
							}
						}
					}
					
					canvas.drawRoundRect(keyRect, Theme.CORNER_RADIUS_X, Theme.CORNER_RADIUS_Y, paint);
				}
				break;
			/*
			 * Style for lines between
			 */
			case IKnowUKeyboardView.THEME_STYLE_LINESBETWEEN:
				paint.setStyle(Paint.Style.FILL);
				paint.setStrokeWidth(Theme.BORDER_STROKE);
				if (key.pressed) {
					paint.setColor(Theme.KEY_PRESSED_COLOR);
					canvas.drawRoundRect(keyRect, Theme.CORNER_RADIUS_X, Theme.CORNER_RADIUS_Y, paint);
					//this.previewText.setBackgroundColor(this.backgroundColor);
					//if (!this.mPopupKeyboardActive && !this.mSearchMode)
					//	this.showPreview(key);
				} else {
					if (bKeyDarkBackground) {
						paint.setColor(Theme.KEY_DARK_COLOR);
					} else {
						paint.setColor(Theme.KEY_COLOR);
					}
					//canvas.drawLine(keyRect.right + mPadRight, keyRect.top, keyRect.right + mPadRight, keyRect.bottom, mPaint);
					//canvas.drawLine(keyRect.left + mPadLeft, keyRect.bottom, keyRect.right + mPadRight + (STROKE_WIDTH/2), keyRect.bottom, mPaint);
					canvas.drawLine(keyRect.right + keyPaddingLR, keyRect.top, keyRect.right + keyPaddingLR, keyRect.bottom, paint);
					canvas.drawLine(keyRect.left + keyPaddingLR, keyRect.bottom, keyRect.right + keyPaddingLR + (Theme.BORDER_STROKE/2), keyRect.bottom, paint);
				}
				break;
			}
			
			//reset shader and fill settings
			paint.setShader(null);
			paint.setStyle(Paint.Style.FILL);

			// either draw the background or color the background of the key
			if (colorRef != 0xFFFFFFFF) {
				//Log.d("COLOR_REF =", ""+colorRef);
				paint.setColor(colorRef);
                if (Theme.USE_GRADIENT) {
                    switch (Theme.GRADIENT_DIRECTION) {
					case IKnowUKeyboardView.THEME_GRADIENT_DIAGONAL:
						paint.setShader(new LinearGradient(keyRect.left, keyRect.top, keyRect.right, keyRect.bottom, Theme.KEY_PRESSED_COLOR, colorRef, Shader.TileMode.REPEAT));
						break;
					case IKnowUKeyboardView.THEME_GRADIENT_VERTICAL:
						paint.setShader(new LinearGradient(keyRect.left, keyRect.top, keyRect.left, keyRect.bottom, Theme.KEY_PRESSED_COLOR, colorRef, Shader.TileMode.REPEAT));
						break;
					case IKnowUKeyboardView.THEME_GRADIENT_HORIZONTAL:
						paint.setShader(new LinearGradient(keyRect.left, keyRect.top, keyRect.right, keyRect.top, Theme.KEY_PRESSED_COLOR, colorRef, Shader.TileMode.REPEAT));
						break;
					}
				}
				canvas.drawRoundRect(keyRect, Theme.CORNER_RADIUS_X, Theme.CORNER_RADIUS_Y, paint);
				paint.setShader(null);
			}

			if (key.label != null) {
				boolean functionLabel = key.label.length() > 1 && (key.codes[0] < 0 || key.codes[0] == 10);
				
				paint.setStyle(Paint.Style.FILL);
				if (bKeyDarkBackground && Theme.KEY_STYLE == THEME_STYLE_LINESBETWEEN) {
					paint.setColor(Theme.KEY_DARK_COLOR);
		    	} else {
		    		paint.setColor(Theme.KEY_TEXT_COLOR);
		    	}
				
				if (key.label.length() > 1 && key.codes.length < 2) {
					paint.setTextSize(mLabelTextSize);
				} else {
					paint.setTextSize(mKeyTextSize);
				}
				
				this.labelText = key.label.toString();
				this.labelWidth = key.labelWidthLower;
				// make sure you only upper case the keys used for input, not keys indicating function
	 			if (this.keyboard.isShifted() && key.codes[0] > 0) {
					// regular label lower case or upper case the main key
                    if (key.labelUpperCase != null) {
                        this.labelText = key.labelUpperCase.toString();
                    } else {
                        this.labelText = key.label.toString().toUpperCase();
                    }
	 				this.labelWidth = key.labelWidthUpper;
				}
				
				switch (this.textStyle) {
					case TEXT_STYLE_REGULAR:
						if (bCenterText) {
							// regular keys center the label in their key, make sure caps don't overlap with upper labels
							if (functionLabel) {
								if (labelWidth == 0) {
									labelWidth = paint.measureText(key.label.toString());
								}
								canvas.drawText(this.labelText, keyRect.left + ( ((keyRect.right - keyRect.left) / 2) - (this.labelWidth / 2) ), 
										key.y + (key.height - (keyPaddingTB * 2)) / 2 + 
											(paint.getTextSize() - paint.descent())/2 + keyPaddingTB, paint);
							} else {
								canvas.drawText(this.labelText, keyRect.left + (((keyRect.right - keyRect.left) / 2) - (this.labelWidth / 2)),
										keyRect.bottom - this.keyPaddingTB - this.keyPaddingLR - this.keyCharPaddingB, paint);
							}
						}
					break;
					case TEXT_STYLE_BIG:
						if (bCenterText) {
							// regular keys center the label in their key, make sure caps don't overlap with upper labels
							if (functionLabel) {
								canvas.save();
								canvas.clipRect(keyRect, Region.Op.REPLACE);
								canvas.drawText(this.labelText, key.x + (key.width - labelWidth) / 2 , key.y + (key.height - (keyPaddingTB * 2)) / 2 + 
											(paint.getTextSize() - paint.descent())/2 + keyPaddingTB, paint);
								canvas.restore();
							} else {
								canvas.save();
								int bottomUp = 0;
								if ( !this.keyboard.isShifted() ) {
									bottomUp = (int) (paint.descent() - this.clipPadding);
								} else if (this.labelText.equals(",")) {
									bottomUp = (int) (paint.descent() - this.clipPadding);
								}
								canvas.clipRect(keyRect, Region.Op.REPLACE);
								canvas.drawText(this.labelText, keyRect.left + (((keyRect.right - keyRect.left) / 2) - (this.labelWidth / 2)),
										keyRect.bottom - this.keyPaddingTB - this.keyPaddingLR - (this.keyCharPaddingB / 2), paint);
								canvas.restore();
							}
						}
					break;
					case TEXT_STYLE_BIG_OFFSET:
						if (bCenterText) {	
							// regular keys center the label in their key, make sure caps don't overlap with upper labels
							if (functionLabel) {
								canvas.save();
								canvas.clipRect(keyRect, Region.Op.REPLACE);
								canvas.drawText(this.labelText, key.x + (key.width - labelWidth) / 2 , key.y + (key.height - (keyPaddingTB * 2)) / 2 + 
											(paint.getTextSize() - paint.descent())/2 + keyPaddingTB, paint);
								canvas.restore();
							} else {
								canvas.save();
								int bottomUp = 0;
								if ( !this.keyboard.isShifted() ) {
									bottomUp = (int) (paint.descent() - this.clipPadding);
								} else if (this.labelText.equals(",")) {
									bottomUp = (int) (paint.descent() - this.clipPadding);
								}
								canvas.clipRect(keyRect, Region.Op.REPLACE);
								canvas.drawText(this.labelText, keyRect.right - this.labelWidth + this.clipPadding,
										(keyRect.bottom - bottomUp), paint);
								canvas.restore();
							}
						}
					break;
				}
			}
			else if (key.icon != null) {
				// how do I retrieve the current padding
				// Rect left , top, right , bottom
				// main icon central on the key
				Rect padding = new Rect (mpkUprIconPadLeft, mpkUprIconPadTop, mpkUprIconPadRight, mpkUprIconPadBottom);
				
				Drawable icon = key.icon;
				
				//if key is the shift key
				if (key.codes[0] == -1) {
					if (this.keyboard.isShifted()) {
						icon = keyboardService.isCapsLock() ? mShiftLockActive : mShiftActive; 
					}
				}
				
				if (icon != null) {
				    final int drawableX = (key.width - padding.left - padding.right - icon.getIntrinsicWidth()) / 2 + padding.left;
				    final int drawableY = (key.height - padding.top - padding.bottom - icon.getIntrinsicHeight()) / 2 + padding.top;
				    canvas.save();
				    canvas.translate(key.x + drawableX, key.y + drawableY);
				    icon.setBounds(0, 0, icon.getIntrinsicWidth(), icon.getIntrinsicHeight());
				    //dont want to color over emoticons
				    if(key.codes[0] != -44) {
				    	if (bKeyDarkBackground && Theme.KEY_STYLE == THEME_STYLE_LINESBETWEEN) {
				    		icon.setColorFilter( Theme.KEY_DARK_COLOR, PorterDuff.Mode.SRC_ATOP );
				    	} else {
				    		icon.setColorFilter( Theme.KEY_TEXT_COLOR, PorterDuff.Mode.SRC_ATOP );
				    	}
				    }
				    icon.draw(canvas);
				    //canvas.translate(-key.x -drawableX, -key.y -drawableY);
				    canvas.restore();
				}
			}
			
			//Draw upper default characters for the key
            //IKnowUKeyboardService.log(Log.VERBOSE, "paintkey", "uppertext = "+key.upperText);
			if ( key.codes[0] != -44 && ( (key.upperText != null && key.upperText.length() > 0) || (key.upperTextCaps != null && key.upperTextCaps.length() > 0) ) ) {
				paint.setColor(Theme.KEY_UPPER_ICON_COLOR);
				paint.setTextSize(mUprLabelTextSize);
				if (this.keyboard.isShifted() && key.codes[0] > 0) {
					canvas.drawText(key.upperTextCaps , key.x + 10, key.y + mpkPadUprTextTopEdgeBelow /*24*/, paint);
				} else {
					canvas.drawText( key.upperText, key.x + 10, key.y + mpkPadUprTextTopEdgeBelow /*24*/, paint);
				}
			}
			
			if ((key.codes[0] == -6 || key.codes[0] == -2 )) { // 123 or abc label
//					Rect padding = new Rect (6,8,6,4);
				Rect padding = new Rect ( mpkUprIconPadLeft, mpkUprIconPadTop, mpkUprIconPadRight, mpkUprIconPadBottom);
				int iconWidth = mKbSwitchingIcon.getIntrinsicWidth();
				int iconHeight = mKbSwitchingIcon.getIntrinsicHeight();
				
				canvas.save();
			    canvas.translate(key.x + key.width - iconWidth - padding.right, key.y + padding.top );
			    mKbSwitchingIcon.setBounds(0, 0, iconWidth, iconHeight);
			    mKbSwitchingIcon.setColorFilter( Theme.KEY_UPPER_ICON_COLOR, PorterDuff.Mode.SRC_ATOP );
			    mKbSwitchingIcon.draw(canvas);
			    canvas.restore();
			    //canvas.translate(-(key.x + key.width - iconWidth - padding.right), -(key.y + padding.top));
			}
			
			if (key.codes[0] == -3 ) { // close button 
//					Rect padding = new Rect (6,8,6,4);
				Rect padding = new Rect ( mpkUprIconPadLeft, mpkUprIconPadTop, mpkUprIconPadRight, mpkUprIconPadBottom);
				int iconWidth = mLogoSettingsIcon.getIntrinsicWidth();
				int iconHeight = mLogoSettingsIcon.getIntrinsicHeight();
				
				canvas.save();
			    canvas.translate(key.x + key.width - iconWidth - padding.right, key.y + padding.top );
			    mLogoSettingsIcon.setBounds(0, 0, iconWidth, iconHeight);
			    mLogoSettingsIcon.setColorFilter( Theme.KEY_UPPER_ICON_COLOR, PorterDuff.Mode.SRC_ATOP );
			    mLogoSettingsIcon.draw(canvas);
			    canvas.restore();
			    //canvas.translate(-(key.x + key.width - iconWidth - padding.right), -(key.y + padding.top));
			}

			if (key.codes[0] == -5 ) { // back button 
				Rect padding = new Rect ( mpkUprIconPadLeft, mpkUprIconPadTop, mpkUprIconPadRight, mpkUprIconPadBottom);
				int iconWidth = mBackspaceLongPressIcon.getIntrinsicWidth();
				int iconHeight = mBackspaceLongPressIcon.getIntrinsicHeight();
				
				canvas.save();
			    canvas.translate(key.x + key.width - iconWidth - padding.right, key.y + padding.top );
			    mBackspaceLongPressIcon.setBounds(0, 0, iconWidth, iconHeight);
			    this.mBackspaceLongPressIcon.setColorFilter( Theme.KEY_UPPER_ICON_COLOR, PorterDuff.Mode.SRC_ATOP );
			    mBackspaceLongPressIcon.draw(canvas);
			    canvas.restore();
			    //canvas.translate(-(key.x + key.width - iconWidth - padding.right), -(key.y + padding.top));
			}
			
			if (key.codes[0] == 10 ) { // enter 
				Rect padding = new Rect ( mpkUprIconPadLeft, mpkUprIconPadTop, mpkUprIconPadRight, mpkUprIconPadBottom);
				int iconWidth = 15; /* this.enterLongPressIcon.getIntrinsicWidth(); */
				int iconHeight = 15; /* this.enterLongPressIcon.getIntrinsicHeight(); */
				
				canvas.save();
			    canvas.translate(key.x + key.width - iconWidth - padding.right, key.y + padding.top );
			    this.enterLongPressIcon.setBounds(0, 0, iconWidth, iconHeight);
			    this.enterLongPressIcon.setColorFilter( Theme.KEY_UPPER_ICON_COLOR, PorterDuff.Mode.SRC_ATOP );
			    this.enterLongPressIcon.draw(canvas);
			    canvas.restore();
			    //canvas.translate(-(key.x + key.width - iconWidth - padding.right), -(key.y + padding.top));
			}
		} catch (Exception e) {
			IKnowUKeyboardService.sendErrorMessage(e);
		}
	}// end paintKey
	
	/**
	 * Shows our own custom preview due to the stock android implementation being
	 * very inflexible
	 * @param key the key to show a preview for
	 */
	protected void showPreview(Key key) {
		try {
			mHandler.removeMessages(MSG_REMOVE_PREVIEW);

            this.previewText.setTextColor(Theme.PREVIEW_TEXT_COLOR);
			if (key.label != null) {
				this.previewText.setCompoundDrawables(null, null, null, null);
				if (key.label.length() > 1) {
					this.previewText.setTextSize(TypedValue.COMPLEX_UNIT_SP, 30);
				} else {
                    this.previewText.setTextSize(TypedValue.COMPLEX_UNIT_SP, 66);
                }
				
				if (this.keyboard.isShifted()) {
					this.previewText.setText(key.label.toString().toUpperCase());
				} else {
					this.previewText.setText(key.label); 
				}
			} else if (key.icon != null) {
                if (key.codes[0] == -1 || key.codes[0] == 32 || key.codes[0] == -5 || key.codes[0] == -32) {
                    this.previewText.setText(key.text);
                    this.previewText.setTextSize(TypedValue.COMPLEX_UNIT_SP, 20);
                    key.iconPreview.mutate().setColorFilter(Theme.PREVIEW_TEXT_COLOR, PorterDuff.Mode.SRC_ATOP);
                } else if (key.codes[0] != -44) {
				    this.previewText.setText("");
                    key.iconPreview.mutate().setColorFilter(Theme.PREVIEW_TEXT_COLOR, PorterDuff.Mode.SRC_ATOP);
                } else {
                    this.previewText.setText(key.text);
                    this.previewText.setTextSize(TypedValue.COMPLEX_UNIT_SP, 40);
                    this.previewText.setTextColor(Theme.PREVIEW_ALT_TEXT_COLOR);
                }
                this.previewText.setCompoundDrawables(null, key.iconPreview, null, null);
			}

			this.previewPopupX = (key.x + (key.width / 2)) - (this.previewPopup.getWidth() / 2);
			
			int[] loc = new int[2];
			this.getLocationOnScreen(loc);
			this.previewPopupX += loc[0];
			
			this.previewPopupY = key.y - this.previewPopup.getHeight();
			
			if (keyboardService.isExtractViewShown() || keyboardService.isFullscreenMode()) {
				this.getLocationInWindow(loc);
				this.previewPopupY += (loc[1] - 40);
			}

            // Calculate the x and y offset of this view within the window that contains it.
            int[] xy = new int[2];
            this.getLocationInWindow(xy);
            this.previewPopupY += xy[1]; // Only the y offset is needed.

            IKnowUKeyboardService.log(Log.WARN, "IKnowUKeyboardView.showPreview()", "previewY = " + this.previewPopupY + ", key.y = " + key.y);
			
			if (this.previewPopup.isShowing()) {
				this.previewPopup.update(previewPopupX, previewPopupY, -1, -1);
			} else {
				this.previewPopup.showAtLocation(this, Gravity.NO_GRAVITY, this.previewPopupX, this.previewPopupY);
			}

			Message msg = mHandler.obtainMessage(MSG_REMOVE_PREVIEW);
			mHandler.sendMessageDelayed(msg, PREVIEW_DELAY);

		} catch (Exception e) {
			IKnowUKeyboardService.sendErrorMessage(e);
		}
	}
	
	/**
	 * Hide the currently shown key preview
	 */
	public void hidePreview() {
		IKnowUKeyboardService.log(Log.INFO, "HIDE PREVIEW", "CALLED");
		if (this.previewPopup != null) {
			if (this.previewPopup.isShowing())
				this.previewPopup.dismiss();
		}
	}
	
	/**
	 * Get the {@link IKnowUKeyboard} associated with this view
	 * @return the keyboard
	 */
	public IKnowUKeyboard getKeyboard() {
		return this.keyboard;
	}
	
	/**
	 * Set the keyboard to associate with this view
	 * When we set a keyboard, try to resize it to fit into the
	 * keyboardview's width
	 * 
	 * @param keyboard the keyboard to associate with
	 */
	public void setKeyboard(IKnowUKeyboard keyboard) {
		//IKnowUKeyboard kb = (IKnowUKeyboard) keyboard
		keyboard.setKeyHeights(this.keyHeightScale);
		this.keyboard = keyboard;
		//super.setKeyboard(kb);
		this.setKeyAttributes();
		//this.tutorialView.createKeyGrid(kb);
		this.requestLayout();
	}
	
	/**
	 * This view has reported a size change.
	 * Check to see if we set it to "resizable" and if so
	 * resize its current keyboard
	 */
	@Override
	public void onSizeChanged(int w, int h, int oldw, int oldh) {
		super.onSizeChanged(w, h, oldw, oldh);
		try {
			/*if (this.resizable) {
				//IKnowUKeyboard kb = (IKnowUKeyboard) this.getKeyboard();
				this.keyboard.resizeKeys(this.getWidth(), keyboardService.displayWidth);
			}*/
			this.searchList.setViewport(this.getWidth(), this.getHeight());
		} catch (Exception e) {
			IKnowUKeyboardService.sendErrorMessage(e);
		}
	}
	
	@Override
	public void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
		// Round up a little
		//IKnowUKeyboard kb = (IKnowUKeyboard) this.getKeyboard();
		if (this.keyboard == null) {
			setMeasuredDimension(this.getPaddingLeft() + this.getPaddingRight(), this.getPaddingTop()
					+ this.getPaddingBottom());
		} else {
			int width = this.keyboard.getMinWidth() + this.getPaddingLeft() + this.getPaddingRight();
			if (MeasureSpec.getSize(widthMeasureSpec) < width + 10) {
				width = MeasureSpec.getSize(widthMeasureSpec);
			}

            if (width < this.viewWidth) {
                width = viewWidth;
            }

            int suggestionHeight = 0;
            if (!this.isSideView) {
                this.suggestionsView.resizeComponents(width);
                this.suggestionsView.scrollToCenter();
                this.suggestionsView.scrollPhraseToCenter();
                suggestionHeight = this.suggestionsView.getHeight();
            }
			int height = this.keyboard.getHeight() + suggestionHeight;
			//Log.v("KBView", "kb height = "+height);
			setMeasuredDimension(width, height + this.getPaddingTop() + this.getPaddingTop());
		}
	}

    public void resizeComponents(int width) {
        this.viewWidth = width;
    }

    public int getViewWidth() {
        return this.viewWidth;
    }
	
	@Override
	public void onAttachedToWindow() {
		super.onAttachedToWindow();
	    this.isattached = true;
	}
	
	/**
	 * Check to see if the primary code passed in matches any of the codes
	 * in the array passed in, and if so, return the number of followers
	 * that the key has
	 * @param codes the key to check
	 * @param keysToPaint the array that contains the keys that need to be highlighted
	 * @return the number of followers that this key has, or -1 if none
	 */
	private int keyNeedsColor(int[] codes, int keysToPaint[]) {
		for (int i = 0; i < keysToPaint.length; i++) {
			for (int j=0; j < codes.length; j++) {
				if (keysToPaint[i] == codes[j]) {
					return numberOfFollowers[i];
				}
			}
		}
		return -1;
	}
	
	/**
	 * Clear all the highlighted keys on the keyboard
	 */
	public void clearNextKeyHighlighting() {
		for (int i = 0; i < 5; i++) {
			m_newColorKeyHighlights[i] = 0;
			numberOfFollowers[i] = 0; // 0 is still considered color, -1 not
		}
	}
	
	/**
	 * Determine which keys should be highlighted now based on the previous text that has been input.
	 */
	public void prepareNextKeyHighlighting() {
		IKnowUKeyboardService.log(Log.WARN, "PREPARENEXTKEYHIGHLTING", "HERE");
		try {
			if (!keyboardService.getTrialExpired()) {
				// first preserve the old key coloring
				System.arraycopy(m_newColorKeyHighlights, 0, m_oldColorKeyHighlights, 0, 5);
				
				clearNextKeyHighlighting();
				
				if (keyboardService.numNextWords > 0) {
					// get the information on the new key coloring
					this.nextLetters = IKnowUKeyboardService.getPredictionEngine().nextLetters(numberOfFollowers);
					//IKnowUKeyboardService.log(Log.WARN, "Key Highlighting", "Colored letters = "+coloredLetters);
					
					if (this.nextLetters != null) {
						for (int i = 0; i < 5 && i < this.nextLetters.length(); i++) {
							m_newColorKeyHighlights[i] = this.nextLetters.charAt(i);
						}
					} else {
						this.clearNextKeyHighlighting();
						this.invalidate();
					}
				} else {
					this.clearNextKeyHighlighting();
					this.invalidate();
				}
			} else {
				this.clearNextKeyHighlighting();
				this.invalidate();
			}
		} catch (Exception e) {
			IKnowUKeyboardService.sendErrorMessage(e);
		}
	}

	@SuppressLint("WrongCall")
	@Override
	public void onDraw(Canvas canvas) {
		try {
			//IKnowUKeyboardService.log(Log.DEBUG, "ONDRAW", "CALLED, preview isShowing = "+this.previewPopup.isShowing());

            //draw background
            this.paint.setColor(Theme.BACKGROUND_COLOR);
            if (!this.isSideView) {
                canvas.drawRect(0f, this.getSuggestionsView().getHeight(), this.getWidth(), this.getHeight(), this.paint);
            }

			if (this.getKeyboard() != null) {
				List<Key> keys = this.getKeyboard().getKeys();
				Iterator<Key> it = keys.iterator();
				
				boolean bColorKey = keyboardService.mHighlightedKeysOn;

				while (it.hasNext()) {
					IKnowUKeyboard.Key key = it.next();

					int nrOfFollowers = keyNeedsColor(key.codes, m_newColorKeyHighlights);
					if (nrOfFollowers >= 5) {
						paintKey(key, canvas, bColorKey ? Theme.KEY_COLOR_MORE_THAN_FIVE : 0xFFFFFFFF, true);
					} else if (nrOfFollowers >= 0) {
						paintKey(key, canvas, bColorKey ? Theme.KEY_COLOR_LESS_THAN_FIVE : 0xFFFFFFFF , true);
					} else {
						paintKey(key, canvas, 0xFFFFFFFF, true);
					}
				}// end paint keys loop
			}

            if (!this.isSideView)
                this.suggestionsView.onDraw(canvas);

            if (IKnowUKeyboardService.SWIPE_ENABLED && this.inTouch) {
                this.drawPathLine(canvas);
            }
			
			if (mPopupKeyboardActive || this.mSearchMode) {
				//dim out the background keyboard
                this.paint.setStyle(Paint.Style.FILL);
				paint.setColor(0x99000000);
				canvas.drawRect(0,0,getWidth(), getHeight(), paint);
			}
			
			if (this.mSearchMode) {
				paint.setTextSize(this.searchListTextSize);
				//check to see whether we are animating, if we are, then
				//perform calculations and draw everything
				if (animStartTime > 0) {
					
					double interpolation = (double) ((System.currentTimeMillis() - animStartTime) / (double) (searchList.animDuration));
					
					if (interpolation >= 1) {
						animStartTime = 0;
						interpolation = 1;
					}
					
					this.searchList.draw(canvas, interpolation, paint);
					
					invalidate();
				} else {
					animStartTime = 0;
					this.searchList.draw(canvas, 1, paint);
				}
			}//end if searchmode
			
			if (this.tutorialMode) {
				this.tutorialView.onDraw(canvas, paint);
			}

			if (this.drawFlash) {
				paint.setColor(0xffececec);
				canvas.drawRect(0, 0, this.getWidth(), this.getHeight(), paint);
                canvas.drawRect(0, 0, this.getWidth(), this.getSuggestionsView().getHeight(), paint);
				this.drawFlash = false;
				this.invalidate();
			}
			
			if (this.miniAppsAvailable) {
				paint.setShader(this.miniAppGradient);
				canvas.drawRect(miniAppRect, paint);
			}
			paint.setShader(null);
		} catch (Exception e) {
	    	IKnowUKeyboardService.sendErrorMessage(e);
	    }
	}//end onDraw

	/**
	 * Add a highlighted bar as a visual indication to the user that there are REach min-apps available to be accessed
	 * @param on whether or not to show or hide the indicator
	 */
	public void setMiniAppIndicator(boolean on) {
		this.miniAppsAvailable = on;
		this.miniAppRect = new Rect(0, this.getHeight()-30, this.getWidth(), this.getHeight());

		/*
		int red = Color.red(this.backgroundColor);
		int green = Color.green(this.backgroundColor);
		int blue = Color.blue(this.backgroundColor);

		int top = Color.parseColor("#00"+red+green+blue);
		*/

		this.miniAppGradient = new LinearGradient(miniAppRect.left, miniAppRect.top, miniAppRect.left, miniAppRect.bottom, 0x0033B5E5, 0xAA33B5E5, Shader.TileMode.REPEAT);
	}

    /**
     * Draw a line showing the path a user has traversed via their swiping touch gestures
     * @param canvas
     */
    private void drawPathLine(Canvas canvas) 
    {
        Paint.Style oldStyle = paint.getStyle();
        float oldFloatWidth = paint.getStrokeWidth();

        paint.setStyle(Paint.Style.STROKE);
        paint.setStrokeWidth(8);
        paint.setColor(0xaa4Eb7ff);

        Path path = new Path();
        if (swiper.size() > 1) 
        {
        	String spath = swiper.printPath();
        	Swipe.Sw_Node prevNode = null;
        	
            for (int i = 0; i < swiper.size(); i++) 
            {
            	Swipe.Sw_Node node = swiper.get(i);
                IKnowUKeyboardService.log(Log.VERBOSE, "IKnowUKeyboardView", "drawPathLine:"+i+":"+node.x+","+node.y+":"+node.count+","+node.val);

                if (i == 0) {
                    path.moveTo(node.x, node.y);
                } else {
                    float midX = (prevNode.x + node.x) / 2.0f;
                    float midY = (prevNode.y + node.y) / 2.0f;

                    if (i == 1) {
                        path.lineTo(midX, midY);
                    } else {
                        path.quadTo(prevNode.x, prevNode.y, midX, midY);
                    }
                }
                prevNode = node;
            }
            path.lineTo(prevNode.x, prevNode.y);
            
            IKnowUKeyboardService.getPredictionEngine().advanceLetterSwipe(swiper.getNodes());
        }
        canvas.drawPath(path, paint);

        paint.setStyle(oldStyle);
        paint.setStrokeWidth(oldFloatWidth);
    }
	
	/***************************************************
	 * Set the flash on for a visual indication that a correction has occured.
	 **************************************************/
	public void setFlashOn() {
		this.drawFlash = true;
		this.invalidate();
	}
	
	@Override
	public boolean onTouchEvent(MotionEvent me) {
        if (!isEnabled()) {
            return true;
        }

		try {
			int action = me.getAction();
			
			this.releaseWithNoConsequence = false;
			
			/**
			 * if not in tutorial mode then proceed as normal, otherwise pass all touch events to tutorial view
			 */
			if (!this.tutorialMode) {
				
				if (action == MotionEvent.ACTION_CANCEL) {
                    IKnowUKeyboardService.log(Log.VERBOSE, "onTouch", "action cancel");

					this.touchDown = false;
					mHandler.removeMessages(MSG_LONGPRESS);
					mHandler.removeMessages(MSG_DELETEWORDFROMEDITOR);
					mHandler.removeMessages(MSG_LONGPAUSE);
					this.hidePreview();
					
					int x = (int) me.getX();
					int y = (int) me.getY();

                    if (this.getKeyboard() != null) {
                        if ( this.getKeyboard().currentPressedKey != null) 
                        	this.getKeyboard().currentPressedKey.pressed = false;
                        Key key = this.keyboard.getKeyFromCoords(x, y);
                        if (key != null) {
                            key.pressed = false;
                        }
                    }
				}
				
				/**
                 * if showing a popup keyboard
                 */
                if (mPopupKeyboardActive) {
                    final boolean ret = this.popupTouchEvent(me);
                    if (ret) 
                    	return ret;
                }
		        
		        int pointerCount = me.getPointerCount();
		        int multiAction = -1;
		        int pointerIndex = 0;
				//Log.d("POINTER COUNT = "+pointerCount, "action = "+action);
				
				if (pointerCount > 1) {
					// Extract the index of the pointer that left the touch sensor
			        pointerIndex = (action & MotionEvent.ACTION_POINTER_INDEX_MASK) >> MotionEvent.ACTION_POINTER_INDEX_SHIFT;
					multiAction = action & MotionEvent.ACTION_MASK;
				}


                //flag to tell the rest of the function whether or not the touch is being handled by the suggestion bar.
                boolean touchInSuggestions = false;
                if ( !this.isSideView && this.suggestionsView.isInside((int)me.getX(), (int)me.getY()) ) {
                    //IKnowUKeyboardService.log(Log.INFO, "onTouchEvent()", "suggestionsView.isInside() = true");
                    touchInSuggestions = true;
                    this.suggestionsView.onTouchEvent(me);
                    this.invalidate();
                }

				IKnowUKeyboardService.log(Log.VERBOSE, "keyboardView", "onTouchEvent action = "+action+", multiAction = "+multiAction);
				
				if (action == android.view.MotionEvent.ACTION_DOWN || multiAction == MotionEvent.ACTION_POINTER_DOWN) 
				{
					IKnowUKeyboardService.log(Log.ERROR, "ONTOUCHEVENT DOWN", "===================================================================");
                    //this.ikuScroller.setPagingEnabled(false);

                    this.touchDown = true;
                    this.inTouch = true;

						lastTouchX = (int) me.getX(pointerIndex);
						lastTouchY = (int) me.getY(pointerIndex);
						this.touchStartX = me.getX(pointerIndex);
                        this.touchStartY = me.getY(pointerIndex);
						this.cancelSwipeDetect = (pointerIndex > 0);

                    if (!this.isSideView) {
                        //Utilize a custom long press to allow for the user to change this in the settings
                        Message msg = mHandler.obtainMessage(MSG_LONGPRESS);
                        msg.arg1 = (int) me.getX();
                        msg.arg2 = (int) me.getY();
                        mHandler.sendMessageDelayed(msg, Theme.LONG_PRESS_TIMEOUT);
                    }
					
					/**
					 * Find out if this touch lands in a key
					 */
					int x = (int) me.getX();
					int y = (int) me.getY();
					
					Key key = this.keyboard.getKeyFromCoords(x, y);

                    if (IKnowUKeyboardService.SWIPE_ENABLED)
                        this.tryAddPoint(x, y, key);

                    //don't need to do these if the touch is in the suggestion bar
                    if (!touchInSuggestions) {
                        if (key != null) {
                            this.showPreview(key);
                            this.onPressKey(key);
                            invalidate();
                        }

                        //Utilize a custom long press to allow for the user to change this in the settings
                        Message msg = mHandler.obtainMessage(MSG_LONGPRESS);
                        msg.arg1 = (int) me.getX();
                        msg.arg2 = (int) me.getY();
                        mHandler.sendMessageDelayed(msg, Theme.LONG_PRESS_TIMEOUT);
                    }

				} 
				else if (action == android.view.MotionEvent.ACTION_UP || multiAction == android.view.MotionEvent.ACTION_POINTER_UP) 
				{
					//keyboardService.startTimer();
					this.touchDown = false;
					//turn scroller back on
					mHandler.removeMessages(MSG_LONGPRESS);
                    mHandler.removeMessages(MSG_LONG_PRESS_KB_PICKER);
					//this.previewPopup.dismiss();

                    this.releaseWithNoConsequence = !IKnowUKeyboardService.SWIPE_ENABLED && this.checkForGesture(me.getX(), me.getY());
 
					//this check is for detecting a left swipe, which will delete a word
					int diffx = (int) (this.touchStartX - this.lastTouchX);
					Log.d("diffx = "+diffx, "threshold = "+MOVEMENT_THRESHHOLD);
                    //check for a swipe to the left
					if (!IKnowUKeyboardService.SWIPE_ENABLED && !mSearchMode && !mPopupKeyboardActive && 
							!this.releaseWithNoConsequence && !this.cancelSwipeDetect && diffx > MOVEMENT_THRESHHOLD) {
					//	keyboardService.nextCorrection();
						this.releaseWithNoConsequence = true;
					}

                    //check for a swipe to the right
                    if (!IKnowUKeyboardService.SWIPE_ENABLED && !mSearchMode && !mPopupKeyboardActive && 
                    		!this.releaseWithNoConsequence && !this.cancelSwipeDetect && (-diffx) > MOVEMENT_THRESHHOLD) {
                        this.onReleaseKey(this.getKeyboard().getKeyFromPrimaryCode(32));
                        this.releaseWithNoConsequence = true;
                    }
					this.cancelSwipeDetect = false;

					/**
					 * find a key that may have been pressed
					 */
					
					int x = (int) me.getX(pointerIndex);
					int y = (int) me.getY(pointerIndex);

                    if (this.getKeyboard() != null && this.getKeyboard().currentPressedKey != null) {
					    this.getKeyboard().currentPressedKey.pressed = false;
                    }
					if (!touchInSuggestions && !this.mSearchMode) {
						Key key = this.keyboard.getKeyFromCoords(x, y);
		                  if (IKnowUKeyboardService.SWIPE_ENABLED){
		                        this.tryAddPoint(x, y, key);
		                        this.swiper.printPath();
		                        this.swiper.clear();
		                  }
						if (key != null) {
							this.onReleaseKey(key);
							invalidate();
						}
					}
					//keyboardService.stopTimer("ACTION UP"); what does this error message means bellow??!!! Reza
					IKnowUKeyboardService.log(Log.ERROR, "ONTOUCHEVENT UP", "!!!!!!!!!!!!!!!!!!!====================================================================");
					
				} 
				else if (action == android.view.MotionEvent.ACTION_MOVE && !this.mSearchMode) 
				{
					int x = (int) me.getX();
					int y = (int) me.getY();

                    if (this.getKeyboard() != null && this.getKeyboard().currentPressedKey != null)
					    this.getKeyboard().currentPressedKey.pressed = false;

					Key key = this.keyboard.getKeyFromCoords(x, y);

                    if (IKnowUKeyboardService.SWIPE_ENABLED)
                        this.tryAddPoint(x, y, key);


					if (!touchInSuggestions && key != null) {
						this.showPreview(key);
						key.pressed = true;

                        /*
                         * if we are showing the keyboard picking screen then allow for menu navigation
                         * by simply hovering over an item.
                         */
                        if ( this.inKeyboardPicker && !mHandler.hasMessages(MSG_LONG_PRESS_KB_PICKER) && key.codes[0] == IKnowUKeyboard.KEYCODE_SWITCH_TO_PICKER ) {
                            Message msg = mHandler.obtainMessage(MSG_LONG_PRESS_KB_PICKER);
                            msg.arg1 = (int) me.getX();
                            msg.arg2 = (int) me.getY();
                            mHandler.sendMessageDelayed(msg, Theme.LONG_PRESS_TIMEOUT);
                        }
					}

					if (this.cancelSwipeDetect == false) {
						lastTouchX = x;
						lastTouchY = y;
					}

					invalidate();
					//return true;
				}
				
				if (this.mSearchMode) 
				{
					//if it was a down or move touch event then return true since the searchlist will handle it!
					if (this.searchModeTouchEvent(me)) {
						return true;
					//if it was an up event, check to see if we should add a space
					} else if (!this.releaseWithNoConsequence) {
                        keyboardService.incrementKeysAhead(1);
						Key key = this.keyboard.getKeyFromPrimaryCode(32);
						if (key != null) {
							this.onReleaseKey(key);
							invalidate();
						}
					}
				}
				return true;
			} else {
				this.tutorialView.onTouch(me);
				return true;
			}
		} catch (Exception e) {
			IKnowUKeyboardService.sendErrorMessage(e);
			return false;
		}
	}

    /*******************************************************
     * Add a point to the list of points to be displayed as the trailing line
     * behind the user's swiping
     * @param x the x-coordinate
     * @param y the y-coordinate
     *********************************************************/
    private void tryAddPoint(float x, float y, Key key)
    {

        if (this.swiper.size() >= Swipe.MAX_POINTS) {
          //  this.points.remove();
            return;
        }


        if (key != null && key.label != null) 
        {
            char val = key.label.toString().charAt(0);
        //    int[] nearestKeys = this.keyboard.getNearestKeys(key.index);

            if (currentChar.set == true && currentChar.val != val) {
             	this.swiper.addPoint(currentChar.val, x, y, currentChar.count, false);
              //  keyboardService.handleMultiCharacter(key.label.toString(), new byte[]{1}, (int)val);      
            }
            else if(currentChar.set == false)
            	this.swiper.addPoint(currentChar.val, x, y, currentChar.count, true);
            
            currentChar.set(val);
            IKnowUKeyboardService.log(Log.VERBOSE, "KeyboardView.tryAddPoint()", "x = "+x+", y = "+y+", key = "+key.label+", width = "+key.width+", height = "+key.height);
         }
    }

  /*  private void doesKeyMakeSense(int xdir, int ydir, Key key, int[] nearestKeys) 
    {

        Vector2 prevPoint = this.swiper.getLast();
        PossibleChar prevPossible = this.possibleChars.getLast();
        IKnowUKeyboardService.log(Log.VERBOSE, "doesKeyMakeSense()", "key = "+ ((char)key.codes[0]) + ", prevPosChar = "+prevPossible.val );
        //if it's the same key that has been hit, increase it's preference
        if (key.codes[0] == prevPoint.key.codes[0] ) {
            prevPossible.prefs[prevPossible.curIndex-1] += 2;
            IKnowUKeyboardService.log(Log.VERBOSE, "doesKeyMakeSense()", "pref = "+ prevPossible.prefs[prevPossible.curIndex-1] );
        } else if (prevPossible.prefs[prevPossible.curIndex-1] >= PREFERENCE_THRESHOLD) {

            this.checkDirectionAndFillRemainingSpots(xdir, ydir, key, nearestKeys, prevPossible);

            PossibleChar pc = new PossibleChar();
            pc.addItem(key.codes[0], (byte)2);
            this.addToPossibleChars(pc);
            //this.possibleChars.add(pc);
        } else {
            if ( !prevPossible.addItem(key.codes[0], (byte)2) ) {
                PossibleChar pc = new PossibleChar();
                pc.addItem(key.codes[0], (byte)2);
                this.addToPossibleChars(pc);
                //this.possibleChars.add(pc);
            }
        }
    }

    private void checkDirectionAndFillRemainingSpots(int xdir, int ydir, Key key, int[] nearestKeys, PossibleChar prev) 
    {

        int xmove = 0;
        int ymove = 0;

        int spotsLeft = prev.chars.length - prev.curIndex - 1;
        IKnowUKeyboardService.log(Log.WARN, "checkDirectionAndFillRemainingSpots()", "spotsLeft = "+spotsLeft);

        for (int j=0; j < nearestKeys.length; j++) {

            Key nearKey = this.getKeyboard().getKey(nearestKeys[j]);
            //if significant movement on the x-axis
            if (Math.abs(xdir) > SWIPE_MOVEMENT_THRESHOLD) {
                //moving left or right
                xmove = xdir < 0 ? -1:1;
             }
            else
            	xmove = 0;
            
            //if significant movement on the y-axis
            if (Math.abs(ydir) > SWIPE_MOVEMENT_THRESHOLD) {
                //moving up or down
                ymove = ydir < 0? -1:1;
            }
            else 
            	ymove = 0;

            if ( spotsLeft > 0 ) {
                //check all directions to see which of the nearest keys may have also been the target
                //moving left and up
                if (xmove < 0 && ymove < 0 && nearKey.x < key.x && nearKey.y < key.y) {
                    prev.addItem(nearKey.codes[0], (byte) (prev.pref-2) );
                    IKnowUKeyboardService.log(Log.WARN, "checkDirectionAndFillRemainingSpots()", "neighbour = "+nearKey.label+", adding neighbour from top left");
                    spotsLeft--;
                }
                //moving left
                else if (xmove < 0 && nearKey.x < key.x) {
                    prev.addItem(nearKey.codes[0], (byte) (prev.prefs[prev.curIndex-1]-2) );
                    IKnowUKeyboardService.log(Log.WARN, "checkDirectionAndFillRemainingSpots()", "neighbour = "+nearKey.label+", adding neighbour from left");
                    spotsLeft--;
                }
                //moving up
                else if (ymove < 0 && nearKey.y < key.y) {
                    prev.addItem(nearKey.codes[0], (byte) (prev.prefs[prev.curIndex-1]-2) );
                    IKnowUKeyboardService.log(Log.WARN, "checkDirectionAndFillRemainingSpots()", "neighbour = "+nearKey.label+", adding neighbour from top");
                    spotsLeft--;
                }
                //moving right and up
                else if (xmove > 0 && ymove < 0 && nearKey.x > key.x && nearKey.y < key.y) {
                    prev.addItem(nearKey.codes[0], (byte) (prev.prefs[prev.curIndex-1]-2) );
                    IKnowUKeyboardService.log(Log.WARN, "checkDirectionAndFillRemainingSpots()", "neighbour = "+nearKey.label+", adding neighbour from top right");
                    spotsLeft--;
                }
                //moving right
                else if (xmove > 0 && nearKey.x > key.x) {
                    prev.addItem(nearKey.codes[0], (byte) (prev.prefs[prev.curIndex-1]-2) );
                    IKnowUKeyboardService.log(Log.WARN, "checkDirectionAndFillRemainingSpots()", "neighbour = "+nearKey.label+", adding neighbour from right");
                    spotsLeft--;
                }
                //moving down
                else if (ymove > 0 && nearKey.y > key.y) {
                    prev.addItem(nearKey.codes[0], (byte) (prev.prefs[prev.curIndex-1]-2) );
                    IKnowUKeyboardService.log(Log.WARN, "checkDirectionAndFillRemainingSpots()", "neighbour = "+nearKey.label+", adding neighbour from bottom");
                    spotsLeft--;
                }
                //moving right and down
                else if (xmove > 0 && ymove > 0 && nearKey.x > key.x && nearKey.y > key.y) {
                    prev.addItem(nearKey.codes[0], (byte) (prev.prefs[prev.curIndex-1]-2) );
                    IKnowUKeyboardService.log(Log.WARN, "checkDirectionAndFillRemainingSpots()", "neighbour = "+nearKey.label+", adding neighbour from bottom right");
                    spotsLeft--;
                }
                //moving left and down
                else if (xmove < 0 && ymove > 0 && nearKey.x < key.x && nearKey.y > key.y) {
                    prev.addItem(nearKey.codes[0], (byte) (prev.prefs[prev.curIndex-1]-2) );
                    IKnowUKeyboardService.log(Log.WARN, "checkDirectionAndFillRemainingSpots()", "neighbour = "+nearKey.label+", adding neighbour from bottom left");
                    spotsLeft--;
                }
            } else {
                break;
            }
        }
    }
*/
    public void addToPossibleChars(PossibleChar pc) 
    {
    //    this.possibleChars.add(pc);

        /*int hindex = pc.getHighestPref();
        char ch = pc.chars[hindex];

        byte[] prefs = new byte[pc.curIndex];
        String chars = "";

        for(int i=0; i < pc.chars.length; i++) {
            IKnowUKeyboardService.log(Log.VERBOSE, "addToPossibleChars()", "pc.char = "+pc.chars[i]);
            if (pc.chars[i] != '\0') {
                chars += pc.chars[i];
                prefs[i] = pc.prefs[i];
            }
        }

        keyboardService.handleMultiCharacter(chars, prefs, (int)ch);*/
    }

    /**
     * Remove all of the points
     */
 /*   private void clearPoints()
    {
        this.inTouch = false;

        if (this.possibleChars.size() > 1) {
            //this.createTypedChars();
            this.releaseWithNoConsequence = true;
            /*for (int i=0; i < this.possibleChars.size(); i++) {
                PossibleChar pc = this.possibleChars.get(i);
                int hindex = pc.getHighestPref();
                char ch = pc.chars[hindex];
                String chars = ""+ch;
                IKnowUKeyboardService.log(Log.VERBOSE, "clearPoints()", "possible char = "+ch+", pref = "+pc.prefs[hindex]);
                keyboardService.handleMultiCharacter(chars, new byte[]{pc.prefs[hindex]}, (int)ch);
            }
        }

     //   currentChar = null;
        this.possibleChars.clear();
        this.swiper.clear();
    }
 */
    private void createTypedChars() {
        /*int[] nearestKeys;
        Vector2 point;
        for (int i=0; i < this.points.size(); i++) {
            point = this.points.get(i);
            if (point.key != null) {
                nearestKeys = this.keyboard.getNearestKeys(point.key.index);
                IKnowUKeyboardService.log(Log.INFO, "createTypedChars()", "key = "+point.key.label);
                for (int j=0; j < nearestKeys.length; j++) {
                    IKnowUKeyboardService.log(Log.INFO, "createTypedChars()", "neighbour = "+nearestKeys[j]);
                }
            }
        }*/
    }

    private boolean checkForGesture(float touchx, float touchy) {
        /*
        if (touchx - this.touchStartX > GESTURE_X_COORD_THRESHOLD && touchy - this.touchStartY > GESTURE_Y_COORD_THRESHOLD) {
            keyboardService.switchToCompressed();
        }
        */
        return false;
    }
	
	/**
	 * Pass a {@link TouchEvent} off to the current {@link PopupKeyboardView} and let it process it
	 * @param event the event that occured
	 * @return the result of the event
	 */
	protected boolean popupTouchEvent(MotionEvent event) {
		// pass any event on to the popup keyboard view 
    	//int [] kbPopupLoc = new int[2];
		//int [] kbLoc = new int[2];

        //IKnowUKeyboardService.log(Log.VERBOSE, "popupTouchEvent", "event.x = "+event.getRawX()+", event.y = "+event.getRawY());

		//kbPopupLoc[0] = kbPopupLoc[1] = -1;
		//kbLoc[0] = kbLoc[1] = -1;
		
    	//keyboardService.popupManager.getPopupKeyboardView().getLocationOnScreen(kbPopupLoc);
        //this.getLocationOnScreen(kbLoc);
    	int action = event.getAction();

        int popupLocx = keyboardService.popupManager.popupLocX;
        int popupLocy = keyboardService.popupManager.popupLocY;

        //kbLoc[1] += this.getSuggestionsView().getHeight();
        //IKnowUKeyboardService.log(Log.DEBUG, "popupTouchEvent", "loc.x = "+popupLocx+", loc.y = "+popupLocy);
        //IKnowUKeyboardService.log(Log.DEBUG, "popupTouchEvent", "kb.x = "+kbLoc[0]+", kb.y = "+kbLoc[1]);

		event.offsetLocation( /*kbLoc[0] - */-popupLocx, /*kbLoc[1] - */-popupLocy );
		
    	if (action == android.view.MotionEvent.ACTION_MOVE && !keyboardService.popupManager.getPopupKeyboardView().hasSeenDownEvent()) {
    		keyboardService.popupManager.getPopupKeyboardView().setSeenDownEvent(true);
            this.invalidate();
    		return true;
    	}
    	//hack to work around keys staying highlighted after releasing from a popup keyboard
    	
    	//if this is an action up then we are not going to return from here, but instead go through the full
    	//motions of a touch event, only to pass on that the InputConnection should not
    	//do anything with the following onkey event that it receives
    	keyboardService.popupManager.getPopupKeyboardView().onTouchEvent(event);
    	if (action == android.view.MotionEvent.ACTION_UP) {
    		//Log.d("SETTING RELEASE WITH NO CONSEQUENCE", "TO TRUE");
    		this.releaseWithNoConsequence = true;
    		return false;
    	} else {
            this.invalidate();
            return true;
    	}
	}
	
	/**
	 * Pass a {@link TouchEvent} off to the current {@link SearchList} and let it process it
	 * @param event the event to be processed
	 * @return the result of the event
	 */
	private boolean searchModeTouchEvent(MotionEvent event) {
		boolean handled = this.searchList.onTouch(event);
		this.invalidate();
		return handled;
	}
	
	/**
	 * Returns a key from the keyboard by searching its coordinates.
	 * 
	 * @param xpos the x-coord of the touch
	 * @param ypos the y-coord of the touch
	 * @return the key that was touched, or null if none
	 */
	private Key findKeyByCoords(int xpos, int ypos) {
		List<Key> keys = getKeyboard().getKeys();			
		Iterator<Key> it = keys.iterator();
		
		while(it.hasNext()) {
			Key key = it.next();
			boolean trueX = false;
			boolean trueY = false;
			
			if (key.x <= xpos && xpos <= (key.x+key.width)) trueX = true;
			if (key.y <= ypos && ypos <= (key.y+key.height)) trueY = true;
			
			if(trueX && trueY) return key;
		}
		return null;
	}
	
	/**
	 * Handle a back button press in the {@link SearchList}
	 */
	private void handleSearchBackButtonPressed(int textMode) {
		switch (textMode) {
            case Suggestion.TEXT_MODE_LATIN:
                this.handleSearchBackButtonLatin();
                break;
            case Suggestion.TEXT_MODE_KOREAN:
                this.handleSearchBackButtonKorean();
                break;
            default:
                this.handleSearchBackButtonLatin();
                break;
        }
	}

    private void handleSearchBackButtonLatin() {
        try {
            this.onkey = true;
            //keyboardService.commitCurrentWord();
            mLongPressDeleteWordsEngaged = true;
            mHandler.removeMessages(MSG_SEARCH_BACKBUTTON);

            //String currentText = IKnowUKeyboardService.getPredictionEngine().setWordInfo();
            String currentText = IKnowUKeyboardService.getPredictionEngine().getRootText();
            IKnowUKeyboardService.log(Log.DEBUG, "BACK BUTTON ROOT =", ""+currentText);

            IKnowUKeyboardService.log(Log.VERBOSE, "handle search back button", "stack.size = "+this.searchList.searchListStack.size());

            if (this.searchList.searchListStack.size() > 0) {
                InputConnection ic = keyboardService.getCurrentInputConnection();
                if (ic != null) {
                    ic.beginBatchEdit();
                    //TODO: fix this in new engine so that we use the undoWordORLetter function again
                    //String textAfterDelete = IKnowUKeyboardService.getPredictionEngine().undoWordOrLetter();
                    String pop = this.searchList.searchListStack.remove(this.searchList.searchListStack.size()-1);
                    IKnowUKeyboardService.log(Log.DEBUG, "pop the stack returns =", "|"+pop+"|");

                    //int goTo = pop.contains(" ") ? pop.length() : pop.length();

                    int start = 0;
                    int finish = 0;

                    if (keyboardService.getCurrentWord().length() > 0) {
                        start = keyboardService.getCurrentWord().length() - pop.length();
                        finish = keyboardService.getCurrentWord().length();
                        keyboardService.deleteFromCurrentWord(start, finish, true);
                    } else {
                        ic.deleteSurroundingText(pop.length(), 0);
                    }

                    if (pop.equals(" ")) {
                    	IKnowUKeyboardService.getPredictionEngine().backspaceLetter();
                    } else {
                        //TODO: this is incorrect behaviour, need to have ability to undo last addition, be it character or word
                    	IKnowUKeyboardService.getPredictionEngine().eraseLastWord();
                        String lastword = keyboardService.getLastWord(ic, false, true);
                        if (lastword != null && lastword.length() > 0) {
                            this.searchList.capsFirst = Character.isUpperCase(lastword.charAt(0));
                        }

                        IKnowUKeyboardService.getPredictionEngine().advanceWord(lastword, false);
                    }
                    //keyboardService.updatePredictionEngineOnCursorPosition(true, false);

                    keyboardService.numNextWords = IKnowUKeyboardService.getPredictionEngine().getNumNextWords();

                    IKnowUKeyboardService.log(Log.VERBOSE, "back button", "root text = |"+IKnowUKeyboardService.getPredictionEngine().getRootText()+"|");
					/*
					if (IKnowUKeyboardService.getPredictionEngine().getRootWordNextWords().length() < 1 || keyboardService.numNextWords <= 0) {
						keyboardService.updatePredictionEngineOnCursorPosition(true, true);
					}
					*/
                    ic.endBatchEdit();
                }
            }

            currentText = IKnowUKeyboardService.getPredictionEngine().getRootText();
            IKnowUKeyboardService.log(Log.VERBOSE, "back button", "current text = |"+currentText+"|");
            keyboardService.audioAndClickFeedback(1);

            if (currentText != null && currentText.length() > 0) {
                //IKnowUKeyboardService.getPredictionEngine().setWordInfo();
                this.searchList.searchBackButtonPressed = true;
                this.searchList.deadZoneStack.remove(this.searchList.deadZoneStack.size() - 1);
                switchToSearchListKeyboard(this.searchList.backButtonX, this.searchList.backButtonY, 32, null);
            }
        } catch (Exception e) {
            IKnowUKeyboardService.sendErrorMessage(e);
        }
    }

    private void handleSearchBackButtonKorean() {
        try {
            this.onkey = true;
            //keyboardService.commitCurrentWord();
            mLongPressDeleteWordsEngaged = true;
            mHandler.removeMessages(MSG_SEARCH_BACKBUTTON);

            //String currentText = IKnowUKeyboardService.getPredictionEngine().setWordInfo();
            String currentText = IKnowUKeyboardService.getPredictionEngine().getRootText();
            IKnowUKeyboardService.log(Log.DEBUG, "BACK BUTTON ROOT =", ""+currentText);

            IKnowUKeyboardService.log(Log.VERBOSE, "handle search back button", "stack.size = "+this.searchList.searchListStack.size());

            if (this.searchList.searchListStack.size() > 0) {
                InputConnection ic = keyboardService.getCurrentInputConnection();
                if (ic != null) {
                    ic.beginBatchEdit();
                    //TODO: fix this in new engine so that we use the undoWordORLetter function again
                    //String textAfterDelete = IKnowUKeyboardService.getPredictionEngine().undoWordOrLetter();
                    String pop = this.searchList.searchListStack.remove(this.searchList.searchListStack.size()-1);
                    IKnowUKeyboardService.log(Log.DEBUG, "pop the stack returns =", "|"+pop+"|");

                    int start = 0;
                    int finish = 0;

                    if (keyboardService.getCurrentWord().length() > 0) {

                        String curDecomp = Hangul.decompose(keyboardService.getCurrentWord());
                        IKnowUKeyboardService.log(Log.VERBOSE, "handleSearchBackButtonKorean()", "curDecomp = "+curDecomp+", pop = "+pop);
                        finish = curDecomp.length() - pop.length();
                        IKnowUKeyboardService.log(Log.VERBOSE, "handleSearchBackButtonKorean()", "curDecomp.length = "+curDecomp.length()+", pop.length = "+pop.length()+", finish = "+finish);
                        String curText = Hangul.combineChars(curDecomp.substring(0, finish), false);

                        keyboardService.deleteFromCurrentWord(0, keyboardService.getCurrentWord().length(), true);
                        keyboardService.addToCurrentWord(ic, curText, true);
                    } else {
                        boolean deleteDone = false;
                        int charsDeleted = 0;
                        String text = "";
                        Hangul curHangul = new Hangul();
                        while (!deleteDone) {

                            text = (String) ic.getTextBeforeCursor(1, 0);
                            if (text != null && text.length() > 0) {

                                curHangul = Hangul.split( text.charAt(0) );

                                if (curHangul.hasFinal()) charsDeleted++;
                                if (curHangul.hasMedial()) charsDeleted++;
                                if (curHangul.hasInitial()) charsDeleted++;

                                if (charsDeleted <= pop.length()) {
                                    ic.deleteSurroundingText(1, 0);
                                }

                            } else {
                                deleteDone = true;
                            }

                            if (charsDeleted >= pop.length()) {
                                deleteDone = true;
                            }
                        }
                    }

                    IKnowUKeyboardService.getPredictionEngine().eraseLastWord();

                    String lastword = Hangul.decompose(keyboardService.getLastWord(ic, false, true));

                    IKnowUKeyboardService.getPredictionEngine().advanceWord(lastword, false);

                    keyboardService.numNextWords = IKnowUKeyboardService.getPredictionEngine().getNumNextWords();
                    IKnowUKeyboardService.log(Log.VERBOSE, "back button", "root text = |"+IKnowUKeyboardService.getPredictionEngine().getRootText()+"|");
                    ic.endBatchEdit();
                }
            }

            currentText = IKnowUKeyboardService.getPredictionEngine().getRootText();
            IKnowUKeyboardService.log(Log.VERBOSE, "back button", "current text = |"+currentText+"|");
            keyboardService.audioAndClickFeedback(1);

            if (currentText != null && currentText.length() > 0) {
                //IKnowUKeyboardService.getPredictionEngine().setWordInfo();
                this.searchList.searchBackButtonPressed = true;
                this.searchList.deadZoneStack.remove(this.searchList.deadZoneStack.size() - 1);
                switchToSearchListKeyboard(this.searchList.backButtonX, this.searchList.backButtonY, 32, null);
            }
        } catch (Exception e) {
            IKnowUKeyboardService.sendErrorMessage(e);
        }
    }
	
	/**
	 * Switch out of search mode and back to regular typing
	 */
	public void switchSearchModeOff() {
		try {
            IKnowUKeyboardService.log(Log.VERBOSE, "switchSearchModeOff()", "start func");
			this.mHandler.removeMessages(MSG_SEARCH_SCROLL);
			
			mSearchMode = false;
			this.searchList.searchBackButtonPressed = false;
            this.searchList.searchSpaceButtonPressed = false;
			
			this.searchList.lastDiffY = 0;
			this.searchList.deadZoneStack.clear();
			this.searchList.searchListStack.clear();
			
			//this.releaseWithNoConsequence = true;
			//keyboardService.commitCurrentWord(keyboardService.getCurrentInputConnection(), "switchSearchModeOff");
			keyboardService.setCandidatesViewShown(true);
			keyboardService.setCurrentWord(keyboardService.getCurrentInputConnection());
			keyboardService.updateAutoCorrectOnCursorPosition();
			//keyboardService.updatePredictionEngineOnCursorPosition(true, false);
			keyboardService.updateCandidates();
			
			InputConnection ic = keyboardService.getCurrentInputConnection();
			String last = (String) ic.getTextBeforeCursor(1, 0);
			
			if (last != null && last.equals(" ")) {
				this.releaseWithNoConsequence = true;
			}

			//keyboardService.updatePredictionEngineOnCursorPosition();
			//keyboardService.updateCandidates();
			
			invalidate();
		} catch (Exception e) {
			IKnowUKeyboardService.sendErrorMessage(e);
		}
	}
	
	/**
	 * Handle a long-press on a key. Will show any {@link PopupKeyboard}s linked to this key
	 * @param key the key that was long-pressed
	 * @return the result of the long-press
	 */
	public boolean handleLongPress(Key key, int touchX, int touchY) {
		try {
			//Log.d("LONG PRESS", String.format("IKnowUKeyboardView onLongPress  key.codes[0] = %d", key.codes[0]));
			
			if (key != null && key.codes != null) {
				this.onkey = true;
				
				if (key.codes[0] == IKnowUKeyboard.KEYCODE_CANCEL) {
					stopDeleteEntireWordFromEditor();
					//onKey(KEYCODE_OPTIONS, null);
                    onKey(key);
					return true;
				} else {
					if (key.codes[0] == IKnowUKeyboard.KEYCODE_DELETE || key.codes[0] == -5) { //keycodes for delete
						deleteEntireWordFromEditor();
					} else {
						// stop deletion of entire word if accidentally sliding over
						stopDeleteEntireWordFromEditor();
					}
					
					if (key.codes[0] == 32) {
						if (this.previewPopup.isShowing()) {
			        		this.previewPopup.dismiss();
			        	}
						this.searchList.startText = new String(IKnowUKeyboardService.getPredictionEngine().getRootWordNextWords());
						this.searchList.setStartX(lastTouchX);
						this.searchList.setStartY(lastTouchY);
						switchToSearchListKeyboard(lastTouchX, lastTouchY, key.codes[0], null);
					}/* else if (key.codes[0] == IKnowUKeyboard.KEYCODE_SWITCH_TO_PICKER) {
                        this.inKeyboardPicker = true;
                        this.switchKeyboard(KeyboardLinearLayout.KEYBOARD_PICKER);
                    } else if (key.codes[0] <= IKnowUKeyboard.KEYCODE_SWITCH_TO_QWERTY && key.codes[0] >= IKnowUKeyboard.KEYCODE_SWITCH_TO_KOREAN) {
                        this.inKeyboardPicker = true;
                        this.switchKeyboard(KeyboardLinearLayout.POPUP_MENU);
                    } */else if (key.codes[0] != IKnowUKeyboard.KEYCODE_DELETE || key.codes[0] != -5) {
						//turn off the pager
						// if it has character it pops something up,
						// if not, nothing pops up and subsequently the long press returns false as well
						if (this.previewPopup.isShowing()) {
			        		this.previewPopup.dismiss();
			        	}
						mPopupKeyboardActive = keyboardService.popupManager.showPopup(this, key, this.getWidth(), touchX, touchY);
						this.invalidate();
					}
					return true;
				}
			} else {
				return false;
			}
		} catch (Exception e) {
			IKnowUKeyboardService.sendErrorMessage(e);
			return false;
		}
	}
	
	/**
	 * Add text to the {@link SearchList} to advance it forward and produce new predictions
	 * @param textToAdd the text to add
	 */
	public void addTextFromSearchList(String textToAdd, int textMode) {

        switch(textMode) {
            case Suggestion.TEXT_MODE_LATIN:
                this.addTextFromSearchListLatin(textToAdd);
                break;
            case Suggestion.TEXT_MODE_KOREAN:
                this.addTextFromSearchListKorean(textToAdd);
                break;
            default:
                this.addTextFromSearchListLatin(textToAdd);
                break;
        }
	}

    private void addTextFromSearchListLatin(String text) {
        try {
            this.onkey = true;

            keyboardService.audioAndClickFeedback(1);
            //String root = IKnowUKeyboardService.getPredictionEngine().getRootWordNextWords();
            int delete = IKnowUKeyboardService.getPredictionEngine().advanceWord(text, false);

            String bufferedText = IKnowUKeyboardService.getPredictionEngine().getAdvanceWordPrintBuffer(text);

            IKnowUKeyboardService.log(Log.VERBOSE, "addTextFromSearchListLatin()", "text = |"+text+"|, bufferedText = |"+bufferedText+"|");

            InputConnection ic = keyboardService.getCurrentInputConnection();
            ic.beginBatchEdit();
            keyboardService.numNextWords = IKnowUKeyboardService.getPredictionEngine().getNumNextWords();
            //String curWord = IKnowUKeyboardService.getPredictionEngine().setWordInfo();
            //Log.d("ADD FROM SEARCH LIST WORD INFO =", "|"+curWord+"|");
            //Log.d("ROOT WORD NEXT WORDS = ", ""+IKnowUKeyboardService.getPredictionEngine().getRootWordNextWords());

            //Log.d("ROOT NEXT WORDS =", ""+IKnowUKeyboardService.getPredictionEngine().getRootWordNextWords()+"|");
            //InputConnection ic = keyboardService.getCurrentInputConnection();
            if (keyboardService.getCurrentWord().length() > 0) {
                keyboardService.commitCurrentWord(ic, "addFromSearchList");
            }
            String cur = (String) ic.getTextBeforeCursor(delete, 0);
            IKnowUKeyboardService.log(Log.VERBOSE, "Add from search list", "delete = "+delete+", beforeCur = |"+cur+"|");
            if (delete > bufferedText.length()) {
                cur = bufferedText;
            } else {
                cur = bufferedText.substring(delete);
            }
            IKnowUKeyboardService.log(Log.VERBOSE, "Add from search list", "adding = |"+cur+"| to stack");
            this.searchList.searchListStack.add(cur);

            ic.deleteSurroundingText(delete, 0);

            //add the text to the input
            if (keyboardService.capitalizeFirstLetter || this.searchList.capsFirst) {
                char[] chrs = bufferedText.toCharArray();
                chrs[0] = Character.toUpperCase(chrs[0]);
                bufferedText = new String(chrs);
            }

            this.searchList.capsFirst = false;

            ic.commitText(bufferedText, 1);
            //keyboardService.addToCurrentWord(bufferedText);

            //the engine is out of predictions, reset it
            if (cur.contains(" ")) {
                //keyboardService.commitCurrentWord(ic, "addToSearchList");
                keyboardService.capitalizeFirstLetter = false;
                //keyboardService.updatePredictionEngineOnCursorPosition(true, true);
            }
            IKnowUKeyboardService.log(Log.VERBOSE, "Add from search list", "capitalizeFirst = "+keyboardService.capitalizeFirstLetter);
            ic.endBatchEdit();
        } catch (Exception e) {
            IKnowUKeyboardService.sendErrorMessage(e);
        }
    }

    private void addTextFromSearchListKorean(String textToAdd) {
        try {
            this.onkey = true;

            keyboardService.audioAndClickFeedback(1);

            InputConnection ic = keyboardService.getCurrentInputConnection();
            ic.beginBatchEdit();
            //String root = IKnowUKeyboardService.getPredictionEngine().getRootWordNextWords();
            int deleteLetters = IKnowUKeyboardService.getPredictionEngine().advanceWord(textToAdd, false);
            textToAdd = IKnowUKeyboardService.getPredictionEngine().getAdvanceWordPrintBuffer(textToAdd);

            IKnowUKeyboardService.log(Log.VERBOSE, "addTextFromSearchListKorean()", "delete letters = "+deleteLetters+", pred = "+textToAdd);

            boolean deleteDone = false;
            int charsDeleted = 0;
            String text = "";
            Hangul curHangul = new Hangul();
            while (!deleteDone) {

                text = (String) ic.getTextBeforeCursor(1, 0);
                if (text != null && text.length() > 0) {

                    curHangul = Hangul.split( text.charAt(0) );

                    if (curHangul.hasFinal()) charsDeleted++;
                    if (curHangul.hasMedial()) charsDeleted++;
                    if (curHangul.hasInitial()) charsDeleted++;

                    if (charsDeleted <= deleteLetters) {
                        keyboardService.deleteFromCurrentWord(keyboardService.currentWord.length() - 1, keyboardService.currentWord.length(), true);
                    }

                } else {
                    deleteDone = true;
                }

                if (charsDeleted >= deleteLetters) {
                    deleteDone = true;
                }
            }
            String addPred = Hangul.combineChars(textToAdd, true);

            keyboardService.addToCurrentWord(ic, addPred, true);
            this.searchList.searchListStack.add(textToAdd.substring(deleteLetters, textToAdd.length()));

            this.searchList.capsFirst = false;
            ic.endBatchEdit();
        } catch (Exception e) {
            IKnowUKeyboardService.sendErrorMessage(e);
        }
    }

    /**
     * Add text to the {@link SearchList} to advance it forward and produce new predictions
     * @param textToAdd the text to add
     */
    public void handleSearchListSpace(int textMode) {

        switch(textMode) {
            case Suggestion.TEXT_MODE_LATIN:
                this.handleSearchListSpaceLatin();
                break;
            case Suggestion.TEXT_MODE_KOREAN:
                this.handleSearchListSpaceKorean();
                break;
            default:
                this.handleSearchListSpaceLatin();
                break;
        }
    }

    private void handleSearchListSpaceLatin() {
        try {
            mHandler.removeMessages(MSG_SEARCH_SPACEBUTTON);

            this.onkey = true;

            keyboardService.audioAndClickFeedback(1);

            InputConnection ic = keyboardService.getCurrentInputConnection();

            int deleteLetters = 0;
            String charsToAdd = "";
            deleteLetters = IKnowUKeyboardService.getPredictionEngine().advanceLetter(' ', false, false);
            charsToAdd = IKnowUKeyboardService.getPredictionEngine().getAdvanceLetterPrintBuffer(" ");

            //if we need to get rid of any extra spaces or whatnot, then do it here.
            if (deleteLetters > 0) {
                keyboardService.deleteFromCurrentWord(keyboardService.currentWord.length() - deleteLetters, keyboardService.currentWord.length(), false);
            }

            keyboardService.numNextWords = IKnowUKeyboardService.getPredictionEngine().getNumNextWords();
            //this needs to be after we try to do a correction
            //log(Log.VERBOSE, "handleWordSepLatin()", "printBuffer = |"+charsToAdd+"|");

            if (charsToAdd == null) {
                charsToAdd = " ";
            }

            this.searchList.searchListStack.add(charsToAdd);

            keyboardService.addToCurrentWord(ic, charsToAdd, true);
            keyboardService.commitCurrentWord(ic, "handleSearchListSpaceLatin");

            ic.endBatchEdit();

/*            String currentText = IKnowUKeyboardService.getPredictionEngine().getRootText();
            IKnowUKeyboardService.log(Log.VERBOSE, "space button", "current text = |"+currentText+"|");

            if (currentText != null && currentText.length() > 0) {
                //IKnowUKeyboardService.getPredictionEngine().setWordInfo();*/
            this.searchList.capsFirst = false;
            keyboardService.capitalizeFirstLetter = false;
            this.searchList.searchSpaceButtonPressed = true;
            //this.searchList.deadZoneStack.remove(this.searchList.deadZoneStack.size() - 1);
            switchToSearchListKeyboard(this.searchList.spaceButtonX, this.searchList.spaceButtonY, 32, null);
            //}
        } catch (Exception e) {
            IKnowUKeyboardService.sendErrorMessage(e);
        }
    }

    private void handleSearchListSpaceKorean() {
        try {
            mHandler.removeMessages(MSG_SEARCH_SPACEBUTTON);

            InputConnection ic = keyboardService.getCurrentInputConnection();
            ic.beginBatchEdit();

            int deleteLetters = 0;
            String charsToAdd = "";
            deleteLetters = IKnowUKeyboardService.getPredictionEngine().advanceLetter( ' ', false, false);
            charsToAdd = IKnowUKeyboardService.getPredictionEngine().getAdvanceLetterPrintBuffer(" ");

            //if we need to get rid of any extra spaces or whatnot, then do it here.
            //log(Log.VERBOSE, "handleWordSepLatin()", "deleteLetters = "+deleteLetters);
            if (deleteLetters > 0) {
                keyboardService.deleteFromCurrentWord(keyboardService.currentWord.length() - deleteLetters, keyboardService.currentWord.length(), false);
            }

            keyboardService.numNextWords = IKnowUKeyboardService.getPredictionEngine().getNumNextWords();
            //this needs to be after we try to do a correction
            //log(Log.VERBOSE, "handleWordSepLatin()", "printBuffer = |"+charsToAdd+"|");

            if (charsToAdd == null) {
                charsToAdd = " ";
            }

            keyboardService.addToCurrentWord(ic, charsToAdd, true);
            keyboardService.commitCurrentWord(ic, "handleWordSepKorean");

            ic.endBatchEdit();

            keyboardService.previousHangul = Hangul.copy(keyboardService.currentHangul);
            keyboardService.currentHangul = new Hangul();

/*          String currentText = IKnowUKeyboardService.getPredictionEngine().getRootText();
            IKnowUKeyboardService.log(Log.VERBOSE, "space button", "current text = |"+currentText+"|");

            if (currentText != null && currentText.length() > 0) {
                //IKnowUKeyboardService.getPredictionEngine().setWordInfo();*/
            this.searchList.capsFirst = false;
            keyboardService.capitalizeFirstLetter = false;
            this.searchList.searchSpaceButtonPressed = true;
            //this.searchList.deadZoneStack.remove(this.searchList.deadZoneStack.size() - 1);
            switchToSearchListKeyboard(this.searchList.spaceButtonX, this.searchList.spaceButtonY, 32, null);
            //}
        } catch (Exception e) {
            IKnowUKeyboardService.sendErrorMessage(e);
        }
    }
	
	/**
	 * This function will be used every time we want to show a list of search items. This means that even when
	 * we are hovering over something in the search list, we will call this again to bring up more words
	 * 
	 * @param touchX the x-coord of the event that triggered this
	 * @param touchY the y-coord of the event that triggered this
	 * @param primaryCode the key that was used to switch into this
	 * @param selectedText the text to add to the {@link SearchList}
	 */
	public void switchToSearchListKeyboard(int touchX, int touchY, int primaryCode, String selectedText) {
		try {
			if (!keyboardService.getTrialExpired()) {
				//we need to check the status of our root again
				String rootText = IKnowUKeyboardService.getPredictionEngine().getRootWordNextWords();
                IKnowUKeyboardService.log(Log.VERBOSE, "switchToSearchList()", "root = |"+rootText+"|"+" ][ numNextWords = "+keyboardService.numNextWords);
				//if the root is non-existant, then we can't move to searchlist mode
				if ( keyboardService.numNextWords > 0 || (rootText != null && rootText.length() > 0) ) {
					
					//if we aren't in search mode yet, then set up initial params etc.
					if (mSearchMode == false) {
						if (primaryCode != 0) {
							// We now only switch to searchlist mode
							// by holding space, therefore we don't want to deliver a space,
							// as a space will return nothing from the engine.

                            IKnowUKeyboardService.log(Log.VERBOSE, "switchToSearchList()", "numNextWords = "+keyboardService.numNextWords);
							if (keyboardService.numNextWords <= 0 && primaryCode != 32) {
								//onRelease(primaryCode);
								return;
							}
						}
						mSearchMode = true;
						//deadZoneKeyIndex = 0;
						this.searchList.deadZoneStack.add(1);
					}
					
					//if text has been passed in, this means we are calling this from another search item
					//and in this case some special cases need to be taken care of
					if (selectedText != null && selectedText.length() > 0) {

                        int textMode = Suggestion.TEXT_MODE_LATIN;
                        switch (IKnowUKeyboardView.keyboardService.currentKeyboardLayout) {
                            case KeyboardLinearLayout.KOREAN:
                                textMode = Suggestion.TEXT_MODE_KOREAN;
                                break;
                            default:
                                textMode = Suggestion.TEXT_MODE_LATIN;
                                break;
                        }

						this.addTextFromSearchList(selectedText, textMode);
					}
					
					//get the suggestions for the next words from the engine
					String[] wlpredictions = IKnowUKeyboardService.getPredictionEngine().getSuggestions();
					
					//boolean hasPredictions = searchList.setPredictions(wlpredictions);
					//this.searchList.clearList(wlpredictions.length + 1);
					
					//if (!hasPredictions) {
						//this.switchSearchModeOff();
					//} else {
						this.searchList.rootText = IKnowUKeyboardService.getPredictionEngine().getRootText();
						this.searchList.animationStartx = touchX;
						this.searchList.animationStarty = touchY;
						
						this.searchList.setPredictions(wlpredictions);
						
						IKnowUKeyboardService.log(Log.VERBOSE, "Switch to serach mode", "back x = "+this.searchList.backButtonX+", back y = "+this.searchList.backButtonY);
						
						animStartTime = System.currentTimeMillis();
						//call invalidate so the keyboard knows to redraw itself
						invalidate();
					//}//end if haspredictions
				}//end if root text is present
			}// end if not trial expired
		} catch (Exception e) {
			IKnowUKeyboardService.sendErrorMessage(e);
		}
	}

    /**
     * Set that a key has been pressed down
     * @param key that has been pressed
     */
    public void onPressKey(Key key) {
        keyboardService.audioAndClickFeedback(1);
        key.pressed = true;
    }

    /**
     * Set that a key has been released, and send it to the editor
     * @param key the key that has been released
     */
    public void onReleaseKey(Key key) {
        //keyboardService.startTimer();
        key.pressed = false;
        this.onKey(key);

        mHandler.removeMessages(MSG_LONGPRESS);
        mHandler.removeMessages(MSG_DELETEWORDFROMEDITOR);
        mHandler.removeMessages(MSG_LONGPAUSE);
        mHandler.removeMessages(MSG_CHECK_CURSOR_UPDATES);

        if (this.onkey) {
            IKnowUKeyboardService.log(Log.VERBOSE, "IKnowUKeyboardView", "onReleaseKey(), this.onKey = true, key.codes[0] = "+key.codes[0]);
            this.prepareNextKeyHighlighting();

            if (!this.ignoreShift) {
                keyboardService.updateShiftKeyState(keyboardService.getCurrentInputEditorInfo());
            }

            this.ignoreShift = false;

            keyboardService.updateWordToCorrect();

            if (this.updatePredEngineOnRelease) {
                this.updatePredEngineOnRelease = false;
                keyboardService.updatePredictionEngineOnCursorPosition(false, true);
                keyboardService.updateAutoCorrectOnCursorPosition();
            }

            keyboardService.updateCandidates();
            this.invalidate();
        }

        //keyboardService.stopTimer("onReleaseKey()");
        if (!mHandler.hasMessages(MSG_CHECK_CURSOR_UPDATES)) {
            Message msg = mHandler.obtainMessage(MSG_CHECK_CURSOR_UPDATES);
            mHandler.sendMessageDelayed(msg, UPDATE_INTERVAL);
        }
    }

	/**
	 * Pass a key that has been pressed off to the {@link IKnowUKeyboardService}
     *
     * This will assume default values for any characters on the key and pass those to the engine
     *
	 * @param key the key that was pressed
	 */
	public void onKey(Key key) {
		//Log.d("ON KEY CODES LENGTH =", ""+keyCodes.length);
		//try {
		this.onkey = true;

        int primaryCode = key.codes[0];
		
		//skip everything else if this is true,
		//this means we are in the onkey of an action_up after
		//exiting search mode, or a popup
		if (this.releaseWithNoConsequence) {
			this.releaseWithNoConsequence = false;
		}
		else if ( keyboardService.isWordSeparator((char) primaryCode) ) {
			keyboardService.handleWordSeparator(primaryCode);
		} else if (primaryCode == -1) {
			keyboardService.handleShift();
		} else if (primaryCode == -2) {
			switchKeyboard(primaryCode);
		} else if (primaryCode == -3) {
			if (!keyboardService.settingsOpen) keyboardService.handleClose();
		} else if (primaryCode == -5) {
			if (!longPressDeleteEntireWordEngaged()) {
				keyboardService.handleBackspace();
			}
			keyboardService.updateShiftKeyState(keyboardService.getCurrentInputEditorInfo());
		} else if (primaryCode == -6 || primaryCode == -7 || primaryCode == -8 || primaryCode == -9 || primaryCode == -10 || primaryCode == -11 || primaryCode == -13
                || primaryCode == -14 || primaryCode == -15 || primaryCode == -16 || primaryCode == -17 || primaryCode == -18 || primaryCode == -19
                || primaryCode == -30 || primaryCode == -31 || primaryCode == -32) {
            switchKeyboard(primaryCode);
        } else if (primaryCode == -12) {
            keyboardService.switchToVoiceInput();
        } else if (primaryCode == -20) {
			keyboardService.sendDownUpKeyEvents(KeyEvent.KEYCODE_DPAD_UP);
		} else if (primaryCode == -21) {
			keyboardService.sendDownUpKeyEvents(KeyEvent.KEYCODE_DPAD_LEFT);
		} else if (primaryCode == -22) {
			keyboardService.sendDownUpKeyEvents(KeyEvent.KEYCODE_DPAD_DOWN);
		} else if (primaryCode == -23) {
			keyboardService.sendDownUpKeyEvents(KeyEvent.KEYCODE_DPAD_RIGHT);
		} else if (primaryCode == -24) {
			keyboardService.sendDownUpKeyEvents(0xff50); 
		} else if (primaryCode == -36) {
			keyboardService.jumpWordInEditor(false);
		} else if (primaryCode == -37) {
			keyboardService.jumpWordInEditor(true);
		} else if (primaryCode == -25) {
//					sendDownUpKeyEvents( 122); //KEYCODE_MOVE_HOME); 
			keyboardService.sendKey(0xff57); 
		} else if (primaryCode == IKnowUKeyboardService.KEYCODE_OPTIONS) {
			//keyboardService.settingMenu();
		} else if (primaryCode == -50) {
			keyboardService.copy();
		} else if (primaryCode == -51) {
			keyboardService.cut();
		} else if (primaryCode == -52) {
			keyboardService.paste();
		} else if (primaryCode == -53) {
			keyboardService.selectAll();
        } else if (primaryCode == -44) {
            onText(key.text);
		} else {
            byte[] prefs = new byte[key.codes.length];

            for (int i = 0; i < key.codes.length; i++) {
                prefs[i] = 1;
            }

            String text = "";
            IKnowUKeyboardService.log(Log.VERBOSE, "IKnowUKeyboardView.onKey()", "isShifted = "+this.keyboard.isShifted()+", labelUpperCase = "+key.labelUpperCase);
            if ( this.keyboard.isShifted() && key.labelUpperCase != null ) {
                text = key.labelUpperCase.toString().replace(" ", "");
                keyboardService.handleCharacter(text, prefs, key.upperCharInt);
            }
            else if (key.label != null) {
                text = key.label.toString().toLowerCase().replace(" ", "");
                keyboardService.handleCharacter(text, prefs, key.codes[0]);
            } else {
                keyboardService.handleCharacter(text, prefs, key.codes[0]);
            }
		}
		
		stopDeleteEntireWordFromEditor();
	}

    /**
     * Pass a key that has been pressed off to the {@link IKnowUKeyboardService}
     * @param key the key that was pressed
     */
    public void onKey(String chars, byte[] prefs, int primaryCode) {
        //Log.d("ON KEY CODES LENGTH =", ""+keyCodes.length);
        //try {
        this.onkey = true;

        //skip everything else if this is true,
        //this means we are in the onkey of an action_up after
        //exiting search mode, or a popup
        if (this.releaseWithNoConsequence) {
            this.releaseWithNoConsequence = false;
        }
        else if ( keyboardService.isWordSeparator((char) primaryCode) ) {
            keyboardService.handleWordSeparator(primaryCode);
        } else if (primaryCode == -1) {
            keyboardService.handleShift();
        } else if (primaryCode == -2) {
            switchKeyboard(primaryCode);
        } else if (primaryCode == -3) {
            if (!keyboardService.settingsOpen) keyboardService.handleClose();
        } else if (primaryCode == -5) {
            if (!longPressDeleteEntireWordEngaged()) {
                keyboardService.handleBackspace();
            }
            keyboardService.updateShiftKeyState(keyboardService.getCurrentInputEditorInfo());
        } else if (primaryCode == -6 || primaryCode == -7 || primaryCode == -8 || primaryCode == -9 || primaryCode == -10 || primaryCode == -11 || primaryCode == -13
                || primaryCode == -14 || primaryCode == -15 || primaryCode == -16 || primaryCode == -17 || primaryCode == -18 || primaryCode == -19
                || primaryCode == -30 || primaryCode == -31 || primaryCode == -32) {
            switchKeyboard(primaryCode);
        } else if (primaryCode == -12) {
            keyboardService.switchToVoiceInput();
        } else if (primaryCode == 32) {
            //IKnowUKeyboardService.getPredictionEngine().reset(false);
            //if (keyboardService.mCandidateView != null) keyboardService.mCandidateView.clear();
        } else if (primaryCode == -20) {
            keyboardService.sendDownUpKeyEvents(KeyEvent.KEYCODE_DPAD_UP);
        } else if (primaryCode == -21) {
            keyboardService.sendDownUpKeyEvents(KeyEvent.KEYCODE_DPAD_LEFT);
        } else if (primaryCode == -22) {
            keyboardService.sendDownUpKeyEvents(KeyEvent.KEYCODE_DPAD_DOWN);
        } else if (primaryCode == -23) {
            keyboardService.sendDownUpKeyEvents(KeyEvent.KEYCODE_DPAD_RIGHT);
        } else if (primaryCode == -24) {
            keyboardService.sendDownUpKeyEvents(0xff50);
        } else if (primaryCode == -36) {
            keyboardService.jumpWordInEditor(false);
        } else if (primaryCode == -37) {
            keyboardService.jumpWordInEditor(true);
        } else if (primaryCode == -25) {
//					sendDownUpKeyEvents( 122); //KEYCODE_MOVE_HOME);
            keyboardService.sendKey(0xff57);
        } else if (primaryCode == IKnowUKeyboardService.KEYCODE_OPTIONS) {
            //keyboardService.settingMenu();
        } else if (primaryCode == -50) {
            keyboardService.copy();
        } else if (primaryCode == -51) {
            keyboardService.cut();
        } else if (primaryCode == -52) {
            keyboardService.paste();
        } else if (primaryCode == -53) {
            keyboardService.selectAll();
        } else {
            keyboardService.handleCharacter(chars, prefs, primaryCode);
        }

        stopDeleteEntireWordFromEditor();
    }

	/**
	 * switch this keyboardview's current keyboard
	 * @param primaryCode the keycode that was pressed, determines the resulting keyboard
	 */
	public void switchKeyboard(int primaryCode) {

        if (keyboardService.currentKeyboardLayout == KeyboardLinearLayout.COMPRESSED) {
            keyboardService.switchToFullSize();
        }

		if (primaryCode == IKnowUKeyboard.KEYCODE_SWITCH_TO_NUMERIC) {
			keyboardService.setKeyboard(KeyboardLinearLayout.NUMERIC, false, false, this.isSideView);
		} else if (primaryCode == IKnowUKeyboard.KEYCODE_SWITCH_TO_CURRENT) {
			keyboardService.setKeyboard(keyboardService.currentKeyboardLayout, false, false, this.isSideView);
		} else if (primaryCode == IKnowUKeyboard.KEYCODE_SWITCH_TO_EXTRAS) {
			keyboardService.setKeyboard(KeyboardLinearLayout.EXTRA_LETTERS, false, false, this.isSideView);
		} else if (primaryCode == IKnowUKeyboard.KEYCODE_SWITCH_TO_SMILEY) {
			keyboardService.setKeyboard(KeyboardLinearLayout.SMILEY, false, false, this.isSideView);
		} else if (primaryCode == IKnowUKeyboard.KEYCODE_SWITCH_TO_NAVIGATION) {
			keyboardService.setKeyboard(KeyboardLinearLayout.NAVIGATION, false, false, this.isSideView);
		} else if (primaryCode == IKnowUKeyboard.KEYCODE_SWITCH_TO_SYMBOL) {
			keyboardService.setKeyboard(KeyboardLinearLayout.SYMBOLS, false, false, this.isSideView);
		} else if (primaryCode == KeyboardLinearLayout.KEYBOARD_PICKER) {
			keyboardService.setKeyboard(KeyboardLinearLayout.KEYBOARD_PICKER, false, false, false);
		} else if (primaryCode == KeyboardLinearLayout.POPUP_MENU) {
            keyboardService.setKeyboard(KeyboardLinearLayout.POPUP_MENU, false, false, this.isSideView);
        } else if (primaryCode == IKnowUKeyboard.KEYCODE_SWITCH_TO_COMPRESSED) {
            keyboardService.switchToCompressed();
        } else if (primaryCode == IKnowUKeyboard.KEYCODE_SWITCH_TO_FEATURE) {
            keyboardService.setKeyboard(KeyboardLinearLayout.FEATURE, false, false, this.isSideView);
        } else if (primaryCode == IKnowUKeyboard.KEYCODE_SWITCH_TO_QWERTY) {
            keyboardService.setKeyboard(KeyboardLinearLayout.QWERTY, true, false, this.isSideView);
        } else if (primaryCode == IKnowUKeyboard.KEYCODE_SWITCH_TO_AZERTY) {
            keyboardService.setKeyboard(KeyboardLinearLayout.AZERTY, true, false, this.isSideView);
        } else if (primaryCode == IKnowUKeyboard.KEYCODE_SWITCH_TO_QWERTY_SPANISH) {
            keyboardService.setKeyboard(KeyboardLinearLayout.QWERTY_SPANISH, true, false, this.isSideView);
        } else if (primaryCode == IKnowUKeyboard.KEYCODE_SWITCH_TO_QZERTY) {
            keyboardService.setKeyboard(KeyboardLinearLayout.QZERTY, true, false, this.isSideView);
        } else if (primaryCode == IKnowUKeyboard.KEYCODE_SWITCH_TO_QWERTZ) {
            keyboardService.setKeyboard(KeyboardLinearLayout.QWERTZ, true, false, this.isSideView);
        } else if (primaryCode == IKnowUKeyboard.KEYCODE_SWITCH_TO_KOREAN) {
            keyboardService.setKeyboard(KeyboardLinearLayout.KOREAN, true, false, this.isSideView);
        } else if (primaryCode == IKnowUKeyboard.KEYCODE_SWITCH_TO_RUSSIAN) {
            keyboardService.setKeyboard(KeyboardLinearLayout.RUSSIAN, true, false, this.isSideView);
        } else if (primaryCode == IKnowUKeyboard.KEYCODE_SWITCH_MODE) {
            keyboardService.setKeyboard(keyboardService.currentKeyboardLayout, false, true, this.isSideView);

            //switch the padding between keys to be less when in thumb or portrait mode
            Resources r = this.mContext.getResources();
            switch(this.keyboardLinearLayout.getCurrentMode()) {
                case KeyboardLinearLayout.MODE_TABLET_THUMB:
                    this.keyPaddingLR = (int) TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, KEY_PADDING_LR_THUMB, r.getDisplayMetrics());
                    break;
                case KeyboardLinearLayout.MODE_TABLET_FULL:
                    if (keyboardService.mLandscape) {
                        this.keyPaddingLR = (int) TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, KEY_PADDING_LR_FULL, r.getDisplayMetrics());
                    }
                    break;
            }
            this.keyPaddingTB = (int) TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, KEY_PADDING_TB_FULL, r.getDisplayMetrics());
        }
	}

	/**
	 * Send a {@link String} of text to the {@link IKnowUKeyboardService}
	 * 
	 * ie. ".com" ".ca" ".eu" also handles smiley keys
	 * @param text
	 */
	public void onText(CharSequence text) {
		try {
			//Log.d("WLKB", String.format("softkeyboard onText %s", text.toString()));
			InputConnection ic = keyboardService.getCurrentInputConnection();
			if (ic == null)
				return;
			ic.beginBatchEdit();
			keyboardService.commitCurrentWord(ic, "onText");
			ic.commitText(text, 0);
			ic.endBatchEdit();
			keyboardService.updateShiftKeyState(keyboardService.getCurrentInputEditorInfo());
		} catch (Exception e) {
			IKnowUKeyboardService.sendErrorMessage(e);
		}
	}

    public class PossibleChar 
    {
        public char val;
        public boolean set;
        public byte pref;
        public byte count;
        public PossibleChar() {
        	val = 32;
        	set = false;
        	count = 1;
        	pref = 50; // 50 is a default value.
        }
        
        void setPef(byte pef){
        	pref = pef;
       }
        public boolean set(char code) {
        	set = true;
            if (val == (char)code) {
                count++;
                return true;
            } else {
            	val = code;
            	count = 1;
                return false;
            }
        }
     }
}
