package com.iknowu.util;

/**
 * Created by Justin on 02/10/13.
 *
 */
public class POSProcessor {

    public static String TAG_SENTENCE_CLOSER = ".";	            //sentence closer (. ; ? *)
    public static String TAG_LEFT_PARENTH = "(";	            //left paren
    public static String TAG_RIGHT_PARENTH = ")";	            //right paren
    public static String TAG_NOT  = "*";                        //not, n't
    public static String TAG_DASH = "--";       	            //dash
    public static String TAG_COMMA = ",";       	            //comma
    public static String TAG_COLON =  ":";	                    //colon
    public static String TAG_PRE_QUALIFIER = "ABL";	            //pre-qualifier (quite, rather)
    public static String TAG_PRE_QUANTIFIER = "ABN";	        //pre-quantifier (half, all)
    public static String TAG_PRE_QUANTIFIER_BOTH = "ABX";    	//pre-quantifier (both)
    public static String TAG_POST_DETERMINER = "AP";           	//post-determiner (many, several, next)
    public static String TAG_ARTICLE = "AT";                	//article (a, the, no)

    public POSProcessor() {

    }
}
