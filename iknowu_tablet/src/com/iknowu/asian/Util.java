package com.iknowu.asian;

import java.util.ArrayList;
import java.util.List;

public class Util {
	public static final String      LINE_SEPARATOR  = System.getProperty("line.separator");

    /**
     * Blank or null String input to check the status of
     * 
     * @author therocks
     * @param str check할 string
     * @return null If there is no value or false, In the case of non-true
     */
    public static boolean valid(String str)
    {
            if( str == null || str.trim().equals("") ) return false;
            return true;
    }
    
    /**
     * jdk1.3 In String.split(String) and use it to support the implementation and oemeuro
     * com.prompt.util Taken from
     * 
     * @author      therocks
     * @since       2006. 9. 23
     * @param str
     * @param delimiter
     * @return
     */
    public static String[] split(String string, String delimiter)
    {
            if( string == null ) return null;
            List list = new ArrayList();
            int idx = -1;
            String frag = null, rStr = string;
            while( true ) {
                    idx = rStr.indexOf(delimiter);
                    if( idx < 0 ) {
                            list.add(rStr);
                            break;
                    }
                    frag = rStr.substring(0, idx);
                    list.add(frag);
                    rStr = rStr.substring(idx + delimiter.length());
            }

            int listSize = list.size();
            String[] ret = new String[listSize];
            for( int i = 0; i < listSize; i++ ) {
                    ret[i] = (String) list.get(i);
            }
            return ret;
    }


    /**
     * implements Long.bitCount(long l) for JDK1.4.2 or lower version
     * 
     * @author      therocks
     * @since       2007. 7. 6
     * @param l
     * @return
     */
    public static int bitCount(long l)
    {
            int ret = 0;
            String str = Long.toBinaryString(l);
            for( int i = 0, stop = str.length(); i < stop; i++ ) {
                    if( str.charAt(i) == '1' ) ret++;
            }
            return ret;
    }


    /**
     * 
     * @param str
     * @return
     */
    private static int getTabCnt(String str)
    {
            int cnt = 0;
            char ch;
            for( int i = 0; i < str.length(); i++ ) {
                    ch = str.charAt(i);
                    if( ch == ' ' || ch == '\t' ) cnt++;
            }
            return cnt;
    }


    /**
     * Returns the tab for a certain amount of
     * 
     * @param cnt
     * @return
     */
    private static String getTab(int cnt)
    {
            String tab = "";
            for( int i = 0; i < cnt; i++ )
                    tab += "\t";
            return tab;
    }


    /**
     * A certain size, the width of the string makes. Passing 
     * tab size and width as the width of the tab to be added to the return.
     * 
     * @author      therocks
     * @param string
     * @param tabSize
     * @param width
     * @return
     */
    public static String getTabbedString(String string, int tabSize, int width)
    {
            int cnt = (string == null ? 0 : string.getBytes().length);
            String ret = string + getTab((width - cnt) / tabSize);
            if( cnt % tabSize != 0 ) ret += "\t";
            return ret;
    }
}
