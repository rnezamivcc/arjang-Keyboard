package com.iknowu;

import android.content.res.Resources;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.PorterDuff;
import android.graphics.RectF;
import android.graphics.drawable.Drawable;
import android.os.Message;
import android.util.Log;
import android.view.MotionEvent;

import com.iknowu.scroll.KeyboardScreen;
import com.iknowu.util.DimensionConverter;
import com.iknowu.util.Theme;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import com.iknowu.R;

/**
 * The SearchList class is used to present a vertical list of words, which can be pressed to further move down the chain of words
 * that the prediction engine is supplying us.
 * @author Justin
 *
 */
public class SearchList {
	
	public static double START_ANGLE = IKnowUKeyboardView.keyboardService.mHandednessRight ? 180 : 0;
	public static final double DEG_TO_RAD = Math.PI / 180;
	
	public static final double SCROLL_DILUTION = 10;
	
	private static double SCROLL_DEAD_ZONE_X = IKnowUKeyboardView.keyboardService.mHandednessRight ? 0.7 : 0.3;
	private static final double SCROLL_DEAD_ZONE_Y_LOWER = 0.3;
	private static final double SCROLL_DEAD_ZONE_Y_UPPER = 0.7;
	
	private List<SearchListItem> items;
	
	private int startx;
	private int starty;
	
	public int animationStartx;
	public int animationStarty;
	
	public IKnowUKeyboardView kbView;

    public Drawable backIcon;
	public int backButtonX;
	public int backButtonY;
	public int backButtonWidth;
	public int backButtonHeight;
	public String rootText;
	
	public String startText;
	
	static public IKnowUKeyboardService keyboardService;

    public Drawable spaceIcon;
    public int spaceButtonX;
    public int spaceButtonY;
    public int spaceButtonWidth;
    public int spaceButtonHeight;
	
	private int textColor;
	private int keyColor;
	private int keyPressedColor;
	
	private int backKeyColor;
	
	//private float scale;
	
	public boolean searchBackButtonPressed;
    public boolean searchSpaceButtonPressed;

    public boolean searchBackButtonHovered;
    public boolean searchSpaceButtonHovered;
	
	private boolean landscape;
	
	public final int animDuration = 150;	//duration for the searchlist animation in milliseconds
	
	public ArrayList<Integer> deadZoneStack;
	public ArrayList<String> searchListStack;
	
	public int keyIdxPressed;
	public int fireIndex = -1;
	
	public int viewHeight;
	public int viewWidth;
	public double lastDiffY;
	public boolean isScrolling;
	private int scrollDirection;
	private int lastTouchY;
	
	public boolean capsFirst;

    public int textMode;
	private Resources resources;

    private int backButtonMargin;
	
	//===========================================================================
	//CONSTRUCTOR
	//===========================================================================
	public SearchList(IKnowUKeyboardView kbview, float scale, boolean landscape, int textcolor, int keycolor, int keyPressedColor, int backColor) {
		this.kbView = kbview;
		
		this.lastDiffY = 0;

        this.resources = this.kbView.keyboardService.getResources();

        this.spaceIcon = this.resources.getDrawable(R.drawable.sym_keyboard_space);
        this.spaceIcon.setBounds(0, 0, this.spaceIcon.getIntrinsicWidth(), this.spaceIcon.getIntrinsicHeight());

        this.backIcon = this.resources.getDrawable(R.drawable.sym_keyboard_backspace);
        this.backIcon.setBounds(0, 0, this.backIcon.getIntrinsicWidth(), this.backIcon.getIntrinsicHeight());

		this.backButtonWidth = DimensionConverter.stringToDimensionPixelSize("65dp", this.resources.getDisplayMetrics());
		this.backButtonHeight = DimensionConverter.stringToDimensionPixelSize("55dp", this.resources.getDisplayMetrics());

        this.spaceButtonWidth = DimensionConverter.stringToDimensionPixelSize("65dp", this.resources.getDisplayMetrics());
        this.spaceButtonHeight = DimensionConverter.stringToDimensionPixelSize("55dp", this.resources.getDisplayMetrics());

        this.backButtonMargin = DimensionConverter.stringToDimensionPixelSize("25dp", this.resources.getDisplayMetrics());
		
		this.textColor = textcolor;
		this.keyColor = keycolor;
		this.keyPressedColor = keyPressedColor;
		this.backKeyColor = backColor;

		this.landscape = landscape;
		//this.scale = scale;
		
		this.deadZoneStack = new ArrayList<Integer>();
		this.searchListStack = new ArrayList<String>();

        this.textMode = Suggestion.TEXT_MODE_LATIN;
        switch (IKnowUKeyboardView.keyboardService.currentKeyboardLayout) {
            case KeyboardScreen.KOREAN:
                this.textMode = Suggestion.TEXT_MODE_KOREAN;
                break;
            default:
                this.textMode = Suggestion.TEXT_MODE_LATIN;
                break;
        }
	}
	
	/**
	 * Set the max height and width for the search list to work with
	 * @param width the width of the viewport
	 * @param height the height of the viewport
	 */
	public void setViewport(int width, int height) {
		this.viewHeight = height;
		this.viewWidth = width;
	}
	
	/**
	 * Set the {@link IKnowUKeyboardService} to be set
	 * @param sb
	 */
	public void setKeyboardService(IKnowUKeyboardService sb) {
		keyboardService = sb;
	}
	
	/**
	 * Set the predictions to be displayed
	 * @param predictions the array of predictions to display
	 * @return 
	 */
	public boolean setPredictions(String[] predictions) {
		
		START_ANGLE = IKnowUKeyboardView.keyboardService.mHandednessRight ? 180 : 0;
		SCROLL_DEAD_ZONE_X = IKnowUKeyboardView.keyboardService.mHandednessRight ? 0.7 : 0.3;
		
		boolean hasPredictions = false;
		
		this.viewWidth = this.kbView.getWidth();
		this.viewHeight = this.kbView.getHeight();
		
		//this.lastDiffY = 0;
		
		this.clearList(predictions.length + 1);
		
		int nElems = predictions.length;

        IKnowUKeyboardService.log(Log.VERBOSE, "SearchList.setPredictions()", "numPreds = "+nElems);

		//less than or equal to, because one of the spots is always for the deadzone
		//for (int k = 0; k <= nElems; k++) {
        if (nElems > 0) {
            for (int k = 0; k < IKnowUKeyboardView.NUMBER_OF_SEARCHROWS; k++) {

                SearchListItem item = new SearchListItem(this, "", this.resources, landscape);

                item.index = k;

                if (k != deadZoneStack.get(deadZoneStack.size()-1) /*deadZoneKeyIndex*/) {
                    //however many spots away from the deadzone, determines the predcition, the prediction is always spotsAway - 1;
                    int pred = this.calcSpotsAway(k, deadZoneStack.get(deadZoneStack.size()-1), IKnowUKeyboardView.NUMBER_OF_SEARCHROWS) - 1;
                    if (pred < nElems) {
                        String text = predictions[pred];
                        String origPred = text;

                        if (text != null && text.length() > 0) {
                            text = text.substring(0, text.length()-3);
                            boolean isChunk = true;
                            //int numFollowers = IKnowUKeyboardService.getPredictionEngine().getNumWordsStartingWith(text);
                            /*
                            if (!isChunk) {
                                text = text.substring(text.lastIndexOf(" ")+1);
                                int num = IKnowUKeyboardService.getPredictionEngine().getNumWordsStartingWith(text);
                                if (num > 1) isChunk = true;
                                IKnowUKeyboardService.log(Log.ERROR, "SearchList SetPredictions", "text = "+text+", num = "+num+", isChunk = "+isChunk);
                            }
                            */
                            //if (numFollowers > 0 /* && origPred.contains("...") */) isChunk = true;
                            IKnowUKeyboardService.log(Log.VERBOSE, "SearchList SetPredictions", "rootRightNow = |"+this.rootText+"|, text = |"+text+"|, numFollowers = "+/*numFollowers+*/", isChunk = "+isChunk);
                            item.setIsChunk(isChunk);
                            if ( this.rootText != null && this.rootText.length() > 0 && Character.isUpperCase(rootText.charAt(0)) ) {
                                this.capsFirst = true;
                            } else if (IKnowUKeyboardView.keyboardService.capitalizeFirstLetter) {
                                this.capsFirst = true;
                            }
                            IKnowUKeyboardService.log(Log.VERBOSE, "SearchList.setPredictions()", "capsFirst = "+capsFirst+", where pred = |"+text+"| and root = |"+rootText+"|");
                            if ( this.capsFirst && text.toLowerCase().startsWith(rootText.toLowerCase())) {
                                this.capsFirst = true;
                            } else {
                                this.capsFirst = false;
                            }
                            item.setText(origPred, this.capsFirst, this.textMode, "setPreds");
                        }
                    }
                } else {
                    this.setBackAndSpacePositions(this.lastDiffY, k);
                }

                //item.setText("test", false);

                item.update(this.lastDiffY, this.viewWidth, this.viewHeight, this.startx);

                //item.setIsChunk(true);

                this.items.add(item);
            }
        }
		return hasPredictions;
	}

    private void setBackAndSpacePositions(double diffy, int index) {

        SearchListItem item = new SearchListItem(this, "", this.resources, this.landscape);
        item.index = index;
        item.update(this.lastDiffY, this.viewWidth, this.viewHeight, this.startx);

        boolean leftSide = false;

        if ( this.startx <= this.viewWidth / 2 ) {
            leftSide = true;
        } else if ( this.startx >= this.viewWidth / 2 ) {
            leftSide = false;
        }

        if (leftSide) {
            this.spaceButtonX = item.xCoord - this.backButtonMargin - this.spaceButtonWidth;
            this.spaceButtonY = (int) ( item.yCoord + ( (item.height - this.spaceButtonHeight) / 2 ) );
            this.backButtonX = this.spaceButtonX - this.backButtonWidth - 10;
            this.backButtonY = this.spaceButtonY;
        } else {
            this.spaceButtonX = item.xCoord + item.width + this.backButtonMargin;
            this.spaceButtonY = (int) ( item.yCoord + ( (item.height - this.spaceButtonHeight) / 2 ) );
            this.backButtonX = this.spaceButtonX + this.spaceButtonWidth + 10;
            this.backButtonY = this.spaceButtonY;
        }

        //IKnowUKeyboardService.log(Log.VERBOSE, "SearchList.setBackSpacePositions()", "x = "+x+", y = "+y);
    }

	
	/**
	 * Determine the prediction to put in the spot based on the the deadzone location (last pressed key)
	 * @param spot the spot to fill with a perdiction
	 * @param deadZone the current deadzone
	 * @param max the max spot
	 * @return the index of the prediction that should fill this spot
	 */
	private int calcSpotsAway(int spot, int deadZone, int max) {
		int diff = spot - deadZone;
		int spotsAway = 1;
		
		//if the spot is above the deadzone
		if (diff > 0) {
			//if the deadzone is not in position 0, then figure out how many spots are below it
			//otherwise the spots away are just that current spot
			if (deadZone > 0) {
				if (deadZone - diff < 0) {
					spotsAway = spot;
				} else {
					spotsAway = 1 + (2 * (diff - 1));
				}
			} else {
				spotsAway = spot;
			}
		} else {
			//if the deadzone is not at the max position, then figure out how many spots are above it
			//otherwise the spots away are just equal to the deadzone - spot
			diff = Math.abs(diff);
			if (deadZone < max-1) {
				if (deadZone + diff > max - 1) {
					spotsAway = diff + (max - 1 - deadZone);
				} else {
					spotsAway = 2 * diff;
				}
			} else {
				spotsAway = deadZone - spot;
			}
		}
		
		IKnowUKeyboardService.log(Log.WARN, "calc spots", "spot = "+spot+", deadzone = "+deadZone+", spots away = "+spotsAway);
		
		return spotsAway;
	}
	
	/**
	 * Clear this {@link SearchList}
	 * @param numElems
	 */
	public void clearList(int numElems) {
		this.items = new ArrayList<SearchListItem>(numElems);
	}
	
	/**
	 * Add a prediction to this search list
	 * @param text the prediction to add
	 * @param pos the position to add it
	 * @return true on success
	 */
	public boolean addItem( String text, int pos ) {
		if (text != null && text.length() > 0) {
			SearchListItem item = new SearchListItem(this, text, this.resources, landscape);
			this.items.add(pos, item);
			return true;
		} else {
			return false;
		}
	}
	
	/**
	 * This will scroll the list if we have enabled it
	 */
	public void updateScroll() {
		//IKnowUKeyboardService.log(Log.VERBOSE, "SearchListItem updateScroll", "scrolling ="+this.isScrolling+", dir = "+this.scrollDirection);
		
		if ((this.scrollDirection < 0 && this.items.get(this.items.size() - 1).yCoord > (this.viewHeight / 2) ) ||
				(this.scrollDirection > 0 && this.items.get(0).yCoord < (this.viewHeight / 2) ) ) {
			this.kbView.mHandler.removeMessages(IKnowUKeyboardView.MSG_SEARCH_SCROLL);
			this.isScrolling = false;
		} else {
			if (this.scrollDirection > 0) {
				double ydif = this.lastTouchY / (this.viewHeight * SCROLL_DEAD_ZONE_Y_UPPER);
				if (ydif < 1) ydif = 1;
				this.lastDiffY += (1 / (SCROLL_DILUTION * ydif));
				this.isScrolling = true;
				this.scrollDirection = 1;
			} else if (this.scrollDirection < 0) {
				double ydif = this.lastTouchY / this.viewHeight;
				if (ydif < 1) ydif = 1;
				this.lastDiffY -= (1 / (SCROLL_DILUTION * ydif));
				this.isScrolling = true;
				this.scrollDirection = -1;
			} else {
				this.isScrolling = false;
			}
			
			Iterator<SearchListItem> it = this.items.iterator();
			
			while (it.hasNext()) {
				SearchListItem item = it.next();
				item.update(this.lastDiffY, this.viewWidth, this.viewHeight, this.startx);
			}
			
			if (this.isScrolling) {
				Message msg = this.kbView.mHandler.obtainMessage(IKnowUKeyboardView.MSG_SEARCH_SCROLL);
				this.kbView.mHandler.sendMessage(msg);
			} else {
				this.kbView.mHandler.removeMessages(IKnowUKeyboardView.MSG_SEARCH_SCROLL);
			}
		}
		
		this.kbView.invalidate();
	}
	
	/**
	 * Process the passed in {@link MotionEvent}
	 * @param event the MotionEvent that occured
	 * @return whether we processed it or not
	 */
	public boolean onTouch (MotionEvent event) {
		int action = event.getAction();
		
		this.lastTouchY = (int) event.getY();
		/*
		 * If we are in search mode and we are either on an action down or move event
		 * then we need to check to see if we are hovering over a valid search item
		 * and if so start the timer to allow us to move to the next part of a phrase
		 */
		if ((action == android.view.MotionEvent.ACTION_MOVE || action == android.view.MotionEvent.ACTION_DOWN)) {
			
			this.keyIdxPressed = -1;
			
			Iterator<SearchListItem> it = this.items.iterator();
			int nElems = this.items.size();
			
			this.searchBackButtonHovered = false;
			//String rootText = IKnowUKeyboardService.getPredictionEngine().setWordInfo();
			//Log.d("ROOT TEXT = ", ""+rootText);
			//check to see if we have signaled our global variable to be pressed
			//if so, check to see if we are still hovering the back button
			//if we are, keep global set to true, to avoid repeated presses of button
			if (this.searchListStack.size() > 0) {
				if (this.searchBackButtonPressed) {
                    this.searchBackButtonHovered = this.checkBackButtonCollision(event.getX(), event.getY());
					if (!this.searchBackButtonHovered) {
						this.searchBackButtonPressed = false;
					} else {
                        this.searchBackButtonHovered = false;
					}
				} else {
                    this.searchBackButtonHovered = this.checkBackButtonCollision(event.getX(), event.getY());
				}
			}

            this.searchSpaceButtonHovered = false;
            //String rootText = IKnowUKeyboardService.getPredictionEngine().setWordInfo();
            //Log.d("ROOT TEXT = ", ""+rootText);
            //check to see if we have signaled our global variable to be pressed
            //if so, check to see if we are still hovering the back button
            //if we are, keep global set to true, to avoid repeated presses of button
            if (this.searchListStack.size() > 0) {
                if (this.searchSpaceButtonPressed) {
                    this.searchSpaceButtonHovered = this.checkSpaceButtonCollision(event.getX(), event.getY());
                    if (!this.searchSpaceButtonHovered) {
                        this.searchSpaceButtonPressed = false;
                    } else {
                        this.searchSpaceButtonHovered = false;
                    }
                } else {
                    this.searchSpaceButtonHovered = this.checkSpaceButtonCollision(event.getX(), event.getY());
                }
            }
			/*
			//if we aren't currently scrolling check to see if we are now
			if (IKnowUKeyboardView.keyboardService.mHandednessRight) {
				if (!this.isScrolling && event.getX() > (this.viewWidth * SCROLL_DEAD_ZONE_X)) {
					if (event.getY() > (this.viewHeight * SCROLL_DEAD_ZONE_Y_UPPER)) {
						double ydif = this.lastTouchY / (this.viewHeight * SCROLL_DEAD_ZONE_Y_UPPER);
						if (ydif < 1) ydif = 1;
						this.lastDiffY += (1 / (SCROLL_DILUTION * ydif));
						this.isScrolling = true;
						this.scrollDirection = 1;
					} else if (event.getY() < (this.viewHeight * SCROLL_DEAD_ZONE_Y_LOWER)) {
						double ydif = (this.viewHeight * SCROLL_DEAD_ZONE_Y_LOWER) / this.lastTouchY;
						if (ydif < 1) ydif = 1;
						this.lastDiffY -= (1 / (SCROLL_DILUTION * ydif));
						this.isScrolling = true;
						this.scrollDirection = -1;
					} else {
						this.isScrolling = false;
					}
					//this.lastDiffY = (int) (event.getY() - this.starty);
				//should we still be scrolling?
				} else if (this.isScrolling) {
					if (event.getX() > (this.viewWidth * SCROLL_DEAD_ZONE_X)) {
						if (event.getY() > (this.viewHeight * SCROLL_DEAD_ZONE_Y_UPPER)) {
							this.isScrolling = true;
							this.scrollDirection = 1;
						} else if (event.getY() < (this.viewHeight * SCROLL_DEAD_ZONE_Y_LOWER)) {
							this.isScrolling = true;
							this.scrollDirection = -1;
						} else {
							this.isScrolling = false;
						}
					} else {
						this.isScrolling = false;
					}
				}
			} else {
				if (!this.isScrolling && event.getX() < (this.viewWidth * SCROLL_DEAD_ZONE_X)) {
					if (event.getY() > (this.viewHeight * SCROLL_DEAD_ZONE_Y_UPPER)) {
						double ydif = this.lastTouchY / (this.viewHeight * SCROLL_DEAD_ZONE_Y_UPPER);
						if (ydif < 1) ydif = 1;
						this.lastDiffY += (1 / (SCROLL_DILUTION * ydif));
						this.isScrolling = true;
						this.scrollDirection = 1;
					} else if (event.getY() < (this.viewHeight * SCROLL_DEAD_ZONE_Y_LOWER)) {
						double ydif = (this.viewHeight * SCROLL_DEAD_ZONE_Y_LOWER) / this.lastTouchY;
						if (ydif < 1) ydif = 1;
						this.lastDiffY -= (1 / (SCROLL_DILUTION * ydif));
						this.isScrolling = true;
						this.scrollDirection = -1;
					} else {
						this.isScrolling = false;
					}
					//this.lastDiffY = (int) (event.getY() - this.starty);
				//should we still be scrolling?
				} else if (this.isScrolling) {
					if (event.getX() < (this.viewWidth * SCROLL_DEAD_ZONE_X)) {
						if (event.getY() > (this.viewHeight * SCROLL_DEAD_ZONE_Y_UPPER)) {
							this.isScrolling = true;
							this.scrollDirection = 1;
						} else if (event.getY() < (this.viewHeight * SCROLL_DEAD_ZONE_Y_LOWER)) {
							this.isScrolling = true;
							this.scrollDirection = -1;
						} else {
							this.isScrolling = false;
						}
					} else {
						this.isScrolling = false;
					}
				}
			}
			
			if ( (this.scrollDirection < 0 && this.items.get(this.items.size() - 1).yCoord > (this.viewHeight / 2) ) ||
					(this.scrollDirection > 0 && this.items.get(0).yCoord < (this.viewHeight / 2) ) ) {
				this.kbView.mHandler.removeMessages(IKnowUKeyboardView.MSG_SEARCH_SCROLL);
				this.isScrolling = false;
			} else {
				if (this.isScrolling) {
					Message msg = this.kbView.mHandler.obtainMessage(IKnowUKeyboardView.MSG_SEARCH_SCROLL);
					this.kbView.mHandler.sendMessage(msg);
				} else {
					this.kbView.mHandler.removeMessages(IKnowUKeyboardView.MSG_SEARCH_SCROLL);
				}
			}
			*/
			//update all of the items in the searchlist
			while (it.hasNext()) {
				SearchListItem item = it.next();
				if (item.getText().length() > 0) {
					checkSearchItemCollision(item, event.getX(), event.getY());
					if (item.isHovered) keyIdxPressed = item.index;
				}
				if (this.isScrolling)
					item.update(this.lastDiffY, this.viewWidth, this.viewHeight, this.startx);
			}
			
			if (this.searchBackButtonHovered) {
				Message msg = this.kbView.mHandler.obtainMessage(IKnowUKeyboardView.MSG_SEARCH_BACKBUTTON);
                msg.arg1 = this.textMode;
				this.kbView.mHandler.sendMessageDelayed(msg, Theme.LONG_PRESS_TIMEOUT);
			} else {
				this.kbView.mHandler.removeMessages(IKnowUKeyboardView.MSG_SEARCH_BACKBUTTON);
			}

            if (this.searchSpaceButtonHovered) {
                Message msg = this.kbView.mHandler.obtainMessage(IKnowUKeyboardView.MSG_SEARCH_SPACEBUTTON);
                msg.arg1 = this.textMode;
                this.kbView.mHandler.sendMessageDelayed(msg, Theme.LONG_PRESS_TIMEOUT);
            } else {
                this.kbView.mHandler.removeMessages(IKnowUKeyboardView.MSG_SEARCH_SPACEBUTTON);
            }
			
			if (keyIdxPressed != fireIndex && fireIndex != -1) {
				fireIndex = -1;
				this.kbView.mHandler.removeMessages(IKnowUKeyboardView.MSG_LONGPAUSE);
			}
			
			if (this.kbView.mHandler != null && keyIdxPressed != -1 && fireIndex == -1) {
				boolean allowDeeperLevel = true;
				if (keyIdxPressed >= (nElems - IKnowUKeyboardView.NUMBER_OF_SEARCHROWS)) {
					allowDeeperLevel = this.getItems().get(this.keyIdxPressed).isChunk();
					//allowDeeperLevel = mPredictionIsChunk[keyIdxPressed];
				}
				if (allowDeeperLevel) {
					fireIndex = keyIdxPressed;
					Message msg = this.kbView.mHandler.obtainMessage(IKnowUKeyboardView.MSG_LONGPAUSE);
					msg.arg1 = fireIndex;
					this.kbView.mHandler.sendMessageDelayed(msg, Theme.LONG_PRESS_TIMEOUT);
				}
			}
			return true;
		}//end action move/action down in search mode
		
		if (action == android.view.MotionEvent.ACTION_UP) {
			// we have been moving around in the search list and
			// we are releasing before another longpress occurs
			Iterator<SearchListItem> it = this.items.iterator();
			
			int keyIdx = -1;
			String rootText = keyboardService.getPredictionEngine().getRootWordNextWords();
			
			while (it.hasNext()) {
				SearchListItem item = it.next();
				checkSearchItemCollision(item, event.getX(), event.getY());
				if (item.isHovered) keyIdx= item.index;
			}
			
			SearchListItem item;
			
			if (keyIdx != -1) {
				item = this.items.get(keyIdx);
			} else {
				item = null;
			}
			
			if (item != null && keyIdx != deadZoneStack.get(deadZoneStack.size()-1) /*deadZoneKeyIndex*/) {
				if (item.getText() != null && item.getText().length() > 0) {
					String selectedText = item.getText().toString();
					this.kbView.addTextFromSearchList(selectedText, this.textMode);
					//this.kbView.releaseWithNoConsequence = true;
					IKnowUKeyboardView.keyboardService.capitalizeFirstLetter = false;
				}
			} else if ( rootText == null || this.deadZoneStack.size() <= 1 ) {
				//if there is no root or the stack is in the initial position don't add a space after
				this.kbView.releaseWithNoConsequence = true;
			}
			
			if (fireIndex != -1) {
				this.kbView.mHandler.removeMessages(IKnowUKeyboardView.MSG_LONGPAUSE);
				fireIndex = -1;
			}
			
			this.kbView.prepareNextKeyHighlighting();
			this.kbView.switchSearchModeOff();
		}//end action up in search mode
		return false;
	}
	
	/**
	 * Perform the drawing function of this search list
	 * @param canvas the canvas to draw on
	 * @param interpolation the total interpolation since the first draw, for animating purposes
	 * @param paint a paint object to use
	 */
	public void draw(Canvas canvas, double interpolation, Paint paint) {
		
		if (!this.searchBackButtonPressed && this.searchListStack != null && this.searchListStack.size() > 0) {
			int backX = (int) (this.animationStartx + ((this.backButtonX - this.animationStartx) * interpolation));
			int backY = (int) (this.animationStarty + ((this.backButtonY - this.animationStarty) * interpolation));
			drawBackButton(canvas, backX, backY, paint);
		}

        if (!this.searchSpaceButtonPressed && this.searchListStack != null && this.searchListStack.size() > 0) {
            int x = (int) (this.animationStartx + ((this.spaceButtonX - this.animationStartx) * interpolation));
            int y = (int) (this.animationStarty + ((this.spaceButtonY - this.animationStarty) * interpolation));
            this.drawSpaceButton(canvas, x, y, paint);
        }

		
		Iterator<SearchListItem> searchit = this.items.iterator();
		while (searchit.hasNext()) {
			
			SearchListItem item = searchit.next();
			
			if (item.getText().length() > 0) {
				int x = (int) (this.animationStartx + ((item.xCoord - this.animationStartx) * interpolation));
				int y = (int) (this.animationStarty + ((item.yCoord - this.animationStarty) * interpolation));
				
				item.draw(canvas, x, y, paint, this.textColor, this.keyColor, this.keyPressedColor);
			}
		}
	}
	
	/**
	 * Draw the back button for the search list
	 * @param canvas the canvas to draw on
	 * @param x the x-coord to draw the button at
	 * @param y the y-coord to draw the button at
	 * @param paint the paint object to use
	 */
	private void drawBackButton(Canvas canvas, int x, int y, Paint paint) {
		RectF rect = new RectF( x, y, x+this.backButtonWidth, y+this.backButtonHeight );

        if (this.searchBackButtonHovered) {
		    paint.setColor(Theme.SEARCH_ITEM_PRESSED_COLOR);
        } else {
            paint.setColor(Theme.SEARCH_ITEM_BACK_COLOR);
        }
		paint.setStyle(Paint.Style.STROKE);
		paint.setStrokeWidth(3);
		canvas.drawRoundRect(rect, 5, 5, paint);
		
		paint.setAlpha(200);
		paint.setStyle(Paint.Style.FILL);
		canvas.drawRoundRect(rect, 5, 5, paint);
		
		//mPaint.setShader(new LinearGradient(x, y, x, y+item.ITEM_HEIGHT, Color.RED, Color.argb(0, 175, 175, 175), Shader.TileMode.CLAMP));
		
		paint.setShader(null);
		//mPaint.setStrokeWidth(1);
		paint.setColor(this.textColor);

        int padX = (this.backButtonWidth - this.backIcon.getIntrinsicWidth()) / 2; /*DimensionConverter.stringToDimensionPixelSize(5+"dp", this.resources.getDisplayMetrics());*/
        int padY = (this.backButtonHeight - this.backIcon.getIntrinsicHeight()) / 2; /*DimensionConverter.stringToDimensionPixelSize(3+"dp", this.resources.getDisplayMetrics());*/

        canvas.save();
        canvas.translate(x + padX, y + padY);
        this.backIcon.setColorFilter( Theme.SEARCH_ITEM_TEXT_COLOR, PorterDuff.Mode.SRC_ATOP );
        this.backIcon.draw(canvas);
        //canvas.translate(-key.x -drawableX, -key.y -drawableY);
        canvas.restore();
	}

    /**
     * Draw the back button for the search list
     * @param canvas the canvas to draw on
     * @param x the x-coord to draw the button at
     * @param y the y-coord to draw the button at
     * @param paint the paint object to use
     */
    private void drawSpaceButton(Canvas canvas, int x, int y, Paint paint) {
        RectF rect = new RectF( x, y, x+this.spaceButtonWidth, y+this.spaceButtonHeight );

        if (this.searchSpaceButtonHovered) {
            paint.setColor(Theme.SEARCH_ITEM_PRESSED_COLOR);
        } else {
            paint.setColor(Theme.SEARCH_ITEM_BACK_COLOR);
        }
        paint.setStyle(Paint.Style.STROKE);
        paint.setStrokeWidth(3);
        canvas.drawRoundRect(rect, 5, 5, paint);

        paint.setAlpha(200);
        paint.setStyle(Paint.Style.FILL);
        canvas.drawRoundRect(rect, 5, 5, paint);

        //mPaint.setShader(new LinearGradient(x, y, x, y+item.ITEM_HEIGHT, Color.RED, Color.argb(0, 175, 175, 175), Shader.TileMode.CLAMP));

        paint.setShader(null);

        int padX = (this.spaceButtonWidth - this.spaceIcon.getIntrinsicWidth()) / 2; /*DimensionConverter.stringToDimensionPixelSize(5+"dp", this.resources.getDisplayMetrics());*/
        int padY = (this.spaceButtonHeight - this.spaceIcon.getIntrinsicHeight()) / 2; /*DimensionConverter.stringToDimensionPixelSize(0+"dp", this.resources.getDisplayMetrics());*/

        canvas.save();
        canvas.translate(x + padX, y + padY);
        this.spaceIcon.setColorFilter( Theme.SEARCH_ITEM_TEXT_COLOR, PorterDuff.Mode.SRC_ATOP );
        this.spaceIcon.draw(canvas);
        //canvas.translate(-key.x -drawableX, -key.y -drawableY);
        canvas.restore();
    }
	
	public SearchListItem fireLongPause(int key) {
		SearchListItem item = this.items.get(key);
		this.fireIndex = -1;
		if (item.getText() != null) {
			deadZoneStack.add(key);
			return item;
		}
		return null;
	}
	
	/**
	 * Pass in a SearchListItem and touch coords, to see if we are hovering an item
	 * If so indicate this and allow update to visuals and to possibly move deeper into
	 * prediction of phrase.
	 * 
	 * @param item the item to check for touch collision
	 * @param touchX the x-coord of the touch
	 * @param touchY the y-coord of the touch
	 */
	private void checkSearchItemCollision(SearchListItem item, float touchX, float touchY) {
		boolean trueX = false;
		boolean trueY = false;
		
		if (item.xCoord < touchX && touchX < (item.xCoord+item.width)) trueX = true;
		if (item.index == deadZoneStack.get(deadZoneStack.size()-1) /*deadZoneKeyIndex*/) { trueX = false; trueY = false; }
		if (item.yCoord < touchY && touchY < (item.yCoord+item.height)) trueY = true;
		
		if (trueX && trueY) {
			item.isHovered = true;
		} else {
			item.isHovered = false;
		}
	}
	
	/**
     * Check a set of coords to see if they have crossed into the boundaries of our back button
     *
     * @param touchX the x-coord of the touch
     * @param touchY the y-coord of the touch
     * @return true if the touch is in the boundaries of the back button
     */
    public boolean checkBackButtonCollision(float touchX, float touchY) {
        boolean trueX = false;
        boolean trueY = false;

        if (this.backButtonX < touchX && touchX < (this.backButtonX+this.backButtonWidth)) trueX = true;
        if (this.backButtonY < touchY && touchY < (this.backButtonY+this.backButtonHeight)) trueY = true;

        if (trueX && trueY) {
            return true;
        } else {
            return false;
        }
    }

    /**
     * Check a set of coords to see if they have crossed into the boundaries of our space button
     *
     * @param touchX the x-coord of the touch
     * @param touchY the y-coord of the touch
     * @return true if the touch is in the boundaries of the back button
     */
    public boolean checkSpaceButtonCollision(float touchX, float touchY) {
        boolean trueX = false;
        boolean trueY = false;

        if (this.spaceButtonX < touchX && touchX < (this.spaceButtonX+this.spaceButtonWidth)) trueX = true;
        if (this.spaceButtonY < touchY && touchY < (this.spaceButtonY+this.spaceButtonHeight)) trueY = true;

        if (trueX && trueY) {
            return true;
        } else {
            return false;
        }
    }
	
	public List<SearchListItem> getItems() {
		return this.items;
	}
	
	public void setStartX(int x) {
		this.startx = x;
	}
	
	public int getStartX() {
		return this.startx;
	}
	
	public void setStartY(int y) {
		this.starty = y;
	}
	
	public int getStartY() {
		return this.starty;
	}
}
