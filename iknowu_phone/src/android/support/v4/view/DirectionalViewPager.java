package android.support.v4.view;

import android.content.Context;
import android.os.Parcel;
import android.os.Parcelable;
import android.support.v4.os.ParcelableCompat;
import android.support.v4.os.ParcelableCompatCreatorCallbacks;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.VelocityTracker;
import android.view.View;
import android.view.ViewConfiguration;
import android.widget.Scroller;

import java.util.ArrayList;

// Referenced classes of package android.support.v4.view:
//            ViewPager, ViewConfigurationCompat, VerticalViewPagerCompat, PagerAdapter, 
//            MotionEventCompat, VelocityTrackerCompat

public class DirectionalViewPager extends ViewPager
{
    private class DataSetObserver extends android.database.DataSetObserver
    {

        public void onChanged()
        {
            dataSetChanged();
        }

        final DirectionalViewPager this$0;

        private DataSetObserver()
        {
        	super();
            this$0 = DirectionalViewPager.this;
        }

    }

    public static class SavedState extends android.view.View.BaseSavedState
    {

        public void writeToParcel(Parcel out, int flags)
        {
            super.writeToParcel(out, flags);
            out.writeInt(position);
            out.writeParcelable(adapterState, flags);
        }

        public String toString()
        {
            return (new StringBuilder()).append("FragmentPager.SavedState{").append(Integer.toHexString(System.identityHashCode(this))).append(" position=").append(position).append("}").toString();
        }

        int position;
        Parcelable adapterState;
        ClassLoader loader;
        public static final android.os.Parcelable.Creator CREATOR = ParcelableCompat.newCreator(new ParcelableCompatCreatorCallbacks() {
        	
            public SavedState createFromParcel(Parcel in, ClassLoader loader)
            {
                return new SavedState(in, loader);
            }

            public SavedState[] newArray(int size)
            {
                return new SavedState[size];
            }
            
            /*
            public volatile Object[] newArray(int x0)
            {
                return newArray(x0);
            }

            public volatile Object createFromParcel(Parcel x0, ClassLoader x1)
            {
                return createFromParcel(x0, x1);
            }
            */
        });


        public SavedState(Parcelable superState)
        {
            super(superState);
        }

        SavedState(Parcel in, ClassLoader loader)
        {
            super(in);
            if(loader == null)
                loader = getClass().getClassLoader();
            position = in.readInt();
            adapterState = in.readParcelable(loader);
            this.loader = loader;
        }
    }


    public DirectionalViewPager(Context context)
    {
        super(context);
        mItems = new ArrayList();
        mRestoredCurItem = -1;
        mRestoredAdapterState = null;
        mRestoredClassLoader = null;
        mOrientation = 0;
        mActivePointerId = -1;
        mScrollState = 0;
        initViewPager();
    }

    public DirectionalViewPager(Context context, AttributeSet attrs)
    {
        super(context, attrs);
        mItems = new ArrayList();
        mRestoredCurItem = -1;
        mRestoredAdapterState = null;
        mRestoredClassLoader = null;
        mOrientation = 0;
        mActivePointerId = -1;
        mScrollState = 0;
        initViewPager();
        int orientation = attrs.getAttributeIntValue("http://schemas.android.com/apk/res/android", "orientation", -1);
        if(orientation != -1)
            setOrientation(orientation);
    }

    void initViewPager()
    {
        super.initViewPager();
        setWillNotDraw(false);
        mScroller = new Scroller(getContext());
        ViewConfiguration configuration = ViewConfiguration.get(getContext());
        mTouchSlop = ViewConfigurationCompat.getScaledPagingTouchSlop(configuration);
        mMinimumVelocity = configuration.getScaledMinimumFlingVelocity();
        mMaximumVelocity = configuration.getScaledMaximumFlingVelocity();
    }

    private void setScrollState(int newState)
    {
        if(mScrollState == newState)
            return;
        mScrollState = newState;
        if(mOnPageChangeListener != null)
            mOnPageChangeListener.onPageScrollStateChanged(newState);
    }

    public void setAdapter(PagerAdapter adapter)
    {
        mAdapter = adapter;
        if(mAdapter != null)
        {
            if(mObserver == null)
                mObserver = new DataSetObserver();
            VerticalViewPagerCompat.setDataSetObserver(mAdapter, mObserver);
            mPopulatePending = false;
            if(mRestoredCurItem >= 0)
            {
                mAdapter.restoreState(mRestoredAdapterState, mRestoredClassLoader);
                setCurrentItemInternal(mRestoredCurItem, false, true);
                mRestoredCurItem = -1;
                mRestoredAdapterState = null;
                mRestoredClassLoader = null;
            } else
            {
                populate();
            }
        }
    }

    public PagerAdapter getAdapter()
    {
        return mAdapter;
    }

    public void setCurrentItem(int item)
    {
        mPopulatePending = false;
        setCurrentItemInternal(item, true, false);
    }

    void setCurrentItemInternal(int item, boolean smoothScroll, boolean always)
    {
        if(mAdapter == null || mAdapter.getCount() <= 0)
        {
            setScrollingCacheEnabled(false);
            return;
        }
        if(!always && mCurItem == item && mItems.size() != 0)
        {
            setScrollingCacheEnabled(false);
            return;
        }
        if(item < 0)
            item = 0;
        else
        if(item >= mAdapter.getCount())
            item = mAdapter.getCount() - 1;
        if(item > mCurItem + 1 || item < mCurItem - 1)
        {
            for(int i = 0; i < mItems.size(); i++)
                ((ViewPager.ItemInfo)mItems.get(i)).scrolling = true;

        }
        boolean dispatchSelected = mCurItem != item;
        mCurItem = item;
        populate();
        if(smoothScroll)
        {
            if(mOrientation == 0)
                smoothScrollTo(getWidth() * item, 0);
            else
                smoothScrollTo(0, getHeight() * item);
            if(dispatchSelected && mOnPageChangeListener != null)
                mOnPageChangeListener.onPageSelected(item);
        } else
        {
            if(dispatchSelected && mOnPageChangeListener != null)
                mOnPageChangeListener.onPageSelected(item);
            completeScroll();
            if(mOrientation == 0)
                scrollTo(getWidth() * item, 0);
            else
                scrollTo(0, getHeight() * item);
        }
    }

    public void setOnPageChangeListener(ViewPager.OnPageChangeListener listener)
    {
        mOnPageChangeListener = listener;
    }

    void smoothScrollTo(int x, int y)
    {
        if(getChildCount() == 0)
        {
            setScrollingCacheEnabled(false);
            return;
        }
        int sx = getScrollX();
        int sy = getScrollY();
        int dx = x - sx;
        int dy = y - sy;
        if(dx == 0 && dy == 0)
        {
            completeScroll();
            return;
        } else
        {
            setScrollingCacheEnabled(true);
            mScrolling = true;
            setScrollState(2);
            mScroller.startScroll(sx, sy, dx, dy);
            invalidate();
            return;
        }
    }
    
    @Override
    ItemInfo addNewItem(int position, int index)
    {
        ViewPager.ItemInfo ii = new ViewPager.ItemInfo();
        ii.position = position;
        ii.object = mAdapter.instantiateItem(this, position);
        if(index < 0)
            mItems.add(ii);
        else
            mItems.add(index, ii);

        return ii;
    }

    void dataSetChanged()
    {
        boolean needPopulate = mItems.isEmpty() && mAdapter.getCount() > 0;
        int newCurrItem = -1;
        for(int i = 0; i < mItems.size(); i++)
        {
            ViewPager.ItemInfo ii = (ViewPager.ItemInfo)mItems.get(i);
            int newPos = mAdapter.getItemPosition(ii.object);
            if(newPos == -1)
                continue;
            if(newPos == -2)
            {
                mItems.remove(i);
                i--;
                mAdapter.destroyItem(this, ii.position, ii.object);
                needPopulate = true;
                if(mCurItem == ii.position)
                    newCurrItem = Math.max(0, Math.min(mCurItem, mAdapter.getCount() - 1));
                continue;
            }
            if(ii.position == newPos)
                continue;
            if(ii.position == mCurItem)
                newCurrItem = newPos;
            ii.position = newPos;
            needPopulate = true;
        }

        if(newCurrItem >= 0)
        {
            setCurrentItemInternal(newCurrItem, false, true);
            needPopulate = true;
        }
        if(needPopulate)
        {
            populate();
            requestLayout();
        }
    }

    void populate()
    {
        if(mAdapter == null)
            return;
        if(mPopulatePending)
            return;
        if(getWindowToken() == null)
            return;
        mAdapter.startUpdate(this);
        int startPos = mCurItem <= 0 ? mCurItem : mCurItem - 1;
        int count = mAdapter.getCount();
        int endPos = mCurItem >= count - 1 ? count - 1 : mCurItem + 1;
        int lastPos = -1;
        for(int i = 0; i < mItems.size(); i++)
        {
            ViewPager.ItemInfo ii = (ViewPager.ItemInfo)mItems.get(i);
            if((ii.position < startPos || ii.position > endPos) && !ii.scrolling)
            {
                mItems.remove(i);
                i--;
                mAdapter.destroyItem(this, ii.position, ii.object);
            } else
            if(lastPos < endPos && ii.position > startPos)
            {
                if(++lastPos < startPos)
                    lastPos = startPos;
                while(lastPos <= endPos && lastPos < ii.position) 
                {
                    addNewItem(lastPos, i);
                    lastPos++;
                    i++;
                }
            }
            lastPos = ii.position;
        }

        lastPos = mItems.size() <= 0 ? -1 : ((ViewPager.ItemInfo)mItems.get(mItems.size() - 1)).position;
        if(lastPos < endPos)
            for(lastPos = ++lastPos <= startPos ? startPos : lastPos; lastPos <= endPos; lastPos++)
                addNewItem(lastPos, -1);

        mAdapter.finishUpdate(this);
    }

    public Parcelable onSaveInstanceState()
    {
        Parcelable superState = super.onSaveInstanceState();
        SavedState ss = new SavedState(superState);
        ss.position = mCurItem;
        ss.adapterState = mAdapter.saveState();
        return ss;
    }

    public void onRestoreInstanceState(Parcelable state)
    {
        if(!(state instanceof SavedState))
        {
            super.onRestoreInstanceState(state);
            return;
        }
        SavedState ss = (SavedState)state;
        super.onRestoreInstanceState(ss.getSuperState());
        if(mAdapter != null)
        {
            mAdapter.restoreState(ss.adapterState, ss.loader);
            setCurrentItemInternal(ss.position, false, true);
        } else
        {
            mRestoredCurItem = ss.position;
            mRestoredAdapterState = ss.adapterState;
            mRestoredClassLoader = ss.loader;
        }
    }

    public int getOrientation()
    {
        return mOrientation;
    }

    public void setOrientation(int orientation)
    {
        switch(orientation)
        {
        default:
            throw new IllegalArgumentException("Only HORIZONTAL and VERTICAL are valid orientations.");

        case 0: // '\0'
        case 1: // '\001'
            break;
        }
        if(orientation == mOrientation)
            return;
        completeScroll();
        mInitialMotion = 0.0F;
        mLastMotionX = 0.0F;
        mLastMotionY = 0.0F;
        if(mVelocityTracker != null)
            mVelocityTracker.clear();
        mOrientation = orientation;
        if(mOrientation == 0)
            scrollTo(mCurItem * getWidth(), 0);
        else
            scrollTo(0, mCurItem * getHeight());
        requestLayout();
    }

    public void addView(View child, int index, android.view.ViewGroup.LayoutParams params)
    {
        if(mInLayout)
        {
            addViewInLayout(child, index, params);
            child.measure(mChildWidthMeasureSpec, mChildHeightMeasureSpec);
        } else
        {
            super.addView(child, index, params);
        }
    }

    ViewPager.ItemInfo infoForChild(View child)
    {
        for(int i = 0; i < mItems.size(); i++)
        {
            ViewPager.ItemInfo ii = (ViewPager.ItemInfo)mItems.get(i);
            if(mAdapter.isViewFromObject(child, ii.object))
                return ii;
        }

        return null;
    }

    protected void onAttachedToWindow()
    {
        super.onAttachedToWindow();
        if(mAdapter != null)
            populate();
    }

    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec)
    {
        setMeasuredDimension(getDefaultSize(0, widthMeasureSpec), getDefaultSize(0, heightMeasureSpec));
        mChildWidthMeasureSpec = android.view.View.MeasureSpec.makeMeasureSpec(getMeasuredWidth() - getPaddingLeft() - getPaddingRight(), 1073741824);
        mChildHeightMeasureSpec = android.view.View.MeasureSpec.makeMeasureSpec(getMeasuredHeight() - getPaddingTop() - getPaddingBottom(), 1073741824);
        mInLayout = true;
        populate();
        mInLayout = false;
        int size = getChildCount();
        for(int i = 0; i < size; i++)
        {
            View child = getChildAt(i);
            if(child.getVisibility() != 8)
                child.measure(mChildWidthMeasureSpec, mChildHeightMeasureSpec);
        }

    }

    protected void onSizeChanged(int w, int h, int oldw, int oldh)
    {
        super.onSizeChanged(w, h, oldw, oldh);
        if(mOrientation == 0)
        {
            int scrollPos = mCurItem * w;
            if(scrollPos != getScrollX())
            {
                completeScroll();
                scrollTo(scrollPos, getScrollY());
            }
        } else
        {
            int scrollPos = mCurItem * h;
            if(scrollPos != getScrollY())
            {
                completeScroll();
                scrollTo(getScrollX(), scrollPos);
            }
        }
    }

    protected void onLayout(boolean changed, int l, int t, int r, int b)
    {
        mInLayout = true;
        populate();
        mInLayout = false;
        int count = getChildCount();
        int size = mOrientation != 0 ? b - t : r - l;
        for(int i = 0; i < count; i++)
        {
            View child = getChildAt(i);
            ViewPager.ItemInfo ii;
            if(child.getVisibility() == 8 || (ii = infoForChild(child)) == null)
                continue;
            int off = size * ii.position;
            int childLeft = getPaddingLeft();
            int childTop = getPaddingTop();
            if(mOrientation == 0)
                childLeft += off;
            else
                childTop += off;
            child.layout(childLeft, childTop, childLeft + child.getMeasuredWidth(), childTop + child.getMeasuredHeight());
        }

    }

    public void computeScroll()
    {
        if(!mScroller.isFinished() && mScroller.computeScrollOffset())
        {
            int oldX = getScrollX();
            int oldY = getScrollY();
            int x = mScroller.getCurrX();
            int y = mScroller.getCurrY();
            if(oldX != x || oldY != y)
                scrollTo(x, y);
            if(mOnPageChangeListener != null)
            {
                int size;
                int value;
                if(mOrientation == 0)
                {
                    size = getWidth();
                    value = x;
                } else
                {
                    size = getHeight();
                    value = y;
                }
                int position = value / size;
                int offsetPixels = value % size;
                float offset = (float)offsetPixels / (float)size;
                mOnPageChangeListener.onPageScrolled(position, offset, offsetPixels);
            }
            invalidate();
            return;
        } else
        {
            completeScroll();
            return;
        }
    }

    private void completeScroll()
    {
        boolean needPopulate;
        if(needPopulate = mScrolling)
        {
            setScrollingCacheEnabled(false);
            mScroller.abortAnimation();
            int oldX = getScrollX();
            int oldY = getScrollY();
            int x = mScroller.getCurrX();
            int y = mScroller.getCurrY();
            if(oldX != x || oldY != y)
                scrollTo(x, y);
            setScrollState(0);
        }
        mPopulatePending = false;
        mScrolling = false;
        for(int i = 0; i < mItems.size(); i++)
        {
            ViewPager.ItemInfo ii = (ViewPager.ItemInfo)mItems.get(i);
            if(ii.scrolling)
            {
                needPopulate = true;
                ii.scrolling = false;
            }
        }

        if(needPopulate)
            populate();
    }

    public boolean onInterceptTouchEvent(MotionEvent ev)
    {
        int action = ev.getAction() & 255;
        if(action == 3 || action == 1)
        {
            mIsBeingDragged = false;
            mIsUnableToDrag = false;
            mActivePointerId = -1;
            return false;
        }
        if(action != 0)
        {
            if(mIsBeingDragged)
                return true;
            if(mIsUnableToDrag)
                return false;
        }
        switch(action)
        {
        default:
            break;

        case 2: // '\002'
            int activePointerId = mActivePointerId;
            if(activePointerId != -1 || android.os.Build.VERSION.SDK_INT <= 4)
            {
                int pointerIndex = MotionEventCompat.findPointerIndex(ev, activePointerId);
                float x = MotionEventCompat.getX(ev, pointerIndex);
                float y = MotionEventCompat.getY(ev, pointerIndex);
                float xDiff = Math.abs(x - mLastMotionX);
                float yDiff = Math.abs(y - mLastMotionY);
                float primaryDiff;
                float secondaryDiff;
                if(mOrientation == 0)
                {
                    primaryDiff = xDiff;
                    secondaryDiff = yDiff;
                } else
                {
                    primaryDiff = yDiff;
                    secondaryDiff = xDiff;
                }
                if(primaryDiff > (float)mTouchSlop && primaryDiff > secondaryDiff)
                {
                    mIsBeingDragged = true;
                    setScrollState(1);
                    if(mOrientation == 0)
                        mLastMotionX = x;
                    else
                        mLastMotionY = y;
                    setScrollingCacheEnabled(true);
                } else
                if(secondaryDiff > (float)mTouchSlop)
                    mIsUnableToDrag = true;
            }
            break;

        case 0: // '\0'
            if(mOrientation == 0)
            {
                mLastMotionX = mInitialMotion = ev.getX();
                mLastMotionY = ev.getY();
            } else
            {
                mLastMotionX = ev.getX();
                mLastMotionY = mInitialMotion = ev.getY();
            }
            mActivePointerId = MotionEventCompat.getPointerId(ev, 0);
            if(mScrollState == 2)
            {
                mIsBeingDragged = true;
                mIsUnableToDrag = false;
                setScrollState(1);
            } else
            {
                completeScroll();
                mIsBeingDragged = false;
                mIsUnableToDrag = false;
            }
            break;

        case 6: // '\006'
            onSecondaryPointerUp(ev);
            break;
        }
        return mIsBeingDragged;
    }

    public boolean onTouchEvent(MotionEvent ev)
    {
        if(ev.getAction() == 0 && ev.getEdgeFlags() != 0)
            return false;
        if(mAdapter == null || mAdapter.getCount() == 0)
            return false;
        if(mVelocityTracker == null)
            mVelocityTracker = VelocityTracker.obtain();
        mVelocityTracker.addMovement(ev);
        int action = ev.getAction();
        switch(action & 255)
        {
        case MotionEvent.ACTION_OUTSIDE: // '\004'
        default:
            break;

        case MotionEvent.ACTION_DOWN: // '\0'
        {
            completeScroll();
            if(mOrientation == 0)
                mLastMotionX = mInitialMotion = ev.getX();
            else
                mLastMotionY = mInitialMotion = ev.getY();
            mActivePointerId = MotionEventCompat.getPointerId(ev, 0);
            break;
        }

        case MotionEvent.ACTION_MOVE: // '\002'
        {
            float x;
            float y;
            if(!mIsBeingDragged)
            {
                int pointerIndex = MotionEventCompat.findPointerIndex(ev, mActivePointerId);
                x = MotionEventCompat.getX(ev, pointerIndex);
                y = MotionEventCompat.getY(ev, pointerIndex);
                float xDiff = Math.abs(x - mLastMotionX);
                float yDiff = Math.abs(y - mLastMotionY);
                float primaryDiff;
                float secondaryDiff;
                if(mOrientation == 0)
                {
                    primaryDiff = xDiff;
                    secondaryDiff = yDiff;
                } else
                {
                    primaryDiff = yDiff;
                    secondaryDiff = xDiff;
                }
                if(primaryDiff > (float)mTouchSlop && primaryDiff > secondaryDiff)
                {
                    mIsBeingDragged = true;
                    if(mOrientation == 0)
                        mLastMotionX = x;
                    else
                        mLastMotionY = y;
                    setScrollState(1);
                    setScrollingCacheEnabled(true);
                }
            }
            if(!mIsBeingDragged)
                break;
            int activePointerIndex = MotionEventCompat.findPointerIndex(ev, mActivePointerId);
            x = MotionEventCompat.getX(ev, activePointerIndex);
            y = MotionEventCompat.getY(ev, activePointerIndex);
            int size;
            float scroll;
            if(mOrientation == 0)
            {
                size = getWidth();
                scroll = (float)getScrollX() + (mLastMotionX - x);
                mLastMotionX = x;
            } else
            {
                size = getHeight();
                scroll = (float)getScrollY() + (mLastMotionY - y);
                mLastMotionY = y;
            }
            float lowerBound = Math.max(0, (mCurItem - 1) * size);
            float upperBound = Math.min(mCurItem + 1, mAdapter.getCount() - 1) * size;
            if(scroll < lowerBound)
                scroll = lowerBound;
            else
            if(scroll > upperBound)
                scroll = upperBound;
            if(mOrientation == 0)
            {
                mLastMotionX += scroll - (float)(int)scroll;
                scrollTo((int)scroll, getScrollY());
            } else
            {
                mLastMotionY += scroll - (float)(int)scroll;
                scrollTo(getScrollX(), (int)scroll);
            }
            if(mOnPageChangeListener != null)
            {
                int position = (int)scroll / size;
                int positionOffsetPixels = (int)scroll % size;
                float positionOffset = (float)positionOffsetPixels / (float)size;
                mOnPageChangeListener.onPageScrolled(position, positionOffset, positionOffsetPixels);
            }
            break;
        }

        case MotionEvent.ACTION_UP: // '\001'
        {
            if(!mIsBeingDragged)
                break;
            VelocityTracker velocityTracker = mVelocityTracker;
            velocityTracker.computeCurrentVelocity(1000, mMaximumVelocity);
            int initialVelocity;
            float lastMotion;
            int sizeOverThree;
            if(mOrientation == 0)
            {
                initialVelocity = (int)VelocityTrackerCompat.getXVelocity(velocityTracker, mActivePointerId);
                lastMotion = mLastMotionX;
                sizeOverThree = getWidth() / 2;
            } else
            {
                initialVelocity = (int)VelocityTrackerCompat.getYVelocity(velocityTracker, mActivePointerId);
                lastMotion = mLastMotionY;
                sizeOverThree = getHeight() / 2;
            }
            mPopulatePending = true;
            if(Math.abs(initialVelocity) > mMinimumVelocity || Math.abs(mInitialMotion - lastMotion) >= (float)sizeOverThree)
            {
                if(lastMotion > mInitialMotion)
                    setCurrentItemInternal(mCurItem - 1, true, true);
                else
                    setCurrentItemInternal(mCurItem + 1, true, true);
            } else
            {
                setCurrentItemInternal(mCurItem, true, true);
            }
            mActivePointerId = -1;
            endDrag();
            break;
        }

        case MotionEvent.ACTION_CANCEL: // '\003'
        {
            if(mIsBeingDragged)
            {
                setCurrentItemInternal(mCurItem, true, true);
                mActivePointerId = -1;
                endDrag();
            }
            break;
        }

        case MotionEvent.ACTION_POINTER_1_DOWN: // '\005'
        {
            int index = MotionEventCompat.getActionIndex(ev);
            if(mOrientation == 0)
                mLastMotionX = MotionEventCompat.getX(ev, index);
            else
                mLastMotionY = MotionEventCompat.getY(ev, index);
            mActivePointerId = MotionEventCompat.getPointerId(ev, index);
            break;
        }

        case MotionEvent.ACTION_POINTER_1_UP: // '\006'
        {
            onSecondaryPointerUp(ev);
            int index = MotionEventCompat.findPointerIndex(ev, mActivePointerId);
            if(mOrientation == 0)
                mLastMotionX = MotionEventCompat.getX(ev, index);
            else
                mLastMotionY = MotionEventCompat.getY(ev, index);
            break;
        }
        }
        return true;
    }

    private void onSecondaryPointerUp(MotionEvent ev)
    {
        int pointerIndex = MotionEventCompat.getActionIndex(ev);
        int pointerId = MotionEventCompat.getPointerId(ev, pointerIndex);
        if(pointerId == mActivePointerId)
        {
            int newPointerIndex = pointerIndex != 0 ? 0 : 1;
            if(mOrientation == 0)
                mLastMotionX = MotionEventCompat.getX(ev, newPointerIndex);
            else
                mLastMotionY = MotionEventCompat.getY(ev, newPointerIndex);
            mActivePointerId = MotionEventCompat.getPointerId(ev, newPointerIndex);
            if(mVelocityTracker != null)
                mVelocityTracker.clear();
        }
    }

    private void endDrag()
    {
        mIsBeingDragged = false;
        mIsUnableToDrag = false;
        if(mVelocityTracker != null)
        {
            mVelocityTracker.recycle();
            mVelocityTracker = null;
        }
    }

    private void setScrollingCacheEnabled(boolean enabled)
    {
        if(mScrollingCacheEnabled != enabled)
            mScrollingCacheEnabled = enabled;
    }

    private static final String TAG = "DirectionalViewPager";
    private static final String XML_NS = "http://schemas.android.com/apk/res/android";
    private static final boolean DEBUG = false;
    private static final boolean USE_CACHE = false;
    public static final int HORIZONTAL = 0;
    public static final int VERTICAL = 1;
    private final ArrayList mItems;
    private PagerAdapter mAdapter;
    private int mCurItem;
    private int mRestoredCurItem;
    private Parcelable mRestoredAdapterState;
    private ClassLoader mRestoredClassLoader;
    private Scroller mScroller;
    private int mChildWidthMeasureSpec;
    private int mChildHeightMeasureSpec;
    private boolean mInLayout;
    private boolean mScrollingCacheEnabled;
    private boolean mPopulatePending;
    private boolean mScrolling;
    private boolean mIsBeingDragged;
    private boolean mIsUnableToDrag;
    private int mTouchSlop;
    private float mInitialMotion;
    private float mLastMotionX;
    private float mLastMotionY;
    private int mOrientation;
    private int mActivePointerId;
    private static final int INVALID_POINTER = -1;
    private VelocityTracker mVelocityTracker;
    private int mMinimumVelocity;
    private int mMaximumVelocity;
    DataSetObserver mObserver;
    private ViewPager.OnPageChangeListener mOnPageChangeListener;
    private int mScrollState;
}
