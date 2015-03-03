package com.iknowu.setup;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.LinearGradient;
import android.graphics.Paint;
import android.graphics.RadialGradient;
import android.graphics.Rect;
import android.graphics.Shader;
import android.util.AttributeSet;
import android.util.TypedValue;
import android.view.MotionEvent;
import android.widget.RelativeLayout;
import android.widget.TextView;

import com.iknowu.IKnowUKeyboardService;
import com.iknowu.R;

public class SetupRelativeLayout extends RelativeLayout {

    private static final float ANIM_DURATION = 500f;
    private static final int RECT_COLOR_START = 0xAA92D34C;
    private static final int RECT_COLOR_FINISH = 0xAA52930C;

    private static final int BG_COLOR = 0x11BBBBBB;
    private static final int COLOR_BLUE = 0xFF1E87D2;
    private static final int COLOR_BLACK = 0xFF000000;
    private static final int COLOR_WHITE = 0xFFFFFFFF;

    private static final float NORMAL_TEXT_SIZE = 40f;
    private static final float LARGE_TEXT_SIZE = 50f;
    private static final float SMALL_TEXT_SIZE = 30f;

    private float textSizeIncrement = 0.5f;
    private float currentTextSize;

	public int index;
	
	public boolean enabled;
	
	private TextView textView;
	//private ImageView imgView;

	public String dictName;
	
	private int densityScale;

    private Paint paint;

    private boolean complete;
    private boolean isCurrentTask;
    private long animStart;
	
	public SetupRelativeLayout(Context context) {
		super(context);
		this.enabled = true;
		this.densityScale = this.getResources().getDisplayMetrics().densityDpi;
        this.paint = new Paint();
        //this.paint.setColor(RECT_COLOR);
		this.establishViews(context);
	}
	
	public SetupRelativeLayout(Context context, AttributeSet attrs) {
		super(context, attrs);
		this.enabled = true;
		this.densityScale = this.getResources().getDisplayMetrics().densityDpi;
        this.paint = new Paint();
        //this.paint.setColor(RECT_COLOR);
		this.establishViews(context);
	}
	
	public SetupRelativeLayout(Context context, AttributeSet attrs, int defStyle) {
		super(context, attrs, defStyle);
		this.enabled = true;
		this.densityScale = this.getResources().getDisplayMetrics().densityDpi;
        this.paint = new Paint();
        //this.paint.setColor(RECT_COLOR);
		this.establishViews(context);
	}
	
	private void establishViews(Context context) {
		try {
            this.currentTextSize = NORMAL_TEXT_SIZE;

			//this.setBackgroundResource(R.drawable.info_bg_grey);
			this.setBackgroundColor(BG_COLOR);

			this.textView = new TextView(context);
			RelativeLayout.LayoutParams params = new RelativeLayout.LayoutParams(RelativeLayout.LayoutParams.WRAP_CONTENT, RelativeLayout.LayoutParams.WRAP_CONTENT);
			params.addRule(RelativeLayout.CENTER_IN_PARENT);
			this.textView.setLayoutParams(params);
			//this.textView.setOnTouchListener(this);
			this.textView.setTextSize(TypedValue.COMPLEX_UNIT_SP, 20f);
			this.textView.setTextColor(0xFF8C8C8C);
			this.addView(textView);

            /*
			params = new RelativeLayout.LayoutParams(RelativeLayout.LayoutParams.WRAP_CONTENT, RelativeLayout.LayoutParams.WRAP_CONTENT);
			params.addRule(RelativeLayout.ALIGN_PARENT_RIGHT);
			params.addRule(RelativeLayout.CENTER_VERTICAL);
			this.imgView = new ImageView(context);
			this.imgView.setLayoutParams(params);
			this.addView(imgView);
			*/
		} catch (Exception e) {
			IKnowUKeyboardService.sendErrorMessage(e);
		}
	}
	
	public void disable() {
		this.enabled = false;
		//this.imgView.setImageResource();
		//this.checkBox.setEnabled(false);
		//this.textView.setEnabled(false);
	}
	
	public void setTaskComplete() {
		//this.setBackgroundResource(R.drawable.info_bg_green);
		//this.imgView.setImageResource(R.drawable.task_complete);
		this.textView.setTextColor(COLOR_WHITE);
        this.isCurrentTask = false;
        this.complete = true;
	}

    public void startCompleteAnimation() {
        if (this.complete) {
            this.animStart = System.currentTimeMillis();
            this.invalidate();
        }
    }
	
	public void setAsCurrentTask() {
		//this.setBackgroundResource(R.drawable.info_bg_blue);
        //this.setBackgroundColor(BG_COLOR_BLUE);
        this.isCurrentTask = true;
        this.textView.setTextColor(COLOR_BLACK);
		//this.imgView.setImageResource(R.drawable.go_arrow);
	}
	
	public void setAsStaticTask() {
		this.setBackgroundResource(R.drawable.info_bg_blue);
		//this.imgView.setImageResource(R.drawable.go_arrow);
		this.textView.setTextColor(0xFF1E87D2);
	}
	
	public void setImageResource(int resId) {
		//this.imgView.setImageResource(resId);
	}
	
	public CharSequence getText() {
		return this.textView.getText();
	}
	
	public void setText(String text) {
		this.textView.setText(text);
	}

    @Override
    public void onDraw(Canvas canvas) {
        super.onDraw(canvas);

        if (complete) {
            final long diff = System.currentTimeMillis() - this.animStart;

            if (diff < ANIM_DURATION) {
                Rect rect = new Rect();
                this.getDrawingRect(rect);

                //RectF rrect = new RectF(rect);

                final float factor = diff / ANIM_DURATION;
                //final int height = (int) (factor * rect.height());
                //rect.bottom = rect.top + height;
                final int width = (int) (factor * rect.width());
                rect.right = rect.left + width;

                LinearGradient grad = new LinearGradient(rect.left, rect.top, rect.right,rect.top, RECT_COLOR_START, RECT_COLOR_FINISH, Shader.TileMode.CLAMP);
                paint.setShader(grad);
                //canvas.drawRoundRect(rrect, 4.0f, 4.0f, this.paint);

                canvas.drawRect(rect, this.paint);
                this.invalidate();
            } else {
                Rect rect = new Rect();
                this.getDrawingRect(rect);

                //RectF rrect = new RectF(rect);

                LinearGradient grad = new LinearGradient(rect.left, rect.top, rect.right,rect.top, RECT_COLOR_START, RECT_COLOR_FINISH, Shader.TileMode.CLAMP);
                paint.setShader(grad);
                //canvas.drawRoundRect(rect, 4.0f, 4.0f, this.paint);
                canvas.drawRect(rect, this.paint);
            }
        }

        if (this.isCurrentTask) {
            this.currentTextSize += this.textSizeIncrement;

            if (this.currentTextSize > LARGE_TEXT_SIZE) {
                this.textSizeIncrement *= -1f;
                this.currentTextSize += this.textSizeIncrement;
            } else if ( this.currentTextSize < SMALL_TEXT_SIZE) {
                this.textSizeIncrement *= -1f;
                this.currentTextSize += this.textSizeIncrement;
            }

            float x = this.getWidth() / 2;
            float y = this.getHeight() / 2;

            //IKnowUKeyboardService.log(Log.VERBOSE, "SetupRelativeLayout.onDraw()", "x = "+x+", y = "+y);

            RadialGradient grad = new RadialGradient(x, y, this.currentTextSize, 0xFFFFFFFF, 0x00FFFFFF, Shader.TileMode.CLAMP);
            paint.setShader(grad);

            canvas.drawCircle(x, y, this.currentTextSize, paint);
            //Rect rect = new Rect();
            //this.getDrawingRect(rect);
            //canvas.drawOval(new RectF(rect), this.paint);

            //this.textView.setTextSize(TypedValue.COMPLEX_UNIT_SP, this.currentTextSize);

            this.invalidate();
        }
    }
	
	@Override
	public boolean onTouchEvent(MotionEvent event) {
		//Log.d("On Touch Event", "Setup Rel Lay");
		return super.onTouchEvent(event);
	}
}
