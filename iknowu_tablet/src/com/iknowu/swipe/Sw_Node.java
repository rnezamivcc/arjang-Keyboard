/**
 * Classes to support swiping action in keyboard
 * 
 * @author Reza Nezami
 *
 */
package com.iknowu.swipe;

public class Sw_Node 
{
    public float x, y;
    public char val;  // main char this key represents. Maybe later on we expand it to represent multiple chars, for example for compressed keyboards
 //   byte coverage; // integer from 1 to 127 representing how much of the key was covered when used swipped on it, based on distance from center of the key
//    boolean lingered; // flag to show if user lingered on this key more than normal.
//	boolean pressed; // flag to show if user pressed on this key more than normal.
//	boolean required; // flag to say if this key is a required key on the path, so cannot be ingnored. For instance start key, or end key or a turn key and such.
    
    /*
     *  position and motion values, to be used in drawing a line for displaying the user's touch inputs via swiping gestures
     * For now, each key represents only one char, but for compressed keyboard we may need to extend this to multi chars
     * @author Reza
     */
    public int count;
  //  public int color;
//    public byte pref;
        
    public Sw_Node() { val = 0; x = y = 0;}
    public void set(char cc, float xx, float yy, int cnt, boolean req) {
        val = cc; x = xx; y = yy; count = cnt;// required = req;
    }
    public void setPos(float a, float b){
        x = a; y = b;
    }
    public void set(char cc, float xx, float yy){
        val = cc; x=xx; y=yy;
    }
        
    @Override
    public String toString() {
        return "("+val + ":"+x + ", " + y+")";
    }
  }
