package com.iknowu.setup;

import android.content.Context;
import android.os.Parcelable;
import android.support.v4.view.PagerAdapter;
import android.support.v4.view.ViewPager;
import android.util.AttributeSet;
import android.util.DisplayMetrics;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;

import com.iknowu.R;
import com.iknowu.animation.AnimatedFrameLayout;
import com.iknowu.animation.Animation;
import com.iknowu.animation.AnimationStep;
import com.iknowu.util.DimensionConverter;

import java.util.ArrayList;

/**
 * Created by Justin on 16/07/13.
 *
 */
public class StartupViewPager extends ViewPager implements ViewPager.OnPageChangeListener {

    private ArrayList<ViewGroup> views;
    private Context context;
    private TutorialActivity activity;
    private StartupPagerAdapter adapter;

    private int currentItem;

    private ArrayList<Animation> animations;

    public StartupViewPager(Context context) {
        super(context);
        this.context = context;
    }

    public StartupViewPager(Context context, AttributeSet attrs) {
        super(context, attrs);
        this.context = context;
    }

    public void init(TutorialActivity act) {

        this.activity = act;
        this.views = new ArrayList<ViewGroup>();

        this.createAnimations();

        /*LayoutInflater li = (LayoutInflater) this.context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);

        ScrollView first = (ScrollView) li.inflate(R.layout.thanks_for_choosing, null);
        assert first != null;
        Button takeTour = (Button) first.findViewById(R.id.button_take_tour);
        takeTour.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                incrementView();
            }
        });

        Button skip = (Button) first.findViewById(R.id.button_skip);
        skip.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                activity.close();
            }
        });

        this.views.add(first);

        ScrollView second = (ScrollView) li.inflate(R.layout.tour_reach_screen, null);
        assert second != null;
        Button next = (Button) second.findViewById(R.id.button_next);
        next.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                incrementView();
            }
        });

        //add the animation to the AnimatedFrameLayout, which will pass it on to the appropriate child
        AnimatedFrameLayout afl = (AnimatedFrameLayout) second.findViewById(R.id.animated_frame_layout);
        afl.init(this.animations.get(0));
        afl.setOnClickListener( new OnClickListener() {
            @Override
            public void onClick(View v) {
                activity.openHelpVideos();
            }
        });

        this.views.add(second);

        ScrollView third = (ScrollView) li.inflate(R.layout.tour_prediction_screen, null);
        assert third != null;
        next = (Button) third.findViewById(R.id.button_next);
        next.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                incrementView();
            }
        });

        //add the animation to the AnimatedFrameLayout, which will pass it on to the appropriate child
        afl = (AnimatedFrameLayout) third.findViewById(R.id.animated_frame_layout);
        afl.init(this.animations.get(1));
        afl.setOnClickListener( new OnClickListener() {
            @Override
            public void onClick(View v) {
                activity.openHelpVideos();
            }
        });

        this.views.add(third);

        ScrollView fourth = (ScrollView) li.inflate(R.layout.tour_popup_screen, null);
        assert fourth != null;
        next = (Button) fourth.findViewById(R.id.button_next);
        next.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                incrementView();
            }
        });
        ImageView img = (ImageView) fourth.findViewById(R.id.imageView);
        img.setOnClickListener( new OnClickListener() {
            @Override
            public void onClick(View v) {
                activity.openHelpVideos();
            }
        });

        this.views.add(fourth);

        ScrollView fifth = (ScrollView) li.inflate(R.layout.tour_theme_screen, null);
        assert fifth != null;
        next = (Button) fifth.findViewById(R.id.button_next);
        next.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                incrementView();
            }
        });
        img = (ImageView) fifth.findViewById(R.id.imageView);
        img.setOnClickListener( new OnClickListener() {
            @Override
            public void onClick(View v) {
                activity.openHelpVideos();
            }
        });

        LinearLayout lin = (LinearLayout) fifth.findViewById(R.id.go_to_settings);
        lin.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                activity.gotoSettings(IKnowUSettings.ACTION_PERSONAL);
            }
        });

        this.views.add(fifth);

        ScrollView sixth = (ScrollView) li.inflate(R.layout.tour_quick_change_screen, null);
        assert sixth != null;
        next = (Button) sixth.findViewById(R.id.button_next);
        next.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                incrementView();
            }
        });
        img = (ImageView) sixth.findViewById(R.id.imageView);
        img.setOnClickListener( new OnClickListener() {
            @Override
            public void onClick(View v) {
                activity.openHelpVideos();
            }
        });

        this.views.add(sixth);

        ScrollView seventh = (ScrollView) li.inflate(R.layout.tour_language_screen, null);
        assert seventh != null;
        next = (Button) seventh.findViewById(R.id.button_next);
        next.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                incrementView();
            }
        });
        img = (ImageView) seventh.findViewById(R.id.imageView);
        img.setOnClickListener( new OnClickListener() {
            @Override
            public void onClick(View v) {
                activity.openHelpVideos();
            }
        });

        lin = (LinearLayout) seventh.findViewById(R.id.go_to_languages);
        lin.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                activity.goToLanguages();
            }
        });

        this.views.add(seventh);

        //TODO: add more pages to the tutorials

        this.adapter = new StartupPagerAdapter();
        this.setAdapter(this.adapter);
        if (this.currentItem <= 0 )
            this.currentItem = 0;
        this.setCurrentItem(this.currentItem);*/
    }

    private void createAnimations() {

        this.animations = new ArrayList<Animation>();

        DisplayMetrics metrics = this.context.getResources().getDisplayMetrics();

        Animation first = new Animation(true);
        AnimationStep step = new AnimationStep(50, 1500);
        step.setTranslation(DimensionConverter.stringToDimensionPixelSize("120dp", metrics), DimensionConverter.stringToDimensionPixelSize("180dp", metrics),
                DimensionConverter.stringToDimensionPixelSize("180dp", metrics), DimensionConverter.stringToDimensionPixelSize("20dp", metrics));
        first.addStep(step);
        step = new AnimationStep(50, 1500);
        step.setTranslation(DimensionConverter.stringToDimensionPixelSize("180dp", metrics), DimensionConverter.stringToDimensionPixelSize("20dp", metrics),
                DimensionConverter.stringToDimensionPixelSize("120dp", metrics), DimensionConverter.stringToDimensionPixelSize("180dp", metrics));
        first.addStep(step);

        this.animations.add(first);

        Animation second = new Animation(true);
        step = new AnimationStep(50, 700);
        step.setTranslation(DimensionConverter.stringToDimensionPixelSize("220dp", metrics), DimensionConverter.stringToDimensionPixelSize("40dp", metrics),
                DimensionConverter.stringToDimensionPixelSize("70dp", metrics), DimensionConverter.stringToDimensionPixelSize("0dp", metrics));
        second.addStep(step);
        step = new AnimationStep(50, 100);
        step.setScale(1.0f, 1.0f, 0.9f, 0.8f);
        second.addStep(step);
        step = new AnimationStep(50, 100);
        step.setScale(0.9f, 0.8f, 1.0f, 1.0f);
        second.addStep(step);

        this.animations.add(second);
    }

    public void incrementView() {
        this.stopAnimations();

        this.currentItem++;

        if (this.currentItem >= this.views.size()) this.activity.close();
        else {
            this.setCurrentItem(this.currentItem);
            this.startAnimations();
        }
    }

    public void decrementView() {
        this.stopAnimations();

        this.currentItem--;

        if (this.currentItem <= 0) this.currentItem = 0;

        this.setCurrentItem(this.currentItem);
        this.startAnimations();
    }

    private void stopAnimations() {
        AnimatedFrameLayout afl = (AnimatedFrameLayout) this.views.get(this.currentItem).findViewById(R.id.animated_frame_layout);

        if (afl != null) {
            afl.stop();
        }
    }

    private void startAnimations() {
        AnimatedFrameLayout afl = (AnimatedFrameLayout) this.views.get(this.currentItem).findViewById(R.id.animated_frame_layout);

        if (afl != null) {
            afl.start();
        }
    }

    @Override
    public void onSizeChanged(int w, int h, int oldw, int oldh) {
        super.onSizeChanged(w, h, oldw, oldh);

        AnimatedFrameLayout afl = (AnimatedFrameLayout) this.views.get(this.currentItem).findViewById(R.id.animated_frame_layout);

        if (afl != null) {
            //this.startAnimations();
        }
    }

    @Override
    public boolean onInterceptTouchEvent(MotionEvent arg0) {
        // Never allow swiping to switch between pages
        return false;
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        // Never allow swiping to switch between pages
        return false;
    }

    @Override
    public void onPageScrolled(int position, float offset, int offsetPixels) {
        super.onPageScrolled(position,offset,offsetPixels);
    }

    @Override
    public void onPageSelected(int i) {

    }

    @Override
    public void onPageScrollStateChanged(int i) {

    }

    public class StartupPagerAdapter extends PagerAdapter {

        @Override
        public int getCount() {
            return views.size();
        }

        /**
         * Create the page for the given position. The adapter is responsible
         * for adding the view to the container given here, although it only
         * must ensure this is done by the time it returns from
         * {@link #finishUpdate(android.view.ViewGroup)}.
         *
         * @param collection
         *            The containing View in which the page will be shown.
         * @param position
         *            The page position to be instantiated.
         * @return Returns an Object representing the new page. This does not
         *         need to be a View, but can be some other container of the
         *         page.
         */
        @Override
        public Object instantiateItem(ViewGroup collection, int position) {
            final ViewGroup view = views.get(position);
            collection.addView(view, 0);

            return view;
        }

        /**
         * Remove a page for the given position. The adapter is responsible for
         * removing the view from its container, although it only must ensure
         * this is done by the time it returns from
         * {@link #finishUpdate(android.view.ViewGroup)}.
         *
         * @param collection
         *            The containing View from which the page will be removed.
         * @param position
         *            The page position to be removed.
         * @param view
         *            The same object that was returned by
         *            {@link #instantiateItem(android.view.View, int)}.
         */
        @Override
        public void destroyItem(ViewGroup collection, int position, Object view) {
            collection.removeView((View)view);
        }


        /**
         * Determines whether a page View is associated with a specific key
         * object as returned by instantiateItem(ViewGroup, int). This method is
         * required for a PagerAdapter to function properly.
         *
         * @param view
         *            Page View to check for association with object
         * @param object
         *            Object to check for association with view
         * @return
         */
        @Override
        public boolean isViewFromObject(View view, Object object) {
            return (view == object);
        }

        /**
         * Called when the a change in the shown pages has been completed. At
         * this point you must ensure that all of the pages have actually been
         * added or removed from the container as appropriate.
         *
         * @param arg0
         *            The containing View which is displaying this adapter's
         *            page views.
         */
        @Override
        public void finishUpdate(ViewGroup arg0) {
        }

        @Override
        public void restoreState(Parcelable arg0, ClassLoader arg1) {
        }

        @Override
        public Parcelable saveState() {
            return null;
        }

        @Override
        public void startUpdate(ViewGroup arg0) {
        }
    }
}
