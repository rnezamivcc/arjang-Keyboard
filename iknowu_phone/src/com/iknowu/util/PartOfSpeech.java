package com.iknowu.util;

/**
 * Created by Justin on 02/10/13.
 *
 */
public class PartOfSpeech {

    private String tag;
    private int frequency;

    public PartOfSpeech(String tag) {
        this.tag = tag;
        this.frequency = 0;
    }

    public void setTag(String tag) {
        this.tag = tag;
    }

    public void setFrequency(int freq) {
        this.frequency = freq;
    }

    public void increaseFrequency() {
        this.frequency++;
    }

    public String getTag() {
        return this.tag;
    }

    public int getFrequency() {
        return this.frequency;
    }
}
