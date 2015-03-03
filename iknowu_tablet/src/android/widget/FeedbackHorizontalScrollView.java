package android.widget;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Path;
import android.graphics.Point;
import android.graphics.Rect;
import android.util.AttributeSet;

/**
 * FeedbackHorizontalScrollView class adds markers to indicate overflow areas.
 * Created by Justin on 07/05/2014.
 */
public class FeedbackHorizontalScrollView extends HorizontalScrollView {
    private final int X_OFFSET = 6;
    private final int Y_OFFSET = 6;
    private final int MAX_ALPHA = 100;

    private static final int FADE_MILLISECONDS = 6000;
    private static final int FADE_STEP = 120;

    // Calculate our alpha step from our fade parameters
    private static final int ALPHA_STEP = 255 / (FADE_MILLISECONDS / FADE_STEP);

    private Paint mPaint;
    private Rect mDrawingRect;
    private Path mPath;

    private Point mPointA;
    private Point mPointB;
    private Point mPointC;

    private int mLeftAlpha = MAX_ALPHA;
    private int mRightAlpha = MAX_ALPHA;

    public FeedbackHorizontalScrollView(Context context) {
        super(context);
        init();
    }

    public FeedbackHorizontalScrollView(Context context, AttributeSet attrs) {
        super(context, attrs);
        init();
    }

    public FeedbackHorizontalScrollView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        init();
    }

    /**
     * Draw all child components plus the overflow indicators.
     * @param  canvas The Canvas to which the View is rendered.
     */
    @Override
    public void draw(Canvas canvas) {
        super.draw(canvas);

        int range = computeHorizontalScrollRange();
        int extent = computeHorizontalScrollExtent();
        int offset = computeHorizontalScrollOffset();

        getDrawingRect(mDrawingRect);
        int width = this.getWidth();

        if (range > extent) {
            final int restoreCount = canvas.save();
            if (offset == 0) {
                // show right indicator
                drawRightIndicator(canvas, (width - X_OFFSET) + mDrawingRect.left, Y_OFFSET, false);
                drawLeftIndicator(canvas, X_OFFSET + mDrawingRect.left, Y_OFFSET, true);

            } else if (offset == (range - extent)) {
                // show left indicator
                drawLeftIndicator(canvas, X_OFFSET + mDrawingRect.left, Y_OFFSET, false);
                drawRightIndicator(canvas, (width - X_OFFSET) + mDrawingRect.left, Y_OFFSET, true);

            } else {
                // show both indicators
                drawRightIndicator(canvas, (width - X_OFFSET) + mDrawingRect.left, Y_OFFSET, false);
                drawLeftIndicator(canvas, X_OFFSET + mDrawingRect.left, Y_OFFSET, false);
            }
            canvas.restoreToCount(restoreCount);
        }
    }

    /**
     * Initialize the View.
     */
    private void init() {
        mPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
        mPaint.setARGB(MAX_ALPHA, 0, 0, 0);
        mPaint.setStyle(Paint.Style.FILL);

        mDrawingRect = new Rect();
        mPath = new Path();

        mPointA = new Point();
        mPointB = new Point();
        mPointC = new Point();
    }

    /**
     * Draw the left indicator. If fade is true then fade out the indicator.
     */
    private void drawLeftIndicator(Canvas canvas, int x, int y, boolean fade) {
        mPointA.set(x + X_OFFSET, y - Y_OFFSET);
        mPointB.set(x + X_OFFSET, y + Y_OFFSET);
        mPointC.set(x - X_OFFSET, y);

        mPath.reset();
        mPath.moveTo(mPointB.x, mPointB.y);
        mPath.lineTo(mPointC.x, mPointC.y);
        mPath.lineTo(mPointA.x, mPointA.y);
        mPath.close();

        if (fade) {
            if (mLeftAlpha > 0) {
                mLeftAlpha -= ALPHA_STEP;
                postInvalidateDelayed(FADE_STEP, x - X_OFFSET, y - Y_OFFSET, x + X_OFFSET, y + Y_OFFSET);
            }
        } else {
            mLeftAlpha = MAX_ALPHA;
        }

        mPaint.setARGB(mLeftAlpha, 0, 0, 0);
        canvas.drawPath(mPath, mPaint);
    }

    /**
     * Draw the right indicator. If fade is true then fade out the indicator.
     */
    private void drawRightIndicator(Canvas canvas, int x, int y, boolean fade) {
        mPointA.set(x - X_OFFSET, y - Y_OFFSET);
        mPointB.set(x - X_OFFSET, y + Y_OFFSET);
        mPointC.set(x + X_OFFSET, y);

        mPath.reset();
        mPath.moveTo(mPointB.x, mPointB.y);
        mPath.lineTo(mPointC.x, mPointC.y);
        mPath.lineTo(mPointA.x, mPointA.y);
        mPath.close();

        if (fade) {
            if (mRightAlpha > 0) {
                mRightAlpha -= ALPHA_STEP;
                postInvalidateDelayed(FADE_STEP, x - X_OFFSET, y - Y_OFFSET, x + X_OFFSET, y + Y_OFFSET);
            }
        } else {
            mRightAlpha = MAX_ALPHA;
        }

        mPaint.setARGB(mRightAlpha, 0, 0, 0);
        canvas.drawPath(mPath, mPaint);
    }
}
