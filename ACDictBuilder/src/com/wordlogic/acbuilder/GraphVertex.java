package com.wordlogic.acbuilder;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.HashMap;

public class GraphVertex {
	public final char ch;
	public HashMap<Character,GraphVertex> children;
	public ArrayList<GraphVertex> parents;
	public int offset;
	
	public GraphVertex(char c) {
		ch = c;
		children = new HashMap<Character,GraphVertex>();
		parents = new ArrayList<GraphVertex>();
		offset = -1;
	}
	
	public GraphVertex getChildByChar(char c) {
		return children.get(c);
	}
	
	/**
	 * Return the ith child of this vertex
	 * 
	 * @param i		the index of the child to return must be in the range [0,n) where n is the value returned by a call to the
	 * 				method countChildren()
	 * @return
	 */
	public GraphVertex getChild(int i) {
		int n = 0;
		
		for(GraphVertex v : children.values()) {
			if(n == i) { return v; }
			n++;
		}
		
		throw new IllegalArgumentException();
	}
	
	public int countChildren() {
		return children.size();
	}
	
	public GraphVertex addChild(char c) {
		GraphVertex ret;
		
		ret = new GraphVertex(c);
		
		children.put(c, ret);
		
		ret.parents.add(this);
		
		return ret;
	}
	
	public boolean equals(Object obj) {
		GraphVertex ov;
		int c;
		GraphVertex rc;
		
		if(obj == null) { return false; }
		if(obj.getClass() != GraphVertex.class) { return false; }
		
		ov = (GraphVertex)obj;
		
		if(ov.countChildren() != countChildren()) { return false; }
		
		for(c = 0;c < countChildren();c++) {
			rc = getChild(c);
			
			if(!rc.equals(ov.getChildByChar(rc.ch))) {
				return false;
			}
		}
		
		return true;
	}
	
	public void replaceWith(GraphVertex v) {
		for(GraphVertex child : children.values()) {
			child.parents.remove(this);
			child.parents.add(v);
		}
		
		for(GraphVertex parent : parents) {
			parent.children.put(ch,v);
		}
	}
	
	public void writebin(ByteBuffer bb) {
		bb.putChar(ch);
		bb.putShort((short)countChildren());
		
		for(GraphVertex child : children.values()) {
			bb.putInt(child.offset);
		}
	}
	
	public int computeSize() {
		return 4 + (4*countChildren());
	}
}