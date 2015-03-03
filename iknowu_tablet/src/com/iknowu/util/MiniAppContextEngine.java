package com.iknowu.util;

import android.util.Log;
import android.view.inputmethod.InputConnection;

import com.iknowu.IKnowUKeyboardService;
import com.iknowu.miniapp.MiniAppManager;

import java.util.ArrayList;
import java.util.LinkedList;
import java.util.regex.Pattern;

/**
 * Created by Justin on 01/10/13.
 *
 */
public class MiniAppContextEngine {

    public final Pattern PATTERN_EMAIL;
    public final Pattern PATTERN_PHONE_NUMBER;
    public final Pattern PATTERN_PROPER_NOUN;
    public final Pattern PATTERN_NUMBERS;
    public final Pattern PATTERN_MEASUREMENT;
    public final Pattern PATTERN_EMAIL_START;

    public static final String REGEX_EMAIL = "[_A-Za-z0-9-]+(\\.[_A-Za-z0-9-]+)*@[A-Za-z0-9]+(\\.[A-Za-z0-9]+)*(\\.[A-Za-z]{2,})";
    public static final String REGEX_PHONE_NUMBER = "(\\d{3}-?){1,2}\\d{4}";
    public static final String REGEX_PROPER_NOUN = "[A-Z][a-z]+";
    public static final String REGEX_NUMBERS = "^[0-9]+";
    public static final String REGEX_MEASUREMENT = "([\\d.]+)(\\s*)+(lbs?|oz|mg|g|kg|mm|cm|m|km|ft|in|yd|mi|C|F)";
    public static final String REGEX_EMAIL_START = "[_A-Za-z0-9-]+(\\.[_A-Za-z0-9-]+)*@[A-Za-z0-9]+(\\.[A-Za-z0-9])*";

    public static final String[] CALENDAR_TERMS = { "appointment", "meeting", "meet", "schedule", "reservation", "reserve",
            "sunday", "monday", "tuesday", "wednesday", "thursday", "friday", "saturday",
            "january", "february", "march", "april", "may", "june", "july", "august", "september", "october", "november", "december"};
    public static final String[] WEB_TERMS = { "link", "website", "web", "browser" };

    public StringBuilder textBody;

    public LinkedList<Word> words;

    private boolean needNewWord;

    public MiniAppContextEngine() {

        this.PATTERN_EMAIL = Pattern.compile(REGEX_EMAIL);
        this.PATTERN_EMAIL_START = Pattern.compile(REGEX_EMAIL_START);
        this.PATTERN_MEASUREMENT = Pattern.compile(REGEX_MEASUREMENT);
        this.PATTERN_NUMBERS = Pattern.compile(REGEX_NUMBERS);
        this.PATTERN_PHONE_NUMBER = Pattern.compile(REGEX_PHONE_NUMBER);
        this.PATTERN_PROPER_NOUN = Pattern.compile(REGEX_PROPER_NOUN);
    }

    /**
     * Tell the engine to advance on a String
     * This string will include punctuation.
     *
     * @param chars the String of chars to append
     */
    public void advanceString(String chars) {
        this.words.getLast().addChars(chars);
    }

    /**
     * Tell the engine to advance on a space
     * This will create a new node to add characters to
     *
     */
    public void advanceSpace() {
        Word word = new Word("");
        this.words.add(word);
        //TODO: analyze word for context and other things...
    }

    /**
     * Backspace out a specified number of characters
     * checking to see if words are being removed from the list
     *
     * @param length
     */
    public void backspace(int length) {

        int deleted = 0;
        Word curWord;

        while( deleted < length) {
            curWord = this.words.getLast();

            if ( (length - deleted) <= curWord.getWordLength()) {
                curWord.deleteChars( (length - deleted) );
                deleted = (length - deleted);
            } else {
                this.words.removeLast();
                deleted = curWord.getWordLength() + 1; //plus 1 for a space that would be in between.
                curWord = null;
            }
        }
    }

    public void analyzeForContext() {
        new Thread(new Runnable() {
            @Override
            public void run() {

            }
        }).start();
    }

    private void loopWords() {
        for (Word word : this.words) {

        }
    }

    /**
     * Analyze a string of words for any possible key words that could be used to fire up a Reach mini-app.
     *
     * @param words the text to analyze
     */
    public ArrayList<String[]> getContext(IKnowUKeyboardService kbs, MiniAppManager mngr, String words) {

        //kbs.startTimer();

        IKnowUKeyboardService.log(Log.WARN, "getContext()", "start function");
        //this.startTimer();

        ArrayList<String[]> ret = new ArrayList<String[]>();

        InputConnection ic = kbs.getCurrentInputConnection();
        String current = kbs.getCurrentWord();
        String last = kbs.getLastWord(ic, false, false);
        String lastTwo = "";

        if (current == null || current.length() <= 0) {
            current = kbs.getLastWord(ic, false, false);
            if (last.length() > 0 && !last.equals(current)) {
                lastTwo = last+" "+current;
            } else {
                lastTwo = kbs.getWordsBeforeCursor(ic, 2);
            }
        } else {
            //String last = this.getLastWord(ic, false, false);
            if (last.length() > 0 && !last.equals(current)) {
                lastTwo = last+" "+current;
            }
            //log(Log.VERBOSE, "Last two = ", ""+lastTwo);
        }

        final String lastTwoLower = lastTwo.toLowerCase();
        final String currentLower = current.toLowerCase();

        IKnowUKeyboardService.log(Log.VERBOSE, "current = |"+current+"|", "last Two = |"+lastTwo+"| , last = |"+last+"|");

        if (mngr.needsCategory(MiniAppManager.CATEGORY_MINIAPP_EMAIL_ADDRESS)) {
            if (PATTERN_EMAIL.matcher(current).matches()) {
                //log(Log.VERBOSE, "Text = "+current, "Mini-app Category = email");
                String[] ar = new String[2];
                ar[0] = MiniAppManager.CATEGORY_MINIAPP_EMAIL_ADDRESS;
                ar[1] = current;
                ret.add(ar);
            }
        }

        if (mngr.needsCategory(MiniAppManager.CATEGORY_MINIAPP_PHONE_NUMBER)) {
            if (PATTERN_PHONE_NUMBER.matcher(current).matches()) {
                IKnowUKeyboardService.log(Log.VERBOSE, "Text = "+current, "Mini-app Category = phone #");
                String[] ar = new String[2];
                ar[0] = MiniAppManager.CATEGORY_MINIAPP_PHONE_NUMBER;
                ar[1] = current;
                ret.add(ar);
            }
        }

        if (mngr.needsCategory(MiniAppManager.CATEGORY_MINIAPP_WEB_URL)) {
            if (lastTwoLower != null && lastTwoLower.contains("http://") || lastTwoLower.contains("www.") || lastTwoLower.contains(".com")
                    || lastTwoLower.contains(".ca") || lastTwoLower.contains(".org") || lastTwoLower.contains(".net")
                    || lastTwoLower.contains(".us") || lastTwoLower.contains(".edu") || lastTwoLower.contains(".gov")) {
                IKnowUKeyboardService.log(Log.VERBOSE, "Text = "+current, "Mini-app Category = url");
                String[] ar = new String[2];
                ar[0] = MiniAppManager.CATEGORY_MINIAPP_WEB_URL;
                ar[1] = current;
                ret.add(ar);
            } else if (lastTwoLower != null && (lastTwoLower.contains(WEB_TERMS[0]) || lastTwoLower.contains(WEB_TERMS[1])
                    || lastTwoLower.contains(WEB_TERMS[2]) || lastTwoLower.contains(WEB_TERMS[3]) ) ) {
                IKnowUKeyboardService.log(Log.VERBOSE, "Text = "+current, "Mini-app Category = url");
                String[] ar = new String[2];
                ar[0] = MiniAppManager.CATEGORY_MINIAPP_WEB_URL;
                ar[1] = current;
                ret.add(ar);
            }
        }

        if (mngr.needsCategory(MiniAppManager.CATEGORY_MINIAPP_PROPER_NOUN)) {
            if (PATTERN_PROPER_NOUN.matcher(current).matches()) {
                if ( lastTwo.length() > 0 && Character.isUpperCase(lastTwo.charAt(0)) ) {
                    IKnowUKeyboardService.log(Log.VERBOSE, "Text = "+lastTwo, "Mini-app Category = noun");
                    String[] ar = new String[2];
                    ar[0] = MiniAppManager.CATEGORY_MINIAPP_PROPER_NOUN;
                    ar[1] = (lastTwo != null) ? lastTwo.trim() : "";
                    ret.add(ar);
                } else {
                    IKnowUKeyboardService.log(Log.VERBOSE, "Text = "+current, "Mini-app Category = noun");
                    String[] ar = new String[2];
                    ar[0] = MiniAppManager.CATEGORY_MINIAPP_PROPER_NOUN;
                    ar[1] = (current != null) ? current.trim() : "";
                    ret.add(ar);
                }
            }
        }

        if (mngr.needsCategory(MiniAppManager.CATEGORY_MINIAPP_PROPER_NOUN_EXTENDED)) {
            if (PATTERN_PROPER_NOUN.matcher(current).matches()) {
                if ( lastTwo.length() > 0 && Character.isUpperCase(lastTwo.charAt(0)) ) {
                    IKnowUKeyboardService.log(Log.VERBOSE, "Text = "+lastTwo, "Mini-app Category = noun");
                    String[] ar = new String[2];
                    ar[0] = MiniAppManager.CATEGORY_MINIAPP_PROPER_NOUN_EXTENDED;
                    ar[1] = (lastTwo != null) ? lastTwo.trim() : "";
                    ret.add(ar);
                } else {
                    IKnowUKeyboardService.log(Log.VERBOSE, "Text = "+current, "Mini-app Category = noun");
                    String[] ar = new String[2];
                    ar[0] = MiniAppManager.CATEGORY_MINIAPP_PROPER_NOUN_EXTENDED;
                    ar[1] = (current != null) ? current.trim() : "";
                    ret.add(ar);
                }
            } else {
                // If the last word or the last two words are not proper nouns then check last n words.
                final String lastNWords = kbs.getWordsBeforeCursorExtraLong(ic, 4); // n = 4
                // Build a string of proper nouns from the last n words.
                String wordArray[] = lastNWords.split(" ");
                StringBuilder sb = new StringBuilder();
                for (int i = 0; i < wordArray.length; i++) {
                    if (PATTERN_PROPER_NOUN.matcher(wordArray[i]).matches()) {
                        sb.append(((i > 0) ? " " : "") + wordArray[i]);
                    }
                }
                if (sb.length() > 0) {
                    String[] ar = new String[2];
                    ar[0] = MiniAppManager.CATEGORY_MINIAPP_PROPER_NOUN_EXTENDED;
                    ar[1] = sb.toString().trim();
                    ret.add(ar);
                }
            }
        }

        if (mngr.needsCategory(MiniAppManager.CATEGORY_MINIAPP_NUMBER)) {
            if (PATTERN_NUMBERS.matcher(current).matches()) {
                IKnowUKeyboardService.log(Log.VERBOSE, "Text = "+current, "Mini-app Category = number");
                String[] ar = new String[2];
                ar[0] = MiniAppManager.CATEGORY_MINIAPP_NUMBER;
                ar[1] = current;
                ret.add(ar);
            }
        }

        if (mngr.needsCategory(MiniAppManager.CATEGORY_MINIAPP_MEASUREMENT)) {
            if (PATTERN_MEASUREMENT.matcher(lastTwo).matches()) {
                IKnowUKeyboardService.log(Log.VERBOSE, "Text = "+lastTwo, "Mini-app Category = measurement");
                String[] ar = new String[2];
                ar[0] = MiniAppManager.CATEGORY_MINIAPP_MEASUREMENT;
                ar[1] = lastTwo;
                ret.add(ar);
            }

            if (PATTERN_MEASUREMENT.matcher(current).matches()) {
                IKnowUKeyboardService.log(Log.VERBOSE, "Text = "+current, "Mini-app Category = measurement");
                String[] ar = new String[2];
                ar[0] = MiniAppManager.CATEGORY_MINIAPP_MEASUREMENT;
                ar[1] = current;
                ret.add(ar);
            }
        }

        if (mngr.needsCategory(MiniAppManager.CATEGORY_MINIAPP_SCHEDULE)) {
            boolean found = false;
            for (int i=0 ; i < CALENDAR_TERMS.length; i++) {
                if ( current != null && ( currentLower.contains(CALENDAR_TERMS[i]) ) ) {
                    String[] ar = new String[2];
                    ar[0] = MiniAppManager.CATEGORY_MINIAPP_SCHEDULE;
                    ar[1] = current;
                    ret.add(ar);
                    found = true;
                    break;
                }
            }

            if (!found) {
                // If the last word is not a calendar term then check last n words.
                final String lastNWords = kbs.getWordsBeforeCursorExtraLong(ic, 8); // n = 8
                // Build a string of terms from the last n words.
                String wordArray[] = lastNWords.split(" ");
                StringBuilder sb = new StringBuilder();
                for (int i = 0; i < wordArray.length; i++){
                    for (int j = 0; j < CALENDAR_TERMS.length; j++) {
                        if (wordArray[i].toLowerCase().contains(CALENDAR_TERMS[j])) {
                            sb.append(((i > 0) ? " " : "") + wordArray[i]);
                            break;
                        }
                    }
                }
                if (sb.length() > 0) {
                    String[] ar = new String[2];
                    ar[0] = MiniAppManager.CATEGORY_MINIAPP_SCHEDULE;
                    ar[1] = sb.toString().trim();
                    ret.add(ar);
                }
            }
        }

        if (current != null && current.length() > 0) {
            String[] ar = new String[2];
            ar[0] = MiniAppManager.CATEGORY_MINIAPP_ALL_TEXT;
            ar[1] = (String) ic.getTextBeforeCursor(IKnowUKeyboardService.EXTRA_LONG_WORD_LENGTH, 0);
            ret.add(ar);

            IKnowUKeyboardService.log(Log.VERBOSE, "Text = "+current, "Mini-app Category = last word");
            ar = new String[2];
            ar[0] = MiniAppManager.CATEGORY_MINIAPP_LAST_WORD;
            ar[1] = current;
            ret.add(ar);
        }

        //kbs.stopTimer("getContext()");
        if (ret.size() <= 0) {
            return null;
        } else {
            return ret;
        }
    }
}
