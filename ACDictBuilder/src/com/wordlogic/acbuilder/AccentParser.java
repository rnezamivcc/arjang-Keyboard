package com.wordlogic.acbuilder;

import java.io.File;
import java.io.FileNotFoundException;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.Scanner;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import com.wordlogic.acbuilder.AccentParser.Substitution.SubstitutionComparator;

public class AccentParser {
	private ArrayList<Substitution> substitutions;
	
	public AccentParser(File accentsFile) throws FileNotFoundException {
		Scanner scanner;
		String line;
		Pattern pattern = Pattern.compile("^\\s*ACCENT\\((\\d+),(\\d+)\\).*");
		Matcher m;
		int n = 0;
		int c;
		
		scanner = new Scanner(accentsFile,"UTF-16");
		
		substitutions = new ArrayList<Substitution>();
		
		while(scanner.hasNextLine()) {
			line = scanner.nextLine();
			
			m = pattern.matcher(line);
			
			if(m.matches()) {
				try {
				substitutions.add(new Substitution(Integer.parseInt(m.group(1)),Integer.parseInt(m.group(2))));
				} catch(NumberFormatException e) {
					System.out.println("Exception on line");
					System.out.println(line);
				}
				
				n++;
			}
		}
		
		System.out.println("Loaded "+Integer.toString(n)+" accent items");
		
		scanner.close();
		
		SubstitutionComparator comparator = new SubstitutionComparator();
		
		Collections.sort(substitutions,comparator);
		
		/*for(c = 0;c < substitutions.size();c++) {
			System.out.println(Integer.toString(substitutions.get(c).ch1) + " , " + Integer.toString(substitutions.get(c).ch2));
		}*/
	}
	
	public static class Substitution {
		public final char ch1, ch2;
		
		public Substitution(int a,int b) {
			ch1 = (char)a;
			ch2 = (char)b;
		}
		
		public static class SubstitutionComparator implements Comparator<Substitution> {
			@Override
			public int compare(Substitution l, Substitution r) {
				if(l.ch1 > r.ch1) { return 1; }
				else if(l.ch1 == r.ch1) { return 0; }
				else { return -1; }
			}
		}
	}
	
	public HashMap<Character,Character> getMap() {
		int c;
		HashMap<Character,Character> ret = new HashMap<Character,Character>();
		
		for(c = 0;c < substitutions.size();c++) {
			ret.put(substitutions.get(c).ch1, substitutions.get(c).ch2);
		}
		
		return ret;
	}
	
	public byte[] writeIndex() {
		byte[] ret;
		ByteBuffer bb;
		int c;
		
		ret = new byte[substitutions.size()*2];
		
		bb = ByteBuffer.wrap(ret);
		
		bb.order(FileBuilder.byteOrder);
		bb.position(0);
		
		for(c = 0;c < substitutions.size();c++) {
			bb.putChar(substitutions.get(c).ch1);
		}
		
		return ret;
	}
	
	public byte[] writeValues() {
		byte[] ret;
		ByteBuffer bb;
		int c;
		
		ret = new byte[substitutions.size()*2];
		
		bb = ByteBuffer.wrap(ret);
		
		bb.order(FileBuilder.byteOrder);
		bb.position(0);
		
		for(c = 0;c < substitutions.size();c++) {
			bb.putChar(substitutions.get(c).ch2);
		}
		
		return ret;
	}
	
	
}