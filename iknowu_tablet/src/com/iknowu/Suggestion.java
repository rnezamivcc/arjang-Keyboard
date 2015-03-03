package com.iknowu;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.RectF;
import android.graphics.drawable.Drawable;
import android.view.MotionEvent;

/**
 * Created by Justin on 17/10/13.
 *
 */
public class Suggestion {

    public static final int TEXT_MODE_LATIN = 0;
    public static final int TEXT_MODE_KOREAN = 1;

    public static final int PADDING_TOP_BOTTOM = 10;
    public static final int PADDING_LEFT_RIGHT = 15;

    // Use this property to adjust spacing between buttons.
    public static final int MARGIN_TOP_BOTTOM = 8;

    public static final int CAPS_NONE = 0;
    public static final int CAPS_FIRST_LETTER = 1;
    public static final int CAPS_ALL = 2;

    public static final String TAG = "DEBUG";

    protected String textShown;
    protected String rawPrediction;
    protected int selectedColor;
    protected boolean longPressOccurred;
    protected boolean isNewWord;
    protected boolean isRoot;

    protected boolean isBold;
    protected boolean isPressed;

    protected int fontColor;
    protected int fontSize;

    protected int xpos;
    protected int ypos;
    protected int marginLeft;
    protected int marginTop;
    protected int width;
    protected int height;
    protected int minWidth;


    public boolean sawDownEvent;

    public int capsMode;

    protected SuggestionsLinearLayout parent;

    protected int index;

    protected Context context;
    protected IKnowUKeyboardService service;
    protected Drawable bg;

    protected RectF bgRect;
    protected RectF shadowRect;

    public Suggestion(Context context) {
        this.context = context;
        //this.createBackgroundDrawable();
    }

    /**
     * Set the position and dimensions for this view,
     * Default implementation does nothing.
     */
    public void setLayoutParameters() {

    }

    /**
     * Reset this view's state. Essentially clear it's text and hide it.
     */
    public void reset() {
        this.index = -1;

        this.capsMode = CAPS_FIRST_LETTER;

        this.rawPrediction = "";
        this.textShown = "";

        //this.longPressOccurred = false;
        this.isNewWord = false;
        this.isRoot = false;
        this.isBold = false;
    }

    /**
     * Initialize this view with attributes required for it to be displayed in the {@link SuggestionsLinearLayout}
     *
     * @param serv the {@link IKnowUKeyboardService} to be used for any callback methods
     * @param parent the parent view of this item, our {@link SuggestionsLinearLayout}
     * @param fontSize the size of the font to be displayed
     * @param fontColor the color of the font to be displayed
     * @param selectedColor the color of the background when this item is being selected
     * @param minWidth the minimum width that this view can be
     */
    public void init(IKnowUKeyboardService serv, SuggestionsLinearLayout parent, int fontSize, int fontColor, int selectedColor, int minWidth) {
        this.parent = parent;
        this.service = serv;
        this.fontColor = fontColor;

        this.selectedColor = selectedColor;

        this.minWidth = minWidth;

        //this.bg.setColorFilter(Theme.KEY_COLOR, PorterDuff.Mode.SRC_ATOP );
        //this.setBackgroundResource(R.drawable.suggestion_bg_state_list);
        //this.setBackgroundDrawable(this.bg);
    }

    /**
     * Set the text of this view to be displayed.
     *
     * Must override to show text, default implementation does nothing.
     *
     * @param index the index to use when identifying any actions that occur on this view
     * @param rootText the root text to be displayed if any
     * @param suggestion the text of the prediction or correction
     * @param capsMode the capitalization mode of this item
     */
    public void setSuggestion( int index, String rootText, String suggestion, int capsMode, int textMode ) {

    }

    public void setMinWidth(int width) {
        this.minWidth = width;
    }

    /**
     * Set the text of this textview to be bold and colored
     *
     */
    public void setBold() {
        this.isBold = true;
    }

    /**
     * Set the index
     *
     * @param idx the index to set
     */
    public void setIndex(int idx) {
        this.index = idx;
    }

    /**
     * Get the raw, untouched prediction of this view
     *
     * @return the raw untouched prediction
     */
    public String getRawSuggestion() {
        return this.rawPrediction;
    }

    /**
     * Set that a long-press has occurred on this view
     */
    public void setLongPressOccurred() {
        this.longPressOccurred = true;
    }

    /**
     * Set if this word is a new word
     */
    public void setAsNewWord() {
        this.isNewWord = true;
    }

    /**
     * Set this word as the root word currently in the engine
     */
    public void setAsRoot() {
        this.isRoot = true;
    }

    /**
     * Retrieve whether or not this word is a new word
     *
     * @return
     */
    public boolean isNewWord() {
        return this.isNewWord;
    }

    /**
     * Retrieve whether or not this word is the root word
     *
     * @return
     */
    public boolean isRoot() {
        return this.isRoot;
    }

    public int getCapsMode() {
        return this.capsMode;
    }

    public int getWidth() {
        return this.width;
    }

    public int getHeight() {
        return this.height;
    }

    public void setPosition(int x, int y) {
        this.xpos = x;
        this.ypos = y;

        this.bgRect = new RectF(x+3, y+2, x+this.width-3, y+this.height);
        //IKnowUKeyboardService.log(Log.VERBOSE, "setPosition()", "left = "+ bgRect.left+", right = "+bgRect.right+", top = "+bgRect.top+", bottom = "+bgRect.bottom);
    }

    /**
     * Check to see if a pair of x and y coordinates fall inside this view
     *
     * @param x the x-coord
     * @param y the y-coord
     * @return whether or not hte point is inside this view
     */
    public boolean isInside(float x, float y) {
        if ( xpos < x && x < (xpos + width) && ypos < y && y < (ypos + height) ) return true;
        return false;
    }

    public void setPressed(boolean press) {
        this.isPressed = press;
    }

    /**
     * Performa draw of this Suggestions content, default implementation does nothing.
     * @param canvas
     */
    public void onDraw(Canvas canvas) {

    }

    /**
     * Handle a touch event on this Suggestion, default implementation always returns false;
     * @param event
     * @return
     */
    public boolean onTouch(MotionEvent event) {
        return false;
    }
}
