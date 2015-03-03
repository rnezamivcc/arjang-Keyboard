/**
 * Classes to support swiping action in keyboard
 * 
 * @author Reza Nezami
 *
 */
package com.iknowu.swipe;

import android.util.Log;

import com.iknowu.IKnowUKeyboardService;


	public class Swipe{
		public class Sw_Node {
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
	        public float x, y;
	        public byte count;
	      //  public int color;
	    //    public byte pref;
		        
	        public Sw_Node() { val = 0; x = y = 0;}
        	public void set(char cc, float xx, float yy, byte cnt, boolean req) {
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
		public void addPoint(char cc, float x, float y, byte cnt, boolean required){
			sPath[count++].set(cc, x, y, cnt, required);
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