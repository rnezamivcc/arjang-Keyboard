package com.wordlogic.lib;

public class KeyPosition {
	public final char ch;
	public final float x1, x2;
	public final float y1, y2;
	
	public KeyPosition(char c,float x,float y,float width,float height) {
		ch = Character.toLowerCase(c);
		x1 = x;
		x2 = x + width;
		y1 = y;
		y2 = y + height;
	}
	
	public KeyPosition(char c,double x,double y,double width,double height) {
		this(c,(float)x,(float)y,(float)width,(float)height);
	}

	public String toString() {
		StringBuilder sbuilder = new StringBuilder();
		
		sbuilder.append("KeyPosition {ch = '"+ch+"', ("+x1+","+y1+")  ("+x2+","+y2+") }");
		
		return sbuilder.toString();
	}
}