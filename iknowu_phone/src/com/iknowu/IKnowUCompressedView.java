package com.iknowu;

import android.content.Context;
import android.content.res.Resources;
import android.graphics.Canvas;
import android.graphics.LinearGradient;
import android.graphics.Paint;
import android.graphics.Path;
import android.graphics.PorterDuff;
import android.graphics.Rect;
import android.graphics.RectF;
import android.graphics.Shader;
import android.graphics.Typeface;
import android.graphics.drawable.Drawable;
import android.os.Message;
import android.util.AttributeSet;
import android.util.Log;
import android.view.Gravity;
import android.view.MotionEvent;
import android.widget.PopupWindow;
import android.widget.TextView;

import com.iknowu.IKnowUKeyboard.Key;
import com.iknowu.util.Theme;

import java.io.IOException;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;

/**
 * Currently in testing, this class will need to be fleshed out.
 * 
 * The compressed view is a subclass of {@link IKnowUKeyboardView} that displays its keys in a non-tradtitional matter
 * The idea behind the compressed view is to show the keys in a single row to reduce the amount of screen
 * space the keyboard takes up
 * @author Justin
 *
 */
public class IKnowUCompressedView extends IKnowUKeyboardView {
	
	private static final int MOVING_STOPPED = 0;
	private static final int MOVING_LEFT = -1;
	private static final int MOVING_RIGHT = 1;
	
	private static final int MAX_POINTS = 50;
	
	private static final float VELOCITY_THRESHOLD = 0.05f;
	public static final float X_THRESHOLD = 0.15f;
	public static final float Y_THRESHOLD = 0.5f;
	
	private static final long POPUP_ANIMATION_LENGTH = 350;
	
	private static final float PADDING_TOP = 7.5f;
	
	private Context context;
	private Paint paint;
	private RectF keyRect;
	private int keyPaddingLR = 1;
	private int keyPaddingTB = 0;
	
	private int leastX;
	private int mostX;
	private int currentDirection = 0; //0 = stopped, 1 = right, -1 = left
	
	private boolean inTouch;
	private LinkedList<Vector2> points;
	private LinkedList<PopupWindow> popups;
	
	private long lastMoveTime;
	private float lastVelocity;
	
	private float lastVelX;
	private float lastVelY;
	
	private String composingWord;
	private int lastAdded;
	
	private boolean wentInSuggestions;
	
	private String curChar;
	private float curCharHeight;
	private float curCharXPos;
	private float curCharYPos;
	private Rect curCharBounds;

    private float predictedTextSize;
	
	//========================================================================================
	//CONTRUCTORS
	//========================================================================================
	public IKnowUCompressedView(Context context, AttributeSet attrs) throws IOException {
		super(context, attrs);
		this.context = context;
		if (this.paint == null) this.paint = new Paint();
		this.points = new LinkedList<Vector2>();
		this.popups = new LinkedList<PopupWindow>();
		paint.setAntiAlias(true);
	}
	
	public IKnowUCompressedView(Context context, AttributeSet attrs, int defStyle) {
		super(context, attrs, defStyle);
		this.context = context;
		if (this.paint == null) this.paint = new Paint();
		this.points = new LinkedList<Vector2>();
		this.popups = new LinkedList<PopupWindow>();
		paint.setAntiAlias(true);
	}
	
	//========================================================================================
	//METHODS
	//========================================================================================
	
	@Override
	public void processAttributes() {
		super.processAttributes();
		
		Resources r = context.getResources();

        mKeyTextSize = r.getDimensionPixelSize(R.dimen.compressed_tablet_keytextsize);
        mLabelTextSize = r.getDimensionPixelSize(R.dimen.compressed_tablet_labeltextsize);
        this.predictedTextSize = r.getDimensionPixelSize(R.dimen.compressed_tablet_predicted_text_size);

        //this.setPrivateTheme(themeId);
	}

    /**
     * Set the theme of this keyboard
     * @param themeId the theme to load
     */
    /*private void setPrivateTheme(int themeId) {
        XmlResourceParser parser;
        try {
            parser = this.context.getResources().getXml(themeId);
        } catch (Resources.NotFoundException nfe) {
            parser = this.context.getResources().getXml(IKnowUKeyboardService.DEFAULT_KEYBOARD_THEME_ID);
        }

        try {
            int event;
            while ((event = parser.next()) != XmlResourceParser.END_DOCUMENT) {
                if (event == XmlResourceParser.START_TAG) {
                    String tag = parser.getName();
                    if (TAG_THEME_ITEM.equals(tag)) {
                        String attrName = parser.getAttributeValue(null, "name");

                        if (attrName.equals("compressedColorMoreThan5")) {
                            this.keyColorMoreThan5 = parser.getAttributeIntValue(null, "value", 0xFF007399);
                        } else if (attrName.equals("compressedColorLessThan5")) {
                            this.keyColorLessThan5 = parser.getAttributeIntValue(null, "value", 0xFF009926);
                        }
                    }
                } else if (event == XmlResourceParser.END_TAG) {}
            }
        } catch (Exception e) {
            IKnowUKeyboardService.sendErrorMessage(e);
        }
    }*/
	
	/**
	 * Process a touch event on this view
	 */
	@Override
	public boolean onTouchEvent(MotionEvent ev) {
		try {
			int action = ev.getAction();
			
			this.releaseWithNoConsequence = false;
				
			if (action == MotionEvent.ACTION_CANCEL) {
				this.touchDown = false;
				mHandler.removeMessages(MSG_LONGPRESS);
				mHandler.removeMessages(MSG_DELETEWORDFROMEDITOR);
				mHandler.removeMessages(MSG_LONGPAUSE);
				
				int x = (int) ev.getX();
				int y = (int) ev.getY();
				
				if (this.getKeyboard().currentPressedKey != null)
					this.getKeyboard().currentPressedKey.pressed = false;
				//Key key = this.keyboard.getKeyAndSetWidths(x, y);
				final Key key = this.keyboard.getKeyAndShiftX(0, x, y);
				//Key key = this.keyboard.getKeyFromCoords(x, y);
				if (key != null) {
					key.pressed = false;
					//return true;
				}
				this.hidePreview();
			}

            /**
             * if showing a popup keyboard
             */
            if (mPopupKeyboardActive) {
                final boolean ret = this.popupTouchEvent(ev);
                if (ret) return ret;
            }
	        
	        int pointerCount = ev.getPointerCount();
	        int multiAction = -1;
	        int pointerIndex = -1;
			//Log.d("POINTER COUNT = "+pointerCount, "action = "+action);
			
			if (pointerCount > 1) {
				// Extract the index of the pointer that left the touch sensor
		        pointerIndex = (action & MotionEvent.ACTION_POINTER_INDEX_MASK) >> MotionEvent.ACTION_POINTER_INDEX_SHIFT;
				multiAction = action & MotionEvent.ACTION_MASK;
			}
	        
			//IKnowUKeyboardService.log(Log.VERBOSE, "keyboardView", "onTouchEvent action = "+action);
			
			if (action == android.view.MotionEvent.ACTION_DOWN || multiAction == MotionEvent.ACTION_POINTER_DOWN) {
				
				this.touchDown = true;
				this.inTouch = true;
				this.lastVelocity = 0f;
				this.composingWord = "";
				this.lastAdded = -1;
				
				if (pointerIndex > 0) {
					lastTouchX = (int) ev.getX(pointerIndex);
					lastTouchY = (int) ev.getY(pointerIndex);
					this.touchStartX = (int) ev.getX(pointerIndex);
					this.cancelSwipeDetect = true;
				} else {
					lastTouchX = (int) ev.getX();
					lastTouchY = (int) ev.getY();
					this.touchStartX = (int) ev.getX();
				}
				//this.startY = (int) ev.getY();
				//fireIndex = -1;
				
				/**
				 * Find out if this touch lands in a key
				 */
				int x = (int) ev.getX();
				int y = (int) ev.getY();
				
				//Key key = this.keyboard.getKeyAndSetWidths(x, y);
				//final Key key = this.keyboard.getKeyAndShiftX(-x, x, y);
				//Key key = this.keyboard.getKeyFromCoords(x, y);
				Key key = this.keyboard.getKeyFromCoords((int)x, (int)y);
				if (key != null) {
					this.onPressKey(key);
					this.showPreview(key);

                    //Utilize a custom long press to allow for the user to change this in the settings
                    Message msg = mHandler.obtainMessage(MSG_LONGPRESS);
                    msg.arg1 = x;
                    msg.arg2 = y;
                    mHandler.sendMessageDelayed(msg, Theme.LONG_PRESS_TIMEOUT);

					invalidate();
				}
				
				this.lastMoveTime = System.currentTimeMillis();
				
			} else if (action == android.view.MotionEvent.ACTION_UP) {
				
				this.touchDown = false;
				this.clearPoints();

				mHandler.removeMessages(MSG_LONGPRESS);
				mHandler.removeMessages(MSG_DELETEWORDFROMEDITOR);

                boolean gestureOccured = this.checkForGesture(ev.getX(), ev.getY());
                if (gestureOccured) return true;

				/**
				 * find a key that may have been pressed
				 */
				int x = (int) ev.getX();
				int y = (int) ev.getY();
				
				this.setMovingStopped(x);
				
				if (this.getKeyboard().currentPressedKey != null)
					this.getKeyboard().currentPressedKey.pressed = false;
				//Key key = this.keyboard.getKeyFromCoords(x, y);
				Key key = this.keyboard.getKeyFromCoords((int)x, (int)y);
				//Key key = this.keyboard.getKeyAndSetWidths(x, y);
				//final Key key = this.keyboard.getKeyAndShiftX(0, x, y);
				
				if (key != null) {
					key.pressed = false;
					this.onReleaseKey(key);
					invalidate();
				}
				
				this.keyboard.resetKeys();
				
			} else if (action == android.view.MotionEvent.ACTION_MOVE) {
				float x = ev.getX();
				final float y = ev.getY();
				
				//IKnowUKeyboardService.log(Log.VERBOSE, "Action move", "x = "+x+", y = "+y);
				
				//if sliding up to the suggestions view, then pass this event off to it
				//if (y < 0) {
				//	this.suggestionsView.onTouchEvent(ev, this.getTop());
				//	this.wentInSuggestions = true;
				//} else {
					//if we flagged that we sent touch events to the suggestions view, then
					//tell it to remove any long press messages that it may have fired off
					if (this.wentInSuggestions) {
						this.suggestionsView.cancelLongPressMessage();
						this.wentInSuggestions = false;
					}
					
					final float xdist = x - lastTouchX;
					final float ydist = y - lastTouchY;
									
					if (xdist > 5) {
						mHandler.removeMessages(MSG_LONGPRESS);
						mHandler.removeMessages(MSG_DELETEWORDFROMEDITOR);
					}
					
					final float dtime = (float) (System.currentTimeMillis() - this.lastMoveTime);
					float xVel = (float) (xdist / dtime);
					final float yVel = (float) (ydist / dtime);
					
					if (yVel < 0 && yVel < -Y_THRESHOLD && Math.abs(xVel) < X_THRESHOLD ) {
						x = this.lastTouchX;
						xVel = 0;
					}
					
					Key current = null;
					if (this.getKeyboard().currentPressedKey != null) {
						this.getKeyboard().currentPressedKey.pressed = false;
						current = this.getKeyboard().currentPressedKey;
					}
					//Key key = this.keyboard.getKeyAndSetWidths(x, y);
					//final Key key = this.keyboard.getKeyAndShiftX((int) -x, x, y);
					
					//this.addCharByVelocity(xVel, yVel, key);
					//this.addCharByTouchZone(key, y);
					
					this.lastMoveTime = System.currentTimeMillis();
					//this.tryAddPoint(x, y);
					
					//IKnowUKeyboardService.log(Log.INFO, "onTouch Move", "velocity = "+velocity);
					
					Key key = this.keyboard.getKeyFromCoords((int)x, (int)y);
					if (key != null) {
						key.pressed = true;
						this.showPreview(key);
						//IKnowUKeyboardService.log(Log.VERBOSE, "current pressed", "= "+current.label);
						/*
						if ( dtime > 100 && ( current != null && !current.equals(key) ) ) {
							this.onReleaseKey(key);
						}
						*/
					}
					
					lastTouchX = (int) x;
					lastTouchY = (int) y;
					this.lastVelX = xVel;
					this.lastVelY = yVel;
					
					invalidate();
					//return true;
				//}
			}
			
			return true;
		} catch (Exception e) {
			IKnowUKeyboardService.sendErrorMessage(e);
			return false;
		}
	}

    private boolean checkForGesture(float touchx, float touchy) {
        /*
        if ( (touchx - this.touchStartX) > GESTURE_X_COORD_THRESHOLD && (this.touchStartY - touchy) > (GESTURE_Y_COORD_THRESHOLD / 2) ) {
            keyboardService.switchToFullSize();
        }
        */
        return false;
    }

    @Override
    /**
     * Set that a key has been released, and send it to the editor
     * @param key the key that has been released
     */
    public void onReleaseKey(Key key) {
        key.pressed = false;
        this.onKey(key);

        mHandler.removeMessages(MSG_LONGPRESS);
        mHandler.removeMessages(MSG_DELETEWORDFROMEDITOR);
        mHandler.removeMessages(MSG_LONGPAUSE);

        if (this.onkey) {
            IKnowUKeyboardService.log(Log.VERBOSE, "IKnowUKeyboardView", "onReleaseKey(), this.onKey = true");
            this.prepareNextKeyHighlighting();

            if (!this.ignoreShift) {
                keyboardService.updateShiftKeyState(keyboardService.getCurrentInputEditorInfo());
            }

            this.ignoreShift = false;

            keyboardService.updateWordToCorrect();

            if (this.updatePredEngineOnRelease) {
                this.updatePredEngineOnRelease = false;
                keyboardService.updatePredictionEngineOnCursorPosition(false, true);
            }

            keyboardService.updateCandidates();
            this.invalidate();
        }
    }

	/**
	 * Add a point to the list of points to be displayed as the trailing line
	 * behind the user's swiping
	 * @param x the x-coordinate
	 * @param y the y-coordinate
	 */
	private void tryAddPoint(float x, float y) {
		if (this.points.size() >= MAX_POINTS) {
			this.points.remove();
		}
		
		Vector2 point = new Vector2(x,y);
        points.add(point);
	}
	
	/**
	 * Remove all of the points
	 */
	private void clearPoints() {
		this.inTouch = false;
		this.points.clear();
	}
	
	/**
	 * Try to add a character to the text, do this by checking the x and y velocities for a distinct swipe up gesture on a key 
	 * 
	 * @param xvel the velocity on the x-axis
	 * @param yvel the velocity on the y-axis
	 * @param key the key that is being touched
	 */
	private void addCharByVelocity( float xvel, float yvel, Key key) {
		//if under the x threshold and over the y threshold then probably want to add this character
		//IKnowUKeyboardService.log(Log.VERBOSE, "trying to add xvel = "+xvel, "yvel = "+yvel);
		if (Math.abs(xvel) <= X_THRESHOLD && yvel <= -Y_THRESHOLD && key != null && this.lastAdded != key.codes[0]) {
			//this.composingWord += key.label;
			this.lastAdded = key.codes[0];
			IKnowUKeyboardService.log(Log.INFO, "adding key, xvel = "+xvel + ", yvel = "+yvel, "sending key = "+key.label);
			this.onReleaseKey(key);
			this.showPopupAnimation(key);
		}
		/*
		else if ( key != null && key.codes[0] == IKnowUKeyboard.KEYCODE_SPACE && this.lastAdded != IKnowUKeyboard.KEYCODE_SPACE ) {
			this.onReleaseKey(key);
			this.composingWord = "";
			this.lastAdded = IKnowUKeyboard.KEYCODE_SPACE;
		} else if ( key != null && key.codes[0] == IKnowUKeyboard.KEYCODE_DELETE && this.lastAdded != IKnowUKeyboard.KEYCODE_DELETE ) {
			this.onReleaseKey(key);
			this.composingWord = "";
			this.lastAdded = IKnowUKeyboard.KEYCODE_DELETE;
		}
		*/
		else if ( Math.abs(xvel) >= X_THRESHOLD && this.lastAdded != IKnowUKeyboard.KEYCODE_SPACE && this.lastAdded != IKnowUKeyboard.KEYCODE_DELETE ) {
			this.lastAdded = -1;
		}
		//this.lastVelocity = velocity;
	}
	
	private void addCharByTouchZone(Key key, float y) {
		/*
		float yarea = key.height - (key.height * Key.TOUCH_Y_PERCENT);
		
		//if in the special hit zone for the key then add it
		//IKnowUKeyboardService.log(Log.VERBOSE, "trying to add xvel = "+xvel, "yvel = "+yvel);
		if (yarea < y && y < (key.y + key.height) && key != null && this.lastAdded != key.codes[0]) {
			//this.composingWord += key.label;
			this.lastAdded = key.codes[0];
			//IKnowUKeyboardService.log(Log.INFO, "adding key, xvel = "+xvel + ", yvel = "+yvel, "sending key = "+key.label);
			keyboardService.audioAndClickFeedback(1);
			this.onReleaseKey(key);
			this.showPopupAnimation(key);
		}
		*/
		/*
		else if ( key != null && key.codes[0] == IKnowUKeyboard.KEYCODE_SPACE && this.lastAdded != IKnowUKeyboard.KEYCODE_SPACE ) {
			this.onReleaseKey(key);
			this.composingWord = "";
			this.lastAdded = IKnowUKeyboard.KEYCODE_SPACE;
		} else if ( key != null && key.codes[0] == IKnowUKeyboard.KEYCODE_DELETE && this.lastAdded != IKnowUKeyboard.KEYCODE_DELETE ) {
			this.onReleaseKey(key);
			this.composingWord = "";
			this.lastAdded = IKnowUKeyboard.KEYCODE_DELETE;
		}
		*/
	}
	
	/**
	 * Show and animation that sends a popup window flying up towards the input indicating that that key has been pressed
	 * 
	 * @param key the key that was sent ot the editor and should be used to get the display label from
	 */
	private void showPopupAnimation(Key key) {
		TextView tv = new TextView(this.context);
		tv.setText(key.label);
		tv.setTextColor(Theme.KEY_TEXT_COLOR);
		tv.setBackgroundColor(Theme.KEY_COLOR);
		PopupWindow popWindow = new PopupWindow(tv, 50, 50, false);
		popWindow.setAnimationStyle(R.style.PopupWindowAnimation);
		popWindow.showAtLocation(this, Gravity.NO_GRAVITY, key.x, key.y);
		this.popups.add(popWindow);
		Message msg = mHandler.obtainMessage(MSG_REMOVE_POPUP);
		mHandler.sendMessageDelayed(msg, POPUP_ANIMATION_LENGTH);
	}
	
	/*
	@Override
	public void hidePreview() {
		try {
			this.popups.remove().dismiss();
		} catch (Exception e) {
			
		}
	}
	*/
	
	/**
	 * Check for a direction change in the user's swiping motion
	 * @param sendKey if true, sends a key press on a direction change
	 * @param x the current x-coordinate
	 * @param y the current y-coordinate
	 */
	private void checkForDirectionChange(boolean sendKey, int x, int y) {
		//IKnowUKeyboardService.log(Log.VERBOSE, "CheckFor Direction Change", "currentDir = "+this.currentDirection);
		switch (this.currentDirection) {
		case MOVING_STOPPED:
			if ( this.currentDirection == MOVING_STOPPED ) {
				if ( this.mostX - x > 5 ) {
					if (sendKey) this.getAndSendKeyPress(this.mostX, y);
					this.setMovingLeft(x);
				} else if ( x - this.leastX > 5 ) {
					if (sendKey) this.getAndSendKeyPress(this.leastX, y);
					this.setMovingRight(x);
				}
			}
			break;
		case MOVING_LEFT:
			if ( x - this.leastX > 5 ) {
				if (sendKey) this.getAndSendKeyPress(this.leastX, y);
				this.setMovingRight(x);
			}
			break;
		case MOVING_RIGHT:
			if ( this.mostX - x > 5 ) {
				if (sendKey) this.getAndSendKeyPress(this.mostX, y);
				this.setMovingLeft(x);
			}
			break;
		}
	}
	
	/**
	 * Gets a key form the given coords and sends it to the editor
	 * @param x the x-coord of the current touch press
	 * @param y the y-coord of the current touch press
	 */
	private void getAndSendKeyPress(int x, int y) {
		if (this.getKeyboard().currentPressedKey != null)
			this.getKeyboard().currentPressedKey.pressed = false;
		Key key = this.keyboard.getKeyAndSetWidths(x, y);
		//Key key = this.keyboard.getKeyFromCoords(x, y);
		if (key != null) {
			IKnowUKeyboardService.log(Log.VERBOSE, "getAndSend", "key = "+key.label);
			this.onReleaseKey(key);
			this.setMovingStopped(x);
		}
	}
	
	/**
	 * Set the user's current action to stopped
	 * @param x the x-coord of the stopped position
	 */
	private void setMovingStopped(int x) {
		this.mostX = x;
		this.leastX = x;
		this.currentDirection = MOVING_STOPPED;
	}
	
	/**
	 * Set user's current action to moving left
	 * @param x the x-coord of the current touch position
	 */
	private void setMovingLeft(int x) {
		this.mostX = 0;
		this.leastX = x;
		this.currentDirection = MOVING_LEFT;
	}
	
	/**
	 * Set user's current action to moving right
	 * @param x the x-coord of the current touch position
	 */
	private void setMovingRight(int x) {
		this.leastX = 0;
		this.mostX = x;
		this.currentDirection = MOVING_RIGHT;
	}
	
	@Override
	public void onDraw(Canvas canvas) {
		try {
			if (this.getKeyboard() != null) {
				List<Key> keys = this.getKeyboard().getKeys();
				Iterator<Key> it = keys.iterator();
				
				boolean bColorKey = keyboardService.mHighlightedKeysOn;
	
				while (it.hasNext()) {
					IKnowUKeyboard.Key key = it.next();
					
					if (key.isVisible) {
						int[] needsColor = keyNeedsColor(key.codes, m_newColorKeyHighlights);
						if (needsColor[0] > 0 && needsColor[1] >= 5) {
							paintKey(key, canvas, bColorKey ? Theme.KEY_COLOR_MORE_THAN_FIVE : 0xFFFFFFFF, (char)needsColor[0], true);
						} else if (needsColor[0] > 0 && needsColor[1] >= 0) {
							paintKey(key, canvas, bColorKey ? Theme.KEY_COLOR_LESS_THAN_FIVE : 0xFFFFFFFF, (char)needsColor[0], true);
						} else {
							paintKey(key, canvas, 0xFFFFFFFF, '\0', true);
						}
					}
				}// end paint keys loop
			}
			
			if (this.inTouch) {
				this.drawPathLine(canvas);
			}
			paint.setShader(null);
		} catch (Exception e) {
	    	IKnowUKeyboardService.sendErrorMessage(e);
	    }
		
		//super.onDraw(canvas);
	}

    /**
     * Check to see if the primary code passed in matches any of the codes
     * in the array passed in, and if so, return the number of followers
     * that the key has
     * @param codes the key to check
     * @param keysToPaint the array that contains the keys that need to be highlighted
     * @return the number of followers that this key has, or -1 if none
     */
    private int[] keyNeedsColor(int[] codes, int keysToPaint[]) {
        for (int i = 0; i < keysToPaint.length; i++) {
            for (int j=0; j < codes.length; j++) {
                if (keysToPaint[i] == codes[j]) {
                    return new int[]{codes[j], numberOfFollowers[i]};
                }
            }
        }
        return new int[]{-1, -1};
    }
	
	/**
	 * Draw a line showing the path a user has traversed via their swiping touch gestures
	 * @param canvas
	 */
	private void drawPathLine(Canvas canvas) {
		paint.setStyle(Paint.Style.STROKE);
		paint.setStrokeWidth(3);
		paint.setColor(0xFF4Eb7ff);
		
		Path path = new Path();
		if (points.size() > 1) {
		    Vector2 prevPoint = null;
		    for (int i = 0; i < points.size(); i++) {
		        Vector2 point = points.get(i);

		        if (i == 0) {
		            path.moveTo(point.x, point.y);
		        } else {
		            float midX = (prevPoint.x + point.x) / 2;
		            float midY = (prevPoint.y + point.y) / 2;

		            if (i == 1) {
		                path.lineTo(midX, midY);
		            } else {
		                path.quadTo(prevPoint.x, prevPoint.y, midX, midY);
		            }
		        }
		        prevPoint = point;
		    }
		    path.lineTo(prevPoint.x, prevPoint.y);
		}
	    canvas.drawPath(path, paint);
	}
	
	/**
	 * Paints a key to the canvas
	 * @param key the key to be painted
	 * @param canvas the canvas on which to paint the key
	 * @param colorRef the color, if any to be placed over the key, for key highlighting purposes
	 * @param bCenterText whether or not to center the text
	 */
	private void paintKey(Key key, Canvas canvas, int colorRef, char charToHighlight, boolean bCenterText) {
		try {
			bKeyDarkBackground = false;

			if (key instanceof IKnowUKeyboard.Key /* IKnowUKeyboard.LatinKey */ ) {
				//IKnowUKeyboard.LatinKey lKey = (IKnowUKeyboard.LatinKey) key;
				//bKeyDarkBackground = key.getDarkBackground();
			}
			
			//keyRect = new RectF( (key.x + mPadLeft), (key.y + mPadTop), (key.x + key.width - mPadLeft - mPadRight), (key.y + key.height - mPadBottom) );
			keyRect = new RectF( (key.x + keyPaddingLR), (key.y + keyPaddingTB), (key.x + key.width - (keyPaddingLR * 2)), (key.y + key.height - keyPaddingTB) );
			
			float width = keyRect.right - keyRect.left;
			float height = keyRect.bottom - keyRect.top;
			
			/*
			Path diamondPath = new Path();
			diamondPath.moveTo(key.x + (key.width / 2), key.y); //top point
			diamondPath.lineTo(key.x, key.y + (key.height / 2)); //left point
			diamondPath.lineTo(key.x + (key.width / 2), key.y + key.height); //bottom point
			diamondPath.lineTo(key.x + key.width, key.y + (key.height / 2)); //right point
			diamondPath.lineTo(key.x + (key.width / 2), key.y); //top point
			*/
			
			if (key instanceof IKnowUKeyboard.Key /* IKnowUKeyboard.LatinKey */ ) {
				//IKnowUKeyboard.LatinKey lKey = (IKnowUKeyboard.LatinKey) key;
				bKeyDarkBackground = key.getDarkBackground();
			}
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
					//canvas.drawPath(diamondPath, paint);
						//this.showPreview(key);
					paint.setShader(null);
					//paint.setColor(0xFFFF0000);
					//canvas.drawRect(keyRect.left, keyRect.bottom - (int)( (keyRect.bottom - keyRect.top) * Key.TOUCH_Y_PERCENT), keyRect.right, keyRect.bottom, paint);
				} else {
					this.shadowRect = new RectF( (keyRect.left+2), (keyRect.top+2), (keyRect.right+2), (keyRect.bottom+2) );
					paint.setColor(Theme.KEY_SHADOW_COLOR);
					//canvas.drawPath(diamondPath, paint);
					canvas.drawRoundRect(shadowRect, Theme.CORNER_RADIUS_X, Theme.CORNER_RADIUS_Y, paint);
					
					if (bKeyDarkBackground) {
						paint.setColor(Theme.KEY_DARK_COLOR);
						/*
						if (this.useGradient) {
							switch (this.gradientDirection) {
							case IKnowUKeyboardView.THEME_GRADIENT_DIAGONAL:
								paint.setShader(new LinearGradient(keyRect.left, keyRect.top, keyRect.right, keyRect.bottom, this.keyPressedColor, this.keyDarkColor, Shader.TileMode.REPEAT));
								break;
							case IKnowUKeyboardView.THEME_GRADIENT_VERTICAL:
								paint.setShader(new LinearGradient(keyRect.left, keyRect.top, keyRect.left, keyRect.bottom, this.keyPressedColor, this.keyDarkColor, Shader.TileMode.REPEAT));
								break;
							case IKnowUKeyboardView.THEME_GRADIENT_HORIZONTAL:
								paint.setShader(new LinearGradient(keyRect.left, keyRect.top, keyRect.right, keyRect.top, this.keyPressedColor, this.keyDarkColor, Shader.TileMode.REPEAT));
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
					//canvas.drawPath(diamondPath, paint);
					canvas.drawRoundRect(keyRect, Theme.CORNER_RADIUS_X, Theme.CORNER_RADIUS_Y, paint);
					
					paint.setShader(null);
					//paint.setColor(0xFFFF0000);
					//canvas.drawRect(keyRect.left, keyRect.bottom - (int)( (keyRect.bottom - keyRect.top) * Key.TOUCH_Y_PERCENT), keyRect.right, keyRect.bottom, paint);
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
					//canvas.drawPath(diamondPath, paint);
					canvas.drawRoundRect(keyRect, Theme.CORNER_RADIUS_X, Theme.CORNER_RADIUS_Y, paint);
					//if (!this.mPopupKeyboardActive && !this.mSearchMode)
						//this.showPreview(key);
				} else {
					paint.setStyle(Paint.Style.FILL);
					this.shadowRect = new RectF( keyRect.left, keyRect.top, keyRect.right, keyRect.bottom );
					paint.setColor(Theme.KEY_SHADOW_COLOR);
					//canvas.drawPath(diamondPath, paint);
					canvas.drawRoundRect(shadowRect, Theme.CORNER_RADIUS_X, Theme.CORNER_RADIUS_Y, paint);
					paint.setStyle(Paint.Style.STROKE);
					if (bKeyDarkBackground) {
						paint.setColor(Theme.KEY_DARK_COLOR);
                        if (Theme.USE_GRADIENT) {
                            switch (Theme.GRADIENT_DIRECTION) {
							case IKnowUKeyboardView.THEME_GRADIENT_DIAGONAL:
								paint.setShader(new LinearGradient(keyRect.left, keyRect.top - 3, keyRect.right, keyRect.bottom + 3, Theme.KEY_PRESSED_COLOR, Theme.KEY_DARK_COLOR, Shader.TileMode.REPEAT));
								break;
							case IKnowUKeyboardView.THEME_GRADIENT_VERTICAL:
								paint.setShader(new LinearGradient(keyRect.left, keyRect.top - 3, keyRect.left, keyRect.bottom + 3, Theme.KEY_PRESSED_COLOR, Theme.KEY_DARK_COLOR, Shader.TileMode.REPEAT));
								break;
							case IKnowUKeyboardView.THEME_GRADIENT_HORIZONTAL:
								paint.setShader(new LinearGradient(keyRect.left - 3, keyRect.top - 3, keyRect.right + 3, keyRect.top - 3, Theme.KEY_PRESSED_COLOR, Theme.KEY_DARK_COLOR, Shader.TileMode.REPEAT));
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
					//canvas.drawPath(diamondPath, paint);
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
					//canvas.drawPath(diamondPath, paint);
					canvas.drawRoundRect(keyRect, Theme.CORNER_RADIUS_X, Theme.CORNER_RADIUS_Y, paint);
					this.previewText.setBackgroundColor(Theme.BACKGROUND_COLOR);
					//if (!this.mPopupKeyboardActive && !this.mSearchMode)
						//this.showPreview(key);
				} else {
					if (bKeyDarkBackground) {
						paint.setColor(Theme.KEY_DARK_COLOR);
					} else {
					paint.setColor(Theme.KEY_COLOR);
					}
					//canvas.drawLine(keyRect.right + mPadRight, keyRect.top, keyRect.right + mPadRight, keyRect.bottom, paint);
					//canvas.drawLine(keyRect.left + mPadLeft, keyRect.bottom, keyRect.right + mPadRight + (STROKE_WIDTH/2), keyRect.bottom, paint);
					canvas.drawLine(keyRect.right + keyPaddingLR, keyRect.top, keyRect.right + keyPaddingLR, keyRect.bottom, paint);
					canvas.drawLine(keyRect.left + keyPaddingLR, keyRect.bottom, keyRect.right + keyPaddingLR + (Theme.BORDER_STROKE/2), keyRect.bottom, paint);
				}
				break;
			}
			
			//reset shader and fill settings
			paint.setShader(null);
			paint.setStyle(Paint.Style.FILL);

			// either draw the background or color the background of the key
            /*
			if (colorRef != 0xFFFFFFFF) {
				//Log.d("COLOR_REF =", ""+colorRef);
				paint.setColor(colorRef);
				if (this.useGradient) {
					switch (this.gradientDirection) {
					case IKnowUKeyboardView.THEME_GRADIENT_DIAGONAL:
						paint.setShader(new LinearGradient(keyRect.left, keyRect.top, keyRect.right, keyRect.bottom, this.keyPressedColor, colorRef, Shader.TileMode.REPEAT));
						break;
					case IKnowUKeyboardView.THEME_GRADIENT_VERTICAL:
						paint.setShader(new LinearGradient(keyRect.left, keyRect.top, keyRect.left, keyRect.bottom, this.keyPressedColor, colorRef, Shader.TileMode.REPEAT));
						break;
					case IKnowUKeyboardView.THEME_GRADIENT_HORIZONTAL:
						paint.setShader(new LinearGradient(keyRect.left, keyRect.top, keyRect.right, keyRect.top, this.keyPressedColor, colorRef, Shader.TileMode.REPEAT));
						break;
					}
				}
				//canvas.drawPath(diamondPath, paint);
				canvas.drawRoundRect(keyRect, this.cornerRadiusX, this.cornerRadiusY, paint);
				paint.setShader(null);
			}
			*/

			if (key.label != null) {
				boolean functionLabel = key.label.length() > 1 && key.codes[0] < 0;
				
				paint.setStyle(Paint.Style.FILL);
				if (bKeyDarkBackground && Theme.KEY_STYLE == THEME_STYLE_LINESBETWEEN) {
					paint.setColor(Theme.KEY_DARK_COLOR);
		    	} else {
		    		paint.setColor(Theme.KEY_TEXT_COLOR);
		    	}
				
				
				if (key.label.length() > 1 && key.codes.length < 2) {
					paint.setTypeface(Typeface.SANS_SERIF);
					//mLabelTextSize = (int) width-2;
					//if (this.mLabelTextSize > 40) this.mLabelTextSize = 40;
					paint.setTextSize(mLabelTextSize);
				} else {
					paint.setTypeface(Typeface.SANS_SERIF);
					//mKeyTextSize = (int) width-2;
					//if (this.mKeyTextSize > 40) this.mKeyTextSize = 40;
					paint.setTextSize(mKeyTextSize);
				}
				
				this.labelText = key.label.toString();
				this.labelWidth = key.labelWidthLower;
				// make sure you only upper case the keys used for input, not keys indicating function
	 			if (this.keyboard.isShifted() && key.codes[0] > 0) {
					// regular label lower case or upper case the main key
	 				this.labelText = key.label.toString().toUpperCase();
	 				this.labelWidth = key.labelWidthUpper;
				}
	 			
	 			//this.labelWidth = paint.measureText(this.labelText);
	 			
				if (bCenterText) {
					// regular keys center the label in their key, make sure caps don't overlap with upper labels
					if (functionLabel) {
						if (labelWidth == 0) {
							this.labelWidth = paint.measureText(key.label.toString());
						}
						canvas.drawText(this.labelText, key.x + (key.width - this.labelWidth) / 2 , key.y + (key.height - (keyPaddingTB * 2)) / 2 + 
									(paint.getTextSize() - paint.descent())/2 + keyPaddingTB, paint);
						
					} else if ( key.label.equals(".") || key.label.equals(",") ) {
                        this.paint.setTextSize(mKeyTextSize);
                        this.paint.setTypeface(Typeface.create(this.font, Typeface.NORMAL));
                        if (labelWidth == 0) {
                            this.labelWidth = paint.measureText(key.label.toString());
                        }
                        canvas.drawText(this.labelText, key.x + (key.width - this.labelWidth) / 2 , key.y + (key.height - (keyPaddingTB * 2)) / 2 +
                                (paint.getTextSize() - paint.descent())/2 + keyPaddingTB, paint);
                    } else {
						for (int i=0; i < this.labelText.length(); i++) {
							this.curChar = ""+this.labelText.charAt(i);
							if (this.keyboard.isShifted() && key.codes[0] > 0) this.curChar = this.curChar.toUpperCase();

                            if (charToHighlight != '\0' && charToHighlight == this.labelText.charAt(i)) {
                                this.paint.setTextSize(this.predictedTextSize);
                                this.paint.setTypeface(Typeface.create(this.font, Typeface.BOLD));
                                this.paint.setColor(colorRef);
                            } else {
                                this.paint.setTextSize(mKeyTextSize);
                                this.paint.setTypeface(Typeface.create(this.font, Typeface.NORMAL));
                                this.paint.setColor(Theme.KEY_TEXT_COLOR);
                            }

							this.curCharBounds = new Rect();
							paint.getTextBounds(this.curChar, 0, 1, this.curCharBounds);
							this.curCharHeight = this.curCharBounds.bottom - this.curCharBounds.top;
							
							this.curCharXPos = keyRect.left + (width * (0.5f * i) ) - ( paint.measureText(this.curChar) * (0.5f * i) );
							this.curCharYPos = keyRect.top + (height * (0.5f * i) ) + ( this.curCharHeight * (0.5f * (2-i)) + (PADDING_TOP * (2-i)) );
                            if (this.curChar.charAt(0) == 'y' || this.curChar.charAt(0) == 'q') this.curCharYPos = this.curCharYPos - (this.paint.descent() / 2);
							//IKnowUKeyboardService.log(Log.VERBOSE, "painting letter = "+this.labelText.charAt(i), "x = "+this.curCharXPos+", y = "+this.curCharYPos);
							canvas.drawText(this.labelText.charAt(i)+"", this.curCharXPos, this.curCharYPos, paint);
						}
						//canvas.drawText(this.labelText, keyRect.left + (((keyRect.right - keyRect.left) / 2) - (this.labelWidth / 2)), 
						//		keyRect.top + this.keyPaddingTB + paint.getTextSize() - paint.descent(), paint);
					}
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
				    canvas.translate(key.x + drawableX, key.y + drawableY);
				    icon.setBounds(0, 0, icon.getIntrinsicWidth(), icon.getIntrinsicHeight());
				    //dont want to color over emoticons
				    if(key.codes[0] != -44) {
				    	//if (bKeyDarkBackground && this.keyStyle == THEME_STYLE_LINESBETWEEN) {
				    	//	icon.setColorFilter( this.keyDarkColor, PorterDuff.Mode.SRC_ATOP );
				    	//} else {
				    		icon.setColorFilter( Theme.KEY_TEXT_COLOR, PorterDuff.Mode.SRC_ATOP );
				    	//}
				    }
				    icon.draw(canvas);
				    canvas.translate(-key.x -drawableX, -key.y -drawableY);
				}
			}

            //Draw upper default characters for the key
            if ( key.codes[0] != -44 && ( (key.upperText != null && key.upperText.length() > 0) || (key.upperTextCaps != null && key.upperTextCaps.length() > 0) ) ) {
                paint.setColor(Theme.KEY_UPPER_ICON_COLOR);
                paint.setTextSize(mUprLabelTextSize);
                if (this.keyboard.isShifted() && key.codes[0] > 0) {
                    canvas.drawText(key.upperTextCaps , key.x + key.width - key.upperTextWidthCaps - 5, key.y + 15 /*24*/, paint);
                } else {
                    canvas.drawText( key.upperText, key.x + key.width - key.upperTextWidth - 5, key.y + 15 /*24*/, paint);
                }
            }
		} catch (Exception e) {
			IKnowUKeyboardService.sendErrorMessage(e);
		}
	}// end paintKey 
	
	/**
	 * Flag keys for highlighting based on input from our engine
	 */
	@Override
	public void prepareNextKeyHighlighting() {
		super.prepareNextKeyHighlighting();
		//String nextLetters = IKnowUKeyboardService.getPredictionEngine().nextLetters(this.numberOfFollowers);
		//this.keyboard.setKeyWidthsFromString(nextLetters, lastTouchX, lastTouchY);
	}
	
	/**
	 * A point to be used in drawing a line for displaying the user's touch inputs via swiping gestures
	 * @author Justin
	 *
	 */
	public class Vector2 {
	    public float x, y;
	    public float dx, dy;
	    public int color;

	    public Vector2(float x2, float y2) {
			x = x2; y = y2;
		}

		@Override
	    public String toString() {
	        return x + ", " + y;
	    }
	}
}
