package com.iknowu.util;

import java.util.ArrayList;
import java.util.Iterator;

/**
 * Created by Justin on 02/10/13.
 *
 */
public class Word {

    private String word;
    private ArrayList<PartOfSpeech> partsOfSpeech;


    public  Word(String w) {
        this.word = w;
        this.partsOfSpeech = new ArrayList<PartOfSpeech>();
    }

    public String getWord() {
        return this.word;
    }

    public int getWordLength() {
        return this.word.length();
    }

    public void addChars(String chars) {
        this.word += chars;
    }

    public void deleteChars(int length) {
        if (length < this.word.length()) {
            this.word = this.word.substring(0, this.word.length() - length);
        } else {
            this.word = "";
        }
    }

    public void addPartOfSpeech(PartOfSpeech pos) {
        PartOfSpeech myPos = this.checkForPartOfSpeech(pos);

        if (myPos != null) {
            myPos.increaseFrequency();
        } else {
            pos.increaseFrequency();
            this.partsOfSpeech.add(pos);
        }
    }

    private PartOfSpeech checkForPartOfSpeech(PartOfSpeech thePos) {

        Iterator<PartOfSpeech> it = this.partsOfSpeech.iterator();

        PartOfSpeech myPos = null;
        Boolean found = false;

        while(it.hasNext()) {
            myPos = it.next();

            if (myPos.getTag().equals(thePos.getTag())) {
                found = true;
                break;
            }
        }

        if (found) return myPos;
        else return null;
    }
}
