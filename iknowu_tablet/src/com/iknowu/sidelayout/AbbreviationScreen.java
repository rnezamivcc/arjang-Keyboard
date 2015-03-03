package com.iknowu.sidelayout;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.graphics.Color;
import android.preference.PreferenceManager;
import android.util.AttributeSet;
import android.util.Xml;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AbsListView;
import android.widget.BaseAdapter;
import android.widget.GridView;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.iknowu.IKnowUKeyboardService;
import com.iknowu.KeyboardContainerView;
import com.iknowu.R;
import com.iknowu.util.Theme;

import org.xmlpull.v1.XmlPullParser;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.util.ArrayList;

/**
 * Created by Justin on 20/11/13.
 *
 */
public class AbbreviationScreen extends SideScreen implements View.OnClickListener, View.OnLongClickListener {

    private static final int COLUMN_WIDTH = 150;

    public static final int ADD_DELETE_ID = 45678;

    private Context context;
    private int width;
    private GridView gridView;
    private ArrayList<AbbreviationItemLayout> views;

    public AbbreviationScreen(Context ctx) {
        super(ctx);
        this.context = ctx;
    }

    public AbbreviationScreen(Context context, AttributeSet attrs) {
        super(context, attrs);
        this.context = context;
    }

    public AbbreviationScreen(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        this.context = context;
    }

    @Override
    public void init(IKnowUKeyboardService serv, KeyboardContainerView kbcv, SideRelativeLayout sfl, boolean islefty, int index) {
        //this.ikScroller = scroll;

        super.init(serv, kbcv, sfl, islefty, index);

        this.tab = new SideTab(this.context);
        this.tab.init(this);
        this.tab.setImageResource(this.isLefty ? R.drawable.tab_abbrev_lefty : R.drawable.tab_abbrev);
        LayoutParams params = new LayoutParams(LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT);
        this.tab.setLayoutParams(params);

        //this.setTabClickListeners();

        this.gridView = new GridView(this.context);
        this.gridView.setColumnWidth(COLUMN_WIDTH);
        this.gridView.setNumColumns(4);
        this.gridView.setVerticalSpacing(5);
        this.gridView.setHorizontalSpacing(5);
        this.gridView.setStretchMode(GridView.STRETCH_COLUMN_WIDTH);
        this.gridView.setBackgroundColor(Theme.SIDE_BACKGROUND_COLOR);
        params = new LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT);
        this.gridView.setLayoutParams(params);
        this.gridView.setPadding(5,5,5,5);
        //this.gridView.setGravity(Gravity.CENTER);

        if (this.isLefty) {
            this.initLefty();
        } else {
            this.initRighty();
        }

        this.createItems();

        this.contentView = this.gridView;

        disableEnableScreen(this.isSelected);
    }

    @Override
    public void disableEnableScreen(boolean enable) {
        super.disableEnableScreen(enable);
        this.gridView.setEnabled(enable);
        if (this.gridView.getChildCount() == 0) {
            // If the abbreviation buttons are not attached to grid yet.
            for (AbbreviationItemLayout abbr : this.views) {
                abbr.setEnabled(enable);
            }
        } else {
            disableEnableChildControls(enable, this.gridView);
        }
        this.gridView.setVisibility((enable) ? View.VISIBLE : View.INVISIBLE);
    }

    private void initRighty() {
        this.addView(this.tab);
        this.addView(this.gridView);
    }

    private void initLefty() {
        this.addView(this.gridView);
        this.addView(this.tab);
    }

    private void createItems() {
        try {

            this.views = new ArrayList<AbbreviationItemLayout>();

            File directory = this.context.getFilesDir();
            //File directory = Environment.getExternalStorageDirectory();
            File abbrevsFile = new File(directory, "wordlogic/dictionary/abbreviations.xml");

            AbbreviationItemLayout abbrev = new AbbreviationItemLayout(this.context);
            abbrev.index = 0;
            abbrev.shortText = "";
            abbrev.longText = "";
            //abbrev.setText(abbrev.shortText);
            abbrev.setImage(R.drawable.add);
            abbrev.setId(ADD_DELETE_ID);
            abbrev.setColor(AbbreviationItemLayout.COLOR_GREEN);

            abbrev.setOnClickListener(this);

            this.views.add(abbrev);

            int count = 1;

            if (abbrevsFile.exists()) {
                FileInputStream in = new FileInputStream(abbrevsFile);
                XmlPullParser xmp = Xml.newPullParser();
                xmp.setInput(in, null);
                int event;

                while ((event = xmp.next()) != XmlPullParser.END_DOCUMENT) {
                    if (event == XmlPullParser.START_TAG) {
                        String tag = xmp.getName();
                        if (tag.equals("abbreviation")) {
                            abbrev = new AbbreviationItemLayout(this.context);
                            abbrev.index = count;
                            abbrev.shortText = xmp.getAttributeValue(null, "short");
                            IKnowUKeyboardService.addWord(abbrev.shortText);
                            abbrev.longText = xmp.getAttributeValue(null, "long");
                            abbrev.setText(abbrev.shortText);

                            if (xmp.getAttributeValue(null, "color") != null) {
                                abbrev.setColor( Integer.parseInt( xmp.getAttributeValue(null, "color") ) );
                            }

                            abbrev.setOnClickListener(this);
                            abbrev.setOnLongClickListener(this);

                            this.views.add(abbrev);

                            count++;
                        }
                    } else if (event == XmlPullParser.END_TAG) {}
                }
            }

        } catch (FileNotFoundException fnfe) {
            //do nothing, their phone's storage has been cut off for some reason
            //Toast.makeText(this, this.getResources().getString(R.string.file_not_found) + "code = " + 1002, Toast.LENGTH_LONG).show();
        } catch (Exception e) {
            IKnowUKeyboardService.sendErrorMessage(e);
        }

        MenuAdapter adapter = new MenuAdapter(this.context);
        this.gridView.setAdapter(adapter);
    }

    /**
     * Handle open/close state of voice screen.
     * @param isOpen the state flag
     */
    @Override
    public void setOpenState(boolean isOpen) {
        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(this.context);
        SharedPreferences.Editor edit = sp.edit();
        edit.putString(SideRelativeLayout.OPEN_SIDE_SCREEN, (isOpen) ? SideRelativeLayout.ABBREVIATION_SCREEN : "");
        edit.commit();
    }

    @Override
    public void onMeasure( int widthMeasureSpec, int heightMeasureSpec) {
        super.onMeasure(widthMeasureSpec, heightMeasureSpec);

        /*for (AbbreviationItemLayout item : this.views) {
            int height = this.getMeasuredHeight() / 2;
            item.setHeight(height);
        }*/
    }

    @Override
    public void onClick(View v) {
        //if clicking on the add/deltete item, then start the add/delete activity
        if(v.getId() == ADD_DELETE_ID) {
            if (this.sideRelativeLayout.lastSelected.equals(this)) {
                Intent intent = new Intent();
                intent.setClass(this.context, AbbreviationActivity.class);
                intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                this.context.startActivity(intent);
            }
        } else {
            AbbreviationItemLayout abbr = (AbbreviationItemLayout) v;
            this.keyboardService.sendTextToEditor(abbr.shortText+" ",0,0, false);
        }
    }

    @Override
    public boolean onLongClick(View v) {
        AbbreviationItemLayout abbr = (AbbreviationItemLayout) v;
        this.keyboardService.sendTextToEditor(abbr.longText+" ",0,0, false);

        return true;
    }

    private class AbbreviationItemLayout extends LinearLayout {

        private static final int IMAGE_WIDTH = 35;
        private static final int IMAGE_HEIGHT = 35;

        private static final int ITEM_HEIGHT = 75;

        private static final int TEXT_COLOR = 0xFF1E87D2;

        private static final int COLOR_BLUE = 0;
        private static final int COLOR_GREEN = 1;
        private static final int COLOR_ORANGE = 2;
        private static final int COLOR_PINK = 3;
        private static final int COLOR_RED = 4;
        private static final int COLOR_YELLOW = 5;

        public int index;
        public String shortText;
        public String longText;

        public TextView text;
        public ImageView image;

        public AbbreviationItemLayout(Context context) {
            super(context);
            init();
        }

        public AbbreviationItemLayout(Context context, AttributeSet attrs) {
            super(context, attrs);
            init();
        }

        public AbbreviationItemLayout(Context context, AttributeSet attrs, int defStyle) {
            super(context, attrs, defStyle);
            init();
        }

        private void init() {
            AbsListView.LayoutParams vparams = new AbsListView.LayoutParams(AbsListView.LayoutParams.MATCH_PARENT, ITEM_HEIGHT);
            this.setLayoutParams(vparams);
            this.setColor(COLOR_YELLOW);
            //this.setPadding(5,5,5,5);
            this.setGravity(Gravity.CENTER);
            this.setOrientation(HORIZONTAL);

            LayoutParams params = new LayoutParams(LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT);
            params.gravity = Gravity.CENTER;
            text = new TextView(context);
            //text.setGravity(Gravity.CENTER);
            text.setLayoutParams(params);
            text.setTextColor(Color.BLACK);
            text.setTextSize(20);

            this.addView(text);

            params = new LayoutParams(LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT);
            params.gravity = Gravity.CENTER;
            image = new ImageView(context);
            image.setLayoutParams(params);

            this.addView(this.image);
        }

        public void setHeight(int height) {
            AbsListView.LayoutParams vparams = new AbsListView.LayoutParams(AbsListView.LayoutParams.MATCH_PARENT, height);
            this.setLayoutParams(vparams);
        }

        public void setColor(int color) {
            switch (color) {
                case COLOR_BLUE:
                    this.setBackgroundResource(R.drawable.abbrev_bg_blue);
                    break;
                case COLOR_GREEN:
                    this.setBackgroundResource(R.drawable.abbrev_bg_green);
                    break;
                case COLOR_ORANGE:
                    this.setBackgroundResource(R.drawable.abbrev_bg_orange);
                    break;
                case COLOR_PINK:
                    this.setBackgroundResource(R.drawable.abbrev_bg_pink);
                    break;
                case COLOR_RED:
                    this.setBackgroundResource(R.drawable.abbrev_bg_red);
                    break;
                case COLOR_YELLOW:
                    this.setBackgroundResource(R.drawable.abbrev_bg_yellow);
                    break;
                default:
                    this.setBackgroundResource(R.drawable.abbrev_bg_yellow);
                    break;
            }
        }

        public void setText(String text) {
            this.text.setText(text);
            this.image.setVisibility(View.GONE);
        }

        public void setImage(int resId) {
            this.image.setImageResource(resId);
            this.text.setVisibility(View.GONE);
        }
    }

    public class MenuAdapter extends BaseAdapter {
        private Context mContext;

        public MenuAdapter(Context c) {
            mContext = c;
        }

        public int getCount() {
            return views.size();
        }

        public Object getItem(int position) {
            return views.get(position);
        }

        public long getItemId(int position) {
            return 0;
        }

        // create a new ImageView for each item referenced by the Adapter
        public View getView(int position, View convertView, ViewGroup parent) {
            AbbreviationItemLayout menuItem;
            //IKnowUKeyboardService.log(Log.VERBOSE, "SettingsMenuAdapter.getView()", "position = "+position+", convertView = "+convertView);
            //if (convertView == null) {  // if it's recycled
            menuItem = views.get(position);
            //} else {
            //    menuItem = (AbbreviationItemLayout) convertView;
            //}

            return menuItem;
        }
    }
}
