package com.iknowu.downloader;

import android.content.Context;
import android.util.AttributeSet;
import android.util.Log;
import android.util.TypedValue;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.RelativeLayout;
import android.widget.TextView;

import com.iknowu.IKnowUKeyboardService;
import com.iknowu.R;
import com.iknowu.draganddrop.DragDropItem;

import java.util.ArrayList;

/**
 * A {@link LinearLayout} subclass that is used to display a single dictionary that either can be
 * downloaded, or has already been downloaded. It will provide the ability to enable/disable the dictionary,
 * as well as download/update/delete the dictionary to/from the device.
 * 
 * It also implements our custom {@link DragDropItem} interface, to enable the placement of this item
 * in order to set it's priority in the dictionary list
 * 
 * @author Justin Desjardins
 *
 */
public class DictionaryItem extends LinearLayout implements DragDropItem {
	
	private static final int DOWNLOAD_BUTTON_ID = 111111;
	private static final int TEXT_VIEW_ID = 111112;
	private static final int CHECKBOX_ID = 111113;
	
	private Context context;
	private TextView dictname;
	private Button downloadButton;
	private Button updateButton;
	private Button deleteButton;
	public ProgressBar progress;
	
	private int index;
	private int version;
	private String name;
	private int layout;
	private String fileName;
	private String localFileName;
	private boolean isEnabled;
	
	private boolean hasAutoDict;
	
	private UserDictionary userDictionary;
	
	private DownloadActivity downloader;
	private CheckBox checkBox;
	
	private boolean draggable;
	
	private int maxPriority;
	
	public DictionaryItem(Context context) {
		super(context);
		this.context = context;
		this.init();
	}
	
	public DictionaryItem(Context context, AttributeSet attrs) {
		super(context, attrs);
		this.context = context;
		this.init();
	}
	
	/**
	 * Initialize this item. Set up it's views and layout parameters
	 */
	private void init() {
		LinearLayout.LayoutParams linparams = new LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT);
		linparams.setMargins(0, 0, 0, 10);
		this.setLayoutParams(linparams);
		this.setPadding(10, 3, 8, 3);
		this.setOrientation(LinearLayout.VERTICAL);
		//this.setOrientation(LinearLayout.HORIZONTAL);
		this.setBackgroundColor(0xFF000000);
		//this.setBackgroundResource(R.drawable.info_bg_black);
		
		linparams = new LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT);
		//this will be composed of a relative layout and a linear layout
		RelativeLayout rel = new RelativeLayout(this.context);
		rel.setLayoutParams(linparams);
		
		RelativeLayout rel2 = new RelativeLayout(this.context);
		//rel2.setGravity(Gravity.CENTER_VERTICAL);
		rel2.setLayoutParams(linparams);
		
		RelativeLayout.LayoutParams relparams = new RelativeLayout.LayoutParams(RelativeLayout.LayoutParams.WRAP_CONTENT, RelativeLayout.LayoutParams.WRAP_CONTENT);
		relparams.addRule(RelativeLayout.ALIGN_PARENT_LEFT);
		relparams.addRule(RelativeLayout.CENTER_VERTICAL);
		//params.addRule(RelativeLayout.ALIGN_BOTTOM, DOWNLOAD_BUTTON_ID);
		this.dictname = new TextView(this.context);
		this.dictname.setId(TEXT_VIEW_ID);
		this.dictname.setTextSize(TypedValue.COMPLEX_UNIT_SP, 18);
		this.dictname.setTextColor(0xFF000000);
		this.dictname.setLayoutParams(relparams);
		rel.addView(this.dictname);
		
		//checkbox is not checked by default
		RelativeLayout.LayoutParams rel2params = new RelativeLayout.LayoutParams(RelativeLayout.LayoutParams.WRAP_CONTENT, RelativeLayout.LayoutParams.WRAP_CONTENT);
		rel2params.addRule(RelativeLayout.ALIGN_PARENT_RIGHT);
		rel2params.addRule(RelativeLayout.CENTER_VERTICAL);
		this.checkBox = new CheckBox(this.context);
		this.checkBox.setId(CHECKBOX_ID);
		this.checkBox.setLayoutParams(rel2params);
		this.checkBox.setChecked(false);
		rel.addView(this.checkBox);
		
		rel2params = new RelativeLayout.LayoutParams(175, 50);
		rel2params.addRule(RelativeLayout.LEFT_OF, CHECKBOX_ID);
		rel2params.addRule(RelativeLayout.CENTER_VERTICAL);
		//rel2params.gravity = Gravity.CENTER_HORIZONTAL;
		this.downloadButton = new Button(this.context);
		this.downloadButton.setText(this.context.getResources().getString(R.string.label_download));
		this.downloadButton.setTextColor(0xFFFFFFFF);
		this.downloadButton.setBackgroundResource(R.drawable.black_state_button);
		this.downloadButton.setLayoutParams(rel2params);
		this.downloadButton.setPadding(5, 4, 5, 4);
		this.downloadButton.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				downloadButtonClicked();
			}
		});
		rel.addView(this.downloadButton);
		
		this.deleteButton = new Button(this.context);
		this.deleteButton.setText(this.context.getResources().getString(R.string.label_delete));
		this.deleteButton.setTextColor(0xFFFFFFFF);
		this.deleteButton.setBackgroundResource(R.drawable.blue_state_button);
		this.deleteButton.setLayoutParams(rel2params);
		this.deleteButton.setPadding(5, 4, 5, 4);
		this.deleteButton.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				deleteButtonClicked();
			}
		});
		rel.addView(this.deleteButton);
		
		this.updateButton = new Button(this.context);
		this.updateButton.setText(this.context.getResources().getString(R.string.label_update));
		this.updateButton.setTextColor(0xFFFFFFFF);
		this.updateButton.setBackgroundResource(R.drawable.green_state_button);
		this.updateButton.setLayoutParams(rel2params);
		this.updateButton.setPadding(5, 4, 5, 4);
		this.updateButton.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				downloadButtonClicked();
			}
		});
		rel.addView(this.updateButton);
		
		relparams = new RelativeLayout.LayoutParams(RelativeLayout.LayoutParams.MATCH_PARENT, RelativeLayout.LayoutParams.WRAP_CONTENT);
		relparams.addRule(RelativeLayout.ALIGN_PARENT_LEFT);
		this.progress = new ProgressBar(this.context, null, android.R.attr.progressBarStyleHorizontal);
		this.progress.setLayoutParams(relparams);
		this.progress.setVisibility(View.GONE);
		rel2.addView(this.progress);
		
		this.addView(rel);
		this.addView(rel2);
		
		this.setAsDownloadable();
	}
	
	@Override
	public void onDropLayoutParams() {
		LinearLayout.LayoutParams linparams = new LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT);
		linparams.setMargins(0, 0, 0, 10);
		this.setLayoutParams(linparams);
	}
	
	/**
	 * Set the downloader connected with this item
	 * 
	 * @param downloader
	 */
	public void setDownloader(DownloadActivity downloader) {
		this.downloader = downloader;
	}
	
	/**
	 * The download button has been clicked
	 */
	public void downloadButtonClicked() {
		this.progress.setVisibility(View.VISIBLE);
		this.downloader.startDownload(this);
	}
	
	/**
	 * The delete button has been clicked
	 */
	public void deleteButtonClicked() {
		this.downloader.delete(this.localFileName);
	}
	
	/**
	 * Set the name to be displayed for this item
	 * 
	 * @param name the name of the dictionary
	 */
	public void setDictName(String name) {
		this.name = name;
		this.dictname.setText(name);
	}
	
	/**
	 * Get the displayed name of this dictionary
	 * @return the name
	 */
	public String getDictName() {
		return this.name;
	}
	
	/**
	 * Hide the download/update/delete buttons indicating that we are in offline mode
	 */
	public void hideButtons() {
		this.setBackgroundColor(0xFF000000);
		this.dictname.setTextColor(0xFFFFFFFF);
		//this.setBackgroundResource(R.drawable.info_bg_grey);
		this.updateButton.setVisibility(View.GONE);
		this.downloadButton.setVisibility(View.GONE);
		this.deleteButton.setVisibility(View.GONE);
		this.checkBox.setVisibility(View.VISIBLE);
		this.draggable = true;
	}
	
	/**
	 * Set this item as downloadable, in other words it hasn't been downloaded yet.
	 */
	public void setAsDownloadable() {
		this.setBackgroundColor(0xFFaFaFaF);
		this.dictname.setTextColor(0xFF000000);
		//this.setBackgroundResource(R.drawable.info_bg_grey);
		this.updateButton.setVisibility(View.GONE);
		this.downloadButton.setVisibility(View.VISIBLE);
		this.deleteButton.setVisibility(View.GONE);
		this.checkBox.setVisibility(View.INVISIBLE);
		this.draggable = false;
	}
	
	/**
	 * Set this item as deleteable, in other words it has been downloaded, and can be deleted
	 */
	public void setAsDeleteable() {
		this.setBackgroundColor(0xFF000000);
		this.dictname.setTextColor(0xFFFFFFFF);
		//this.setBackgroundResource(R.drawable.info_bg_black);
		this.updateButton.setVisibility(View.GONE);
		this.downloadButton.setVisibility(View.GONE);
		this.deleteButton.setVisibility(View.VISIBLE);
		this.checkBox.setVisibility(View.VISIBLE);
		this.draggable = true;
	}
	
	/**
	 * Set this item as having an update available, that can be downloaded
	 */
	public void setUpdateAvailable() {
		this.setBackgroundColor(0xFF000000);
		this.dictname.setTextColor(0xFFFFFFFF);
		//this.setBackgroundResource(R.drawable.info_bg_green);
		this.updateButton.setVisibility(View.VISIBLE);
		this.downloadButton.setVisibility(View.GONE);
		this.deleteButton.setVisibility(View.GONE);
		this.checkBox.setVisibility(View.VISIBLE);
		this.draggable = true;
	}
	
	public void setIndex(int index) {
		this.index = index;
	}
	
	public int getIndex() {
		return this.index;
	}
	
	public void setVersion(int vers) {
		this.version = vers;
	}
	
	public int getVersion() {
		return this.version;
	}
	
	/**
	 * Set the layout to be used with this dictionary. IE. QWERTY, QZERTY, AZERTY etc.
	 * @param lay
	 */
	public void setLayout(int lay) {
		this.layout = lay;
	}
	
	public int getLayout() {
		return this.layout;
	}
	
	/**
	 * Set the file path for this dictionary located on the cloud
	 * @param name
	 */
	public void setFileName(String name) {
		this.fileName = name;
	}
	
	public String getFileName() {
		return this.fileName;
	}
	
	/**
	 * Set the local name of this file as it will be stored on this device
	 * This name is without the file extension
	 * 
	 * @param name
	 */
	public void setLocalFileName(String name) {
		this.localFileName = name;
	}
	
	public String getLocalFileName() {
		return this.localFileName;
	}
	
	/**
	 * Set this item to be enabled or disabled. When disabled it won't be drag and droppable.
	 */
	public void setEnabled(boolean enable) {
		this.isEnabled = enable;
		this.checkBox.setChecked(this.isEnabled);
	}
	
	public boolean getEnabled() {
		return this.checkBox.isChecked();
	}
	
	public void setUserDictionary(UserDictionary dict) {
		this.userDictionary = dict;
	}
	
	public UserDictionary getUserDictionary() {
		return this.userDictionary;
	}
	
	/**
	 * Set whether or not there is an auto-correct dictionary associated with this dictionary
	 * @param has
	 */
	public void setHasAutoDict(boolean has) {
		this.hasAutoDict = has;
	}
	
	public boolean getHasAutoDict() {
		return this.hasAutoDict;
	}
	
	/**
	 * Set the maximum spot this item can take on the priority list
	 * @param max
	 */
	public void setMaxPriority(int max) {
		this.maxPriority = max;
	}
	
	@Override
	public int getMaxPosition() {
		return this.maxPriority;
	}

	@Override
	public void highlight() {
		this.setBackgroundColor(0xFF3Ea7f2);
		//this.setBackgroundResource(R.drawable.bg_blue);
	}

	@Override
	public void unHighlight() {
		this.setBackgroundColor(0xFF000000);
		//this.setBackgroundResource(R.drawable.edit_text_background);
	}

	@Override
	public boolean isDraggable() {
		return this.draggable;
	}

	@Override
	public ArrayList<View> getChildren() {
		ArrayList<View> children = new ArrayList<View>(this.getChildCount());
		this.getViewGroupChildren(children, this);
		return children;
	}
	
	/**
	 * Fill the ArrayList children with all of this ViewGroup's views that are not
	 * ViewGroups themselves. This fills with only the views that we want to be able to
	 * receive touch events
	 * 
	 * @param children the ArrayList to be filled
	 * @param parent the ViewGroup to search in, to be used recursively.
	 */
	private void getViewGroupChildren(ArrayList<View> children, ViewGroup parent) {
		for (int i=0; i < parent.getChildCount(); i++) {
			View child = parent.getChildAt(i);
			if (child instanceof ViewGroup) {
				final ViewGroup vgchild = (ViewGroup) child;
				this.getViewGroupChildren(children, vgchild);
			} else {
				if (child instanceof CheckBox || child instanceof Button) {
					IKnowUKeyboardService.log(Log.VERBOSE, "Getting Child", "Child = "+child);
					children.add(child);
				}
			}
		}
	}
}
