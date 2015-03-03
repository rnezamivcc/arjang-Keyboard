package com.iknowu;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.drawable.Drawable;
import android.text.Layout;
import android.text.StaticLayout;
import android.text.TextPaint;
import android.util.Log;
import android.view.MotionEvent;

import com.iknowu.R;

/**
 * To be used as an overlay to the {@link IKnowUKeyboardView} an provide helpful tips/tricks or new features that have been added to the keyboard
 *
 *
 *
 * @author Justin
 *
 */
public class TutorialView {
	
	private static final int STEP_ONE = 1;
	private static final int STEP_TWO = 2;
	private static final int STEP_THREE = 3;
	private static final int STEP_FOUR = 4;
	private static final int STEP_FIVE = 5;
	private static final int STEP_SIX = 6;
	private static final int STEP_SEVEN = 7;
	private static final int STEP_EIGHT = 8;
	
	private static final int MAX_STEP = 1;
	
	private static final int FRAME_DURATION = 1;
	
	private Context context;
	private IKnowUKeyboardView inputView;
	
	private int currentStep;
	
	private long animStartTime;
	
	private Drawable arrow;
	
	private float textSize;
	
	public TutorialView(Context context, IKnowUKeyboardView inputView) {
		try {
			this.context = context;
			this.inputView = inputView;
			this.currentStep = STEP_ONE;
			
			this.textSize = context.getResources().getDimensionPixelSize(R.dimen.tutorial_text_size);
			this.arrow = context.getResources().getDrawable(R.drawable.swipe_down);
			
			//this.keys = new HashMap<Integer, Key>();
		} catch (Exception e) {
			IKnowUKeyboardService.sendErrorMessage(e);
		}
	}
	
	/**
	 * Set the start time for any animations that might need to happen
	 * 
	 * @param start the start time in milliseconds
	 */
	public void setStartTime(long start) {
		this.animStartTime = start;
	}
	
	/**
	 * Call this to have the tutorial draw itself on the canvas provided
	 * 
	 * @param canvas the canvas to draw on
	 * @param paint the paint object to use when drawing
	 */
	public void onDraw(Canvas canvas, Paint paint) {
		try {
			//check to see whether we are animating, if we are, then
			//perform calculations and draw everything
			double interpolation = 0;
			
			boolean shouldRedraw = false;
			
			if (animStartTime > 0) {
				
				interpolation = (double) ((System.currentTimeMillis() - animStartTime) / (double) (FRAME_DURATION));
				
				IKnowUKeyboardService.log(Log.VERBOSE, "TutorialView onDraw", "interpolation = "+interpolation);
				
				if (interpolation >= 1) {
					animStartTime = 0;
					interpolation = 1;
				}
				
				shouldRedraw = true;
			} else {
				animStartTime = 0;
				interpolation = 1;
			}
			
			switch (this.currentStep) {
				case STEP_ONE:
					this.drawStepOne(canvas, paint, interpolation);
					break;
				case STEP_TWO:
					this.drawStepTwo(canvas, paint, interpolation);
					break;
				case STEP_THREE:
					this.drawStepThree(canvas, paint, interpolation);
					break;
				case STEP_FOUR:
					this.drawStepFour(canvas, paint, interpolation);
					break;
				case STEP_FIVE:
					this.drawStepFive(canvas, paint, interpolation);
					break;
				case STEP_SIX:
					this.drawStepSix(canvas, paint, interpolation);
					break;
				case STEP_SEVEN:
					this.drawStepSeven(canvas, paint, interpolation);
					break;
				case STEP_EIGHT:
					this.drawStepEight(canvas, paint, interpolation);
					break;
			}
			
			if (shouldRedraw) {
				this.inputView.invalidate();
			}
		} catch (Exception e) {
			IKnowUKeyboardService.sendErrorMessage(e);
		}
	}
	
	/**
	 * Pass a touch event to this tutorial. Generally just a tap to register that a new tip should appear, or that it should now hide itself
	 * 
	 * @param event the {@link MotionEvent} that occurred
	 */
	public void onTouch(MotionEvent event) {
		try {
			int action = event.getAction();
			
			IKnowUKeyboardService.log(Log.VERBOSE, "TutorialView onTouch", "action = "+action);
			
			if (action == MotionEvent.ACTION_UP) {
				this.currentStep++;
				if (this.currentStep > MAX_STEP) {
					this.inputView.tutorialMode = false;
					this.inputView.invalidate();
					return;
				}
				//Key k;
				switch (this.currentStep) {
					case STEP_ONE:
						
						break;
					case STEP_TWO:
						
						break;
					case STEP_THREE:
						
						break;
					case STEP_FOUR:
						
						break;
					case STEP_FIVE:
						
						break;
					case STEP_SIX:
						
						break;
					case STEP_SEVEN:
						
						break;
					case STEP_EIGHT:
						
						break;
				}
				
				this.animStartTime = System.currentTimeMillis();
				this.inputView.invalidate();
			}
		} catch (Exception e) {
			IKnowUKeyboardService.sendErrorMessage(e);
		}
	}
	
	
	private void drawStepOne(Canvas canvas, Paint paint, double interpol) {
		try {
			paint.setTextSize(this.textSize);
			paint.setColor(0xFFFFFFFF);
			
			TextPaint tp = new TextPaint(paint);
			StaticLayout layout = new StaticLayout("NEW!! Use the slide out tabs on the side to easily access settings, and numeric/voice keypads!", tp, this.inputView.getWidth(), Layout.Alignment.ALIGN_CENTER, 1.3f, 0, false);
			
			paint.setColor(0xaa000000);
			canvas.drawRect(0, 0, this.inputView.getWidth(), this.inputView.getHeight(), paint);
			
			canvas.save();
			canvas.translate(20, this.inputView.getHeight() / 2); //position the text
			layout.draw(canvas);
			canvas.restore();
			
			/*final int x = (this.inputView.getWidth() / 2) - (this.arrow.getIntrinsicWidth() / 2);
			final int y = 20;
			
			final Rect rect = new Rect(x, y, x+this.arrow.getIntrinsicWidth(), y+this.arrow.getIntrinsicHeight());
			//IKnowUKeyboardService.log(Log.VERBOSE, "drawStepTwo", "rect left = "+rect.left+", right = "+rect.right+", top = "+rect.top+", bottom = "+rect.bottom);
			this.arrow.setBounds(rect);
			this.arrow.draw(canvas);*/
		} catch (Exception e) {
			IKnowUKeyboardService.sendErrorMessage(e);
		}
	}
	
	private void drawStepTwo(Canvas canvas, Paint paint, double interpol) {
		try {
			
		} catch (Exception e) {
			IKnowUKeyboardService.sendErrorMessage(e);
		}
	}
	
	private void drawStepThree(Canvas canvas, Paint paint, double interpol) {
		try {
			
		} catch (Exception e) {
			IKnowUKeyboardService.sendErrorMessage(e);
		}
	}

	private void drawStepFour(Canvas canvas, Paint paint, double interpol) {
		try {
			
		} catch (Exception e) {
			IKnowUKeyboardService.sendErrorMessage(e);
		}
	}

	private void drawStepFive(Canvas canvas, Paint paint, double interpol) {
		try {
			
		} catch (Exception e) {
			IKnowUKeyboardService.sendErrorMessage(e);
		}
	}

	private void drawStepSix(Canvas canvas, Paint paint, double interpol) {
		try {
			
		} catch (Exception e) {
			IKnowUKeyboardService.sendErrorMessage(e);
		}
	}

	private void drawStepSeven(Canvas canvas, Paint paint, double interpol) {
		try {
			
		} catch (Exception e) {
			IKnowUKeyboardService.sendErrorMessage(e);
		}
	}
	
	private void drawStepEight(Canvas canvas, Paint paint, double interpol) {
		try {
			
		} catch (Exception e) {
			IKnowUKeyboardService.sendErrorMessage(e);
		}
	}
}
