package com.iknowu.dictionarymanager;

import android.app.ListActivity;
import android.graphics.drawable.ColorDrawable;
import android.os.Bundle;
import android.text.Editable;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.widget.Toast;

import com.iknowu.IKnowUKeyboardService;
import com.iknowu.PredictionEngine;
import com.iknowu.R;
import com.parse.FindCallback;
import com.parse.ParseException;
import com.parse.ParseObject;
import com.parse.ParseQuery;
import com.wordlogic.lib.AutoCorrect;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public class DictionaryManager extends ListActivity {
	
	private static int MAX_DICTIONARIES = 20;
	private PredictionEngine predictionEngine;
//	private AutoCorrect autoCorrectEngine;
	
	private ArrayList<String> words;
	
	private EditText addWordBox;
	private ImageView addWordButton;
	private ImageView refreshButton;
	private DictWordAdapter dictWordAdapter;
	
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
	}
	
	@Override
	public void onResume() {
		super.onResume();
		
		this.words = new ArrayList<String>();
		
		this.predictionEngine = IKnowUKeyboardService.getPredictionEngine();
	//	this.autoCorrectEngine = IKnowUKeyboardService.getAutoCorrectEngine();
		
		//this.runOnUiThread(this.autoCorrectEngine.getWordList(this));
		this.getListView().setBackgroundColor(0xFFdfdfdf);
		this.getListView().setPadding(5, 5, 5, 5);
		this.getListView().setDivider( new ColorDrawable(0xFFdfdfdf) );
		this.getListView().setDividerHeight(5);
		this.getListView().setCacheColorHint(0x00000000);
		this.getPersonalWords();
	}
	
	@Override
	public void onPause() {
//		if (this.autoCorrectEngine != null) {
//			this.autoCorrectEngine.saveUpdates();
//		}
		super.onPause();
	}
	
	private ArrayList<String> getDictionaries() {
		ArrayList<String> dicts = new ArrayList<String>();
		for (int i = 0; i < MAX_DICTIONARIES; i++) {
			boolean bool = this.predictionEngine.getDictInfo(i);
			if (bool) {
				dicts.add(this.predictionEngine.getDictName());
				IKnowUKeyboardService.log(Log.DEBUG, "DICTIONARY ["+i+"] =", ""+dicts.get(i));
			} else {
				break;
			}
		}
		return dicts;
	}
	
	private void getPersonalWords() {
		try {
	//		String[] autoWords = this.autoCorrectEngine.listWords();
			String [] predWords = this.predictionEngine.getPersonalDictWords();
			
			//Log.v("Dict Manager", "autoWords size = "+autoWords.size());
			//Log.v("Dict Manager", "predWords size = "+predWords.length);
			//find out which of the two lists is bigger, because they might be different sizes.
			int biggest;
			
			//if (autoWords == null) {
				if (predWords != null) {
					IKnowUKeyboardService.log(Log.VERBOSE, "Auto words = null", "Setting biggest to predWords.lenght = "+predWords.length);
					biggest = predWords.length;
				} else {
					IKnowUKeyboardService.log(Log.VERBOSE, "predWords = null", "Setting biggest to 0");
					biggest = 0;
				}
		//	} else if (predWords == null) {
		//		IKnowUKeyboardService.log(Log.VERBOSE, "Pred words = null", "Setting biggest to autowords.lenght = "+autoWords.length);
		//		biggest = autoWords.length;
		//	} else {
			//	biggest = autoWords.length > predWords.length ? autoWords.length : predWords.length;
		//	}
			
			//loop through the lists and try to add words to the list of words to show
			for (int i=0; i < biggest; i ++) {
			/*	String aword = null;
				if ( autoWords != null && i < autoWords.length ) {
					aword = autoWords[i];
					IKnowUKeyboardService.log(Log.INFO, "Dict Manager", "auto personal word "+i+" = "+aword);
				}
				*/
				String pword = null;
				if ( i < predWords.length ) {
					pword = predWords[i];
					IKnowUKeyboardService.log(Log.WARN, "Dict Manager", "pred personal word "+i+" = "+pword);
				}
				
				//if we still have a word from the autocorrect side,
				//check if it hasn't already been added and add it
			//	if (aword != null && aword.length() > 0) {
			//		if (!this.words.contains(aword)) {
			//			this.words.add(aword);
			//		}
			//	}
				
				//if we still have a word from the prediction side,
				//check if it hasn't already been added and add it
				if (pword != null && pword.length() > 0) {
					if (!this.words.contains(pword)) {
						this.words.add(pword);
					}
				}
			}
			
			Collections.sort(this.words, String.CASE_INSENSITIVE_ORDER);
			
			ListView lview = this.getListView();
			if (lview.getHeaderViewsCount() == 0) {
				LayoutInflater inflater = getLayoutInflater();
				LinearLayout header = (LinearLayout)inflater.inflate(R.layout.add_word_bar, lview, false);
				RelativeLayout bar = (RelativeLayout) header.findViewById(R.id.add_word_bar);
				this.addWordBox = (EditText) bar.findViewById(R.id.add_word_box);
				this.addWordButton = (ImageView) bar.findViewById(R.id.add_word_button);
				this.addWordButton.setOnClickListener(new View.OnClickListener() {
					@Override
					public void onClick(View v) {
						addWord();
					}
				});
				
				this.refreshButton = (ImageView) bar.findViewById(R.id.refresh_button);
				this.refreshButton.setOnClickListener(new View.OnClickListener() {
					@Override
					public void onClick(View v) {
						refreshClicked();
					}
				});
				lview.addHeaderView(header, null, false);
			}
			
			this.dictWordAdapter = new DictWordAdapter();
			this.setListAdapter(this.dictWordAdapter);
		} catch(Exception e) {
			IKnowUKeyboardService.sendErrorMessage(e);
		}
	}
	
	private void addWord() {
		try {
			Editable text = this.addWordBox.getText();
			String text2 = text.toString();
			if (text.length() > 0) {
				IKnowUKeyboardService.addWord(text2);
				this.addWordBox.setText("");
				//this.words.add(text2);
				//Collections.sort(this.words, String.CASE_INSENSITIVE_ORDER);
				//this.dictWordAdapter.notifyDataSetChanged();
				this.getPersonalWords();
			} else {
				Toast.makeText(this, "Please enter a word first", Toast.LENGTH_SHORT).show();
			}
		} catch (Exception e) {
			IKnowUKeyboardService.sendErrorMessage(e);
		}
	}
	
	private void deleteWord(String word) {
		try {
			//Log.d("deleting word =", ""+word);
			if (word != null) {
				boolean deleted = IKnowUKeyboardService.deleteWord(word);
				//Log.e("Delete word", "RETURNS ="+deleted);
				this.words.remove(word);
				this.dictWordAdapter.notifyDataSetChanged();
			}
		} catch (Exception e) {
			IKnowUKeyboardService.sendErrorMessage(e);
		}
	}
	
	private void refreshClicked() {
		try {
			if (IKnowUKeyboardService.getCurrentUser() != null) {
				ParseQuery query = new ParseQuery("Word");
				query.whereEqualTo("User", IKnowUKeyboardService.getCurrentUser().getObjectId());
				query.findInBackground(new FindCallback() {
				    public void done(List<ParseObject> list, ParseException e) {
				        if (e == null) {
				            refresh(list);
				        }
				    }
				});
			} else {
				Toast.makeText(this, "Please Sign-in/enable cloud sync first", Toast.LENGTH_SHORT).show();
			}
		} catch (Exception e) {
			IKnowUKeyboardService.sendErrorMessage(e);
		}
	}
	
	private void refresh(List<ParseObject> list) {
		try {
			for ( int j = 0; j < list.size(); j++) {
				ParseObject word = list.get(j);
				
				String wordStr = word.getString("Word");
				
				IKnowUKeyboardService.log(Log.VERBOSE, "Refreshing personal from cloud", "word = "+wordStr);
				
				//add words to the engines, known words won't be added
				this.predictionEngine.addWord(wordStr, 1);
			//	this.autoCorrectEngine.addWord(wordStr);
			}
			
			this.getPersonalWords();
		} catch (Exception e) {
			IKnowUKeyboardService.sendErrorMessage(e);
		}
	}
	
	private class DictWordAdapter extends ArrayAdapter<String> {
		public DictWordAdapter() {
			super(DictionaryManager.this, R.layout.dictionary_manager_word, R.id.dict_manager_word, words);
		}

		@Override
		public View getView(int position, View convertView, ViewGroup parent) {
			View row = super.getView(position, convertView, parent);
			ImageView icon = (ImageView) row.findViewById(R.id.dict_manager_delete_btn);
			//TextView word = (TextView) row.findViewById(R.id.dict_manager_word);
			icon.setOnClickListener(new View.OnClickListener() {
				@Override
				public void onClick(View v) {
					//Log.v("delete", "clicked");
					RelativeLayout lin = (RelativeLayout) v.getParent();
					TextView textv = (TextView) lin.findViewById(R.id.dict_manager_word);
					int pos = getPosition(textv.getText().toString());
					IKnowUKeyboardService.log(Log.VERBOSE, "Dictwordadapter onClick", "position = "+pos);
					deleteWord(textv.getText().toString());
				}
			});
			return (row);
		}
	}
}
