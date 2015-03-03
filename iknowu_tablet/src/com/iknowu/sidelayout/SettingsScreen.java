package com.iknowu.sidelayout;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.graphics.Color;
import android.graphics.Paint;
import android.net.Uri;
import android.preference.PreferenceManager;
import android.util.AttributeSet;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AbsListView;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TextView;

import com.iknowu.IKnowUKeyboardService;
import com.iknowu.KeyboardContainerView;
import com.iknowu.R;
import com.iknowu.dictionarymanager.Analyzer;
import com.iknowu.downloader.DownloadActivity;
import com.iknowu.preferences.IKnowUSettings;
import com.iknowu.setup.TutorialActivity;
import com.iknowu.util.Theme;

import java.util.ArrayList;

public class SettingsScreen extends SideScreen {

    private static final int COLUMN_WIDTH = 150;

	private boolean setupComplete;
	//private ScrollView myView;
	private Context context;
	private int width;
    private ListView listView;
    private ArrayList<MenuItemLayout> views;

    private Paint paint;

    public SettingsScreen(Context context) {
        super(context);
        this.context = context;
        this.paint = new Paint();
    }

    public SettingsScreen(Context context, AttributeSet attrs) {
        super(context, attrs);
        this.context = context;
        this.paint = new Paint();
    }

    public SettingsScreen(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        this.context = context;
        this.paint = new Paint();
    }

    /*@Override
    public void onDraw(Canvas canvas) {
        super.onDraw(canvas);

        this.paint.setColor(Color.GREEN);
        this.paint.setStyle(Paint.Style.STROKE);
        this.paint.setStrokeWidth(5);

        canvas.drawRect(0, 0, this.getWidth(), this.getHeight(), paint);
    }*/

    @Override
	public void init(IKnowUKeyboardService serv, KeyboardContainerView kbcv, SideRelativeLayout sfl, boolean islefty, int index) {
		//this.ikScroller = scroll;

        super.init(serv, kbcv, sfl, islefty, index);

        this.tab = new SideTab(this.context);
        this.tab.init(this);
        this.tab.setImageResource(this.isLefty ? R.drawable.tab_menu_lefty : R.drawable.tab_menu);
        LayoutParams params = new LayoutParams(LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT);
        this.tab.setLayoutParams(params);

        //this.setTabClickListeners();

        this.listView = new ListView(this.context);
        /*this.listView.setColumnWidth(COLUMN_WIDTH);
        this.listView.setNumColumns(3);
        this.listView.setVerticalSpacing(5);
        this.listView.setHorizontalSpacing(5);
        this.listView.setStretchMode(GridView.STRETCH_COLUMN_WIDTH);*/
        this.listView.setBackgroundColor(Theme.SIDE_BACKGROUND_COLOR);
        params = new LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT);
        this.listView.setLayoutParams(params);
        //this.listView.setGravity(Gravity.CENTER);

        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(this.context);
        this.setupComplete = sp.getBoolean(IKnowUKeyboardService.PREF_SETUP_COMPLETE, false);

        if (this.isLefty) {
            this.initLefty();
        } else {
            this.initRighty();
        }
		
		this.createScreens();

        this.contentView = this.listView;

        disableEnableScreen(this.isSelected);
	}

    @Override
    public void disableEnableScreen(boolean enable) {
        super.disableEnableScreen(enable);
        this.contentView.setEnabled(enable);
        this.contentView.setVisibility((enable) ? View.VISIBLE : View.INVISIBLE);
    }

    private void initRighty() {
        this.addView(this.tab);
        this.addView(this.listView);
    }

    private void initLefty() {
        this.addView(this.listView);
        this.addView(this.tab);
    }

    private void createScreens() {

        this.views = new ArrayList<MenuItemLayout>();

        //help item
        MenuItemLayout help = new MenuItemLayout(this.context);
        help.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                launchHelp();
            }
        });
        help.setImage(R.drawable.iknowu_help_icon);
        String text = this.context.getResources().getString(R.string.label_help);
        help.setText(text);
        this.views.add(help);

        //settings item
        MenuItemLayout settings = new MenuItemLayout(this.context);
        settings.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                launchSettings();
            }
        });
        settings.setImage(R.drawable.iknowu_settings_icon);
        text = this.context.getResources().getString(R.string.label_settings);
        settings.setText(text);
        this.views.add(settings);

        //downloader item
        MenuItemLayout download = new MenuItemLayout(this.context);
        download.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                launchDownloader();
            }
        });
        download.setImage(R.drawable.iknowu_download_icon);
        text = this.context.getResources().getString(R.string.label_download);
        download.setText(text);
        this.views.add(download);

        //downloader item
        MenuItemLayout manager = new MenuItemLayout(this.context);
        manager.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                launchDictionaryManager();
            }
        });
        manager.setImage(R.drawable.iknowu_dict_manage_icon);
        text = this.context.getResources().getString(R.string.dict_manager_title);
        manager.setText(text);
        this.views.add(manager);

        //downloader item
        MenuItemLayout website = new MenuItemLayout(this.context);
        website.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                launchWebsite();
            }
        });
        website.setImage(R.drawable.iknowulogo);
        text = this.context.getResources().getString(R.string.website);
        website.setText(text);
        this.views.add(website);

        MenuAdapter adapter = new MenuAdapter(this.context);
        this.listView.setAdapter(adapter);
    }

    /**
     * Handle open/close state of settings screen.
     * @param isOpen the state flag
     */
    @Override
    public void setOpenState(boolean isOpen) {
        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(this.context);
        SharedPreferences.Editor edit = sp.edit();
        edit.putString(SideRelativeLayout.OPEN_SIDE_SCREEN, (isOpen) ? SideRelativeLayout.SETTINGS_SCREEN : "");
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
	
	private void launchSetup() {
        if (this.sideRelativeLayout.lastSelected.equals(this)) {
            //handleClose();
            Intent intent = new Intent();
            intent.setClass(this.context, TutorialActivity.class);
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            this.context.startActivity(intent);
        }
	}

	/*
	 * Launch a new intent that will bring us to the website
	 */
	private void launchWebsite() {
        if (this.sideRelativeLayout.lastSelected.equals(this)) {
            //handleClose();
            String url = "http://www.iknowu.net";
            Intent intent = new Intent(Intent.ACTION_VIEW);
            intent.setData(Uri.parse(url));
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            this.context.startActivity(intent);
        }
	}
	
	/*
	 * launch the settings activity
	 */
	private void launchSettings() {
        if (this.sideRelativeLayout.lastSelected.equals(this)) {
            if (this.setupComplete) {
                //handleClose();
                Intent intent = new Intent();
                intent.setClass(this.context, IKnowUSettings.class);
                intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                this.context.startActivity(intent);
            } else {
                launchSetup();
            }
        }
	}
	
	/*
	 * launch the dictionary manager activity
	 */
	private void launchDictionaryManager() {
        if (this.sideRelativeLayout.lastSelected.equals(this)) {
            //handleClose();
            Intent intent = new Intent();
            intent.setClass(this.context, Analyzer.class);
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            this.context.startActivity(intent);
        }
	}
	
	public void launchAnalyzer() {
        if (this.sideRelativeLayout.lastSelected.equals(this)) {
            //handleClose();
            Intent intent = new Intent();
            intent.setClass(this.context, Analyzer.class);
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            this.context.startActivity(intent);
        }
	}
	
	public void launchDownloader() {
        if (this.sideRelativeLayout.lastSelected.equals(this)) {
            //handleClose();
            Intent intent = new Intent();
            intent.setClass(this.context, DownloadActivity.class);
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            this.context.startActivity(intent);
        }
	}
	
	/*
	 * show the help menu screen
	 */
	private void launchHelp() {
        if (this.sideRelativeLayout.lastSelected.equals(this)) {
            Intent intent = new Intent();
            intent.setClass(this.context, TutorialActivity.class);
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            this.context.startActivity(intent);
        }
	}

    private class MenuItemLayout extends LinearLayout {

        private static final int IMAGE_WIDTH = 35;
        private static final int IMAGE_HEIGHT = 35;

        private static final int TEXT_COLOR = 0xFF1E87D2;

        ImageView image;
        TextView text;

        public MenuItemLayout(Context context) {
            super(context);
            init();
        }

        public MenuItemLayout(Context context, AttributeSet attrs) {
            super(context, attrs);
            init();
        }

        public MenuItemLayout(Context context, AttributeSet attrs, int defStyle) {
            super(context, attrs, defStyle);
            init();
        }

        private void init() {
            AbsListView.LayoutParams vparams = new AbsListView.LayoutParams(AbsListView.LayoutParams.MATCH_PARENT, AbsListView.LayoutParams.WRAP_CONTENT);
            this.setLayoutParams(vparams);
            this.setBackgroundResource(R.drawable.menu_state_button);
            if (keyboardService.mLandscape) this.setPadding(50,10,10,10);
            else this.setPadding(10,10,10,10);
            //this.setGravity(Gravity.CENTER);
            this.setOrientation(HORIZONTAL);

            LayoutParams params = new LayoutParams(50, 50);
            params.gravity = Gravity.CENTER;
            //params.bottomMargin = 10;
            params.rightMargin = 10;
            image = new ImageView(context);
            image.setLayoutParams(params);

            this.addView(image);

            params = new LayoutParams(LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT);
            params.gravity = Gravity.CENTER;
            text = new TextView(context);
            text.setGravity(Gravity.CENTER);
            text.setLayoutParams(params);
            text.setTextColor(Color.WHITE);
            text.setTextSize(20);

            this.addView(text);
        }

        public void setHeight(int height) {
            AbsListView.LayoutParams vparams = new AbsListView.LayoutParams(AbsListView.LayoutParams.MATCH_PARENT, height);
            this.setLayoutParams(vparams);
        }

        public void setImage(int resId) {
            this.image.setImageResource(resId);
        }

        public void setText(String text) {
            this.text.setText(text);
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
            MenuItemLayout menuItem;
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
