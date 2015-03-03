package com.iknowu.util;

import android.os.Environment;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.Locale;

/**
 * Utility class for logging error messages.
 *
 * Created by Chris on 08/04/14.
 */
public class MessageLogger {

    private MessageLogger() {}

    /**
     * Print error messages to a log file in the download directory.
     * @param e the exception
     */
    public static void logErrorMessageToDownloadDir(Exception e) throws IOException {
        File directory = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS);

        SimpleDateFormat dateFormat = new SimpleDateFormat("yyyy-MM-dd_HH-mm-ss");
        String dateStr = dateFormat.format(new Date());

        File errorFile = new File(directory, "/iknowu_error_log-" + dateStr + ".txt");
        FileWriter out = new FileWriter(errorFile);
        PrintWriter pWriter = new PrintWriter(out);

        pWriter.write("<------------------------------------------------------------------------->" + System.getProperty("line.separator"));
        pWriter.write("MESSAGE = " + e.getMessage() + System.getProperty("line.separator"));

        Calendar cal = Calendar.getInstance();
        //can only do getDisplayName in api 9 and up
        if (android.os.Build.VERSION.SDK_INT > android.os.Build.VERSION_CODES.FROYO) {
            pWriter.write("DATE = " + cal.getDisplayName(Calendar.MONTH, Calendar.SHORT, Locale.CANADA) +
                    " " + cal.get(Calendar.DAY_OF_MONTH)
                    + ", " + cal.get(Calendar.HOUR)
                    + ":" + cal.get(Calendar.MINUTE)
                    + ":" + cal.get(Calendar.SECOND)
                    + System.getProperty("line.separator"));
        }

        pWriter.write(System.getProperty("line.separator"));
        e.printStackTrace(pWriter);

        pWriter.write("<------------------------------------------------------------------------->" + System.getProperty("line.separator"));

        pWriter.flush();
        pWriter.close();
    }
}
