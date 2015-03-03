package com.iknowu.popup;

import android.content.Context;
import android.content.res.XmlResourceParser;
import android.util.Log;
import android.view.Gravity;
import android.widget.PopupWindow;

import com.iknowu.IKnowUKeyboard.Key;
import com.iknowu.IKnowUKeyboardService;
import com.iknowu.IKnowUKeyboardView;
import com.iknowu.R;
import com.iknowu.scroll.KeyboardScreen;
import com.iknowu.util.DimensionConverter;

import java.util.ArrayList;
import java.util.HashMap;

/**
 * 
 * 
 * @author Justin Desjardins
 *
 */
public class PopupManager {

    private static final String Y_OFFSET = "90dp";
    private static final String Y_OFFSET_SINGLE_ROW = "60dp";

	private Context context;
	private PopupWindow popupWindow;
	private IKnowUKeyboardService inputService;
	
	final private int [] nmaxcolumns = { 0,1,2,3,4,4,4,4,4,5, 5, 6, 6, 7, 7, 8 };
	
	private static final String TAG_CHAR = "character";
	private static final String TAG_KEY = "key";

    private int currentKeyboardLayout;

	private HashMap<String, PopupKeyboard> popupKeyboards;
	private PopupKeyboardView popupKbView;
	private IKnowUKeyboardView parentView;

    private int popupYOffset;
    private int popupYOffsetSingle;

    public int popupLocX;
    public int popupLocY;
	
	public PopupManager( IKnowUKeyboardService service) {
		this.context = service;
		this.inputService = service;
		this.popupKeyboards = new HashMap<String, PopupKeyboard>();

        this.popupYOffset = DimensionConverter.stringToDimensionPixelSize(Y_OFFSET, context.getResources().getDisplayMetrics());
        this.popupYOffsetSingle = DimensionConverter.stringToDimensionPixelSize(Y_OFFSET_SINGLE_ROW, context.getResources().getDisplayMetrics());
	}

    public void setAndLoadCurrentKeyboard( int curLayout ) {
        this.currentKeyboardLayout = curLayout;

        IKnowUKeyboardService.log(Log.VERBOSE, "POPUP MANAGER", "Set and load current keyboard");

        switch ( this.currentKeyboardLayout ) {
            case KeyboardScreen.QWERTY:
                this.loadPopupResource(R.xml.popup_english_keyboards);
                break;
            case KeyboardScreen.QWERTY_SPANISH:
                this.loadPopupResource(R.xml.popup_qwerty_spanish_keyboards);
                break;
            case KeyboardScreen.AZERTY:
                this.loadPopupResource(R.xml.popup_english_keyboards);
                break;
            case KeyboardScreen.QWERTZ:
                this.loadPopupResource(R.xml.popup_english_keyboards);
                break;
            case KeyboardScreen.QZERTY:
                this.loadPopupResource(R.xml.popup_english_keyboards);
                break;
            case KeyboardScreen.RUSSIAN:
                this.loadPopupResource(R.xml.popup_rus_keyboards);
                break;
            case KeyboardScreen.KOREAN:
                this.loadPopupResource(R.xml.popup_korean_keyboards);
                break;
            case KeyboardScreen.COMPRESSED:
                this.loadPopupResource(R.xml.popup_compressed_keyboards);
                break;
            case KeyboardScreen.NUMERIC:
                this.loadPopupResource(R.xml.popup_number_keyboards);
                break;
        }
    }
	
	/**
	 * Load an xml file that dexcribes all the popup keys for a keyboard layout.
	 * @param resId the id of the xml file located in the xml folder
	 */
	public void loadPopupResource(int resId) {
		this.popupKeyboards = new HashMap<String, PopupKeyboard>();
		
		XmlResourceParser parser = this.context.getResources().getXml(resId);
		
		boolean inChar = false;
		boolean inKey = false;
		PopupKeyboard currentKeyboard = new PopupKeyboard("");
		String theChar = "";
		
		try {
            int event;
            while ((event = parser.next()) != XmlResourceParser.END_DOCUMENT) {
                if (event == XmlResourceParser.START_TAG) {
                    String tag = parser.getName();
                    if (TAG_CHAR.equals(tag)) {
                        inChar = true;
                        theChar = parser.getAttributeValue(null, "value");
                       // Log.v("PopupManager", "character = "+theChar);
                        currentKeyboard = new PopupKeyboard(theChar);
                       // x = this.popupPadding;
                    } else if (TAG_KEY.equals(tag)) {
                        inKey = true;
                        String keyChr = parser.getAttributeValue(null, "value");
                        boolean isDefault = parser.getAttributeBooleanValue(null, "default", false);
                        
                        //Log.w("PopupManager", "key char = "+keyChr+", default = "+isDefault);
                        
                        currentKeyboard.addKeyChar(keyChr, isDefault);
                    }
                } else if (event == XmlResourceParser.END_TAG) {
                    if (inKey) {
                        inKey = false;
                    } else if (inChar) {
                    	if ( currentKeyboard.getKeyChars().size() > 0 )
                    		this.popupKeyboards.put(theChar, currentKeyboard);
                    }
                }
            }
            //this.popupHeight = y;
        } catch (Exception e) {
        	IKnowUKeyboardService.sendErrorMessage(e);
        }
	}
	
	public String getDefaultChar( String key ) {
		if (this.popupKeyboards.get(key) != null) {
			return this.popupKeyboards.get(key).getDefaultChar();
		} else {
			return "";
		}
	}
	
	public ArrayList<String> getKeyboardChars( String key ) {
		if (this.popupKeyboards.get(key) != null) {
			return this.popupKeyboards.get(key).getKeyChars();
		} else {
			return null;
		}
	}
	
	private int getMaxColumns(int len) 	{
		try {
			int maxColumns = 8;
			if (len < 21 && len >= 0) {
				maxColumns = nmaxcolumns[len];
				if (len == 9) {
					maxColumns = 5;
				}
			}
			return maxColumns;
		} catch (Exception e) {
			IKnowUKeyboardService.sendErrorMessage(e);
			return 5;
		}
	}
	
	public boolean showPopup( IKnowUKeyboardView parent, Key key, int maxWidth, int touchX, int touchY ) {
		try {
			String keyLabel = (String) key.label;
			PopupKeyboard pkb = null;
			
			IKnowUKeyboardService.log(Log.VERBOSE, "show popup", "key label = "+keyLabel);
			
			if (keyLabel != null) {
				if (parent.getKeyboard().isShifted()) {
					keyLabel = keyLabel.toUpperCase();
				}
				pkb = this.popupKeyboards.get(keyLabel);
			}
			
			//if null, we don't have a popup keyboard for this key
			//therefore return false;
			if (pkb == null && key.popupResId < 0) {
				return false;
			}
			//Log.e("PopupManager", "Showing Popup");
			//LinearLayout linlay = (LinearLayout) this.inputService.getLayoutInflater().inflate(R.layout.popup_relatedchars_keyboard_no_close, null);
			this.popupKbView = new PopupKeyboardView(this.context);//linlay.findViewById(R.id.popupKbView);
			this.popupKbView.setSoftKeyboard(this.inputService);
			this.popupKbView.processAttributes(this.inputService.themeId);
			
			if (popupWindow == null) {
				popupWindow = new PopupWindow(this.context);
				popupWindow.setBackgroundDrawable(null);
			}
			
			this.parentView = parent;
			
			ArrayList<String> chars = null;
			
			if (keyLabel != null) {
				if (pkb != null) {
					chars = pkb.getKeyChars();
				}
			}
			
			boolean leftToRight = true;
			if (key.x > parent.getWidth()/2) {
				leftToRight = false;
			}

            // Popup keys should be the same height as the keyboard keys since user can adjust key height in settings.
            this.popupKbView.popupKeyHeight = key.height;
			boolean hasKb = false;
			if (chars == null) {
				hasKb = this.popupKbView.createKeyboard(key.popupResId, 5, maxWidth, leftToRight);
			} else {
				hasKb = this.popupKbView.createKeyboard(chars, this.getMaxColumns(chars.size()), maxWidth, leftToRight);
			}
			
			if (hasKb) {
				//every popup has a default selection
				int defaultPressedKeyIdx = 0;
				
				// see first whether we can still shift it left or right
				int paddingX = 5;
				int leftEdgeX = touchX /*- this.popupKbView.popupPadding*/ - (PopupKey.MAX_WIDTH / 2);
				
				int centerX = key.x+key.width/2;
				
				if (key.x > parent.getWidth()/2) {
					// absolute screen coordinate (taken from kbview, which takes the whole screenwidth)
					// right adjust make sure we take care of the window padding on the right side
					//int rightEdgeX = centerX + key.width/2;
					int rightEdgeX = touchX + (this.popupKbView.popupPadding * 2);
					// right slide
					if ((rightEdgeX - this.popupKbView.popupWidth) < paddingX) {
						// move the popup further to the right
						int diffX = this.popupKbView.popupWidth - rightEdgeX + paddingX;
						// falls off the left side of screen, adjust further to the right
						rightEdgeX += diffX;
					}
					leftEdgeX = rightEdgeX - this.popupKbView.popupWidth + this.popupKbView.popupPadding;
				}
	
				popupKbView.setDefaultCharacterIdx( defaultPressedKeyIdx, key.height / 2); // more vertical room
				
				// no preview on these keys
				//popupKbView.setPreviewEnabled(false);
				
				// show at location is relative from the parentview window.
				// don't be fooled with landscape, the whole screen in extracted mode is part of the keyboard
				int x = leftEdgeX;
                int y = key.y + key.height - this.popupKbView.popupHeight - this.popupKbView.popupPadding - this.popupKbView.sensitivityRange/* - parent.getSuggestionsView().getHeight()*/;

                /*if (this.popupKbView.isSingleRow) {
                    y += this.popupYOffsetSingle;
                } else {
                    y += this.popupYOffset;
                }*/
				
				int[] parentLoc;
                parentLoc = new int[2];
                parent.getLocationOnScreen(parentLoc);
                x += parentLoc[0];
                //y += parentLoc[1];
			    if (this.inputService.isExtractViewShown() || this.inputService.isFullscreenMode()) {
					//x += parentLoc[0];
					y += parentLoc[1];
					y -= this.popupYOffset;
				}

                this.popupLocX = x;
                this.popupLocY = y;
				
				//Log.d("PopupManager", "showing popup at x = "+x+" and y = "+y);
				this.popupWindow.setContentView(this.popupKbView);
				if (this.popupWindow.isShowing()) {
					this.popupWindow.update(x, y, this.popupKbView.popupWidth + (2*this.popupKbView.popupPadding), this.popupKbView.popupHeight + (2*this.popupKbView.popupPadding) + this.popupKbView.sensitivityRange);
				} else {
					this.popupWindow.setWidth(this.popupKbView.popupWidth + (2*this.popupKbView.popupPadding));
					this.popupWindow.setHeight(this.popupKbView.popupHeight + (2*this.popupKbView.popupPadding) + this.popupKbView.sensitivityRange);
                    this.popupWindow.setAnimationStyle(R.style.PopupWindowAnimation);
					this.popupWindow.showAtLocation(parent, Gravity.NO_GRAVITY, x, y);
				}
				
				return true;
			} else {
				return false;
			}
		} catch (Exception e) {
			IKnowUKeyboardService.sendErrorMessage(e);
			return false;
		}
	}
	
	public void keyPress( PopupKey key ) {
		if (key.text != null) {
			//Log.d("WLKB", String.format("overrideKbActionListener text %s", key.text));
			this.parentView.onText(key.text);
		}
		else {
            byte[] prefs = new byte[key.codes.length];
            for (int i=0; i < key.codes.length; i++) {
                prefs[i] = 1;
            }

            String lbl = "";
            if (key.label != null) lbl = key.label.toString();
            this.parentView.onKey(lbl, prefs, key.codes[0]);
			//onReleaseProcessing(key.codes[0]);
		}
		
		dismissPopupWindow();
	}
	
	public PopupKeyboardView getPopupKeyboardView() {
		return (PopupKeyboardView) this.popupWindow.getContentView();
	}
	
	public IKnowUKeyboardView getParentView() {
		return this.parentView;
	}

	public PopupWindow getPopupWindow() {
		return popupWindow;
	}

	public void dismissPopupWindow() {
		try {
			if (this.parentView != null) {
				this.parentView.mPopupKeyboardActive = false;
				if (popupWindow != null && popupWindow.isShowing()) {
					popupWindow.dismiss();
					popupWindow = null; // make a new one every time
					if (this.parentView  != null) {
						this.parentView.invalidate();
					}
				}
			}
		} catch (Exception e) {
			IKnowUKeyboardService.sendErrorMessage(e);
		}
	}
}
