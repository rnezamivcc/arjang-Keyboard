package com.iknowu.sidelayout;

import android.app.Activity;
import android.content.Context;
import android.content.SharedPreferences;
import android.graphics.Color;
import android.os.Bundle;
import android.util.AttributeSet;
import android.util.Xml;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AbsListView;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.GridView;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import android.widget.TextView;
import android.widget.Toast;

import com.iknowu.IKnowUKeyboardService;
import com.iknowu.R;
import com.iknowu.util.Size;
import com.iknowu.util.Theme;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlSerializer;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileWriter;
import java.io.StringWriter;
import java.util.ArrayList;

/**
 * Created by Justin on 21/11/13.
 *
 */
public class AbbreviationActivity extends Activity implements View.OnClickListener {

    private static final int COLUMN_WIDTH = 150;

    public static final int ADD_BUTTON_ID = 45678;
    public static final int DELETE_BUTTON_ID = 45678;
    public static final int COLOR_PICKER_BLUE_ID = 10;
    public static final int COLOR_PICKER_GREEN_ID = 11;
    public static final int COLOR_PICKER_ORANGE_ID = 12;
    public static final int COLOR_PICKER_PINK_ID = 13;
    public static final int COLOR_PICKER_RED_ID = 14;
    public static final int COLOR_PICKER_YELLOW_ID = 15;

    private int width;

    private ScrollView scrollView;
    private LinearLayout contentView;
    private LinearLayout addView;
    private EditText shortText;
    private EditText longText;
    private LinearLayout colorPicker;
    private Button addButton;

    private GridView gridView;
    private ArrayList<AbbreviationItemLayout> views;

    private boolean isLandscape;
    private int currentColorSelected;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

    }

    @Override
    public void onResume() {
        super.onResume();

        this.init();
    }

    @Override
    public void onPause() {
        this.writeXML();
        super.onPause();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
    }

    public void init() {

        this.checkLandscape();

        if (this.isLandscape) {
            this.initLandscape();
        } else {
            this.initPortrait();
        }

        this.createItems();

        this.contentView.addView(this.gridView);

        this.setContentView(this.contentView);
    }

    private void checkLandscape() {

        int screenWidth = Size.getScreenWidth(this);
        int screenHeight = Size.getScreenHeight(this);

        this.isLandscape = screenWidth > screenHeight;

    }

    private void initLandscape() {

        this.contentView = new LinearLayout(this);
        this.contentView.setOrientation(LinearLayout.HORIZONTAL);
        this.contentView.setLayoutParams(new ViewGroup.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT));

        this.scrollView = new ScrollView(this);
        this.scrollView.setLayoutParams(new LinearLayout.LayoutParams(0, ViewGroup.LayoutParams.MATCH_PARENT, 0.6f));
        this.scrollView.setPadding(10,10,10,10);
        this.scrollView.setBackgroundColor(0xFFcfcfcf);

        this.addView = new LinearLayout(this);
        this.addView.setOrientation(LinearLayout.VERTICAL);
        LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT);
        params.gravity = Gravity.CENTER;
        this.addView.setGravity(Gravity.CENTER_VERTICAL);
        this.addView.setLayoutParams(params);
        this.addView.setPadding(20,20,20,20);
        this.addView.setBackgroundResource(R.drawable.bg_square_grey);


        TextView stv = new TextView(this);
        stv.setTextSize(25);
        stv.setText("Abbreviated:");
        stv.setTextColor(0xFF000000);
        this.addView.addView(stv);

        this.shortText = new EditText(this);
        this.shortText.setLayoutParams(new LinearLayout.LayoutParams(200, ViewGroup.LayoutParams.WRAP_CONTENT));
        this.shortText.setTextSize(25);
        this.addView.addView(this.shortText);

        TextView ltv = new TextView(this);
        ltv.setTextSize(25);
        ltv.setText("Full Text:");
        ltv.setTextColor(0xFF000000);
        this.addView.addView(ltv);

        this.longText = new EditText(this);
        params = new LinearLayout.LayoutParams(500, 200);
        params.bottomMargin = 10;
        this.longText.setLayoutParams(params);
        this.longText.setTextSize(25);
        this.longText.setGravity(Gravity.TOP|Gravity.LEFT);
        this.addView.addView(this.longText);

        TextView ctv = new TextView(this);
        ctv.setTextSize(25);
        ctv.setText("Select Color:");
        ctv.setTextColor(0xFF000000);
        this.addView.addView(ctv);

        this.createColorPicker();
        this.addView.addView(this.colorPicker);

        this.addButton = new Button(this);
        params = new LinearLayout.LayoutParams(200, ViewGroup.LayoutParams.WRAP_CONTENT);
        params.gravity = Gravity.RIGHT;
        this.addButton.setLayoutParams(params);
        this.addButton.setText("Add");
        this.addButton.setTextSize(25);
        this.addButton.setTextColor(0xFFFFFFFF);
        this.addButton.setId(ADD_BUTTON_ID);
        this.addButton.setOnClickListener(this);
        this.addButton.setBackgroundResource(R.drawable.green_state_button);
        this.addView.addView(this.addButton);

        this.scrollView.addView(this.addView);

        this.contentView.addView(this.scrollView);

        this.gridView = new GridView(this);
        this.gridView.setColumnWidth(COLUMN_WIDTH);
        this.gridView.setNumColumns(4);
        this.gridView.setVerticalSpacing(5);
        this.gridView.setHorizontalSpacing(5);
        this.gridView.setStretchMode(GridView.STRETCH_COLUMN_WIDTH);
        this.gridView.setBackgroundColor(0xFFafafaf);
        params = new LinearLayout.LayoutParams(0, ViewGroup.LayoutParams.MATCH_PARENT, 0.4f);
        this.gridView.setLayoutParams(params);
        this.gridView.setPadding(5,5,5,5);
        //this.gridView.setGravity(Gravity.CENTER);
    }

    /**
     * create the portrait layout
     */
    private void initPortrait() {
        this.contentView = new LinearLayout(this);
        this.contentView.setOrientation(LinearLayout.VERTICAL);
        this.contentView.setLayoutParams(new ViewGroup.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT));

        this.scrollView = new ScrollView(this);
        this.scrollView.setLayoutParams(new LinearLayout.LayoutParams( ViewGroup.LayoutParams.MATCH_PARENT, 0, 0.5f ));
        this.scrollView.setPadding(10,10,10,10);
        this.scrollView.setBackgroundColor(0xFFcfcfcf);

        this.addView = new LinearLayout(this);
        this.addView.setOrientation(LinearLayout.VERTICAL);
        LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT);
        params.gravity = Gravity.CENTER;
        this.addView.setGravity(Gravity.CENTER_VERTICAL);
        this.addView.setLayoutParams(params);
        this.addView.setPadding(20, 20, 20, 20);
        this.addView.setBackgroundResource(R.drawable.bg_square_grey);


        TextView stv = new TextView(this);
        stv.setTextSize(25);
        stv.setText("Abbreviation:");
        stv.setTextColor(0xFF000000);
        this.addView.addView(stv);

        this.shortText = new EditText(this);
        this.shortText.setLayoutParams(new LinearLayout.LayoutParams(100, ViewGroup.LayoutParams.WRAP_CONTENT));
        this.shortText.setTextSize(25);
        this.addView.addView(this.shortText);

        TextView ltv = new TextView(this);
        ltv.setTextSize(25);
        ltv.setText("Expanded Text:");
        ltv.setTextColor(0xFF000000);
        this.addView.addView(ltv);

        this.longText = new EditText(this);
        params = new LinearLayout.LayoutParams(500, 200);
        params.bottomMargin = 10;
        this.longText.setLayoutParams(params);
        this.longText.setTextSize(25);
        this.longText.setGravity(Gravity.TOP|Gravity.LEFT);
        this.addView.addView(this.longText);

        TextView ctv = new TextView(this);
        ctv.setTextSize(25);
        ctv.setText("Select Color:");
        ctv.setTextColor(0xFF000000);
        this.addView.addView(ctv);

        this.createColorPicker();
        this.addView.addView(this.colorPicker);

        this.addButton = new Button(this);
        params = new LinearLayout.LayoutParams(200, ViewGroup.LayoutParams.WRAP_CONTENT);
        params.gravity = Gravity.LEFT;
        this.addButton.setLayoutParams(params);
        this.addButton.setText("Add");
        this.addButton.setTextSize(25);
        this.addButton.setTextColor(0xFFFFFFFF);
        this.addButton.setId(ADD_BUTTON_ID);
        this.addButton.setOnClickListener(this);
        this.addButton.setBackgroundResource(R.drawable.green_state_button);
        this.addView.addView(this.addButton);

        this.scrollView.addView(this.addView);

        this.contentView.addView(this.scrollView);

        this.gridView = new GridView(this);
        this.gridView.setColumnWidth(COLUMN_WIDTH);
        this.gridView.setNumColumns(4);
        this.gridView.setVerticalSpacing(5);
        this.gridView.setHorizontalSpacing(5);
        this.gridView.setStretchMode(GridView.STRETCH_COLUMN_WIDTH);
        this.gridView.setBackgroundColor(0xFFafafaf);
        params = new LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, 0, 0.5f);
        this.gridView.setLayoutParams(params);
        this.gridView.setPadding(5,5,5,5);
        //this.gridView.setGravity(Gravity.CENTER);
    }

    private void createColorPicker() {

        this.currentColorSelected = 0;

        this.colorPicker = new LinearLayout(this);
        this.colorPicker.setOrientation(LinearLayout.HORIZONTAL);
        LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT);
        params.bottomMargin = 10;
        this.colorPicker.setLayoutParams(params);

        ImageView img = new ImageView(this);
        img.setBackgroundResource(R.drawable.color_picker_blue);
        img.setId(COLOR_PICKER_BLUE_ID);
        img.setOnClickListener(this);
        this.colorPicker.addView(img);

        img = new ImageView(this);
        img.setBackgroundResource(R.drawable.color_picker_green);
        img.setId(COLOR_PICKER_GREEN_ID);
        img.setOnClickListener(this);
        this.colorPicker.addView(img);

        img = new ImageView(this);
        img.setBackgroundResource(R.drawable.color_picker_orange);
        img.setId(COLOR_PICKER_ORANGE_ID);
        img.setOnClickListener(this);
        this.colorPicker.addView(img);

        img = new ImageView(this);
        img.setBackgroundResource(R.drawable.color_picker_pink);
        img.setId(COLOR_PICKER_PINK_ID);
        img.setOnClickListener(this);
        this.colorPicker.addView(img);

        img = new ImageView(this);
        img.setBackgroundResource(R.drawable.color_picker_red);
        img.setId(COLOR_PICKER_RED_ID);
        img.setOnClickListener(this);
        this.colorPicker.addView(img);

        img = new ImageView(this);
        img.setBackgroundResource(R.drawable.color_picker_yellow);
        img.setId(COLOR_PICKER_YELLOW_ID);
        img.setOnClickListener(this);
        this.colorPicker.addView(img);
    }

    private void createItems() {
        try {

            this.views = new ArrayList<AbbreviationItemLayout>();

            File directory = this.getFilesDir();
            //File directory = Environment.getExternalStorageDirectory();
            File abbrevsFile = new File(directory, "wordlogic/dictionary/abbreviations.xml");

            if (abbrevsFile.exists()) {
                FileInputStream in = new FileInputStream(abbrevsFile);
                XmlPullParser xmp = Xml.newPullParser();
                xmp.setInput(in, null);
                int event;
                int count = 0;
                while ((event = xmp.next()) != XmlPullParser.END_DOCUMENT) {
                    if (event == XmlPullParser.START_TAG) {
                        String tag = xmp.getName();
                        if (tag.equals("abbreviation")) {
                            AbbreviationItemLayout abbrev = new AbbreviationItemLayout(this);
                            abbrev.index = count;
                            abbrev.shortText = xmp.getAttributeValue(null, "short");
                            abbrev.longText = xmp.getAttributeValue(null, "long");
                            abbrev.setText(abbrev.shortText);
                            abbrev.setColor( Integer.parseInt( xmp.getAttributeValue(null, "color") ) );

                            abbrev.setOnClickListener(this);
                            //abbrev.setOnLongClickListener(this);

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

        MenuAdapter adapter = new MenuAdapter(this);
        this.gridView.setAdapter(adapter);
    }

    private void writeXML() {
        File directory = this.getFilesDir();
        File stats = new File(directory, "wordlogic/dictionary/abbreviations.xml");

        XmlSerializer serializer = Xml.newSerializer();
        StringWriter writer = new StringWriter();
        try {
            serializer.setOutput(writer);
            serializer.startDocument("UTF-8", true);
            serializer.startTag("", "abbreviations");
            AbbreviationItemLayout abbr;
            for (int i=0; i < this.views.size(); i++) {
                abbr = this.views.get(i);
                serializer.startTag("", "abbreviation");
                    serializer.attribute("", "short", ""+abbr.shortText);
                    serializer.attribute("", "long", ""+abbr.longText);
                    serializer.attribute("", "color", ""+abbr.color);
                serializer.endTag("", "abbreviation");
            }
            serializer.endTag("", "abbreviations");
            serializer.endDocument();
            String doc = writer.toString();

            FileWriter fw = new FileWriter(stats);
            fw.write(doc);

            fw.flush();
            fw.close();
        } catch (FileNotFoundException fnfe) {
            //do nothing, their phone's storage has been cut off for some reason
            //Toast.makeText(this, this.getResources().getString(R.string.file_not_found) + "code = " + 1004, Toast.LENGTH_LONG).show();
        } catch (Exception e) {
            IKnowUKeyboardService.sendErrorMessage(e);
        }
    }

    public void deleteItem(AbbreviationItemLayout item) {
        this.views.remove(item);
        this.gridView.invalidateViews();
    }
      
    public void addItem() {

        if ( (this.shortText.getText() != null && this.shortText.getText().length() > 0) && (this.longText.getText() != null && this.longText.getText().length() > 0) ) {

            AbbreviationItemLayout abbr = new AbbreviationItemLayout(this);
            abbr.index = this.views.size();
            abbr.shortText = this.shortText.getText().toString();
            abbr.longText = this.longText.getText().toString();
            abbr.setText(this.shortText.getText().toString());
            abbr.setColor( this.currentColorSelected );

            abbr.setOnClickListener(this);
            //abbrev.setOnLongClickListener(this);
            this.views.add(abbr);

            this.shortText.setText("");
            this.longText.setText("");

            Toast.makeText(this, "Added!", Toast.LENGTH_SHORT).show();

            this.gridView.invalidateViews();
        } else {
            Toast.makeText(this, "You must fill out all fields", Toast.LENGTH_LONG).show();
        }
    }

    @Override
    public void onClick(View v) {
        //AbbreviationItemLayout abbr = (AbbreviationItemLayout) v;
        //this.keyboardService.sendTextToEditor(abbr.shortText,0,0, false);

        if (v.getId() == COLOR_PICKER_BLUE_ID) {
            this.colorPicker.getChildAt(this.currentColorSelected).setSelected(false);
            this.currentColorSelected = 0;
            this.colorPicker.getChildAt(this.currentColorSelected).setSelected(true);
        } else if (v.getId() == COLOR_PICKER_GREEN_ID) {
            this.colorPicker.getChildAt(this.currentColorSelected).setSelected(false);
            this.currentColorSelected = 1;
            this.colorPicker.getChildAt(this.currentColorSelected).setSelected(true);
        } else if (v.getId() == COLOR_PICKER_ORANGE_ID) {
            this.colorPicker.getChildAt(this.currentColorSelected).setSelected(false);
            this.currentColorSelected = 2;
            this.colorPicker.getChildAt(this.currentColorSelected).setSelected(true);
        } else if (v.getId() == COLOR_PICKER_PINK_ID) {
            this.colorPicker.getChildAt(this.currentColorSelected).setSelected(false);
            this.currentColorSelected = 3;
            this.colorPicker.getChildAt(this.currentColorSelected).setSelected(true);
        } else if (v.getId() == COLOR_PICKER_RED_ID) {
            this.colorPicker.getChildAt(this.currentColorSelected).setSelected(false);
            this.currentColorSelected = 4;
            this.colorPicker.getChildAt(this.currentColorSelected).setSelected(true);
        } else if (v.getId() == COLOR_PICKER_YELLOW_ID) {
            this.colorPicker.getChildAt(this.currentColorSelected).setSelected(false);
            this.currentColorSelected = 5;
            this.colorPicker.getChildAt(this.currentColorSelected).setSelected(true);
        } else if (v.getId() == ADD_BUTTON_ID) {
            this.addItem();
        } else {
            //an abbreviation box has been clicked
            //AbbreviationItemLayout abbr =
        }

    }

    private class AbbreviationItemLayout extends LinearLayout implements View.OnClickListener {

        private static final int IMAGE_WIDTH = 35;
        private static final int IMAGE_HEIGHT = 35;

        private static final int ITEM_HEIGHT = 150;

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
        private Context context;
        public int color;

        public Button deleteButton;
        public TextView text;

        public AbbreviationItemLayout(Context context) {
            super(context);
            this.context = context;
            init();
        }

        public AbbreviationItemLayout(Context context, AttributeSet attrs) {
            super(context, attrs);
            this.context = context;
            init();
        }

        public AbbreviationItemLayout(Context context, AttributeSet attrs, int defStyle) {
            super(context, attrs, defStyle);
            this.context = context;
            init();
        }

        private void init() {
            AbsListView.LayoutParams vparams = new AbsListView.LayoutParams(AbsListView.LayoutParams.MATCH_PARENT, ITEM_HEIGHT);
            this.setLayoutParams(vparams);
            this.setColor(COLOR_YELLOW);
            //this.setPadding(5,5,5,5);
            this.setGravity(Gravity.CENTER);
            this.setOrientation(VERTICAL);

            LayoutParams params = new LayoutParams(LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT);
            params.gravity = Gravity.CENTER;
            text = new TextView(context);
            //text.setGravity(Gravity.CENTER);
            text.setLayoutParams(params);
            text.setTextColor(Color.BLACK);
            text.setTextSize(25);

            this.addView(text);

            params = new LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT);
            params.gravity = Gravity.CENTER;
            params.topMargin = 10;
            deleteButton = new Button(this.context);
            deleteButton.setLayoutParams(params);
            deleteButton.setPadding(10, 10, 10, 10);
            deleteButton.setBackgroundResource(R.drawable.black_state_button);
            deleteButton.setTextColor(0xFFFFFFFF);
            deleteButton.setTextSize(20);
            deleteButton.setOnClickListener(this);
            deleteButton.setText(this.context.getResources().getString(R.string.label_delete));

            this.addView(deleteButton);
        }

        public void setColor(int color) {

            this.color = color;

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

        public void setHeight(int height) {
            AbsListView.LayoutParams vparams = new AbsListView.LayoutParams(AbsListView.LayoutParams.MATCH_PARENT, height);
            this.setLayoutParams(vparams);
        }

        public void setText(String text) {
            this.text.setText(text);
        }

        @Override
        public void onClick(View v) {
            deleteItem(this);
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
            AbbreviationItemLayout menuItem = views.get(position);

            return menuItem;
        }
    }

}
