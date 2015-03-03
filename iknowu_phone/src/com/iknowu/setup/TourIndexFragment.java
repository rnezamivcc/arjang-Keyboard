package com.iknowu.setup;

import android.app.Activity;
import android.app.ListFragment;
import android.content.Context;
import android.os.Bundle;
import android.util.Log;
import android.view.InflateException;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;

import com.iknowu.IKnowUKeyboardService;
import com.iknowu.R;

import java.util.ArrayList;
import java.util.List;

/**
 * Created by Justin on 19/09/13.
 *
 */
public class TourIndexFragment extends ListFragment {

    public int currentSelectedIndex;
    public ArrayList<String> itemStrings;

    private TourListAdapter adapter;
    public ListView listView;

    public TutorialActivity activity;
    public View previousView;

    private View rootView;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        IKnowUKeyboardService.log(Log.VERBOSE, "TourIndexFragment.onCreate()", "start func");

        this.currentSelectedIndex = 0;
        this.itemStrings = new ArrayList<String>();

        this.itemStrings.add(this.getString(R.string.tour_title_home));
        this.itemStrings.add(this.getString(R.string.tour_title_reach));
        this.itemStrings.add(this.getString(R.string.tour_prediction_title));
        this.itemStrings.add(this.getString(R.string.tour_popup_title));
        this.itemStrings.add(this.getString(R.string.tour_themes_title));
        this.itemStrings.add(this.getString(R.string.tour_title_quick_change));
        this.itemStrings.add(this.getString(R.string.tour_title_language));
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState ) {
        IKnowUKeyboardService.log(Log.VERBOSE, "TourIndexFragment.onCreateView()", "start func");
        if (rootView != null) {
            ViewGroup parent = (ViewGroup) rootView.getParent();
            if (parent != null)
                parent.removeView(rootView);
        }
        try {
            rootView = inflater.inflate(R.layout.tour_index_fragment, container, false);
        } catch (InflateException e) {
        /* view is already there, just return as it is */
        }
        return rootView;
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        IKnowUKeyboardService.log(Log.VERBOSE, "TourIndexFragment.onActivityCreated()", "start func");
        super.onActivityCreated(savedInstanceState);
        this.adapter = new TourListAdapter(this.getActivity(), R.layout.tour_index_item, itemStrings);
        this.setListAdapter(this.adapter);
    }

    @Override
    public void onPause() {
        super.onPause();
    }

    @Override
    public void onListItemClick(ListView listView, View view, int position, long id) {
        super.onListItemClick(listView, view, position, id);
        IKnowUKeyboardService.log(Log.VERBOSE, "TourIndexFragment.onListItemClicked()", "position = "+position);

        //deselect the current item
        ViewGroup vg = (ViewGroup) listView.getChildAt(this.currentSelectedIndex);
        if (vg != null) {
            ImageView icon = (ImageView) vg.findViewById(R.id.tour_index_item_icon);
            icon.setImageResource(R.drawable.index_deselected_icon);

            this.currentSelectedIndex = position;

            //select the new item
            icon = (ImageView) view.findViewById(R.id.tour_index_item_icon);
            icon.setImageResource(R.drawable.index_selected_icon);

            this.activity.changeContent(this.currentSelectedIndex);
        }
    }

    public void setActivity(TutorialActivity act) {
        this.activity = act;
    }

    private class TourListAdapter extends ArrayAdapter {

        private Context context;

        public TourListAdapter(Context context, int textViewResourceId) {
            super(context, textViewResourceId);
            this.context = context;
        }

        public TourListAdapter(Context context, int resource, int textViewResourceId) {
            super(context, resource, textViewResourceId);
            this.context = context;
        }

        public TourListAdapter(Context context, int textViewResourceId, Object[] objects) {
            super(context, textViewResourceId, objects);
            this.context = context;
        }

        public TourListAdapter(Context context, int resource, int textViewResourceId, Object[] objects) {
            super(context, resource, textViewResourceId, objects);
            this.context = context;
        }

        public TourListAdapter(Context context, int textViewResourceId, List objects) {
            super(context, textViewResourceId, objects);
            this.context = context;
        }

        public TourListAdapter(Context context, int resource, int textViewResourceId, List objects) {
            super(context, resource, textViewResourceId, objects);
            this.context = context;
        }

        /*private view holder class*/
        private class ViewHolder {
            ImageView itemIcon;
            TextView itemText;
        }

        public View getView(int position, View convertView, ViewGroup parent) {

            ViewHolder holder = null;

            LayoutInflater mInflater = (LayoutInflater) context.getSystemService(Activity.LAYOUT_INFLATER_SERVICE);
            if (convertView == null) {
                convertView = mInflater.inflate(R.layout.tour_index_item, null);
                holder = new ViewHolder();
                holder.itemText = (TextView) convertView.findViewById(R.id.tour_index_item_text);
                holder.itemIcon = (ImageView) convertView.findViewById(R.id.tour_index_item_icon);
                convertView.setTag(holder);
            } else {
                holder = (ViewHolder) convertView.getTag();
            }

            holder.itemText.setText(itemStrings.get(position));

            if (position == currentSelectedIndex) {
                holder.itemIcon.setImageResource(R.drawable.index_selected_icon);
                previousView = convertView;
            } else {
                holder.itemIcon.setImageResource(R.drawable.index_deselected_icon);
            }

            return convertView;
        }
    }
}
