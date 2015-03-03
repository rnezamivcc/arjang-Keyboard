package com.iknowu;

import android.annotation.SuppressLint;
import android.annotation.TargetApi;
import android.app.AlertDialog;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.ClipData;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.content.res.AssetManager;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.graphics.Rect;
import android.inputmethodservice.InputMethodService;
import android.inputmethodservice.Keyboard;
import android.media.AudioManager;
import android.media.SoundPool;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Environment;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.Vibrator;
import android.preference.PreferenceManager;
import android.provider.Settings;
import android.text.ClipboardManager;
import android.text.InputType;
import android.text.SpannableString;
import android.text.method.MetaKeyKeyListener;
import android.util.Log;
import android.util.Xml;
import android.view.Display;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.view.inputmethod.CompletionInfo;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;
import android.widget.LinearLayout.LayoutParams;
import android.widget.Toast;

import com.iknowu.asian.Hangul;
import com.iknowu.downloader.DownloadActivity;
import com.iknowu.downloader.UserDictionary;
import com.iknowu.miniapp.MiniAppDetector;
import com.iknowu.miniapp.MiniAppManager;
import com.iknowu.miniapp.MiniAppMessageReceiver;
import com.iknowu.miniapp.MiniAppScreen;
import com.iknowu.popup.PopupManager;
import com.iknowu.setup.TutorialActivity;
import com.iknowu.sidelayout.SideRelativeLayout;
import com.iknowu.util.MessageLogger;
import com.iknowu.util.MiniAppContextEngine;
import com.iknowu.util.Size;
import com.iknowu.util.Theme;
import com.iknowu.voice.VoiceInputLinearLayout;
import com.parse.FindCallback;
import com.parse.LogInCallback;
import com.parse.Parse;
import com.parse.ParseException;
import com.parse.ParseObject;
import com.parse.ParsePush;
import com.parse.ParseQuery;
import com.parse.ParseUser;
import com.parse.PushService;
import com.parse.SignUpCallback;
import com.wordlogic.lib.AutoCorrect;
import com.wordlogic.lib.KeyPosition;

import org.json.JSONException;
import org.json.JSONObject;
import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserFactory;
import org.xmlpull.v1.XmlSerializer;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.PrintWriter;
import java.io.StringWriter;
import java.lang.ref.WeakReference;
import java.net.URL;
import java.net.URLConnection;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Locale;

import com.iknowu.R;

/**
 * The IKnowUKeyboardService is the main class of the keyboard. It is a subclass of android's {@link InputMethodService}
 * This class contains references to the {@link PredictionEngine} and {@link AutoCorrect} engines.
 * 
 * It handles all of the processing of all the text and delegates tasks to the appropriate places
 * 
 * @author Justin
 *
 */
@SuppressWarnings("deprecation")
public class IKnowUKeyboardService extends InputMethodService{

    //================================================================================================

    //!!!!!!!!!!!!!!!!!!!!!! Set this flag to true to enable logs !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	public static final boolean DEBUG = true;

    //!!!!!!!!!!!!!!!!!!!!!! Set this flag to true to enable reach !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    public static final boolean MINIAPP_ON = true;

    //!!!!!!!!!!!!!!!!!!!!!! Set this flag to true to enable phrase prediction !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //now set by a setting in the shared preferences
    public static boolean PHRASE_PREDICTION_ON;

    //================================================================================================

    public static final String TAG = "iKnowUKeyboardService";

    static final boolean PROCESS_HARD_KEYS = true;

	private static final int KBVIEW_AND_CANDLIST = 1;
	private static final int KBVIEW_NO_CANDLIST = 2;
	private static final int CANDLISTONLY = 3;

    public static final int DEFAULT_KEYBOARD_THEME_ID = R.xml.theme13;
	
	private static final int LONG_WORD_LENGTH = 50;
	public static final int EXTRA_LONG_WORD_LENGTH = 1000;
	private static final long UPDATE_INTERVAL = 43200000;
	//private static final long UPDATE_INTERVAL = 2000; //2 seconds
	public static final int NOTIFICATION_ID = 1234;
	
	private static final String PREF_LAST_CHECK_EXPIRARY = "com.iknowu.EXPIRARY";
	private static final String PREF_IS_EXPIRED = "com.iknowu.IS_EXPIRED";
	private static final long TRIAL_DURATION = 2592000000L;	//30 days
	//private static final long TRIAL_DURATION = 60000L;	//1 min

    public static boolean SWIPE_ENABLED = true;
	
	private int currentInputDisplayState = KBVIEW_AND_CANDLIST;
	private int changeOverInputDisplayState = KBVIEW_AND_CANDLIST;
	
	private static final String WL_DIR = "wordlogic/";
	
	private static Context context;
	
	public static String filesDir;

	//public View mInputViewContainer;
	//public IKnowUScroller ikuScroller;
    public KeyboardContainerView kbContainerView;
	//private CandidateView mCandidateView;
	public boolean mPredictionOn;
	public boolean mCompletionOn;
	public boolean mCapsLock;
	private long mLastShiftTime;
	private long mMetaState;
	
	public int currentKeyboardLayout;
	
	//public  boolean mPopupKeyboardActive = false;
	public boolean mLandscape = false;
	private LayoutParams mCandParams;

	private static PredictionEngine mPredictionEngine;
	private static AutoCorrect mAutoCorrect;
    private MiniAppContextEngine miniAppContextEngine;

	Keyboard current;
	private String mWordSeparators;
	static final int KEYCODE_OPTIONS = -100;
	private AlertDialog mOptionsDialog;
	public String[] mkey;
	
	public boolean mAutoSpaceOn;
	private boolean mVibrateOn;
	private Vibrator mVibrator;
	private boolean mSoundOn;
	private long mVibrateDuration;
	private float mSoundIntensity;
	public boolean mHighlightedKeysOn;
	public boolean mAutoCapOn;
	public boolean m_showTouchPointsOn;
	public boolean mHandednessRight;
	public boolean mInlineAdditionOn;
	public boolean mAutoLearnOn;
	public boolean mLargeKeysOn;
    public boolean tabsOnRight;
	
	//private String[][] suggestionsAr;
	
	public boolean firstTime;		//a flag to indicate a first time user of the keyboard
	public boolean settingsOpen;
	
	public double mScreenInches;
	public int displayWidth;
	
	public boolean bRemoveTrailingTextOnUpdate = false;
	
	private int selectionStart;
	private int selectionEnd;
	
	// keep track whether the last space entered is explicit or implicit (punctuation stuff)
	//public boolean mImplicitSpace = false;
	
	// keep track whether the cursor is repositioned due to characters movements or arrow or touch movements
	public int nonCursorRepositionUpdates = 0;

	public static final int KEYCODE_ENTER = 10;
	private static final String PREF_VIBRATE_SETTING = "vibrateListPref";
	private static final String PREF_SOUND_SETTING = "soundListPref";
	private static final String PREF_AUTO_CAP = "autocap_on";
	private static final String PREF_AUTO_SPACE = "autospace_on";
	private static final String PREF_TEXTTOSPEECH_ON = "texttospeech_on";
	private static final String PREF_HANDEDNESS_RIGHT = "handedness_right";
	private static final String PREF_HIGHLIGHTEDKEYS_ON = "highlightedkeys_on";
	private static final String PREF_INLINE_ADDITION_ON = "inlineaddition_on";
	private static final String PREF_AUTOLEARN_ON = "autolearn_on";
	private static final String PREF_LARGEKEYS_ON = "largekeys_on";
	private static final String PREF_LONG_PRESS_TIMEOUT = "long_press_timeout";
    private static final String PREF_DELETE_WORD_DELAY_TIMEOUT = "delete_word_delay_timeout";
	private static final String PREF_FIRST_TIME = "first_time_user";
	private static final String PREF_THEME_SETTING = "themeListPref";
	private static final String PREF_SIGNUP_FIRST = "firstSignUpValue";
	private static final String PREF_SIGNUP_SECOND = "secondSignUpValue";
	private static final String PREF_CLOUD_SYNC_ON = "cloudSyncOn";
	public static final String PREF_TABLET_LAYOUT = "tabletLayoutListPref";
	public static final String PREF_KEYBOARD_LAYOUT = "keyboard_layout";
	private static final String PREF_ASKED_FOR_RATING = "asked_for_rating";
	public static final String PREF_KB_FONT_STYLE = "keyboard_font_style";
	public static final String PREF_SPACELESS_TYPING = "spaceless_typing";
    public static final String PREF_SWIPE_ENABLED = "swipe_enabled";
	public static final String PREF_PHRASE_PREDICTIONS = "phrase_predictions";
    public static final String PREF_TABS_SIDE = "tabs_side";
	
	private static final String PREF_DOUBLE_SPACE = "double_space_period";
	public static final String PREF_SETUP_COMPLETE = "setup_complete";
	private static final String PREF_COPY_PERSONAL = "copied_personal";
	private static final String PREF_PERSONAL_TO_CLOUD = "personal_to_cloud";
	
	private static final String PLAIN_TEXT_CLIP_DATA = "iknowu_plain_text";
	private static final String PICTURE_CLIP_DATA = "iknowu_plain_text";
	private static final String URI_CLIP_DATA = "iknowu_plain_text";
	public static final String PREF_AUTOCORRECT_ON = "autocorrect_on";
	public static final String PREF_AUTOCORRECT_INSERT = "autocorrect_insert_auto";
	
	public static final String PREF_KB_LAYOUT_CHANGED = "kb_layout_changed";
	public static final String PREF_KEY_HEIGHT = "key_height";
	public static final String PREF_CORRECTION_FEEDBACK = "correction_feedback";
	
	public static final String PREF_AUTO_CORRECT_DICT = "auto_correct_dict_new";
	private static final String PREF_LAST_CHECK_FOR_UPDATES = "last_check_for_updates";
	
	private static final String PREF_FLASHED_TRACKER_FIRST_TIME = "flashed_tracker_first_time";
	public static final String PREF_VERSION_NAME = "version_name";
	
	private static final String PREF_FERRARI_INSTALL = "ferrari_install_tracking";
	
	public boolean cloudSyncOn;
	public static ParseUser currentUser;
	
	private boolean mTrialExpired = false;
	
	public String lmkey[];
	
	//public boolean tabletMode;
	public boolean tabletSplitView;
	public boolean tabletThumbView;
	
	public boolean onCreateCalled;
	
	public int themeId;
	
	private static ArrayList<String> addedWordsList;		//this will hold our added words until it gets to a certain amount, we can reduce the number of network calls this way
	private static ArrayList<String> deletedWordsList;		//this will hold our deleted words until it gets to a certain amount, we can reduce the number of network calls this way
	
	private SoundPool soundPool;
    private HashMap<Integer, Integer> soundPoolMap;
    
    private int editorAction;
    
    public boolean doubleSpacePeriod;
    private boolean setupComplete;
    
    private boolean isInitialized;
	
    private String wordBeforeCorrection;
    private boolean previousCanBeAdded;
    
    private boolean isEmailField;
    private boolean isPasswordField;
    private boolean autocorrectOn;
    private boolean insertCorrection;
    private boolean correctionFeedback;
    private boolean miniAppTextSent;
    
    private boolean kbLayoutChanged;
    public boolean suggestionPicked;
    private boolean removeExtraSpaces;


    private MiniAppMessageReceiver miniAppMessageReceiver;
    private MiniAppManager miniAppManager;
    private IntentFilter packageFilter;
    private MiniAppDetector miniAppDetector;
    public static boolean keepCurrentMiniApp;
    //public IKnowULocationManager locationManager;

    public static InputConnection currentInputConnection;
    
    public static final String CHECK_FOR_MINI_APPS = "com.iknowu.miniapp.CHECK_FOR_NEW";
    public static final String UPDATE_MINI_APP = "com.iknowu.miniapp.UPDATE";
    public static final String CLIP_TEXT = "com.iknowu.miniapp.CLIP_TEXT";
    
    public static final int MSG_UPDATE_MINIAPP_VIEW = 0;
    public static final int MSG_CLIP_TEXT = 1;
    public static final int MSG_CLOSE_MINIAPP = 2;
    public static final int MSG_DELETE_CHARS = 3;
    
    public StringBuilder currentWord;
    public float keyHeightDP;
    
	public PopupManager popupManager;
	
	private Long timerStartTime;
	private int keysAheadOfSystem;
	
	public Hangul currentHangul;
	public Hangul previousHangul;
	
	/**
	 * A handler to pass information from our Broadcast receiver to the keyboard
	 * @author Justin
	 *
	 */
	public static class MiniAppHandler extends Handler {
		private final WeakReference<IKnowUKeyboardService> mService;

		MiniAppHandler(IKnowUKeyboardService service) {
			mService = new WeakReference<IKnowUKeyboardService>(service);
		}

		@Override
		public void handleMessage(Message msg) {
			IKnowUKeyboardService service = mService.get();
			if (service != null) {
				switch (msg.what) {
				case MSG_UPDATE_MINIAPP_VIEW:
					service.updateMiniAppView();
					break;
				case MSG_CLIP_TEXT:
					service.copy("");
					break;
				case MSG_CLOSE_MINIAPP:
					service.closeMiniApp();
					break;
				case MSG_DELETE_CHARS:
					service.deleteChars(msg.arg1, msg.arg2);
					break;
				}
			}
		}
	}
    
    public IKnowUKeyboardService.MiniAppHandler miniAppHandler;

	public boolean capitalizeFirstLetter;
    
    public boolean correctionMade;
    public int cursorPosCorrection;
    public int correctionStart;
    public String wordToCorrect;
    public String lastCorrection;
    public int lastCorrectionTaken;
    
    public int numNextWords;
    private boolean showOnlyPreds;
    
    //all variables for stat tracking
    private int totalCharsTyped;
    private int totalPredsSelected;
    private int totalCorrectionsInserted;
    private int totalKeyStrokesSaved;
    private long totalTimeOpen;
    private long startTime;
    private int totalWordsTyped;
    
    private boolean configChanged;

    //========================================================================================
  	//CONTRUCTORS
  	//========================================================================================
    public IKnowUKeyboardService() {
    	super();
    }
    
    public IKnowUKeyboardService getInstance() {
    	return this;
    }
    
	/**
	 * Main initialization of the input method component. Be sure to call to
	 * super class.
	 */
	@Override
	public void onCreate() {
		try {
			super.onCreate();
			
			context = this;
			this.onCreateCalled = true;
			
			addedWordsList = new ArrayList<String>();
			deletedWordsList = new ArrayList<String>();
			mWordSeparators = getResources().getString(R.string.word_separators);

            File internal = this.getFilesDir();
            filesDir = internal.getAbsolutePath()+"/wordlogic";
            //filesDir = "/data/data/com.iknowu/files/wordlogic";

			//copy all the existing and newly provided files to the appropriate places
			this.copyAssets();
			try {
				mPredictionEngine = new PredictionEngine();
                this.startTimer();
                this.isInitialized = mPredictionEngine.initialize(filesDir);
                this.stopTimer("onCreate() --> initialize prediction engine");
			} catch (Exception e) {
				sendErrorMessage(e);
			}
			//Log.d("Path to autocorrect", ""+sdcardwordlogicdictionary.getPath());
			SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(this);
			//autoCorrectEngine = new AutoCorrect(this);
			//autoCorrectEngine.loadCustomFromFile();
			
			try {
			//	String autoDict = sp.getString(PREF_AUTO_CORRECT_DICT, filesDir+"/dictionary/english.aac");
			//	log(Log.VERBOSE, "auto correct pref", "= "+autoDict);
             //   this.startTimer();
                mAutoCorrect = new AutoCorrect();
            //    this.stopTimer("onCreate() --> initialize autocorrect engine");
		//		mAutoCorrect.callback = this;
			} catch (IllegalArgumentException iae) {
                // the file we have pointed to as the auto-correct dictionary is not available, so turn auto-correct off.
                //Toast.makeText(this, this.getResources().getString(R.string.file_not_found) + "code = "+1003, Toast.LENGTH_LONG).show();
                this.autocorrectOn = false;
            } catch (Exception e) {
				sendErrorMessage(e);
			}

            this.miniAppContextEngine = new MiniAppContextEngine();
			
			//initialize parse to connect with the application
			Parse.initialize(this, "j17LUua9VZ96CCEAP1QKimCoUuGlOfKRYnyLoqWv", "C7QJdvSFey3DwfZSCSqMaC2hjWFZwvQCr9ruiOoE");
			
			//initialize the flurry analytics
			//FlurryAgent.onStartSession(this, "Z69X2MN55Q5BJKJH3VJ4");
            if (MINIAPP_ON) {
                this.doBindService();

                this.miniAppManager = new MiniAppManager(this);
                this.miniAppDetector = new MiniAppDetector();
                this.miniAppDetector.setKeyboardService(this);

                this.packageFilter = new IntentFilter();
                this.packageFilter.addAction( Intent.ACTION_PACKAGE_ADDED  );
                this.packageFilter.addAction( Intent.ACTION_PACKAGE_REPLACED );
                this.packageFilter.addAction( Intent.ACTION_PACKAGE_REMOVED );
                this.packageFilter.addCategory( Intent.CATEGORY_DEFAULT );
                this.packageFilter.addDataScheme( "package" );

                registerReceiver( this.miniAppDetector, this.packageFilter );

                this.miniAppHandler = new IKnowUKeyboardService.MiniAppHandler(this);
                //this.checkForMiniApps();
            }
			
			/*
			 * Set up audio playback, this is clean now
			 * audio stream type set to the system stream,
			 * and will be partly dependent on the volume set there
			 */
			soundPool = new SoundPool(4, AudioManager.STREAM_SYSTEM, 100); 
	        soundPoolMap = new HashMap<Integer, Integer>();
	        soundPoolMap.put(1, soundPool.load(this, R.raw.click, 1));
	        soundPoolMap.put(2, soundPool.load(this, R.raw.error, 1));
	        
			
			this.firstTime = sp.getBoolean(PREF_FIRST_TIME, true);
			
			if (this.firstTime) {
				SharedPreferences.Editor editor = sp.edit();
				editor.putBoolean(PREF_FIRST_TIME, false);
				// Commit the edits!
				editor.commit();
			}
			
			//this.locationManager = new IKnowULocationManager(this);
			
			popupManager = new PopupManager(this);
			
			boolean shouldCopy = sp.getBoolean(PREF_PERSONAL_TO_CLOUD, true);
			if (shouldCopy) {
				String[] words = mPredictionEngine.getPersonalDictWords();
				
				for (int i=0; i < words.length; i++) {
					log(Log.VERBOSE, "Adding personal to cloud", "word["+i+"] = "+words[i]);
					addedWordsList.add(words[i]);
				}
				
				trySendCloudMessages("");
				
				SharedPreferences.Editor editor = sp.edit();
				editor.putBoolean(PREF_PERSONAL_TO_CLOUD, false);
				// Commit the edits!
				editor.commit();
			}
			
			this.currentHangul = new Hangul();
			
		} catch (Exception e) {
			sendErrorMessage(e);
		}
	}
	
	/**
	 * Establish a connection between the broadcast receiver and this
	 */
	private void initMiniAppReceiver() {
        if (MINIAPP_ON)
		    this.miniAppMessageReceiver.setKeyboardService(this);
	}
	
	/**
	 * The actual {@link ServiceConnection} that we will bind to
	 * For use in Reach communication
	 */
	private ServiceConnection mConnection = new ServiceConnection() {
	    public void onServiceConnected(ComponentName className, IBinder service) {
            if (MINIAPP_ON) {
                log(Log.VERBOSE, "Connected to service", "MiniappMessageReceiver");
                if (service instanceof MiniAppMessageReceiver.LocalBinder) {
                    miniAppMessageReceiver = ((MiniAppMessageReceiver.LocalBinder) service).getService();
                    miniAppMessageReceiver.isBound = true;
                    initMiniAppReceiver();
                }
            }
	    }

	    public void onServiceDisconnected(ComponentName className) {
	    	log(Log.VERBOSE, "Disconnected from service", "MiniappMessageReceiver");
            if (miniAppMessageReceiver != null) {
                miniAppMessageReceiver.isBound = false;
                miniAppMessageReceiver = null;
            }
	    }
	};
	
	/**
	 * Bind to our {@link ServiceConnection}
	 */
	private void doBindService() {
        if (MINIAPP_ON) {
            // Establish a connection with the service.  We use an explicit
            // class name because we want a specific service implementation that
            // we know will be running in our own process (and thus won't be
            // supporting component replacement by other applications).

            // if the connection is already bound, unbind it and create a new one
            if (this.miniAppMessageReceiver != null && this.miniAppMessageReceiver.isBound) {
                this.doUnbindService();
            }
            this.bindService(new Intent(MiniAppMessageReceiver.LOCAL_BIND), mConnection, Context.BIND_AUTO_CREATE);
        }
	}

	/**
	 * Unbind from our {@link ServiceConnection}
	 */
	private void doUnbindService() {
	    if (this.miniAppMessageReceiver != null && this.miniAppMessageReceiver.isBound) {
	        // Detach our existing connection.
	        this.unbindService(mConnection);
	        //mConnection = null;
	    }
	}

	@Override
	public void onDestroy() {
		//FlurryAgent.onEndSession(this);
		try {
			if (this.soundPool != null) {
				this.soundPool.release();
				this.soundPool = null;
			}
			
			if (this.cloudSyncOn) {
				if ( addedWordsList.size() > 0 ) {
					addWordsToCloud();
				}
				if ( deletedWordsList.size() > 0 ) {
					deleteWordsToCloud("");
				}
			}
            if (MINIAPP_ON)
			    this.unregisterReceiver(this.miniAppDetector);
			this.doUnbindService();
			super.onDestroy();
		} catch (Exception e) {
			sendErrorMessage(e);
		}
	}
	
	/**
	 * helper to make enabling and disabling logging easier
	 * @param tag
	 * @param message
	 */
	public static void log(int type, String tag, String message) {
		if (DEBUG) {
			switch (type) {
				case Log.DEBUG:
					Log.d(tag, message);
					break;
				case Log.VERBOSE:
					Log.v(tag, message);
					break;
				case Log.ERROR:
					Log.e(tag, message);
					break;
				case Log.INFO:
					Log.i(tag, message);
					break;
				case Log.WARN:
					Log.w(tag, message);
					break;
			}
		}
	}
	
	/**
	 * Copy the the contents of the assets dir (dictionaries, setup files, etc.) to the sd card if they aren't already there
	 */
	private void copyAssets() {
		try {
			AssetManager assetManager = getAssets();
			
		    final String wl = "wordlogic";
		    
		    //Copy any existing files in the wordlogic directory on an external
		    //location to the new internal files location.
		    File internalDir = new File(filesDir, "/dictionary/");
		    internalDir.mkdirs();
		    File extDir = Environment.getExternalStorageDirectory();
		    File externalDict = new File(extDir, wl + "/dictionary/");
		    if (externalDict.exists()) {
			    String[] extFiles = externalDict.list();
			    
			    if (extFiles != null) {
			    	for (int i=0; i < extFiles.length; i++) {
			    		
			    		File existing = new File(externalDict, extFiles[i]);
			    		File internal = new File(internalDir, extFiles[i]);
			    		log(Log.VERBOSE, "IKnowUKB", "copying existing wordlogic dir, currentFile = "+existing);
				        InputStream in = null;
				        OutputStream out = null;
				        
				        log(Log.VERBOSE, "copying to ", "file = "+internal.getAbsolutePath());
				        
				        in = new FileInputStream(existing);
				        out = new FileOutputStream(internal);
				        copyFile(in, out);
				        in.close();
				        in = null;
				        out.flush();
				        out.close();
				        out = null;
			    	}
			    }
		    }
			File sdcardwordlogic = new File(extDir, wl);
			
			if (sdcardwordlogic.exists()) {
				log(Log.INFO, "Removing all traces of old directory", "deleting dir = "+sdcardwordlogic);
				this.deleteDirectory(sdcardwordlogic);
			}
			
			if (internalDir.exists()) {
				
			    String[] files = null;
			    files = assetManager.list("");
			    
			    for(int i=0; i<files.length; i++) {
			    	boolean wlDictFile = (files[i].endsWith(".txt")  || files[i].endsWith(".xml") || files[i].endsWith(".ldat") );
			    	boolean wlDictConfigFile = files[i].endsWith(".wlkb");
			    	if ((!wlDictFile && !wlDictConfigFile))	{
			    		continue;
			    	}

			    	File testFile = new File( internalDir, files[i]);

                    log(Log.VERBOSE, "copyAssets()", "file name = "+testFile.getName());

                    if (testFile.exists() && testFile.getName().equals("abbreviations.xml")) {
                        continue;
                    }

			    	if (testFile.exists() && !wlDictConfigFile)	{
			    		testFile.delete();
			    		//Log.i("CopyAssets", "deleting file "+testFile.getPath()+", returns "+del);
			    	}
			    	
			        InputStream in = null;
			        OutputStream out = null;
			        
			        log(Log.VERBOSE, "copyAssets()", "file = "+testFile.getAbsolutePath()+" exists before copy = "+ testFile.exists());
			        
			        in = assetManager.open(files[i]);
			        out = new FileOutputStream(testFile);
			        copyFile(in, out);
			        in.close();
			        in = null;
			        out.flush();
			        out.close();
			        out = null;

                    log(Log.VERBOSE, "copyAssets()", "file = "+testFile.getAbsolutePath() + " exists after copy = "+testFile.exists());
			    }
	    	}
		} catch (Exception e) {
			sendErrorMessage(e);
		}
	}
	
	/**
	 * Recursively delete all contents of a directory
	 * @param path the directory to delete
	 * @return true on completion
	 */
	private boolean deleteDirectory(File path) {
		if (path.exists()) {
			File[] files = path.listFiles();
			for (int i = 0; i < files.length; i++) {
				if (files[i].isDirectory()) {
					deleteDirectory(files[i]);
				} else {
					log(Log.VERBOSE, "deleting file", "= "+files[i]);
					files[i].delete();
				}
			}
		}
		return (path.delete());
	}
	
	/**
	 * Copy a file from an {@link InputStream} to an {@link OutputStream}
	 * @param in the file to be copied
	 * @param out the file to copy to
	 * @throws IOException
	 */
	private void copyFile(InputStream in, OutputStream out) throws IOException {
	    byte[] buffer = new byte[1024];
	    int read;
	    while((read = in.read(buffer)) != -1){
	      out.write(buffer, 0, read);
	    }
	}
	
	@Override
	public void onConfigurationChanged(Configuration newConfig) {
		try {
			log(Log.WARN, "ON CONFIG CHANGED", "HERE");
			this.configChanged = true;
			this.commitCurrentWord(this.getCurrentInputConnection(), "onconfigchanged");
			if (this.kbContainerView != null) {
				this.kbContainerView.onConfigurationChanged(newConfig);			}
			/*if (ikuScroller != null) {
				ikuScroller.setOnTouchListener(null);
			}*/
			if (this.popupManager != null) {
				this.popupManager.dismissPopupWindow();
			}
			
			super.onConfigurationChanged(newConfig);
		} catch (Exception e) {
			sendErrorMessage(e);
		}
	}
	
	/**
	 * This is the point where you can do all of your UI initialization. It is
	 * called after creation and any configuration change.
	 */
	@Override
	public void onInitializeInterface() {
		log(Log.WARN, "ON INITIALIZE INTERFACE", "FUNCTION START");
		try {
			Display display = ((WindowManager) getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay();
			
			int width = display.getWidth();
			this.displayWidth = width;
			int height = display.getHeight();
			//Log.d("DISPLAY WIDTH x HEIGHT =", ""+width+" x "+height);
			mLandscape = width > height;
			mScreenInches = Size.calculateScreenInches(this);

            mTrialExpired = false;

		} catch (Exception e) {
			sendErrorMessage(e);
		}
	}
	
	/**
	 * This is the main point where we do our initialization of the input method
	 * to begin operating on an application. At this point we have been bound to
	 * the client, and are now receiving all of the detailed information about
	 * the target of our edits.
	 */
	@Override
	public void onStartInput(EditorInfo attribute, boolean restarting) {
		
		log(Log.WARN, "ON START INPUT", "HERE");
		super.onStartInput(attribute, restarting);
		//check for type null, don't do anything if this is the case, android calls this function whenever it feels like
		//or so it seems
		if (attribute.inputType != EditorInfo.TYPE_NULL) {
			this.initializeInput(attribute, restarting);
		}
	}
	
	/**
	 * Inflate the appropriate layout file for displaying our keyboard
	 * @return The inflated view hierarchy
	 */
	private View inflateKeyboardLayout() {
		try {
			log(Log.WARN, "INFLATE KB LAYOUT", "HERE");
			this.kbContainerView = new KeyboardContainerView(this);

			log(Log.VERBOSE, "inflate", "current kb layout = "+this.currentKeyboardLayout);
            if (this.currentKeyboardLayout == KeyboardLinearLayout.COMPRESSED) {
                if (this.tabletThumbView) this.kbContainerView.init(this, R.layout.compressed_keyboard_view, this.themeId, KeyboardLinearLayout.MODE_TABLET_THUMB, !this.mHandednessRight, !this.tabsOnRight);
                else this.kbContainerView.init(this, R.layout.compressed_keyboard_view, this.themeId, KeyboardLinearLayout.MODE_TABLET_FULL, !this.mHandednessRight, !this.tabsOnRight);
            } else {
                if (this.tabletThumbView) this.kbContainerView.init(this, R.layout.keyboard_view, this.themeId, KeyboardLinearLayout.MODE_TABLET_THUMB, !this.mHandednessRight, !this.tabsOnRight);
                else this.kbContainerView.init(this, R.layout.keyboard_view, this.themeId, KeyboardLinearLayout.MODE_TABLET_FULL, !this.mHandednessRight, !this.tabsOnRight);
            }
            //log(Log.ERROR, "inflateKeyBoardLayout()", "setting theme to = "+this.themeId);
			//this.ikuScroller.setMiniAppTheme(this.themeId);
            if (MINIAPP_ON) {
                this.miniAppManager.setMiniAppScreen(this.kbContainerView.getSideRelativeLayout().getMiniAppScreen());
                this.miniAppManager.setKeyboardScreen(this.kbContainerView.getKbLinearLayout());
                //this needs to come after the screen and drawer have been set.
                this.miniAppManager.detectMiniAppList();
            }
			
			return this.kbContainerView;
		} catch (Exception e) {
			sendErrorMessage(e);
			return new View(this);
		}
	}
	
	
	/**
	 * Wraps all of the functionality that should be in onStartInput.
	 * Need to do this because some apps specify a type of null even when they actually want to
	 * have an input connection. If this is the case, startInputView will detect a type of null
	 * and call this function
	 */
	private void initializeInput(EditorInfo attribute, boolean restarting) {
		try {
			log(Log.WARN, "INITIALIZE INPUT", "HERE");
			if (!this.isInitialized) {
				this.isInitialized = mPredictionEngine.initialize(filesDir);
			}
			
			this.loadSettings();
			
			int inputType = attribute.inputType & EditorInfo.TYPE_MASK_CLASS;
			
			this.currentWord = new StringBuilder();
			
			//Log.d("ON START INPUT", "REstarting = "+restarting);
			//if the editor is reporting that it is restarting then we know the keyboard is in fact already
			//showing and therefore do not need to do any initialization at this point
			if (!restarting) {
				// Clear shift states.
				mMetaState = 0;
				
				mPredictionOn = false;
				mCompletionOn = false;
				this.removeExtraSpaces = false;
				mPredictionEngine.reset(true);
				mPredictionEngine.ResetTGraphHistory();
				this.updatePredEngineHistoryLatin(false);
				
				//this is now needed to display the correct layout for tablets, because the user
				//could have switched split view on or off
				//also for different language layouts, need to load appropriate dictionaries
				if (this.kbLayoutChanged) {
					SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(this);
					
					this.initializeAutoCorrect();
				}
				if (mAutoCorrect != null)
					mAutoCorrect.replaceBuffers("", "");
				this.setInputView(this.inflateKeyboardLayout());
			}//end if not restarting
			
			if (this.kbContainerView == null) {
				this.inflateKeyboardLayout();
                this.setInputView(this.kbContainerView);
			}

			if (this.kbContainerView.getSideRelativeLayout().getMiniAppScreen() != null) {
                this.kbContainerView.getSideRelativeLayout().getMiniAppScreen().requestLayout();
			}
			
			//updateCandidates();
			// We are now going to initialize our state based on the type of
			// text being edited.
			switch (inputType) {
			case EditorInfo.TYPE_CLASS_NUMBER:
				//Log.d("INPUT TYPE =", "TYPE CLASS NUMBER");
                this.setKeyboard(KeyboardLinearLayout.NUMERIC, false, false, false);
				//this.setPopupManagerKeyboardLayout();
				//this.ikuScroller.setKeyboardLayout(KeyboardScreen.NUMERIC);
				this.removeExtraSpaces = true;
				mPredictionOn = true;
				break;
			case EditorInfo.TYPE_CLASS_DATETIME:
				//Log.d("INPUT TYPE =", "TYPE CLASS DATETIME");
				// Numbers and dates default to the numeric keyboard, with
				// no extra features.
                this.setKeyboard(KeyboardLinearLayout.NUMERIC, false, false, false);
				//this.setPopupManagerKeyboardLayout();
				//this.ikuScroller.setKeyboardLayout(KeyboardScreen.NUMERIC);
				this.removeExtraSpaces = true;
				break;
			case EditorInfo.TYPE_CLASS_PHONE:
				//Log.d("INPUT TYPE =", "TYPE CLASS PHONE");
				// Phones will also default to the numbers keyboard, though
				// often you will want to have a dedicated phone keyboard.
                this.setKeyboard(KeyboardLinearLayout.NUMERIC, false, false, false);
				//this.setPopupManagerKeyboardLayout();
				//this.ikuScroller.setKeyboardLayout(KeyboardScreen.NUMERIC);
				break;
			case EditorInfo.TYPE_CLASS_TEXT:
				//Log.d("INPUT TYPE =", "TYPE CLASS TEXT");
                this.setKeyboard(this.currentKeyboardLayout, false, false, false);
				//this.setPopupManagerKeyboardLayout();
				//this.ikuScroller.setKeyboardLayout(this.currentKeyboardLayout);
				
				mPredictionOn = true;
				this.isEmailField = false;
				this.isPasswordField = false;
				
				int variation = attribute.inputType & EditorInfo.TYPE_MASK_VARIATION;
				log(Log.VERBOSE, "Initialize Input", "Variation = "+variation);
				//Log.v("Variation =", ""+variation);
				if (variation == EditorInfo.TYPE_TEXT_VARIATION_PASSWORD || variation == EditorInfo.TYPE_TEXT_VARIATION_VISIBLE_PASSWORD || variation == EditorInfo.TYPE_TEXT_VARIATION_WEB_PASSWORD) {
					mPredictionOn = false;
					this.autocorrectOn = false;
					this.removeExtraSpaces = true;
					this.isPasswordField = true;
				}

				if (variation == EditorInfo.TYPE_TEXT_VARIATION_EMAIL_ADDRESS || variation == EditorInfo.TYPE_TEXT_VARIATION_WEB_EMAIL_ADDRESS) {
					//Log.d("TYPE VARIATION = ", "EMAIL || URI || VAR FILTER");
					//mPredictionOn = false;
					mPredictionOn = false;
					this.autocorrectOn = false;
					this.removeExtraSpaces = true;
				} else if ( variation == EditorInfo.TYPE_TEXT_VARIATION_URI || variation == EditorInfo.TYPE_TEXT_VARIATION_FILTER ) {
					mPredictionOn = true;
					this.autocorrectOn = false;
					this.removeExtraSpaces = true;
				} else if ( variation == EditorInfo.TYPE_TEXT_VARIATION_WEB_EDIT_TEXT ) {
					//keep this the same as any other edit text for now
                    //this.removeExtraSpaces = true;
				}

				if ((attribute.inputType & EditorInfo.TYPE_TEXT_FLAG_AUTO_COMPLETE) != 0) {
					//Log.d("AUTO COMPLETE", "ON");
					//mPredictionOn = false;
				}
				break;
			default:
                this.setKeyboard(this.currentKeyboardLayout, false, false, false);
				//this.setPopupManagerKeyboardLayout();
				//this.ikuScroller.setKeyboardLayout(this.currentKeyboardLayout);
			}
			
			if (!restarting) {
				updateShiftKeyState(attribute);
				updateAutoCorrectOnCursorPosition();
				updatePredictionEngineOnCursorPosition(false, true);
				updateCandidates();
			}
			
			// Update the label on the enter key, depending on what the application
			// says it will do.
			// also store this value so we know what to do when enter is pressed
			if (this.kbContainerView.getKbLinearLayout().getKeyboard() != null) {
				this.editorAction = this.kbContainerView.getKbLinearLayout().getKeyboard().setImeOptions(getResources(), attribute.imeOptions);
			}
		} catch (Exception e) {
			sendErrorMessage(e);
		}
	}
	
	/**
	 * Pass the keyboard layout to the {@link IKnowUScroller}
	 * @param layout
	 */
	public void setKeyboard(int layout, boolean setAsCurrent, boolean changeMode, boolean isSideView) {

        if (isSideView) {
            this.kbContainerView.getSideRelativeLayout().getSideKeyboardScreen().setKeyboard(layout);
            this.kbContainerView.getSideRelativeLayout().getSideKeyboardScreen().saveKeyboardTypePreference(layout);
        } else {
            this.popupManager.setAndLoadCurrentKeyboard(layout);
            this.kbContainerView.getKbLinearLayout().setKeyboard(layout, changeMode);

            if (setAsCurrent) {
				SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(this);
            	Editor edit = sp.edit();
            	edit.putString(PREF_KEYBOARD_LAYOUT, ""+layout);
            	edit.putBoolean(PREF_KB_LAYOUT_CHANGED, true);
            	edit.commit();
				this.currentKeyboardLayout = layout;
			}
        }
	}

    public void switchToCompressed() {
        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(this);
        Editor edit = sp.edit();
        edit.putString(PREF_KEYBOARD_LAYOUT, ""+KeyboardLinearLayout.COMPRESSED);
        edit.putBoolean(PREF_KB_LAYOUT_CHANGED, true);
        edit.commit();
        this.currentKeyboardLayout = KeyboardLinearLayout.COMPRESSED;
        this.inflateKeyboardLayout();
        this.setKeyboard(KeyboardLinearLayout.COMPRESSED, false, false, false);
        if (mAutoCorrect != null) {
            KeyPosition[] keypos = this.getKeyboardPositionListForAutoCorrect(this.kbContainerView.getKbLinearLayout().getKeyboard(), 500);
            if (keypos != null) mAutoCorrect.loadKeyboardLayout(keypos);
        }
        this.setInputView(this.kbContainerView);
    }

    public void switchToFullSize() {
        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(this);
        Editor edit = sp.edit();
        edit.putString(PREF_KEYBOARD_LAYOUT, ""+KeyboardLinearLayout.QWERTY);
        edit.putBoolean(PREF_KB_LAYOUT_CHANGED, true);
        edit.commit();
        this.currentKeyboardLayout = KeyboardLinearLayout.QWERTY;
        this.inflateKeyboardLayout();
        this.setKeyboard(KeyboardLinearLayout.QWERTY, false, false, false);
        if (mAutoCorrect != null) {
            KeyPosition[] keypos = this.getKeyboardPositionListForAutoCorrect(this.kbContainerView.getKbLinearLayout().getKeyboard(), 0);
            if (keypos != null) mAutoCorrect.loadKeyboardLayout(keypos);
        }
        this.setInputView(this.kbContainerView);
    }

	/**
	 * Called by the framework when your view for creating input needs to be
	 * generated. This will be called the first time your input method is
	 * displayed, and every time it needs to be re-created such as due to a
	 * configuration change.
	 */
	@Override
	public View onCreateInputView() {
        log(Log.WARN, "ON CREATE INPUT VIEW", "HERE");
        return this.inflateKeyboardLayout();
    }
	
	@Override
	public void onStartInputView(EditorInfo attribute, boolean restarting) {
		try {
			log(Log.WARN, "ON START INPUT VIEW", "HERE");
			log(Log.INFO, "onStartInputView", "inputType = "+attribute.inputType+", fieldId = "+attribute.fieldId+", packageName = "+attribute.packageName+", privateOptions = "+attribute.privateImeOptions
            +", label = "+attribute.label);
			int variation = attribute.inputType & EditorInfo.TYPE_MASK_VARIATION;
			log(Log.VERBOSE, "onStartInputView", "Variation = "+variation+", restarting = "+restarting);

            //log(Log.DEBUG, "onStartInputView", "current word = "+this.currentWord);
			//get rid of a composing word when a restart has been flagged.
			if (restarting && this.currentWord != null && this.currentWord.length() > 0) {
				this.deleteFromCurrentWord(0, this.currentWord.length(), false);
				//this.commitCurrentWord(this.getCurrentInputConnection(), "onStartInputView");
			}
			
			super.onStartInputView(attribute, restarting);
			/*
			 * Put this code back in to enabled trial period detection
			 */
			/*this.getexpirationStatus();
			if (!this.mTrialExpired && this.isExpirationTimerExpired() ) {
				this.isExpired();
		    } else if (this.mTrialExpired) {
		    	this.showTrialExpiredDialog();
		    }*/

			this.mTrialExpired = false;
			
			// this.locationManager.getLocation();
			//check to see how many times the keyboard has been opened, after a couple times
			//prompt the user for a rating
			SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(this);
			boolean askedfor = sp.getBoolean(PREF_ASKED_FOR_RATING, false);
			
			if (!restarting) this.loadStats();
			
			this.checkForDictUpdates();
			
			SharedPreferences.Editor editor = sp.edit();
			//Log.v("AskedFor = "+askedfor, "Opened = "+opened);
			if (!askedfor && this.totalCharsTyped >= 500) {
				this.promptForRating();
				editor.putBoolean(PREF_ASKED_FOR_RATING, true);
			}
			// Commit the edits!
			editor.commit();
			
			//if we signalled a config changed, then initialize our input again, as the function call order is
			//different in this case, resulting in a null keyboard if this is not done.
			if (this.configChanged) {
				this.configChanged = false;
				this.initializeInput(attribute, restarting);
			}
			
			//make sure the auto-correct engine has been initialized 
	//		if (mAutoCorrect == null) {
	//			this.initializeAutoCorrect();
	//		}
			
	//		if (mAutoCorrect != null && !restarting) {
	//			mAutoCorrect.setSettings(false, true);
	//		}
			
			this.keysAheadOfSystem = 0;
			
			// Apply the selected keyboard to the input view.
			//if the editor is reporting that it is restarting then we know the keyboard is in fact already
			//showing and therefore do not need to do any initialization at this point
			if (!restarting) {
                log(Log.VERBOSE, "onStartInputView()", "layoutChanged = " + this.kbLayoutChanged + ", newAutoCorrect = " +mAutoCorrect);
				if (this.kbLayoutChanged || this.onCreateCalled) {
					//autoCorrectEngine.loadKeyboardLayout(this.createKeyMatrixForAutoCorrect(mCurKeyboard));
                    try {
                        if (mAutoCorrect != null) {
                            KeyPosition[] keypos = new KeyPosition[0];

                            if ( this.currentKeyboardLayout == KeyboardLinearLayout.COMPRESSED || this.currentKeyboardLayout == KeyboardLinearLayout.FEATURE) {
                                keypos = this.getKeyboardPositionListForAutoCorrect(this.kbContainerView.getKbLinearLayout().getKeyboard(), 500);
                            } else {
                                keypos = this.getKeyboardPositionListForAutoCorrect(this.kbContainerView.getKbLinearLayout().getKeyboard(), 0);
                            }

                            if (keypos != null) {
                            	mAutoCorrect.loadKeyboardLayout(keypos);
                            }
                        }
                    } catch (IllegalStateException ise) {
                        //do nothing here, this needs to be fixed on auto-correct side
                    }
					this.kbLayoutChanged = false;
					this.onCreateCalled = false;
					sp = PreferenceManager.getDefaultSharedPreferences(this);
					Editor edit = sp.edit();
					edit.putBoolean(PREF_KB_LAYOUT_CHANGED, false);
					edit.commit();
				}
			}//end if not restarting
			
			if (this.kbContainerView.getKbLinearLayout().getKeyboardView() != null) {
                this.kbContainerView.getKbLinearLayout().getKeyboardView().clearNextKeyHighlighting();
				evaluateInputNeeds();
				rearrangeDisplay();
			}

            this.kbContainerView.resizeComponents();
			
			if (!restarting) {
				//load the correct resource for producing popup keys
				//this.setPopupManagerKeyboardLayout();
				this.setCurrentWord(this.getCurrentInputConnection());
				updateShiftKeyState(attribute);
				this.updateWordToCorrect();
				updateAutoCorrectOnCursorPosition();
				updatePredictionEngineOnCursorPosition(false, true);
				this.startTime = System.currentTimeMillis();
				updateCandidates();
			//need this check as web fields don't fire the restarting flag like other regular android edit_texts do
			} else if (variation == InputType.TYPE_TEXT_VARIATION_WEB_EDIT_TEXT || variation == InputType.TYPE_TEXT_VARIATION_WEB_EMAIL_ADDRESS
					|| variation == InputType.TYPE_TEXT_VARIATION_WEB_PASSWORD || variation == InputType.TYPE_TEXT_VARIATION_NORMAL
					|| variation == InputType.TYPE_TEXT_VARIATION_SHORT_MESSAGE || variation == InputType.TYPE_TEXT_VARIATION_LONG_MESSAGE) {
				//this.setPopupManagerKeyboardLayout();
				//this.setCurrentWord(this.getCurrentInputConnection());
				updateShiftKeyState(attribute);
				this.updateWordToCorrect();
				updateAutoCorrectOnCursorPosition();
				updatePredictionEngineOnCursorPosition(false, true);
				updateCandidates();
				mPredictionEngine.ResetTGraphHistory();
			}

            this.kbContainerView.restoreSideScreen();
		} catch (Exception e) {
			sendErrorMessage(e);
		}
	}
	
	/**
	 * This function will try to initialize the auto correct dictionary, if for some reason the dictionary
	 * that has been set via preferences fails, it will search the wordlogic directory for another auto-correct file
	 * and use the first one that it finds.
	 */
	private void initializeAutoCorrect() 
	{
		mAutoCorrect = new AutoCorrect();
	//	mAutoCorrect.callback = this;
	}
	
	/**
	 * When called, will show an {@link AlertDialog} to the user
	 * asking to have them give the app a rating on the Play Store
	 */
	private void promptForRating() {
		try {
			AlertDialog.Builder builder = new AlertDialog.Builder(this);
			builder.setMessage(R.string.prompt_rating_text);
			builder.setCancelable(true);
			builder.setIcon(R.drawable.iknowulogo);
			builder.setTitle(this.getResources().getString(R.string.ime_name));
			
			builder.setPositiveButton(R.string.label_ok_key, new OnClickListener() {
				@Override
				public void onClick(DialogInterface dialog, int which) {
					gotoMarket(getPackageName());
				}
			});
			
			builder.setNegativeButton(this.getResources().getString(R.string.label_no_thanks), null);
			
			AlertDialog currentDialog = builder.create();
			Window window = currentDialog.getWindow();
			WindowManager.LayoutParams ra = window.getAttributes();
			
			IBinder token = this.kbContainerView.getKbLinearLayout().getKeyboardView().getWindowToken();
			
			if ( token != null ) {
				ra.token = token;
				//Log.d("WINDOW TOKEN = ", ""+ra.token);
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
	 * Set the {@link PopupManager}'s layout. This will be used to display the {@link com.iknowu.popup.PopupKeyboard}s
	 */
	private void setPopupManagerKeyboardLayout() {
        this.popupManager.setAndLoadCurrentKeyboard( this.currentKeyboardLayout );
	}
	
	/**
	 * Pass the keyboard key size information to allow for better correction results
	 * @param keyboard the current keyboard that is being used
	 * @return an array of {@link KeyPosition}
	 */
	public KeyPosition[] getKeyboardPositionListForAutoCorrect(IKnowUKeyboard keyboard, int keyGap) {
        log(Log.VERBOSE, "getPosListForAutoCorrect()", "START, keyboard = "+keyboard);
		if (keyboard != null) {

			Iterator<IKnowUKeyboard.Key> it = keyboard.getKeys().iterator();
			
			ArrayList<com.wordlogic.lib.KeyPosition> keys = new ArrayList<com.wordlogic.lib.KeyPosition>();

            int gap = 0;

			while(it.hasNext()) {
				IKnowUKeyboard.Key key = it.next();
				
				if(key.label != null && key.codes[0] != -2 && key.codes[0] != -6 && key.codes[0] != -8 && key.codes[0] != -9 && key.codes[0] != -10 && key.codes[0] != -11 && key.codes[0] != -14) {
					if(key.codes.length < 1) {
						continue;
					} else {
                        for (int i=0; i < key.codes.length; i++) {
                            log(Log.VERBOSE, "getPosListForAutoCorrect()", "char = "+(char)(key.codes[i])+", x = "+(key.x+1+gap)+", y = "+(key.y+1));
                            keys.add(new KeyPosition((char)(key.codes[i]),key.x+1+gap,key.y+1,key.width-2,key.height-2));
                        }
						
						switch((char)key.codes[0]) {
						case 'q':
							keys.add(new KeyPosition('1',key.x+1,key.y+1,key.width-2,key.height-2));
							break;
						case 'w':
							keys.add(new KeyPosition('2',key.x+1,key.y+1,key.width-2,key.height-2));
							break;
						case 'e':
							keys.add(new KeyPosition('3',key.x+1,key.y+1,key.width-2,key.height-2));
							break;
						case 'r':
							keys.add(new KeyPosition('4',key.x+1,key.y+1,key.width-2,key.height-2));
							break;
						case 't':
							keys.add(new KeyPosition('5',key.x+1,key.y+1,key.width-2,key.height-2));
							break;
						case 'y':
							keys.add(new KeyPosition('6',key.x+1,key.y+1,key.width-2,key.height-2));
							break;
						case 'u':
							keys.add(new KeyPosition('7',key.x+1,key.y+1,key.width-2,key.height-2));
							break;
						case 'i':
							keys.add(new KeyPosition('8',key.x+1,key.y+1,key.width-2,key.height-2));
							break;
						case 'o':
							keys.add(new KeyPosition('9',key.x+1,key.y+1,key.width-2,key.height-2));
							break;
						case 'p':
							keys.add(new KeyPosition('0',key.x+1,key.y+1,key.width-2,key.height-2));
							break;
						}

                        gap += keyGap;
					}
				}
			}
			
			com.wordlogic.lib.KeyPosition[] theKeys = new com.wordlogic.lib.KeyPosition[keys.size()];
			theKeys = keys.toArray(theKeys);
			
			return theKeys;
		} else {
			return null;
		}
	}
	
	private void setScreenCharacteristics() {
		
	}
	
	/**
	 * Get our trial expired variable
	 * @return the current trial expired value
	 */
	public boolean getTrialExpired() {
		return this.mTrialExpired;
	}
	
	@Override
	public void onUpdateExtractingVisibility(EditorInfo ei) {
		this.setExtractViewShown(false);
	}
	
	@Override
	public boolean onEvaluateFullscreenMode() {
		return false;
	}
	
	/**
	 * Tell the {@link MiniAppManager} to detect the currently installed Miniapps
	 */
	public void checkForMiniApps() {
        if (MINIAPP_ON)
		    this.miniAppManager.detectMiniAppList();
	}
	
	@Override
    public void onBindInput() {
        super.onBindInput();
        log(Log.VERBOSE, "onBindInput", "function");
        currentInputConnection = this.getCurrentInputConnection();
    }
	
	@Override
    public void onUnbindInput() {
        super.onUnbindInput();
        log(Log.VERBOSE, "onUnbindInput", "function");
        currentInputConnection = null;
    }

	/**
	 * Close the currently displayed MiniApp and return the view to the {@link KeyboardScreen}
	 */
	public void closeMiniApp() {
		if (MINIAPP_ON && this.miniAppManager != null) {
			this.miniAppManager.clearMiniAppList(false);
			//this.ikuScroller.scrollToKeyboard();
            //TODO: probably in the new stuff it should slide the side view back out
		}
	}
	
	/**
	 * Update the view being displayed in the {@link MiniAppScreen}.
	 * 
	 * This is called from the actual MiniApps themselves, when they want to
	 * change the content of their view
	 */
	public void updateMiniAppView() {
        if (MINIAPP_ON) {
            log(Log.INFO, "UpdateMiniAppView", "MAMessageReceiver = "+this.miniAppMessageReceiver+", updatedView = "+this.miniAppMessageReceiver.updatedView);
            if (this.miniAppMessageReceiver != null && this.miniAppMessageReceiver.updatedView != null) {
                this.miniAppManager.updateView(this.miniAppMessageReceiver.updatedView, this.miniAppMessageReceiver.animNum);
            }
        }
	}
	
	/**
	 * Delete characters from the current editor.
	 * 
	 * This is called by the MiniApps
	 * @param before the number of characters before the cursor to delete
	 * @param after the number of characters after the cursor to delete
	 */
	public void deleteChars(int before, int after) {
		try {
            if (MINIAPP_ON) {
                boolean ret = false;

                //if (currentInputConnection != null) {
                //	this.commitCurrentWord(currentInputConnection, "deleteChars1");
                //	ret = currentInputConnection.deleteSurroundingText(before, after);
                //} else {
                    InputConnection ic = this.getCurrentInputConnection();
                    if (ic != null) {
                        this.commitCurrentWord(ic, "deleteChars2");
                        ret = ic.deleteSurroundingText(before, after);
                    }
                //}
                log(Log.VERBOSE, "IKnowUKeyboardService", "DeleteCharacter inputConn = "+currentInputConnection+", getCurInputConn = "+this.getCurrentInputConnection());
                log(Log.VERBOSE, "IKnowUKeyboardService", "DeleteCharacter before = "+before+", after = "+after+", returns = "+ret);
            }
		} catch (Exception e) {
			sendErrorMessage(e);
		}
	}
	
	/**
	 * Send a body of text to the editor.
	 * 
	 * This is called by the MiniApps
	 * @param text the text to be inserted
	 * @param before not being used
	 * @param after not being used
	 * @param stayAlive Whether or not the current MiniApp should stay active after inserting the text
	 */
	public void sendTextToEditor(String text, int before, int after, boolean stayAlive) {
        if (MINIAPP_ON) {
            log(Log.WARN, "SENDTEXTTOEDITOR", "HERE");
            this.commitCurrentWord(this.getCurrentInputConnection(), "sendTextToEditor");
            this.miniAppTextSent = true;
            keepCurrentMiniApp = stayAlive;
            this.getCurrentInputConnection().commitText(text, 1);
            //newAutoCorrect.addString(text);
            //mPredictionEngine.advanceWordComplete(text);
        }
	}
	
	/**
	 * Get our {@link PredictionEngine}
	 * @return
	 */
	public static PredictionEngine getPredictionEngine() {
		return mPredictionEngine;
	}
	
	/**
	 * Get our {@link AutoCorrect} engine
	 * @return
	 */
	public static AutoCorrect getAutoCorrectEngine() {
		return mAutoCorrect;
	}
	
	/**
	 * Get the current user, if this has been established
	 * @return the current {@link ParseUser} that is logged in
	 */
	public static ParseUser getCurrentUser() {
		return currentUser;
	}

	/**
	 * Called by the framework when your view for showing candidates needs to be
	 * generated, like {@link #onCreateInputView}.
	 */
	@Override
	public View onCreateCandidatesView() {
		// has been taken out as the candidate view is implemented as insets
		// because it obscures the application area
		// mCandidateView = new CandidateView(this);
		// mCandidateView.setService(this);
		// mCandidateView.setSoftKeyboard(this);
		// return mCandidateView;
		return null;
	}
	
	/**
	 * Determines whether we need to show a full soft input area, or just our {@link SuggestionsLinearLayout}
	 */
	private void evaluateInputNeeds() {
		try {
			Configuration config = getResources().getConfiguration();
			changeOverInputDisplayState = KBVIEW_AND_CANDLIST;
			
			if (config.keyboard == Configuration.KEYBOARD_NOKEYS) {
				return;
			}
			
			if (config.keyboard == Configuration.KEYBOARD_QWERTY && 
				config.hardKeyboardHidden == Configuration.HARDKEYBOARDHIDDEN_NO) {
				//Log.d("Evaluate input needs", "Switching to cand list only");
				// keyboard has been slid out, present prediction list (horz cand list) only)
				changeOverInputDisplayState = CANDLISTONLY;
			}
		} catch (Exception e) {
			sendErrorMessage(e);
		}
	}
	
	@Override
	public boolean onEvaluateInputViewShown() {
		evaluateInputNeeds();
		return true;
	}
	
	/**
	 * This is called when the user is done editing a field. We can use this to
	 * reset our state.
	 */
	@Override
	public void onFinishInput() {
		try {
			log(Log.WARN, "ON FINISH INPUT", "START FUNCTION");
			//mPredictionEngine.reset(true);
			if (this.kbContainerView != null) {
				this.kbContainerView.onFinishInput();
			}
			
			super.onFinishInput();
		} catch (Exception e) {
			sendErrorMessage(e);
		}
	}
	
	@Override
	public void onFinishInputView( boolean finishingInput) {
		try {
			log(Log.ERROR, "ONFINISHINPUTVIEW", "HERE");
			this.commitCurrentWord(this.getCurrentInputConnection(), "onFinishInputView");
			//updateCandidates();

            if (this.kbContainerView != null) {
                this.kbContainerView.onFinishInput();
            }
			
			if (this.startTime > 0) {
				this.totalTimeOpen += System.currentTimeMillis() - this.startTime;
			}
			
			this.saveStats();
			
//			if( mAutoCorrect.isSet()) {
//				mAutoCorrect.saveUpdates();
//			}
		} catch (Exception e) {
			sendErrorMessage(e);
		}
		
		super.onFinishInputView(finishingInput);
	}
	
	/**
	 * Save our typing statistic variables to the stats.xml file
	 */
	private void saveStats() {
		File directory = this.getFilesDir();
	    File stats = new File(directory, "wordlogic/dictionary/stats.xml");
	    
	    XmlSerializer serializer = Xml.newSerializer();
	    StringWriter writer = new StringWriter();
	    try {
	    	serializer.setOutput(writer);
	    	serializer.startDocument("UTF-8", true);
	    	serializer.startTag("", "stats");
	    	serializer.startTag("", "totals");
	    	serializer.attribute("", "totalChars", ""+this.totalCharsTyped);
	    	serializer.attribute("", "totalPredsSelected", ""+this.totalPredsSelected);
	    	serializer.attribute("", "totalCorrectionsInserted", ""+this.totalCorrectionsInserted);
	    	serializer.attribute("", "totalKeyStrokesSaved", ""+this.totalKeyStrokesSaved);
	    	serializer.attribute("", "totalTimeOpen", ""+this.totalTimeOpen);
	    	serializer.attribute("", "totalWordsTyped", ""+this.totalWordsTyped);
	        serializer.endTag("", "totals");
	        serializer.endTag("", "stats");
	        serializer.endDocument();
	        String doc = writer.toString();
	        
	        FileWriter fw = new FileWriter(stats);
	        fw.write(doc);
	        
	        fw.flush();
	        fw.close();
	    } catch (FileNotFoundException fnfe) {
            //do nothing, their phone's storage has been cut off for some reason
            Toast.makeText(this, this.getResources().getString(R.string.file_not_found) + "code = "+1004, Toast.LENGTH_LONG).show();
        } catch (Exception e) {
	        sendErrorMessage(e);
	    } 
	}
	
	/*
	 * Load the typing statistic variable form the stats.xml
	 */
	private void loadStats() {
		try {
			File directory = this.getFilesDir();
			//File directory = Environment.getExternalStorageDirectory();
		    File stats = new File(directory, "wordlogic/dictionary/stats.xml");
		    
		    if (stats.exists()) {
				FileInputStream in = new FileInputStream(stats);
				XmlPullParser xmp = Xml.newPullParser();
				xmp.setInput(in, null);
	            int event;
	            while ((event = xmp.next()) != XmlPullParser.END_DOCUMENT) {
	                if (event == XmlPullParser.START_TAG) {
	                    String tag = xmp.getName();
	                    if (tag.equals("totals")) {
	                    	this.totalCharsTyped = Integer.parseInt(xmp.getAttributeValue(null, "totalChars"));
	                    	this.totalPredsSelected = Integer.parseInt(xmp.getAttributeValue(null, "totalPredsSelected"));
	                    	this.totalCorrectionsInserted = Integer.parseInt(xmp.getAttributeValue(null, "totalCorrectionsInserted"));
	                    	this.totalKeyStrokesSaved = Integer.parseInt(xmp.getAttributeValue(null, "totalKeyStrokesSaved"));
	                    	this.totalTimeOpen = Long.parseLong(xmp.getAttributeValue(null, "totalTimeOpen"));
	                    	this.totalWordsTyped = Integer.parseInt(xmp.getAttributeValue(null, "totalWordsTyped"));
	                    } 
	                } else if (event == XmlPullParser.END_TAG) {}
	            }
		    } else {
		    	this.totalCharsTyped = 0;
		    	this.totalCorrectionsInserted = 0;
		    	this.totalKeyStrokesSaved = 0;
		    	this.totalPredsSelected = 0;
		    	this.totalTimeOpen = 0;
		    	this.totalWordsTyped = 0;
		    }
            
            log(Log.VERBOSE, "Stats Loaded", "Total Chars = "+totalCharsTyped+", Total Preds = "+this.totalPredsSelected+
            		", Total Corrs = "+this.totalCorrectionsInserted+", Total keys saved = "+this.totalKeyStrokesSaved+", Time Open = "+this.totalTimeOpen
            		+", Total words = "+this.totalWordsTyped);
        } catch (FileNotFoundException fnfe) {
            //do nothing, their phone's storage has been cut off for some reason
            Toast.makeText(this, this.getResources().getString(R.string.file_not_found) + "code = "+1002, Toast.LENGTH_LONG).show();
        } catch (Exception e) {
        	IKnowUKeyboardService.sendErrorMessage(e);
        }
	}
	
	/**
	 * Rearrange the display according to our current input needs. Generally just going to show
	 * the full soft input area, or the candidate view only if a hard keyboard has been attached.
	 */
	private void rearrangeDisplay() {
		try {
			switch (currentInputDisplayState) {
				case KBVIEW_AND_CANDLIST: {
					//Log.e("WordLogicKeyboard", String.format("onStartInputView 1aa %d",	currentInputDisplayState));
					if (changeOverInputDisplayState == KBVIEW_NO_CANDLIST) {
						hideCandidateView();
						currentInputDisplayState = changeOverInputDisplayState; 
					}
					else if (changeOverInputDisplayState == CANDLISTONLY) {
						hideKeyboardViewShowCandidateOnlyView();
						currentInputDisplayState = changeOverInputDisplayState;
					}
					break;
				}
				case KBVIEW_NO_CANDLIST: {
					//Log.e("WordLogicKeyboard", String.format("onStartInputView 1bb %d",	currentInputDisplayState));
					if (changeOverInputDisplayState == KBVIEW_AND_CANDLIST) {
						revealCandidateView();
						currentInputDisplayState = changeOverInputDisplayState; 
					}
					else if (changeOverInputDisplayState == CANDLISTONLY) {
						hideKeyboardViewShowCandidateOnlyView();
						currentInputDisplayState = changeOverInputDisplayState; 
					}
					break;
				}
				case CANDLISTONLY: {
					//Log.e("WordLogicKeyboard", String.format("onStartInputView 1cc %d %s",
							//currentInputDisplayState, mInputViewShown()));
	
					if (changeOverInputDisplayState == KBVIEW_AND_CANDLIST) {
						revealKbView();
						revealCandidateView();
						currentInputDisplayState = changeOverInputDisplayState;
					} else if (changeOverInputDisplayState == KBVIEW_NO_CANDLIST) {
						hideCandidateView();
						revealKbView();
						currentInputDisplayState = changeOverInputDisplayState; 
					} else if (changeOverInputDisplayState == CANDLISTONLY) {
						// do this explicitly every fricking time
						hideKeyboardViewShowCandidateOnlyView();
						currentInputDisplayState = changeOverInputDisplayState; 
					}
					break;
				}
			}
		} catch (Exception e) {
			sendErrorMessage(e);
		}
	}

	@Override
	public void onUpdateCursor(Rect newRect) {
		//log(Log.WARN, "ONUPDATECURSOR", "CURSOR UPDATED!!!!");
		super.onUpdateCursor(newRect);
	}
	
	/**
	 * Check if the last character is a space
	 * if it is, append a period
	 * 
	 * Also, handle '@' symbols in email fields, meaning remove any unwanted spaces in these
	 * situations
	 */
	public char handleSpace() {
		try {
            char ret = '0';

			InputConnection ic = this.getCurrentInputConnection();
			
			String lastTwoChars = (String) ic.getTextBeforeCursor(2, 0);
			String curWord = this.getLastWord(ic, false, false);
			if (lastTwoChars != null && lastTwoChars.length() > 1 && lastTwoChars.charAt(1) == ' ') {
				String firstChar = Character.toString(lastTwoChars.charAt(0));
				if (!this.mWordSeparators.contains(firstChar)) {
					ic.deleteSurroundingText(1, 0);
					mPredictionEngine.backspaceLetter();
					if (this.isEmailField && !curWord.contains("@")) {
						ret = '@';
					} else if (!this.isEmailField){
                        ret = '.';
					}
				}
			} else if (lastTwoChars != null && lastTwoChars.length() > 0 && lastTwoChars.length() < 2 && lastTwoChars.charAt(0) == ' ') {
				ic.deleteSurroundingText(1, 0);
				mPredictionEngine.backspaceLetter();
				if (this.isEmailField && !curWord.contains("@")) {
					ret = '@';
				} else if (!this.isEmailField){
                    ret = '.';
				}
			}
			return ret;
			
		} catch (Exception e) {
			sendErrorMessage(e);
			return '0';
		}
	}
	
	/**
	 * Get the text before the cursor position up until there is a space, if the space
	 * is the first character, continue until the next space
	 * @param ic The current {@link InputConnection} to use for analyzing the text
	 * @return the text found
	 */
	private String getTextBeforeSpace(InputConnection ic) {
		log(Log.WARN, "GETTEXTBEFORESPACE", "HERE");
		try {
			String text = (String) ic.getTextBeforeCursor(LONG_WORD_LENGTH, 0);
			
			//if the text is valid
			if (text != null && text.length() > 0) {
				if (text.contains(" ") && text.lastIndexOf(" ") != text.length() - 1) {
					text = text.substring(text.lastIndexOf(" ")+1);
				} else if (text.contains(" ") && text.lastIndexOf(" ") == text.length() - 1) {
					//log(Log.VERBOSE, "there is at least one space", "and it's in the spot before the cursor, text = |"+text+"|");
					String chars = "";
					for (int i = 1; i < text.length(); i++) {
						chars = (String) ic.getTextBeforeCursor(i, 0);
						if (chars.length() > 1) {
							chars = chars.substring(0, 1);
						}
						//log(Log.VERBOSE, "Analyzing", "Current Char = |"+chars+"|");
						if (chars.contains(" ") && i != 1) {
							text = text.substring(text.length() - i + 1);
							//log(Log.VERBOSE, "Chars = |"+chars+"|", "returning = |"+text+"|");
							break;
						}
					}
				}
			} else {
				text = "";
			}
			
			return text;
		} catch (Exception e) {
			sendErrorMessage(e);
			return "";
		}
	}
	
	/**
	 * Gets the last full word before the current cursor position
	 * @param ic the {@link InputConnection} to use
	 * @param includeWordSeps if true, back out if a word separator ('.' ',' '?' '!' etc.) is detected before a space
	 * @param includeCurrent if true, return the current word being typed, otherwise look further back
	 * @return the text found, or an empty {@link String} if any conditions are not met.
	 */
	public String getLastWord(InputConnection ic, boolean includeWordSeps, boolean includeCurrent) {
		log(Log.WARN, "GETLASTWORD", "HERE");

        //this.startTimer();

		try {
			String text = "";
			if (ic != null)
				text = (String) ic.getTextBeforeCursor(LONG_WORD_LENGTH, 0);
			
			//log(Log.DEBUG, "Get last word", "Text before cursor first = "+text);
			//if the text is valid
			if (text != null && text.length() > 0) {
				
				//always check for spaces or new lines, we do not want these
				int spaceIndex = text.lastIndexOf(" ");
				int newLineIndex = text.lastIndexOf(System.getProperty("line.separator"));
				int closest = spaceIndex > newLineIndex ? spaceIndex : newLineIndex; //closer of a new line or a space to the cursor
				
				//check to see where the cursor is, we do not want the current word being typed,
				//if this is the case, then substring that part out
				if ((spaceIndex > 0 || newLineIndex > 0) && closest != text.length() - 1) {
					if (includeWordSeps) {
						if (includeCurrent) {
							text = text.substring(closest+1, text.length());
							//log(Log.VERBOSE, "Get last word", "Included current, returning = "+text);
							return text;
						}
						//if we are looking for word seps, try to see if the last character
						//is one. If it is, return an empty string
						String lastChar = text.substring(text.length() - 1, text.length());
						if(this.mWordSeparators.contains(lastChar)) {
							return "";
						} else {
							text = text.substring(0, closest);
						}
					} else {
						if (includeCurrent) {
							text = text.substring(closest+1, text.length());
							//log(Log.VERBOSE, "Get last word", "Included current, returning = "+text);
							return text;
						} else {
							text = text.substring(0, closest);
						}
					}
				} else if (spaceIndex < 0 && newLineIndex < 0) {
					if (includeCurrent) {
						//log(Log.VERBOSE, "Get Last word", "Included current, Returning = |"+text+"|");
						return text;
					} else {
						text = ""; // this would mean the very first word in the box of text
					}
				}
				
				spaceIndex = text.lastIndexOf(" ");
				newLineIndex = text.lastIndexOf(System.getProperty("line.separator"));
				closest = spaceIndex > newLineIndex ? spaceIndex : newLineIndex;
				
				if (text.length() > 0) {
					if ((spaceIndex > 0 || newLineIndex > 0) && closest == text.length() - 1) {
						text = text.trim();
						if (includeWordSeps) {
							//check to see if there is some sort of period or comma before the previous word
							if (text.length() > 0) {
								String lastChar = text.substring(text.length() - 1, text.length());
								if(this.mWordSeparators.contains(lastChar)) {
									return "";
								}
							} else {
								return "";
							}
						}
					}
					
					spaceIndex = text.lastIndexOf(" ");
					newLineIndex = text.lastIndexOf(System.getProperty("line.separator"));
					closest = spaceIndex > newLineIndex ? spaceIndex : newLineIndex;
					
					//log(Log.WARN, "Get Last word", "space = "+spaceIndex+", newLine = "+newLineIndex);
					
					if ((spaceIndex > 0 || newLineIndex > 0)) {
						text = text.substring(closest+1);
					}
				} else {
					text = "";
				}
			} else {
				text = "";
			}

            //this.stopTimer("getLastWord()");
			//log(Log.VERBOSE, "Get Last word", "Returning = |"+text+"|");
			return text;
		} catch (Exception e) {
			sendErrorMessage(e);
			return "";
		}
	}
	
	/**
	 * Set the current word being typed based on the cursor's position.
	 * 
	 * This will only look at the text before the cursor and if there is a space, nothing will be set.
	 * @param ic the {@link InputConnection} to use
	 */
	public void setCurrentWord(InputConnection ic) {
		log(Log.WARN, "SETCURRENTWORD", "HERE");
		try {
			String text = "";
			if (ic != null) 
				text = (String) ic.getTextBeforeCursor(LONG_WORD_LENGTH, 0);
			
			this.commitCurrentWord(ic, "SetCurrentWord");
			//if the text is valid
			if (text != null && text.length() > 0) {
				
				String lastChar = text.substring(text.length()-1, text.length());
				if (this.mWordSeparators.contains(lastChar)) {
					this.currentWord.setLength(0);
                    /*
					String lastWord = this.getLastWord(ic, true, false);
					if (lastWord != null && lastWord.length() > 0 && Character.isUpperCase(lastWord.charAt(0)) ) {
						this.capitalizeFirstLetter = true;
					} else {
						this.capitalizeFirstLetter = false;
					}
					*/
					//mPredictionEngine.reset(false);
					return;
				}
				//check to see where the cursor is, we want the current word being typed
				if (text.contains(" ") || text.contains("\n")) {
					int spaceIndex = text.lastIndexOf(" ");
					int newLineIndex = text.lastIndexOf("\n");
					int closest = Math.max(spaceIndex, newLineIndex); 
					
					if (closest != text.length() - 1) {
						String addText = text.substring(closest+1);
						//ic.beginBatchEdit();
						//ic.deleteSurroundingText(addText.length(), 0);
						this.addToCurrentWord(ic, addText, false);
						//ic.endBatchEdit();
					} else {
						this.currentWord.setLength(0);
					}
				} else {
					//ic.beginBatchEdit();
					//ic.deleteSurroundingText(text.length(), 0);
					this.addToCurrentWord(ic, text, false);
					//ic.endBatchEdit();
				}
			} else {
				if (this.currentWord != null) {
				} else {
					this.currentWord = new StringBuilder();
				}
				this.currentWord.setLength(0);
			}
			
			if ( this.currentWord.length() > 0 && Character.isUpperCase(this.currentWord.charAt(0)) ) {
				this.capitalizeFirstLetter = true;
			} else {
				this.capitalizeFirstLetter = false;
			}
			//log(Log.VERBOSE, "Set current word", "word = "+this.currentWord);
			//return text;
		} catch (Exception e) {
			log(Log.ERROR, "SETCURRENTWORD", "exception:"+e.getMessage());
			sendErrorMessage(e);
			//return "";
		}
	}
	
	/**
	 * Helper function to get the current word being typed
	 * @return the current word
	 */
	public String getCurrentWord() {
		return this.currentWord.toString();
	}
	
	/**
	 * helper function to add the specified text to the end of the current
	 * text that is being typed
	 * @param ic the {@link InputConnection} to use
	 * @param textToAdd the text to be appended to the current word
	 * @param commitChanges Whether or not to commit this change to the editor
	 */
	public void addToCurrentWord(InputConnection ic, String textToAdd, boolean commitChanges) {
		log(Log.WARN, "addToCurrentWord()", "textToAdd = "+textToAdd+", commitChanges = "+commitChanges);
		try {
			if (textToAdd != null) {
				this.currentWord.append(textToAdd);
				
				if (commitChanges) {
					ic.commitText(textToAdd, 1);
				}
				//this.getCurrentInputConnection().setComposingText(this.currentWord, 1);
			}
		} catch (Exception e) {
			log(Log.ERROR, "addToCurrentWord()", "Exception:"+e.getMessage());
			sendErrorMessage(e);
		}
	}
	
	/**
	 * Delete text from the current word
	 * @param start is inclusive so it will start at the position provided to start
	 * @param finish is exclusive so it will end at end-1 of the position provided to end
	 * @param commitChanges Whether or not to commit this change to the editor
	 */
	public void deleteFromCurrentWord(int start, int finish, boolean commitChanges) {
		try {
			if (this.currentWord.length() >= (finish - start)) {
				this.currentWord.delete(start, finish);
				//Log.v("Current word = ", "|"+currentWord+"|");
			} else {
				this.currentWord.setLength(0);
			}
			
			this.getCurrentInputConnection().deleteSurroundingText(finish - start, 0);
			/*
			if (commitChanges) {
				//this.getCurrentInputConnection().setComposingText(this.currentWord, 1);
			}
			*/
		} catch (Exception e) {
			sendErrorMessage(e);
		}
	}
	
	/**
	 * Helper function to commit any text being composed in to the editor.
	 * @param ic the {@link InputConnection} to use
	 * @param calledfrom the function that this was called from, purely for debugging purposes
	 */
    public void commitCurrentWord(InputConnection ic, String calledfrom) {
    	log(Log.WARN, "COMMITCURRENTWORD", "called from = "+calledfrom);
    	//log(Log.ERROR, "Commit CurrentWord", "called from = "+calledfrom);
    	//log(Log.WARN, "Commit CurrentWord", "current word = "+this.currentWord);
        if (this.currentWord != null && this.currentWord.length() > 0) {
            //this.getCurrentInputConnection().commitText(this.currentWord, this.currentWord.length());
        	//if (ic != null)
        	//	ic.finishComposingText();
            this.currentWord.setLength(0);
            //updateCandidates();
        } else {
        	//Log.e("Current word is null", "ignoring call");
        }
    }
	
    /**
     * Get the text from the editor surrounding the cursor either in front or behind it
     * until a space is found
     * @param ic the {@link InputConnection} to use
     * @param before if true get the the text before the cursor, otherwise get the text after the cursor
     * @return the text found
     */
	public String pullText(InputConnection ic, boolean before) {
		log(Log.WARN, "PULLTEXT", "HERE");
		try {
			String text = "";
			int nChars = 0;
			int spaceIndex;
			int newlineIndex;
			CharSequence cs;
			
			while (! text.contains(" ") && ! text.contains("\n")) {
				nChars += 6;
				if (before) {
					cs = ic.getTextBeforeCursor(nChars, 0);
				} else {
					cs = ic.getTextAfterCursor(nChars, 0);
				}
				text = (cs == null) ? "" : cs.toString(); 
				
				if (text.length() < nChars)
				   break;
			}
			
			if (before) {
				spaceIndex = text.lastIndexOf(' ');
				newlineIndex = text.lastIndexOf('\n');
				// handles -1 perfectly
				spaceIndex = Math.max(spaceIndex,newlineIndex);
				
				return text.substring(spaceIndex+1);
			} else {
				spaceIndex = text.indexOf(' ');
				newlineIndex = text.indexOf('\n');
				if (spaceIndex == -1)
					spaceIndex = text.length();
				if (newlineIndex == -1)
					newlineIndex = text.length();
				spaceIndex = Math.min(spaceIndex,newlineIndex);
				if (spaceIndex == 0)
					return "";
				return text.substring(0,spaceIndex);
			}
		} catch (Exception e) {
			sendErrorMessage(e);
			return "";
		}
	}
	
	/**
	 * Get the last word from the editor before the cursor
	 * @param ic the {@link InputConnection} to use
	 * @return the text found
	 */
	public String pullLastWord(InputConnection ic) {
		log(Log.WARN, "PULLLASTWORD", "HERE");
		try {
			String text = "";
			int nChars = 0;
			int spaceIndex;
			int newlineIndex;
			int beginningOfWordIndex = 0;
		
			nChars = 100;
			CharSequence cs = ic.getTextBeforeCursor(nChars, 0);
			if (cs == null)
			{
				return "";
			}
			text = cs.toString();
			if (text == null || text.length() <= 0) {
				return "";
			}
			
			spaceIndex = text.lastIndexOf(' ');
			newlineIndex = text.lastIndexOf('\n');
			int lastIndex = Math.max(spaceIndex,newlineIndex);
			lastIndex = lastIndex >= 0 ? lastIndex : 0;
			if (lastIndex < text.length()-1) {
				return text.substring(lastIndex);
			}
			
			// we might have some leading spaces
			int lastSpaceIndex = lastIndex;
			while (lastSpaceIndex > 0 && Character.isWhitespace(text.charAt(lastSpaceIndex))) {
				lastSpaceIndex--;
			} 
			
			if (lastSpaceIndex > 0) {
				// we encountered a non white space
				spaceIndex = text.lastIndexOf(' ', lastSpaceIndex);
				newlineIndex = text.lastIndexOf('\n', lastSpaceIndex);
				beginningOfWordIndex = Math.max(spaceIndex,newlineIndex);
				beginningOfWordIndex = beginningOfWordIndex >= 0 ? beginningOfWordIndex+1 : 0;
				return text.substring(beginningOfWordIndex, text.length());
			}
			return text.substring(0, text.length());
		} catch (Exception e) {
			sendErrorMessage(e);
			return "";
		}
	}
	
	/**
	 * Get the next word after the cursor form the current editor
	 * @param ic the {@link InputConnection} to use
	 * @return the text found
	 */
	public String pullNextWord(InputConnection ic) {
		log(Log.WARN, "PULLNEXTWORD", "HERE");
		try {
			String text = "";
			int nChars = 0;
			int spaceIndex;
			int newlineIndex;
		
			nChars = 100;
			CharSequence cs = ic.getTextAfterCursor(nChars, 0);
			if (cs == null)
			{
				return "";
			}
			text = cs.toString();
			if (text == null || text.length() <= 0) {
				return "";
			}
			
			spaceIndex = text.indexOf(' ');
			newlineIndex = text.indexOf('\n');
			int lastIndex = 0;
			if (spaceIndex >= 0 && newlineIndex >= 0)
				lastIndex = Math.min(spaceIndex,newlineIndex);
			else if (spaceIndex >= 0)
				lastIndex = spaceIndex;
			else if (newlineIndex >= 0)
				lastIndex = newlineIndex;
			else {
				// no spacing or  new line encountered
				// jump over the whole word
				lastIndex = text.length() - 1;
			}
			
			// we might have some trailing spaces
			int lastSpaceIndex = lastIndex;
			while (lastSpaceIndex < text.length() && Character.isWhitespace(text.charAt(lastSpaceIndex))) {
				lastSpaceIndex++;
			}
			return text.substring(0, lastIndex+1);
		} catch (Exception e) {
			sendErrorMessage(e);
			return "";
		}
	}

	
	/**
	 * Take words that have been auto added to the engine, and put them in our added words list
	 * this way we can batch our added words to the cloud
	 */
	public void autoAddWords() {
		log(Log.WARN, "AUTOADDEDWORDS", "HERE");
		try {
			String[] addedWords = mPredictionEngine.getAutoAddedWords();
			for ( int i = 0; i < addedWords.length; i++) {
				addedWordsList.add(addedWords[i]);
			}
		} catch (Exception e) {
			sendErrorMessage(e);
		}
	}
	
	/**
	 * add a word to our added words list and to the engine
	 * 
	 * @param newWord the word to be added
	 * @param addtoAutoCor whether to add to the auto-correct or not
	 * @return true if the word has been added
	 */
	public static boolean addWord(String newWord) {
		log(Log.WARN, "Appside: ADDWORD():", newWord);
		try {
			newWord = newWord.toLowerCase();
			
			//Minkyu:2014.11.18
			//New word should be case sensitive.
			boolean add = mPredictionEngine.addWord(newWord, 1);
			if (add)
			{
				addedWordsList.add(newWord);
				trySendCloudMessages( newWord );
			}
			//log(Log.WARN, "Added word", "success = "+add);
            return true;
			//return add;
		} catch (Exception e) {
			sendErrorMessage(e);
			return false;
		}
	}
	
	/**
	 * add a word to our deleted words list and remove from the engine
	 * @param newWord
	 * @return true if the word has been deleted
	 */
	public static boolean deleteWord(String newWord) {
		log(Log.WARN, "DELETEWORD", "HERE");
		try {
			//mAutoCorrect.deleteWord(newWord);
			
			boolean del = mPredictionEngine.deleteWord(newWord);
			if (del) {
				deletedWordsList.add(newWord);
				trySendCloudMessages( newWord );
			}
			return del;
		} catch (Exception e) {
			sendErrorMessage(e);
			return false;
		}
	}
	
	/**
	 * Check to see if we should send messages to the cloud
	 * 
	 * @param word
	 */
	public static void trySendCloudMessages( String word ) {
		log(Log.WARN, "TRYSENDCLOUDMESSAGES", "HERE");
		try {
			if ( addedWordsList.size() > 0 ) {
				addWordsToCloud();
			}
			if ( deletedWordsList.size() > 0 ) {
				deleteWordsToCloud( word );
			}
		} catch (Exception e) {
			sendErrorMessage(e);
		}
	}
	
	/**
	 * Loop through our added words list and construct a json message to send to the cloud
	 */
	public static void addWordsToCloud() {
		log(Log.WARN, "ADDWORDSTOCLOUD", "HERE");
		try {
			String jsonMessage = "{\"action\": \"com.keyboard.ADD_WORD\"";
			for ( int i = 0; i < addedWordsList.size(); i++) {
				jsonMessage += ", \"word"+i+"\": \""+addedWordsList.get(i)+"\"";
			}
			jsonMessage += "}";
			//Log.d("JSON MSG =", jsonMessage);
			// send push notifiaction
			if (currentUser != null) {
				ParsePush push = new ParsePush();
				push.setChannel(currentUser.getObjectId());
				try {
					push.setData(new JSONObject(jsonMessage));
					push.sendInBackground();
				} catch (JSONException e) {
					e.printStackTrace();
				}
				
				List<ParseObject> words = new ArrayList<ParseObject>();
				for ( int j = 0; j < addedWordsList.size(); j++) {
					ParseObject word = new ParseObject("Word");
					word.put("User", currentUser.getObjectId());
					word.put("Word", addedWordsList.get(j));
					words.add(word);
				}
				ParseObject.saveAllInBackground(words);
				addedWordsList.removeAll(addedWordsList);
			}
		} catch (Exception e) {
			sendErrorMessage(e);
		}
	}
	
	/**
	 * loop through our deleted words list and construct a json message to send to the cloud
	 */
	public static void deleteWordsToCloud( String word) {
		log(Log.WARN, "DELETEWORDSTOCLOUD", "HERE");
		try {
			String jsonMessage = "{\"action\": \"com.keyboard.DELETE_WORD\"";
			for ( int i = 0; i < deletedWordsList.size(); i++) {
				jsonMessage += ", \"word"+i+"\": \""+deletedWordsList.get(i)+"\"";
			}
			jsonMessage += "}";
			
			//send push notification
			if (currentUser != null) {
				ParsePush push = new ParsePush();
				push.setChannel(currentUser.getObjectId());
				try {
					push.setData(new JSONObject(jsonMessage));
					push.sendInBackground();
				} catch (JSONException e) {
					e.printStackTrace();
				}
				
				ParseQuery query = new ParseQuery("Word");
				query.whereEqualTo("User", currentUser.getObjectId());
				query.whereEqualTo("Word", word);
				query.findInBackground(new FindCallback() {
				    public void done(List<ParseObject> list, ParseException e) {
				        if (e == null) {
				            deleteWordsFromTable(list);
				        }
				    }
				});
				deletedWordsList.removeAll(deletedWordsList);
			}
		} catch (Exception e) {
			sendErrorMessage(e);
		}
	}
	
	/**
	 * Delete words from our parse table that correspond with this user
	 * @param words the list of words to be deleted
	 */
	private static void deleteWordsFromTable(List<ParseObject> words) {
		for ( int j = 0; j < words.size(); j++) {
			ParseObject word = words.get(j);
			word.deleteEventually();
		}
	}
	
	/**
	 * Update the {@link AutoCorrect} engine based on our current cursor position.
	 * Call this to ensure that the auto-correct is in sync with the current input
	 */
	public void updateAutoCorrectOnCursorPosition() {
		log(Log.WARN, "UPDATE AUTOCORRECT ON CURSOR POSITION", "HERE");
		InputConnection ic = this.getCurrentInputConnection();
		
		if (ic != null) {
			String before = (String) ic.getTextBeforeCursor(LONG_WORD_LENGTH, 0);
			//log(Log.VERBOSE, "Setting auto-correct before cursor to", "= "+before);
			String after = (String) ic.getTextAfterCursor(LONG_WORD_LENGTH, 0);
			//log(Log.VERBOSE, "Setting auto-correct after cursor to", "= "+after);
			if (after == null) 
				after = "";
			if (before == null) 
				before = "";
				mAutoCorrect.replaceBuffers(before, after);
		}
	}
	
	/**
	 * Update the {@link PredictionEngine} based on the current cursor position.
	 * This will ensure that the Predictions are in sync with the current input
	 * 
	 * @param ignoreRoot If false, this function will check the engine's current root and compare it to the current input text.
	 * To see if it's necessary to reset the engine.
	 * @param replace Whether or not to replace previous word in pred engine with current word. This happens when a correction is done!
	 */
	public void updatePredictionEngineOnCursorPosition(boolean ignoreRoot, boolean fullReset) {
		log(Log.WARN, "UPDATE PREDICTION ENGINE ON CURSOR POSITION", "HERE");
		
		try {
			//if trial not expired, then proceed as normal
			if (!this.mTrialExpired) {
				switch (this.currentKeyboardLayout) {
					case KeyboardLinearLayout.QWERTY:
						this.updatePredictionEngineOnCursorLatin(ignoreRoot, fullReset);
					break;
					case KeyboardLinearLayout.QWERTY_SPANISH:
						this.updatePredictionEngineOnCursorLatin(ignoreRoot, fullReset);
					break;
					case KeyboardLinearLayout.AZERTY:
						this.updatePredictionEngineOnCursorLatin(ignoreRoot, fullReset);
					break;
					case KeyboardLinearLayout.QWERTZ:
						this.updatePredictionEngineOnCursorLatin(ignoreRoot, fullReset);
					break;
					case KeyboardLinearLayout.QZERTY:
						this.updatePredictionEngineOnCursorLatin(ignoreRoot, fullReset);
					break;
					case KeyboardLinearLayout.RUSSIAN:
						this.updatePredictionEngineOnCursorLatin(ignoreRoot, fullReset);
					break;
					case KeyboardLinearLayout.KOREAN:
						this.updatePredictionEngineOnCursorKorean(ignoreRoot, fullReset);
					break;
					case KeyboardLinearLayout.NUMERIC:
						this.updatePredictionEngineOnCursorLatin(ignoreRoot, fullReset);
					break;
					case KeyboardLinearLayout.SYMBOLS:
						this.updatePredictionEngineOnCursorLatin(ignoreRoot, fullReset);
					break;
					case KeyboardLinearLayout.SYMBOLS_2:
						this.updatePredictionEngineOnCursorLatin(ignoreRoot, fullReset);
					break;
					case KeyboardLinearLayout.EXTRA_LETTERS:
						this.updatePredictionEngineOnCursorLatin(ignoreRoot, fullReset);
					break;
					case KeyboardLinearLayout.SMILEY:
						this.updatePredictionEngineOnCursorLatin(ignoreRoot, fullReset);
					break;
					case KeyboardLinearLayout.NAVIGATION:
						this.updatePredictionEngineOnCursorLatin(ignoreRoot, fullReset);
					break;
					case KeyboardLinearLayout.COMPRESSED:
						this.updatePredictionEngineOnCursorLatin(ignoreRoot, fullReset);
					break;
					default:
						this.updatePredictionEngineOnCursorLatin(ignoreRoot, fullReset);
					break;
				}
			//trial is expired just send key to editor
			}
		} catch (Exception e) {
			sendErrorMessage(e);
		}
	}

    /**
     * Update the prediction engine because the cursor has moved position in the body of text.
     *
     * @param ignoreRoot whether or not to check if the current word is the same as the current root word in the engine
     * @param replace whether or not to replace the previous word in the engine with the new current word behind the cursor position.
     */
	private void updatePredictionEngineOnCursorLatin(boolean ignoreRoot, boolean fullReset) {
		try {
			if (!mPredictionOn) {
				return;
			}
			
			// reset the predictions and be editor aware where the cursor is 
			InputConnection ic = getCurrentInputConnection();
			if (ic != null) {
				String leadingText = this.getCurrentWord();
				boolean advanceComplete = false;

				if (leadingText == null || leadingText.length() <= 0) {
					leadingText = this.getLastWord(ic, true, true);
					if (leadingText.length() > 0) {
						String last = (String) ic.getTextBeforeCursor(1, 0);
						if (last != null && last.equals(" ")) {
							advanceComplete = true;
						} else if (last != null && last.equals(System.getProperty("line.separator"))) {
							advanceComplete = true;
						}
					}
				}
				
				log(Log.INFO, "Update pred on cursor", "Current word = "+leadingText + ", Engine root = "+mPredictionEngine.getRootWordNextWords());
				
				//if the prediction engines state is different from the current word
				//then it needs to be updated
				//Log.i("LEADING TEXT = "+leadingText, "root = "+mPredictionEngine.getRootWordNextWords()+"|");
				if ( ignoreRoot || (leadingText.length() > 0 && !leadingText.equals(mPredictionEngine.getRootWordNextWords())) ) {
					//Log.i("LEADING TEXT =", ""+leadingText+"|");
					bRemoveTrailingTextOnUpdate = (leadingText.length() > 0);

				    mPredictionEngine.reset(fullReset);

					if (leadingText.length() < LONG_WORD_LENGTH) {
                        if (advanceComplete) {
						    mPredictionEngine.advanceWordComplete(leadingText, false);
                        } else {
                            mPredictionEngine.advanceWord(leadingText, false);
                        }
					}

					this.numNextWords = mPredictionEngine.getNumNextWords();
					
					//log(Log.ERROR, "Pred updated on cursor, root = "+mPredictionEngine.getRootText(), "num next words = "+this.numNextWords);
					
					if (this.kbContainerView != null) {
						this.kbContainerView.getKbLinearLayout().doKeyHighlighting(false);
					}
				} else if (leadingText.length() <= 0) {
					mPredictionEngine.reset(false);
					this.numNextWords = mPredictionEngine.getNumNextWords();
				} else {
					this.numNextWords = mPredictionEngine.getNumNextWords();
				}
			}
		} catch (Exception e) {
			sendErrorMessage(e);
		}
	}
	
	private void updatePredictionEngineOnCursorKorean(boolean ignoreRoot, boolean fullReset) {
		try {
			if (!mPredictionOn) {
				return;
			}
			
			// reset the predictions and be editor aware where the cursor is 
			InputConnection ic = getCurrentInputConnection();
			if (ic != null) {
				//this.setCurrentWord(ic);
				
				this.setCurrentWord(ic);
				
				log(Log.VERBOSE, "updatePredOnCursorAsian", "current word = "+this.currentWord);
				
				if (this.currentWord != null && this.currentWord.length() <= 0) {
					this.currentHangul = new Hangul();
				} else {
					this.currentHangul = Hangul.split( this.currentWord.charAt(this.currentWord.length()-1) );
				}
				this.previousHangul = new Hangul();
				
				String decomposed = Hangul.decompose(this.currentWord.toString());
				
				mPredictionEngine.reset(true);
				
				for (int i=0; i < decomposed.length(); i++ ) {
					//mPredictionEngine.advanceLetter(decomposed.charAt(i), false);
                    mPredictionEngine.advanceLetter(decomposed.charAt(i), false, false);
				}
				
				this.numNextWords = mPredictionEngine.getNumNextWords();
			}
		} catch (Exception e) {
			sendErrorMessage(e);
		}
	}
	
	/**
	 * Sets the {@link PredictionEngine}'s root to the {@link String} provided
	 * @param newRoot the new root word for the engine
	 */
	private void updatePredictionEngine(int backspaceCount, String newRoot, boolean appendSpace) {
		log(Log.WARN, "updatePredictionEngine()", "start func");
		try {
			if (!mPredictionOn) {
				return;
			}
			
			// reset the predictions and be editor aware where the cursor is 
			InputConnection ic = getCurrentInputConnection();
			if (ic != null) {
				//log(Log.INFO, "Update predEngine new root", "New root = "+newRoot + ", Engine root = "+mPredictionEngine.getRootWordNextWords());
                if (appendSpace) {
				    mPredictionEngine.advanceWordComplete(newRoot, true);
                } else {
                    mPredictionEngine.advanceWord(newRoot, true);
                }

				this.numNextWords = mPredictionEngine.getNumNextWords();

                if (this.kbContainerView != null) {
                    this.kbContainerView.getKbLinearLayout().doKeyHighlighting(false);
                }
			}
		} catch (Exception e) {
			sendErrorMessage(e);
		}
	}
	
	/**
	 * Check the last two characters of the text for any punctuation.
	 * @param ic the {@link InputConnection} to use
	 * @return an array of {@link Character}s. If no punctuation are found (including space) then the character's
	 * returned will be 'e'.
	 */
	private char[] getLastPunctuation(InputConnection ic) {
        log(Log.WARN, "getLastPunctuation()", "start function");
		String text = (String) ic.getTextBeforeCursor(2, 0);
		
		char[] chs = new char[2];
		chs[0] = 'e';
		chs[1] = 'e';

        log(Log.VERBOSE, "getLastPunctuation", "last Two Chars = |"+text+"|");

		if (text != null && text.length() > 1) {
			if ( this.isWordSeparator(text.charAt(1)) ) {
				chs[1] = text.charAt(1);
			}
            if ( this.isWordSeparator(text.charAt(0)) ) {
				chs[0] = text.charAt(0);
			}
		} else if ( text != null && text.length() > 0 ) {
            if ( this.isWordSeparator(text.charAt(0)) ) {
                chs[0] = text.charAt(0);
            }
        }
		
		return chs;
	}

    /**
     * Reset the keystrokes that have been recorded as being ahead of the android system. This will ensure that if the user moves the
     * cursor, will update the engines appropriately.
     *
     * This function should only be called from the inputView when it's update timer has expired.
     */
    public void resetKeysAhead() {
        if (this.keysAheadOfSystem > 0) {
            this.keysAheadOfSystem = 0;
        }
    }

    public void incrementKeysAhead(int num) {
        this.keysAheadOfSystem += num;
    }
	
	/**
	 * Deal with the editor reporting movement of its cursor.
	 * 
	 * This function is not always guaranteed to be called and therefore can not be relied upon
	 * It's main use for us is to determine when the cursor has moved because of the user tapping on th etext in a different area.
	 * 
	 */
	@Override
	public void onUpdateSelection(int oldSelStart, int oldSelEnd, int newSelStart, int newSelEnd, int candidatesStart, int candidatesEnd) {
		log(Log.WARN, "ONUPDATESELECTION", "onUpdateSelection");
		try {
			super.onUpdateSelection(oldSelStart, oldSelEnd, newSelStart, newSelEnd, candidatesStart, candidatesEnd);
			
			//log(Log.WARN, "old sel start =" + oldSelStart, "old sel end = " + oldSelEnd);
			//log(Log.WARN, "new sel start =" + newSelStart, "new sel end = " + newSelEnd);
			//log(Log.WARN, "Cand start =" + candidatesStart, "cand end = " + candidatesEnd);
			//log(Log.ERROR, "Current word.length = ", ""+this.currentWord.length());
			//log(Log.INFO, "Correction made = ", ""+this.correctionMade);
			//log(Log.INFO, "Input view onkey = ", ""+this.mInputView.onkey);
			//log(Log.INFO, "Suggestion picked = ", ""+this.suggestionPicked);
			//log(Log.VERBOSE, "input view = ", "|"+this.mInputView.isShown()+"|");
			
			this.selectionStart = newSelStart;
			this.selectionEnd = newSelEnd;
			log(Log.VERBOSE, "ONUPDATESELECTION", "keysAheadOfSystem = "+this.keysAheadOfSystem+", onKey = "+this.kbContainerView.getKbLinearLayout().getKeyboardView().onkey
                    +", correctionMade = "+this.correctionMade+", suggestionPicked = "+this.suggestionPicked+", newSelEnd = "+newSelEnd+", oldSelEnd = "
                    +oldSelEnd+", newSelStart = "+newSelStart+", oldSelStart = "+oldSelStart+", completionOn = "+this.mCompletionOn);
			if (this.kbContainerView.getKbLinearLayout().getKeyboardView() != null && this.kbContainerView.getKbLinearLayout().getKeyboardView().isShown() && !this.mTrialExpired) {
				if (this.keysAheadOfSystem <= 0 && !this.kbContainerView.getKbLinearLayout().getKeyboardView().onkey && !this.correctionMade && !this.suggestionPicked
						&& (newSelEnd != oldSelEnd) && (newSelStart == newSelEnd) && !this.mCompletionOn && !this.miniAppTextSent) {
					//this.capitalizeFirstLetter = false;
					InputConnection ic = this.getCurrentInputConnection();
					ic.beginBatchEdit();
					//this.commitCurrentWord(ic, "onUpdateSelection");
					this.setCurrentWord(ic);
					ic.endBatchEdit();
					
					this.wordBeforeCorrection = null;
					
					this.updateAutoCorrectOnCursorPosition();
					this.updatePredictionEngineOnCursorPosition(false, true);
					this.updateWordToCorrect();
					this.updateCandidates();
				} else if (this.miniAppTextSent) {
					InputConnection ic = this.getCurrentInputConnection();
					ic.beginBatchEdit();
					this.setCurrentWord(ic);
					ic.endBatchEdit();
					this.miniAppTextSent = false;
					this.updateAutoCorrectOnCursorPosition();
					this.updatePredictionEngineOnCursorPosition(true, true);
					this.updateCandidates();
				}
				else if (this.kbContainerView.getKbLinearLayout().getKeyboardView() != null) {
					this.kbContainerView.getKbLinearLayout().getKeyboardView().onkey = false;
				}
				
				this.suggestionPicked = false;
				this.correctionMade = false;
				//this.wordBeforeCorrection = null;
				//log(Log.ERROR, "Update Select", "word correct to null");
				this.previousCanBeAdded = false;
			}
			
			/*
			 * This is a new way to keep track of the keystrokes that the user has entered.
			 * This is needed as this function can get called several times after the user has already finished typing a sequence,
			 * it plays "catch-up" and will call this function after every key stroke even if we have already logged all the key-strokes.
			 */
			if (this.keysAheadOfSystem > 0) {
				this.keysAheadOfSystem--;
				if (this.keysAheadOfSystem < 0) this.keysAheadOfSystem = 0;
			}
			
		} catch (Exception e) {
			sendErrorMessage(e);
		}
	}
	
	/**
	 * Delete entire words from the editor
	 * @return true if there is still more text that can be deleted, otherwise return false
	 */
	public boolean deleteEntireWordFromEditor() {
		log(Log.WARN, "DELETEENTIREWORDFROMEDITOR", "HERE");
		try {
			//if trial not expired, then proceed as normal
			if (!this.mTrialExpired) {

				this.suggestionPicked = false;
				this.correctionMade = false;
				this.wordBeforeCorrection = null;
				//log(Log.ERROR, "delete entire", "word correct to null");
				this.previousCanBeAdded = false;
				// reset the predictions and be editor aware where the cursor is 
				InputConnection ic = getCurrentInputConnection();
				
				ic.beginBatchEdit();
				this.commitCurrentWord(ic, "deleteEntireWord");

                this.clearCandidateView();

				String leadingText = "";
				if (ic != null) {

                    int deleteChars = mPredictionEngine.eraseLastWord();

                    this.numNextWords = mPredictionEngine.getNumNextWords();

                    if (deleteChars <= 0 || this.numNextWords <= 0) {
                        this.kbContainerView.getKbLinearLayout().getKeyboardView().updatePredEngineOnRelease = true;

                        leadingText = this.getTextBeforeSpace(ic);

                        if (leadingText == null || leadingText.length() <= 0) {
                            leadingText = (String) ic.getTextBeforeCursor(LONG_WORD_LENGTH, 0);
                        }

                        deleteChars = leadingText.length();
                    } else {
                        this.kbContainerView.getKbLinearLayout().getKeyboardView().updatePredEngineOnRelease = false;
                    }
                    //log(Log.ERROR, "Delete entire word", "leading text = |"+leadingText+"|");

                    ic.deleteSurroundingText(deleteChars, 0);
					
				//	if (mAutoCorrect != null) {
				//		mAutoCorrect.backspace(deleteChars);
				//	}

					if (this.kbContainerView != null) {
						this.kbContainerView.getKbLinearLayout().doKeyHighlighting(true);
					}
					leadingText = pullLastWord(ic);
				}
				
				ic.endBatchEdit();

				
				this.keysAheadOfSystem = 1;

				if (!this.mCompletionOn) {
					this.capitalizeFirstLetter = false;
					//this.updateWordToCorrect();
					//this.updateShiftKeyState(this.getCurrentInputEditorInfo());
				}
				
				if (leadingText != null && leadingText.length() > 0) {
					return true;
				} else {
					return false;
				}
			//trial is expired just send key to editor
			} else {
				this.getCurrentInputConnection().deleteSurroundingText(1, 0);
				String leadingText = pullLastWord(this.getCurrentInputConnection());
				if (leadingText != null && leadingText.length() > 0) {
					return true;
				} else {
					return false;
				}
			}
		} catch (Exception e) {
			sendErrorMessage(e);
			return false;
		}
	}

	/**
	 * Jump the cursor ahead or behind a word in the editor.
	 * @param bForward If true, move the cursor forward, otherwise move it backward
	 */
	public void jumpWordInEditor(boolean bForward) {
		log(Log.WARN, "JUMPWORDINEDITOR", "HERE");
		try {
			//Log.d("JUMP in editor", "start");
			mPredictionEngine.reset(false);
			// reset the predictions and be editor aware where the cursor is 
			InputConnection ic = getCurrentInputConnection();
			this.commitCurrentWord(ic, "jumpWord");
			
			ic.beginBatchEdit();
			
			if (ic != null) {
				if (bForward) {
					String trailingText = pullNextWord(ic);
					//log(Log.VERBOSE, "Jump word, next word =", "|"+trailingText+"|");
					if (trailingText != null) {
						int pos = this.selectionEnd + trailingText.length();
						this.selectionEnd = pos;
						//log(Log.VERBOSE, "Jump word forwards, pos =", "|"+pos+"|");
						ic.setSelection(pos, pos);
					}
				} else	{
					//String trailingText = pullText(ic, false);
					//String f = (String) ic.getTextBeforeCursor(1, 0);
					String leadingText = this.getTextBeforeSpace(ic);
					//log(Log.VERBOSE, "jump word in editor", "leadingText = "+leadingText+", selectionEnd = "+this.selectionEnd);
					if (leadingText != null) {
						int pos = this.selectionEnd - leadingText.length();
						this.selectionEnd = pos;
						//log(Log.VERBOSE, "Jump backwards", "Setting position to = "+pos);
						ic.setSelection(pos, pos);
					}
				}
			}
			
			ic.endBatchEdit();
			
			this.updatePredictionEngineOnCursorPosition(true, true);
			this.updateAutoCorrectOnCursorPosition();

            if (this.kbContainerView != null) {
                this.kbContainerView.getKbLinearLayout().doKeyHighlighting(true);
            }
			//updateCandidates();
		} catch (Exception e) {
			sendErrorMessage(e);
		}
	}
	
	/**
	 * This function will remove any text in front of the cursor.
	 * Use this when a prediction or correction has been selected in the middle of an existing word.
	 * 
	 * @param ic the {@link InputConnection} to use
	 * @param generatedSpaces  amount of spaces inserted before the trailing text
	 * @param calledFrom the function this was called from, for debugging purposes
	 * @param nSpacesToRemoveFromTrailingText the number of spaces that should be removed from any trailing text
	 */
	public void removeTrailingText(InputConnection ic, int generatedSpaces, String calledFrom, int nSpacesToRemoveFromTrailingText) {
		log(Log.WARN, "REMOVETRAILINGTEXT", "HERE");
		try {
			if (bRemoveTrailingTextOnUpdate) {
				if (ic != null) {
					String trailingText = pullText(ic, false);
					
					log(Log.VERBOSE, "remove trailing text = ", "= "+trailingText);
					if (trailingText.length() > 0 && !Character.isWhitespace(trailingText.charAt(0))) {
						int orgLen = trailingText.length();
						int nLeadingSpacesToRemove = 0;
						trailingText = trimTrailingPunctuation(trailingText);

						if (trailingText.length() < orgLen) {
							nSpacesToRemoveFromTrailingText = 0;
							nLeadingSpacesToRemove = generatedSpaces;
						}
						ic.deleteSurroundingText(nLeadingSpacesToRemove, trailingText.length()+ nSpacesToRemoveFromTrailingText);
					}
					else if (trailingText.length() == 0 && nSpacesToRemoveFromTrailingText > 0) {
						// this is for preventing that advanceWord at the end of a text correction will result in
						// one from advance word, and one from the space currently separating the word
						ic.deleteSurroundingText(0, nSpacesToRemoveFromTrailingText);
						//mPredictionEngine.backspaceLetter();
					}
				}
				// another cursor repositioning needs to happen before other trailing text will be removed
				bRemoveTrailingTextOnUpdate = false;
//				Log.d("WLKB", String.format("removeTrailingText bRemoveTrailTextOnUpdate %s", Boolean.toString(bRemoveTrailingTextOnUpdate)));
			}
		} catch (Exception e) {
			sendErrorMessage(e);
		}
	}
	
	/**
	 * This tells us about completions that the editor has determined based on
	 * the current text in it. We want to use this in fullscreen mode to show
	 * the completions ourself, since the editor can not be seen in that
	 * situation.
	 */
	@Override
	public void onDisplayCompletions(CompletionInfo[] completions) {
		//Log.e("KeyboardService", "On display completions");
		//this.commitCurrentWord("onDisplayCompletions");
		this.autocorrectOn = false;
		//this.mCompletionOn = true;
		//this.commitCurrentWord(this.getCurrentInputConnection(), "ondisplaycompletions");
		//super.onDisplayCompletions(completions);
	}

	/**
	 * Use this to monitor key events being delivered to the application. We get
	 * first crack at them, and can either resume them or let them continue to
	 * the app.
	 */
	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		log(Log.WARN, "ONKEYDOWN", "HERE");
		//Log.d("WordLogicKeyboard", "keycode = "+keyCode);
		try {
			switch (keyCode) {
			case KeyEvent.KEYCODE_BACK:
				break;
			case KeyEvent.KEYCODE_DEL:
				// Special handling of the delete key: if we currently are
				// composing text for the user, we want to modify that instead
				// of let the application do the delete itself.
				this.kbContainerView.getKbLinearLayout().getKeyboardView().onKey(this.kbContainerView.getKbLinearLayout().getKeyboardView().getKeyboard().findKey(IKnowUKeyboard.KEYCODE_DELETE));
				return true;
			//always let the system handle these hard keys, also don't hide the keyboard when these are pressed
			//it causes weird behavior on the ui side
			case KeyEvent.KEYCODE_ENTER:
				// Let the underlying text editor always handle these.
				//Log.d("ON KEY DOWN", "ENTER KEY PRESSED");
				break;
			case KeyEvent.KEYCODE_VOLUME_DOWN:
				//Log.d("ON KEY DOWN", "KEY VOLUME DOWN");
				break;
			case KeyEvent.KEYCODE_VOLUME_UP:
				//Log.d("ON KEY DOWN", "KEY VOLUME UP");
				break;
			case KeyEvent.KEYCODE_MENU:
				break;
			case KeyEvent.KEYCODE_CAMERA:
				break;
			default:
				// For all other keys, if we want to do transformations on
				// text being entered with a hard keyboard, we need to process
				// it and do the appropriate action.
				//Log.d("ON KEY DOWN", "KeyCode ="+keyCode);
				
				if (PROCESS_HARD_KEYS) {
					changeOverInputDisplayState = CANDLISTONLY;
					//rearrangeDisplay();
					if (mPredictionOn) {
						//return true;
					}
				}
			}
			return super.onKeyDown(keyCode, event);
		} catch (Exception e) {
			sendErrorMessage(e);
			return false;
		}
	}

	/**
	 * Use this to monitor key events being delivered to the application. We get
	 * first crack at them, and can either resume them or let them continue to
	 * the app.
	 */
	@Override
	public boolean onKeyUp(int keyCode, KeyEvent event) {
		log(Log.WARN, "ONKEYUP", "HERE");
		try {
			// If we want to do transformations on text being entered with a hard
			// keyboard, we need to process the up events to update the meta key
			// state we are tracking.

			//Log.d("WordLogicKeyboard", String.format("onKeyUp keyCode %x", keyCode));
			
			if (PROCESS_HARD_KEYS) {
				if (mPredictionOn) {
					mMetaState = MetaKeyKeyListener.handleKeyUp(mMetaState,
							keyCode, event);
				}
			}
			return super.onKeyUp(keyCode, event);
		} catch (Exception e) {
			sendErrorMessage(e);
			return false;
		}
	}

	/**
	 * Helper to update the shift state of our keyboard based on the current
	 * editor state.
	 * 
	 * @param attr the descriptive info of the current editor
	 */
	public void updateShiftKeyState(EditorInfo attr) {
		log(Log.WARN, "UPDATESHIFTKEYSTATE", "HERE");
		try {
            if (this.currentKeyboardLayout == KeyboardLinearLayout.KOREAN) {

                if (this.kbContainerView.getKbLinearLayout().getKeyboardView() != null && this.kbContainerView.getKbLinearLayout().getKeyboardView().getKeyboard() != null
                        && this.kbContainerView.getKbLinearLayout().getKeyboardView().getKeyboard().isShifted()) {
                    int caps = 0;
                    EditorInfo ei = getCurrentInputEditorInfo();

                    if (ei != null && ei.inputType != EditorInfo.TYPE_NULL && mAutoCapOn) {
                        caps = getCurrentInputConnection().getCursorCapsMode(attr.inputType);
                    }

                    boolean newShiftState = mCapsLock || caps != 0;
                    this.kbContainerView.getKbLinearLayout().getKeyboard().setShifted(newShiftState);
                }

            } else {
                if (attr != null && this.kbContainerView.getKbLinearLayout().getKeyboardView() != null) {
                    int caps = 0;
                    EditorInfo ei = getCurrentInputEditorInfo();

                    if (ei != null && ei.inputType != EditorInfo.TYPE_NULL && mAutoCapOn) {
                        caps = getCurrentInputConnection().getCursorCapsMode(attr.inputType);
                    }

                    boolean newShiftState = mCapsLock || caps != 0;
                    //mInputView.setShifted(newShiftState);

                    if (newShiftState && this.currentWord.length() <= 0) {
                        this.capitalizeFirstLetter = true;
                    }
                    /*
                     * TODO:
                     * Comment this back in for the new engine (when it only shows one word predictions) as this
                     * will change the capitalization behaviour
                     * */
                    else if (this.currentWord.length() <= 0) {
                        this.capitalizeFirstLetter = false;
                    }

                    //log(Log.VERBOSE, "updateshiftkey", "capFirstLetter = "+this.capitalizeFirstLetter);
                    if (this.kbContainerView.getKbLinearLayout().getKeyboard() != null) {
                        this.kbContainerView.getKbLinearLayout().getKeyboard().setShifted(newShiftState);
                    }
                }
            }
		} catch (Exception e) {
			sendErrorMessage(e);
		}
	}

	/**
	 * Helper to send a key down / key up pair to the current editor.
	 */
	private void keyDownUp(int keyEventCode) {
		log(Log.WARN, "KEYDOWNUP", "HERE");
		try {
			InputConnection ic = this.getCurrentInputConnection();
			if (ic != null) {
				if (keyEventCode == KeyEvent.KEYCODE_ENTER) {
					switch (this.editorAction) {
			            case EditorInfo.IME_ACTION_GO:
			            	mPredictionEngine.reset(true);
			                ic.performEditorAction(EditorInfo.IME_ACTION_GO);
			                break;
			            case EditorInfo.IME_ACTION_NEXT:
			            	mPredictionEngine.reset(true);
			            	ic.performEditorAction(EditorInfo.IME_ACTION_NEXT);
			                break;
			            case EditorInfo.IME_ACTION_SEARCH:
			            	mPredictionEngine.reset(true);
			            	ic.performEditorAction(EditorInfo.IME_ACTION_SEARCH);
			                break;
			            case EditorInfo.IME_ACTION_SEND:
			            	mPredictionEngine.reset(true);
			            	ic.performEditorAction(EditorInfo.IME_ACTION_SEND);
			                break;
			            default:
			            	//mPredictionEngine.reset(false);
			            	ic.sendKeyEvent(new KeyEvent(KeyEvent.ACTION_DOWN, keyEventCode));
			    			ic.sendKeyEvent(new KeyEvent(KeyEvent.ACTION_UP, keyEventCode));
			                break;
			        }
				} else {
					//mPredictionEngine.reset(false);
					ic.sendKeyEvent(new KeyEvent(KeyEvent.ACTION_DOWN, keyEventCode));
	    			ic.sendKeyEvent(new KeyEvent(KeyEvent.ACTION_UP, keyEventCode));
				}
			}
		} catch (Exception e) {
			sendErrorMessage(e);
		}
	}

	/**
	 * Helper to send a character to the editor as raw key events.
	 */
	public void sendKey(int keyCode) {
		log(Log.WARN, "SENDKEY", "HERE, keycode = "+keyCode);
		try {
			switch (keyCode) {
			case IKnowUKeyboard.KEYCODE_ENTER:
				keyDownUp(KeyEvent.KEYCODE_ENTER);
				break;
			default:
				if (keyCode >= '0' && keyCode <= '9') {
					keyDownUp(keyCode - '0' + KeyEvent.KEYCODE_0);
				} else {
					getCurrentInputConnection().commitText(String.valueOf((char) keyCode), 1);
				}
				break;
			}
		} catch (Exception e) {
			sendErrorMessage(e);
		}
	}

	/**
	 * Hide the candidate view
	 */
	private void hideCandidateView() {
		try {
			if (this.kbContainerView.getKbLinearLayout().getSuggestionView() != null) {
				/*LayoutParams candParams = (LayoutParams) this.kbContainerView.getKbLinearLayout().getSuggestionView().getLayoutParams();
				mCandParams = candParams;
				LinearLayout.LayoutParams layoutParams = new LinearLayout.LayoutParams(200, 0);
				this.kbContainerView.getKbLinearLayout().getSuggestionView().setLayoutParams(layoutParams);*/
			}
		} catch (Exception e) {
			sendErrorMessage(e);
		}
	}
	
	/**
	 * Hide the keyboard and only show the candidate view
	 */
	private void hideKeyboardViewShowCandidateOnlyView() {
		try {
			if (this.kbContainerView.getKbLinearLayout().getKeyboardView() != null) {
				this.kbContainerView.getKbLinearLayout().getKeyboardView().setVisibility(View.GONE);
			}
		} catch (Exception e) {
			sendErrorMessage(e);
		}
	}
	
	/**
	 * Show the keyboard view
	 */
	private void revealKbView() {
		try {
			this.kbContainerView.setVisibility(View.VISIBLE);
			if (kbContainerView.getKbLinearLayout().getKeyboardView() != null) {
                this.setKeyboard(this.currentKeyboardLayout, false, false, false);
			}
		} catch (Exception e) {
			sendErrorMessage(e);
		}
	}
	
	/**
	 * show the candidate view
	 */
	public void revealCandidateView() {
		try {
			if (kbContainerView.getKbLinearLayout().getSuggestionView() != null && mCandParams != null);
                //kbContainerView.getKbLinearLayout().getSuggestionView().setLayoutParams(mCandParams);
		} catch (Exception e) {
			sendErrorMessage(e);
		}
	}

	/**
	 * Determines whether the predictions returned from the {@link PredictionEngine} have any value
	 * or if they are just empty {@link String}s
	 * @param suggestionsAr The array of predictions from the engine
	 * @return true if there is at least one meaningful prediction, otherwise false
	 */
	private boolean hasMeaningFullPredictions(String[][] suggestionsAr) {
		log(Log.WARN, "HASMEANINGFULLPREDICTIONS", "HERE");
		try {
			if (suggestionsAr != null && suggestionsAr.length > 0) {
				for (int j = 0; j < suggestionsAr.length; j++) {
					 if (suggestionsAr[j][0] != null && suggestionsAr[j][0].length() > 0)
						 return true;
				}
			}
			
			return false;
		} catch (Exception e) {
			sendErrorMessage(e);
			return false;
		}
	}
	
	/**
	 * Helper to get the predictions from the {@link PredictionEngine}
	 * @return an array of {@link String}s
	 */
	public String[] getPredictions() {
		log(Log.WARN, "GETPREDICTIONS", "HERE"); 
		String[] ar = mPredictionEngine.getSuggestions();
		String[] ar2 = new String[0];
		if (ar != null) ar2 = ar.clone();
		
		return ar2;
	}

    /**
     * Helper to get the predictions from the {@link PredictionEngine}
     * @return an array of {@link String}s
     */
    public String[] getPhrasePredictions() {
        log(Log.WARN, "getPhrasePredictions()", "HERE");
        String[] ar = mPredictionEngine.getPhraseSuggestions();
        if (ar != null)
            log(Log.WARN, "getPhrasePredictions()", "ar = "+ar+", length = "+ar.length);
        String[] ar2 = new String[0];
        if (ar != null) ar2 = ar.clone();

        return ar2;
    }
    
    public String[] GetNounResult() {
        log(Log.WARN, "GetNounResult()", "HERE");
        String[] ar = mPredictionEngine.getNounSuggestions();
        String[] ar2 = new String[0];
        if (ar != null) ar2 = ar.clone();

        return ar2;
    }
	
	
	/*************************************************************
	 * Get predictions from the predictionEngine and corrections from the correction engine
	 * and merge them into a two-dimensional array of strings of at most five, that will be
	 * passed to the {@link SuggestionsLinearLayout} that will display them.
	 * 
	 * @param corrections the array of corrections obtained from the auto-correct
	 ****************************************************************************************/
	@SuppressLint("DefaultLocale")
	public void FillCandidateViewWithPredictions() 
	{
		log(Log.WARN, "FillCandidateViewWithPredictions", "HERE");
		String[] predictions = null;
        int availableSpaces  = SuggestionsLinearLayout.MAX_SUGGESTIONS;
		try {
            SuggestionsLinearLayout candView = this.kbContainerView.getKbLinearLayout().getSuggestionView();
 
			InputConnection ic = this.getCurrentInputConnection();
			if ((ic != null)  && (candView != null)) 
			{
			    candView.clear();
		//		String currentWord = this.getCurrentWord();
		//		String lastWord = this.getLastWord(ic, true, false);
				
                predictions = mPredictionEngine.m_nextWordsInfo.nextWordsAr;// this.getPredictions();
  
                int capsMode = Suggestion.CAPS_NONE;
                if (this.mCapsLock) 
                    capsMode = Suggestion.CAPS_ALL;
                else if (this.capitalizeFirstLetter) 
                    capsMode = Suggestion.CAPS_FIRST_LETTER;

                int textMode = Suggestion.TEXT_MODE_LATIN;
                if (this.currentKeyboardLayout==KeyboardLinearLayout.KOREAN)
                    textMode = Suggestion.TEXT_MODE_KOREAN;

/*				if (!this.showOnlyPreds && !this.suggestionPicked && !this.correctionMade && this.autocorrectOn &&
                        (currentWord.length() > 0) && !shouldCorrect) 
				{
					realWord = true;
					if (currentWord.length() > 1) 
					{
                        if (this.numNextWords >= SuggestionsLinearLayout.MAX_SUGGESTIONS) {
                            //candView.addPredictionSuggestion(currentWord + "...", "", capsMode, textMode, false, true);
                            candView.addPredictionSuggestion(currentWord + "   ", "", capsMode, textMode, false, true);
                        } else {
                            candView.addPredictionSuggestion(currentWord, "", capsMode, textMode, false, false);
                        }
                        availableSpaces--;
					}
				}
*/				
				// always show the corrected word in the bar so that the user can go back and type that
				// if they actually wanted it in there
				if ((this.lastCorrection != null) && (this.lastCorrection.length() > 0) && (this.wordBeforeCorrection != null)) 
				{
                    candView.addCorrectionSuggestion(this.wordBeforeCorrection, capsMode, textMode, this.previousCanBeAdded, true, false);
					availableSpaces--;
				}

				int predCounter = 0;
                // Fill all available spaces with predictions then corrections. Test for duplicates before adding a prediction or correction.
                while (availableSpaces>0 && (predCounter < predictions.length))// || corCounter < corrections.length)) 
                {
					if ((predCounter < predictions.length) && (predictions[predCounter].length() > 0)) 
					{
						String pred = predictions[predCounter];
						if (!candView.containsSuggestion(pred))
						{    
							candView.addPredictionSuggestion(pred, mPredictionEngine.m_nextWordsInfo.rootWord, capsMode, textMode, false, false);
							availableSpaces--;
						}
						predCounter++;
					} 
/*					else if ((corCounter < corrections.length) && (corrections[corCounter].length() > 0)) 
					{
						String cor = corrections[corCounter];
						int numpreds = mPredictionEngine.getNumWordsStartingWith(cor);
						if (!candView.containsSuggestion(cor)) 
						{
							candView.addCorrectionSuggestion(cor, capsMode, textMode, false, false, (numpreds > 0));
							availableSpaces--;
						}
						corCounter++;
					}
*/				}
				
				this.showOnlyPreds = false;

                candView.scrollToCenter();
			}
		} catch (Exception e) {
			log(Log.ERROR, "!!ERROR! exception at FillCandidateViewWithPredictions()", "predictions.length = "+predictions.length + ", availableSpaces="+availableSpaces);
			sendErrorMessage(e);
		}
	}
	
	/**************************************
	 * Send the newly merged predictions/corrections to the {@link SuggestionsLinearLayout}
	 ***************************************/
	/*private void sendSuggestionsToView() {
		log(Log.WARN, "SENDSUGGESTIONSTOVIEW", "HERE");
		boolean hasPredictions = true;
		if (suggestionsAr != null && suggestionsAr.length > 0) {
			hasPredictions = hasMeaningFullPredictions(suggestionsAr);
		}
		
		if (suggestionsAr != null && suggestionsAr.length > 0 && hasPredictions) {
			if (this.ikuScroller.getKeyboardScreen().getCandidateView() != null) {
				int capsMode = CorrectionSuggestion.CAPS_NONE;
				if (this.mCapsLock) {
					capsMode = CorrectionSuggestion.CAPS_ALL;
				} else if (this.capitalizeFirstLetter) {
					capsMode = CorrectionSuggestion.CAPS_FIRST_LETTER;
				}
				//log(Log.ERROR, "Send suggestions to view", "capsMode = "+capsMode);
				//Log.v("Send Suggestions to View", "Sending array to candidate view");
				switch (this.currentKeyboardLayout) {
				case KeyboardScreen.KOREAN:
                    this.ikuScroller.getKeyboardScreen().getCandidateView().setSuggestionsArray(suggestionsAr, mPredictionEngine.getRootWordNextWords(), capsMode, CorrectionSuggestion.TEXT_MODE_KOREAN);
					break;
				default:
                    this.ikuScroller.getKeyboardScreen().getCandidateView().setSuggestionsArray(suggestionsAr, mPredictionEngine.getRootWordNextWords(), capsMode, CorrectionSuggestion.TEXT_MODE_LATIN);
					break;
				}
				
				//mCandidateView.setSuggestionsArray(suggestionsAr, mPredictionEngine.getRootWordNextWords(), 0);
			}
		} else {
			if (this.ikuScroller.getKeyboardScreen().getCandidateView() != null) this.ikuScroller.getKeyboardScreen().getCandidateView().clear();
		}
	}*/

	//////////////////////////////////////////////////////////////////////////////////////////////////////
    private void addPhrasePredictions() {
        log(Log.WARN, "addPhrasePredictions()", "HERE");
        try {
            //this.kbContainerView.getKbLinearLayout().getSuggestionView().clear();

            InputConnection ic = this.getCurrentInputConnection();
            //log(Log.WARN, "Merge", "InputConnection = "+ic);
            if (ic != null) {
                String[] predictions = this.getPhrasePredictions();
                log(Log.VERBOSE, "addPhrasePredictions()", "predictions = "+predictions);
                //no phrasee predictions so return
                if (predictions == null || predictions.length <= 0) {
                    //return the keys back to
                    this.kbContainerView.getKbLinearLayout().getKeyboardView().getSuggestionsView().hidePhraseBar();
                    return;
                }

                int capsMode = Suggestion.CAPS_NONE;
                if (this.mCapsLock) {
                    capsMode = Suggestion.CAPS_ALL;
                } else if (this.capitalizeFirstLetter) {
                    capsMode = Suggestion.CAPS_FIRST_LETTER;
                }

                int textMode = Suggestion.TEXT_MODE_LATIN;
                switch (this.currentKeyboardLayout) {
                    case KeyboardLinearLayout.KOREAN:
                        textMode = Suggestion.TEXT_MODE_KOREAN;
                        break;
                    default:
                        textMode = Suggestion.TEXT_MODE_LATIN;
                        break;
                }

                log(Log.VERBOSE, "addPhrasePredictions()", "predictions.length = "+predictions.length);
                for (int i=0; i < predictions.length; i++) {
                    if (i < predictions.length && predictions[i] != null && predictions[i].length() > 0) {
                        //Log.d("Adding pred to array", "pos = "+(startingPos+i)+"pred = "+predictions[predCounter]);
                        this.kbContainerView.getKbLinearLayout().getSuggestionView().addPhraseSuggestion(predictions[i], "", capsMode, textMode);
                    }
                }

                //this.showOnlyPreds = false;
                //this.kbContainerView.getKbLinearLayout().getSuggestionView().scrollToCenter();
                //this.sendSuggestionsToView();
            }

            this.kbContainerView.getKbLinearLayout().getSuggestionView().scrollPhraseToCenter();
        } catch (Exception e) {
            sendErrorMessage(e);
        }
    }

	/**
	 * Update the list of available candidates from the current composing text.
	 * This will call all the appropriate functions get and merge the predictions and corrections
	 * 
	 * This will also determine the current Reach mini-apps to show to the user.
	 */
	public void updateCandidates() {
		try {
			log(Log.WARN, "UPDATECANDIDATES", "HERE");
			if (!mTrialExpired) {
                if (MINIAPP_ON) {
                    if (!keepCurrentMiniApp) {
                        ArrayList<String[]> mini = this.miniAppContextEngine.getContext(this, this.miniAppManager, "");
                        if (mini != null) {
                            //this.mInputView.setMiniAppIndicator(true);
                            int numMiniApps = this.miniAppManager.getMiniApps(mini);
                            if (numMiniApps > 0) {
                                //this.mInputView.setMiniAppIndicator(true);
                                this.kbContainerView.getSideRelativeLayout().highlightReachBG();
                            } else {
                                //this.mInputView.setMiniAppIndicator(false);
                                this.kbContainerView.getSideRelativeLayout().unHighlightReachBG();
                            }
                        } else {
                            if (this.kbContainerView != null) {
                                this.kbContainerView.getSideRelativeLayout().unHighlightReachBG();
                            }
                            this.miniAppManager.clearMiniAppList(true);
                            this.miniAppManager.prevTotal = 0;
                        }
                    } else {
                        keepCurrentMiniApp = false;
                    }
                }
				//long startTime = System.currentTimeMillis();
				//System.out.println("mTrialExpired? " + mTrialExpired);
				
		/*		String[] corrs = new String[5];
				if (this.autocorrectOn && !this.suggestionPicked && !this.correctionMade && mAutoCorrect.isSet()) {
                    this.startTimer();
					corrs = mAutoCorrect.getPendingCorrections(5);
                    this.stopTimer("updateCandidates() --> AutoCorrect.getPendingCorrections()");
				} else {
					for(int i=0; i<5; i++)
						corrs[i] = "";
				}
				*/
				if (!this.mCompletionOn) {
					this.FillCandidateViewWithPredictions();
                    if (PHRASE_PREDICTION_ON)
                        this.addPhrasePredictions();
				}
			}
            if (this.kbContainerView != null) {
                this.kbContainerView.getKbLinearLayout().getKeyboardView().invalidate();
            }
		} catch (Exception e) {
			sendErrorMessage(e);
		}
	}
	
	/**
	 * Handle a backspace key press
	 */
	public void handleBackspace() {
		log(Log.WARN, "HANDLEBACKSPACE", "HERE");
		try {
			//if trial not expired, then proceed as normal
			if (!this.mTrialExpired) {
				switch (this.currentKeyboardLayout) {
					case KeyboardLinearLayout.QWERTY:
						this.handleLatinBackspace();
					break;
					case KeyboardLinearLayout.QWERTY_SPANISH:
						this.handleLatinBackspace();
					break;
					case KeyboardLinearLayout.AZERTY:
						this.handleLatinBackspace();
					break;
					case KeyboardLinearLayout.QWERTZ:
						this.handleLatinBackspace();
					break;
					case KeyboardLinearLayout.QZERTY:
						this.handleLatinBackspace();
					break;
					case KeyboardLinearLayout.RUSSIAN:
						this.handleLatinBackspace();
					break;
					case KeyboardLinearLayout.KOREAN:
						this.handleKoreanBackspace();
					break;
					case KeyboardLinearLayout.NUMERIC:
						this.handleLatinBackspace();
					break;
					case KeyboardLinearLayout.SYMBOLS:
						this.handleLatinBackspace();
					break;
					case KeyboardLinearLayout.SYMBOLS_2:
						this.handleLatinBackspace();
					break;
					case KeyboardLinearLayout.EXTRA_LETTERS:
						this.handleLatinBackspace();
					break;
					case KeyboardLinearLayout.SMILEY:
						this.handleLatinBackspace();
					break;
					case KeyboardLinearLayout.NAVIGATION:
						this.handleLatinBackspace();
					break;
					case KeyboardLinearLayout.COMPRESSED:
						this.handleLatinBackspace();
					break;
					default:
						this.handleLatinBackspace();
					break;
				}
			//trial is expired just send key to editor
			} else {
				this.getCurrentInputConnection().deleteSurroundingText(1, 0);
			}
		} catch (Exception e) {
			sendErrorMessage(e);
		}
	}
	
	/***********************************************************************************
	 * Handle a backspace key press
	 ***********************************************************************************/
	private void handleLatinBackspace() 
	{
		//update that we have hit a key, since we do not want onUpdateSelection to fire any events if it doesn't have to
		this.keysAheadOfSystem++;
		
		this.suggestionPicked = false;
		this.correctionMade = false;
		this.wordBeforeCorrection = null;
		//log(Log.ERROR, "backspace", "word correct to null");
		InputConnection ic = this.getCurrentInputConnection();
		if (ic != null) {
			ic.beginBatchEdit();
			if (this.selectionEnd - this.selectionStart > 0) {
				ic.commitText("", 1);
				this.updatePredictionEngineOnCursorPosition(true, false);
				this.updateAutoCorrectOnCursorPosition();
            } else {
				if (this.mCompletionOn) {
					ic.deleteSurroundingText(1, 0);
				} else {
					//Log.d("I KNOW U SERVICE", "BackSpace Root text before = "+mPredictionEngine.getRootWordNextWords());

                    CharSequence last = ic.getTextBeforeCursor(1, 0);

					if (this.mPredictionOn && last != null && last.length() > 0) 
						mPredictionEngine.backspaceLetter();
										
					//if there is a current word just remove a letter from it
					//otherwise delete a letter from the editor, and try to set a current word again
					if (this.currentWord.length() > 0) {
			            this.deleteFromCurrentWord(this.currentWord.length() - 1, this.currentWord.length(), true);
			        } else {
			            ic.deleteSurroundingText(1, 0);
			            //this.setCurrentWord(ic);
			        }
					
				//	if (mAutoCorrect.isSet()) {
				//		mAutoCorrect.backspace();
				//	}
					
					String remainingtext = (String) ic.getTextBeforeCursor(LONG_WORD_LENGTH, 0);
					if (remainingtext != null && remainingtext.length() <= 0) {
						mPredictionEngine.reset(true);
					}

                    this.updatePredEngineHistoryLatin(true);

					//this.backspaceHit = true;
					if (this.mPredictionOn) {
						this.numNextWords = mPredictionEngine.getNumNextWords();

                        if (this.numNextWords <= 0) {
                            this.updatePredictionEngineOnCursorPosition(true, true);
                            this.updateAutoCorrectOnCursorPosition();
                        }
					}
					//updateCandidates();
					updateShiftKeyState(getCurrentInputEditorInfo());
					
					if (this.currentWord.length() <= 0) {
						this.setCurrentWord(ic);
					}
					
					//if there is a current word, check to see if we should be capitalizing the first letter of our predictions
					if (this.currentWord != null && this.currentWord.length() > 0) {
						this.capitalizeFirstLetter = Character.isUpperCase(this.currentWord.charAt(0));
					}
				}
			}
			
			ic.endBatchEdit();
		}
	}
	////////////////////////////////////////////////////////////////////////////////////////////
	private void handleKoreanBackspace() 
	{
		//update that we have hit a key, since we do not want onUpdateSelection to fire any events if it doesn't have to
		
		this.suggestionPicked = false;
		this.correctionMade = false;
		this.wordBeforeCorrection = null;
		//log(Log.ERROR, "backspace", "word correct to null");
		InputConnection ic = this.getCurrentInputConnection();
		if (ic != null) {
			ic.beginBatchEdit();
			if (this.selectionEnd - this.selectionStart > 0) {
				ic.commitText("", 1);
				//this.updatePredictionEngineOnCursorPosition(true, true);
				//this.updateAutoCorrectOnCursorPosition();
			} else {
				if (this.mCompletionOn) {
					ic.deleteSurroundingText(1, 0);
				} else {
					//String text = (String) ic.getTextBeforeCursor(1, 0);
					//Log.d("I KNOW U SERVICE", "BackSpace Root text before = "+mPredictionEngine.getRootWordNextWords());
					
					if (this.currentHangul != null && this.currentHangul.hasFinal() ) {
						
						final char[] split = Hangul.splitLastChar(this.currentHangul.finalc);
						if (split[0] != 0) {
							this.currentHangul.finalc = split[0];
							mPredictionEngine.backspaceLetter();
							mPredictionEngine.advanceLetter(split[0], true, false);
							final String text = "" + Hangul.combine(this.currentHangul.initial, this.currentHangul.medial, this.currentHangul.finalc);
							this.deleteFromCurrentWord(this.currentWord.length()-1, this.currentWord.length(), true);
							this.addToCurrentWord(ic, text, true);
						} else {
							this.currentHangul.finalc = 0;
							final String text = "" + Hangul.combine(this.currentHangul.initial, this.currentHangul.medial, this.currentHangul.finalc);
							mPredictionEngine.backspaceLetter();
							this.deleteFromCurrentWord(this.currentWord.length()-1, this.currentWord.length(), true);
							this.addToCurrentWord(ic, text, true);
						}
					} else if (this.currentHangul != null && this.currentHangul.hasMedial() ) {

                        final char[] split = Hangul.splitMedialChar(this.currentHangul.medial);

                        if (split[0] != 0) {
                            this.currentHangul.medial = split[0];
                            mPredictionEngine.backspaceLetter();
                            mPredictionEngine.advanceLetter(split[0], true, false);
                            final String text = "" + Hangul.combine(this.currentHangul.initial, this.currentHangul.medial, this.currentHangul.finalc);
                            this.deleteFromCurrentWord(this.currentWord.length()-1, this.currentWord.length(), true);
                            this.addToCurrentWord(ic, text, true);
                        } else {
                            this.currentHangul.medial = 0;
                            final String text = String.valueOf( (char) this.currentHangul.initial );
                            mPredictionEngine.backspaceLetter();
                            this.deleteFromCurrentWord(this.currentWord.length()-1, this.currentWord.length(), true);
                            this.addToCurrentWord(ic, text, true);
                        }
					} else {
						CharSequence charToDelete = ic.getTextBeforeCursor(1, 0);
						
						if (this.currentHangul.hasInitial() || ( charToDelete != null && charToDelete.length() > 0 && this.isWordSeparator(charToDelete.charAt(0)) ) ) {
							this.keysAheadOfSystem++;
						}
						
						mPredictionEngine.backspaceLetter();
						this.deleteFromCurrentWord(this.currentWord.length()-1, this.currentWord.length(), true);
						
						CharSequence cs = ic.getTextBeforeCursor(1, 0);
						
						if ( cs != null && cs.length() > 0 && !this.isWordSeparator(cs.charAt(0)) ) {
							char last = cs.charAt(0);
							this.currentHangul = Hangul.split(last);
						} else {
							this.currentHangul = new Hangul();
							this.previousHangul = new Hangul();
						}
					}

                    this.updatePredEngineHistoryKorean(true);
					
					//this.backspaceHit = true;
					if (this.mPredictionOn) {
						this.numNextWords = mPredictionEngine.getNumNextWords();
						//mPredictionEngine.setWordInfo();
						this.kbContainerView.getKbLinearLayout().doKeyHighlighting(false);
					}
					//updateCandidates();
					//updateShiftKeyState(getCurrentInputEditorInfo());
				}
			}
			
			ic.endBatchEdit();
		}
	}
	
	/**
	 * Handle a shift key press
	 */
	public void handleShift() {
		log(Log.WARN, "HANDLESHIFT", "HERE");
		try {
			if (this.kbContainerView.getKbLinearLayout().getKeyboardView() == null)
				return;
			
			boolean newShiftState = false;

            IKnowUKeyboard currentKeyboard = this.kbContainerView.getKbLinearLayout().getKeyboard();
            checkToggleCapsLock();
            newShiftState = mCapsLock || !currentKeyboard.isShifted();
            //this.mInputView.setShifted(newShiftState);
//            if (this.tabletSplitView) 
                currentKeyboard.setShifted(newShiftState);
			
			//capitalize the first letter of the current word if there is one
			//and set the newSentence variable to true
			if (newShiftState && this.currentWord != null && this.currentWord.length() > 0) {
				char f = Character.toUpperCase(this.currentWord.charAt(0));
				this.currentWord.setCharAt(0, f);
				//this.addToCurrentWord("");
				this.capitalizeFirstLetter = true;
			}

            this.kbContainerView.getKbLinearLayout().getKeyboardView().ignoreShift = true;
		} catch (Exception e) {
			sendErrorMessage(e);
		}
	}

    /**
     * Check the input to see if we are at a new sentece.
     *
     * @param ic the The current {@link InputConnection} to use for analyzing the text
     * @return whether or not we ar ebeginning a new sentence.
     */
    private boolean checkForNewSentence(InputConnection ic) 
    {
        //check the last two characters, first to see if there is a sentence ending character followed
        //by a space, meaning it is a new sentence
        //if not, then check just the last character to see if it is space, meaning we are not at a new sentence anymore
        boolean beginSentence = false;
        CharSequence lastTwo = ic.getTextBeforeCursor(2, 0);
        //log(Log.VERBOSE, "handle character", "last two = |"+lastTwo+"|");
        if (lastTwo != null && (lastTwo.length() == 0 || (lastTwo.length() > 1 &&
                (lastTwo.charAt(0) == '.' || lastTwo.charAt(0) == '!' || lastTwo.charAt(0) == '?') && lastTwo.charAt(1) == ' ') ) ) 
        {
            beginSentence = true;
        }
        else if (lastTwo != null && lastTwo.length() > 0) 
        {
            //if  the current word's first letter is capitalized, then keep it capitalized
            if (this.currentWord != null && this.currentWord.length() > 0) 
            {
                this.capitalizeFirstLetter = Character.isUpperCase(this.currentWord.charAt(0));
                //if there is no current word, and there is a space before the cursor then we do not need to capitalize now
            } 
            else if (lastTwo.length() > 1 && !this.isWordSeparator( lastTwo.charAt(0) ) && lastTwo.charAt(1) == ' ') 
            {
                this.capitalizeFirstLetter = false;
            }
        }
        return beginSentence;
    }
	
	/**
	 * Handle determine where to pass this key press of to for proper handling
	 * 
	 * @param primaryCode the primary key code of the character that was pressed
	 * @param keyCodes any other key codes associated with this character
	 * @throws NullPointerException 
	 */
	public void handleCharacter(String chars, byte[] prefs, int primaryCode) throws NullPointerException {
		log(Log.WARN, "HANDLECHARACTER", "HERE");
		try {
			//if trial not expired, then proceed as normal
			if (!this.mTrialExpired) {
				switch (this.currentKeyboardLayout) {
					case KeyboardLinearLayout.QWERTY:
						this.handleMultiCharacter(chars, prefs, primaryCode);
					break;
					case KeyboardLinearLayout.QWERTY_SPANISH:
						this.handleMultiCharacter(chars, prefs, primaryCode);
					break;
					case KeyboardLinearLayout.AZERTY:
						this.handleMultiCharacter(chars, prefs, primaryCode);
					break;
					case KeyboardLinearLayout.QWERTZ:
						this.handleMultiCharacter(chars, prefs, primaryCode);
					break;
					case KeyboardLinearLayout.QZERTY:
						this.handleMultiCharacter(chars, prefs, primaryCode);
					break;
					case KeyboardLinearLayout.RUSSIAN:
						this.handleMultiCharacter(chars, prefs, primaryCode);
					break;
					case KeyboardLinearLayout.KOREAN:
						this.handleKoreanCharacter(primaryCode);
					break;
					case KeyboardLinearLayout.NUMERIC:
						this.handleMultiCharacter(chars, prefs, primaryCode);
					break;
					case KeyboardLinearLayout.SYMBOLS:
						this.handleMultiCharacter(chars, prefs, primaryCode);
					break;
					case KeyboardLinearLayout.SYMBOLS_2:
						this.handleMultiCharacter(chars, prefs, primaryCode);
					break;
					case KeyboardLinearLayout.EXTRA_LETTERS:
						this.handleMultiCharacter(chars, prefs, primaryCode);
					break;
					case KeyboardLinearLayout.SMILEY:
						this.handleMultiCharacter(chars, prefs, primaryCode);
					break;
					case KeyboardLinearLayout.NAVIGATION:
						this.handleMultiCharacter(chars, prefs, primaryCode);
					break;
					case KeyboardLinearLayout.COMPRESSED:
						this.handleMultiCharacter(chars, prefs, primaryCode);
					break;
					default:
						this.handleMultiCharacter(chars, prefs, primaryCode);
					break;
				}
			//trial is expired just send key to editor
			} else {
				if (this.kbContainerView.getKbLinearLayout().getKeyboard().isShifted() && 
					!kbContainerView.getKbLinearLayout().getKeyboardView().mPopupKeyboardActive) {
					primaryCode = Character.toUpperCase(primaryCode);
				}
				this.getCurrentInputConnection().commitText(String.valueOf((char) primaryCode), 1);
			}
		} catch (Exception e) {
			sendErrorMessage(e);
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	
    public void handleMultiCharacter(String chars, byte[] prefs, int primaryCode) 
    {
        log(Log.WARN, "handleMultiCharacter()", "START");
        try {
            char mainChar = (char) primaryCode;
            if (mainChar == ' ')  //Minkyu:2013.12.12
            {
            	this.updatePredEngineHistoryLatin(false);
            }
            
            if (this.kbContainerView.getKbLinearLayout().getKeyboard().isShifted()) 
				mainChar = Character.toUpperCase(mainChar);

            //update that we have hit a key, since we do not want onUpdateSelection to fire any events if it doesn't have to
            this.keysAheadOfSystem++;

            this.suggestionPicked = false;
            this.correctionMade = false;
            this.wordBeforeCorrection = null;
            //log(Log.ERROR, "char", "word correct to null");

            InputConnection ic = this.getCurrentInputConnection();
			if ( ic != null ) {
            ic.beginBatchEdit();
            //boolean isNewSentence = false;

            if (isInputViewShown()) {
                if (this.kbContainerView.getKbLinearLayout().getKeyboard().isShifted()  &&
					!kbContainerView.getKbLinearLayout().getKeyboardView().mPopupKeyboardActive) {
                    chars = chars.toUpperCase();
                }
            }

/*                if (mAutoCorrect.isSet()) {
                    this.startTimer();
                    mAutoCorrect.addChar(mainChar);
                    this.stopTimer("handleCharacter() --> AutoCorrect.addChar()");
                }
*/
                if (this.mCompletionOn) {
                    ic.commitText(""+mainChar, 1);
                } else if (mPredictionOn) {

                //log(Log.VERBOSE, "handle character", "begin sentence = "+beginSentence);
                //int predCode =Integer.valueOf(primaryCode);
                boolean beginSentence = this.checkForNewSentence(ic);

                if(SWIPE_ENABLED)
                {
                	String temp = this.getCurrentWord();
     //            	mPredictionEngine.advanceLetterSwipe(temp+chars);
                }
                else
                {
                	mPredictionEngine.advanceLetterMulti(chars, prefs, beginSentence);
                }

                this.numNextWords = mPredictionEngine.getNumNextWords();
                //= mPredictionEngine.nextWords();

                //String textToPrint = mPredictionEngine.getAdvanceLetterPrintBuffer(chars);

                String textToPrint = ""+mainChar;
                //if this is the beginning of a sentence, make sure the first letter is capitalized
                //only if the keyboard is actually still shifted
                if (textToPrint != null && textToPrint.length() > 0 && this.kbContainerView.getKbLinearLayout().getKeyboard().isShifted() && beginSentence) {
                    char[] chrs = textToPrint.toCharArray();
                    char f = Character.toUpperCase(chrs[0]);
                    chrs[0] = f;
                    textToPrint = new String(chrs);
                }
                //mPredictionEngine.setWordInfo();

                    //handle lower case "i" in "i'm" and "i'll"
                    if (textToPrint != null && textToPrint.length() > 1 && (textToPrint.charAt(0) == 'I' || textToPrint.charAt(0) == 'i')) {
                        this.deleteFromCurrentWord(this.currentWord.length()-1, this.currentWord.length(), false);
                        //ic.deleteSurroundingText(1, 0);
                    }

                if (textToPrint != null && textToPrint.length() > 0) {
                    this.addToCurrentWord(ic, textToPrint, true);
                } else {
                    this.commitCurrentWord(ic, "handleCharacter");
                    ic.commitText(chars.substring(0,1), 0);
                }
                //this.updateShiftKeyState(getCurrentInputEditorInfo());
            } else {
                this.commitCurrentWord(ic, "handleCharacter predict off");
                ic.commitText(chars.substring(0,1), 1);
            }
            ic.endBatchEdit();
            
            this.updatePredEngineHistoryLatin(false);

            this.totalCharsTyped++;
            //mInputView.prepareNextKeyHighlighting();
            //this.updateShiftKeyState(getCurrentInputEditorInfo());
            //this.updateWordToCorrect();
			}
        } catch (Exception e) {
            sendErrorMessage(e);
        }
    }
	
	/**
	 * Handle a latin alpha-numeric character key press
	 * 
	 * @param primaryCode the primary key code of the character that was pressed
	 */
	private void handleLatinCharacter(int primaryCode) {
		log(Log.WARN, "HANDLELATINCHARACTER", "HERE");
		try {
			//update that we have hit a key, since we do not want onUpdateSelection to fire any events if it doesn't have to
			this.keysAheadOfSystem++;
			
			this.suggestionPicked = false;
			this.correctionMade = false;
			this.wordBeforeCorrection = null;
			//log(Log.ERROR, "char", "word correct to null");
			
			InputConnection ic = this.getCurrentInputConnection();
			
			ic.beginBatchEdit();
			//boolean isNewSentence = false;
			
			if (isInputViewShown()) {
				if (this.kbContainerView.getKbLinearLayout().getKeyboard().isShifted() && !kbContainerView.getKbLinearLayout().getKeyboardView().mPopupKeyboardActive) {
					primaryCode = Character.toUpperCase(primaryCode);
				}
			}
			
/*			if (mAutoCorrect.isSet()) {
				mAutoCorrect.addChar((char)primaryCode);
			}
*/			
			if (this.mCompletionOn) {
				ic.commitText(String.valueOf((char) primaryCode), 1);
			} else if (mPredictionOn) {

                boolean beginSentence = this.checkForNewSentence(ic);
				//log(Log.VERBOSE, "handle character", "begin sentence = "+beginSentence);
				int predCode =Integer.valueOf(primaryCode);
				if (beginSentence) predCode = Character.toLowerCase(predCode);
				mPredictionEngine.advanceLetter((char) predCode, beginSentence, false);
				
				this.numNextWords = mPredictionEngine.getNumNextWords();
				
				String textToPrint = mPredictionEngine.getAdvanceLetterPrintBuffer(String.valueOf((char) primaryCode));
				
				//if this is the beginning of a sentence, make sure the first letter is capitalized
				//only if the keyboard is actually still shifted
				if (textToPrint != null && textToPrint.length() > 0 && this.kbContainerView.getKbLinearLayout().getKeyboard().isShifted() && beginSentence) {
					char[] chrs = textToPrint.toCharArray();
					char f = Character.toUpperCase(chrs[0]);
					chrs[0] = f;
					textToPrint = new String(chrs);
				}
				//mPredictionEngine.setWordInfo();
				
				//handle lower case "i" in "i'm" and "i'll"
				if (textToPrint != null && textToPrint.length() > 1 && (textToPrint.charAt(0) == 'I' || textToPrint.charAt(0) == 'i')) {
					this.deleteFromCurrentWord(this.currentWord.length()-1, this.currentWord.length(), false);
					//ic.deleteSurroundingText(1, 0);
				}
				
				if (textToPrint != null && textToPrint.length() > 0) {
					this.addToCurrentWord(ic, textToPrint, true);
				} else {
					this.commitCurrentWord(ic, "handleCharacter");
					ic.commitText((char)primaryCode+"", 0);
				}
				//this.updateShiftKeyState(getCurrentInputEditorInfo());
			} else {
				this.commitCurrentWord(ic, "handleCharacter predict off");
				ic.commitText(String.valueOf((char) primaryCode), 1);
			}
			ic.endBatchEdit();
			
			this.totalCharsTyped++;
			//mInputView.prepareNextKeyHighlighting();
            //this.updateShiftKeyState(getCurrentInputEditorInfo());
			//this.updateWordToCorrect();
		} catch (Exception e) {
			sendErrorMessage(e);
		}
	}
	
	/**
	 * Handle an asian character key press - right now specific to Korean
	 * 
	 * @param primaryCode the code of the character that was pressed
	 */
	private void handleKoreanCharacter(int primaryCode) {
		try {
			//update that we have hit a key, since we do not want onUpdateSelection to fire any events if it doesn't have to
			
			this.suggestionPicked = false;
			this.correctionMade = false;
			this.wordBeforeCorrection = null;
			//log(Log.ERROR, "char", "word correct to null");

            char typed = (char) primaryCode;
			
			InputConnection ic = this.getCurrentInputConnection();
			
			ic.beginBatchEdit();
			
			if (this.mCompletionOn) {
				ic.commitText(String.valueOf(typed), 1);
			} else if (mPredictionOn) {
				
				IKnowUKeyboardService.log(Log.VERBOSE, "char is initial", "= "+Hangul.isInitial( typed ) + ", code = "+ primaryCode );
				
				//figure out what the character that should be added to the text should be
				String charToPrint = "";
				int deleteChars = 0;
				Boolean shouldSkip = false;
                char combined = 0;
				if ( this.currentHangul.hasInitial() && this.currentHangul.hasMedial() && this.currentHangul.hasFinal() ) {
					combined = Hangul.getCombined(this.currentHangul.finalc, typed);
					//log(Log.VERBOSE, "getCombined for "+this.currentHangul.finalc+" and "+typed, "= "+combined);
					if (combined != 0x0 && !Hangul.isInitial(combined)) {
						//log(Log.VERBOSE, "got Combined", "using combined");
						this.currentHangul.finalc = combined;
						mPredictionEngine.backspaceLetter();
						charToPrint =  ""+Hangul.combine(this.currentHangul.initial, this.currentHangul.medial, this.currentHangul.finalc);
						deleteChars = 1;
						shouldSkip = true;
					} else {
						this.previousHangul = Hangul.copy(this.currentHangul);
						this.currentHangul = new Hangul();
					}
				} else if ( this.currentHangul.hasInitial() && this.currentHangul.hasMedial() ) {
                    combined = Hangul.getCombinedMedial(this.currentHangul.medial, typed);
                    //log(Log.VERBOSE, "getCombined for "+this.currentHangul.finalc+" and "+typed, "= "+combined);
                    if (combined != 0) {
                        //log(Log.VERBOSE, "got Combined", "using combined");
                        this.currentHangul.medial = combined;
                        mPredictionEngine.backspaceLetter();
                        charToPrint =  ""+Hangul.combine(this.currentHangul.initial, this.currentHangul.medial, this.currentHangul.finalc);
                        deleteChars = 1;
                        shouldSkip = true;
                    }
                }

				if (!shouldSkip) {
					//if there is an intial character and a medial character, then check to see if this character can be a final character, and not a medial character
					//and then combine it with the current characters
					if ( this.currentHangul.hasInitial() && this.currentHangul.hasMedial() && Hangul.isFinal(typed) && !Hangul.isMedial(typed) ) {
						this.currentHangul.finalc = typed;
						charToPrint =  ""+Hangul.combine(this.currentHangul.initial, this.currentHangul.medial, this.currentHangul.finalc);
						deleteChars = 1;
						//this.currentHangul = new Hangul();
					//if there is an initial character present, then check to see if this character is not an initial character
					//and combine it with the current inital one
					} else if ( this.currentHangul.hasInitial() && !Hangul.isInitial(typed) ) {
						this.currentHangul.medial = typed;
						charToPrint =  ""+Hangul.combine(this.currentHangul.initial, this.currentHangul.medial, (char) 0);
						deleteChars = 1;
					} else {
						this.keysAheadOfSystem++;
						log(Log.INFO, "char typed", "increasing keysAheadOfSystem = "+this.keysAheadOfSystem);
						this.currentHangul = new Hangul();
						//if there is a previous charcter, and the character has a final character, and the final charcter can
						//be used as an initial character, then split those characters and use the final as the initial for the new  medial character that was typed
						if (this.previousHangul != null && this.previousHangul.hasFinal() && Hangul.isMedial(typed) ) {
							final char[] split = Hangul.splitLastChar(this.previousHangul.finalc);
                            if ( Hangul.isInitial(this.previousHangul.finalc) ) {
                                log(Log.VERBOSE, "didn't split last char", "using combined");
                                deleteChars = 1;
                                this.currentHangul.initial = this.previousHangul.finalc;
                                this.currentHangul.medial = typed;
                                charToPrint = ""+Hangul.combine(this.previousHangul.initial, this.previousHangul.medial, (char) 0);
                                charToPrint += Hangul.combine(this.currentHangul.initial, this.currentHangul.medial, (char) 0);
                            } else if (split[0] != 0) {
								deleteChars = 1;
								this.previousHangul.finalc = split[0];
								this.currentHangul.initial = split[1];
								this.currentHangul.medial = typed;
								charToPrint = ""+Hangul.combine(this.previousHangul.initial, this.previousHangul.medial, this.previousHangul.finalc);
								charToPrint += Hangul.combine(this.currentHangul.initial, this.currentHangul.medial, (char) 0);
							}
						}
						else if ( Hangul.isInitial( typed ) ) {
							this.currentHangul.initial = typed;
							charToPrint = ""+typed;
						} else {
							charToPrint = ""+typed;
						}
					}
				}
				
				//CharSequence cs = ic.getTextBeforeCursor(1, 0);
				if ( deleteChars > 0 ) {
					this.deleteFromCurrentWord(this.currentWord.length()-deleteChars, this.currentWord.length(), false);
				}
				
				if (charToPrint != null && charToPrint.length() > 0) {
					this.addToCurrentWord(ic, ""+charToPrint, true);
				} else {
					this.commitCurrentWord(ic, "handleCharacter");
					ic.commitText(typed+"", 0);
				}

                if (combined != 0) {
				    mPredictionEngine.advanceLetter(combined, false, false);
                } else {
                    mPredictionEngine.advanceLetter(typed, false, false);
                }
				
				this.numNextWords = mPredictionEngine.getNumNextWords();
				
				//this.updateShiftKeyState(getCurrentInputEditorInfo());
			} else {
				this.commitCurrentWord(ic, "handleCharacter predict off");
				ic.commitText(String.valueOf(typed), 1);
			}
			ic.endBatchEdit();

            this.updatePredEngineHistoryKorean(false);
			
			this.totalCharsTyped++;
		} catch (Exception e) {
			sendErrorMessage(e);
		}
	}
	
	/**
	 * Handle a word separator key press. This is a press on a non alpha-numeric key.
	 * @param primaryCode the primary code of the key that was pressed.
	 */
	public void handleWordSeparator(int primaryCode) {
		log(Log.WARN, "HANDLEWORDSEPERATOR", "HERE");
		try {
			//if trial not expired, then proceed as normal
			if (!this.mTrialExpired) {
				switch (this.currentKeyboardLayout) {
					case KeyboardLinearLayout.QWERTY:
						this.handleLatinWordSeparator(primaryCode);
					break;
					case KeyboardLinearLayout.QWERTY_SPANISH:
						this.handleLatinWordSeparator(primaryCode);
					break;
					case KeyboardLinearLayout.AZERTY:
						this.handleLatinWordSeparator(primaryCode);
					break;
					case KeyboardLinearLayout.QWERTZ:
						this.handleLatinWordSeparator(primaryCode);
					break;
					case KeyboardLinearLayout.QZERTY:
						this.handleLatinWordSeparator(primaryCode);
					break;
					case KeyboardLinearLayout.RUSSIAN:
						this.handleLatinWordSeparator(primaryCode);
					break;
					case KeyboardLinearLayout.KOREAN:
						this.handleKoreanWordSeparator(primaryCode);
					break;
					case KeyboardLinearLayout.NUMERIC:
						this.handleLatinWordSeparator(primaryCode);
					break;
					case KeyboardLinearLayout.SYMBOLS:
						this.handleLatinWordSeparator(primaryCode);
					break;
					case KeyboardLinearLayout.SYMBOLS_2:
						this.handleLatinWordSeparator(primaryCode);
					break;
					case KeyboardLinearLayout.EXTRA_LETTERS:
						this.handleLatinWordSeparator(primaryCode);
					break;
					case KeyboardLinearLayout.SMILEY:
						this.handleLatinWordSeparator(primaryCode);
					break;
					case KeyboardLinearLayout.NAVIGATION:
						this.handleLatinWordSeparator(primaryCode);
					break;
					case KeyboardLinearLayout.COMPRESSED:
						this.handleLatinWordSeparator(primaryCode);
					break;
					default:
						this.handleLatinWordSeparator(primaryCode);
					break;
				}
			//trial is expired just send key to editor
			} else {
				this.getCurrentInputConnection().commitText(String.valueOf((char) primaryCode), 1);
			}
		} catch (Exception e) {
			sendErrorMessage(e);
		}
	}
	
	//////////////////////////////////////////////////////////////////////////////////////////////
	private void handleLatinWordSeparator(int primaryCode) 
	{
		//update that we have hit a key, since we do not want onUpdateSelection to fire any events if it doesn't have to
		this.keysAheadOfSystem++;
		
		// Handle separator space and period are separators !!
		this.wordBeforeCorrection = this.getCurrentWord();
		this.lastCorrection = null;
		boolean isThisI = wordBeforeCorrection.equals("i");
		if(this.wordBeforeCorrection.length() > 0 && mPredictionEngine.m_nextWordsInfo.nextWordsAr.length > 0)
		{
			if(isThisI)
			{
				this.lastCorrection = "I";	
			}
			else
			{
				if(mPredictionEngine.m_nextWordsInfo.rootWord.length() == 0) // rootWord length is 0 if the word (typed till here, before the separtor) is not in any dictionary! so either a new word or a mistype!
				{
					this.lastCorrection = mPredictionEngine.m_nextWordsInfo.nextWordsAr[0]; // assumes the first nextWord is the top   correction					
				}
				else if(mPredictionEngine.m_nextWordsInfo.rootWordExist==0) // rootWordExist=true means rootWord up to here is a word and not just stems for other words!
				{
					for(int i=0; i<mPredictionEngine.m_nextWordsInfo.nextWordsAr.length; i++)
					{
						String word = mPredictionEngine.m_nextWordsInfo.nextWordsAr[i];
						if(Character.isUpperCase(word.charAt(0)))
						{
							String tempRoot = mPredictionEngine.m_nextWordsInfo.rootWord;
							String rWord = tempRoot.substring(0, 1).toUpperCase() + tempRoot.substring(1);
							if(mPredictionEngine.m_nextWordsInfo.nextWordsAr[i].startsWith(rWord)) //Need to compare with first upper case letter 
							{
								this.lastCorrection = mPredictionEngine.m_nextWordsInfo.nextWordsAr[i];
								break;
							}
						}
						else
						{
							if(mPredictionEngine.m_nextWordsInfo.nextWordsAr[i].startsWith(mPredictionEngine.m_nextWordsInfo.rootWord))
							{
								this.lastCorrection = mPredictionEngine.m_nextWordsInfo.nextWordsAr[i];
								break;
							}
						}

					}
				}
			}
				
		}
		
		if (this.wordBeforeCorrection.length() > 0) {
			this.totalWordsTyped++;
		}
		
		//if the enter key was pressed let the editor deal with it
		if (primaryCode == IKnowUKeyboard.KEYCODE_ENTER) {
			//log(Log.VERBOSE, "Enter key hit", "resetting");
			this.commitCurrentWord(this.getCurrentInputConnection(), "handleWordSepLatin enter key");
			
			//if a new line has been entered then pass it to the engines
			if (this.editorAction == 0) {
				int deleteLetters = mPredictionEngine.advanceLetterMulti( ""+((char) primaryCode), new byte[]{1}, false);

                //if we need to get rid of any extra spaces or whatnot, then do it here.
                if (deleteLetters > 0) {
                    this.deleteFromCurrentWord(this.currentWord.length() - deleteLetters, this.currentWord.length(), false);
                }
			}
			sendKey(primaryCode);
			this.numNextWords = mPredictionEngine.getNumNextWords();
			//this.mInputView.prepareNextKeyHighlighting();
			//updateShiftKeyState(getCurrentInputEditorInfo());
			//this.updateCandidates();
		} else {
			if (this.mCompletionOn) {
				sendKey(primaryCode);
			} else if (mPredictionOn) {
				InputConnection ic = this.getCurrentInputConnection();
                if (ic != null) {
                    ic.beginBatchEdit();

                    char charToAdd = '0';
                    if ((char) primaryCode == ' ') {
                        if (doubleSpacePeriod) {
                            charToAdd = this.handleSpace();
                        }
                    }

                    this.correctionMade = false;

                    char nextchar = charToAdd != '0' ? charToAdd : (char) primaryCode;
                    int deleteLetters = mPredictionEngine.advanceLetter(nextchar, false, this.isPasswordField || this.isEmailField || this.removeExtraSpaces ? true : false);
                    String charsToAdd = mPredictionEngine.getAdvanceLetterPrintBuffer(String.valueOf(nextchar));
                    //mPredictionEngine.advanceLetterMulti(""+nextchar, new byte[]{1});
                    
                    //if we need to get rid of any extra spaces or whatnot, then do it here.
                    log(Log.VERBOSE, "handleWordSepLatin()", "deleteLetters = "+deleteLetters);
                    if (deleteLetters > 0) {
                        this.deleteFromCurrentWord(this.currentWord.length() - deleteLetters, this.currentWord.length(), false);
          //              mAutoCorrect.backspace(deleteLetters);
                    }

                    this.numNextWords = mPredictionEngine.getNumNextWords();
                    //this needs to be after we try to do a correction
                    this.suggestionPicked = false;

                    log(Log.VERBOSE, "handleWordSepLatin()", "printBuffer = |"+charsToAdd+"|");

                    if (charsToAdd == null) {
                        charsToAdd = ""+ ((char) primaryCode);
                    }

                    if(this.lastCorrection != null && ((char) primaryCode == ' ' || (char) primaryCode == '.' || (char) primaryCode == '?') )
                    	this.makeCorrection(this.correctionStart, true);
                    
                    this.addToCurrentWord(ic, charsToAdd, true);
                    this.commitCurrentWord(ic, "handleWordSepLatin");

                    ic.endBatchEdit();

                    this.updatePredEngineHistoryLatin(false);

                    //this.mInputView.prepareNextKeyHighlighting();
                    bRemoveTrailingTextOnUpdate = false;
                    this.capitalizeFirstLetter = false;
                }
			} else {
				this.commitCurrentWord(this.getCurrentInputConnection(), "handleWordSepLatin predict off");
				// should theoretically be the string of advanceLetter instead
				sendKey(primaryCode);
			}
			
			this.totalCharsTyped++;
			//this.updateWordToCorrect();
			//updateShiftKeyState(getCurrentInputEditorInfo());
		}
	}
	
	private void handleKoreanWordSeparator(int primaryCode) {
		//update that we have hit a key, since we do not want onUpdateSelection to fire any events if it doesn't have to
		this.keysAheadOfSystem++;
		
		// Handle separator space and period are separators !!
		this.wordBeforeCorrection = this.getCurrentWord();
		//log(Log.ERROR, "word sep", "word correct to null");
		//this.newSentence = false;
		
		if (this.getCurrentWord().length() > 0) {
			this.totalWordsTyped++;
		}
		
		//if the enter key was pressed let the editor deal with it
		if (primaryCode == IKnowUKeyboardService.KEYCODE_ENTER) {
			//log(Log.VERBOSE, "Enter key hit", "resetting");
			this.commitCurrentWord(this.getCurrentInputConnection(), "handleWordSepKorean enter key");
			/*
			if (newAutoCorrect != null)
				newAutoCorrect.addChar((char) primaryCode);
				*/
			mPredictionEngine.advanceLetter((char) primaryCode, false, false);
			
			sendKey(primaryCode);
			this.numNextWords = mPredictionEngine.getNumNextWords();

            this.previousHangul = Hangul.copy(this.currentHangul);
            this.currentHangul = new Hangul();
			//this.mInputView.prepareNextKeyHighlighting();
			//updateShiftKeyState(getCurrentInputEditorInfo());
			//this.updateCandidates();
		} else {
			if (this.mCompletionOn) {
				sendKey(primaryCode);
			} else if (mPredictionOn) {
				InputConnection ic = this.getCurrentInputConnection();
				ic.beginBatchEdit();

                char charToAdd = '0';
                if ((char) primaryCode == ' ') {
                    if (doubleSpacePeriod) {
                        charToAdd = this.handleSpace();
                    }
                }

                this.correctionMade = false;

                int deleteLetters = 0;
                String charsToAdd = "";
                if (charToAdd != '0') {
                    deleteLetters = mPredictionEngine.advanceLetter(charToAdd, false, this.isPasswordField || this.isEmailField || this.removeExtraSpaces ? true : false);
                    charsToAdd = mPredictionEngine.getAdvanceLetterPrintBuffer(String.valueOf(charToAdd));
                    //mPredictionEngine.advanceLetterMulti(""+charToAdd, new byte[]{1});
                } else {
                    deleteLetters = mPredictionEngine.advanceLetter( (char) primaryCode, false, this.isPasswordField || this.isEmailField || this.removeExtraSpaces ? true : false);
                    charsToAdd = mPredictionEngine.getAdvanceLetterPrintBuffer(String.valueOf((char) primaryCode));
                    //mPredictionEngine.advanceLetterMulti(""+((char) primaryCode), new byte[]{1});
                }

                //if we need to get rid of any extra spaces or whatnot, then do it here.
                log(Log.VERBOSE, "handleWordSepLatin()", "deleteLetters = "+deleteLetters);
                if (deleteLetters > 0) {
                    this.deleteFromCurrentWord(this.currentWord.length() - deleteLetters, this.currentWord.length(), false);
             //       mAutoCorrect.backspace(deleteLetters);
                }

                this.numNextWords = mPredictionEngine.getNumNextWords();
                //this needs to be after we try to do a correction
                this.suggestionPicked = false;

                log(Log.VERBOSE, "handleWordSepLatin()", "printBuffer = |"+charsToAdd+"|");

                if (charsToAdd == null) {
                    charsToAdd = ""+ ((char) primaryCode);
                }

                this.addToCurrentWord(ic, charsToAdd, true);
                this.commitCurrentWord(ic, "handleWordSepKorean");

                //log(Log.VERBOSE, "Text to print sent to auto correct =", "|"+charsToAdd+"|");
       //         if (mAutoCorrect.isSet()) {
       //        	mAutoCorrect.addString(charsToAdd);
        //        }

                this.previousHangul = Hangul.copy(this.currentHangul);
                this.currentHangul = new Hangul();
				ic.endBatchEdit();

                this.updatePredEngineHistoryKorean(false);
				
				//this.mInputView.prepareNextKeyHighlighting();
				bRemoveTrailingTextOnUpdate = false;
			} else {
				this.commitCurrentWord(this.getCurrentInputConnection(), "handleWordSepKorean predict off");
				// should theoretically be the string of advanceLetter instead
				sendKey(primaryCode);
			}
			
			this.totalCharsTyped++;
			//this.updateWordToCorrect();
			//updateShiftKeyState(getCurrentInputEditorInfo());
		}
	}
	
	/*
	 * Look at previous character, if it is a space then delete it
	 */
	public void handleImplicitSpace() 
	{
		InputConnection ic = this.getCurrentInputConnection();
		
		CharSequence lastChar = ic.getTextBeforeCursor(1, 0);
		
		if(lastChar != null && lastChar.length() == 0) { return; }
		
		if(lastChar.charAt(0) == ' ') {
			ic.deleteSurroundingText(1, 0);
		//	mAutoCorrect.backspace();
			mPredictionEngine.backspaceLetter();
		}
	}

    public void updatePredEngineHistoryLatin(boolean backspace) {
        String lastFour = this.getWordsBeforeCursor(this.getCurrentInputConnection(), 4);

        log(Log.INFO, "updatePredEngineHistory()", "lastFour = |"+lastFour+"|");

        mPredictionEngine.setHistory(lastFour, backspace);
    }
    
    private void updatePredEngineHistoryKorean(boolean backspace) {
        String lastFour = this.getWordsBeforeCursor(this.getCurrentInputConnection(), 4);
        log(Log.INFO, "updatePredEngineHistory()", "lastFour = |"+lastFour+"|");

        lastFour = Hangul.decompose(lastFour);

        mPredictionEngine.setHistory(lastFour,backspace);
    }
	
	/**
	 * Get the specified number of words prior to the current cursor position.
	 * This will generally only be called when a correction inserted 2 or more words
	 * and the user wants to change it back to one.
	 *
	 * This function will grab the current word being typed if any, and
	 * count that as a word to be returned
	 *
	 * @param numWords
	 * @return the text found
	 */
	public String getWordsBeforeCursor(InputConnection ic, int numWords) {
		log(Log.WARN, "GETWORDSBEFORECURSOR", "HERE");
		try {
			String words = "";
			String text = (String) ic.getTextBeforeCursor(LONG_WORD_LENGTH, 0);
			String word = "";

            if (text != null) {
                String[] wordsAR = text.split(" ");

            int start = wordsAR.length-1;

            int stop = numWords <= wordsAR.length ? wordsAR.length-numWords : 0;

            log(Log.VERBOSE, "getWordsBeforeCursor()", "start = "+start+", stop = "+stop);
			//check to see where the cursor is, if there is a word being typed, grab
			//it and place it in the string that will be returned;
			for (int i=start; i >= stop; i--) {
				//if the text is valid
				if (text != null && text.length() > 0) {
					/*if (text.contains(" ") && text.lastIndexOf(" ") != text.length() - 1) {
						word = text.substring(text.lastIndexOf(" ")+1)+" ";
						text = text.substring(0, text.lastIndexOf(" "));
					} else if (text.contains(" ") && text.lastIndexOf(" ") == text.length() - 1) {
						text = text.substring(0, text.lastIndexOf(" "));

						if (text.contains(" ")) {
							word = text.substring(text.lastIndexOf(" ")+1)+" ";
							text = text.substring(0, text.lastIndexOf(" "));
						}
					} else {
						word = text+" ";
					}
					//Log.w("TEXT =", "|"+text+"|");
					//Log.v("WORD =", "|"+word+"|");
					words = word + words;*/
                    if (i == start && text.charAt(text.length()-1) != ' ') {
                        words = wordsAR[i];
                    } else {
                        words = wordsAR[i] + " " + words;
                    }
				}
			}
            }
			
			return words;
		} catch (Exception e) {
			sendErrorMessage(e);
			return "";
		}
	}

    /**
     * Get the specified number of words prior to the current cursor position.
     *
     * This function will grab the current word being typed if any, and
     * count that as a word to be returned
     *
     * @param numWords
     * @return the text found
     */
    public String getWordsBeforeCursorExtraLong(InputConnection ic, int numWords) {
        log(Log.WARN, "GETWORDSBEFORECURSOREXTRALONG", "HERE");
        try {
            String words = "";
            String text = (String) ic.getTextBeforeCursor(EXTRA_LONG_WORD_LENGTH, 0);

            if (text != null) {
                String[] wordsAR = text.split(" ");
                int start = wordsAR.length-1;
                int stop = numWords <= wordsAR.length ? wordsAR.length-numWords : 0;

                log(Log.VERBOSE, "getWordsBeforeCursorExtraLong()", "start = "+start+", stop = "+stop);
                //check to see where the cursor is, if there is a word being typed, grab
                //it and place it in the string that will be returned;
                for (int i=start; i >= stop; i--) {
                    //if the text is valid
                    if (text != null && text.length() > 0) {
                        if (i == start && text.charAt(text.length()-1) != ' ') {
                            words = wordsAR[i];
                        } else {
                            words = wordsAR[i] + " " + words;
                        }
                    }
                }

                return words;
            } else {
                return "";
            }
        } catch (Exception e) {
            sendErrorMessage(e);
            return "";
        }
    }
	
	/**
	 * Handle all closing functionality of the keyboard in order to hide it.
	 */
	public void handleClose() {
		try {
			//this.settingsOpen = false;
			requestHideSelf(0);
			setCandidatesViewShown(false);
			//mInputView.closing();
			
			//runTest();
			//this.launchAnalyzer();
		} catch (Exception e) {
			sendErrorMessage(e);
		}
	}
	
	/**
	 * Switch the input view to our custom made voice input view
	 */
	public void switchToVoiceInput() {
		
		boolean connected = this.checkNetworkState();
		
		if (connected) {
			VoiceInputLinearLayout viLin = (VoiceInputLinearLayout) getLayoutInflater().inflate( R.layout.voice_input, null);
			this.setInputView(viLin);
			viLin.init();
			viLin.setInputService(this);
		} else {
			Toast.makeText(this, "Network Connection Required", Toast.LENGTH_SHORT).show();
		}
	}
	
	/**
	 * Check the current network conneciton of the device
	 * @return true if a network connection of any type has been found, false otherwise
	 */
	private boolean checkNetworkState() {
		ConnectivityManager cm = (ConnectivityManager) this.getSystemService(Context.CONNECTIVITY_SERVICE);
		NetworkInfo netInfo = cm.getActiveNetworkInfo();
		if (netInfo != null && netInfo.isConnected()) {
			return true;
		}
		return false;
	}
	
	/**
	 * Starts the Play Store activity, or opens the browser to it if the Play Store app has not been installed.
	 * 
	 * Opens either one to this applications page. Can be used for getting the user to rate the application
	 * 
	 * @param packageName
	 */
	public void gotoMarket(String packageName) {
		try {
			Intent in = new Intent(Intent.ACTION_VIEW, Uri.parse("market://details?id="+packageName));
			in.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
			startActivity(in);
		} catch (Exception e) {
			Intent in = new Intent(Intent.ACTION_VIEW, Uri.parse("http://play.google.com/store/apps/details?id="+packageName));
			in.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
			startActivity(in);
		}
	}
	
	/**
	 * Switch the input view back to the main keyboard view
	 */
	public void switchToKeyboard() {
		this.setInputView(this.kbContainerView);
	}
	
	private void runTest() {
		try {
			
			//Class<?> someClass = Class.forName("com.iknowu.SearchList");
			//Method[] methods = someClass.getDeclaredMethods();
			
			//for (int i = 0; i < methods.length; i++) {
			//	Log.i("Method Name = ", ""+methods[i].getName());
			//}
			
			/*
			mPredictionEngine.advanceLetter((char) 'h');
			mPredictionEngine.nextWords();
			String s = mPredictionEngine.getAdvanceLetterPrintBuffer("h");
			Log.d("TEST", "WORDINFO 1 = "+mPredictionEngine.setWordInfo());
			getCurrentInputConnection().commitText(s, 1);
			//mPredictionEngine.setWordInfo();
			
			//updateCandidates();
			
			Thread.sleep(500);
			
			mPredictionEngine.advanceLetter((char) 'e');
			mPredictionEngine.nextWords();
			s = mPredictionEngine.getAdvanceLetterPrintBuffer("e");
			Log.d("TEST", "WORDINFO 2 = "+mPredictionEngine.setWordInfo());
			getCurrentInputConnection().commitText(s, 1);
			//mPredictionEngine.setWordInfo();
			
			//updateCandidates();
			
			Thread.sleep(500);
			
			mPredictionEngine.advanceLetter((char) ' ');
			mPredictionEngine.nextWords();
			s = mPredictionEngine.getAdvanceLetterPrintBuffer(" ");
			Log.d("TEST", "WORDINFO 3 = "+mPredictionEngine.setWordInfo());
			getCurrentInputConnection().commitText(s, 1);
			//mPredictionEngine.setWordInfo();
			
			//updateCandidates();
			
			Thread.sleep(500);
			
			mPredictionEngine.advanceWord("said...");
			s = mPredictionEngine.getAdvanceWordPrintBuffer("said...");
			mPredictionEngine.nextWords();
			Log.d("TEST", "WORDINFO 4 = "+mPredictionEngine.setWordInfo());
			getCurrentInputConnection().commitText(s, 1);
			
			//updateCandidates();
			
			Thread.sleep(500);
			
			Log.d("TEST", "WORDINFO 5 = "+mPredictionEngine.undoWordOrLetter());
			
			/*
			mPredictionEngine.advanceLetter((char) 'f');
			handleTextCorrectionsIfRequired(true, (int) 'f');
			mPredictionEngine.nextWords();
			s = mPredictionEngine.getAdvanceLetterPrintBuffer("f");
			//Log.d("predictkey TEST", "ROOT TEXT 3 = "+mPredictionEngine.getRootWordNextWords());
			getCurrentInputConnection().commitText(s, 1);
			mPredictionEngine.setWordInfo();
			
			updateCandidates();
			
			Thread.sleep(500);
			
			mPredictionEngine.backspaceLetter();
			getCurrentInputConnection().deleteSurroundingText(1, 0);
			mPredictionEngine.nextWords();
			//Log.d("predictkey TEST", "ROOT TEXT 4 = "+mPredictionEngine.getRootWordNextWords());
			mPredictionEngine.setWordInfo();
			
			updateCandidates();
			
			Thread.sleep(500);
			
			mPredictionEngine.advanceWord("not...");
			s = mPredictionEngine.getAdvanceWordPrintBuffer("not...");
			mPredictionEngine.nextWords();
			//Log.d("predictkey TEST", "ROOT TEXT 5 = "+mPredictionEngine.getRootWordNextWords());
			getCurrentInputConnection().commitText(s, 1);
			
			updateCandidates();
			*/
			
		} catch(Exception e) {
			//Log.d("predictkey TEST", "EXCEPTION = "+e.getMessage());
		}
	}
	
	/**
	 * Check whether the caps lock should be on
	 */
	private void checkToggleCapsLock() {
		try {
			if (!mCapsLock)
			{
				long now = System.currentTimeMillis();
				if (mLastShiftTime + 800 > now) {
					mCapsLock = !mCapsLock;
					mLastShiftTime = 0;
				} else {
					mLastShiftTime = now;
				}
			} else {
				mCapsLock = !mCapsLock;
				mLastShiftTime = 0;
			}
		} catch (Exception e) {
			sendErrorMessage(e);
		}
	}

	public boolean isCapsLock() {
		return mCapsLock;
	}
	
	private String getWordSeparators() {
		return mWordSeparators;
	}

	/**
	 * Determine if a {@link Character} is one of our defined word serparators
	 * @param ch the {@link Character} to check
	 * @return true if the {@link Character} is a word separator, or false otherwise
	 */
	public boolean isWordSeparator(char ch) {
		String separators = getWordSeparators();
		
		final char[] chars = new char[1];
		chars[0] = ch;
		final String sch = new String(chars);
		
		return separators.contains(sch);
	}

	/**
	 * Clear the candidate view of any predictions/corrections it may have right now
	 */
	public void clearCandidateView() {
		if (this.kbContainerView.getKbLinearLayout().getSuggestionView() != null) {
            this.kbContainerView.getKbLinearLayout().getSuggestionView().clear();
		}
	}

	/**
	 * Pick a suggestion from the {@link PredictionEngine} based on user touch input
	 * @param pred the prediction that was selected
	 * @param appendSpace whether or nto to add a space
	 * @param capsMode the current capitalization mode to be used with the prediction. 
	 * One of: lowercase, first letter, or all caps
	 */
	public void pickSuggestionManually(String pred, boolean appendSpace, int capsMode) {
		try {
			switch (this.currentKeyboardLayout) {
				case KeyboardLinearLayout.QWERTY:
					this.pickSuggestionManuallyLatin(pred, appendSpace, '\0', capsMode);
				break;
				case KeyboardLinearLayout.QWERTY_SPANISH:
					this.pickSuggestionManuallyLatin(pred, appendSpace, '\0', capsMode);
				break;
				case KeyboardLinearLayout.AZERTY:
					this.pickSuggestionManuallyLatin(pred, appendSpace, '\0', capsMode);
				break;
				case KeyboardLinearLayout.QWERTZ:
					this.pickSuggestionManuallyLatin(pred, appendSpace, '\0', capsMode);
				break;
				case KeyboardLinearLayout.QZERTY:
					this.pickSuggestionManuallyLatin(pred, appendSpace, '\0', capsMode);
				break;
				case KeyboardLinearLayout.RUSSIAN:
					this.pickSuggestionManuallyLatin(pred, appendSpace, '\0', capsMode);
				break;
				case KeyboardLinearLayout.KOREAN:
					this.pickSuggestionManuallyKorean(pred, appendSpace, '\0');
				break;
				case KeyboardLinearLayout.NUMERIC:
					this.pickSuggestionManuallyLatin(pred, appendSpace, '\0', capsMode);
				break;
				case KeyboardLinearLayout.SYMBOLS:
					this.pickSuggestionManuallyLatin(pred, appendSpace, '\0', capsMode);
				break;
				case KeyboardLinearLayout.SYMBOLS_2:
					this.pickSuggestionManuallyLatin(pred, appendSpace, '\0', capsMode);
				break;
				case KeyboardLinearLayout.EXTRA_LETTERS:
					this.pickSuggestionManuallyLatin(pred, appendSpace, '\0', capsMode);
				break;
				case KeyboardLinearLayout.SMILEY:
					this.pickSuggestionManuallyLatin(pred, appendSpace, '\0', capsMode);;
				break;
				case KeyboardLinearLayout.NAVIGATION:
					this.pickSuggestionManuallyLatin(pred, appendSpace, '\0', capsMode);
				break;
				case KeyboardLinearLayout.COMPRESSED:
					this.pickSuggestionManuallyLatin(pred, appendSpace, '\0', capsMode);
				break;
				default:
					this.pickSuggestionManuallyLatin(pred, appendSpace, '\0', capsMode);
				break;
			}
			
		} catch (Exception e) {
			sendErrorMessage(e);
		}
	}
	
	/**
	 * The functionality for picking a suggestion from the {@link PredictionEngine} based on user touch input
	 * when the keyboard is showing latin based characters
	 * 
	 * @param pred the prediction that was selected
	 * @param appendSpace whether or nto to add a space
	 * @param separator
	 * @param capsMode the current capitalization mode to be used with the prediction. 
	 * One of: lowercase, first letter, or all caps
	 */
	private void pickSuggestionManuallyLatin(String pred, boolean appendSpace, char separator, int capsMode) {
		log(Log.WARN, "pickSuggestionManuallyLatin()", "HERE");
		try {
			this.totalPredsSelected++;
			if (appendSpace) {
				this.totalWordsTyped++;
			}
			
			this.wordBeforeCorrection = null;
			//log(Log.ERROR, "sugg manual", "word correct to null");
			this.previousCanBeAdded = false;
			
			this.totalKeyStrokesSaved += (pred.length() - this.getCurrentWord().length()) + (appendSpace ? 1 : 0);
			
			//Log.v("PICK suggestion manually", "prediction = "+pred+", root = "+mPredictionEngine.getRootWordNextWords());
			InputConnection ic = this.getCurrentInputConnection();
			ic.beginBatchEdit();

            log(Log.VERBOSE, "pickSuggestionManuallyLatin()", "before advanceWord()");

			String root = mPredictionEngine.getRootWordNextWords();
			
			int deleteLetters = 0;
			//pred = pred.replace("\u2192", "");
			//pred = pred.replace("...", "");
            pred = pred.replace("   ", "");
			if (appendSpace) {
				deleteLetters = mPredictionEngine.advanceWordComplete(pred, false);
			} else {
				deleteLetters = mPredictionEngine.advanceWord(pred, false);
			}
			
			String s = mPredictionEngine.getAdvanceWordPrintBuffer(pred);
			log(Log.VERBOSE, "pickSuggestionManuallyLatin()", "prediction = |"+pred+"|, delete = "+deleteLetters+", printBuffer = |"+s+"| , appendSpace = "+appendSpace);
			/*
			if (root.length() > 0) {
				pred = pred.substring(root.length(), pred.length()-3);
			}
			*/
			//log(Log.VERBOSE, "Pick manually", "pred = |"+pred+"|");
		//	mAutoCorrect.addPrediction(pred);
			
			this.suggestionPicked = true;
			
			//this.currentWord.delete(0, deleteLetters);
			if (capsMode == 0) {
				String before = (String) ic.getTextBeforeCursor(deleteLetters, 0);
				if (before != null && before.length() > 0 && Character.isUpperCase(before.charAt(0))) {
					capsMode = CorrectionSuggestion.CAPS_FIRST_LETTER;
				}
			}
			
			this.deleteFromCurrentWord(this.currentWord.length() - deleteLetters, this.currentWord.length(), true);
			//ic.deleteSurroundingText(deleteLetters, 0);
			//handleTextCorrectionsIfRequired(false,0);
			
			switch (capsMode) {
				case CorrectionSuggestion.CAPS_ALL:
					s = s.toUpperCase();
					break;
				case CorrectionSuggestion.CAPS_FIRST_LETTER:
					char[] chrs = s.toCharArray();
					chrs[0] = Character.toUpperCase(chrs[0]);
					s = new String(chrs);
					break;
			}
			
			this.addToCurrentWord(ic, s, true);
			//ic.commitText(s, 1);
			
			if (appendSpace || s.charAt(s.length()-1) == ' ') {
				this.commitCurrentWord(ic, "pickSuggestionManually");
			}

			removeTrailingText(ic, appendSpace && mAutoSpaceOn ? 1 : 0, "pickSuggestionManuallyHelper", appendSpace ? 1 : 0);
			
//			if (appendSpace) {
//				mAutoCorrect.addChar(' ');
//			}
			ic.endBatchEdit();
			
			this.numNextWords = mPredictionEngine.getNumNextWords();
			//do this because of the way engine gets reset sometimes, only need to remove capitalization when there is a space
			//or when we run out of predictions, meaning a new root will need to be created
			String last = this.getLastWord(ic, true, true);
			boolean keepCap = false;
			if ( last.length() > 0 && Character.isUpperCase(last.charAt(0)) ) {
				keepCap = true;
			}
			
			if ( s.charAt(s.length()-1) == ' ' || !keepCap ) {
				this.capitalizeFirstLetter = false;
			} else {
				if (this.numNextWords <= 0) {
					this.capitalizeFirstLetter = false;
				}
			}
			log(Log.INFO, "Pick sugg", "capitalizeFirstLetter = "+this.capitalizeFirstLetter+", lastChar = |"+s.charAt(s.length()-1)+"|");
			//mInputView.prepareNextKeyHighlighting();
			
			this.updateWordToCorrect();
			updateShiftKeyState(getCurrentInputEditorInfo());

            this.updatePredEngineHistoryLatin(false);

			this.kbContainerView.getKbLinearLayout().doKeyHighlighting(false);
			this.updateCandidates();
		} catch (Exception e) {
			sendErrorMessage(e);
		}
	}
	
	/**
	 * The functionality for picking a suggestion from the {@link PredictionEngine} based on user touch input
	 * when the keyboard is showing east asian characters
	 * 
	 * @param pred the prediction that was selected
	 * @param appendSpace whether or nto to add a space
	 * @param separator
	 * One of: lowercase, first letter, or all caps
	 */
	private void pickSuggestionManuallyKorean(String pred, boolean appendSpace, char separator) {
		log(Log.WARN, "PICKSUGGESTIONMANUALLYASIAN", "HERE");
		try {
			this.totalPredsSelected++;
			if (appendSpace) {
				this.totalWordsTyped++;
			}
			
			this.suggestionPicked = true;
			this.wordBeforeCorrection = null;
			this.previousCanBeAdded = false;
			this.totalKeyStrokesSaved += (pred.length() - this.getCurrentWord().length()) + (appendSpace ? 1 : 0);
			
			//Log.v("PICK suggestion manually", "prediction = "+pred+", root = "+mPredictionEngine.getRootWordNextWords());
			InputConnection ic = this.getCurrentInputConnection();
			ic.beginBatchEdit();
			
			int deleteLetters = 0;
			String decomposed = Hangul.decompose(pred);
			if (appendSpace) {
				
				deleteLetters = mPredictionEngine.advanceWordComplete(decomposed, false);
			} else {
				deleteLetters = mPredictionEngine.advanceWord(decomposed, false);
			}
			
			log(Log.VERBOSE, "Pick sugg asian", "delete letters = "+deleteLetters+", pred = "+pred);
			
			boolean deleteDone = false;
			int charsDeleted = 0;
			String text = "";
			Hangul curHangul = new Hangul();
			while (!deleteDone) {
				
				text = (String) ic.getTextBeforeCursor(1, 0);
				if (text != null && text.length() > 0) {
					
					curHangul = Hangul.split( text.charAt(0) );
					
					if (curHangul.hasFinal()) charsDeleted++;
					if (curHangul.hasMedial()) charsDeleted++;
					if (curHangul.hasInitial()) charsDeleted++;
					
					if (charsDeleted <= deleteLetters) {
						this.deleteFromCurrentWord(this.currentWord.length() - 1, this.currentWord.length(), true);
					}
					
				} else {
					deleteDone = true;
				}
				
				if (charsDeleted >= deleteLetters) {
					deleteDone = true;
				}
			}
			
			this.bRemoveTrailingTextOnUpdate = true;
			removeTrailingText(ic, appendSpace && mAutoSpaceOn ? 1 : 0, "pickSuggestionManuallyHelper", appendSpace ? 1 : 0);
			//String s = mPredictionEngine.getAdvanceWordPrintBuffer(pred);
			String addPred = Hangul.combineChars(pred, true);
			//log(Log.VERBOSE, "Pick manually", "pred = |"+pred+"|");
			//newAutoCorrect.addPrediction(pred);
			
			//ic.deleteSurroundingText(deleteLetters, 0);
			//handleTextCorrectionsIfRequired(false,0);
			
			this.addToCurrentWord(ic, addPred, true);
			//ic.commitText(s, 1);
			
			if (appendSpace) {
				this.commitCurrentWord(ic, "pickPredAsian");
				ic.commitText(" ", 1);
				this.currentHangul = new Hangul();
			} else {
				this.currentHangul = Hangul.split( addPred.charAt(addPred.length()-1) );
			}
			this.previousHangul = new Hangul();

			//if (appendSpace) {
			//	newAutoCorrect.addChar(' ');
			//}
			ic.endBatchEdit();
			
			this.numNextWords = mPredictionEngine.getNumNextWords();
			//mInputView.prepareNextKeyHighlighting();
			
			this.updateWordToCorrect();
			updateShiftKeyState(getCurrentInputEditorInfo());

            this.updatePredEngineHistoryKorean(false);

            this.kbContainerView.getKbLinearLayout().doKeyHighlighting(false);
			this.updateCandidates();
		} catch (Exception e) {
			sendErrorMessage(e);
		}
	}
	
	/**
	 * Play the key sound and vibrate
	 * @param soundKey
	 */
	public void audioAndClickFeedback(int soundKey) {
		vibrate();
		playKeyClick(soundKey);
	}
	
	/**
	 * Load all of the settings saved in the {@link SharedPreferences}.
	 * Including themes, keyboard layouts, sounds, vibrations etc.
	 */
	private void loadSettings() {
		try {
			// Get the settings preferences
			Resources res = getResources();
			lmkey= res.getStringArray(R.array.language);
			
			SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(this);
			
			this.settingsOpen = false;
			
			this.setupComplete = sp.getBoolean(PREF_SETUP_COMPLETE, false);
			
			this.kbLayoutChanged = sp.getBoolean(PREF_KB_LAYOUT_CHANGED, true);

			this.currentKeyboardLayout = Integer.parseInt(sp.getString(PREF_KEYBOARD_LAYOUT, ""+KeyboardLinearLayout.QWERTY));
			
//			mVibrateOn = sp.getBoolean(PREF_VIBRATE_ON, true);
//			mVibrateSetting = sp.getInt(PREF_VIBRATE_SETTING, 0);
			int vibrate = sp.getInt(PREF_VIBRATE_SETTING, 15);
			mVibrateOn = vibrate <= 0 ? false : true;
			if (mVibrateOn)
				mVibrateDuration = vibrate;
			else
				mVibrateDuration = 0;
				
			int sound = sp.getInt(PREF_SOUND_SETTING, 50);
			mSoundOn = sound <= 0 ? false : true;
			if (mSoundOn)
				mSoundIntensity = sound / 100f;
			else
				mSoundIntensity = 0f;
			
			this.themeId = Integer.parseInt(sp.getString(PREF_THEME_SETTING, DEFAULT_KEYBOARD_THEME_ID+""));
            Theme.setTheme(this, this.themeId);

            mHighlightedKeysOn = sp.getBoolean(PREF_HIGHLIGHTEDKEYS_ON, true);
			mAutoCapOn = sp.getBoolean(PREF_AUTO_CAP, true);
//			mAutoSpaceOn = sp.getBoolean(PREF_AUTO_SPACE, true);
			mAutoSpaceOn = true;
			//mInlineAdditionOn = sp.getBoolean(PREF_INLINE_ADDITION_ON, false);
			mAutoLearnOn = sp.getBoolean(PREF_AUTOLEARN_ON, true);
			mLargeKeysOn = sp.getBoolean(PREF_LARGEKEYS_ON, true);
			
			//changing this to allow android app to override autolearn capabilities
			//mPredictionEngine.setAutoLearn(mAutoLearnOn);
			mPredictionEngine.setAutoLearn(false);
			
			this.autocorrectOn = sp.getBoolean(PREF_AUTOCORRECT_ON, true);
			this.insertCorrection = sp.getBoolean(PREF_AUTOCORRECT_INSERT, true);
			
			this.correctionFeedback = sp.getBoolean(PREF_CORRECTION_FEEDBACK, true);
			
			Theme.LONG_PRESS_TIMEOUT = sp.getInt(PREF_LONG_PRESS_TIMEOUT, 300);
            Theme.DELETE_WORD_DELAY_TIMEOUT = sp.getInt(PREF_DELETE_WORD_DELAY_TIMEOUT, 300);
			mHandednessRight = sp.getBoolean(PREF_HANDEDNESS_RIGHT, true);
            this.tabsOnRight = sp.getBoolean(PREF_TABS_SIDE, true);

            SWIPE_ENABLED = sp.getBoolean(PREF_SWIPE_ENABLED, true);
            PHRASE_PREDICTION_ON = sp.getBoolean(PREF_PHRASE_PREDICTIONS, false);
			
			int tabletLayout = Integer.parseInt(sp.getString(PREF_TABLET_LAYOUT, "0"));
			
			this.tabletSplitView = false;
			this.tabletThumbView = false;
			
			if (tabletLayout == 2 && this.mLandscape) {
				this.tabletSplitView = true;
			} else if (tabletLayout == 1 && this.mLandscape) {
				this.tabletThumbView = true;
			}

            log(Log.INFO, "loadSettings()!", "tabletThumb = "+this.tabletThumbView+", tabletLayout = "+tabletLayout);
			
			this.doubleSpacePeriod = sp.getBoolean(PREF_DOUBLE_SPACE, true);
			
			float keyHeightPref = sp.getInt(PREF_KEY_HEIGHT, 75);
            //combat the changing of the preference values from old versions to new versions
			if (keyHeightPref < 50) {
                keyHeightPref = 75;
            }
			this.keyHeightDP = keyHeightPref / 75.0f;
			log(Log.INFO, "keyHeightPref!!!!!", "heightPref = "+keyHeightPref+", heightDP = "+this.keyHeightDP);
			
			//mPredictionEngine.setSpacelessTyping(sp.getBoolean(PREF_SPACELESS_TYPING, true));
            mPredictionEngine.setSpacelessTyping(false);

			this.cloudSyncOn = sp.getBoolean(PREF_CLOUD_SYNC_ON, false);
			this.doLogin();
		} catch (Exception e) {
			sendErrorMessage(e);
		}
	}
	
	/**
	 * Copy any highlighted text to the clipboard
	 */
	@SuppressLint("ServiceCast")
	@TargetApi(11)
	public void copy() {
		try {
			if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.HONEYCOMB) {
				android.content.ClipboardManager clipboard = (android.content.ClipboardManager) getSystemService(Context.CLIPBOARD_SERVICE);
				InputConnection ic = this.getCurrentInputConnection();
				String text = (String) ic.getSelectedText(0);
				if (clipboard != null) {
					if (text != null && text.length() > 0) {
						ClipData clip = ClipData.newPlainText(PLAIN_TEXT_CLIP_DATA, text);
						clipboard.setPrimaryClip(clip);
						this.showAlert("Text copied to clipboard");
					}
				}
			} else {
				ClipboardManager clipboard = (ClipboardManager) getSystemService(Context.CLIPBOARD_SERVICE);
				InputConnection ic = this.getCurrentInputConnection();
				//workaround due to lack of this capability in android 2.2 and lower
				int selLength = this.selectionEnd - this.selectionStart;
				if ( selLength > 0 ) {
					ic.setSelection(this.selectionStart, this.selectionStart);
					String text = (String) ic.getTextAfterCursor(selLength, 0);
					if (clipboard != null) {
						if (text != null && text.length() > 0) {
							clipboard.setText(text);
							this.showAlert("Text copied to clipboard");
							ic.setSelection(this.selectionStart, this.selectionEnd);
						}
					}
				}
			}
		} catch (Exception e) {
			sendErrorMessage(e);
		}
	}
	
	/**
	 * Copy the specified {@link String} to the clipboard
	 * 
	 * Right now only used by the Reach mini-apps
	 * @param text1 the text to be placed on the clipboard
	 */
	@SuppressLint("ServiceCast")
	@TargetApi(11)
	public void copy(String text1) {
		try {
            if (MINIAPP_ON) {
                String text = this.miniAppMessageReceiver.clipText;
                if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.HONEYCOMB) {
                    android.content.ClipboardManager clipboard = (android.content.ClipboardManager) getSystemService(Context.CLIPBOARD_SERVICE);
                    if (clipboard != null) {
                        log(Log.VERBOSE, "IKUKbService", "copying text = " +text);
                        if (text != null && text.length() > 0) {
                            ClipData clip = ClipData.newPlainText(PLAIN_TEXT_CLIP_DATA, text);
                            clipboard.setPrimaryClip(clip);
                            this.showAlert("Text copied to clipboard");
                        }
                    }
                } else {
                    ClipboardManager clipboard = (ClipboardManager) getSystemService(Context.CLIPBOARD_SERVICE);
                    InputConnection ic = this.getCurrentInputConnection();
                    //workaround due to lack of this capability in android 2.2 and lower
                    if (clipboard != null) {
                        log(Log.VERBOSE, "IKUKbService", "copying text = " +text);
                        if (text != null && text.length() > 0) {
                            clipboard.setText(text);
                            this.showAlert("Text copied to clipboard");
                            ic.setSelection(this.selectionStart, this.selectionEnd);
                        }
                    }
                }
            }
		} catch (Exception e) {
			sendErrorMessage(e);
		}
	}
	
	/**
	 * Cut any highlted text from the editor to the clipboard
	 */
	@SuppressLint("ServiceCast")
	@TargetApi(11)
	public void cut() {
		try {
			if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.HONEYCOMB) {
				android.content.ClipboardManager clipboard = (android.content.ClipboardManager) getSystemService(Context.CLIPBOARD_SERVICE);
				InputConnection ic = this.getCurrentInputConnection();
				if (clipboard != null) {
					String text = (String) ic.getSelectedText(0);
					if (text != null && text.length() > 0) {
						ic.commitText("", 1);
						ClipData clip = ClipData.newPlainText(PLAIN_TEXT_CLIP_DATA, text);
						clipboard.setPrimaryClip(clip);
					}
				}
			} else {
				ClipboardManager clipboard = (ClipboardManager) getSystemService(Context.CLIPBOARD_SERVICE);
				InputConnection ic = this.getCurrentInputConnection();
				//showAlert("Cursor pos = ");
				//workaround due to lack of this capability in android 2.2 and lower
				int selLength = this.selectionEnd - this.selectionStart;
				if ( selLength > 0 ) {
					ic.setSelection(this.selectionStart, this.selectionStart);
					String text = (String) ic.getTextAfterCursor(selLength, 0);
					ic.setSelection(this.selectionStart, this.selectionEnd);
					if (clipboard != null) {
						if (text != null && text.length() > 0) {
							ic.commitText("", 1);
							clipboard.setText(text);
						}
					}
				}
			}
		} catch (Exception e) {
			sendErrorMessage(e);
		}
	}
	
	/**
	 * Paste any text from the clipboard to the current input editor
	 */
	@SuppressLint("ServiceCast")
	@TargetApi(11)
	public void paste() {
		try {
			if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.HONEYCOMB) {
				android.content.ClipboardManager clipboard = (android.content.ClipboardManager) getSystemService(Context.CLIPBOARD_SERVICE);
				if (clipboard != null && clipboard.hasPrimaryClip()) {
					ClipData.Item item = clipboard.getPrimaryClip().getItemAt(0);
					SpannableString text = new SpannableString(item.getText());
					if (text != null && text.length() > 0) {
						InputConnection ic = this.getCurrentInputConnection();
						ic.commitText(text, 1);
					}
				}
			} else {
				ClipboardManager clipboard = (ClipboardManager) getSystemService(Context.CLIPBOARD_SERVICE);
				if (clipboard != null) {
					SpannableString text = new SpannableString(clipboard.getText());
					if (text != null && text.length() > 0) {
						InputConnection ic = this.getCurrentInputConnection();
						ic.commitText(text, 1);
					}
				}
			}
		} catch (Exception e) {
			sendErrorMessage(e);
		}
	}
	
	public void select() {
		
	}
	
	/**
	 * Select all of the text from the editor
	 */
	public void selectAll() {
		try {
			InputConnection ic = this.getCurrentInputConnection();
			ic.performContextMenuAction(android.R.id.selectAll);
			/*
			InputConnection ic = this.getCurrentInputConnection();
			String before = (String) ic.getTextBeforeCursor(1000000, 0);
			String allText = before + ic.getTextAfterCursor(1000000, 0);
			ic.setSelection(0, allText.length());
			*/
		} catch (Exception e) {
			sendErrorMessage(e);
		}
	}
	
	/*
	 * Initiate the login process, in other words, try to log the user in
	 */
	private void doLogin() {
		try {
			if (this.cloudSyncOn) {
				SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(this);
				ParseUser user = ParseUser.getCurrentUser();
				
				//if there is a currentUser check to see if they
				//are supposed to still be logged in
				if (user != null) {
					String email = sp.getString(PREF_SIGNUP_FIRST, "");
					String pass = sp.getString(PREF_SIGNUP_SECOND, "");
					//Log.d("PREF EMAIL = "+email, "USER EMAIL = "+user.getEmail());
					if (!user.getEmail().equals(email)) {
						ParseUser.logOut();
						login(email, pass);
					} else {
						createUser();
					}
				//otherwise try to log in, and if that fails, sign them up
				} else {
					String username = sp.getString(PREF_SIGNUP_FIRST, "");
					String pass = sp.getString(PREF_SIGNUP_SECOND, "");
					
					//if these preferences have been set, try to log in
					//if log in fails, sign them up
					if (username.length() > 0 && pass.length() > 0) {
						login(username, pass);
					}
				}
			}
		} catch (Exception e) {
			sendErrorMessage(e);
		}
	}
	
	/**
	 * Try to actually log the user in.
	 * @param username the username
	 * @param pass the password
	 */
	private void login(String username, String pass) {
		ParseUser.logInInBackground(username, pass,
			new LogInCallback() {
				public void done(ParseUser user, ParseException e) {
					if (user != null) {
						showAlert("Logged in as "+user.getEmail());
						createUser();
					} else {
						if (e.getCode() == ParseException.OBJECT_NOT_FOUND) {
							signUp();
						}
					}
				}
			});
	}
	
	/**
	 * Try to sign the user up for cloud functionality
	 */
	private void signUp() {
		try {
			//Log.d("CAN'T LOG IN", "SIGNING UP");
			SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(this);
			String username = sp.getString(PREF_SIGNUP_FIRST, "");
			//Log.d("USERNAME =", ""+username);
			String pass = sp.getString(PREF_SIGNUP_SECOND, "");
			ParseUser user = new ParseUser();
			user.setUsername(username);
			user.setPassword(pass);
			user.setEmail(username);
			
			user.signUpInBackground(new SignUpCallback() {
				public void done(ParseException e) {
					if (e == null) {
						showAlert("Sign up successful!");
						createUser();
					} else {
						//Log.d("PARSE EXCEPTION CODE =", ""+e.getCode());
						if (e.getCode() == ParseException.INVALID_EMAIL_ADDRESS) {
							showAlert("Invalid Email");
							//Log.d("PARSE SIGN UP", "INVALID EMAIL");
						} else if (e.getCode() == ParseException.EMAIL_TAKEN) {
							showAlert("Email address already taken");
							//Log.d("PARSE SIGN UP", "USER ALREADY EXISTS");
						} else if (e.getCode() == ParseException.PASSWORD_MISSING) {
							showAlert("Password Missing");
							//Log.d("PARSE SIGN UP", "PASSWORD MISSING");
						}  else if (e.getCode() == ParseException.USERNAME_TAKEN) {
							showAlert("Username already taken");
							//Log.d("PARSE SIGN UP", "USER ALREADY EXISTS");
						}  else if (e.getCode() == ParseException.USERNAME_MISSING) {
							showAlert("Username missing");
							//Log.d("PARSE SIGN UP", "USERNAME MISSING");
						}
					}
				}
			});
		} catch (Exception e) {
			sendErrorMessage(e);
		}
	}
	
	/**
	 * Set the currentUser to be used for this session
	 */
	private void createUser() {
		try {
			currentUser = ParseUser.getCurrentUser();
			//:TODO set up a real class that extends activity to handle tray messages
			//if we are going to use them
			try {
				PushService.subscribe(this, currentUser.getObjectId(), TutorialActivity.class, R.drawable.iknowulogo);
			} catch(IllegalArgumentException e) {
				//Log.d("PARSE SUBSCRIBE ERROR =", "message ="+e.getMessage());
				//showAlert(e.getMessage());
				sendErrorMessage(e);
			}
			//Log.d("CURRENT USER =", ""+this.currentUser.getEmail());
		} catch (Exception e) {
			sendErrorMessage(e);
		}
	}
	
	/**
	 * Show an alert on the screen via a {@link Toast} message
	 * @param text the text to show in the {@link Toast}
	 */
	public void showAlert(String text) {
		try {
			Toast toast = Toast.makeText(this, "IKnowU Keyboard: "+text, Toast.LENGTH_SHORT);
			toast.setGravity(Gravity.TOP | Gravity.CENTER_HORIZONTAL, 0, 0);
			toast.show();
		} catch (Exception e) {
			sendErrorMessage(e);
		}
	}
	
	/**
	 * Vibrate the phone based on the duration setting set in the application preferences.
	 */
	private void vibrate() {
		try {
			if (!mVibrateOn) {
				return;
			}
			if (mVibrator == null) {
				mVibrator = (Vibrator) getSystemService(VIBRATOR_SERVICE);
			}
			mVibrator.vibrate(mVibrateDuration);
		} catch (Exception e) {
			sendErrorMessage(e);
		}
	}
	
	/**
	 * Play a sound
	 * @param soundKey the key of the sound to be played fomr the pool
	 */
	public void playKeyClick(int soundKey)  {
		if (! mSoundOn)
			return;
		this.soundPool.play(this.soundPoolMap.get(soundKey), this.mSoundIntensity, this.mSoundIntensity, 1, 0, 1f);
	}
	
	/**
	 * Remove any extra punctuation from the text
	 * @param sText the text to trim the punctuation from
	 * @return the newly trimmed text
	 */
	public String trimTrailingPunctuation(String sText) {
		try {
			String onepunctstr = ".,`\"\'?/)}]|:;$%#@! ";
			int textLen = sText.length();
			int c = sText.charAt(textLen-1);
			if (onepunctstr.indexOf(c) >= 0)
				return sText.substring(0, textLen - 1);
			return sText;
		} catch (Exception e) {
			sendErrorMessage(e);
			return "";
		}
	}
	
	/**
	 * Get a key code from a {@link KeyEvent}
	 * @param keyCode the keycode to create the event
	 * @return the keycode from the event
	 */
	 public int getKeyEvent(String keyCode) {
         try {
             return KeyEvent.class.getDeclaredField(keyCode).getInt(null);
         } catch (Exception e) {
        	 sendErrorMessage(e);
             return -1;
         }
     }
	 
	 /**
	  * Get the current device orientation
	  * @return one of Landscape or Portrait
	  */
     public int getDeviceOrientation() {
         return getResources().getConfiguration().orientation;
     }
     
     /**
      * Log an error message. If debugging is off, the error will be sent to the parse cloud. Otherwise it will just log to ADB
      * @param e the Exception that was caught
      */
	@TargetApi(9)
	public static void sendErrorMessage(Exception e) {
    	try {
            if (DEBUG) {
                MessageLogger.logErrorMessageToDownloadDir(e);
            }

			File directory = new File(filesDir);
    		 
			//if in release mode, send the error to the cloud
        	if (!DEBUG) {
	        	//save this error to the cloud for us to see as well
	        	ParseObject errorObject = new ParseObject("Error");
	        	errorObject.put("API", ""+android.os.Build.VERSION.RELEASE);
	        	errorObject.put("Device", ""+android.os.Build.DEVICE);
	        	errorObject.put("Model", ""+android.os.Build.MODEL+", Product = "+android.os.Build.PRODUCT);
	        	errorObject.put("iKnowUVersion", context.getResources().getString(R.string.version_name));
	        	
	        	StringWriter sw = new StringWriter();
	        	e.printStackTrace(new PrintWriter(sw));
	        	String errorMsg = sw.toString();
	        	errorObject.put("Message", errorMsg);
	        	
	        	errorObject.saveInBackground();
        	}
			
    		Calendar cal = Calendar.getInstance();
    		
        	File errorFile = new File(directory, "/dictionary/error.txt");
        	FileWriter out;
        	
        	if (errorFile.exists()) {
        		out = new FileWriter(errorFile, true);
        	} else {
        		out = new FileWriter(errorFile);
        	}
        	
        	PrintWriter pWriter = new PrintWriter(out);
        	
        	pWriter.write("<------------------------------------------------------------------------->"+System.getProperty("line.separator"));
        	pWriter.write("MESSAGE = "+e.getMessage()+System.getProperty("line.separator"));
        	
        	//can only do getDisplayName in api 9 and up
        	if (android.os.Build.VERSION.SDK_INT > android.os.Build.VERSION_CODES.FROYO) {
	        	pWriter.write("DATE = "+cal.getDisplayName(Calendar.MONTH, Calendar.SHORT, Locale.CANADA)+
	        							" "+cal.get(Calendar.DAY_OF_MONTH)
	        							+", "+cal.get(Calendar.HOUR)
	        							+":"+cal.get(Calendar.MINUTE)
	        							+":"+cal.get(Calendar.SECOND)
	        							+System.getProperty("line.separator"));
        	}
        	
        	e.printStackTrace(pWriter);
        	
        	pWriter.write("<------------------------------------------------------------------------->"+System.getProperty("line.separator"));
        	
        	pWriter.flush();
        	pWriter.close();
        	//out.flush();
        	//out.close();
        	pWriter = null;
    	} catch (Exception thisError) {
    		//Log.d("I KNOW U", "Error writing error file, message = "+thisError.getMessage());
    		thisError.printStackTrace();
    	}
    	e.printStackTrace();
    }
	
	/**
	 * Set a flag to only show predictions. No corrections will be presented
	 */
	public void showAllPredictions() {
        log(Log.VERBOSE, "showAllPredictions()", "start function");
		this.showOnlyPreds = true;
		this.updateCandidates();
	}
	
	/**
	 * Update the current word we are tracking to be corrected
	 * This is needed so we can display this word to be re-corrected in the event that they actually meant to
	 * type it.
	 */
	public void updateWordToCorrect() {
		log(Log.WARN, "UPDATEWORDTOCORRECT", "HERE");
		try {
			if (!correctionMade) {
				String word = this.getCurrentWord();
				
				if (word == null || word.length() <= 0) {
					word = this.getLastWord(this.getCurrentInputConnection(), false, true);
				//	this.correctionStart = word.length() + 1;
				}// else {
				this.correctionStart = word.length();
				//}
				
				this.wordToCorrect = word;
				
				log(Log.INFO, "Update word to correct","Correction Start = "+this.correctionStart + ", Word to correct = |"+this.wordToCorrect+"|");
			}
		} catch (Exception e) {
			sendErrorMessage(e);
		}
	}
	
	/**
	 * Use this after a correction has been insterted to erase that correction and then present the next most likely correction
	 * from the auto-correct.
	 */
/*	public void nextCorrection() {
		log(Log.WARN, "NEXTCORRECTION", "HERE");
		//log(Log.INFO, "Next Correction", "Word before = "+this.wordBeforeCorrection);
		try {
			
			if (this.wordBeforeCorrection != null) {
				String last = this.getLastWord(getCurrentInputConnection(), true, false);
				String next = mAutoCorrect.getSecondCorrection(this.correctionStart-1, this.wordBeforeCorrection, last);
				
				//if the word is capitalized, keep it capaitalized
				if (Character.isUpperCase(last.charAt(0))) {
					char[] chars = next.toCharArray();
					chars[0] = Character.toUpperCase(chars[0]);
					next = new String(chars);
				}
				
//				mAutoCorrect.reverseCorrection(this.correctionStart-1, last, next);
				if (next != null) {
					try {
						InputConnection ic = this.getCurrentInputConnection();
						
						this.correctionMade = true;
						
						ic.beginBatchEdit();
						
						ic.deleteSurroundingText(this.correctionStart, 0);
						ic.commitText(next+" ", 1);
						
						ic.endBatchEdit();
						
						this.wordBeforeCorrection = null;
						//log(Log.ERROR, "Next corr", "word correct to null");
						if (this.correctionFeedback)
                            this.ikuScroller.getKeyboardScreen().getKeyboardView().setFlashOn();
					
					} catch(Exception e) {
						sendErrorMessage(e);
					}
				}
				
				this.updatePredictionEngine(this.correctionStart, next, true);
				this.updateCandidates();
			} else {
				this.deleteEntireWordFromEditor();
			}
		} catch (Exception e) {
			sendErrorMessage(e);
		}
	}
	*/
	/**
	 * This is used when a correction needs to be inserted via a user selecting it from the {@link SuggestionsLinearLayout}
	 * @param replacement the correction
	 * @param newWord whether this word should be added to the personal dictionary or not
	 * @param appendSpace whether or not to add a space to the end of the word
	 * @param capsmode the capitalization mode
	 */
	public void manualCorrect(String replacement, boolean newWord, boolean appendSpace, int capsmode) {
		if (newWord) {
			addWord(replacement);
		}
		this.manualCorrect(replacement, appendSpace, capsmode);
	}
	
	/*********************************************
	 * Helper function to make a correction
	 * will use the wordToCorrect and correctionStart variables to determine where it should do the correction
	 * 
	 * @param replacement - the word to insert as a correction
	 ***********************************************/
	public void manualCorrect(String replacement, boolean appendSpace, int capsmode) 
	{
		log(Log.WARN, "MANUALCORRECT", "replacement="+replacement);
	//	this.makeCorrection(this.correctionStart, this.wordToCorrect, replacement, false);
		if (this.autocorrectOn) {
			try {
				this.totalCorrectionsInserted++;
				
				this.totalKeyStrokesSaved += (1 + replacement.length()); //this means, one long press delete plus the number of chars in the replacement word
				
				int reqLen;
				StringBuffer buf;
				
				InputConnection ic = this.getCurrentInputConnection();
				ic.beginBatchEdit();
				this.commitCurrentWord(ic, "manualCorrect");
				
				mAutoCorrect.reverseCorrection(this.correctionStart-1, this.wordToCorrect, replacement);

                log(Log.VERBOSE, "IKnowUKeyboardService","manualCorrect("+Integer.toString(this.correctionStart)+","+this.wordToCorrect+","+replacement+") called");
				
				this.correctionMade = true;
				
				if (capsmode == 0) {
					String before = (String) ic.getTextBeforeCursor(this.correctionStart, 0);
					if (before != null && before.length() > 0 && Character.isUpperCase(before.charAt(0))) {
						capsmode = CorrectionSuggestion.CAPS_FIRST_LETTER;
					}
				}
				
				switch (capsmode) {
					case CorrectionSuggestion.CAPS_ALL:
						replacement = replacement.toUpperCase();
						break;
					case CorrectionSuggestion.CAPS_FIRST_LETTER:

                        //if capsmode has been flagged as first letter, check the
                        //current text to see if we are at a period. If so, do not capitalize the correction.
                        boolean shouldCaps = true;

                        CharSequence two = ic.getTextBeforeCursor(2, 0);
                        if (two != null && two.length() > 1) {
                            if (two.charAt(0) == '.' || two.charAt(0) == '!' || two.charAt(0) == '?') shouldCaps = false;
                        }

                        if (shouldCaps) {
                            char[] chrs = replacement.toCharArray();
                            chrs[0] = Character.toUpperCase(chrs[0]);
                            replacement = new String(chrs);
                        }
						break;
				}
				
				if(this.correctionStart == this.wordToCorrect.length()) {
					//ic.deleteSurroundingText(this.correctionStart, 0);
					//Minkyu:delete one more space since '+' attached.
					ic.deleteSurroundingText(this.correctionStart+1, 0);
					if (appendSpace) {
						ic.commitText(replacement+" ", 1);
					//	mAutoCorrect.addChar(' ');
					} else {
						ic.commitText(replacement, 1);
					}
				} else {
					reqLen = this.correctionStart;
					
					//log(Log.VERBOSE, "IKnowU","getting last "+Integer.toString(reqLen)+" characters");
					
					buf = new StringBuffer(ic.getTextBeforeCursor(reqLen, 0));
					
					//log(Log.VERBOSE, "IKnowU","got \""+buf.toString()+"\"");
					
					ic.deleteSurroundingText(reqLen-currentWord.length()+1, 0);
					
					//this.deleteFromCurrentWord(0, currentWord.length(), false);
					
					buf.replace(0, this.wordToCorrect.length(), replacement);
					
					//log(Log.VERBOSE, "IKnowU","updated buffer to \""+buf+"\"");
					
					ic.commitText(buf+" ", 1);
					//this.addToCurrentWord(buf.toString());
				}
				
				this.removeTrailingText(getCurrentInputConnection(), 0, "Manual correct", 0);
				ic.endBatchEdit();
                if (!appendSpace) {
                    this.setCurrentWord(ic);
                }

				//do this because of the way engine gets reset sometimes, only need to remove capitalization when there is a space
				//or when we run out of predictions, meaning a new root will need to be created
				String last = this.getCurrentWord();
				boolean keepCap = false;
				if ( capsmode == 0 && last.length() > 0 && Character.isUpperCase(last.charAt(0)) ) {
					keepCap = true;
				}
				
				if ( !keepCap ) {
					this.capitalizeFirstLetter = false;
				}
				
				this.wordBeforeCorrection = null;
				//log(Log.ERROR, "Manual corr", "word correct to null");
				this.previousCanBeAdded = false;

                this.updateShiftKeyState(this.getCurrentInputEditorInfo());
				this.updatePredictionEngine(this.correctionStart, replacement, appendSpace);
                this.updatePredEngineHistoryLatin(false);

			} catch(Exception e) {
				sendErrorMessage(e);
			}
		}
		this.updateCandidates();
	}
	//////////////////////////////////////////////////////////////////////////////////////////
	//public void makeCorrection(int si,String previous,String replacement) {
	//	this.makeCorrection(si, previous, replacement, true);
	//}
	
	/**************************************
	 * Helper function to the makeCorrection called by the auto correct engine
	 * @param si - start index of correction
	 * @param previous - the word to be corrected
	 * @param replacement - the word to be inserted as a correction
	 * @param showPrevious - whether or not to set the wordBeforeCorrection variable
	 ***************************************/
	public void makeCorrection(int si, boolean showPrevious) 
	{ 
		log(Log.VERBOSE, "makeCorrection", "this.wordToCorrect = "+this.wordToCorrect+", replacement = "+this.lastCorrection);
		if (this.autocorrectOn && this.insertCorrection) {
			try {
				this.totalCorrectionsInserted++;					
				if (this.getCurrentWord().length() <= 0) {
					si++;
				}
				
				this.totalKeyStrokesSaved += (1 + this.wordToCorrect.length()); //this means, one long press delete plus the number of chars in the replacement word
				
				this.correctionStart = si + (this.lastCorrection.length() - this.wordToCorrect.length());
				log(Log.WARN, "Correction start = "+this.correctionStart, "Word to correct = "+this.wordToCorrect);
				//log(Log.VERBOSE, "IKnowUKeyboardService","makeCorrection("+Integer.toString(si)+","+previous+","+replacement+","+showPrevious+") called");

				InputConnection ic = this.getCurrentInputConnection();				
				ic.beginBatchEdit();
				this.commitCurrentWord(ic, "makeCorrection");

				// take care of first char upper case
				if(Character.isUpperCase(this.wordToCorrect.charAt(0)) && !wordBeforeCorrection.equals("i"))
				{
					StringBuffer buf = new StringBuffer(this.lastCorrection);
					char f = Character.toUpperCase(this.lastCorrection.charAt(0));
					buf.setCharAt(0, f);
					this.lastCorrection = buf.toString();
				}

				
				if(si == this.wordToCorrect.length()) {
					ic.deleteSurroundingText(this.wordToCorrect.length(), 0);
					ic.commitText(this.lastCorrection, 1);
				} else {
					CharSequence cs = ic.getTextBeforeCursor(si, 0);
					if (cs == null) 
						cs = "";
					StringBuffer buf = new StringBuffer(cs);
					
					//log(Log.VERBOSE, "IKnowU","got \""+buf.toString()+"\"");
					ic.deleteSurroundingText(si-this.currentWord.length(), 0);
					//this.deleteFromCurrentWord(0, currentWord.length(), false);
					buf.replace(0, this.wordToCorrect.length(), this.lastCorrection);
					
					ic.commitText(buf, 1);
					//log(Log.VERBOSE, "IKnowU","updated buffer to \""+buf+"\" result = "+done);
					//this.addToCurrentWord(buf.toString());
				}
				
				ic.endBatchEdit();
				this.correctionMade = true;
				
				if (showPrevious) 
				{
					this.wordBeforeCorrection = this.wordToCorrect;
					//String wordinfo = mPredictionEngine.setWordInfo();
					this.previousCanBeAdded = mPredictionEngine.canWordBeAdded();
					if(previousCanBeAdded == false)
					{
						this.wordBeforeCorrection = null;
					}
					log(Log.VERBOSE, "can word be added = "+this.wordToCorrect, "New word = "+this.previousCanBeAdded);
				}

				log(Log.VERBOSE, "Corr Made", "sending replacement to predictions = "+this.lastCorrectionTaken+"|");

				this.updatePredictionEngine(si, this.lastCorrection, true);

                char[] puncs = this.getLastPunctuation(ic);
                if (puncs[0] != 'e' && !this.wordToCorrect.equals("i")) 
                	mPredictionEngine.advanceLetter(puncs[0], false, false);
				//this.updateCandidates();
				
				if (this.correctionFeedback) 
					this.kbContainerView.getKbLinearLayout().getKeyboardView().setFlashOn();
				this.audioAndClickFeedback(2);
				
			} catch(Exception e) {
				log(Log.ERROR, "makeCorrection", "Exception:"+e.getMessage());
				sendErrorMessage(e);
			}
		}
	}

	public void bufferLow(int n) {
		log(Log.WARN, "BUFFERLOW", "HERE");
		//log(Log.VERBOSE, "Auto-correct buffer low", "characters left = "+n);
		this.updateAutoCorrectOnCursorPosition();
	}
	
	/****************
	 * Check the cloud to see if the user's current installed dictionaries are out of date.
	 ******************/
	private void checkForDictUpdates() {
		try {
			SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(this);
			
			long last = sp.getLong(PREF_LAST_CHECK_FOR_UPDATES, 0);
			
			//checks every twelve hours
			if (System.currentTimeMillis() - last > UPDATE_INTERVAL) {
				DownloadXML dictInfo = new DownloadXML();
				dictInfo.execute(DownloadActivity.DICTS_INFO_FILE_DL);
				
				Editor edit = sp.edit();
				edit.putLong(PREF_LAST_CHECK_FOR_UPDATES, System.currentTimeMillis());
				edit.commit();
			}
		} catch (Exception e) {
			sendErrorMessage(e);
		}
	}
	
	/**
	 * {@link AsyncTask} to get the descriptive xml file for the dictionary information
	 * @author Justin
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
	            String dir = filesDir + "/dictionary/" + DownloadActivity.DICTS_INFO_FILE;
	            log(Log.DEBUG, "output dir = ", dir);
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
                return "success";
	        } catch (Exception e) {
	        	sendErrorMessage(e);
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
            if (result != null) {
	    	    analyzeForDictUpdates();
            }
	    }
	}
	
	/**
	 * Analyze the retrieved XML file from the cloud and compare it against the currently installed user dictionaries
	 */
	private void analyzeForDictUpdates() {
		try {
			//get a list of the currently installed dictionaries
			ArrayList<UserDictionary> userDicts = new ArrayList<UserDictionary>();
			for (int i = 1; i < 15; i++) {
				boolean exists = mPredictionEngine.getDictInfo(i);
				
				if (exists) {
					UserDictionary dict = new UserDictionary();
					dict.langIdx = mPredictionEngine.getDictIndex();
					dict.listIdx = mPredictionEngine.getDictPriority();
					dict.dictname = mPredictionEngine.getDictName();
					dict.codeVersion = mPredictionEngine.getDictCodeVersion();
					dict.dataVersion = mPredictionEngine.getDictDataVersion();
					IKnowUKeyboardService.log(Log.VERBOSE, "Got user dict", "langidx = "+dict.langIdx+", listidx = "+dict.listIdx+
							", dictname = "+dict.dictname+", codeVersion = "+dict.codeVersion+", dataVersion = "+dict.dataVersion);
					userDicts.add(dict);
				}
			}
			
			XmlPullParserFactory factory = XmlPullParserFactory.newInstance();
			XmlPullParser xpp = factory.newPullParser();
			
			File file = new File(filesDir + "/dictionary/" + DownloadActivity.DICTS_INFO_FILE);
			FileInputStream in = new FileInputStream(file);
			
			xpp.setInput(in, null);
			
			int eventType = xpp.getEventType();
			
			while (eventType != XmlPullParser.END_DOCUMENT) {
				switch (eventType) {
				case XmlPullParser.START_TAG:
					if (xpp.getName().equals(DownloadActivity.TAG_DICTIONARY)) {
						
						String name = xpp.getAttributeValue(null, "name");
						int index = Integer.parseInt(xpp.getAttributeValue(null, "index"));
						int version = Integer.parseInt(xpp.getAttributeValue(null, "versionCode"));
						int layout = Integer.parseInt(xpp.getAttributeValue(null, "layout"));
						
						UserDictionary userDict = null;
						for (int i = 0; i < userDicts.size(); i++) {
							UserDictionary dict = userDicts.get(i);
							
							if (dict.langIdx == index) userDict = dict;
						}
						
						//if we have this dictionary, set up some variables
						if (userDict != null && userDict.dataVersion < version) {
							//update available, send notification to user
							this.sendNotification(NOTIFICATION_ID, getResources().getString(R.string.dictionary_update_title), getResources().getString(R.string.dictionary_update_message));
						}
						
						IKnowUKeyboardService.log(Log.VERBOSE, "Dictionary = "+name, "id="+index+", version="+version+", layout="+layout);
					}
					break;
				}
				eventType = xpp.next();
			}
			
			in.close();
			file.delete();
		} catch (Exception e) {
			sendErrorMessage(e);
		}
	}
	
	/**
	 * Send a notification to the sytem bar telling the user that they have an out of date dictionary
	 * @param id the id for the notification
	 * @param title the title of the notification
	 * @param message the messge of the notification
	 */
	private void sendNotification(int id, String title, String message) {
		NotificationManager mNotificationManager = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);

		PendingIntent pin = PendingIntent.getActivity(getApplicationContext(), 0, new Intent(this, DownloadActivity.class), 0);
		Notification notif = new Notification(R.drawable.iknowulogo, title, System.currentTimeMillis());
	       
	    notif.setLatestEventInfo(this, title, message, pin);

		mNotificationManager.notify(id, notif);
	}
	
	/**
	 * Start a timer for tracking method duration
	 */
	public void startTimer() {
		this.timerStartTime = System.currentTimeMillis();
	}
	
	/**
	 * Stop the timer and log the difference fomr the startTime
	 * @param function
	 */
	public void stopTimer(String function) {
		Long diff = System.currentTimeMillis() - this.timerStartTime;
		double seconds = diff / 1000.0;
		log(Log.INFO, function + " TIMER", "= "+seconds + "s, or "+diff+"ms");
		this.startTime = 0;
	}
	
	/**
	 * Check for the expiration set in the shared preferences,
	 * To avoid a network call if necessary
	 */
	private void getexpirationStatus() {
    	SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(this);
		this.mTrialExpired = sp.getBoolean(PREF_IS_EXPIRED, false);
    }
    
    private boolean isExpirationTimerExpired() {
		SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(this);
		
		long last = sp.getLong(PREF_LAST_CHECK_EXPIRARY, 0);
		
		//checks every twelve hours
		if (System.currentTimeMillis() - last > UPDATE_INTERVAL) {
			return true;
		}
		
		return false;
	}
    
    /**
     * Check for device expiration, if not found then register the device
     */
    private void isExpired() {
    	try {
			new Thread() {
	            public void run() {
	                try {
	                	String androidId = Settings.Secure.getString(getContentResolver(),Settings.Secure.ANDROID_ID);
	                	log(Log.VERBOSE, TAG, "androidId = "+androidId);
	            		if (androidId != null) {
	        	    		ParseQuery query = new ParseQuery("Trial");
	        				query.whereEqualTo("device", androidId);
	        				
	        				List<ParseObject> list = query.find();
	        				expiraryCheck(list);
	            		} else {
	            			log(Log.VERBOSE, TAG, "Android ID not present, invalidating Trial!!");
	            			//mTrialExpired = true;
	            			//showTrialExpiredDialog();
	            		}
	                } catch (Exception e) {
	                	log(Log.VERBOSE, TAG, "Random Exception Caught, invalidating Trial!!");
	                	//mTrialExpired = true;
	                	//showTrialExpiredDialog();
	                }
	            }
	        }.start();
    	} catch (Exception e) {
			sendErrorMessage(e);
			log(Log.VERBOSE, TAG, "Random Exception Caught, invalidating Trial!!");
			//mTrialExpired = true;
			//showTrialExpiredDialog();
		}
    }
    
    /**
     * Check the device in the parse table for to see if it is expired.
     * 
     * If the device can't be found, register it's trial start period to the current time in milliseconds
     * @param list the retrieved list of information form the cloud
     */
    private void expiraryCheck(List<ParseObject> list) {
    	SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(this);
		
		//if the device is found check if it's trial is expired
		if (list.size() > 0) {
			ParseObject po = list.get(0);
			long installDate = po.getLong("installDate");
			
			Editor edit = sp.edit();
			edit.putLong(PREF_LAST_CHECK_EXPIRARY, System.currentTimeMillis());
			
			//check for expirary
			if (System.currentTimeMillis() - installDate >= TRIAL_DURATION) {
				edit.putBoolean(PREF_IS_EXPIRED, true);
				edit.commit();
				log(Log.VERBOSE, TAG, "Expiration time exceeded, Trial has Expired!!");
				this.mTrialExpired = true;
				showTrialExpiredDialog();
			} else {
				log(Log.VERBOSE, TAG, "Expiration not exceeded, Trial still valid");
				this.mTrialExpired = false;
			}
			edit.commit();
		//not registered yet so register now
		} else {
			Editor edit = sp.edit();
			edit.putLong(PREF_LAST_CHECK_EXPIRARY, System.currentTimeMillis());
			edit.commit();
			
			new Thread() {
	            public void run() {
	                try {
	                	String androidId = Settings.Secure.getString(getContentResolver(),Settings.Secure.ANDROID_ID);
	            		log(Log.VERBOSE, TAG, "androidId = "+androidId);
	                	
	                	ParseObject expirary = new ParseObject("Trial");
	        			expirary.put("device", androidId);
	        			expirary.put("installDate", System.currentTimeMillis());
	        			expirary.save();
	                } catch (Exception e) {
	                	log(Log.VERBOSE, TAG, "Random Exception Caught, invalidating Trial!!");
	                }
	            }
	        }.start();
	        log(Log.VERBOSE, TAG, "New Subcriber, Trial still valid");
			this.mTrialExpired = false;
		}
    }
    
    /**
     * If the trial is expired, show a dialog indicating so.
     * 
     * The "OK" button will take the user to the Play Store to allow them to purchase the full version
     */
    public void showTrialExpiredDialog() {
		try {
			AlertDialog.Builder builder = new AlertDialog.Builder(this);
			builder.setMessage(R.string.expirery_text);
			builder.setCancelable(true);
			builder.setIcon(R.drawable.iknowulogo);
			builder.setTitle(this.getResources().getString(R.string.ime_name));
			
			builder.setPositiveButton(R.string.label_ok_key, new OnClickListener() {
				@Override
				public void onClick(DialogInterface dialog, int which) {
					gotoMarket("com.iknowu.paid");
				}
			});
			
			builder.setNegativeButton(this.getResources().getString(R.string.label_no_thanks), null);
			
			AlertDialog currentDialog = builder.create();
			Window window = currentDialog.getWindow();
			WindowManager.LayoutParams ra = window.getAttributes();
			
			IBinder token = this.kbContainerView.getWindowToken();
			
			log(Log.INFO, TAG, "Starting Trial Dialog, token = "+token);
			
			if ( token != null ) {
				ra.token = token;
				//Log.d("WINDOW TOKEN = ", ""+ra.token);
				ra.type = WindowManager.LayoutParams.TYPE_APPLICATION_ATTACHED_DIALOG;
				window.setAttributes(ra);
				window.addFlags(WindowManager.LayoutParams.FLAG_ALT_FOCUSABLE_IM);
				currentDialog.show();
			}
		} catch (Exception e) {
			IKnowUKeyboardService.sendErrorMessage(e);
		}
	}
}