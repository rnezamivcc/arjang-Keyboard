/**
 * Classes to support swiping action in keyboard
 * 
 * @author Reza Nezami
 *
 */
package com.iknowu.swipe;

import android.util.Log;

import com.iknowu.IKnowUKeyboardService;


public class Swipe
{	  
	public Swipe() {
		  count = 0;
		  sPath = new Sw_Node[MAX_POINTS];
		  for(int i=0; i<MAX_POINTS; i++)
			  sPath[i] = new Sw_Node();
	}
	
	public int size() {
		return count;
	}
	public Sw_Node get(int i) {
		return sPath[i];
	}
	public Sw_Node getLast() {
		if(count > 0)
			return sPath[count-1];
		return null;
	}
	public void addPoint(char cc, float x, float y, boolean required){
		if(count>0 && cc == sPath[count-1].val)
			sPath[count].count++;
		else
			sPath[count++].set(cc, x, y, 1, required);
	}
	
	public String printPath(){
		char seq[] = new char[count];
		for(int i=0; i<count;i++)
			seq[i] = sPath[i].val;
		String path = String.valueOf(seq);
		IKnowUKeyboardService.log(Log.DEBUG, "Swipe = ", path);
		return path;
	}
	
	public void clear(){
		count = 0;
	}
	public Sw_Node[] getNodes() {
		if(count<= 0)
			return null;
		Sw_Node[] output = new Sw_Node[count];
		for(int i=0; i<count; i++)
			output[i] = sPath[i];
		
		return output;
	}
	
   // final int max_count = 20; // max number of Sw_Node in the path
    public static final int MAX_POINTS = 250;
    
    private int count;
	private Sw_Node  sPath[]; // path containing all elements in the swipe path.
}
	
	 /*
     * This is the main container class to contain info for swipe path and update it. This is going to be used
     * by jni engine for its algorithms. 
     */
  /*  public class SwipePoints {
         public class Point {
        	public char c;
        	public float vx, vy;
        	public void set(char cc, float v_x, float v_y) {
        		c = cc; vx = v_x; vy = v_y;
        	}
        }
        
         public SwipePoints() {
        	 pts = new Point[max_count];
        	 for(int i=0; i<max_count; i++)
        		 pts[i] = new Point();
        	 count = 0;
         }
         
        public void addPoint(char cc, float v_x, float v_y){
        	pts[count++].set(cc, v_x, v_y);
        }
        
        public Point[] getPoints() { return pts; }
        private Point[] pts; 
        private int count; // number of points in the path pts
        public int getCount() { return count; }
    	void clear() { count = 0; }
    }
    public SwipePoints swipePts;
*/