package com.iknowu.popup;

import android.content.Context;
import android.content.res.Resources;
import android.content.res.Resources.NotFoundException;
import android.content.res.XmlResourceParser;
import android.graphics.Canvas;
import android.graphics.LinearGradient;
import android.graphics.Paint;
import android.graphics.PorterDuff;
import android.graphics.Rect;
import android.graphics.RectF;
import android.graphics.Shader;
import android.graphics.Typeface;
import android.graphics.drawable.Drawable;
import android.inputmethodservice.KeyboardView;
import android.util.AttributeSet;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;

import com.iknowu.IKnowUKeyboardService;
import com.iknowu.R;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Iterator;

/**
 * An extension of {@link KeyboardView} to display a keyboard in a {@link PopupWindow}
 * 
 * @author Justin Desjardins
 *
 */
public class PopupKeyboardView extends View {
	
	static final int KEYCODE_OPTIONS = -100;
	static final int NUMBER_OF_SEARCHROWS = 6;
	
	private static final int THEME_GRADIENT_DIAGONAL = 0;
	private static final int THEME_GRADIENT_VERTICAL = 1;
	private static final int THEME_GRADIENT_HORIZONTAL = 2;
	
	private static final int THEME_STYLE_REGULAR = 0;
	private static final int THEME_STYLE_OUTLINE = 1;
	private static final int THEME_STYLE_LINESBETWEEN = 2;

    private static final float DOUBLE_ROW_SCALE_FACTOR = 3.4f;

    private static final String ANDROID_NAMESPACE = "http://schemas.android.com/apk/res/android";

	static public IKnowUKeyboardService keyboardService;
	private Context mContext;
	private int[] keyboardLoc = new int[2];
	
	private Paint mPaint;
	
	private Drawable mShiftActive = null; 
	private Drawable mShiftLockActive = null;

	private boolean mSeenDownEvent = false;
	private boolean bKeyHasBeenRepeating = false;

	//private LatinKeyboardView mMiniKeyboardView = null;
	public KeyboardView mMiniKeyboardView = null;
	
	private int selectedKeyIndex = -1;
	private int mPopupDistY = -1;
	private int mKeyTextSize = 1; //34;
	private int mLabelTextSize = 1; //22; // longer than 1 character

    private int mPadLeft = 1;
    private int mPadRight = 1;
    private int mPadTop = 1;
    private int mPadBottom = 1;
	

	int mpkPadTextVertical = 1;
	int mpkPadUprTextRightEdgeIn = 1;
	int mpkPadUprTextTopEdgeBelow = 1;

	int mpkUprIconPadLeft = 1;
	int mpkUprIconPadTop = 1;
	int mpkUprIconPadRight = 1;
	int mpkUprIconPadBottom = 1;
    
	private int keyTextColor;
	private int backgroundColor;
	private int keyColor;
	private int keyPressedColor;
	private int keyStyle;
	private boolean useGradient;
	private int gradientDirection;
	private int keyShadowColor;
	private int cornerRadiusX;
	private int cornerRadiusY;
	private int borderStroke;
	
	private static final int STROKE_WIDTH = 3;
	
	public int popupPadding = 10;	//the padding around the edges for the popupkeys
	private int strokeWidth = 2;

	private static final String TAG_THEME_ITEM = "item";
    
	private int mCharSelectedInPopupColor;

	static int counttt = 0;
	
	public float densityScale;					//passed on from the input service, allows us to draw things to correct sizes on screen
	
	public boolean releaseWithNoConsequence;	//a flag to tell the input service that it should ignore the following key release
	public boolean resizable;					//it this is true then when we measure this view we will know to resize the keys
    public int sensitivityRange;
	
	public ArrayList<PopupKey> popupKeys;
	public int popupWidth;
	public int popupHeight;
    public int popupKeyHeight = 0;
	
	// Keyboard XML Tags
    private static final String TAG_ROW = "Row";
    private static final String TAG_KEY = "Key";

    public boolean isSingleRow;

	public void setSoftKeyboard(IKnowUKeyboardService sb) {
		keyboardService = sb;
	}

    public PopupKeyboardView(Context context) {
        super(context);
        mContext = context;
        //processAttributes(context, attrs, 0);
        if (mPaint == null) mPaint = new Paint();
        mPaint.setAntiAlias(true);
        this.setWillNotDraw(false);
    }
	
	public PopupKeyboardView(Context context, AttributeSet attrs) throws IOException {
		super(context, attrs);
		mContext = context;
		//processAttributes(context, attrs, 0);
		if (mPaint == null) mPaint = new Paint();
		mPaint.setAntiAlias(true);
        this.setWillNotDraw(false);
	}

	public PopupKeyboardView(Context context, AttributeSet attrs, int defStyle) {
		super(context, attrs, defStyle);
		mContext = context;
		//processAttributes(context, attrs, defStyle);
		if (mPaint == null) mPaint = new Paint();
		mPaint.setAntiAlias(true);
        this.setWillNotDraw(false);
	}
	
	/**
	 * Initialize this view, processing all attributes required for viewing it.
	 * 
	 * @param themeId ID of the resource (R.xml) file for the theme
	 */
	public void processAttributes( int themeId ) {
		try {
			Resources r = mContext.getResources();
	        
	        this.setTheme(themeId);
	        
	        mCharSelectedInPopupColor = r.getColor(R.color.popup_char_selected);
	        
			mShiftActive = r.getDrawable(R.drawable.sym_keyboard_shift_1); 
			mShiftLockActive = r.getDrawable(R.drawable.sym_keyboard_shift_lock);

	        mKeyTextSize = r.getDimensionPixelSize(R.dimen.kbview_tablet_keytextsize);
	        mLabelTextSize = r.getDimensionPixelSize(R.dimen.kbview_tablet_labeltextsize);

	        mPadLeft = r.getDimensionPixelSize(R.dimen.kbview_key_padleft);
	        mPadRight = r.getDimensionPixelSize(R.dimen.kbview_key_padright);
	        mPadTop = r.getDimensionPixelSize(R.dimen.kbview_key_padtop);
	        mPadBottom = r.getDimensionPixelSize(R.dimen.kbview_key_padbottom);
	        
	        mpkPadTextVertical = r.getDimensionPixelSize(R.dimen.kbview_key_pkpadtextvertical);
	        mpkPadUprTextRightEdgeIn = r.getDimensionPixelSize(R.dimen.kbview_key_tablet_pkpaduprtextrightedgein);
	        mpkPadUprTextTopEdgeBelow = r.getDimensionPixelSize(R.dimen.kbview_key_pkpaduprtexttopedgebelow);

			mpkUprIconPadLeft = r.getDimensionPixelSize(R.dimen.kbview_key_upriconpadleft);
			mpkUprIconPadTop = r.getDimensionPixelSize(R.dimen.kbview_key_upriconpadright);
			mpkUprIconPadRight = r.getDimensionPixelSize(R.dimen.kbview_key_upriconpadtop);
			mpkUprIconPadBottom = r.getDimensionPixelSize(R.dimen.kbview_key_upriconpadbottom);
			
			this.releaseWithNoConsequence = false;
			this.resizable = false;
			this.densityScale = getResources().getDisplayMetrics().densityDpi;
		} catch (Exception e) {
			IKnowUKeyboardService.sendErrorMessage(e);
		}
	}
	
	/**
	 * Parse the theme file and set the appropriate attributes
	 * 
	 * @param themeId ID of the resource (R.xml) file for the theme
	 */
	public void setTheme( int themeId) {
		XmlResourceParser parser;
		try {
			parser = this.mContext.getResources().getXml(themeId);
		} catch (NotFoundException nfe) {
			parser = this.mContext.getResources().getXml(IKnowUKeyboardService.DEFAULT_KEYBOARD_THEME_ID);
		}
		this.setBackgroundColor(0x00000000);
        try {
            int event;
            while ((event = parser.next()) != XmlResourceParser.END_DOCUMENT) {
                if (event == XmlResourceParser.START_TAG) {
                    String tag = parser.getName();
                    if (TAG_THEME_ITEM.equals(tag)) {
                        String attrName = parser.getAttributeValue(null, "name");
                        
                        if (attrName.equals("keyColor")) {
                        	this.keyColor = parser.getAttributeIntValue(null, "value", 0xFF4f4f4f);
                        } else if (attrName.equals("keyPressedColor")) {
                        	this.keyPressedColor = parser.getAttributeIntValue(null, "value", 0xFFd3d3d3);
                        } else if (attrName.equals("backgroundColor")) {
                        	this.backgroundColor = parser.getAttributeIntValue(null, "value", 0xFF000000);
                        } else if (attrName.equals("keyTextColor")) {
                        	this.keyTextColor = parser.getAttributeIntValue(null, "value", 0xFFFFFFFF);
                        } else if (attrName.equals("style")) {
                        	this.keyStyle = parser.getAttributeIntValue(null, "value", 0);
                        } else if (attrName.equals("stroke")) {
                        	this.borderStroke = parser.getAttributeIntValue(null, "value", 0);
                        } else if (attrName.equals("keyShadowColor")) {
                        	this.keyShadowColor = parser.getAttributeIntValue(null, "value", 0xDD000000);
                        } else if (attrName.equals("useGradient")) {
                        	this.useGradient = parser.getAttributeBooleanValue(null, "value", false);
                        } else if (attrName.equals("gradientDirection")) {
                        	this.gradientDirection = parser.getAttributeIntValue(null, "value", 0);
                        } else if (attrName.equals("cornerRadiusX")) {
                        	this.cornerRadiusX = parser.getAttributeIntValue(null, "value", 0);
                        } else if (attrName.equals("cornerRadiusY")) {
                        	this.cornerRadiusY = parser.getAttributeIntValue(null, "value", 0);
                        }
                   }
                } else if (event == XmlResourceParser.END_TAG) {}
            }
        } catch (Exception e) {
        	IKnowUKeyboardService.sendErrorMessage(e);
        }
	}
	
	/**
	 * Creates the keys for this keyboardview based on the resource layout id given
	 * 
	 * @param xmlLayoutResId the resource to get the keys from
	 * @param columns how many columns of keys should be present in this view
	 * @param maxWidth the maximum width that this view should take up
	 * @param leftToRight whether or not the keys should fill from left to right
	 * @return true on successful creation, false otherwise
	 */
	public boolean createKeyboard(int xmlLayoutResId,  int columns, int maxWidth, boolean leftToRight) {
		XmlResourceParser parser;
		try {
			parser = this.mContext.getResources().getXml(xmlLayoutResId);
		} catch (NotFoundException nfe) {
			parser = null;
		}
		
		if (parser != null) {
			boolean inKey = false;
	        boolean inRow = false;
	        int x = this.popupPadding;
	        int y = this.popupPadding;
	        PopupKey key = null;
	        Resources res = mContext.getResources();
	        
	        float rowWidth = 0;
	        
	        this.popupWidth = 0;
	        this.popupHeight = 0;
	        
	        this.popupKeys = new ArrayList<PopupKey>();
	
	        try {
	            int event;
	            while ((event = parser.next()) != XmlResourceParser.END_DOCUMENT) {
	                if (event == XmlResourceParser.START_TAG) {
	                    String tag = parser.getName();
	                    if (TAG_ROW.equals(tag)) {
	                        inRow = true;
	                        rowWidth = parser.getAttributeFloatValue(null, "iknowuKeyWidth", 0);
	                        x = this.popupPadding;
	                   } else if (TAG_KEY.equals(tag)) {
	                        inKey = true;
	                        key = new PopupKey(mContext, maxWidth);
	                        key.x = x;
	                        key.y = y;
	                        key.label = parser.getAttributeValue(ANDROID_NAMESPACE, "keyLabel");
	                        int iconid = parser.getAttributeResourceValue(ANDROID_NAMESPACE, "keyIcon", -1);
	                        if (iconid != -1) key.icon = res.getDrawable(iconid);
	                        if (key.icon != null) {
	                            key.icon.setBounds(0, 0, key.icon.getIntrinsicWidth(), key.icon.getIntrinsicHeight());
	                        }
	                        
	                        if (rowWidth > 0) key.width = (int) (maxWidth * rowWidth);
                            IKnowUKeyboardService.log(Log.VERBOSE, "PopupKeyboardView.createKeyboard()", "rowWidth = "+rowWidth+", key.width = "+key.width);

                            if (this.popupKeyHeight > 0) key.height = this.popupKeyHeight;

	                        if (key.width > PopupKey.MAX_WIDTH) key.width = PopupKey.MAX_WIDTH;
	                        
	                        String code = parser.getAttributeValue(ANDROID_NAMESPACE, "codes");
	                        if (code != null) {
	                        	key.codes = new int[] { Integer.parseInt(code) };
	                        } else {
	                        	key.codes = new int[] { key.label.charAt(0) };
	                        }
	                        
	                        String text = parser.getAttributeValue(ANDROID_NAMESPACE, "keyOutputText");
	                        if (text != null) {
	                        	key.text = text;
	                        }
	                        
	                        this.popupKeys.add(key);
	                    }
	                } else if (event == XmlResourceParser.END_TAG) {
	                    if (inKey) {
	                        inKey = false;
	                        x += key.gap + key.width;
	                        if (x > this.popupWidth) {
	                            this.popupWidth = x;
	                        }
	                    } else if (inRow) {
	                        inRow = false;
	                        y += key.height + 5;
	                    }
	                }
	            }
	            this.popupHeight = y;

                if (this.popupHeight / 2 > this.popupKeys.get(0).height) {
                    this.sensitivityRange = (int) (mPopupDistY + (this.popupHeight / DOUBLE_ROW_SCALE_FACTOR));
                    this.isSingleRow = false;
                } else {
                    this.sensitivityRange = (int) (mPopupDistY + (this.popupHeight));
                    this.isSingleRow = true;
                }

	            return true;
	        } catch (Exception e) {
	        	IKnowUKeyboardService.sendErrorMessage(e);
	        	return false;
	        }
		}
		return false;
	}
	
	/**
	 * Creates the keys for this keyboard view, based on the CharSequence passed in
	 * 
	 * @param characters the charcters to be used on the keys that are displayed
	 * @param columns the number of columns to use when displaying the keys
	 * @param maxWidth the maximum width this view should take on screen
	 * @param leftToRight whether or not the keys should fill from left to right
	 * @return true on success or false otherwise
	 */
	public boolean createKeyboard(ArrayList<String> characters, int columns, int maxWidth, boolean leftToRight) {
		try {
			
			int x = 0;
			int width = 0;
			if (leftToRight) {
				x = this.popupPadding;
				this.popupWidth = 0;
			} else {
				int keywidth = PopupKey.calcKeyWidth(maxWidth);
				//Log.v("keywidth =", ""+keywidth);
				width = keywidth*columns + this.popupPadding;
				this.popupWidth = width;
				x = width - (keywidth);
			}
			
			int y = 0;
			if (characters.size() > columns) {
				int keyheight = PopupKey.calcKeyHeight(mContext);
                if (this.popupKeyHeight > 0) keyheight = this.popupKeyHeight;
				y = keyheight + 5 + this.popupPadding;
			} else {
				y = this.popupPadding;
			}
			
	        int column = 0;
	        //this.popupWidth = 0;
	        this.popupHeight = 0;
	        
	        this.popupKeys = new ArrayList<PopupKey>();
	        
	        final int maxColumns = columns == -1 ? Integer.MAX_VALUE : columns;
	        
	        for (int i = 0; i < characters.size(); i++) {
	            String c = characters.get(i);
	            final PopupKey key = new PopupKey(this.mContext, maxWidth);

                if (this.popupKeyHeight > 0) key.height = this.popupKeyHeight;

	            if (leftToRight && (column >= maxColumns || x + key.width > maxWidth)) {
	                x = this.popupPadding;
	                y -= key.height + 5;
	                column = 0;
	            } else if (!leftToRight && (column >= maxColumns || x < 0)) {
	            	x = width - (key.width);
	            	//Log.i("x = ", ""+x);
	                y -= key.height + 5;
	                column = 0;
	            }
	            
	            key.x = x;
	            //Log.w("x = ", ""+x);
	            key.y = y;
	            key.label = c;
	            if (c.length() > 1) {
	            	key.text = c;
	            }
	            
	            key.codes = new int[] { c.charAt(0) };
	            column++;
	            
	            if (leftToRight) {
	            	x += (key.width + key.gap);
	            } else {
	            	x -= (key.width + key.gap);
	            }
	            
	            //Log.e("x = ", ""+x);
	            this.popupKeys.add(key);
	            if (x > this.popupWidth) {
	                this.popupWidth = x;
	            }
	            
	            if (y > this.popupHeight) {
	            	this.popupHeight = y + key.height;
	            }
	        }

            if (this.popupHeight / 2 > this.popupKeys.get(0).height) {
                this.sensitivityRange = (int) (mPopupDistY + (this.popupHeight/ DOUBLE_ROW_SCALE_FACTOR));
                this.isSingleRow = false;
            } else {
                this.sensitivityRange = (int) (mPopupDistY + (this.popupHeight));
                this.isSingleRow = true;
            }

	        return true;
	        //Log.d("Popup width = ", ""+this.popupWidth);
	        //this.popupHeight = y + this.popupKeys.get(0).height;
		} catch (Exception e) {
			IKnowUKeyboardService.sendErrorMessage(e);
			return false;
		}
	}
	
	/**
	 * Override the onMeasure to allow us to set the desired width and height of our
	 * popup, if we dont the system will detect that the view has no content and effectively
	 * set its width and height to 0;
	 */
	@Override
	public void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {

        widthMeasureSpec = MeasureSpec.makeMeasureSpec(this.popupWidth+this.popupPadding+this.strokeWidth, MeasureSpec.EXACTLY);
        heightMeasureSpec = MeasureSpec.makeMeasureSpec(this.popupHeight+this.popupPadding+this.strokeWidth+this.sensitivityRange, MeasureSpec.EXACTLY);
        IKnowUKeyboardService.log(Log.VERBOSE, "PopupView", "onMeasure, sensitivityRange = "+this.sensitivityRange);
		//this.setMeasuredDimension(this.popupWidth+this.popupPadding+this.strokeWidth, this.popupHeight+this.popupPadding+this.strokeWidth+this.sensitivityRange);
        super.onMeasure(widthMeasureSpec, heightMeasureSpec);
        IKnowUKeyboardService.log(Log.VERBOSE, "PopupKeyboardView.onMeasure()", "measuredWidth = "+this.getMeasuredWidth()+", measuredHeight = "+this.getMeasuredHeight());
	}
	
	/**
	 * Paint the supplied key to the canvas
	 * @param key the key to be painted
	 * @param canvas the canavas to paint to
	 * @param bCenterText
	 */
	private void paintKey(PopupKey key, Canvas canvas, int colorRef, boolean bCenterText) {
		try {
			RectF rect = new RectF( (key.x + PopupKey.PAD_SIDES), (key.y + mPadTop), (key.x + key.width - (2 * PopupKey.PAD_SIDES)), (key.y + key.height - mPadBottom) );
			
			boolean isShiftKey = key.codes[0] == -1;
			
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
			switch(this.keyStyle) {
			/*
			 * Style for regular
			 */
			case PopupKeyboardView.THEME_STYLE_REGULAR:
				mPaint.setStyle(Paint.Style.FILL);
				if (key.pressed) {
                    RectF shadowRect = new RectF( (rect.left+2), (rect.top+2), (rect.right+2), (rect.bottom+2) );
                    mPaint.setColor(this.keyShadowColor);
                    canvas.drawRoundRect(shadowRect, this.cornerRadiusX, this.cornerRadiusY, mPaint);
					mPaint.setColor(this.keyPressedColor);
					canvas.drawRoundRect(rect, this.cornerRadiusX, this.cornerRadiusY, mPaint);
				} else {
					RectF shadowRect = new RectF( (rect.left+2), (rect.top+2), (rect.right+2), (rect.bottom+2) );
					mPaint.setColor(this.keyShadowColor);
					canvas.drawRoundRect(shadowRect, this.cornerRadiusX, this.cornerRadiusY, mPaint);
					mPaint.setColor(this.keyColor);
					if (this.useGradient) {
						switch (this.gradientDirection) {
						case PopupKeyboardView.THEME_GRADIENT_DIAGONAL:
							mPaint.setShader(new LinearGradient(rect.left, rect.top, rect.right, rect.bottom, this.keyPressedColor, this.keyColor, Shader.TileMode.REPEAT));
							break;
						case PopupKeyboardView.THEME_GRADIENT_VERTICAL:
							mPaint.setShader(new LinearGradient(rect.left, rect.top, rect.left, rect.bottom, this.keyPressedColor, this.keyColor, Shader.TileMode.REPEAT));
							break;
						case PopupKeyboardView.THEME_GRADIENT_HORIZONTAL:
							mPaint.setShader(new LinearGradient(rect.left, rect.top, rect.right, rect.top, this.keyPressedColor, this.keyColor, Shader.TileMode.REPEAT));
							break;
						}
					}
					canvas.drawRoundRect(rect, this.cornerRadiusX, this.cornerRadiusY, mPaint);
				}
				break;
			/*
			 * Styling for outline
			 */
			case PopupKeyboardView.THEME_STYLE_OUTLINE:
				mPaint.setStyle(Paint.Style.STROKE);
				mPaint.setStrokeWidth(3);
				if (key.pressed) {
					mPaint.setColor(this.keyPressedColor);
					canvas.drawRoundRect(rect, this.cornerRadiusX, this.cornerRadiusY, mPaint);
				} else {
					mPaint.setStyle(Paint.Style.FILL);
					RectF shadowRect = new RectF( rect.left, rect.top, rect.right, rect.bottom );
					mPaint.setColor(this.keyShadowColor);
					canvas.drawRoundRect(shadowRect, this.cornerRadiusX, this.cornerRadiusY, mPaint);
					mPaint.setStyle(Paint.Style.STROKE);
					mPaint.setColor(this.keyColor);
					if (this.useGradient) {
						switch (this.gradientDirection) {
						case PopupKeyboardView.THEME_GRADIENT_DIAGONAL:
							mPaint.setShader(new LinearGradient(rect.left, rect.top - 3, rect.right, rect.bottom + 3, this.keyPressedColor, this.keyColor, Shader.TileMode.REPEAT));
							break;
						case PopupKeyboardView.THEME_GRADIENT_VERTICAL:
							mPaint.setShader(new LinearGradient(rect.left, rect.top - 3, rect.left, rect.bottom + 3, this.keyPressedColor, this.keyColor, Shader.TileMode.REPEAT));
							break;
						case PopupKeyboardView.THEME_GRADIENT_HORIZONTAL:
							mPaint.setShader(new LinearGradient(rect.left - 3, rect.top - 3, rect.right + 3, rect.top - 3, this.keyPressedColor, this.keyColor, Shader.TileMode.REPEAT));
							break;
						}
					}
					canvas.drawRoundRect(rect, this.cornerRadiusX, this.cornerRadiusY, mPaint);
				}
				break;
			/*
			 * Style for lines between
			 */
			case PopupKeyboardView.THEME_STYLE_LINESBETWEEN:
				mPaint.setStyle(Paint.Style.FILL);
				mPaint.setStrokeWidth(3);
				mPaint.setColor(this.keyColor);
				canvas.drawLine(rect.right + mPadRight +mPadLeft, rect.top, rect.right + mPadRight + mPadLeft, rect.bottom, mPaint);
				canvas.drawLine(rect.left + mPadLeft, rect.bottom, rect.right + mPadRight + (STROKE_WIDTH/2), rect.bottom, mPaint);
				break;
			}
			//**** END KEY BACKGROUND COLORING ******//

            // either draw the background or color the background of the key
            if (colorRef != 0xFFFFFFFF) {
                //Log.d("COLOR_REF =", ""+colorRef);
                mPaint.setColor(colorRef);
                if (this.useGradient) {
                    switch (this.gradientDirection) {
                        case PopupKeyboardView.THEME_GRADIENT_DIAGONAL:
                            mPaint.setShader(new LinearGradient(rect.left, rect.top, rect.right, rect.bottom, this.keyPressedColor, colorRef, Shader.TileMode.REPEAT));
                            break;
                        case PopupKeyboardView.THEME_GRADIENT_VERTICAL:
                            mPaint.setShader(new LinearGradient(rect.left, rect.top, rect.left, rect.bottom, this.keyPressedColor, colorRef, Shader.TileMode.REPEAT));
                            break;
                        case PopupKeyboardView.THEME_GRADIENT_HORIZONTAL:
                            mPaint.setShader(new LinearGradient(rect.left, rect.top, rect.right, rect.top, this.keyPressedColor, colorRef, Shader.TileMode.REPEAT));
                            break;
                    }
                }
                canvas.drawRoundRect(rect, this.cornerRadiusX, this.cornerRadiusY, mPaint);
                mPaint.setShader(null);
            }

			mPaint.setShader(null);
			mPaint.setStyle(Paint.Style.FILL);

			if (key.label != null) {
				boolean functionLabel = key.label.length() > 1 && key.codes[0] < 0;
				
				mPaint.setStyle(Paint.Style.FILL);
				mPaint.setColor(this.keyTextColor);
				if (key.label.length() > 1 && key.codes.length < 2) {
					mPaint.setTypeface(Typeface.SANS_SERIF);
//					mPaint.setTypeface(keyboardService.getUnicodeTypeFace());
					mPaint.setTextSize(mLabelTextSize);
	                
				} else {
					mPaint.setAntiAlias(true);
					mPaint.setTypeface(Typeface.SANS_SERIF);
//					mPaint.setTypeface(keyboardService.getUnicodeTypeFace());
					mPaint.setTextSize(mKeyTextSize);
				}
				//override in latin key and privatize mLabelWidth;
	           	float labelWidth = mPaint.measureText(key.label.toString());

				String labelTxt = key.label.toString();
				// make sure you only upper case the keys used for input, not keys indicating function
				/*if (this.getKeyboard().isShifted() && key.codes[0] > 0) {
					// regular label lower case or upper case the main key
					labelTxt = key.label.toString().toUpperCase();
				}*/
		
				if (bCenterText) {	
					// regular keys center the label in their key, make sure caps don't overlap with upper labels
					
					if (functionLabel) {
						canvas.drawText(
								labelTxt,
								key.x + (key.width - labelWidth) / 2 ,
								key.y + (key.height - mPadTop - mPadBottom) / 2 + 
									(mPaint.getTextSize() - mPaint.descent())/2 + mPadTop, mPaint);
						
					} else {
						canvas.drawText(
							labelTxt,
							key.x + ((key.width - labelWidth) / 2)  - 2,
							key.y + (key.height - mPadTop - mPadBottom) / 2 + mPadTop + mpkPadTextVertical, mPaint);
					}
				}
			} 
			else if (key.icon != null) {
				// how do I retrieve the current padding
				// Rect left , top, right , bottom
				// main icon central on the key
				Rect padding = new Rect (mpkUprIconPadLeft, mpkUprIconPadTop, mpkUprIconPadRight, mpkUprIconPadBottom);
				
				Drawable icon = key.icon;
				
			    final int drawableX = (key.width - padding.left - padding.right - icon.getIntrinsicWidth()) / 2 + padding.left;
			    final int drawableY = (key.height - padding.top - padding.bottom - icon.getIntrinsicHeight()) / 2 + padding.top;
			    
			    canvas.translate(key.x + drawableX, key.y + drawableY);
			    icon.setBounds(0, 0, icon.getIntrinsicWidth(), icon.getIntrinsicHeight());
			    //dont want to color over emoticons
			    if(key.codes[0] != -44) {
			    	icon.setColorFilter( this.keyTextColor, PorterDuff.Mode.SRC_ATOP );
			    }
			    icon.draw(canvas);
			    canvas.translate(-key.x -drawableX, -key.y -drawableY);
			}
		} catch (Exception e) {
			IKnowUKeyboardService.sendErrorMessage(e);
		}
	}

	@Override
	public void onDraw(Canvas canvas) {
		try {

            //IKnowUKeyboardService.log(Log.VERBOSE, "PopupKeyboardView.onDraw()", "starting draw");
			//List<Key> keys = this.getKeyboard().getKeys();
			Iterator<PopupKey> it = this.popupKeys.iterator();
			
			//draw the background
			RectF rect = new RectF( 0 + 5, 0 + 5, (this.popupWidth + this.popupPadding), (this.popupHeight + this.popupPadding) +this.sensitivityRange );
            this.mPaint.setShader(new LinearGradient(rect.left, rect.top, rect.left, rect.bottom, this.backgroundColor, this.keyColor, Shader.TileMode.REPEAT));
			//mPaint.setColor(this.backgroundColor);
			mPaint.setStyle(Paint.Style.FILL);
			canvas.drawRoundRect(rect, 5, 5, mPaint);

            this.mPaint.setShader(null);
            //this.mPaint.setShader(new LinearGradient(rect.left, rect.top, rect.left, rect.bottom, this.backgroundColor,
            //        Color.argb(0, Color.red(this.keyColor), Color.green(this.keyColor), Color.blue(this.keyColor)), Shader.TileMode.REPEAT));
            //draw background border
            mPaint.setColor(this.keyColor);
			mPaint.setStyle(Paint.Style.STROKE);
			mPaint.setStrokeWidth(this.strokeWidth);
			canvas.drawRoundRect(rect, 5, 5, mPaint);

			int i = 0;
			while (it.hasNext()) {
				PopupKey key = it.next();
                //IKnowUKeyboardService.log(Log.INFO, "PopupKeyboardView.onDraw()", "defaultKey = "+ selectedKeyIndex);
				if (selectedKeyIndex != -1 && i == selectedKeyIndex) {

                    paintKey(key, canvas, mCharSelectedInPopupColor, true);
					//mPaint.setColor(mCharSelectedInPopupColor);
					//mPaint.setStyle(Paint.Style.STROKE);
					//mPaint.setStrokeWidth(5);
					//canvas.drawRect(key.x+1, key.y+1, key.x+key.width-1, key.y+key.height-1, mPaint);
				} else {
                    paintKey(key, canvas, 0xFFFFFFFF, true);
                }
				i++;
			}// end paint keys loop
		} catch (Exception e) {
			IKnowUKeyboardService.sendErrorMessage(e);
		}
	}//end onDraw
	
	/**
	 * Clear the currently pressed key, so it will not be highlighted anymore
	 */
	private void clearPressedKey() {
		//List<Key> keys = getKeyboard().getKeys();
		Iterator<PopupKey> it = this.popupKeys.iterator();

		while (it.hasNext()) 
		{
			PopupKey itKey = it.next();
			if (itKey.pressed) 
			{
				itKey.pressed = false;
				//itKey.onReleased(false);  // not release inside the key, code doesn't match description
			}
		}
		selectedKeyIndex = -1;
		invalidate();
	}

	/**
	 * Update the currently pressed key
	 * 
	 * @param touchX the x-coord of the touch event
	 * @param touchY the y-coord of the touch event
	 * @param sensitivityRange the distance in the y-axis that should be considered for a
	 * touch to be inside a key
	 */
	private void updatePressedKey(int touchX, int touchY, int sensitivityRange) {
		try {
			selectedKeyIndex = findKeyWithinKeyRange(touchX, touchY, sensitivityRange);
			invalidate();
		} catch (Exception e) {
			IKnowUKeyboardService.sendErrorMessage(e);
		}
	}

	/**
	 * Try to find a key that contains the given coordinates
	 * 
	 * @param touchX the x-coord of the touch event
	 * @param touchY the y-coord of the touch event
	 * @param sensitivityKeyRangeY the distance in the y-axis that should be considered for a
	 * touch to be inside a key
	 * @return the index of the currently touched key, or -1 if no key was found
	 */
	private int findKeyWithinKeyRange(int touchX, int touchY, int sensitivityKeyRangeY) {
		//Rect keyRect = new Rect();
		int possibleCandidate = -1;
		
		int yRange = touchY - this.sensitivityRange;

        //IKnowUKeyboardService.log(Log.VERBOSE, "findKeyWithinRange", "yRange = " + yRange +", touchX = "+touchX+", touchY = "+touchY);

        int greatestXValue = -1;
        int leastXValue = this.popupWidth;
        int greatestXIndex = -1;
        int leastXIndex = -1;
		
		for (int i=0; i < this.popupKeys.size(); i++) {
			PopupKey itKey = this.popupKeys.get(i);
			//Log.d("POPUP KEY WIDTH x HEIGHT =", itKey.width+" x "+itKey.height);
			//keyRect.set(itKey.x, itKey.y, itKey.x + itKey.width, itKey.y + itKey.height);

            if (itKey.x > greatestXValue) {
                greatestXIndex = i;
                greatestXValue = itKey.x;
            }
            if (itKey.x < leastXValue) {
                leastXIndex = i;
                leastXValue = itKey.x;
            }

			//didn't find in our ideal range, if not already a possible candidate
			//try to find in another range, and if so, mark as possible candidate
			if (yRange < (itKey.y + itKey.height) && touchX > itKey.x && touchX < (itKey.x + itKey.width)) {
				if (possibleCandidate >= 0) {
					if ( itKey.y < this.popupKeys.get(possibleCandidate).y) {
						possibleCandidate = i;
					}
				} else {
					possibleCandidate = i;
				}
			}

		}

        //IKnowUKeyboardService.log(Log.VERBOSE, "findKeyWithinRange", "possibleCandidate = " + possibleCandidate);

        if (possibleCandidate == -1) {
            if ( yRange < (this.popupKeys.get(leastXIndex).y + this.popupKeys.get(leastXIndex).height) && touchX < (this.popupKeys.get(leastXIndex).x + this.popupKeys.get(leastXIndex).width) ) {
                possibleCandidate = leastXIndex;
            } else if ( yRange < (this.popupKeys.get(greatestXIndex).y + this.popupKeys.get(greatestXIndex).height) && touchX > this.popupKeys.get(greatestXIndex).x ) {
                possibleCandidate = greatestXIndex;
            }
        }

		return possibleCandidate;
	}
	
	/**
	 * Set the default character to be highlighted when the keyboard opens
	 * 
	 * @param idx the index of the key to be used as the default
	 * @param popupDistY
	 */
	public void setDefaultCharacterIdx(int idx, int popupDistY) {
		selectedKeyIndex = idx;
		mPopupDistY = popupDistY;
	}
	
	/**
	 * Get the key associate with the default index set
	 * 
	 * @return the key found or null if not
	 */
	public PopupKey getDefaultCharIdxKey() {
		if (selectedKeyIndex != -1) {
			//Log.d("POPUP KBVIEW DEFAULT INDEX =", ""+selectedKeyIndex);
			//List<Key> keys = getKeyboard().getKeys();
			return this.popupKeys.get(selectedKeyIndex);
		}
		return (PopupKey) null;
	}
	
	@Override
	public boolean onTouchEvent(MotionEvent me) {
		try {
			//List<Key> keys = getKeyboard().getKeys();
			int action = me.getAction();

            //IKnowUKeyboardService.log(Log.DEBUG, "PopupKeyboardView.onTouchEvent()", "isVisible = "+this.getVisibility()+", width = "+this.getWidth()+", height = "+this.getHeight());
			//int iwidth = getWidth();
	    	//int iheight = getHeight();

	    	//Rect kbRect = new Rect(0,0,iwidth,iheight + sensitivityRange);
	    	//boolean moveInsideKb = kbRect.contains((int) me.getX(), (int) me.getY());

			// only clear when we have started moving initially
	    	// moving outside the keyboard and in is allowed, however there is no cancel option, which 
	    	// seems to be consistent with the overall keyboard behaviour (for instance engage multilingual keyboard and move out)
			
	    	//boolean monitorPress = (action == android.view.MotionEvent.ACTION_DOWN || action == android.view.MotionEvent.ACTION_MOVE) ?  true : false;
	    	
			// if we have tap mode, we don't care about an artificial generated down event
			//if (mSeenDownEvent) {
				//if (moveInsideKb || monitorPress) {

			this.updatePressedKey((int) me.getX(), (int) me.getY(), this.sensitivityRange);
            //IKnowUKeyboardService.log(Log.VERBOSE,"PopupKeyboardView.onTouchEvent()", "selectedKey = "+this.selectedKeyIndex +", touchX = "+me.getX()+", touchY = "+me.getY());
				//}
                /*
				else {
					clearPressedKey();
					//stopRepeat("stepped outside");
					mSelectedIndexIamPopup  = -1;
				}
				*/
			//}
			
			if (action == android.view.MotionEvent.ACTION_UP) {

                //IKnowUKeyboardService.log(Log.VERBOSE, "popupKbView", "selectedIndex = "+selectedKeyIndex);

				if (selectedKeyIndex >= 0) {
					// if key is unrepeatable or has not repeated yet, generate onkey
					PopupKey key = this.getDefaultCharIdxKey();
					bKeyHasBeenRepeating = false;
                    keyboardService.popupManager.getPopupWindow().update();
					keyboardService.popupManager.keyPress(key);
                    this.invalidate();
					//this.releaseWithNoConsequence = true;
					return true;
				} else {
					bKeyHasBeenRepeating = false;
					keyboardService.popupManager.dismissPopupWindow();
					this.clearPressedKey();
                    this.invalidate();
					//this.releaseWithNoConsequence = true;
					return true;
				}
			}
            keyboardService.popupManager.getPopupWindow().update();
            this.invalidate();
			return true;
			//return super.onTouchEvent(me);
		} catch (Exception e) {
			IKnowUKeyboardService.sendErrorMessage(e);
			return false;
		}
	}
	
/*	@Override
	protected boolean onLongPress(Key key) {
		return false;
	}*/

	public boolean hasSeenDownEvent() 
	{
		return mSeenDownEvent;
	}
	
	public void setSeenDownEvent(boolean val)
	{
		mSeenDownEvent = val;
	}
	
}
