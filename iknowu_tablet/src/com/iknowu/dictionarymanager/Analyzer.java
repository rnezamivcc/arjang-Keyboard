package com.iknowu.dictionarymanager;

import android.accounts.Account;
import android.accounts.AccountManager;
import android.accounts.AccountManagerCallback;
import android.accounts.AccountManagerFuture;
import android.accounts.AuthenticatorException;
import android.accounts.OperationCanceledException;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.Signature;
import android.database.Cursor;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Environment;
import android.os.StrictMode;
import android.util.Base64;
import android.util.Log;
import android.view.View;
import android.view.Window;
import android.webkit.WebView;
import android.widget.ScrollView;
import android.widget.Toast;

import com.facebook.HttpMethod;
import com.facebook.Request;
import com.facebook.Response;
import com.facebook.Session;
import com.facebook.SessionState;
import com.facebook.widget.LoginButton;
import com.iknowu.IKnowUKeyboardService;
import com.iknowu.PredictionEngine;
import com.iknowu.R;
import com.parse.LogInCallback;
import com.parse.ParseException;
import com.parse.ParseTwitterUtils;
import com.parse.ParseUser;
import com.wordlogic.lib.AutoCorrect;

import org.apache.http.client.HttpClient;
import org.apache.http.client.ResponseHandler;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.impl.client.BasicResponseHandler;
import org.apache.http.impl.client.DefaultHttpClient;
import org.json.JSONArray;
import org.json.JSONObject;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.Arrays;
import java.util.Properties;

import javax.mail.BodyPart;
import javax.mail.Folder;
import javax.mail.Message;
import javax.mail.Multipart;
import javax.mail.Store;

//import javax.mail.Session;
//import javax.mail.*;
//import javax.mail.event.*;
//import javax.mail.internet.*;

public class Analyzer extends Activity implements View.OnClickListener {

    private static final String TAG = "Analyzer";

	private static final String WL_DIR = "wordlogic/";

    private static final String TWITTER_CONSUMER_KEY = "dzu0eYnkzrRY7GLzjzCNg";
    private static final String TWITTER_CONSUMER_SECRET = "rWhB7opBmhnAWB46fhOkyGBfZXRbHDD5sORs4YFa8";

    private Context ctx;

    private String wordSeparators;
	private int total;

    private PredictionEngine predEngine;
    private AutoCorrect corrEngine;

    private ScrollView scroller;
    private WebView twitterWebView;

    private File theFile;
	
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

        this.ctx = (Context) this;

        if (android.os.Build.VERSION.SDK_INT > 9) {
            StrictMode.ThreadPolicy policy = new StrictMode.ThreadPolicy.Builder().permitAll().build();
            StrictMode.setThreadPolicy(policy);
        }

        this.predEngine = IKnowUKeyboardService.getPredictionEngine();
        this.corrEngine = IKnowUKeyboardService.getAutoCorrectEngine();

        this.createTextFile();

        this.requestWindowFeature(Window.FEATURE_NO_TITLE);
        //this.checkKeyHash();
        setContentView(R.layout.analyzer_layout);

        //ParseFacebookUtils.initialize(this.getResources().getString(R.string.facebook_id));

        ParseTwitterUtils.initialize(TWITTER_CONSUMER_KEY, TWITTER_CONSUMER_SECRET);


        Session ses = Session.getActiveSession();
        if (ses != null) {
            Session.NewPermissionsRequest reauthRequest = new Session.NewPermissionsRequest(this, Arrays.asList("user_status", "read_mailbox"));
            ses.requestNewReadPermissions(reauthRequest);
        }

	}

    private void checkKeyHash() {
        // Add code to print out the key hash
        try {
            PackageInfo info = getPackageManager().getPackageInfo(
                    "com.iknowu",
                    PackageManager.GET_SIGNATURES);
            for (Signature signature : info.signatures) {
                MessageDigest md = MessageDigest.getInstance("SHA");
                md.update(signature.toByteArray());
                IKnowUKeyboardService.log(Log.DEBUG, "KeyHash:", Base64.encodeToString(md.digest(), Base64.DEFAULT));
            }
        } catch (PackageManager.NameNotFoundException e) {

        } catch (NoSuchAlgorithmException e) {

        }
    }

    private void createTextFile() {
        try {
            File directory = Environment.getExternalStorageDirectory();
            //File directory = this.getFilesDir();
            File wldir = new File(directory, WL_DIR);
            wldir.mkdirs();

            this.theFile = new File(wldir, "learnedwords.txt");

            if (!this.theFile.exists()) this.theFile.createNewFile();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
	
	@Override
	public void onResume() {
		super.onResume();
		total = 0;
		wordSeparators = getResources().getString(R.string.word_separators);

        this.findViewById(R.id.edit_button).setOnClickListener(this);


        this.findViewById(R.id.sms_button).setOnClickListener(this);
        this.findViewById(R.id.gmail_button).setOnClickListener(this);
        LoginButton facebookButton = (LoginButton) this.findViewById(R.id.facebook_button);
        facebookButton.setOnClickListener(this);
        facebookButton.setReadPermissions(Arrays.asList("user_status", "read_mailbox"));
        this.findViewById(R.id.twitter_button).setOnClickListener(this);

		//this.getGmailAccount();
		//this.readSMS();
	}
	
	@Override
	public void onPause() {
		super.onPause();
	}

    private void launchDictionaryManager() {
        //handleClose();
        Intent intent = new Intent();
        intent.setClass(this, DictionaryManager.class);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        this.startActivity(intent);
    }

    public void getFaceBookInfo() {
        // start Facebook Login

        Session.openActiveSession(this, true, new Session.StatusCallback() {
            // callback when session changes state
            @Override
            public void call(Session session, SessionState state, Exception exception) {
                IKnowUKeyboardService.log(Log.VERBOSE, TAG, "session.call(), isOpen = "+session.isOpened()+", state = "+state.isOpened());
                if (session.isOpened()) {
                    sendFacebookRequest(session);
                }
            }
        });
    }

    private void sendFacebookRequest(Session session) {

        String statusQuery = "SELECT message FROM status WHERE uid = me() LIMIT 300";
        Bundle params = new Bundle();
        params.putString("q", statusQuery);
        Request request = new Request(session,
                "/fql",
                params,
                HttpMethod.GET,
                new Request.Callback(){
                    public void onCompleted(Response response) {
                        analyzeFacebookResponse(response.getGraphObject().getInnerJSONObject(), "message");
                    }
                });
        Request.executeBatchAsync(request);

        String messageQuery = "SELECT body FROM message WHERE thread_id IN (SELECT thread_id FROM thread WHERE folder_id = 1) AND author_id = me() LIMIT 300"; //select messages from a users outbox
        Bundle params1 = new Bundle();
        params1.putString("q", messageQuery);
        Request request1 = new Request(session,
                "/fql",
                params1,
                HttpMethod.GET,
                new Request.Callback(){
                    public void onCompleted(Response response) {
                        IKnowUKeyboardService.log(Log.VERBOSE, TAG, "response = "+response);
                        analyzeFacebookResponse(response.getGraphObject().getInnerJSONObject(), "body");
                    }
                });
        Request.executeBatchAsync(request1);
    }

    public void analyzeFacebookResponse(JSONObject response, String dataToGrab) {
        try {
            JSONArray data = response.getJSONArray("data");

            FileWriter out;
            out = new FileWriter(this.theFile);

            PrintWriter pWriter = new PrintWriter(out);

            for ( int i=0; i < data.length(); i++ ) {

                JSONObject message = data.getJSONObject(i);

                String value = message.getString(dataToGrab);

                IKnowUKeyboardService.log(Log.DEBUG, TAG, "value["+i+"]= " + value);

                this.writeStringToFile(pWriter, value);
            }

            pWriter.flush();
            pWriter.close();
            //out.flush();
            //out.close();
            pWriter = null;

            this.predEngine.learnFromFile(this.theFile.getAbsolutePath());
        } catch (Exception e) {
            IKnowUKeyboardService.sendErrorMessage(e);
        }
    }

    public void writeStringToFile(PrintWriter writer, String words) {

        if (words.length() > 0) {
            if (Character.isLetter(words.charAt(words.length()-1)) ) words += '.';
            writer.write(words+System.getProperty("line.separator")+System.getProperty("line.separator"));
        }
        //String[] wordList = words.split("\\s+");
    }

    private String stripExtras(String word) {
        word = word.replaceAll("[^A-Za-z0-9 ']", "");
        return word;
    }

	/**
	 * Function to read the sms messages on a phone and learn words from them
	 */
	private void readSMS() {
		try {
			Uri uri = Uri.parse("content://sms/sent");
			Cursor c = getContentResolver().query(uri, null, null ,null,null);
			startManagingCursor(c);

        	FileWriter out;
        	out = new FileWriter(this.theFile);
        	 
        	PrintWriter pWriter = new PrintWriter(out);

			if (c.moveToFirst()) {
				for (int i = 0; i < c.getCount(); i++) {
                    this.writeStringToFile(pWriter, c.getString(c.getColumnIndexOrThrow("body")));
					c.moveToNext();
				}
			}
			c.close();

        	pWriter.flush();
        	pWriter.close();
        	pWriter = null;

        	IKnowUKeyboardService.log(Log.VERBOSE, "learnFromSMS()", "done! file exists at... "+this.theFile.getAbsolutePath());

            /*
            new Thread() {
                public void run() {
                    if (theFile.exists()) {
                        IKnowUKeyboardService.log(Log.VERBOSE, "learnFromSMS()", "sending path to engine...");
                        predEngine.learnFromFile(theFile.getAbsolutePath());
                        IKnowUKeyboardService.log(Log.VERBOSE, "learnFromSMS()", "engine done! File learned");
                    }
                }
            }.start();
            */

    	} catch (Exception thisError) {
    		//Log.d("I KNOW U", "Error writing error file, message = "+thisError.getMessage());
    		thisError.printStackTrace();
    	}
	}

    private void authorizeTwitter() {
        ParseTwitterUtils.logIn(this, new LogInCallback() {
            @Override
            public void done(ParseUser user, ParseException err) {
                if (user == null) {
                    IKnowUKeyboardService.log(Log.ERROR, TAG, "Uh oh. The user cancelled the Twitter login.");
                    Toast.makeText(ctx, "Ooops, there was an error, please try again.", Toast.LENGTH_SHORT).show();
                }
                /*else if (user.isNew()) {
                    Log.d(TAG, "User signed up and logged in through Twitter!");
                } */
                else {
                    IKnowUKeyboardService.log(Log.INFO, TAG, "User logged in through Twitter!");
                    getTwitterTweets(user);
                }
            }
        });
    }

    private void getTwitterTweets(ParseUser user) {
        try {
            /*
             * Get the users information
             */
            HttpClient client = new DefaultHttpClient();
            HttpGet verifyGet = new HttpGet("https://api.twitter.com/1.1/account/verify_credentials.json");
            ParseTwitterUtils.getTwitter().signRequest(verifyGet);
            //HttpResponse response = client.execute(verifyGet);
            ResponseHandler<String> responseHandler = new BasicResponseHandler();

            String responseBody = client.execute(verifyGet, responseHandler);
            IKnowUKeyboardService.log(Log.VERBOSE, TAG, "twitter response = "+responseBody);
            JSONObject jsonObject = new JSONObject(responseBody);

            /*
             * Get the users tweets
             */
            String screenName = jsonObject.getString("screen_name");
            client = new DefaultHttpClient();
            verifyGet = new HttpGet("https://api.twitter.com/1.1/statuses/user_timeline.json?screen_name="+screenName+"&count=200&trim_user=true&exclude_replies=true&include_rts=false");
            ParseTwitterUtils.getTwitter().signRequest(verifyGet);
            //HttpResponse response = client.execute(verifyGet);
            responseHandler = new BasicResponseHandler();

            responseBody = client.execute(verifyGet, responseHandler);

            JSONArray jsonArray = new JSONArray(responseBody);
            IKnowUKeyboardService.log(Log.VERBOSE, TAG, "jsonArray = "+jsonArray);

            FileWriter out;
            out = new FileWriter(this.theFile);

            PrintWriter pWriter = new PrintWriter(out);

            for (int i = 0; i < jsonArray.length(); i++) {
                jsonObject = (JSONObject) jsonArray.get(i);
                IKnowUKeyboardService.log(Log.VERBOSE, TAG, "message["+i+"] = "+jsonObject.getString("text"));
                this.writeStringToFile(pWriter, jsonObject.getString("text"));
            }

            pWriter.flush();
            pWriter.close();
            //out.flush();
            //out.close();
            pWriter = null;

            this.predEngine.learnFromFile(this.theFile.getAbsolutePath());
        } catch (Exception e) {
            IKnowUKeyboardService.log(Log.ERROR, TAG, "Error getting twitter request");
            e.printStackTrace();
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        Session.getActiveSession().onActivityResult(this, requestCode, resultCode, data);
    }
	
	private void getWords(String text, PrintWriter pWriter) {
		String currentWord = "";
		String lastWord = "";
		char currentChar;
		for (int i=0; i < text.length(); i++) {
			currentChar = text.charAt(i);
			if ( this.wordSeparators.contains(String.valueOf(currentChar)) || i == text.length()-1) {
				if (currentWord.length() > 0) {
					if (lastWord.length() > 0) {
						pWriter.write(lastWord+" "+currentWord+System.getProperty("line.separator"));
						this.total++;
					}
					lastWord = new String(currentWord);
				}
				
				currentWord = "";
			} else {
				currentWord += currentChar;
			}
		}
	}
	
	private void getGmailAccount() {
		// Get the account list, and pick the first one
		final String ACCOUNT_TYPE_GOOGLE = "com.google";
		final String[] FEATURES_MAIL = {"service_mail"};
		
		AccountManager.get(this).getAccountsByTypeAndFeatures(ACCOUNT_TYPE_GOOGLE, FEATURES_MAIL,
		        new AccountManagerCallback<Account[]>() {
		        	
					@Override
		            public void run(AccountManagerFuture<Account[]> future) {
		                Account[] accounts = null;
		                try {
		                    accounts = future.getResult();
		                    if (accounts != null && accounts.length > 0) {
		                        String selectedAccount = accounts[0].name;
		                        readGmail(selectedAccount);
		                    }
		                } catch (OperationCanceledException oce) {
		                    // TODO: handle exception
		                } catch (IOException ioe) {
		                    // TODO: handle exception
		                } catch (AuthenticatorException ae) {
		                    // TODO: handle exception
		                }
		            }
		        }, null /* handler */);
	}
	
	/**
	 * not working at this point in time, it only reads the labels that gmail provides and 
	 * the limited information that they contain
	 * @param account
	 */
	private void readGmail(String account) {
		GetGmailTask task = new GetGmailTask();
		task.execute("");
	}
	
	private void printEmail(Store store) {
		
	}

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.edit_button:
                this.launchDictionaryManager();
                break;
            case R.id.sms_button:
                this.readSMS();
                break;
            case R.id.gmail_button:
                break;
            case R.id.facebook_button:
                this.getFaceBookInfo();
                break;
            case R.id.twitter_button:
                //this.getTwitterTweets("@jusdes19");
                this.authorizeTwitter();
                break;
        }
    }

    class GetGmailTask extends AsyncTask<String, Boolean, String> {
		
		private Store store;

	    protected String doInBackground(String... urls) {
	    	Properties props = System.getProperties();
			props.setProperty("mail.store.protocol", "imaps");
			try {
				//Session session = Session.getDefaultInstance(props, null);
				//store = session.getStore("imaps");
				store.connect("imap.gmail.com", "", "");
				
				IKnowUKeyboardService.log(Log.INFO, "Store =", store.toString());
				Folder inbox = store.getFolder("[Gmail]/Sent Mail");
				inbox.open(Folder.READ_ONLY);
				
				Message messages[] = inbox.getMessages();
				
				File directory = Environment.getExternalStorageDirectory();
				
	        	File smsFile = new File(directory, WL_DIR+"email.txt");
	        	FileWriter out;
	        	
	        	out = new FileWriter(smsFile);
	        	 
	        	PrintWriter pWriter = new PrintWriter(out);
	        	
				for (Message msg : messages) {
					//Log.d("Content Type", " = "+msg.getContentType());
					//Log.v("Email Message = ", ""+ (String)message.getContent());
					String contentType = msg.getContentType().toLowerCase();
					if(contentType.contains("text/plain") || contentType.contains("text/html")){
						//DataHandler dh = msg.getDataHandler();
						String content = (String)msg.getContent();
			        	pWriter.write(content);
			        	pWriter.write("<------------------------------------------------------------------------->"+System.getProperty("line.separator"));
						//Log.v("Content = ", ""+content);
					} else if (contentType.contains("multipart")){
						
						Multipart mp = (Multipart)msg.getContent();
						for (int i = 0; i < mp.getCount(); i++) {
							
							BodyPart bp = mp.getBodyPart(i);
							String ctype = bp.getContentType().toLowerCase();
							if (ctype.contains("text/plain")) {
								pWriter.write("" + bp.getContent());
							}
							//Log.w("Body part", "= "+bp.getContent());
						}
						pWriter.write("<------------------------------------------------------------------------->"+System.getProperty("line.separator"));
					}
				}
				pWriter.flush();
	        	pWriter.close();
	        	//out.flush();
	        	//out.close();
	        	pWriter = null;
	        	IKnowUKeyboardService.log(Log.VERBOSE, "Analyzing email", "done");
			} catch (Exception e) {
				e.printStackTrace();
			}
			return null;
	    }

	    protected void onPostExecute(String result) {
	        
	    }
	}
}
