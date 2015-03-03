package com.iknowu.preview;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Path;
import android.graphics.RectF;
import android.util.AttributeSet;
import android.view.Gravity;
import android.widget.TextView;

public class PreviewTextView extends TextView {
	
	private static final int BORDER_WIDTH = 2;
	private static final int BORDER_OFFSET = 1;
	private static final int CORNER_RADIUS = 10;
	
	private Context context;
	private int backgroundColor;
	private int textColor;
	private int borderColor;
	private RectF bgRect;
	private RectF borderRect;
	private Paint paint;
	private Path bgPath;

	public PreviewTextView(Context context) {
		super(context);
		this.context = context;
		//this.BORDER_WIDTH = this.context.getResources().getDimensionPixelSize(R.dimen.preview_border_width);
	}
	
	public PreviewTextView(Context context, AttributeSet attrs) {
		super(context, attrs);
		this.context = context;
		//this.BORDER_WIDTH = this.context.getResources().getDimensionPixelSize(R.dimen.preview_border_width);
	}
	
	public PreviewTextView(Context context, AttributeSet attrs, int defStyle) {
		super(context, attrs, defStyle);
		this.context = context;
		//this.BORDER_WIDTH = this.context.getResources().getDimensionPixelSize(R.dimen.preview_border_width);
	}
	
	public void init(int bgColor, int borderColor, int textColor) {
		this.paint = new Paint();
		this.paint.setAntiAlias(true);
		
		this.backgroundColor = bgColor;
		this.borderColor = borderColor;
		this.textColor = textColor;
		//this.backgroundColor = 0xFFFFFFFF;
		//this.borderColor = borderColor;
		//this.textColor = 0xFF000000;
		this.setBackgroundColor(0x00FFFFFF);
		this.setTextColor(this.textColor);
		//this.setTextSize(TypedValue.COMPLEX_UNIT_SP, 30);
		this.setGravity(Gravity.CENTER_HORIZONTAL);
	}
	
	@Override
	public void onSizeChanged(int w, int h, int oldw, int oldh) {
		super.onSizeChanged(w, h, oldw, oldh);
		this.bgRect = new RectF( 0 , 0 , this.getWidth(), this.getHeight());
		
		this.setPathKeyWidth();
		
		this.borderRect = new RectF( BORDER_OFFSET , BORDER_OFFSET , this.getWidth() - BORDER_OFFSET, this.getHeight() - BORDER_OFFSET);
	}
	
	public void setPathKeyWidth() {
		
		float w = this.getWidth();
		float h = this.getHeight();
		
		this.bgPath = new Path();
		this.bgPath.moveTo( 0f, 0f );	//start left, top
		this.bgPath.lineTo( 0f, (h * 0.8f) ); //move to 60% down the left side
		this.bgPath.lineTo( (w * 0.4f), (h * 0.8f) ); //move to 60% down the left side
		this.bgPath.lineTo( (w * 0.5f), h ); //move to 60% down the left side
		this.bgPath.lineTo( (w * 0.6f), (h * 0.8f) ); //move to 60% down the left side
		this.bgPath.lineTo( w, (h * 0.8f) ); //move to 20% to the right, and 100% down
		this.bgPath.lineTo( w, 0f); //move to right, top
		this.bgPath.close();
	}
	
	@Override
	public void onDraw(Canvas canvas) {
		//draw the background
		
		paint.setColor(this.backgroundColor);
		paint.setStyle(Paint.Style.FILL);
		canvas.drawPath(this.bgPath, paint);
		//canvas.drawRoundRect(this.bgRect, CORNER_RADIUS, CORNER_RADIUS, paint);

		paint.setColor(this.borderColor);
		paint.setStyle(Paint.Style.STROKE);
		paint.setStrokeWidth(BORDER_WIDTH);
		canvas.drawPath(this.bgPath, paint);
		//canvas.drawRoundRect(this.borderRect, CORNER_RADIUS, CORNER_RADIUS, paint);

		super.onDraw(canvas);
	}
}
