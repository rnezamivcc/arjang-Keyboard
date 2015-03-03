package com.iknowu;

import com.iknowu.R;
import android.content.Context;
import android.content.res.Resources;
import android.content.res.XmlResourceParser;
import android.graphics.drawable.Drawable;
import android.inputmethodservice.Keyboard;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.WindowManager;
import android.view.inputmethod.EditorInfo;

import com.iknowu.util.DimensionConverter;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;


/**
 * Loads an XML description of a keyboard and stores the attributes of the keys. A keyboard
 * consists of rows of keys.
 * <p>The layout file for a keyboard contains XML that looks like the following snippet:</p>
 * <pre>
 * &lt;Keyboard
 *         android:keyWidth="%10p"
 *         android:keyHeight="50px"
 *         android:horizontalGap="2px"
 *         android:verticalGap="2px" &gt;
 *     &lt;Row android:keyWidth="32px" &gt;
 *         &lt;Key android:keyLabel="A" /&gt;
 *         ...
 *     &lt;/Row&gt;
 *     ...
 * &lt;/Keyboard&gt;
 * </pre>
 */
public class IKnowUKeyboard {
	
	private static final String NAMESPACE_ANDROID = "http://schemas.android.com/apk/res/android";
    static final String TAG = "Keyboard";
    
    // Keyboard XML Tags
    private static final String TAG_KEYBOARD = "Keyboard";
    private static final String TAG_ROW = "Row";
    private static final String TAG_KEY = "Key";

    public static final int EDGE_LEFT = 0x01;
    public static final int EDGE_RIGHT = 0x02;
    public static final int EDGE_TOP = 0x04;
    public static final int EDGE_BOTTOM = 0x08;

    public static final int KEYCODE_SHIFT = -1;
    public static final int KEYCODE_MODE_CHANGE = -2;
    public static final int KEYCODE_CANCEL = -3;
    public static final int KEYCODE_DONE = -4;
    public static final int KEYCODE_DELETE = -5;

    public static final int KEYCODE_SPACE = 32;
    public static final int KEYCODE_ENTER = 10;

    public static final int KEYCODE_SWITCH_TO_PICKER = -7;
    public static final int KEYCODE_SWITCH_TO_CURRENT = -6;
    public static final int KEYCODE_SWITCH_TO_NUMERIC = -2;
    public static final int KEYCODE_SWITCH_TO_SMILEY = -9;
    public static final int KEYCODE_SWITCH_TO_NAVIGATION = -10;
    public static final int KEYCODE_SWITCH_TO_SYMBOL = -11;
    public static final int KEYCODE_SWITCH_TO_VOICE = -12;
    public static final int KEYCODE_SWITCH_TO_EXTRAS = -8;

    public static final int KEYCODE_SWITCH_TO_QWERTY = -13;
    public static final int KEYCODE_SWITCH_TO_QWERTY_SPANISH = -14;
    public static final int KEYCODE_SWITCH_TO_AZERTY = -15;
    public static final int KEYCODE_SWITCH_TO_QWERTZ = -16;
    public static final int KEYCODE_SWITCH_TO_QZERTY = -17;
    public static final int KEYCODE_SWITCH_TO_RUSSIAN = -18;
    public static final int KEYCODE_SWITCH_TO_KOREAN = -19;

    public static final int KEYCODE_SWITCH_TO_COMPRESSED = -30;
    public static final int KEYCODE_SWITCH_TO_FEATURE = -31;
    public static final int KEYCODE_SWITCH_MODE = -32;
    
    public static final int TOP_SPACE = 0;
    
    public boolean lastKeyWasSpaceOrDelete;
    
    /** Keyboard label **/
    private CharSequence mLabel;

    /** Horizontal gap default for all rows */
    private int mDefaultHorizontalGap;
    
    /** Default key width */
    private int mDefaultWidth;

    /** Default key height */
    private int mDefaultHeight;

    /** Default gap between rows */
    private int mDefaultVerticalGap;

    /** Is the keyboard in the shifted state */
    private boolean mShifted;
    
    /** Key instance for the shift key, if present */
    private Key[] mShiftKeys = { null, null };

    /** Key index for the shift key, if present */
    private int[] mShiftKeyIndices = {-1, -1};

    /** Current key width, while loading the keyboard */
    private int mKeyWidth;
    
    /** Current key height, while loading the keyboard */
    private int mKeyHeight;
    
    /** Total height of the keyboard, including the padding and keys */
    private int mTotalHeight;
    
    /** 
     * Total width of the keyboard, including left side gaps and keys, but not any gaps on the
     * right side.
     */
    private int mTotalWidth;
    
    /** List of keys in this keyboard */
    private List<Key> mKeys;
    
    /** List of rows in this keyboard */
    private ArrayList<Row> rows;
    
    /** List of modifier keys such as Shift & Alt, if any */
    private List<Key> mModifierKeys;
    
    /** Width of the screen available to fit the keyboard */
    private int mDisplayWidth;

    /** Height of the screen */
    private int mDisplayHeight;

    /** Keyboard mode, or zero, if none.  */
    private int mKeyboardMode;
    
    private Key mEnterKey;
    private boolean hasBeenResized;
    
    public Key currentPressedKey;

    // Variables for pre-computing nearest keys.
    private static final int GRID_WIDTH = 10;
    private static final int GRID_HEIGHT = 4;
    private static final int GRID_SIZE = GRID_WIDTH * GRID_HEIGHT;
    private int mCellWidth;
    private int mCellHeight;
    private int[][] mGridNeighbors;
    private float mProximityThreshold;
    /** Number of key widths from current touch point to search for nearest keys. */
    private static float SEARCH_DISTANCE = 1.5f;

    public int yOffset;

    /**
     * Container for keys in the keyboard. All keys in a row are at the same Y-coordinate. 
     * Some of the key size defaults can be overridden per row from what the {@link Keyboard}
     * defines.
     */
    public static class Row {
        
    	private static final int X_CUSHION = 35;
    	private static final float X_ACCELERATION = 1.1f;
    	
    	public ArrayList<Key> keys;
    	
    	public int rowHeight;
    	public int defaultWidth; // default width - a percentage of the view parent
    	public IKnowUKeyboard keyboard;
    	
    	/**
         * The value by which to shift all the keys in this row on their x-axis
         */
        private int shiftX;
    	
    	public int vertGap;	//the gap above this row of keys;
    	
    	public static final float MAX_PERCENT = 0.12f;
    	public float minWidthPercent = 0.77f;
    	public float distanceThreshold = 0.27f;
    	
    	public Row(IKnowUKeyboard kb, int width, int height) {
    		this.keys = new ArrayList<Key>();
    		this.keyboard = kb;
    		this.defaultWidth = width;
    		this.rowHeight = height;
    		this.vertGap = 0;
    	}
    	
    	public void addKey(Key key) {
    		this.keys.add(key);
    	}
    	
    	public Key getKey(int index) {
            if (this.keys.size() > 0) {
    		    return this.keys.get(index);
            } else {
                return null;
            }
    	}
    	
    	public Key getKeyFromPrimaryCode(int primaryCode) {
    		for (int i=0; i < this.keys.size(); i++) {
    			Key key = this.getKey(i);
    			if(key != null && key.codes[0] == primaryCode) {
    				return key;
    			}
    		}
    		return null;
    	}
    	
    	public void resetKeys() {
    		for (int i = 0; i < this.keys.size(); i++) {
    			Key key = this.keys.get(i);
    			key.width = key.origWidth;
    			key.height = key.origHeight;
    			key.x = key.origX;
    			key.y = key.origY;
    			key.isVisible = true;
    		}
    	}
    	
    	public Key getKeyFromCoords(int x, int y) {
    		Key key = null;
    		for (int i=0; i < this.keys.size(); i++) {
    			Key key2 = this.getKey(i);
    			if(key2 != null && key2.isInside(x, y)) {
    				key = key2;
    				return key;
    			} else if (key2 != null) {
    				key2.pressed = false;
    			}
    		}
    		return key;
    	}
    	/////////////////////////////////////////////////////////////////////////////////////////////////
    	public Key getKeyAndSetWidths(float x, float y) 
    	{
    		Key key = null;
    		Key previous = null;
			
    		for (int i=0; i < this.keys.size(); i++) {
    			Key key2 = this.getKey(i);
    			
    			//if the key is not one of the following then we can resize it
    			if (key2 != null && key2.codes[0] != KEYCODE_SPACE && key2.codes[0] != KEYCODE_DELETE && key2.codes[0] != KEYCODE_SHIFT ) {
    				
    				float xdist = Math.abs( x - ( key2.x + (key2.width / 2) ) );
        			
        			float distFromEdge = this.keyboard.getMinWidth();
        			float percent = xdist / distFromEdge;
        			percent = 1.0f - Math.min(percent * 2.f, 0.77f); //subtract from 1 to get appropriate scale value
       			        			
        			int heightChange = (int) (percent * this.vertGap);
        			key2.y = key2.origY - heightChange;
        			key2.height = key2.origHeight + heightChange;
        			
        			percent = percent * MAX_PERCENT; //then multiply by 0.12 to get proper percent of max width = 0.12%
        			
        			key2.width = Math.round( percent * distFromEdge );
        			
        			if (previous != null) {
        				key2.x = previous.x + previous.width + key2.gap;
        			}
    			}
    			
    			key2.pressed = key2.isInside(x, y);
    			if(key2.pressed) 
    				key = key2;
    			
    			previous = key2;
    		}
    		return key;
    	}
    	
    	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    	public Key getKeyAndShiftX(int xshift, float x, float y) 
    	{
    		this.shiftX = xshift;
    		
    		Key key = null;
 			
    		for (int i=0; i < this.keys.size(); i++) {
    			Key key2 = this.getKey(i);
    			
    			//if the key is not one of the following then we can resize it
    			if (key2 != null && key2.codes[0] != KEYCODE_SPACE && key2.codes[0] != KEYCODE_DELETE && 
    					key2.codes[0] != KEYCODE_SHIFT && key2.codes[0] != 44 && key2.codes[0] != 46 ) {
    				
    				key2.x = (int) (key2.origX + (this.shiftX * X_ACCELERATION) + X_CUSHION);
    				
    				float xdist = Math.abs( x - ( key2.x + (key2.width / 2) ) );
        			
        			float distFromEdge = this.keyboard.getMinWidth();
        			
        			float percent = ( xdist / distFromEdge ) * 2.f;
        			percent = 1.f - Math.min(percent,  0.77f);       //subtract from 1 to get appropriate scale value  			
        			
        			int heightChange = (int) (percent * this.vertGap);
        			key2.y = key2.origY - heightChange;
        			key2.height = key2.origHeight + heightChange;
        			/*
        			percent = percent * MAX_PERCENT; //then multiply by 0.12 to get proper percent of max width = 0.12%
        			
        			key2.width = Math.round( percent * distFromEdge );
        			
        			if (previous != null) {
        				key2.x = previous.x + previous.width + key2.gap;
        			}
        			*/
    			}
    			
    			if(key2 != null && key2.isInside(x, y)) {
    				key = key2;
    			} else {
    				key2.pressed = false;
    			}
    		}
    		return key;
    	}
    	
    	/**
         * Detects if a point falls inside this row.
         * @param y the y-coordinate of the point
         * @return whether or not the point falls inside the key. If the key is attached to an edge,
         * it will assume that all points between the key and the edge are considered to be inside
         * the key.
         */
        public boolean isInside(float y) 
        {
           return ( y >= this.keys.get(0).y && y < (this.keys.get(0).y + this.keys.get(0).height) );
        }
    }

    /**
     * Class for describing the position and characteristics of a single key in the keyboard.
     */
    public static class Key 
    {
    	public static final float TOUCH_Y_PERCENT = 0.25f;

        /**
         * index of the key in the array of keys
         */
        public int index;

        /** 
         * All the key codes (unicode or custom code) that this key could generate, zero'th 
         * being the most important.
         */
        public int[] codes;
        
        /** Label to display */
        public CharSequence label;
        public CharSequence labelUpperCase;
        public int upperCharInt;
        
        /** Icon to display instead of a label. Icon takes precedence over a label */
        public Drawable icon;
        /** Preview version of the icon, for the preview popup */
        public Drawable iconPreview;
        
        public float percentWidth;
        /** Width of the key, not including the gap */
        public int width;
        public int origWidth;
        /** Height of the key, not including the gap */
        public int height;
        public int origHeight;
        /** The horizontal gap before this key */
        public int gap;
        public float percentGap;
        /** Whether this key is sticky, i.e., a toggle key */
        public boolean sticky;
        /** X coordinate of the key in the keyboard layout */
        public int x;
        public int origX;
        /** Y coordinate of the key in the keyboard layout */
        public int y;
        public int origY;

        /** Flag to tell whether or not this key should be treated a regular character in the text,
         * or if it is some form of special character, ie. Delete, Shift etc.
         */
        public boolean isCharKey;
        
        /**
         * Special var for the compressed view to track when a key is being pressed and in the process of being
         * sent upwards
         */
        public int touchY;
        
        /** The current pressed state of this key */
        public boolean pressed;
        /** If this is a sticky key, is it on? */
        public boolean on;
        /** Text to output when pressed. This can be multiple characters, like ".com" */
        public CharSequence text;
        /** Popup characters */
        public CharSequence popupCharacters;
        
        /** The size of the key's label in lower-case as measured by our paint object */
        public float labelWidthLower;
        /** The size of the key's label in upper-case as measured by our paint object */
        public float labelWidthUpper;
        
        /** The key's upper text/icon in lower case */
        public String upperText;
        /** The key's upper text/icon in upper case */
        public String upperTextCaps;

        /** The size of the key's upper text label in lower-case as measured by our paint object */
        public float upperTextWidth;
        /** The size of the key's upper text label in upper-case as measured by our paint object */
        public float upperTextWidthCaps;
        
        public Row parentRow;
        
        public String uprDescriptionText;
        public String uprLongPressLabel;
        public boolean uprLongPressLabelCentered;
        public boolean darkBackground; // whether the key has a dark background or not
        
        public boolean isVisible;

        /** 
         * Flags that specify the anchoring to edges of the keyboard for detecting touch events
         * that are just out of the boundary of the key. This is a bit mask of 
         * {@link Keyboard#EDGE_LEFT}, {@link Keyboard#EDGE_RIGHT}, {@link Keyboard#EDGE_TOP} and
         * {@link Keyboard#EDGE_BOTTOM}.
         */
        public int edgeFlags;
        /** Whether this is a modifier key, such as Shift or Alt */
        public boolean modifier;
        /** The keyboard that this key belongs to */
        private IKnowUKeyboard keyboard;
        
        /** 
         * If this key pops up a mini keyboard, this is the resource id for the XML layout for that
         * keyboard.
         */
        public int popupResId;
        /** Whether this key repeats itself when held down */
        public boolean repeatable;
        
        /** Create a key with the given top-left coordinate
         * @param parent the row that this key belongs to. The row must already be attached to
         * a {@link Keyboard}.
         * @param x the x coordinate of the top-left
         * @param y the y coordinate of the top-left
         */
        public Key(Row parent, int x, int y, int width, int height, int gap) 
        {
        	//IKnowUKeyboardService.log(Log.VERBOSE, "key constructor", "width = "+width+", height = "+height+ ", y = "+y);
            this.parentRow = parent;
            this.x = x;
            this.origX = x;
            this.y = y;
            this.origY = y;
            this.width = width;
            this.origWidth = width;
            this.height = height;
            this.origHeight = height;
            this.gap = gap;
            this.isCharKey = false;
            
			this.labelUpperCase = null;
            this.isVisible = true;
            
            this.codes = new int[1];
        }
        
        /**
         * Informs the key that it has been pressed, in case it needs to change its appearance or
         * state.
         * @see #onReleased(boolean)
         */
        public void onPressed() {
            pressed = !pressed;
        }
        
        /**
         * Changes the pressed state of the key. If it is a sticky key, it will also change the
         * toggled state of the key if the finger was release inside.
         * @param inside whether the finger was released inside the key
         * @see #onPressed()
         */
        public void onReleased(boolean inside) {
            pressed = !pressed;
            if (sticky) {
                on = !on;
            }
        }

        /**
         * Detects if a point falls inside this key.
         * @param x the x-coordinate of the point 
         * @param y the y-coordinate of the point
         * @return whether or not the point falls inside the key. If the key is attached to an edge,
         * it will assume that all points between the key and the edge are considered to be inside
         * the key.
         */
        public boolean isInside(float x, float y) {
        	y = this.codes[0] == KEYCODE_CANCEL ? y - 10 : y;
        	
            boolean leftEdge = (edgeFlags & EDGE_LEFT) > 0;
            boolean rightEdge = (edgeFlags & EDGE_RIGHT) > 0;
            boolean topEdge = (edgeFlags & EDGE_TOP) > 0;
            boolean bottomEdge = (edgeFlags & EDGE_BOTTOM) > 0;
            if ((x >= this.x || (leftEdge && x <= this.x + this.width)) 
                    && (x < this.x + this.width || (rightEdge && x >= this.x)) 
                    && (y >= this.y || (topEdge && y <= this.y + this.height))
                    && (y < this.y + this.height || (bottomEdge && y >= this.y))) {
                return true;
            } else {
                return false;
            }
        }
    	
    	public String getUprDescriptionText() {
    		return uprDescriptionText;
    	}

    	public String getUprLongPressLabel() {
    		return uprLongPressLabel;
    	}
    		
    	public boolean getUprLongPressLabelCentered() {
    		return uprLongPressLabelCentered;
    	}
    		
    	public boolean getDarkBackground() {
    		return darkBackground;
    	}
    	
    	public float convertToFloat(String str) {
        	if (str != null) {
        		float percent = 0.0f;
        		
        		if (str.contains("%")) {
        			
        			int index = str.indexOf("%");
        			
        			str = str.substring(0, index);
        			
        			float toFloat = Float.parseFloat(str);
                    
                    percent = toFloat / 100.0f;
                    //Log.d("PERCENT =", ""+percent);
        		}
                return percent;
            } else {
            	return 0.0f;
            }
        }

        /**
         * Returns the square of the distance between the center of the key and the given point.
         * @param x the x-coordinate of the point
         * @param y the y-coordinate of the point
         * @return the square of the distance of the point from the center of the key
         */
        public int squaredDistanceFrom(int x, int y) {
            int xDist = (this.x + width / 2) - x;
            int yDist = (this.y + height / 2) - y;
            //IKnowUKeyboardService.log(Log.INFO, "squaredDistanceFrom()", "sd = "+ Math.sqrt(xDist * xDist + yDist * yDist) );
            return (int) Math.sqrt(xDist * xDist + yDist * yDist);
        }
    }

    /**
     * Creates a keyboard from the given xml key layout file.
     * @param context the application or service context
     * @param xmlLayoutResId the resource file that contains the keyboard layout and keys.
     */
    public IKnowUKeyboard(Context context, int xmlLayoutResId, int yoffset) {
    	this.rows = new ArrayList<Row>();
    	this.mKeys = new ArrayList<Key>();
    	this.mModifierKeys = new ArrayList<Key>();
        this.yOffset = yoffset;
        this.loadKeyboard(context, xmlLayoutResId);
    }
    
    public Key getKeyFromPrimaryCode(int primaryCode) {
    	for (int i=0; i < this.rows.size(); i++) {
			Row row = this.rows.get(i);
			
			Key key = row.getKeyFromPrimaryCode(primaryCode);
			if (key != null) {
				return key;
			}
		}
		return null;
    }
    
    public Key getKeyFromCoords(int x, int y) {
		for (int i=0; i < this.rows.size(); i++) {
			Row row = this.rows.get(i);
			
			if(row.isInside(y)){
				currentPressedKey = row.getKeyFromCoords(x, y);
				return currentPressedKey;
			}
		}
		return null;
	}
    
    public Key getKeyAndSetWidths(float x, float y) {
    	for (int i=0; i < this.rows.size(); i++) {
			Row row = this.rows.get(i);
			
			if(row.isInside(y)){
				currentPressedKey = row.getKeyAndSetWidths(x, y);
				return currentPressedKey;
			}
		}
		return null;
    }
    
    public Key getKeyAndShiftX(int shiftx, float x, float y) {
    	for (int i=0; i < this.rows.size(); i++) {
			Row row = this.rows.get(i);
			
			if(row.isInside(y)){
				currentPressedKey = row.getKeyAndShiftX(shiftx, x, y);
				return currentPressedKey;
			}
		}
		return null;
    }
    
    public List<Key> getKeys() {
        return mKeys;
    }
    
    public List<Key> getModifierKeys() {
        return mModifierKeys;
    }
    
    protected int getHorizontalGap() {
        return mDefaultHorizontalGap;
    }
    
    protected void setHorizontalGap(int gap) {
        mDefaultHorizontalGap = gap;
    }

    protected int getVerticalGap() {
        return mDefaultVerticalGap;
    }

    protected void setVerticalGap(int gap) {
        mDefaultVerticalGap = gap;
    }

    protected int getKeyHeight() {
        return mDefaultHeight;
    }

    protected void setKeyHeight(int height) {
        mDefaultHeight = height;
    }

    protected int getKeyWidth() {
        return mDefaultWidth;
    }
    
    protected void setKeyWidth(int width) {
        mDefaultWidth = width;
    }

    /**
     * Find a key form it's primary code
     */
    public Key findKey(int primaryCode) {

        for (int i = 0; i < this.mKeys.size(); i++) {
            if ( this.mKeys.get(i) != null && this.mKeys.get(i).codes != null && this.mKeys.get(i).codes[0] == primaryCode ) return this.mKeys.get(i);
        }

        return null;
    }

    public Key getKey(int index) {
        return this.mKeys.get(index);
    }
    
    public int getMinWidth() {
        return mTotalWidth;
    }

    public boolean setShifted(boolean shiftState) {
        for (Key shiftKey : mShiftKeys) {
            if (shiftKey != null) {
                shiftKey.on = shiftState;
            }
        }
        if (mShifted != shiftState) {
            mShifted = shiftState;
            return true;
        }
        return false;
    }

    public boolean isShifted() {
        return mShifted;
    }

    /**
     * @hide
     */
    public int[] getShiftKeyIndices() {
        return mShiftKeyIndices;
    }

    public int getShiftKeyIndex() {
        return mShiftKeyIndices[0];
    }

    private void createKeyNeighbours(Key key) {

    }
    
    private void computeNearestNeighbors() {
        // Round-up so we don't have any pixels outside the grid
        mCellWidth = (getMinWidth() + GRID_WIDTH - 1) / GRID_WIDTH;
        mCellHeight = (getHeight() + GRID_HEIGHT - 1) / GRID_HEIGHT;

        this.mProximityThreshold = ( (mCellHeight + mCellWidth) / 2 ) * SEARCH_DISTANCE;

        mGridNeighbors = new int[GRID_SIZE][];
        int[] indices = new int[mKeys.size()];
        final int gridWidth = GRID_WIDTH * mCellWidth;
        final int gridHeight = GRID_HEIGHT * mCellHeight;

        for (int x = 0; x < gridWidth; x += mCellWidth) {

            for (int y = 0; y < gridHeight; y += mCellHeight) {
                int count = 0;

                for (int i = 0; i < mKeys.size(); i++) {
                    final Key key = mKeys.get(i);

                    final int xp = x + (mCellWidth / 2);
                    final int yp = y + (mCellHeight / 2);

                    //IKnowUKeyboardService.log(Log.VERBOSE, "IKnowUKeyboard.computeNearestNeighbors()", "proximityThreshold = "+mProximityThreshold+", cellWidth = "+mCellWidth+", cellHeight = "+mCellHeight);
                    if (key.squaredDistanceFrom(xp, yp) < mProximityThreshold /*|| key.squaredDistanceFrom(x + mCellWidth - 1, y) < mProximityThreshold ||
                            key.squaredDistanceFrom(x + mCellWidth - 1, y + mCellHeight - 1) < mProximityThreshold || key.squaredDistanceFrom(x, y + mCellHeight - 1) < mProximityThreshold*/) {
                        indices[count++] = i;
                        //IKnowUKeyboardService.log(Log.WARN, "IKnowUKeyboard.computeNearestNeighbors()", "indices["+count+"] = "+i+ ", neighbor = "+key.label);
                    }
                }

                int [] cell = new int[count];
                System.arraycopy(indices, 0, cell, 0, count);
                mGridNeighbors[(y / mCellHeight) * GRID_WIDTH + (x / mCellWidth)] = cell;
            }
        }
    }

    private void computeNearestKeys() {

        mGridNeighbors = new int[this.mKeys.size()][];
        int[] indices = new int[this.mKeys.size()];
        int curKey = 0;
        for (int i=0; i < this.rows.size(); i++) {
            Row row = this.rows.get(i);
            for (int j=0; j < row.keys.size(); j++) {
                Key theKey = row.getKey(j);
                this.mProximityThreshold = ( (theKey.width + theKey.height) / 2 ) * SEARCH_DISTANCE;
                final int xp = theKey.x + (theKey.width / 2);
                final int yp = theKey.y + (theKey.height / 2);
                int count = 0;

                if (theKey != null && theKey.isCharKey) {
                    for (int k=0; k < this.mKeys.size(); k++) {
                        Key key = this.mKeys.get(k);
                        if ( !theKey.equals(key) && key.isCharKey ) {
                            //IKnowUKeyboardService.log(Log.VERBOSE, "IKnowUKeyboard.computeNearestNeighbors()", "proximityThreshold = "+mProximityThreshold+", keyWidth = "+mCellWidth+", keyHeight = "+mCellHeight);
                            if (key.squaredDistanceFrom(xp, yp) < mProximityThreshold /*|| key.squaredDistanceFrom(x + mCellWidth - 1, y) < mProximityThreshold ||
                            key.squaredDistanceFrom(x + mCellWidth - 1, y + mCellHeight - 1) < mProximityThreshold || key.squaredDistanceFrom(x, y + mCellHeight - 1) < mProximityThreshold*/) {
                                indices[count++] = k;
                                //IKnowUKeyboardService.log(Log.WARN, "IKnowUKeyboard.computeNearestNeighbors()", "mGridNeighbors["+curKey+"]["+count+"] = "+k+", neighbor = "+key.label);
                            }
                        }
                    }

                }
                int [] cell = new int[count];
                System.arraycopy(indices, 0, cell, 0, count);
                mGridNeighbors[curKey] = cell;
                curKey++;
            }

        }
    }
    
    /**
     * Returns the indices of the keys that are closest to the given point.
     * @param index the index of the key to get the nearest keys from
     * @return the array of integer indices for the nearest keys to the given index. If the given
     * index is out of range, then an array of size zero is returned.
     */
    public int[] getNearestKeys(int index) {
        //if (mGridNeighbors == null) computeNearestNeighbors();
        if (mGridNeighbors == null) this.computeNearestKeys();
        if ( index >= 0 && index < this.mKeys.size() ) {
            return mGridNeighbors[index];
        }
        return new int[0];
    }
    
    private int calcScreenWidth(Context ctx) {
    	DisplayMetrics dm = new DisplayMetrics();
		WindowManager window = (WindowManager) ctx.getSystemService(Context.WINDOW_SERVICE);
		window.getDefaultDisplay().getMetrics(dm);
		return dm.widthPixels;
    }
    
    /**
     * Given a string of key labels, assumed the ones that need to be shown
     * 
     * 
     * @param letterKeys
     * @param x
     * @param y
     */
    public void setKeyWidthsFromString(String letterKeys, int x, int y) {
    	for (int i = 0 ; i < this.rows.size(); i++) {
    		Row currentRow = this.rows.get(i);
    		Key previouskey = null;
    		for (int j = 0; j < currentRow.keys.size(); j++) {
    			Key key = currentRow.getKey(j);
    			
    			if ( key != null && key.label != null ) {
    				if (letterKeys.contains(key.label)) {
    					key.isVisible = true;
    					key.width = this.getMinWidth() / (letterKeys.length() + 1);
    					if (previouskey != null) {
    						if (previouskey.x < x && previouskey.x + previouskey.width > x) {
    							key.x = previouskey.x + (previouskey.width * 2) + key.gap;
    						} else {
    							key.x = previouskey.x + previouskey.width + key.gap;
    						}
    					} else {
    						if (0 < x && key.width > x) {
    							key.x = key.width + key.gap;
    						} else {
    							key.x = key.gap;
    						}
    					}
    					previouskey = key;
    				} else {
    					key.width = 0;
    					key.x = 0;
    					key.isVisible = false;
    				}
    			}
    		}
    	}
    }
    
    public void resetKeys() {
    	for (int i = 0; i < this.rows.size(); i++) {
    		this.rows.get(i).resetKeys();
    	}
    }
    
    private void loadKeyboard(Context context, int resLayoutId) {
    	
    	XmlResourceParser parser = context.getResources().getXml(resLayoutId);
    	
    	final int screenWidth = this.calcScreenWidth(context);
    	
        boolean inKey = false;
        boolean inRow = false;
        boolean leftMostKey = false;
        int row = 0;
        int x = 0;
        int y = this.yOffset;
        Key key = null;
        Row currentRow = null;
        Resources res = context.getResources();
        boolean skipRow = false;
        int gap = 0;
        float percentGap = 0f;
        
        int keyHeight = 0;
        int keyWidth = 0;
        float percentKeyWidth = 0;
        
        int rowKeyHeight = 0;
        int rowKeyWidth = 0;
        float rowPercentKeyWidth = 0;
                
        try {
            int event;
            while ((event = parser.next()) != XmlResourceParser.END_DOCUMENT) {
                if (event == XmlResourceParser.START_TAG) {
                   String tag = parser.getName();

                    /**
                     * Row Tag
                     */
                    if (TAG_ROW.equals(tag)) {
                        inRow = true;
                        x = 0;

                        rowKeyHeight = 0;
                        rowKeyWidth = 0;

                        //try to get the height of the keys if set in the row tag
                        int rkeyHeight = parser.getAttributeResourceValue(NAMESPACE_ANDROID, "keyHeight", -1);
                        if (rkeyHeight != -1) {
                            rowKeyHeight = res.getDimensionPixelSize(rkeyHeight);
                        } else {
                            rowKeyHeight = Integer.valueOf(keyHeight);
                        }

                        IKnowUKeyboardService.log(Log.INFO, "loadKeyboard", "row key height = "+rowKeyHeight);

                        String vertGapStr = parser.getAttributeValue(null, "verticalGap");
                        int vertGap = 0;

                        if (vertGapStr != null) {
                            vertGap = DimensionConverter.stringToDimensionPixelSize(vertGapStr, res.getDisplayMetrics());
                        }

                        y += vertGap;

                        //get the default widht of the keys in this row
                        String strWidth = parser.getAttributeValue(NAMESPACE_ANDROID, "keyWidth");
                        //IKnowUKeyboardService.log(Log.INFO, "loadKeyboard", "string key width = "+strWidth);
                        //get rid of the "%p" from the value
                        if (strWidth != null) {
                            strWidth = strWidth.substring(0, strWidth.length() - 2);
                            //divide by 100 to convert to percent value
                            float rkeyWidth = Float.parseFloat(strWidth) / 100f;
                            //IKnowUKeyboardService.log(Log.INFO, "loadKeyboard", "row key width = "+rkeyWidth);
                            if (rkeyWidth != -1) {
                                rowKeyWidth = Math.round(rkeyWidth * screenWidth);
                                rowPercentKeyWidth = rkeyWidth;
                            } else {
                                rowKeyWidth = Math.round(0.1f * screenWidth);
                                rowPercentKeyWidth = 0.1f;
                            }
                        } else {
                            rowKeyWidth = Integer.valueOf(keyWidth);
                            rowPercentKeyWidth = percentKeyWidth;
                        }

                        //IKnowUKeyboardService.log(Log.INFO, "loadKeyboard", "key width = "+keyWidth);
                        currentRow = new Row(this, rowKeyWidth, rowKeyHeight);
                        currentRow.vertGap = vertGap;
                        rows.add(currentRow);
                        /**
                         * Key Tag
                         */
                    } else if (TAG_KEY.equals(tag)) {
                        inKey = true;

                        int keyKeyWidth = 0;
                        int keyKeyHeight = 0;
                        float keyPercentWidth = 0;

                        String strWidth = parser.getAttributeValue(NAMESPACE_ANDROID, "keyWidth");

                        //IKnowUKeyboardService.log(Log.INFO, "loadKeyboard", "string key width = "+strWidth);
                        //if defined for this key, set the width
                        if (strWidth != null) {
                            strWidth = strWidth.substring(0, strWidth.length() - 2);
                            float rkeyWidth = Float.parseFloat(strWidth) / 100f;
                            //IKnowUKeyboardService.log(Log.INFO, "loadKeyboard", "key width = "+rkeyWidth);
                            if (rkeyWidth != -1) {
                                keyKeyWidth = Math.round(rkeyWidth * screenWidth);
                                keyPercentWidth = rkeyWidth;
                            }
                        } else if (rowKeyWidth != 0) {
                            keyKeyWidth = Integer.valueOf(rowKeyWidth);
                            keyPercentWidth = rowPercentKeyWidth;
                        } else {
                            keyKeyWidth = Integer.valueOf(keyWidth);
                            keyPercentWidth = percentKeyWidth;
                        }

                        int kkeyHeight = parser.getAttributeResourceValue(NAMESPACE_ANDROID, "keyHeight", -1);
                        if (kkeyHeight != -1) {
                            keyKeyHeight = res.getDimensionPixelSize(keyHeight);
                        } else if (rowKeyWidth != 0) {
                            keyKeyHeight = Integer.valueOf(rowKeyHeight);
                        } else {
                            keyKeyHeight = Integer.valueOf(keyHeight);
                        }

                        int keyGap = 0;
                        float keyPercentGap = 0f;
                        String strGap = parser.getAttributeValue(NAMESPACE_ANDROID, "horizontalGap");
                        //IKnowUKeyboardService.log(Log.INFO, "loadKeyboard", "string key gap = "+strGap);
                        if (strGap != null) {
                            strGap = strGap.substring(0, strGap.length() - 2);
                            float perGap = Float.parseFloat(strGap) / 100f;
                            //IKnowUKeyboardService.log(Log.INFO, "loadKeyboard", "row key width = "+perGap);
                            if (perGap != -1) {
                                keyGap = Math.round(perGap * screenWidth);
                                keyPercentGap = perGap;
                            }
                        } else {
                            keyGap = Integer.valueOf(gap);
                            keyPercentGap = percentGap;
                        }

                        x += keyGap;

                        key = new Key(currentRow, x, y, keyKeyWidth, keyKeyHeight, keyGap);
                        key.percentWidth = keyPercentWidth;
                        key.percentGap = keyPercentGap;

                        key.isCharKey = parser.getAttributeBooleanValue(null, "isCharKey", false);
                        //get the key codes that are associated with this key
                        String codesString = parser.getAttributeValue(NAMESPACE_ANDROID, "codes");
                        if (codesString != null) {
                            int commaCount = this.countChar(codesString, ',');
                            key.codes = new int[commaCount + 1];
                            int commaIndex = 0;

                            //IKnowUKeyboardService.log(Log.VERBOSE, "Creating Key", "Looping codes");

                            for (int i = 0; i < key.codes.length; i++) {
                                commaIndex = codesString.indexOf(',');
                                if (commaIndex > -1) {
                                    key.codes[i] = Integer.parseInt(codesString.substring(0, commaIndex));
                                    //IKnowUKeyboardService.log(Log.INFO, "Creating codes", "code = "+key.codes[i]);
                                    codesString = codesString.substring(commaIndex + 1, codesString.length());
                                } else {
                                    key.codes[i] = Integer.parseInt(codesString);
                                    //IKnowUKeyboardService.log(Log.INFO, "Creating codes", "code = "+key.codes[i]);
                                }
                            }
                        } else {
                            key.codes[0] = 0;
                        }

                        String label = parser.getAttributeValue(NAMESPACE_ANDROID, "keyLabel");
                        if (label != null) key.label = label;

                        int upperchar = parser.getAttributeIntValue(null, "upperChar", 0);
                        if (upperchar != 0) {
                            key.upperCharInt = upperchar;
                            key.labelUpperCase = "" + (char) upperchar;
                        }
                        IKnowUKeyboardService.log(Log.VERBOSE, "loadKeyboard()", "key.labelUpperCase = "+key.labelUpperCase);

                        int iconid = parser.getAttributeResourceValue("http://schemas.android.com/apk/res/android", "keyIcon", -1);
                        if (iconid != -1) key.icon = res.getDrawable(iconid);
                        if (key.icon != null) {
                            key.icon.setBounds(0, 0, key.icon.getIntrinsicWidth(), key.icon.getIntrinsicHeight());
                            key.iconPreview = res.getDrawable(iconid);
                            key.iconPreview.setBounds(0, 0, key.icon.getIntrinsicWidth(), key.icon.getIntrinsicHeight());
                        }

                        String text = parser.getAttributeValue("http://schemas.android.com/apk/res/android", "keyOutputText");
                        if (text != null) {
                            key.text = text;
                        }

                        if (key.codes[0] == -44) {   // smileys
                            key.uprDescriptionText = parser.getAttributeValue("http://schemas.android.com/apk/res/com.keyboard", "keyUprDescriptionLabel");
                        }

                        key.popupResId = parser.getAttributeResourceValue("http://schemas.android.com/apk/res/android", "popupKeyboard", -1);

                        key.uprLongPressLabel = parser.getAttributeValue("http://schemas.android.com/apk/res/com.keyboard", "keyUprLongPressLabel");

                        key.uprLongPressLabelCentered = parser.getAttributeBooleanValue("http://schemas.android.com/apk/res/com.keyboard", "keyUprLongPressLabelCentered", false);

                        key.darkBackground = parser.getAttributeBooleanValue(null, "keyDarkBackground", false);

                        if (key.codes[0] == 10) {
                            mEnterKey = key;
                        }

                        if (key.codes[0] == KEYCODE_SHIFT) {
                            // Find available shift key slot and put this shift key in it
                            for (int i = 0; i < mShiftKeys.length; i++) {
                                if (mShiftKeys[i] == null) {
                                    mShiftKeys[i] = key;
                                    mShiftKeyIndices[i] = mKeys.size() - 1;
                                    break;
                                }
                            }
                            mModifierKeys.add(key);
                        }/* else if (key.codes[0] == KEYCODE_ALT) {
                            mModifierKeys.add(key);
                        }*/
                        key.index = this.mKeys.size();
                        mKeys.add(key);
                        currentRow.addKey(key);
                        /**
                         * Keyboard Tag
                         */
                    } else if (TAG_KEYBOARD.equals(tag)) {
                        keyHeight = parser.getAttributeResourceValue(NAMESPACE_ANDROID, "keyHeight", -1);
                        if (keyHeight != -1) {
                            keyHeight = res.getDimensionPixelSize(keyHeight);
                        } else {
                            keyHeight = res.getDimensionPixelSize(R.dimen.key_height);
                        }

                        this.mDefaultHeight = keyHeight;

                        String strGap = parser.getAttributeValue(NAMESPACE_ANDROID, "horizontalGap");
                        //IKnowUKeyboardService.log(Log.INFO, "loadKeyboard", "string key gap = "+strGap);
                        strGap = strGap.substring(0, strGap.length() - 2);
                        float perGap = Float.parseFloat(strGap) / 100f;
                        //IKnowUKeyboardService.log(Log.INFO, "loadKeyboard", "row key width = "+perGap);
                        if (perGap != -1) {
                            gap = Math.round(perGap * screenWidth);
                            percentGap = perGap;
                        } else {
                            gap = 0;
                            percentGap = 0;
                        }
                        //IKnowUKeyboardService.log(Log.INFO, "loadKeyboard", "resource key height = "+keyHeight);
                    }
                } else if (event == XmlResourceParser.END_TAG) {
                    if (inKey) {
                        inKey = false;
                        x += key.width;
                        if (x > mTotalWidth) {
                            mTotalWidth = x;
                        }
                    } else if (inRow) {
                        inRow = false;
                        //y += currentRow.verticalGap;
                        Key endkey = currentRow.getKey(0);
                        if (endkey != null) {
                        	//IKnowUKeyboardService.log(Log.WARN, "loadKeyboard", "end row tag endkey height = "+endkey.height);
                        	y += endkey.height;
                        } else {
                        	//IKnowUKeyboardService.log(Log.WARN, "loadKeyboard", "end row tag row height = "+currentRow.rowHeight);
                        	y += currentRow.rowHeight;
                        }
                       
                        row++;
                    } else {
                    	
                    }
                }
            }

            //his.computeNearestNeighbors();
            //this.computeNearestKeys();
        } catch (Exception e) {
            Log.e(TAG, "Parse error:" + e);
            e.printStackTrace();
        }
        mTotalHeight = y - mDefaultVerticalGap;
    }
    
    private int countChar(String text, char ch) {
    	int count = 0;
    	for (int i=0; i < text.length(); i++) {
    		if (text.charAt(i) == ch) count++;
    	}
    	return count;
    }

    public void resizeKeys(int viewWidth) {
        List<Key> keys = this.getKeys();
        Iterator<Key> it = keys.iterator();

        int x = 0;
        while (it.hasNext()) {
            Key key = it.next();
            if (viewWidth > 0) {
                //Log.d("KEY WIDTH =", ""+key.width);
                if ( key.x - key.gap <= 0 ) x = 0;
                //IKnowUKeyboardService.log(Log.VERBOSE, "resizing key = "+key.label, "key WIDTH before = "+key.width);
                key.width = Math.max((int) (viewWidth * key.percentWidth), 1);

                key.gap = (int) (viewWidth * key.percentGap);
                key.x = x + key.gap;
                //IKnowUKeyboardService.log(Log.VERBOSE, "resizing key = ["+key.codes[0]+","+key.label+"]", "key WIDTH = "+key.width+", gap = "+key.gap);
                //Log.d("KEY X =", ""+key.x);
                x += key.gap + key.width;
            }
        }
    }
    
    /**
	 * Resize keys to fit the keyboardview's width, since i haven't found a way to achieve
	 * this through xml. This is needed for the tablet version which wishes to display multiple
	 * keyboards on screen at once, this will size keys according to how they are currently sized
	 * based on the percentage of width they take up in a row
	 */
    public void resizeKeys(int viewWidth, int displayWidth) {
    	try {
    		//Log.d();
    		IKnowUKeyboardService.log(Log.VERBOSE, "KEYBOARD RESIZE KEYS", "TRYING TO RESIZE, WIDTH = "+viewWidth+", displayWidth = "+displayWidth);
        	if (!this.hasBeenResized && viewWidth > 0) {
        		List<Key> keys = this.getKeys();
        		Iterator<Key> it = keys.iterator();
        		
        		
        		//Have to perform this calculation due to different android versions
        		//performing differently when inflating views
        		
        		int currentWidth = 0;
        		int rowWidth = 0;
        		int gap = 0;
        		while (it.hasNext()) {
        			Key key = it.next();
        			if (key.x - key.gap <= 0) {
        				rowWidth = 0;
        			}
        			gap = key.gap;
        			rowWidth += key.gap + key.width;
        			if (rowWidth > currentWidth) currentWidth = rowWidth;
        		}
        		IKnowUKeyboardService.log(Log.VERBOSE, "CURRENT WIDTH = ", "= "+currentWidth);
        		
        		double percentWidth;
        		
        		//can't be, clamp to display width
        		if (currentWidth > displayWidth) {
        			currentWidth = displayWidth;
        		}
        		
        		//plus or minus a key gap, if the widths are almost the same then
        		//dont do any percentage scaling
        		if (currentWidth - gap > viewWidth || currentWidth + gap < viewWidth) {
        			percentWidth = currentWidth / (double) displayWidth;
        		} else {
        			percentWidth = 1.0;
        		}
        		
        		//can't be greater than one, this would imply the view is larger than the width of the screen
        		if (percentWidth > 1) {
        			percentWidth = 1;
        		}
        		
        		it = keys.iterator();
        		
        		int x = 0;
        		while (it.hasNext()) {
        			Key key = it.next();
        			if (key.width > 0 && currentWidth > 0) {
        				//Log.d("KEY WIDTH =", ""+key.width);
        				if ( key.x - key.gap <= 0 ) x = 0;
        				double percentage = (double) (key.width / (double) currentWidth);
        				percentage = percentage * percentWidth;
        				//IKnowUKeyboardService.log(Log.VERBOSE, "resizing key = "+key.label, "key WIDTH before = "+key.width);
        				key.width = (int) (viewWidth * percentage);
        				//IKnowUKeyboardService.log(Log.VERBOSE, "resizing key = "+key.label, "key WIDTH = "+key.width+", percent = "+percentage+", percentWidth = "+percentWidth);
        				key.x = x;
        				double gapPercentage = (double) (key.gap / (double) currentWidth);
        				gapPercentage = gapPercentage * percentWidth;
        				key.gap = (int) (viewWidth * gapPercentage);
        				//Log.d("KEY X =", ""+key.x);
        				x += key.gap + key.width;
        			}
        		}
        		this.hasBeenResized = true;
        	}
    	} catch (Exception e) {
    		IKnowUKeyboardService.sendErrorMessage(e);
    	}
    }
    
    public void setKeyHeights(float scaleFactor) {
    	//this.setKeyHeight(height);
    	IKnowUKeyboardService.log(Log.VERBOSE, "Set keyHeight!!", "defaultHeight  before = "+this.mDefaultHeight);
    	this.mDefaultHeight = (int) (this.mDefaultHeight * scaleFactor);
    	IKnowUKeyboardService.log(Log.VERBOSE, "Set keyHeight!!", "defaultHeight  after = "+this.mDefaultHeight);
    	try {
        	if (scaleFactor > 0) {
        		List<Key> keys = this.getKeys();
        		Iterator<Key> it = keys.iterator();
        		
        		int y = keys.get(0).y;
        		int previousHeight = 0;
        		while (it.hasNext()) {
        			Key key = it.next();
        			if ( key.x - key.gap <= 0 ) y += previousHeight + key.parentRow.vertGap;
        			int height = (int) (key.height * scaleFactor);
        			key.height = height;
        			key.origHeight = height;
        			key.y = y;
        			previousHeight = height;
        			//Log.w("Key y = ", ""+key.y);
        		}
        	}
    	} catch (Exception e) {
    		IKnowUKeyboardService.sendErrorMessage(e);
    	}
    }
    
    public int getHeight() {
    	int height = 0;
    	
    	for ( int i = 0 ; i < this.rows.size() ; i++ ) {
            Row row = this.rows.get(i);
    		Key key = this.rows.get(i).getKey(0);
    		if (row != null && key != null) {
    			height += key.height;
    			height += row.vertGap;
    		} else if (row != null) {
                height += row.rowHeight;
                height += row.vertGap;
            }
    	}
    	//IKnowUKeyboardService.log(Log.VERBOSE, "IKnowUKeyboard.getHeight()", "height = "+height);
    	return height;
    }

    public void offsetKeyYPos(int offset) {
        try {
            if (offset != 0) {
                List<Key> keys = this.getKeys();
                Iterator<Key> it = keys.iterator();

                int y = keys.get(0).y;
                int previousHeight = 0;
                while (it.hasNext()) {
                    Key key = it.next();
                    if ( key.x - key.gap <= 0 ) y += previousHeight + key.parentRow.vertGap;
                    key.y = y + offset;
                    previousHeight = key.height;
                    //Log.w("Key y = ", ""+key.y);
                }
            }
        } catch (Exception e) {
            IKnowUKeyboardService.sendErrorMessage(e);
        }
    }
    
    /**
     * This looks at the ime options given by the current editor, to set the
     * appropriate label on the keyboard's enter key (if it has one).
     */
    public int setImeOptions(Resources res, int options) {
        if (mEnterKey == null) {
            return 0;
        }
        
        switch (options&(EditorInfo.IME_MASK_ACTION|EditorInfo.IME_FLAG_NO_ENTER_ACTION)) {
            case EditorInfo.IME_ACTION_GO:
                mEnterKey.iconPreview = null;
                mEnterKey.icon = null;
                mEnterKey.label = res.getText(R.string.label_go_key);
                return EditorInfo.IME_ACTION_GO;
            case EditorInfo.IME_ACTION_NEXT:
                mEnterKey.iconPreview = null;
                mEnterKey.icon = null;
                mEnterKey.label = res.getText(R.string.label_next_key);
                return EditorInfo.IME_ACTION_NEXT;
            case EditorInfo.IME_ACTION_SEARCH:
            	mEnterKey.iconPreview = res.getDrawable(R.drawable.sym_keyboard_search);
                mEnterKey.icon = res.getDrawable(R.drawable.sym_keyboard_search);
                mEnterKey.label = null;
                return EditorInfo.IME_ACTION_SEARCH;
            case EditorInfo.IME_ACTION_SEND:
                mEnterKey.iconPreview = null;
                mEnterKey.icon = null;
                mEnterKey.label = res.getText(R.string.label_send_key);
                return EditorInfo.IME_ACTION_SEND;
            default:
            	mEnterKey.iconPreview = res.getDrawable(R.drawable.sym_keyboard_return);
                mEnterKey.icon = res.getDrawable(R.drawable.sym_keyboard_return);
                mEnterKey.label = null;
                return 0;
        }
    }
}