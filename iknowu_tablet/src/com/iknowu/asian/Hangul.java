package com.iknowu.asian;

public class Hangul {
	
	public char initial = 0;
	public char medial = 0;
	public char finalc = 0;
	public char extraFinal = 0;

	public String toString() {
		return "(" + initial + "," + medial + "," + finalc + ")";
	}

	/**
	 * Disassemble each consonant and vowel.
	 * 
	 * @return
	 */
	private String get() {
		String ret = (initial == 0 ? "" : initial + "");
		ret += (medial == 0 ? "" : medial + "");

		switch (finalc) {
		case 0:
			ret += "";
			break;
		case 'ㄳ':
			ret += "ㄱㅅ";
			break;
		case 'ㄵ':
			ret += "ㄴㅋ";
			break;
		case 'ㄶ':
			ret += "ㄴㅎ";
			break;
		case 'ㄺ':
			ret += "ㄹㄱ";
			break;
		case 'ㄻ':
			ret += "ㄹㅁ";
			break;
		case 'ㄼ':
			ret += "ㄹㅂ";
			break;
		case 'ㄽ':
			ret += "ㄹㅅ";
			break;
		case 'ㄾ':
			ret += "ㄹㅌ";
			break;
		case 'ㄿ':
			ret += "ㄹㅍ";
			break;
		case 'ㅀ':
			ret += "ㄹㅎ";
			break;
		case 'ㅄ':
			ret += "ㅂㅅ";
			break;
		default:
			ret += finalc;
			break;
		}
		return ret;
	}

	public boolean hasInitial() {
		return initial != 0;
	}

	public boolean hasMedial() {
		return medial != 0;
	}

	public boolean hasFinal() {
		return finalc != 0;
	}
	
	public boolean hasExtraFinal() {
		return extraFinal != 0;
	}
	
	private static final char getInitial(int idx) {
		char ret = 0;
		switch (idx) {
		case 0:
			ret = 'ㄱ';
			break;
		case 1:
			ret = 'ㄲ';
			break;
		case 2:
			ret = 'ㄴ';
			break;
		case 3:
			ret = 'ㄷ';
			break;
		case 4:
			ret = 'ㄸ';
			break;
		case 5:
			ret = 'ㄹ';
			break;
		case 6:
			ret = 'ㅁ';
			break;
		case 7:
			ret = 'ㅂ';
			break;
		case 8:
			ret = 'ㅃ';
			break;
		case 9:
			ret = 'ㅅ';
			break;
		case 10:
			ret = 'ㅆ';
			break;
		case 11:
			ret = 'ㅇ';
			break;
		case 12:
			ret = 'ㅈ';
			break;
		case 13:
			ret = 'ㅉ';
			break;
		case 14:
			ret = 'ㅊ';
			break;
		case 15:
			ret = 'ㅋ';
			break;
		case 16:
			ret = 'ㅌ';
			break;
		case 17:
			ret = 'ㅍ';
			break;
		case 18:
			ret = 'ㅎ';
			break;
		}
		return ret;
	}

	/**
	 * Returns the index for the consonant.
	 * 
	 * @param ch
	 * @return
	 */
	private static final int getInitialIdx(char ch) {
		int ret = -1;
		switch (ch) {
		case 'ㄱ':
			ret = 0;
			break;
		case 'ㄲ':
			ret = 1;
			break;
		case 'ㄴ':
			ret = 2;
			break;
		case 'ㄷ':
			ret = 3;
			break;
		case 'ㄸ':
			ret = 4;
			break;
		case 'ㄹ':
			ret = 5;
			break;
		case 'ㅁ':
			ret = 6;
			break;
		case 'ㅂ':
			ret = 7;
			break;
		case 'ㅃ':
			ret = 8;
			break;
		case 'ㅅ':
			ret = 9;
			break;
		case 'ㅆ':
			ret = 10;
			break;
		case 'ㅇ':
			ret = 11;
			break;
		case 'ㅈ':
			ret = 12;
			break;
		case 'ㅉ':
			ret = 13;
			break;
		case 'ㅊ':
			ret = 14;
			break;
		case 'ㅋ':
			ret = 15;
			break;
		case 'ㅌ':
			ret = 16;
			break;
		case 'ㅍ':
			ret = 17;
			break;
		case 'ㅎ':
			ret = 18;
			break;
		}
		return ret;
	}

	private static final char getMedial(int idx) {
		char ret = 0;
		switch (idx) {
		case 0:
			ret = 'ㅏ';
			break;
		case 1:
			ret = 'ㅐ';
			break;
		case 2:
			ret = 'ㅑ';
			break;
		case 3:
			ret = 'ㅒ';
			break;
		case 4:
			ret = 'ㅓ';
			break;
		case 5:
			ret = 'ㅔ';
			break;
		case 6:
			ret = 'ㅕ';
			break;
		case 7:
			ret = 'ㅖ';
			break;
		case 8:
			ret = 'ㅗ';
			break;
		case 9:
			ret = 'ㅘ';
			break;
		case 10:
			ret = 'ㅙ';
			break;
		case 11:
			ret = 'ㅚ';
			break;
		case 12:
			ret = 'ㅛ';
			break;
		case 13:
			ret = 'ㅜ';
			break;
		case 14:
			ret = 'ㅝ';
			break;
		case 15:
			ret = 'ㅞ';
			break;
		case 16:
			ret = 'ㅟ';
			break;
		case 17:
			ret = 'ㅠ';
			break;
		case 18:
			ret = 'ㅡ';
			break;
		case 19:
			ret = 'ㅢ';
			break;
		case 20:
			ret = 'ㅣ';
			break;
		}
		return ret;
	}

	/**
	 * Returns the index for the medial.
	 * 
	 * @param ch
	 * @return
	 */
	private static final int getMedialIdx(char ch) {
		int ret = -1;
		switch (ch) {
		case 'ㅏ':
			ret = 0;
			break;
		case 'ㅐ':
			ret = 1;
			break;
		case 'ㅑ':
			ret = 2;
			break;
		case 'ㅒ':
			ret = 3;
			break;
		case 'ㅓ':
			ret = 4;
			break;
		case 'ㅔ':
			ret = 5;
			break;
		case 'ㅕ':
			ret = 6;
			break;
		case 'ㅖ':
			ret = 7;
			break;
		case 'ㅗ':
			ret = 8;
			break;
		case 'ㅘ':
			ret = 9;
			break;
		case 'ㅙ':
			ret = 10;
			break;
		case 'ㅚ':
			ret = 11;
			break;
		case 'ㅛ':
			ret = 12;
			break;
		case 'ㅜ':
			ret = 13;
			break;
		case 'ㅝ':
			ret = 14;
			break;
		case 'ㅞ':
			ret = 15;
			break;
		case 'ㅟ':
			ret = 16;
			break;
		case 'ㅠ':
			ret = 17;
			break;
		case 'ㅡ':
			ret = 18;
			break;
		case 'ㅢ':
			ret = 19;
			break;
		case 'ㅣ':
			ret = 20;
			break;
		}

		return ret;
	}

	private static final char getFinal(int idx) {
		char ret = 0;
		switch (idx) {
		case 0:
			ret = 0;
			break;
		case 1:
			ret = 'ㄱ';
			break;
		case 2:
			ret = 'ㄲ';
			break;
		case 3:
			ret = 'ㄳ';
			break;
		case 4:
			ret = 'ㄴ';
			break;
		case 5:
			ret = 'ㄵ';
			break;
		case 6:
			ret = 'ㄶ';
			break;
		case 7:
			ret = 'ㄷ';
			break;
		case 8:
			ret = 'ㄹ';
			break;
		case 9:
			ret = 'ㄺ';
			break;
		case 10:
			ret = 'ㄻ';
			break;
		case 11:
			ret = 'ㄼ';
			break;
		case 12:
			ret = 'ㄽ';
			break;
		case 13:
			ret = 'ㄾ';
			break;
		case 14:
			ret = 'ㄿ';
			break;
		case 15:
			ret = 'ㅀ';
			break;
		case 16:
			ret = 'ㅁ';
			break;
		case 17:
			ret = 'ㅂ';
			break;
		case 18:
			ret = 'ㅄ';
			break;
		case 19:
			ret = 'ㅅ';
			break;
		case 20:
			ret = 'ㅆ';
			break;
		case 21:
			ret = 'ㅇ';
			break;
		case 22:
			ret = 'ㅈ';
			break;
		case 23:
			ret = 'ㅊ';
			break;
		case 24:
			ret = 'ㅋ';
			break;
		case 25:
			ret = 'ㅌ';
			break;
		case 26:
			ret = 'ㅍ';
			break;
		case 27:
			ret = 'ㅎ';
			break;
		}
		return ret;
	}

	/**
	 * Returns the index for the final.
	 * 
	 * @param ch
	 * @return
	 */
	private static final int getFinalIdx(char ch) {
		int ret = -1;
		switch (ch) {
		case 0:
			ret = 0;
			break;
		case ' ':
			ret = 0;
			break;
		case 'ㄱ':
			ret = 1;
			break;
		case 'ㄲ':
			ret = 2;
			break;
		case 'ㄳ':
			ret = 3;
			break;
		case 'ㄴ':
			ret = 4;
			break;
		case 'ㄵ':
			ret = 5;
			break;
		case 'ㄶ':
			ret = 6;
			break;
		case 'ㄷ':
			ret = 7;
			break;
		case 'ㄹ':
			ret = 8;
			break;
		case 'ㄺ':
			ret = 9;
			break;
		case 'ㄻ':
			ret = 10;
			break;
		case 'ㄼ':
			ret = 11;
			break;
		case 'ㄽ':
			ret = 12;
			break;
		case 'ㄾ':
			ret = 13;
			break;
		case 'ㄿ':
			ret = 14;
			break;
		case 'ㅀ':
			ret = 15;
			break;
		case 'ㅁ':
			ret = 16;
			break;
		case 'ㅂ':
			ret = 17;
			break;
		case 'ㅄ':
			ret = 18;
			break;
		case 'ㅅ':
			ret = 19;
			break;
		case 'ㅆ':
			ret = 20;
			break;
		case 'ㅇ':
			ret = 21;
			break;
		case 'ㅈ':
			ret = 22;
			break;
		case 'ㅊ':
			ret = 23;
			break;
		case 'ㅋ':
			ret = 24;
			break;
		case 'ㅌ':
			ret = 25;
			break;
		case 'ㅍ':
			ret = 26;
			break;
		case 'ㅎ':
			ret = 27;
			break;
		}
		return ret;
	}
	
	/**
	 * Try to combine two characters into one, to be used as a final character
	 * 
	 * @param finalc the current final character
	 * @param newChar the character that was typed
	 * @return
	 */
	public static char getCombined(char finalc, char newChar) {
		char ret = 0x0;
		
		if ( finalc == 'ㄱ' && newChar == 'ㅅ' ) {
			ret = 'ㄳ';
		} else if ( finalc == 'ㄱ' && newChar == 'ㄱ' ) {
			ret = 'ㄲ';
		} else if ( finalc == 'ㄴ' && newChar == 'ㅈ' ) {
			ret = 'ㄵ';
		} else if ( finalc == 'ㄴ' && newChar == 'ㅎ' ) {
			ret = 'ㄶ';
		} else if ( finalc == 'ㄹ' && newChar == 'ㄱ' ) {
			ret = 'ㄺ';
		} else if ( finalc == 'ㄹ' && newChar == 'ㅁ' ) {
			ret = 'ㄻ';
		} else if ( finalc == 'ㄹ' && newChar == 'ㅂ' ) {
			ret = 'ㄼ';
		} else if ( finalc == 'ㄹ' && newChar == 'ㅅ' ) {
			ret = 'ㄽ';
		} else if ( finalc == 'ㄹ' && newChar == 'ㅌ' ) {
			ret = 'ㄾ';
		} else if ( finalc == 'ㄹ' && newChar == 'ㅍ' ) {
			ret = 'ㄿ';
		} else if ( finalc == 'ㄹ' && newChar == 'ㅎ' ) {
			ret = 'ㅀ';
		} else if ( finalc == 'ㅅ' && newChar == 'ㅅ' ) {
			ret = 'ㅆ';
		} else if ( finalc == 'ㅂ' && newChar == 'ㅅ' ) {
			ret = 'ㅄ';
		} else {
			ret = 0x0;
		}
		
		return ret;
	}

    /**
     * Try to combine two characters into one, to be used as a medial character
     *
     * @param medial the current final character
     * @param newChar the character that was typed
     * @return
     */
    public static char getCombinedMedial(char medial, char newChar) {
        char ret = 0;

        if ( medial == 'ㅗ' && newChar == 'ㅏ' ) {
            ret = 'ㅘ';
        } else if ( medial == 'ㅗ' && newChar == 'ㅐ' ) {
            ret = 'ㅙ';
        } else if ( medial == 'ㅗ' && newChar == 'ㅣ' ) {
            ret = 'ㅚ';
        } else if ( medial == 'ㅜ' && newChar == 'ㅓ' ) {
            ret = 'ㅝ';
        } else if ( medial == 'ㅜ' && newChar == 'ㅔ' ) {
            ret = 'ㅞ';
        } else if ( medial == 'ㅜ' && newChar == 'ㅣ' ) {
            ret = 'ㅟ';
        } else if ( medial == 'ㅡ' && newChar == 'ㅣ' ) {
            ret = 'ㅢ';
        }

        return ret;
    }
	
	/**
	 * Try to split a specified last character.
	 * 
	 * @param last the character to be split
	 * @return
	 */
	public static char[] splitLastChar(char last) {
		char[] ret = new char[2];
		
		ret[0] = 0;
		ret[1] = 0;
		
		switch (last) {
		case 'ㄳ':
			ret[0] = 'ㄱ';
			ret[1] = 'ㅅ';
			break;
		case 'ㄲ':
			ret[0] = 'ㄱ';
			ret[1] = 'ㄱ';
			break;
		case 'ㄵ':
			ret[0] = 'ㄴ';
			ret[1] = 'ㅈ';
			break;
		case 'ㄶ':
			ret[0] = 'ㄴ';
			ret[1] = 'ㅎ';
			break;
		case 'ㄺ':
			ret[0] = 'ㄹ';
			ret[1] = 'ㄱ';
			break;
		case 'ㄻ':
			ret[0] = 'ㄹ';
			ret[1] = 'ㅁ';
			break;
		case 'ㄼ':
			ret[0] = 'ㄹ';
			ret[1] = 'ㅂ';
			break;
		case 'ㄽ':
			ret[0] = 'ㄹ';
			ret[1] = 'ㅅ';
			break;
		case 'ㄾ':
			ret[0] = 'ㄹ';
			ret[1] = 'ㅌ';
			break;
		case 'ㄿ':
			ret[0] = 'ㄹ';
			ret[1] = 'ㅍ';
			break;
		case 'ㅀ':
			ret[0] = 'ㄹ';
			ret[1] = 'ㅎ';
			break;
		case 'ㅆ':
			ret[0] = 'ㅅ';
			ret[1] = 'ㅅ';
			break;
		case 'ㅄ':
			ret[0] = 'ㅂ';
			ret[1] = 'ㅅ';
			break;
		}
		
		return ret;
	}

    /**
     * Try to split a specified medial character.
     *
     * @param medial the character to be split
     * @return
     */
    public static char[] splitMedialChar(char medial) {
        char[] ret = new char[2];

        ret[0] = 0;
        ret[1] = 0;

        switch (medial) {
            case 'ㅘ':
                ret[0] = 'ㅗ';
                ret[1] = 'ㅏ';
                break;
            case 'ㅙ':
                ret[0] = 'ㅗ';
                ret[1] = 'ㅐ';
                break;
            case 'ㅚ':
                ret[0] = 'ㅗ';
                ret[1] = 'ㅣ';
                break;
            case 'ㅝ':
                ret[0] = 'ㅜ';
                ret[1] = 'ㅓ';
                break;
            case 'ㅞ':
                ret[0] = 'ㅜ';
                ret[1] = 'ㅔ';
                break;
            case 'ㅟ':
                ret[0] = 'ㅜ';
                ret[1] = 'ㅣ';
                break;
            case 'ㅢ':
                ret[0] = 'ㅡ';
                ret[1] = 'ㅣ';
                break;
        }

        return ret;
    }
	
	/**
	 * Check whether the given character can be an initial character
	 * 
	 * @param ch
	 * @return
	 */
	public static boolean isInitial(char ch) {
		if (getInitialIdx(ch) != -1) return true;
		return false;
	}
	
	/**
	 * Check whether the given character can be a medial character
	 * 
	 * @param ch
	 * @return
	 */
	public static boolean isMedial(char ch) {
		if (getMedialIdx(ch) != -1) return true;
		return false;
	}
	
	/**
	 * Check whether the given character can be a final character
	 * 
	 * @param ch
	 * @return
	 */
	public static boolean isFinal(char ch) {
		if (getFinalIdx(ch) != -1) return true;
		return false;
	}
	
	/**
	 * Copy the given Hangul instance into a new one and return the new one
	 * 
	 * @param toCopy
	 * @return
	 */
	public static Hangul copy(Hangul toCopy) {
		Hangul newHangul = new Hangul();
		
		newHangul.initial = toCopy.initial;
		newHangul.medial = toCopy.medial;
		newHangul.finalc = toCopy.finalc;
		
		return newHangul;
	}
	
	public static String combineChars(String chars, boolean ignoreExtras) {
		
		StringBuilder ret = new StringBuilder();
		
		Hangul current = new Hangul();
		Hangul previous = new Hangul();
		
		//IKnowUKeyboardService.log(Log.INFO, "Combining chars", "= "+chars);
		
		for (int i=0; i < chars.length(); i++) {
			//figure out what the character that should be added to the text should be
			char cur = chars.charAt(i);
			if (cur != '.' && cur != ' ') {
				//IKnowUKeyboardService.log(Log.VERBOSE, "current", "= "+cur);
				String charToPrint = "";
				int deleteChars = 0;
				Boolean shouldSkip = false;
				if ( current.hasInitial() && current.hasMedial() && current.hasFinal() ) {
					char combined = Hangul.getCombined(current.finalc, cur);
					if (combined != 0) {
						current.finalc = combined;
						charToPrint =  ""+Hangul.combine(current.initial, current.medial, current.finalc);
						deleteChars = 1;
						shouldSkip = true;
					} else {
						previous = Hangul.copy(current);
						current = new Hangul();
					}
				}
				if (!shouldSkip) {
					//if there is an intial character and a medial character, then check to see if this character can be a final character, and not a medial character
					//and then combine it with the current characters
					if ( current.hasInitial() && current.hasMedial() && Hangul.isFinal(cur) && !Hangul.isMedial(cur) ) {
						current.finalc = cur;
						charToPrint =  ""+Hangul.combine(current.initial, current.medial, current.finalc);
						deleteChars = 1;
						//this.currentHangul = new Hangul();
					//if there is an initial character present, then check to see if this character is not an initial character
					//and combine it with the current inital one
					} else if ( current.hasInitial() && !Hangul.isInitial(cur) ) {
						current.medial = cur;
						charToPrint =  ""+Hangul.combine(current.initial, current.medial, (char) 0);
						deleteChars = 1;
					} else {
						current = new Hangul();
						//if there is a previous charcter, and the character has a final character, and the final charcter can
						//be used as an initial character, then split those characters and use the final as the initial for the new  medial character that was typed
						if (previous != null && previous.hasFinal() && Hangul.isMedial(cur) ) {
							final char[] split = Hangul.splitLastChar(previous.finalc);
							if (split[0] != 0) {
								deleteChars = 1;
								previous.finalc = split[0];
								current.initial = split[1];
								current.medial = cur;
								charToPrint = ""+Hangul.combine(previous.initial, previous.medial, previous.finalc);
								charToPrint += Hangul.combine(current.initial, current.medial, (char) 0);
							} else if ( Hangul.isInitial(previous.finalc) ) {
								deleteChars = 1;
								current.initial = previous.finalc;
								current.medial = cur;
								charToPrint = ""+Hangul.combine(previous.initial, previous.medial, (char) 0);
								charToPrint += Hangul.combine(current.initial, current.medial, (char) 0);
							}
						}
						else if ( Hangul.isInitial( cur ) ) {
							current.initial = cur;
							charToPrint = ""+cur;
						} else {
							charToPrint = ""+cur;
						}
					}
				}
				
				if (deleteChars > 0) {
					ret.delete(ret.length() - deleteChars, ret.length());
					ret.append(charToPrint);
				} else {
					ret.append(charToPrint);
				}
			} else if (cur == '.' && !ignoreExtras) {
				ret.append('.');
			}
		}
		
		return ret.toString();
	}
	
	public static String decompose(String combined) {
		
		StringBuilder sb = new StringBuilder();
		Hangul current = new Hangul();
		
		for (int i=0; i < combined.length(); i ++) {
			
			char cur = combined.charAt(i);
			current = split(cur);
			
			if (current.hasInitial()) sb.append(current.initial);
			if (current.hasMedial()) sb.append(current.medial);
			if (current.hasFinal()) sb.append(current.finalc);
		}
		
		return sb.toString();
	}

	/**
	 * 
	 * @param ch
	 * @return
	 */
	public static Hangul split(char ch) {
		Hangul hangul = new Hangul();
		int x = (ch & 0xFFFF), y = 0, z = 0;
		if (x >= 0xAC00 && x <= 0xD7A3) {
			y = x - 0xAC00;
			z = y % (21 * 28);
			hangul.initial = getInitial(y / (21 * 28));
			hangul.medial = getMedial(z / 28);
			hangul.finalc = getFinal(z % 28);
		} else if (x >= 0x3131 && x <= 0x3163) {
			if (getInitialIdx(ch) > -1) {
				hangul.initial = ch;
			} else if (getMedialIdx(ch) > -1) {
				hangul.medial = ch;
			} else if (getFinalIdx(ch) > -1) {
				hangul.finalc = ch;
			}
		} else {
			hangul.initial = ch;
		}
		return hangul;
	}

	/**
	 * 
	 * @param string
	 * @return
	 */
	public static String split(String string) {
		if (string == null)
			return null;

		StringBuffer sb = new StringBuffer();
		for (int i = 0, stop = string.length(); i < stop; i++) {
			sb.append(split(string.charAt(i)));
		}
		return sb.toString();
	}

	/**
	 * Reads one character to the neutral coda consonants are added together.
	 * 
	 * @param initial
	 *            Consonant
	 * @param medial
	 *            Neutrality
	 * @param finalc
	 *            Granulomatous
	 * @return
	 */
	public static char combine(char initial, char medial, char finalc) {
		return (char) (getInitialIdx(initial) * 21 * 28 + getMedialIdx(medial) * 28
				+ getFinalIdx(finalc) + 0xAC00);
	}

	/**
	 * Attachment to say words that started with consonants by attaching to the front of the coda gives two strings together.
	 * Develop a + b => made
	 * Develop a + d'm =>'ll make
	 * 
	 * @param head
	 * @param tail
	 * @return
	 */
	public static String append(String head, String tail) {
		String ret = null;

		Hangul headTail = split(head.charAt(head.length() - 1));
		Hangul tailHead = split(tail.charAt(0));

		if (tailHead.hasMedial() || headTail.hasFinal()) {
			ret = head + tail;
		} else {
			String headHead = head.substring(0, head.length() - 1);
			String tailTail = tail.substring(1);
			ret = headHead + combine(headTail.initial, headTail.medial, tailHead.initial)
					+ tailTail;
		}
		return ret;
	}

	/**
	 * Check that you have final character.
	 * 
	 * @param ch
	 * @return
	 */
	public static boolean hasFinal(char ch) {
		return split(ch).hasFinal();
	}

	/**
	 * Check that you have final character.
	 * 
	 * @param string
	 * @return
	 */
	public static boolean hasFinal(String string) {
		if (!Util.valid(string))
			return false;
		return hasFinal(string.charAt(string.length() - 1));
	}

	/**
	 * Hangul characters by each decomposing and decomposed paste consonant letters give a collection of independent units, 
	 * each letter: return makes breaking into.
	 * 
	 * @param string
	 * @return
	 */
	private static String split2(String string) {
		if (string == null)
			return null;
		String ret = "";
		for (int i = 0, stop = string.length(); i < stop; i++) {
			ret += split(string.charAt(i)).get() + ":";
		}
		return ret;
	}

	/**
	 * Make sure the String ends with a pattern, which
	 * B d f Wh consonant or vowel, such as including the check, including the check box that
	 * 
	 * @param string
	 * @param pattern
	 * @return
	 */
	public static boolean endsWith(String string, String pattern) {
		if (!Util.valid(string) || !Util.valid(pattern))
			return false;
		int slen = string.length(), plen = pattern.length();
		if (slen < plen)
			return false;
		char sch = 0, pch = 0;
		for (int i = 0; i < plen; i++) {
			sch = string.charAt(slen - i - 1);
			pch = pattern.charAt(plen - i - 1);
			if (pch != sch) {
				if (i == plen - 1)
					return endsWith2(sch, pch);
				return false;
			}
		}
		return true;
	}

	/**
	 * Ensure that the ending to char
	 * 
	 * @param sch
	 * @param pch
	 * @return
	 */
	public static boolean endsWith(char sch, char pch) {
		if (sch == pch)
			return true;
		return endsWith2(sch, pch);
	}

	/**
	 * endsWith(char sch, char pch) The same purpose, but it does not ensure that the strings are equal.
	 * 
	 * @param sch
	 * @param pch
	 * @return
	 */
	private static boolean endsWith2(char sch, char pch) {
		String stemp = split(sch).get(), ptemp = split(pch).get();
		return stemp.endsWith(ptemp);
	}

	/**
	 * Make sure the end of the given pattern and ending with the given pattern
	 * String eliminates part of the pattern.
	 * 'Is the' on 'is f' if you remove the 'this' is returned.
	 * 
	 * @param string
	 * @param pattern
	 * @return
	 */
	public static String removeEnd(String string, String pattern) {
		// validity check
		if (!Util.valid(string) || !Util.valid(pattern))
			return string;
		int slen = string.length(), plen = pattern.length();
		if (slen < plen)
			return string;
		if (string.endsWith(pattern))
			return string.substring(0, slen - plen);

		// All the rest is other than the first letter should match.
		if (!pattern.substring(1).equals(string.substring(slen - plen + 1)))
			return string;

		// Must not have a single shred, consonant, vowel + consonant haejum
		// when handling
		String stemp = split(string.charAt(slen - plen)).get();
		String ptemp = split(pattern.charAt(0)).get();
		if (!stemp.endsWith(ptemp))
			return string;
		String temp = stemp.substring(0, stemp.length() - ptemp.length());
		char[] ch = { 0, 0, 0 };
		for (int i = 0, stop = temp.length(); i < stop; i++) {
			ch[i] = temp.charAt(i);
		}
		String ret = slen > plen ? string.substring(0, slen - plen) : "";
		char rch = combine(ch[0], ch[1], ch[2]);
		if (rch == 0)
			return ret;
		return ret += combine(ch[0], ch[1], ch[2]);
	}

	/**
	 * len With a length corresponding to the mother returns.
	 * The mother 'is f' as the last consonant makes a return as.
	 * 
	 * @param string
	 * @param len
	 * @return
	 */
	public static String extractExtraEomi(final String string, int len) {
		int strlen = string.length();
		if (!Util.valid(string) || strlen < len)
			return null;
		Hangul hg = split(string.charAt(strlen - len));
		if (!hg.hasFinal())
			return null;
		String temp = hg.get();
		return temp.charAt(temp.length() - 1)
				+ string.substring(strlen - len + 1);
	}
}
