package com.iknowu.animation;

import java.util.ArrayList;

/**
 * Created by Justin on 18/07/13.
 *
 */
public class Animation {

    private ArrayList<AnimationStep> steps;
    private boolean looping;
    private int currentStep;

    public Animation(boolean shouldLoop) {
        this.looping = shouldLoop;
        this.steps = new ArrayList<AnimationStep>();
    }

    public void setSteps(ArrayList<AnimationStep> psteps) {
        this.steps = psteps;
    }

    public void setLooping(boolean shouldLoop) {
        this.looping = shouldLoop;
    }

    public void setCurrentStep(int step) {
        this.currentStep = step;
    }

    public void incrementStep() {
        this.currentStep++;
    }

    public ArrayList<AnimationStep> getSteps() {
        return this.steps;
    }

    public boolean isLooping() {
        return this.looping;
    }

    public int getCurrentStepIndex() {
        return this.currentStep;
    }

    public AnimationStep getCurrentStep() {
        return this.steps.get(this.currentStep);
    }

    public void addStep(AnimationStep s) {
        this.steps.add(s);
    }

    public void removeStep(int index) {
        this.steps.remove(index);
    }

    public void reset() {
        this.currentStep = 0;
        for (int i=0; i < this.steps.size(); i++) {
            AnimationStep step = this.steps.get(i);
            step.currentFrame = 0;
        }
    }
}
