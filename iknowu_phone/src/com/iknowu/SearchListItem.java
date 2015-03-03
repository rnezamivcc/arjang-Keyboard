package com.iknowu;

import android.content.res.Resources;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.RectF;
import android.util.Log;

import com.iknowu.asian.Hangul;
import com.iknowu.util.DimensionConverter;

/**
 * An item that holds a precition to be displayed as part of the vertical {@link SearchList}.
 * 
 * @author Justin
 *
 */
public class SearchListItem {
	
	public static double LANDSCAPE_HEIGHT = 22;	//All units are in dp to allow for different screen sizes
	public static double TABLET_HEIGHT = 35;		//conversions are done on the fly to figure out pixels
	public static double POTRAIT_HEIGHT = 26;
	public static double TABLET_WIDTH = 200;
	public static double ITEM_ANGLE = 6.5;
	
	private static int MAX_WIDTH = 550;
	
	public double preferredHeight;
	
	private SearchList searchList;
	private String text;
	private String textShow;
	public boolean isFirstCap;
	public int width;
	public double height;
	public int padTop;
	
	public int xCoord;
	public int yCoord;
	
	private boolean isChunk;
	
	//private float scale;
    private Resources resources;
	
	public int index;
	private int priority;
	
	public boolean isHovered;
	private double fontHeight;
	
	private boolean landscape;
	
	public SearchListItem(SearchList searchList, String text, Resources res, boolean landscape) {
		this.searchList = searchList;
		this.setText(text, false, Suggestion.TEXT_MODE_LATIN, "constructor");
		this.xCoord = 0;
		this.yCoord = 0;
		this.isHovered = false;
        this.resources = res;
		//this.scale = scale;
		this.landscape = landscape;
		
		this.width = DimensionConverter.stringToDimensionPixelSize(TABLET_WIDTH+"dp", this.resources.getDisplayMetrics());
		double hdp = SearchListItem.TABLET_HEIGHT;
		this.preferredHeight = DimensionConverter.stringToDimensionPixelSize(hdp+"dp", this.resources.getDisplayMetrics());
		this.height = DimensionConverter.stringToDimensionPixelSize(hdp+"dp", this.resources.getDisplayMetrics());
		this.padTop = DimensionConverter.stringToDimensionPixelSize(8+"dp", this.resources.getDisplayMetrics());
		this.fontHeight = this.searchList.kbView.searchListTextSize;
	}
	
	/**
	 * Get the piece of text associated with this item
	 * 
	 * @return the text of this item
	 */
	public String getText() {
		return this.text;
	}

    /**
     * Set the text to be associated with this item
     *
     * @param text the text to be set
     * @param firstCaps whether or not the first letter should be displayed as a capital letter
     */
    public void setText(String text, boolean firstCaps, int textmode, String calledFrom) {

        IKnowUKeyboardService.log(Log.INFO, "SearchListItem.setText()", "calledFrom = "+calledFrom+", text = "+text+", firstCaps = "+firstCaps);

        switch (textmode) {
            case Suggestion.TEXT_MODE_LATIN:
                this.setLatinText(text, firstCaps);
                break;
            case Suggestion.TEXT_MODE_KOREAN:
                this.setKoreanText(text);
                break;
            default:
                this.setLatinText(text, firstCaps);
                break;
        }
    }

    private void setLatinText(String suggestion, boolean firstCaps) {

        this.text = suggestion;

        this.isFirstCap = firstCaps;
        if (firstCaps) {
            char[] chrs = text.toCharArray();
            chrs[0] = Character.toUpperCase(chrs[0]);
            text = new String(chrs);
            this.textShow = text;
        } else {
            this.textShow = text;
        }
        //if (this.isChunk && !text.contains("...")) {
        //    this.textShow = this.textShow.trim();
        //     this.textShow = this.textShow + "...";
        //}
        
        if (this.isChunk && !text.contains("   ")) {
            this.textShow = this.textShow.trim();
            this.textShow = this.textShow + "   ";
        }
    }

    private void setKoreanText(String suggestion) {
        this.text = suggestion;
        this.textShow = "";

        this.textShow = Hangul.combineChars(suggestion, false);
    }
	
	/**
	 * Set the priority of this item. Highest priority gets drawn in bold
	 * 
	 * @param prior the priority to set
	 */
	public void setPriority(int prior) {
		this.priority = prior;
	}
	
	/**
	 * Update the position of this item
	 * 
	 * @param diffy For scrolling purposes, the value in the y-axis to offset this item
	 * @param width the width of the of the viewport
	 * @param height the height of the viewport
	 * @param centerx the center of the viewport
	 */
	public void update(double diffy, int width, int height, int centerx) {

		int maxItemHeight = height / 6;
		if ( this.height > maxItemHeight ) {
			this.height = maxItemHeight;
		}
		
		boolean leftSide = false;

        ITEM_ANGLE = 7.5;
        if ( centerx <= width / 2 ) {
            leftSide = true;
        } else if ( centerx >= width / 2 ) {
            leftSide = false;
        }

		double startAngle = !leftSide ? 180 : 0;
		
		int radius = Integer.valueOf(width);
		if (radius > MAX_WIDTH) {
			radius =  MAX_WIDTH;
			centerx = !leftSide ? centerx+200 : centerx-200;
		} else {
			centerx = !leftSide ? width : 0;
		}
		
		//calculate the maximum item_angle
		double maxX = Math.sqrt( (radius*radius) - (height*height) );
        double maxAngle = (Math.atan2(height, maxX) / SearchList.DEG_TO_RAD) / 6;
        //IKnowUKeyboardService.log(Log.VERBOSE, "SearchListItem update", "maxAngle = "+maxAngle+", maxX = "+maxX);
        if (maxAngle < ITEM_ANGLE) {
        	ITEM_ANGLE = maxAngle;
        }
		
		//IKnowUKeyboardService.log(Log.VERBOSE, "SearchListItem update", "width = "+width+", start angle = "+SearchList.START_ANGLE);
		//IKnowUKeyboardService.log(Log.VERBOSE, "SearchListItem update", "width = "+width+", height = "+height);
		double angle;
		if ( !leftSide ) {
			angle = ( ( (this.index + 1) * ITEM_ANGLE) + (diffy /* / SearchList.SCROLL_DILUTION */) + startAngle) * SearchList.DEG_TO_RAD;
		} else {
			angle = ( ( (this.index + 1) * -ITEM_ANGLE) - (diffy /* / SearchList.SCROLL_DILUTION */) + startAngle) * SearchList.DEG_TO_RAD;
		}
		
		int x = (int) (centerx + radius * Math.cos(angle));
		int y = (int) (height + radius * Math.sin(angle));
		
		if ( leftSide ) {
			x -= this.width;
		}
		//IKnowUKeyboardService.log(Log.VERBOSE, "SearchListItem update", "angle = "+angle);
		/*
		double yratio = (y / (double) (this.searchList.getStartY() / 2.0));
		
		if (yratio > 1) {
			yratio = (2 - yratio) + 0.5;
			this.height = this.preferredHeight * yratio;
			this.fontHeight = this.searchList.kbView.searchListTextSize * 1;
		} else {
			yratio += 0.5;
			this.height = this.preferredHeight * yratio;
			this.fontHeight = this.searchList.kbView.searchListTextSize * yratio;
		}
		*/
		
		//IKnowUKeyboardService.log(Log.VERBOSE, "SearchListItem update", "diff = "+diffy+", x = "+x+", y = "+y+", width = "+width);
		
		this.xCoord = x;
		this.yCoord = y;
	}
	
	/**
	 * Draw the search list item at the specified coords
	 * 
	 * @param canvas the canvas to draw on
	 * @param x the x-coord to draw the item at
	 * @param y the y-coord to draw the item at
	 * @param paint the paint object to use for drawing
	 * @param textColor the color of this item's text
	 * @param keyColor the color of this item's background
	 * @param pressedColor the color of this item when it is pressed
	 */
	public void draw(Canvas canvas, int x, int y, Paint paint, int textColor, int keyColor, int pressedColor) {
		
		paint.setTextSize((float) this.fontHeight);
		
		float textWidth = paint.measureText(this.textShow);
		
		int padX = DimensionConverter.stringToDimensionPixelSize(6.5+"dp", this.resources.getDisplayMetrics());
		
		int padY = (int) ((this.height / 2) + (this.fontHeight / 2))/* / 2*/;
		padY = DimensionConverter.stringToDimensionPixelSize(padY+"dp", this.resources.getDisplayMetrics());
		
		boolean textBigger = false;
		int tempWidth = this.width;
		
		if (textWidth > this.width - padX) {
			textBigger = true;
			int dif = (int) (textWidth - this.width);
			x -= dif;
			if (x < 0) {
				x = 0;
			}
			tempWidth += dif;
		}
		
		RectF rect = new RectF( x, y, x+tempWidth, (float) (y+this.height) );
		
		if (!this.isHovered) {
			paint.setColor(keyColor);
			paint.setStyle(Paint.Style.STROKE);
			paint.setStrokeWidth(3);
			canvas.drawRoundRect(rect, 5, 5, paint);
			
			paint.setAlpha(200);
			paint.setStyle(Paint.Style.FILL);
			canvas.drawRoundRect(rect, 5, 5, paint);
			
			paint.setColor(textColor);

			if (textWidth > tempWidth) {
				canvas.drawText(/*this.searchList.rootText+*/this.textShow, 0, 20, x+padX, y+padY, paint);
			} else {
				canvas.drawText(/*this.searchList.rootText+*/this.textShow, x+padX, y+padY, paint);
			}
		} else {
			int width = this.width;
			
			if (textBigger) {
				width = (int)(textWidth + padX);
			}
			
			rect.right = rect.left + width;
			
			paint.setColor(pressedColor);
			paint.setStyle(Paint.Style.STROKE);
			paint.setStrokeWidth(3);
			canvas.drawRoundRect(rect, 5, 5, paint);
			
			paint.setAlpha(200);
			paint.setStyle(Paint.Style.FILL);
			canvas.drawRoundRect(rect, 5, 5, paint);
			
			paint.setColor(textColor);
			canvas.drawText(/*this.searchList.rootText+*/this.textShow, x+padX, y+padY, paint);
		}
	}
	
	/**
	 * This indicates whether or not this item has more predictions behind it. If it does it will be shown with a "..."
	 * to indicate that the user cna move deeper into the list
	 * 
	 * @param chunk Whether or not this item is a chunk of a larger prediction
	 */
	public void setIsChunk(boolean chunk) {
		this.isChunk = chunk;
	}
	
	public boolean isChunk() {
		return this.isChunk;
	}
}
