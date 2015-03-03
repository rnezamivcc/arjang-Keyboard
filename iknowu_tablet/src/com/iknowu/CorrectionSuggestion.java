package com.iknowu;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.LinearGradient;
import android.graphics.Paint;
import android.graphics.RectF;
import android.graphics.Shader;
import android.util.Log;
import android.view.MotionEvent;

import com.iknowu.asian.Hangul;
import com.iknowu.util.Theme;

/**
 * An individual item to hold a prediction/correction and be displayed by the {@link SuggestionsLinearLayout}.
 * 
 * @author Justin Desjardins
 *
 */
public class CorrectionSuggestion extends Suggestion {

    public static final String TAG = "DEBUG";

    private int keyPaddingLR = 2;
    private Paint paint;
	
	public CorrectionSuggestion(Context context) {
		super(context);
		//this.createBackgroundDrawable();
        this.paint = new Paint();
        this.paint.setAntiAlias(true);
	}

    /**
     * Set the position and dimensions for this view
     */
    public void setLayoutParameters() {

        this.width = (int) this.paint.measureText(this.textShown) + (2 * PADDING_LEFT_RIGHT);

        this.marginLeft = PADDING_LEFT_RIGHT;

        if (this.width < this.minWidth) {
            this.marginLeft = ( ( this.minWidth - this.width ) / 2) + PADDING_LEFT_RIGHT;
            this.width = this.minWidth;
        }
        this.height = this.fontSize + ( 2 * PADDING_TOP_BOTTOM);

        this.marginTop = (this.height / 2) + (this.fontSize / 2);

    }
	
	/**
	 * Create the drawable to be used as the background for this view
	 */
	private void createBackgroundDrawable() {
		this.bg = this.context.getResources().getDrawable(R.drawable.suggestion_bg_state_list);
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
        this.fontSize = fontSize;
		
		this.selectedColor = selectedColor;

		this.minWidth = minWidth;

        this.paint.setTypeface(Theme.TYPEFACE);
        this.paint.setColor(fontColor);
        this.paint.setTextSize(fontSize);
		
		//this.bg.setColorFilter(Theme.KEY_COLOR, PorterDuff.Mode.SRC_ATOP );
		//this.setBackgroundResource(R.drawable.suggestion_bg_state_list);
		//this.setBackgroundDrawable(this.bg);
	}
	
	/**
	 * Set the text of this view to be dispalyed.
	 * 
	 * @param index the index to use when identifying any actions that occur on this view
	 * @param rootText the root text to be displayed if any
	 * @param suggestion the text of the prediction or correction
	 * @param capsMode the capitalization mode of this item
	 */
	public void setSuggestion( int index, String rootText, String suggestion, int capsMode, int textMode ) {
		this.index = index;
		this.capsMode = capsMode;
		
		switch (textMode) {
			case TEXT_MODE_LATIN:
				this.setLatinText(rootText, suggestion);
				break;
			case TEXT_MODE_KOREAN:
				this.setKoreanText(rootText, suggestion);
				break;
		}

        this.setLayoutParameters();
	}
	
	private void setLatinText(String rootText, String suggestion) {

        //IKnowUKeyboardService.log(Log.VERBOSE, "setLatinText()", "root = "+rootText+", sugg = "+suggestion);
		//if (suggestion.equals("...")) {
		if (suggestion.equals("   ")) {
			this.rawPrediction = rootText;
			this.textShown = rootText + suggestion;
		} else if (rootText != null && rootText.equals("+ ")) {
			this.rawPrediction = suggestion;
			this.textShown = rootText + suggestion;
		} else {
			this.rawPrediction = suggestion;
			this.textShown = suggestion;
		}

		this.isNewWord = false;
	}
	//////////////////////////////////////////////////////////////////////////////////////
	private void setKoreanText(String rootText, String suggestion) {
		this.rawPrediction = suggestion;
		this.textShown = "";
		
		this.textShown = Hangul.combineChars(suggestion, false);

		this.isNewWord = false;
	}

    @Override
    public void onDraw(Canvas canvas) {
        switch(Theme.KEY_STYLE) {
			/*
			 * Style for regular
			 */
            case IKnowUKeyboardView.THEME_STYLE_REGULAR:
                paint.setStyle(Paint.Style.FILL);
                if (this.isPressed) {
                    paint.setColor(Theme.KEY_PRESSED_COLOR);
                    canvas.drawRoundRect(this.bgRect, Theme.CORNER_RADIUS_X, Theme.CORNER_RADIUS_Y, paint);
                    //if (!this.mPopupKeyboardActive && !this.mSearchMode)
                    //	this.showPreview(key);
                } else {
                    this.shadowRect = new RectF( (this.bgRect.left+2), (this.bgRect.top+2), (this.bgRect.right+2), (this.bgRect.bottom+2) );
                    paint.setColor(Theme.KEY_SHADOW_COLOR);
                    canvas.drawRoundRect(shadowRect, Theme.CORNER_RADIUS_X, Theme.CORNER_RADIUS_Y, paint);

                    paint.setColor(Theme.KEY_COLOR);
                    if (Theme.USE_GRADIENT) {
                        switch (Theme.GRADIENT_DIRECTION) {
                            case IKnowUKeyboardView.THEME_GRADIENT_DIAGONAL:
                                paint.setShader(new LinearGradient(this.bgRect.left, this.bgRect.top, this.bgRect.right, this.bgRect.bottom, Theme.KEY_PRESSED_COLOR, Theme.KEY_COLOR, Shader.TileMode.REPEAT));
                                break;
                            case IKnowUKeyboardView.THEME_GRADIENT_VERTICAL:
                                paint.setShader(new LinearGradient(this.bgRect.left, this.bgRect.top, this.bgRect.left, this.bgRect.bottom, Theme.KEY_PRESSED_COLOR, Theme.KEY_COLOR, Shader.TileMode.REPEAT));
                                break;
                            case IKnowUKeyboardView.THEME_GRADIENT_HORIZONTAL:
                                paint.setShader(new LinearGradient(this.bgRect.left, this.bgRect.top, this.bgRect.right, this.bgRect.top, Theme.KEY_PRESSED_COLOR, Theme.KEY_COLOR, Shader.TileMode.REPEAT));
                                break;
                        }
                    }
                    canvas.drawRoundRect(this.bgRect, Theme.CORNER_RADIUS_X, Theme.CORNER_RADIUS_Y, paint);
                }
                break;
			/*
			 * Styling for outline
			 */
            case IKnowUKeyboardView.THEME_STYLE_OUTLINE:
                paint.setStyle(Paint.Style.STROKE);
                paint.setStrokeWidth(Theme.BORDER_STROKE);
                if (this.isPressed) {
                    paint.setColor(Theme.KEY_PRESSED_COLOR);
                    canvas.drawRoundRect(this.bgRect, Theme.CORNER_RADIUS_X, Theme.CORNER_RADIUS_Y, paint);
                    //if (!this.mPopupKeyboardActive && !this.mSearchMode)
                    //	this.showPreview(key);
                } else {
                    paint.setStyle(Paint.Style.FILL);
                    this.shadowRect = new RectF( this.bgRect.left, this.bgRect.top, this.bgRect.right, this.bgRect.bottom );
                    paint.setColor(Theme.KEY_SHADOW_COLOR);
                    canvas.drawRoundRect(shadowRect, Theme.CORNER_RADIUS_X, Theme.CORNER_RADIUS_Y, paint);
                    paint.setStyle(Paint.Style.STROKE);

                    paint.setColor(Theme.KEY_COLOR);
                    if (Theme.USE_GRADIENT) {
                        switch (Theme.GRADIENT_DIRECTION) {
                            case IKnowUKeyboardView.THEME_GRADIENT_DIAGONAL:
                                paint.setShader(new LinearGradient(this.bgRect.left, this.bgRect.top - 3, this.bgRect.right, this.bgRect.bottom + 3, Theme.KEY_PRESSED_COLOR, Theme.KEY_COLOR, Shader.TileMode.REPEAT));
                                break;
                            case IKnowUKeyboardView.THEME_GRADIENT_VERTICAL:
                                paint.setShader(new LinearGradient(this.bgRect.left, this.bgRect.top - 3, this.bgRect.left, this.bgRect.bottom + 3, Theme.KEY_PRESSED_COLOR, Theme.KEY_COLOR, Shader.TileMode.REPEAT));
                                break;
                            case IKnowUKeyboardView.THEME_GRADIENT_HORIZONTAL:
                                paint.setShader(new LinearGradient(this.bgRect.left - 3, this.bgRect.top - 3, this.bgRect.right + 3, this.bgRect.top - 3, Theme.KEY_PRESSED_COLOR, Theme.KEY_COLOR, Shader.TileMode.REPEAT));
                                break;
                        }
                    }

                    canvas.drawRoundRect(this.bgRect, Theme.CORNER_RADIUS_X, Theme.CORNER_RADIUS_Y, paint);
                }
                break;
			/*
			 * Style for lines between
			 */
            case IKnowUKeyboardView.THEME_STYLE_LINESBETWEEN:
                paint.setStyle(Paint.Style.FILL);
                paint.setStrokeWidth(Theme.BORDER_STROKE);
                if (this.isPressed) {
                    paint.setColor(Theme.KEY_PRESSED_COLOR);
                    canvas.drawRoundRect(this.bgRect, Theme.CORNER_RADIUS_X, Theme.CORNER_RADIUS_Y, paint);
                    //this.previewText.setBackgroundColor(this.backgroundColor);
                    //if (!this.mPopupKeyboardActive && !this.mSearchMode)
                    //	this.showPreview(key);
                } else {
                    paint.setColor(Theme.KEY_COLOR);
                    //canvas.drawLine(keyRect.right + mPadRight, keyRect.top, keyRect.right + mPadRight, keyRect.bottom, mPaint);
                    //canvas.drawLine(keyRect.left + mPadLeft, keyRect.bottom, keyRect.right + mPadRight + (STROKE_WIDTH/2), keyRect.bottom, mPaint);
                    canvas.drawLine(this.bgRect.right + keyPaddingLR, this.bgRect.top, this.bgRect.right + keyPaddingLR, this.bgRect.bottom, paint);
                    canvas.drawLine(this.bgRect.left + keyPaddingLR, this.bgRect.bottom, this.bgRect.right + keyPaddingLR + (Theme.BORDER_STROKE/2), this.bgRect.bottom, paint);
                }
                break;
        }

        this.paint.setShader(null);
        this.paint.setStyle(Paint.Style.FILL);
        if (this.isBold) {
            this.paint.setTypeface(Typefaces.get(this.context, "roboto_bold.ttf"));
            this.paint.setColor(Theme.CANDIDATE_HIGHEST_PRIORITY_COLOR);
        } else {
            this.paint.setTypeface(Typefaces.get(this.context, "roboto_bold.ttf"));
            this.paint.setColor(Theme.KEY_TEXT_COLOR);
        }
        canvas.drawText(this.textShown, this.xpos+this.marginLeft, this.ypos + this.marginTop, this.paint);
    }

    @Override
    public boolean onTouch(MotionEvent event) {
        try {
            //Log.d("TEXTVIEW ON TOUCH EVENT", "event = "+event.getAction());

            int action = event.getAction();

            switch (action) {
                case MotionEvent.ACTION_DOWN:

                    IKnowUKeyboardService.log(Log.VERBOSE, TAG, "onTouchEvent() -- ACTION_DOWN");
                    //this.setBackgroundColor(this.selectedColor);

                    this.sawDownEvent = true;
                    this.setPressed(true);
                    this.parent.sendLongPressMessage(this.index);

                    break;
                case MotionEvent.ACTION_MOVE:
                    //this.parent.cancelLongPressMessage();
                    //this.longPressOccurred = false;
                    this.setPressed(true);
                    return false;
                case MotionEvent.ACTION_UP:

                    IKnowUKeyboardService.log(Log.VERBOSE, TAG, "onTouchEvent() -- ACTION_UP");
                    this.parent.cancelLongPressMessage();
                    this.setPressed(false);
                    this.parent.sendKeyClick();
                    if (!this.longPressOccurred && this.sawDownEvent) {
                        if (this.service.mAutoLearnOn) {
                            if (this.isNewWord) {
                                this.service.manualCorrect(this.rawPrediction, this.isNewWord, true, 0);
                            } else {
                                this.service.manualCorrect(this.rawPrediction, this.isNewWord, true, this.capsMode);
                            }
                        } else {
                            this.service.manualCorrect(this.rawPrediction, true, this.capsMode);
                            //this.service.insertCorrection(this.rawPrediction, false);
                        }
                    }
                    this.sawDownEvent = false;
                    this.longPressOccurred = false;

                    break;
                case MotionEvent.ACTION_CANCEL:
                    this.parent.cancelLongPressMessage();
                    this.setPressed(false);

                    this.sawDownEvent = false;
                    this.longPressOccurred = false;
                    return false;
            }
            return true;
        } catch (Exception e) {
            IKnowUKeyboardService.sendErrorMessage(e);
            return false;
        }
    }
}
