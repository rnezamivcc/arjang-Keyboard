package com.wordlogic.acbuilder;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;

public class WordGraph {
	private HashMap<Character,ArrayList<GraphVertex>> vertices;
	private GraphVertex v0;
	
	public WordGraph() {
		vertices = new HashMap<Character,ArrayList<GraphVertex>>();
		v0 = new GraphVertex((char)0);
	}
	
	public void addWord(String word) {
		int c;
		char ch;
		GraphVertex v = v0;
		GraphVertex vn;
		
		for(c = 0;c < word.length();c++) {
			ch = word.charAt(c);
			
			vn = v.getChildByChar(ch);
			
			if(vn == null) {
				vn = v.addChild(ch);
				
				if(vertices.get(ch) == null) {
					vertices.put(ch, new ArrayList<GraphVertex>());
				}
				
				vertices.get(ch).add(vn);
			}
			
			v = vn;
		}
	}
	
	public int countVertices() {
		int n = 0;
		
		for(ArrayList<GraphVertex> al : vertices.values()) {
			n += al.size();
		}
		
		return n;
	}
	
	public void compress() {
		int c, cc;
		
		for(ArrayList<GraphVertex> al : vertices.values()) {
			for(c = 0;c < al.size();c++) {
				for(cc = c+1;cc < al.size();cc++) {
					if(al.get(c).equals(al.get(cc))) {
						al.get(cc).replaceWith(al.get(c));
						al.remove(cc);
						cc--;
					}
				}
			}
		}
	}
	
	public byte[] writebin() {
		ArrayList<GraphVertex> verts = new ArrayList<GraphVertex>();
		int offset;
		int c;
		byte[] ret;
		ByteBuffer buf;
		
		for(ArrayList<GraphVertex> al : vertices.values()) {
			verts.addAll(al);
		}
		
		v0.offset = 0;
		offset = v0.computeSize();
		
		for(c = 0;c < verts.size();c++) {
			verts.get(c).offset = offset;
			offset += verts.get(c).computeSize();
		}
		
		ret = new byte[offset];
		
		buf  = ByteBuffer.wrap(ret);
		
		buf.order(FileBuilder.byteOrder);
		
		v0.writebin(buf);
		
		for(c = 0;c < verts.size();c++) {
			verts.get(c).writebin(buf);
		}
		
		return ret;
	}
}