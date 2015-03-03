package com.iknowu.preferences;

import android.app.AlertDialog.Builder;
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.res.Resources;
import android.content.res.TypedArray;
import android.graphics.Color;
import android.preference.ListPreference;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.RadioButton;
import android.widget.TextView;

import com.iknowu.IKnowUKeyboardService;
import com.iknowu.R;

import java.util.ArrayList;

public class ImageListPreference extends ListPreference {

	ImageListPreferenceAdapter customListPreferenceAdapter = null;
	Context mContext;
	private LayoutInflater mInflater;
	CharSequence[] entries;
	String[] entryValues;
	ArrayList<RadioButton> rButtonList;
	int[] imageResIds;
	
	private String currentSelected;

	public ImageListPreference(Context context, AttributeSet attrs) {
		super(context, attrs);
		mContext = context;
		mInflater = LayoutInflater.from(context);
		rButtonList = new ArrayList<RadioButton>();
		
		TypedArray a = context.obtainStyledAttributes(attrs, R.styleable.ImageListPreference);
		
		int iconId = a.getResourceId(R.styleable.ImageListPreference_entryIcons, -1);
		//Log.d("ImageListPref", "Entry Icons id = "+id+", Theme icons id = "+R.array.themeIconsArray);
		
		Resources res = context.getResources();
		TypedArray vals = res.obtainTypedArray(iconId);
		
		//load the image ids from the data
		imageResIds = new int[vals.length()];
		
		for (int i=0; i < vals.length(); i++) {
			imageResIds[i] = vals.getResourceId(i, -1);
		}
		
		//load the entry values from the data
		int valsId = a.getResourceId(R.styleable.ImageListPreference_entryValuesIds, -1);
		if (valsId > -1) {
			vals = res.obtainTypedArray(valsId);
			//load the image ids from the data
			entryValues = new String[vals.length()];
			
			for (int i=0; i < vals.length(); i++) {
				entryValues[i] = vals.getResourceId(i, -1)+"";
			}
		} else {
			valsId = a.getResourceId(R.styleable.ImageListPreference_entryValuesInts, -1);
			if (valsId > -1) {
				vals = res.obtainTypedArray(valsId);
				//load the image ids from the data
				entryValues = new String[vals.length()];
				
				for (int i=0; i < vals.length(); i++) {
					entryValues[i] = vals.getInt(i, -1)+"";
				}
			}
		}
		vals.recycle();
		a.recycle();
	}

	@Override
	protected void onPrepareDialogBuilder(Builder builder) {
		try {
			builder.setPositiveButton(null, null);
			entries = getEntries();
			//entryValues = getEntryValues();
			
			this.currentSelected = this.getPersistedString(IKnowUKeyboardService.DEFAULT_KEYBOARD_THEME_ID+"");
			
			if (entries == null || entryValues == null || entries.length != entryValues.length) {
				throw new IllegalStateException("ListPreference requires an entries array and an entryValues array which are both the same length");
			}

			customListPreferenceAdapter = new ImageListPreferenceAdapter(mContext);

			builder.setAdapter(customListPreferenceAdapter,
					new DialogInterface.OnClickListener() {
						public void onClick(DialogInterface dialog, int which) {
							
						}
			});
		} catch (Exception e) {
			IKnowUKeyboardService.sendErrorMessage(e);
		}
	}

	private class ImageListPreferenceAdapter extends BaseAdapter {
		
		public ImageListPreferenceAdapter(Context context) {
		}

		public int getCount() {
			return entries.length;
		}
		
		public Object getItem(int position) {
			return position;
		}

		public long getItemId(int position) {
			return position;
		}
		
		public View getView(final int position, View convertView, ViewGroup parent) {
			try {
				View row = convertView;
				CustomHolder holder = null;
				//Log.d("Get View", "Parent = "+parent);
				
				if (row == null) {
					row = createView(position, row, parent, holder);
				} else {
					TextView tview = (TextView) row.findViewById(R.id.img_list_pref_text_view);
					//Log.d("Get View", "GetText = "+tview.getText()+", Entries = "+entries[position]);
					//check for this because for some reason android will create duplicates of rows at different indexes,
					//so if the entry at the index is not equal to what the view contains, then it needs to be recreated
					if (!tview.getText().equals(entries[position])) {
						row = createView(position, row, parent, holder);
					}
				}
				return row;
			} catch (Exception e) {
				IKnowUKeyboardService.sendErrorMessage(e);
				return convertView;
			}
		}
		
		private View createView(final int position, View row, ViewGroup parent, CustomHolder holder) {
			//Log.d("CreateView", "Position = "+position);
			row = mInflater.inflate(R.layout.image_list_pref_item, parent, false);
			holder = new CustomHolder(row, position);
			row.setTag(holder);
			
			String val = entryValues[position];
			
			//Log.d("Text = "+val, "Current Selected = "+currentSelected);
			
			if (val.equals(currentSelected)) {
				holder.rButton.setChecked(true);
			}
			
			row.setClickable(true);
			row.setOnTouchListener(new View.OnTouchListener() {
				@Override
				public boolean onTouch(View v, MotionEvent event) {
					int action = event.getAction();
					switch (action) {
						case MotionEvent.ACTION_DOWN:
							v.setBackgroundColor(Color.argb(130, 0, 180, 255));
							v.setSelected(true);
							v.setPressed(true);
							return false;
						case MotionEvent.ACTION_MOVE:
							v.setBackgroundColor(Color.TRANSPARENT);
							v.setSelected(false);
							v.setPressed(false);
							break;
						case MotionEvent.ACTION_UP:
							v.setBackgroundColor(Color.TRANSPARENT);
							v.setSelected(false);
							v.setPressed(false);
							break;
						case MotionEvent.ACTION_CANCEL:
							v.setBackgroundColor(Color.TRANSPARENT);
							v.setSelected(false);
							v.setPressed(false);
							break;
					}
					return false;
				}
			});
			row.setOnClickListener(new View.OnClickListener() {
				public void onClick(View v) {
					
					CustomHolder holder = (CustomHolder) v.getTag();
					
					for (RadioButton b : rButtonList) {
						if (b.isChecked())
							b.setChecked(false);
					}
					
					holder.rButton.setChecked(true);
					
					persistString(entryValues[position]);
					
					Dialog mDialog = getDialog();
					if (mDialog != null) {
						mDialog.dismiss();
					}
				}
			});
			return row;
		}

		class CustomHolder {
			private TextView text = null;
			private ImageListRadioButton rButton = null;
			private ImageView image = null;

			CustomHolder(View row, int position) {
				image = (ImageView) row.findViewById(R.id.img_list_pref_image_view);
				image.setImageResource(imageResIds[position]);
				text = (TextView) row.findViewById(R.id.img_list_pref_text_view);
				text.setText(entries[position]);
				rButton = (ImageListRadioButton) row.findViewById(R.id.img_list_pref_radio_button);
				rButton.setId(position);

				rButtonList.add(rButton);
			}
		}
	}

}
