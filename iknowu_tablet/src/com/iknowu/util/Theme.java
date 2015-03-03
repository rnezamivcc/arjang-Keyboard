package com.iknowu.util;

import android.content.Context;
import android.content.res.Resources;
import android.content.res.XmlResourceParser;
import android.graphics.Typeface;

import com.iknowu.IKnowUKeyboardService;
import com.iknowu.R;
import com.iknowu.Typefaces;

/**
 * Created by Justin on 17/10/13.
 *
 */
public class Theme {

    public static final String TAG_THEME_ITEM = "item";

    public static int KEY_COLOR;
    public static int KEY_PRESSED_COLOR;
    public static int KEY_DARK_COLOR;
    public static int BACKGROUND_COLOR;
    public static int KEY_UPPER_ICON_COLOR;
    public static int KEY_COLOR_MORE_THAN_FIVE;
    public static int KEY_COLOR_LESS_THAN_FIVE;
    public static int KEY_TEXT_COLOR;
    public static int CANDIDATE_BACKGROUND_COLOR;
    public static int CANDIDATE_TEXT_COLOR;
    public static int CANDIDATE_SELECTED_COLOR;
    public static int CANDIDATE_ADD_WORD_COLOR;
    public static int CANDIDATE_DELETE_WORD_COLOR;
    public static int CANDIDATE_HIGHEST_PRIORITY_COLOR;
    public static int KEY_STYLE;
    public static int BORDER_STROKE;
    public static int KEY_SHADOW_COLOR;
    public static boolean USE_GRADIENT;
    public static int GRADIENT_DIRECTION;
    public static int CORNER_RADIUS_X;
    public static int CORNER_RADIUS_Y;
    public static int SEARCH_ITEM_COLOR;
    public static int SEARCH_ITEM_PRESSED_COLOR;
    public static int SEARCH_ITEM_BACK_COLOR;
    public static int SEARCH_ITEM_TEXT_COLOR;
    public static int PREVIEW_BACKGROUND_COLOR;
    public static int PREVIEW_TEXT_COLOR;
    public static int PREVIEW_ALT_TEXT_COLOR = -1;
    public static int PREVIEW_BORDER_COLOR;
    public static int SIDE_BACKGROUND_COLOR;

    public static Typeface TYPEFACE;

    public static long LONG_PRESS_TIMEOUT;
    public static long DELETE_WORD_DELAY_TIMEOUT;

    /**
     * Set the theme of this keyboard
     * @param themeId the theme to load
     */
    public static void setTheme(Context context, int themeId) {
        XmlResourceParser parser;
        try {
            parser = context.getResources().getXml(themeId);
        } catch (Resources.NotFoundException nfe) {
            parser = context.getResources().getXml(IKnowUKeyboardService.DEFAULT_KEYBOARD_THEME_ID);
        }

        try {
            int event;
            while ((event = parser.next()) != XmlResourceParser.END_DOCUMENT) {
                if (event == XmlResourceParser.START_TAG) {
                    String tag = parser.getName();
                    if (TAG_THEME_ITEM.equals(tag)) {
                        String attrName = parser.getAttributeValue(null, "name");

                        if (attrName.equals("keyColor")) {
                            KEY_COLOR = parser.getAttributeIntValue(null, "value", 0xFF4f4f4f);
                        } else if (attrName.equals("keyPressedColor")) {
                            KEY_PRESSED_COLOR = parser.getAttributeIntValue(null, "value", 0xFFd3d3d3);
                        } else if (attrName.equals("keyDarkColor")) {
                            KEY_DARK_COLOR = parser.getAttributeIntValue(null, "value", 0xFF2f2f2f);
                        } else if (attrName.equals("backgroundColor")) {
                            BACKGROUND_COLOR = parser.getAttributeIntValue(null, "value", 0xFF000000);
                        } else if (attrName.equals("upperIconColor")) {
                            KEY_UPPER_ICON_COLOR = parser.getAttributeIntValue(null, "value", 0xFFFFFF00);
                        } else if (attrName.equals("keyColorMoreThan5")) {
                            KEY_COLOR_MORE_THAN_FIVE = parser.getAttributeIntValue(null, "value", 0xFF007399);
                        } else if (attrName.equals("keyColorLessThan5")) {
                            KEY_COLOR_LESS_THAN_FIVE = parser.getAttributeIntValue(null, "value", 0xFF009926);
                        } else if (attrName.equals("keyTextColor")) {
                            KEY_TEXT_COLOR = parser.getAttributeIntValue(null, "value", 0xFFFFFFFF);
                        } else if (attrName.equals("candidateBackground")) {
                            CANDIDATE_BACKGROUND_COLOR = parser.getAttributeIntValue(null, "value", 0xFF000000);
                        } else if (attrName.equals("candidateTextColor")) {
                            CANDIDATE_TEXT_COLOR = parser.getAttributeIntValue(null, "value", 0xFFFFFFFF);
                        } else if (attrName.equals("candidateSelectedColor")) {
                            CANDIDATE_SELECTED_COLOR = parser.getAttributeIntValue(null, "value", 0xFF0000FF);
                        } else if (attrName.equals("candidateAddWordColor")) {
                            CANDIDATE_ADD_WORD_COLOR = parser.getAttributeIntValue(null, "value", 0xFF228b22);
                        } else if (attrName.equals("candidateDeleteWordColor")) {
                            CANDIDATE_DELETE_WORD_COLOR = parser.getAttributeIntValue(null, "value", 0xFFb22222);
                        }  else if (attrName.equals("highestPriorityColor")) {
                            CANDIDATE_HIGHEST_PRIORITY_COLOR = parser.getAttributeIntValue(null, "value", 0xFFb22222);
                        } else if (attrName.equals("style")) {
                            KEY_STYLE = parser.getAttributeIntValue(null, "value", 0);
                        } else if (attrName.equals("stroke")) {
                            BORDER_STROKE = parser.getAttributeIntValue(null, "value", 1);
                        } else if (attrName.equals("keyShadowColor")) {
                            KEY_SHADOW_COLOR = parser.getAttributeIntValue(null, "value", 0xDD000000);
                        } else if (attrName.equals("useGradient")) {
                            USE_GRADIENT = parser.getAttributeBooleanValue(null, "value", false);
                        } else if (attrName.equals("gradientDirection")) {
                            GRADIENT_DIRECTION = parser.getAttributeIntValue(null, "value", 0);
                        } else if (attrName.equals("cornerRadiusX")) {
                            CORNER_RADIUS_X = parser.getAttributeIntValue(null, "value", 0);
                        } else if (attrName.equals("cornerRadiusY")) {
                            CORNER_RADIUS_Y = parser.getAttributeIntValue(null, "value", 0);
                        } else if (attrName.equals("searchItemColor")) {
                            SEARCH_ITEM_COLOR = parser.getAttributeIntValue(null, "value", 0xFF000000);
                        } else if (attrName.equals("searchItemPressedColor")) {
                            SEARCH_ITEM_PRESSED_COLOR = parser.getAttributeIntValue(null, "value", 0xFF000000);
                        } else if (attrName.equals("searchBackColor")) {
                            SEARCH_ITEM_BACK_COLOR = parser.getAttributeIntValue(null, "value", 0xFF000000);
                        } else if (attrName.equals("searchItemTextColor")) {
                            SEARCH_ITEM_TEXT_COLOR = parser.getAttributeIntValue(null, "value", 0xFF000000);
                        } else if (attrName.equals("previewBackgroundColor")) {
                            PREVIEW_BACKGROUND_COLOR = parser.getAttributeIntValue(null, "value", 0xFF000000);
                        } else if (attrName.equals("previewTextColor")) {
                            PREVIEW_TEXT_COLOR = parser.getAttributeIntValue(null, "value", 0xFFFFFFFF);
                        } else if (attrName.equals("previewBorderColor")) {
                            PREVIEW_BORDER_COLOR = parser.getAttributeIntValue(null, "value", 0xFF000000);
                        } else if (attrName.equals("sideBackgroundColor")) {
                            SIDE_BACKGROUND_COLOR = parser.getAttributeIntValue(null, "value", 0xFF000000);
                        } else if (attrName.equals("previewAltTextColor")) {
                            PREVIEW_ALT_TEXT_COLOR = parser.getAttributeIntValue(null, "value", 0xFF000000);
                        }
                    }
                } else if (event == XmlResourceParser.END_TAG) {}
            }

            if (PREVIEW_ALT_TEXT_COLOR == -1) {
                PREVIEW_ALT_TEXT_COLOR = PREVIEW_TEXT_COLOR;
            }

            TYPEFACE = Typefaces.get(context, "roboto_bold.ttf");
        } catch (Exception e) {
            IKnowUKeyboardService.sendErrorMessage(e);
        }
    }
}
