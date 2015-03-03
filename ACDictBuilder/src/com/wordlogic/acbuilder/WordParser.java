package com.wordlogic.acbuilder;

import java.io.File;
import java.io.FileNotFoundException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Scanner;

public class WordParser {
	public static HashMap<String,Word> buildList(File infile) throws FileNotFoundException {
		Scanner scanner;
		String line;
		HashMap<String,Word> ret = new HashMap<String,Word>();
		int c;
		int prob;
		String w;
		int linen = 0;
		double maxprob = 0;
		double d;
		
		System.out.println("MARK: Preparing scanner...");
		
		scanner = new Scanner(infile,"UTF-16");
		
		while(scanner.hasNextLine()) {
			
			System.out.println("MARK: Scanning...");
			
			line = scanner.nextLine();
			
			line = line.trim();
			
			System.out.println("MARK: Scanning...[" + line + "]");
			
			linen++;
			
			c = line.length()-1;
			
			while(c >= 0 && Character.isDigit(line.charAt(c))) {
				c--;
			}
			
			try {
			prob = Integer.parseInt(line.substring(c+1).trim());
			} catch(Exception e) {
				e.printStackTrace();
				
				System.out.println("On line (#" + Integer.toString(linen) + ") of:");
				
				for(c = 0;c < line.length();c++) {
					System.out.println(Integer.toString(line.charAt(c)));
				}
				
				scanner.close();
				return null;
			}
			
			w = line.substring(0, c).trim();
			
			System.out.println("Working with [" + w + "]");
			
			if(!w.contains(" ")) {
				ret.put(w.toLowerCase(),new Word(w,prob));
			}
			
			if(prob > maxprob) { maxprob = prob; }
		}
		
		scanner.close();
		
		for(Word word : ret.values()) {
			d = (double)word.prob / maxprob;
			
			d = 1 + (d * 62);
			
			word.prob = (int)d;
		}
		
		System.out.println("MARK: SCAN Complete...");
		
		return ret;
	}
	
	
}