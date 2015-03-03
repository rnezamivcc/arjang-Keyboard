package com.iknowu.downloader;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.NotificationManager;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.graphics.Rect;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.AsyncTask;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.RelativeLayout;
import android.widget.Toast;

import com.iknowu.IKnowUKeyboardService;
import com.iknowu.PredictionEngine;
import com.iknowu.R;
import com.iknowu.draganddrop.DragDropScrollView;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserFactory;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.URL;
import java.net.URLConnection;
import java.util.ArrayList;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

/**
 * The main activity in the dictionary setup/downloader screen. It facilitates all of the
 * actions required to setup/download/update/delete the dictionaries for our {@link PredictionEngine}
 * and {@link AutoCorrect} engine.
 * 
 * @author Justin Desjardins
 *
 */
public class DownloadActivity extends Activity {
	public static final String TAG_DICTIONARY = "dictionary";
    public static final String TAG_LIBRARY = "library";
	
	public static final String DOWNLOAD_DIR = "/wordlogic/dictionary/";
	public static final String DICTS_INFO_FILE = "dictinfo.xml";
	public static final String DICTS_INFO_FILE_OFFLINE = "dictinfonew.xml";
	public static final String DICTS_INFO_FILE_DL = "https://dl.dropboxusercontent.com/s/1y2u9950xommvas/dictionary_info.xml?dl=1";
	private static final String DICT_PACKAGE_NAME = "dictpackage.zip";
	// declare the dialog as a member field of your activity
	private DictionaryItem downloadingItem;
	
	private PredictionEngine predEngine;
	
	private ArrayList<UserDictionary> userDicts;
	private DictionaryList dictList;
	
	private boolean downloaded;
	private AlertDialog currentDialog;
	
	private String autoDictName;
	
	private int installedDictCount = 0;
	
	private boolean offlineMode;
	
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

        this.requestWindowFeature(Window.FEATURE_NO_TITLE);

		NotificationManager mNotificationManager = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
		mNotificationManager.cancel(IKnowUKeyboardService.NOTIFICATION_ID);
	}
	
	@Override
	public void onResume() {
		super.onResume();
		
		this.predEngine = IKnowUKeyboardService.getPredictionEngine();
		this.refresh();
	}
	
	@Override
	public void onPause() {
		if (currentDialog != null) {
			this.currentDialog.dismiss();
		}
		this.close(null);
		super.onPause();
	}
	
	private void refresh() {
		this.offlineMode = !this.checkNetworkState();
		
		if (!this.offlineMode) {
			this.setContentView(R.layout.loading_screen);
			
			this.getUserDicts();
			this.getXMLFile();
		} else {
			this.getUserDicts();
			this.populateOfflineList();
			//this.setContentView(R.layout.download_error);
		}
	}
	
	private boolean checkNetworkState() {
		ConnectivityManager cm = (ConnectivityManager) getSystemService(Context.CONNECTIVITY_SERVICE);
		NetworkInfo netInfo = cm.getActiveNetworkInfo();
		if (netInfo != null && netInfo.isConnected()) {
			return true;
		}
		return false;
	}
	
	private void getXMLFile() {
		DownloadXML dictInfo = new DownloadXML();
		dictInfo.execute(DICTS_INFO_FILE_DL);
	}
	
	/**
	 * Fill the list of installed dictionaries on the device
	 */
	private void getUserDicts() {
		try {
			this.userDicts = new ArrayList<UserDictionary>();
			for (int i = 1; i < 15; i++) {
				boolean exists = this.predEngine.getDictInfo(i);
				
				if (exists) {
					UserDictionary dict = new UserDictionary();
					dict.langIdx = this.predEngine.getDictIndex();
					dict.listIdx = this.predEngine.getDictPriority();
					dict.dictname = this.predEngine.getDictName();
					dict.codeVersion = this.predEngine.getDictCodeVersion();
					dict.dataVersion = this.predEngine.getDictDataVersion();
					dict.enabled = this.predEngine.getDictEnabled();
					IKnowUKeyboardService.log(Log.VERBOSE, "Got user dict", "langidx = "+dict.langIdx+", listidx = "+dict.listIdx+
							", dictname = "+dict.dictname+", codeVersion = "+dict.codeVersion+", dataVersion = "+dict.dataVersion);
					this.userDicts.add(dict);
				}
			}
		} catch (Exception e) {
			IKnowUKeyboardService.sendErrorMessage(e);
		}
	}
	
	/**
	 * Check an index to see if the user has that dictionary installed
	 * 
	 * @param langIdx the index to check for
	 * @return a {@link UserDictionary} if it is installed, or null otherwise
	 */
	private UserDictionary checkForDict(int langIdx) {
		try {
			for (int i = 0; i < this.userDicts.size(); i++) {
				UserDictionary dict = this.userDicts.get(i);
				
				if (dict.langIdx == langIdx) return dict;
			}
		} catch (Exception e) {
			IKnowUKeyboardService.sendErrorMessage(e);
		}
		return null;
	}
	
	/**
	 * Calculate the height of the top display provided by the Android system for accurately diplaying our
	 * list of items
	 * 
	 * @return the height of the top display in pixels
	 */
	private int calcTopHeight() {
		Rect rectgle= new Rect();
		Window window= getWindow();
		window.getDecorView().getWindowVisibleDisplayFrame(rectgle);
		//int StatusBarHeight = rectgle.top;
		int contentViewTop = window.findViewById(Window.ID_ANDROID_CONTENT).getTop();
		//int TitleBarHeight = contentViewTop - StatusBarHeight;
		return contentViewTop;
	}
	
	/**
	 * Parse the downloaded xml file to display the list of dictionaries. This will compare the 
	 * downloaded file contents to the already installed dictionaries on the user device and 
	 * display the appropriate interface icons.
	 */
	private void populateList() {
		try {
			this.installedDictCount = 0;
			
			XmlPullParserFactory factory = XmlPullParserFactory.newInstance();
			XmlPullParser xpp = factory.newPullParser();
			
			File file = new File(IKnowUKeyboardService.filesDir + "/dictionary/" +DICTS_INFO_FILE);
			
			if (file.exists()) {
				FileInputStream in = new FileInputStream(file);
				
				xpp.setInput(in, null);
				
				int eventType = xpp.getEventType();
				
				LayoutInflater inflater = (LayoutInflater) getSystemService(Context.LAYOUT_INFLATER_SERVICE);
				RelativeLayout rlay = (RelativeLayout) inflater.inflate(R.layout.language_activity_layout, null);
				DragDropScrollView sview = (DragDropScrollView) rlay.findViewById(R.id.language_scrollview);
				sview.setBackgroundColor(0xFFDFDFDF);
				
				int top = this.calcTopHeight();

                int myLibVersion = Integer.parseInt( this.getResources().getString(R.string.library_version_code) );
				
				this.dictList = (DictionaryList) sview.findViewById(R.id.language_linlayout);
				this.dictList.setTopHeight(top);
				this.dictList.relLayout = (RelativeLayout) rlay.findViewById(R.id.language_relative);
				this.dictList.parent = sview;

                int libVersion = 1;

				while (eventType != XmlPullParser.END_DOCUMENT) {
					switch (eventType) {
					case XmlPullParser.START_TAG:
                        if ( xpp.getName().equals(TAG_LIBRARY) ) {
                            libVersion = Integer.parseInt(xpp.getAttributeValue(null, "version"));
                        }

                        if (libVersion == myLibVersion) {
                            if (xpp.getName().equals(TAG_DICTIONARY)) {
                                String name = xpp.getAttributeValue(null, "name");
                                int index = Integer.parseInt(xpp.getAttributeValue(null, "index"));
                                int version = Integer.parseInt(xpp.getAttributeValue(null, "versionCode"));
                                int layout = Integer.parseInt(xpp.getAttributeValue(null, "layout"));
                                String fileName = xpp.getAttributeValue(null, "fileName");
                                String localFileName = xpp.getAttributeValue(null, "localFileName");
                                boolean hasAuto = Boolean.parseBoolean(xpp.getAttributeValue(null, "hasAutoCorrect"));
                                int maxPriority = Integer.parseInt(xpp.getAttributeValue(null, "maxPriority"));
                                boolean testMode = Boolean.parseBoolean(xpp.getAttributeValue(null, "testMode"));

                                UserDictionary userDict = this.checkForDict(index);

                                DictionaryItem item = new DictionaryItem(this);
                                item.setDictName(name);
                                item.setIndex(index);
                                item.setVersion(version);
                                item.setLayout(layout);
                                item.setDownloader(this);
                                item.setFileName(fileName);
                                item.setLocalFileName(localFileName);
                                item.setHasAutoDict(hasAuto);
                                item.setMaxPriority(maxPriority);

                                IKnowUKeyboardService.log(Log.VERBOSE, "populateList()", "userDict = "+userDict);
                                //if we have this dictionary, set up some variables
                                if (userDict != null) {
                                    item.setAsDeleteable();
                                    userDict.layout = layout;
                                    IKnowUKeyboardService.log(Log.VERBOSE, "My dict name = "+userDict.dictname, "lang index = "+userDict.langIdx);
                                    IKnowUKeyboardService.log(Log.VERBOSE, "My version", "= "+userDict.dataVersion);

                                    if (userDict.dataVersion < version) item.setUpdateAvailable();

                                    item.setEnabled(userDict.enabled);
                                    item.setUserDictionary(userDict);

                                    //need this because we don't show personal dictionary here
                                    int priority = userDict.listIdx - 1;
                                    if (priority < item.getMaxPosition()-1) priority = item.getMaxPosition()-1;
                                    IKnowUKeyboardService.log(Log.VERBOSE, "Dictionary = "+name, "id="+index+", version="+version+", layout="+layout+", priority = "+priority);
                                    if (priority > this.dictList.getChildCount()) {
                                        this.dictList.addView(item);
                                    } else {
                                        this.dictList.addView(item, priority);
                                    }
                                    this.installedDictCount++;
                                    IKnowUKeyboardService.log(Log.VERBOSE, "Getting dictionaries", "installed dictionaries ="+this.installedDictCount);
                                } else {
                                    if (testMode) {
                                        if (IKnowUKeyboardService.DEBUG) {
                                            IKnowUKeyboardService.log(Log.VERBOSE, "Dictionary = "+name, "id="+index+", version="+version+", layout="+layout);
                                            this.dictList.addView(item);
                                        }
                                    } else {
                                        IKnowUKeyboardService.log(Log.VERBOSE, "Dictionary = "+name, "id="+index+", version="+version+", layout="+layout);
                                        this.dictList.addView(item);
                                    }
                                }
                            }
                        }
						break;
					}
					eventType = xpp.next();
				}
				
				//sview.addView(list);
				this.setContentView(rlay);
				
				in.close();
				file.delete();
				
				if (this.downloaded) {
					this.askToChange();
				}
			} else {
				this.setContentView(R.layout.download_error);
			}
		} catch (FileNotFoundException fnfe) {
            Toast.makeText(this, this.getResources().getString(R.string.file_not_found) + "code = "+1006, Toast.LENGTH_LONG).show();
        } catch (Exception e) {
			IKnowUKeyboardService.sendErrorMessage(e);
			e.printStackTrace();
		}
	}
	
	/**
	 * Populate the list of dictionaries based purely on the current user installed list of dictionaries.
	 * This is for when a network connection is not present and a use may simply want enable/disable a dictionary,
	 * or change its priority
	 */
	private void populateOfflineList() {
		try {
			this.installedDictCount = 0;
			
			XmlPullParserFactory factory = XmlPullParserFactory.newInstance();
			XmlPullParser xpp = factory.newPullParser();
			
			File file = new File(IKnowUKeyboardService.filesDir + "/dictionary/" +DICTS_INFO_FILE_OFFLINE);
			
			if (file.exists()) {
				FileInputStream in = new FileInputStream(file);
				
				xpp.setInput(in, null);
				
				int eventType = xpp.getEventType();
				
				LayoutInflater inflater = (LayoutInflater) getSystemService(Context.LAYOUT_INFLATER_SERVICE);
				RelativeLayout rlay = (RelativeLayout) inflater.inflate(R.layout.language_activity_layout, null);
				DragDropScrollView sview = (DragDropScrollView) rlay.findViewById(R.id.language_scrollview);
				sview.setBackgroundColor(0xFFDFDFDF);
				
				int top = this.calcTopHeight();
				
				this.dictList = (DictionaryList) sview.findViewById(R.id.language_linlayout);
				this.dictList.setTopHeight(top);
				this.dictList.relLayout = (RelativeLayout) rlay.findViewById(R.id.language_relative);
				this.dictList.parent = sview;
				
				while (eventType != XmlPullParser.END_DOCUMENT) {
					switch (eventType) {
					case XmlPullParser.START_TAG:
						if (xpp.getName().equals(TAG_DICTIONARY)) {
							String name = xpp.getAttributeValue(null, "name");
							int index = Integer.parseInt(xpp.getAttributeValue(null, "index"));
							int layout = Integer.parseInt(xpp.getAttributeValue(null, "layout"));
							String fileName = xpp.getAttributeValue(null, "fileName");
							String localFileName = xpp.getAttributeValue(null, "localFileName");
							boolean hasAuto = Boolean.parseBoolean(xpp.getAttributeValue(null, "hasAutoCorrect"));
							int maxPriority = Integer.parseInt(xpp.getAttributeValue(null, "maxPriority"));
							
							UserDictionary userDict = this.checkForDict(index);

							//if we have this dictionary, set up some variables
							if (userDict != null) {
								
								DictionaryItem item = new DictionaryItem(this);
								item.setDictName(name);
								item.setIndex(index);
								item.setLayout(layout);
								item.setDownloader(this);
								item.setFileName(fileName);
								item.setLocalFileName(localFileName);
								item.setHasAutoDict(hasAuto);
								item.setMaxPriority(maxPriority);
								
								item.hideButtons();
								userDict.layout = layout;
								IKnowUKeyboardService.log(Log.VERBOSE, "My dict name = "+userDict.dictname, "lang index = "+userDict.langIdx);
								IKnowUKeyboardService.log(Log.VERBOSE, "My version", "= "+userDict.dataVersion);
								
								item.setEnabled(userDict.enabled);
								item.setUserDictionary(userDict);
								
								//need this because we don't show personal dictionary here
								int priority = userDict.listIdx - 1;
								if (priority < item.getMaxPosition()-1) priority = item.getMaxPosition()-1;
								//IKnowUKeyboardService.log(Log.VERBOSE, "Dictionary = "+name, "id="+index+", version="+version+", layout="+layout+", priority = "+priority);
								if (priority > this.dictList.getChildCount()) {
									this.dictList.addView(item);
								} else {
									this.dictList.addView(item, priority);
								}
								this.installedDictCount++;
								IKnowUKeyboardService.log(Log.VERBOSE, "Getting dictionaries", "installed dictionaries ="+this.installedDictCount);
							}
						}
						break;
					}
					eventType = xpp.next();
				}
				
				//sview.addView(list);
				this.setContentView(rlay);
				
				in.close();
				//file.delete();
				
				if (this.downloaded) {
					this.askToChange();
				}
			} else {
				this.setContentView(R.layout.download_error);
			}
		} catch (FileNotFoundException fnfe) {
            Toast.makeText(this, this.getResources().getString(R.string.file_not_found) + "code = "+1007, Toast.LENGTH_LONG).show();
        } catch (Exception e) {
			IKnowUKeyboardService.sendErrorMessage(e);
			e.printStackTrace();
		}
	}
	
	/**
	 * Start the downloading process of a dictionary file.
	 * Files are stored as a zip with one or more files in each package
	 * 
	 * @param item the item that was clicked to be downloaded
	 */
	public void startDownload(DictionaryItem item) {
		this.downloadingItem = item;
		// execute this when the downloader must be fired
		DownloadDict downloadDict = new DownloadDict();
		downloadDict.execute(item.getFileName());
	}
	
	/**
	 * A dictionary download has finished. Extract the contents of the zip file and place them in the appropriate directory
	 */
	public void dictDownloadFinished() {
		String destination = IKnowUKeyboardService.filesDir + "/dictionary/";
		File zipfile = new File(destination+DICT_PACKAGE_NAME);
		//Log.d("Zip test file exists?", ""+zipfile.exists());
		if (zipfile.exists()) {
			try {
		        byte[] buf = new byte[1024];
		        ZipInputStream zipinputstream = null;
		        ZipEntry zipentry;
		        zipinputstream = new ZipInputStream(new FileInputStream(zipfile));

		        zipentry = zipinputstream.getNextEntry();
		        while (zipentry != null) {
		            //for each entry to be extracted
		            String entryName = destination + zipentry.getName();
		            
		            if (zipentry.getName().endsWith(".aac")) this.autoDictName = zipentry.getName();
		            IKnowUKeyboardService.log(Log.INFO, "Set auto dict name to", "= "+this.autoDictName);
		            //entryName = entryName.replace('/', File.separatorChar);
		            //entryName = entryName.replace('\\', File.separatorChar);
		            //Log.d("entryname =", "" + entryName);
		            int n;
		            FileOutputStream fileoutputstream;
		            File newFile = new File(entryName);
		            if (zipentry.isDirectory()) {
		                if (!newFile.mkdirs()) {
		                    break;
		                }
		                zipentry = zipinputstream.getNextEntry();
		                continue;
		            }

		            fileoutputstream = new FileOutputStream(entryName);

		            while ((n = zipinputstream.read(buf, 0, 1024)) > -1) {
		                fileoutputstream.write(buf, 0, n);
		            }

		            fileoutputstream.close();
		            zipinputstream.closeEntry();
		            zipentry = zipinputstream.getNextEntry();
		            IKnowUKeyboardService.log(Log.VERBOSE, entryName+" exists = ", ""+newFile.exists());
		        }//while
		        zipinputstream.close();
		        
		        zipfile.delete();
		        
		        this.predEngine.resetDictionaryConfiguration();
		        this.downloaded = true;
		        
		        this.refresh();
		    } catch (FileNotFoundException fnfe) {
                Toast.makeText(this, this.getResources().getString(R.string.file_not_found) + "code = "+1005, Toast.LENGTH_LONG).show();
            } catch (Exception e) {
		        IKnowUKeyboardService.sendErrorMessage(e);
		    }
		}
	}
	
	/**
	 * Present an {@link AlertDialog} to use user asking them if they would like to switch
	 * their top priority dictionary to the newly downloaded one.
	 */
	private void askToChange() {
		try {
			if (this.downloadingItem.getHasAutoDict()) {
				AlertDialog.Builder builder = new AlertDialog.Builder(this);
				builder.setMessage(R.string.set_top_priority_string);
				builder.setCancelable(true);
				builder.setIcon(R.drawable.iknowulogo);
				builder.setTitle(this.getResources().getString(R.string.ime_name));
				
				builder.setPositiveButton(R.string.label_ok_key, new OnClickListener() {
					@Override
					public void onClick(DialogInterface dialog, int which) {
						changeTopPriority();
					}
				});
				
				builder.setNegativeButton(this.getResources().getString(R.string.label_no_thanks), new OnClickListener() {
					@Override
					public void onClick(DialogInterface dialog, int which) {
						downloaded = false;
					}
				});
				
				this.currentDialog = builder.create();
				Window window = currentDialog.getWindow();
				WindowManager.LayoutParams ra = window.getAttributes();
				
				ra.type = WindowManager.LayoutParams.TYPE_APPLICATION_ATTACHED_DIALOG;
				window.setAttributes(ra);
				window.addFlags(WindowManager.LayoutParams.FLAG_ALT_FOCUSABLE_IM);
				currentDialog.show();
			}
		} catch (Exception e) {
			IKnowUKeyboardService.sendErrorMessage(e);
		}
	}
	
	/**
	 * Change the top dictionary over to a new one.
	 */
	private void changeTopPriority() {
		try {
			this.downloaded = false;
			
			if (this.downloadingItem != null) {
				this.predEngine.setDictSetting(this.userDicts.get(0).langIdx, this.userDicts.size(), false);
				this.predEngine.setDictSetting(this.downloadingItem.getIndex(), 1, true);
				this.predEngine.resetDictionaryConfiguration();
				
				String destination = IKnowUKeyboardService.filesDir + "/dictionary/";
				String autoDict = destination + this.downloadingItem.getDictName().toLowerCase() + ".aac";
				if (this.autoDictName != null) {
					autoDict = destination + this.autoDictName;
				}
				IKnowUKeyboardService.log(Log.VERBOSE, "Changing auto correct dict to ", "= "+autoDict);
				
				IKnowUKeyboardService.log(Log.VERBOSE, "Changing kb layout ", "to -> "+this.downloadingItem.getLayout());
				
				SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(this);
				
				Editor edit = sp.edit();
				edit.putBoolean(IKnowUKeyboardService.PREF_KB_LAYOUT_CHANGED, true);
				edit.putString(IKnowUKeyboardService.PREF_KEYBOARD_LAYOUT, this.downloadingItem.getLayout()+"");
				edit.putString(IKnowUKeyboardService.PREF_AUTO_CORRECT_DICT, autoDict);
				
				edit.commit();
			}
			
			this.refresh();
		} catch (Exception e) {
			IKnowUKeyboardService.sendErrorMessage(e);
		}
	}
	
	/**
	 * AsyncTask to retrieve a dictionary zip file from the cloud
	 * 
	 * @author Justin Desjardins
	 *
	 */
	private class DownloadDict extends AsyncTask<String, Integer, String> {
	    @Override
	    protected String doInBackground(String... sUrl) {
	        try {
	            URL url = new URL(sUrl[0]);
	            URLConnection connection = url.openConnection();
	            connection.connect();
	            // this will be useful so that you can show a typical 0-100% progress bar
	            int fileLength = connection.getContentLength();

	            // download the file
	            InputStream input = new BufferedInputStream(url.openStream());
	            //String dir = Environment.getExternalStorageDirectory() + DOWNLOAD_DIR + sUrl[1];
	            String dir = IKnowUKeyboardService.filesDir + "/dictionary/" + DICT_PACKAGE_NAME;
	            IKnowUKeyboardService.log(Log.DEBUG, "output dir = ", dir);
	            OutputStream output = new FileOutputStream(dir);
	            
	            byte data[] = new byte[1024];
	            long total = 0;
	            int count;
	            while ((count = input.read(data)) != -1) {
	                total += count;
	                // publishing the progress....
	                publishProgress((int) (total * 100 / fileLength));
	                output.write(data, 0, count);
	            }

	            output.flush();
	            output.close();
	            input.close();
	        } catch (Exception e) {
	        	IKnowUKeyboardService.sendErrorMessage(e);
	        }
	        return null;
	    }
	
	    @Override
	    protected void onPreExecute() {
	        super.onPreExecute();
	    }

	    @Override
	    protected void onProgressUpdate(Integer... progress) {
	        super.onProgressUpdate(progress);
	        downloadingItem.progress.setProgress(progress[0]);
	    }
	    
	    @Override
	    protected void onPostExecute(String result) {
	    	dictDownloadFinished();
	    }
	}
	
	/**
	 * AsyncTask to retrieve the dictionary information file from the cloud
	 * 
	 * @author Justin Desjardins
	 *
	 */
	private class DownloadXML extends AsyncTask<String, Integer, String> {
	    @Override
	    protected String doInBackground(String... sUrl) {
	        try {
	            URL url = new URL(sUrl[0]);
	            IKnowUKeyboardService.log(Log.DEBUG, "Starting download file =", ""+url.toString());
	            URLConnection connection = url.openConnection();
	            connection.connect();
	            // this will be useful so that you can show a typical 0-100% progress bar
	            int fileLength = connection.getContentLength();
	            
	            // download the file
	            InputStream input = new BufferedInputStream(url.openStream());
	            String dir = IKnowUKeyboardService.filesDir + "/dictionary/" + DICTS_INFO_FILE;
	            IKnowUKeyboardService.log(Log.DEBUG, "output dir = ", dir);
	            OutputStream output = new FileOutputStream(dir);
	            
	            byte data[] = new byte[1024];
	            long total = 0;
	            int count;
	            while ((count = input.read(data)) != -1) {
	                total += count;
	                // publishing the progress....
	                publishProgress((int) (total * 100 / fileLength));
	                output.write(data, 0, count);
	            }
	            output.flush();
	            output.close();
	            input.close();
	        } catch (Exception e) {
	        	IKnowUKeyboardService.sendErrorMessage(e);
	        }
	        return null;
	    }
	
	    @Override
	    protected void onPreExecute() {
	        super.onPreExecute();
	    }

	    @Override
	    protected void onProgressUpdate(Integer... progress) {
	        super.onProgressUpdate(progress);
	    }
	    
	    @Override
	    protected void onPostExecute(String result) {
	    	IKnowUKeyboardService.log(Log.VERBOSE, "Post execute", "result = "+result);
	    	populateList();
	    }
	}
	
	/**
	 * Delete a dictionary file from the device. Will delete the associated auto-correct dictionary as well if there are
	 * any to be deleted
	 * 
	 * @param name the file name of the dictionary to delete
	 */
	public void delete(String name) {
		try {
			IKnowUKeyboardService.log(Log.VERBOSE, "Downloader delete word", "dict count = "+this.installedDictCount);
			if (this.installedDictCount <= 1) {
				Toast.makeText(this, "You must have at least one dictionary", Toast.LENGTH_LONG).show();
			} else {
				IKnowUKeyboardService.log(Log.VERBOSE, "Downloader delete word", "deleting = "+name);
				String folder = IKnowUKeyboardService.filesDir + "/dictionary/";
				File pd = new File(folder + name +".dict");
				File ad = new File(folder + name +".aac");
				
				pd.delete();
				ad.delete();
				pd = null;
				ad = null;
				
				this.predEngine.resetDictionaryConfiguration();
				this.refresh();
			}
		} catch (Exception e) {
			IKnowUKeyboardService.sendErrorMessage(e);
		}
	}
	
	/**
	 * Close this activity, saves the dictionary settings out to the engines
	 * 
	 * @param view
	 */
	public void close(View view) {
		saveDictSettings();
        //predictionEngine.reset(true);
        if (this.predEngine != null)
            this.predEngine.resetDictionaryConfiguration();
        this.finish();
	}
	
	/**
	 * Save the dictionary settings out to the engines
	 */
	private void saveDictSettings() {
		try {
			if (this.dictList != null) {
				boolean autoDictSet = false;
                boolean layoutSet = false;
				for (int i = 0; i < this.dictList.getChildCount(); i++) {
					DictionaryItem item = (DictionaryItem) this.dictList.getChildAt(i);
					UserDictionary userDict = this.checkForDict(item.getIndex());
					//if the user has this dictionary, then set the appropriate settings
					if (userDict != null) {
						IKnowUKeyboardService.log(Log.INFO, "WordLogic", "saveDictSettings:DICT NAME = "+item.getDictName()+ ",Priority = "+(i+1)+", ENABLED = "+item.getEnabled());
						this.predEngine.setDictSetting(userDict.langIdx, i+1, item.getEnabled());
						
						IKnowUKeyboardService.log(Log.INFO, "WordLogic", "hasAutoDict = "+item.getHasAutoDict()+", autoDictSet = "+autoDictSet+", layoutSet = "+layoutSet+", maxPriority = "+item.getMaxPosition());
						//set the highest dictionary to the top that is enabled to be the current autocorrect dictionary
						if (!autoDictSet && item.getEnabled()) {
							if (item.getHasAutoDict()) {
								this.changeAutoDict(item, layoutSet);
                                layoutSet = true;
								autoDictSet = true;
							} else if ( !layoutSet && item.getEnabled() && item.getMaxPosition() <= 1) {
                                layoutSet = true;

                                SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(this);

                                Editor edit = sp.edit();
                                edit.putBoolean(IKnowUKeyboardService.PREF_KB_LAYOUT_CHANGED, true);
                                edit.putString(IKnowUKeyboardService.PREF_KEYBOARD_LAYOUT, item.getLayout()+"");
                                IKnowUKeyboardService.log(Log.VERBOSE, "Changing kb layout ", "to -> "+item.getLayout());
                                edit.commit();
                            }
						} else if ( !layoutSet && item.getEnabled() && item.getMaxPosition() <= 1) {
                            layoutSet = true;

                            SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(this);

                            Editor edit = sp.edit();
                            edit.putBoolean(IKnowUKeyboardService.PREF_KB_LAYOUT_CHANGED, true);
                            edit.putString(IKnowUKeyboardService.PREF_KEYBOARD_LAYOUT, item.getLayout()+"");
                            IKnowUKeyboardService.log(Log.VERBOSE, "Changing kb layout ", "to -> "+item.getLayout());
                            edit.commit();
                        }
					}
				}
				//if still not set then use default
				if (!autoDictSet) {
					//DictionaryItem first = 
					this.changeAutoDict( (DictionaryItem) this.dictList.getChildAt(0), layoutSet );
				}
			}
		} catch (Exception e) {
			IKnowUKeyboardService.sendErrorMessage(e);
		}
	}
	
	/**
	 * Change the current auto-correct dictionary to a different one
	 * 
	 * @param item the item to change the auto-correct dictionary to
	 */
	private void changeAutoDict(DictionaryItem item, boolean layoutSet) {
		try {
			String destination = IKnowUKeyboardService.filesDir + "/dictionary/";
			String autoDict = destination + item.getLocalFileName() + ".aac";
			IKnowUKeyboardService.log(Log.VERBOSE, "Changing auto correct dict to ", "= "+autoDict);
			
			SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(this);
			
			Editor edit = sp.edit();
            if (!layoutSet) {
                IKnowUKeyboardService.log(Log.VERBOSE, "Changing kb layout ", "to -> "+item.getLayout());
                edit.putBoolean(IKnowUKeyboardService.PREF_KB_LAYOUT_CHANGED, true);
                edit.putString(IKnowUKeyboardService.PREF_KEYBOARD_LAYOUT, item.getLayout()+"");
            }
			edit.putString(IKnowUKeyboardService.PREF_AUTO_CORRECT_DICT, autoDict);
			
			edit.commit();
			
		} catch (Exception e) {
			IKnowUKeyboardService.sendErrorMessage(e);
		}
	}
}
