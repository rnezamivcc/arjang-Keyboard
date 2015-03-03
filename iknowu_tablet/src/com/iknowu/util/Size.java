package com.iknowu.util;

import android.content.Context;
import android.util.DisplayMetrics;
import android.view.WindowManager;

import com.iknowu.IKnowUKeyboardService;

/**
 * Created by Justin on 30/10/13.
 *
 */
public class Size {

    /**
     * Used to calculate the size of the screen in inches
     * @return the size of the screen in inches as an integer
     */
    public static int calculateScreenInches(Context context) {
        try {
            DisplayMetrics dm = new DisplayMetrics();
            WindowManager window = (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);
            window.getDefaultDisplay().getMetrics(dm);
            double x = Math.pow(dm.widthPixels/dm.xdpi,2);
            double y = Math.pow(dm.heightPixels/dm.ydpi,2);
            double screenInches = Math.sqrt(x+y);
            //Log.d("SCREEN INCHES =","" + screenInches);
            return (int) screenInches;
        } catch (Exception e) {
            IKnowUKeyboardService.sendErrorMessage(e);
            return 4;
        }
    }

    public static int getScreenWidth(Context context) {
        try {
            DisplayMetrics dm = new DisplayMetrics();
            WindowManager window = (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);
            window.getDefaultDisplay().getMetrics(dm);
            //Log.d("SCREEN INCHES =","" + screenInches);
            return dm.widthPixels;
        } catch (Exception e) {
            IKnowUKeyboardService.sendErrorMessage(e);
            return -1;
        }
    }

    public static int getScreenHeight(Context context) {
        try {
            DisplayMetrics dm = new DisplayMetrics();
            WindowManager window = (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);
            window.getDefaultDisplay().getMetrics(dm);
            //Log.d("SCREEN INCHES =","" + screenInches);
            return dm.heightPixels;
        } catch (Exception e) {
            IKnowUKeyboardService.sendErrorMessage(e);
            return -1;
        }
    }
}
