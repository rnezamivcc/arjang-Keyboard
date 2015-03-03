package com.wordlogic.acbuilder;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.util.ArrayList;
import java.util.HashMap;

import javax.swing.ProgressMonitor;

public class DictBuilder {
	public final static int PATHOUTFILE =  1;
	public final static int PATHWORDLIST = 2;
	public final static int PATHACCENTS = 3;
	
	private String outfilePath = null;
	private String wordlistPath = null;
	private String accentPath = null;
	private boolean fetchdb = false;
	private ProgressMonitor prog;
	
	public void setPath(int pathId,String path) {
		switch(pathId) {
		case PATHOUTFILE:
			outfilePath = path;
			break;
		case PATHWORDLIST:
			wordlistPath = path;
			break;
		case PATHACCENTS:
			accentPath = path;
			break;
		}
	}
	
	public void setFetchDB(boolean db) {
		fetchdb = db;
	}
	
	public void setProgressMonitor(ProgressMonitor pm) {
		prog = pm;
	}
	
	private void setProgress(int p) {
		if(prog != null) {
			prog.setProgress(p);
		}
	}
	
	public void go() {
		if(outfilePath == null) {
			throw new IllegalStateException();
		}
		
		HashMap<String,Word> words = null;
		WordGraph graph = new WordGraph();
		PreparedStatement pst;
		ResultSet rs;
		MySQLSession mysql;
		String w1s, w2s;
		Word w1, w2;
		ArrayList<Twogram> twograms;
		Twogram tg;
		WordMap wordmap;
		boolean ngrams;
		AccentParser aparse = null;
		
		System.out.println(">>>>>>>>>>>>>About to call WordParser");
		
		try {
		words = WordParser.buildList(new File(wordlistPath));
		} catch(FileNotFoundException e) {
			throw new IllegalStateException(e);
		}
		
		if(words == null) {
			throw new IllegalStateException("word list not loaded.");
		}
		
		setProgress(5);
		
		for(Word word : words.values()) {
			graph.addWord(word.chars);
		}
		
		System.out.println("longest word is "+Word.longestWord+", "+Integer.toString(Word.longest)+" characters");
		
		setProgress(15);
		
		System.out.println("graph has "+graph.countVertices()+" vertices");
		
		System.out.println("compressing...");
		
		graph.compress();
		
		setProgress(30);
		
		System.out.println("graph now has "+graph.countVertices()+" vertices");
		
		twograms = new ArrayList<Twogram>(80000);
		
		if(fetchdb) {
		try {
			mysql = new MySQLSession();
			
			pst = mysql.con.prepareStatement("SELECT * FROM twograms WHERE compfreq >= 0");
			
			pst.executeQuery();
			
			rs = pst.getResultSet();
			
			while(rs.next()) {
				w1s = mysql.getWord(rs.getInt("w1"));
				w2s = mysql.getWord(rs.getInt("w2"));
				
				if(w1s == null || w2s == null) {
					continue;
				}
				
				w1 = words.get(w1s.toLowerCase());
				w2 = words.get(w2s.toLowerCase());
				
				if(w1 != null && w2 != null) {
					tg = new Twogram(w1,w2,rs.getInt("compfreq"));
					twograms.add(tg);
					w1.twograms.add(tg);
					w2.twograms.add(tg);
				}				
			}
		} catch (Exception e) {
			throw new IllegalStateException(e);
		}
		
		ngrams = true;
		} else {
			ngrams = false;
		}
		
		if(accentPath != null) {
			try {
			aparse = new AccentParser(new File(accentPath));
			} catch(FileNotFoundException e) {
				throw new IllegalStateException(e);
			}
		}
		
		System.out.println("loaded "+Integer.toString(twograms.size())+" twograms from database.");
		
		File outfile = new File(outfilePath);
		
		System.out.println("building word map");
		
		wordmap = new WordMap(words);
		
		setProgress(70);
		
		FileBuilder fbuilder = new FileBuilder();
		
		System.out.println("writing graph");
		
		fbuilder.add(graph.writebin()); // offset 0
		
		System.out.println("writing word hashes");
		
		setProgress(80);
		
		fbuilder.add(wordmap.writeWordHashes()); // offset 1
		
		if(ngrams) {

			System.out.println("building 2-gram list");

			wordmap.buildTwogramList();

			System.out.println("writing 2-gram header");

			fbuilder.add(wordmap.writeTwoGramHeader());

			System.out.println("writing 2-grams");

			fbuilder.add(wordmap.writeTwograms());
			fbuilder.attributes = 1;
		} else {
			fbuilder.attributes = 0;
		}
		
		fbuilder.add(wordmap.writeProbs());
		
		if(aparse != null) {
			fbuilder.attributes += 2;
			
			fbuilder.add(aparse.writeIndex());
			
			fbuilder.add(aparse.writeValues());
		}
		
		setProgress(90);
		
		System.out.println("writing probabilities");
		
		
		
		try {
			fbuilder.write(outfile);
		} catch (IOException e) {
			throw new IllegalStateException(e);
		}
		
		setProgress(100);
	}
}