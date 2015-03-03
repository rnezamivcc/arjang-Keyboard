package com.iknowu.autocorrect;

import java.util.ArrayList;

import android.util.Log;

public class ResultSet {
	private ArrayList<Prediction> ar;
	
	public ResultSet() {
		ar = new ArrayList<Prediction>();
	}
	
	public void add(String w,int l,int p,int fletter,float ngram) {
		ar.add(new Prediction(w,p,l,0,ngram,(fletter == 1)));
	}
	
	public int size() {
		return ar.size();
	}
	
	public Prediction get(int i) {
		return ar.get(i);
	}
}