package com.iknowu.setup;

import android.app.Activity;
import android.app.Fragment;
import android.os.Bundle;
import android.util.Log;
import android.view.InflateException;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ScrollView;

import com.iknowu.IKnowUKeyboardService;
import com.iknowu.R;
import com.iknowu.animation.AnimatedFrameLayout;
import com.iknowu.preferences.IKnowUSettings;

import java.util.ArrayList;

/**
 * Created by Justin on 19/09/13.
 *
 */
public class TourContentFragment extends Fragment {

    public int currentSelectedIndex;
    public ArrayList<ViewGroup> views;

    public FrameLayout rootLayout;

    public TutorialActivity activity;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        IKnowUKeyboardService.log(Log.VERBOSE, "TourIndexFragment.onCreate()", "start func");

        this.currentSelectedIndex = 0;
        this.views = new ArrayList<ViewGroup>();

        LayoutInflater li = (LayoutInflater) this.getActivity().getSystemService(Activity.LAYOUT_INFLATER_SERVICE);

        ScrollView first = (ScrollView) li.inflate(R.layout.thanks_for_choosing, null);
        assert first != null;
//        Button takeTour = (Button) first.findViewById(R.id.button_take_tour);
//        takeTour.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View v) {
//                //incrementView();
//            }
//        });

        Button skip = (Button) first.findViewById(R.id.button_skip);
        skip.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                activity.close();
            }
        });

        this.views.add(first);

        ScrollView second = (ScrollView) li.inflate(R.layout.tour_reach_screen, null);
        assert second != null;
//        Button next = (Button) second.findViewById(R.id.button_next);
//        next.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View v) {
//                //incrementView();
//            }
//        });

        //add the animation to the AnimatedFrameLayout, which will pass it on to the appropriate child
        AnimatedFrameLayout afl = (AnimatedFrameLayout) second.findViewById(R.id.animated_frame_layout);
        //afl.init(this.animations.get(0));
        afl.setOnClickListener( new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                activity.openHelpVideos();
            }
        });

        this.views.add(second);

        ScrollView third = (ScrollView) li.inflate(R.layout.tour_prediction_screen, null);
        assert third != null;
//        next = (Button) third.findViewById(R.id.button_next);
//        next.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View v) {
//                //incrementView();
//            }
//        });

        //add the animation to the AnimatedFrameLayout, which will pass it on to the appropriate child
        afl = (AnimatedFrameLayout) third.findViewById(R.id.animated_frame_layout);
        //afl.init(this.animations.get(1));
        afl.setOnClickListener( new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                activity.openHelpVideos();
            }
        });

        this.views.add(third);

        ScrollView fourth = (ScrollView) li.inflate(R.layout.tour_popup_screen, null);
        assert fourth != null;
//        next = (Button) fourth.findViewById(R.id.button_next);
//        next.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View v) {
//                //incrementView();
//            }
//        });
        ImageView img = (ImageView) fourth.findViewById(R.id.imageView);
        img.setOnClickListener( new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                activity.openHelpVideos();
            }
        });

        this.views.add(fourth);

        ScrollView fifth = (ScrollView) li.inflate(R.layout.tour_theme_screen, null);
        assert fifth != null;
//        next = (Button) fifth.findViewById(R.id.button_next);
//        next.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View v) {
//                //incrementView();
//            }
//        });
        img = (ImageView) fifth.findViewById(R.id.imageView);
        img.setOnClickListener( new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                activity.openHelpVideos();
            }
        });

        LinearLayout lin = (LinearLayout) fifth.findViewById(R.id.go_to_settings);
        lin.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                activity.gotoSettings(IKnowUSettings.ACTION_PERSONAL);
            }
        });

        this.views.add(fifth);

        ScrollView sixth = (ScrollView) li.inflate(R.layout.tour_quick_change_screen, null);
        assert sixth != null;
//        next = (Button) sixth.findViewById(R.id.button_next);
//        next.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View v) {
//                //incrementView();
//            }
//        });
        img = (ImageView) sixth.findViewById(R.id.imageView);
        img.setOnClickListener( new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                activity.openHelpVideos();
            }
        });

        this.views.add(sixth);

        ScrollView seventh = (ScrollView) li.inflate(R.layout.tour_language_screen, null);
        assert seventh != null;
//        next = (Button) seventh.findViewById(R.id.button_next);
//        next.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View v) {
//                //incrementView();
//            }
//        });
        img = (ImageView) seventh.findViewById(R.id.imageView);
        img.setOnClickListener( new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                activity.openHelpVideos();
            }
        });

        lin = (LinearLayout) seventh.findViewById(R.id.go_to_languages);
        lin.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                activity.goToLanguages();
            }
        });

        this.views.add(seventh);

    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState ) {
        //IKnowUKeyboardService.log(Log.VERBOSE, "TourIndexFragment.onCreateView()", "start func");
        if (rootLayout != null) {
            ViewGroup parent = (ViewGroup) rootLayout.getParent();
            if (parent != null)
                parent.removeView(rootLayout);
        }
        try {
            rootLayout = new FrameLayout(this.getActivity());
            this.rootLayout.addView(this.views.get( 0 ));
        } catch (InflateException e) {
        /* view is already there, just return as it is */
        }

        return this.rootLayout;
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        //IKnowUKeyboardService.log(Log.VERBOSE, "TourIndexFragment.onActivityCreated()", "start func");
        super.onActivityCreated(savedInstanceState);
    }

    @Override
    public void onPause() {
        super.onPause();
    }

    public void setActivity(TutorialActivity act) {
        this.activity = act;
    }

    public void changeContent(int pos) {
        this.rootLayout.removeAllViews();
        this.rootLayout.addView(this.views.get(pos));
    }
}
