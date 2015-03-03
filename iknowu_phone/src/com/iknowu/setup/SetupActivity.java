package com.iknowu.setup;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.preference.PreferenceManager;
import android.provider.Settings;
import android.util.DisplayMetrics;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.view.inputmethod.InputMethodInfo;
import android.view.inputmethod.InputMethodManager;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.Toast;

import com.iknowu.IKnowUKeyboardService;
import com.iknowu.R;
import com.iknowu.preferences.IKnowUSettings;
import com.parse.LogInCallback;
import com.parse.Parse;
import com.parse.ParseException;
import com.parse.ParseUser;
import com.parse.SignUpCallback;

import java.util.List;

/**
 * Created by Justin on 22/07/13.
 *
 */
public class SetupActivity extends Activity {
    private static final String PACKAGE_NAME = "com.iknowu.IKnowUKeyboardService";
    private static final String INPUT_NAME = "/com.iknowu.IKnowUKeyboardService";
    private static final String PREF_SIGNUP_FIRST = "firstSignUpValue";
    private static final String PREF_SIGNUP_SECOND = "secondSignUpValue";
    private static final String PREF_HANDEDNESS = "handedness_right";
    private static final String PREF_HANDEDNESS_COMPLETE = "handedness_complete";
    private static final String PREF_SETUP_COMPLETE = "setup_complete";

    private static final String PHONE_TRIAL_PACKAGE = "com.iknowu";
    private static final String TABLET_TRIAL_PACKAGE = "com.iknowu.tabletfree";

    private static final int RESULT_KB_SETTINGS = 10;
    private static final int MSG_CHECK_PICKED = 1;
    private static final int MSG_TASKS_COMPLETED = 2;

    private static final int ENABLE_TASK = 1;
    private static final int SET_DEFAULT_TASK = 2;
    private static final int CLOUD_SYNC_TASK = 3;
    private static final int HANDEDNESS_TASK = 4;
    private static final int TASKS_COMPLETE = 5;
    private static final int NUM_HELP_ITEMS = 6;
    private static final int UNINSTALL_TASK = 7;

    //private SetupRelativeLayout chooseLanguage;
    private SetupRelativeLayout uninstallLayout;
    private SetupRelativeLayout enableIknowu;
    private SetupRelativeLayout setAsDefault;
    private SetupRelativeLayout cloudSignup;
    //private SetupRelativeLayout learnWords;
    private SetupRelativeLayout handedness;

    private SetupRelativeLayout helpVids;
    private SetupRelativeLayout gotoSettings;
    private SetupRelativeLayout finishSetup;

    private AlertDialog signupDialog;
    private EditText emailText;
    private EditText passText;

    /*
    private int[] groupTextIds = {R.string.tutorial_autocorrect_title, R.string.tutorial_gesturing_title, R.string.tutorial_prediction_title,
            R.string.tutorial_popup_title, R.string.tutorial_highlight_title};
    private int[] childTextIds = {R.string.tutorial_autocorrect, R.string.tutorial_swipe_erase, R.string.tutorial_gesturing, R.string.tutorial_prediction_bar,
            R.string.tutorial_popup, R.string.tutorial_highlight};
    private int[] childImageIds = {R.drawable.tutorial_autocorrect, R.drawable.tutorial_swipe_erase, R.drawable.tutorial_gesture, R.drawable.tutorial_candidate,
            R.drawable.tutorial_popup, R.drawable.tutorial_highlight};
            */

    private int[] numChildren = {2, 1, 1, 1, 1};

    private AlertDialog handedDialog;

    private int currentTask;

    private boolean tasksCompleted;
    private String currentPackageUninstall;

    private StartupViewPager svp;

    Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            //Log.d("HANDLER MSG", ""+msg);
            switch (msg.what) {
                case MSG_CHECK_PICKED:
                    checkSetAsDefault(true);
                    break;
                case MSG_TASKS_COMPLETED:
                    close();
                    break;
            }
        }
    };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        this.requestWindowFeature(Window.FEATURE_NO_TITLE);

        int size = this.calculateScreenSize();
        if (size < 6) setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
    }

    /**
     * Used to calculate the size of the screen in inches
     * @return the size of the screen in inches as an integer
     */
    private int calculateScreenSize() {
        try {
            DisplayMetrics dm = new DisplayMetrics();
            WindowManager window = (WindowManager) getSystemService(Context.WINDOW_SERVICE);
            window.getDefaultDisplay().getMetrics(dm);
            double x = Math.pow(dm.widthPixels/dm.xdpi,2);
            double y = Math.pow(dm.heightPixels/dm.ydpi,2);
            double screenInches = Math.sqrt(x+y);
            //Log.d("SCREEN INCHES =","" + screenInches);
            return (int) screenInches;
        } catch (Exception e) {
            IKnowUKeyboardService.sendErrorMessage(e);
            return 4;
        }
    }

    @Override
    public void onResume() {
        super.onResume();


		//initialize parse to connect with the application
		Parse.initialize(this, "j17LUua9VZ96CCEAP1QKimCoUuGlOfKRYnyLoqWv", "C7QJdvSFey3DwfZSCSqMaC2hjWFZwvQCr9ruiOoE");

		this.currentTask = -1;

		LayoutInflater inflater = (LayoutInflater) this.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
		LinearLayout linlay = (LinearLayout) inflater.inflate(R.layout.setup_screen, null);

		String text = "";

        this.uninstallLayout = (SetupRelativeLayout) linlay.findViewById(R.id.setup_uninstall);
        text = getString(R.string.setup_uninstall_string);
        this.uninstallLayout.setText(text);
        this.uninstallLayout.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (currentTask == UNINSTALL_TASK)
                    uninstall();
            }
        });

        this.enableIknowu = (SetupRelativeLayout) linlay.findViewById(R.id.setup_enable_iknowu);
        text = getString(R.string.setup_enable_string);
        this.enableIknowu.setText(text);
        this.enableIknowu.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (currentTask == ENABLE_TASK)
                    openKeyboardSettings();
            }
        });

        this.setAsDefault = (SetupRelativeLayout) linlay.findViewById(R.id.setup_set_default);
        text = getString(R.string.setup_set_default_string);
        this.setAsDefault.setText(text);
        this.setAsDefault.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (currentTask == SET_DEFAULT_TASK)
                    chooseDefault();
            }
        });

        this.handedness = (SetupRelativeLayout) linlay.findViewById(R.id.setup_handedness);
        text = getString(R.string.setup_handedness_string);
        this.handedness.setText(text);
        this.handedness.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (currentTask == HANDEDNESS_TASK)
                    openSetHandedness();
            }
        });

        //do a refresh every time we start up
        this.refresh();

        if (!tasksCompleted) {
            this.setContentView(linlay);
        }

		/*
		this.chooseLanguage = (SetupRelativeLayout) linlay.findViewById(R.id.setup_choose_language);
		text = getString(R.string.setup_choose_language_string);
		this.chooseLanguage.setText(text);
		this.chooseLanguage.setImageResource(R.drawable.task_complete);
		*/

		/*
		this.cloudSignup = (SetupRelativeLayout) linlay.findViewById(R.id.setup_cloud_signup);
		text = getString(R.string.setup_cloud_signup_string);
		this.cloudSignup.setText(text);
		this.cloudSignup.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				if (currentTask == CLOUD_SYNC_TASK) {
					openCloudSignup();
				}
			}
		});
		*/
    }

    @Override
    public void onPause() {
        super.onPause();
    }

    //basically just establishes where we currently are in the setup process
    private void refresh() {
        //run through all of the checks to see where in the setup we currently are
        this.checkUninstall();
        if (this.currentTask == ENABLE_TASK) this.checkEnabled();
        if (this.currentTask == SET_DEFAULT_TASK) this.checkSetAsDefault(false);
        //this.checkCloudSignup();
        if (this.currentTask == HANDEDNESS_TASK) this.checkHandedness();

        switch(this.currentTask) {
            case ENABLE_TASK:
                this.enableIknowu.setAsCurrentTask();
                this.uninstallLayout.startCompleteAnimation();  //start the appropriate animation for the previous task, to indicate completion
                break;
            case SET_DEFAULT_TASK:
                this.setAsDefault.setAsCurrentTask();
                this.enableIknowu.startCompleteAnimation();
                break;
            case CLOUD_SYNC_TASK:
                this.cloudSignup.setAsCurrentTask();
                break;
            case HANDEDNESS_TASK:
                this.handedness.setAsCurrentTask();
                this.setAsDefault.startCompleteAnimation();
                break;
            case UNINSTALL_TASK:
                this.uninstallLayout.setAsCurrentTask();
                break;
            case TASKS_COMPLETE:
                this.handedness.startCompleteAnimation();
                this.allTasksCompleted();
                break;
        }
    }

    private void allTasksCompleted() {
        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(this);

        SharedPreferences.Editor edit = sp.edit();
        edit.putBoolean(PREF_SETUP_COMPLETE, true);
        edit.commit();

        this.tasksCompleted = true;

        Toast.makeText(this, "Setup Complete!", Toast.LENGTH_SHORT).show();

        Message msg = mHandler.obtainMessage(MSG_TASKS_COMPLETED);
        mHandler.sendMessageDelayed(msg, 1000);
        //this.close();
    }

    /*
    public void openSecondaryMenu(View button) {
        LayoutInflater inflater = (LayoutInflater) this.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        LinearLayout linlay = (LinearLayout) inflater.inflate(R.layout.setup_complete_menu, null);

        String text = "";

        this.helpVids = (SetupRelativeLayout) linlay.findViewById(R.id.setup_help_vids);
        text = getString(R.string.setup_help_vids_string);
        this.helpVids.setText(text);
        this.helpVids.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                openHelpVideos();
            }
        });
        this.helpVids.setAsStaticTask();

        this.gotoSettings = (SetupRelativeLayout) linlay.findViewById(R.id.setup_goto_settings);
        text = getString(R.string.setup_goto_settings_string);
        this.gotoSettings.setText(text);
        this.gotoSettings.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                gotoSettings();
            }
        });
        this.gotoSettings.setAsStaticTask();

        this.finishSetup = (SetupRelativeLayout) linlay.findViewById(R.id.setup_finish);
        text = getString(R.string.setup_finish_string);
        this.finishSetup.setText(text);
        this.finishSetup.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                close();
            }
        });
        this.finishSetup.setAsStaticTask();

        this.setContentView(linlay);
    }
    */

    private void uninstall() {
        Intent intent = new Intent(Intent.ACTION_DELETE, Uri.fromParts("package", this.currentPackageUninstall, null));
        startActivity(intent);
    }

    private void checkUninstall() {
        this.currentTask = UNINSTALL_TASK;

        String pname = this.getPackageName();

        //check if the current package is the one we might want to uninstall
        //if it is, skip it
        if (!pname.equals(PHONE_TRIAL_PACKAGE)) {
            PackageManager pm = getPackageManager();
            //try to get info for phone trial package, if we can't
            //then it isn't installed. If we can, then set it as the current package to be
            //uninstalled
            try {
                pm.getPackageInfo(PHONE_TRIAL_PACKAGE, PackageManager.GET_ACTIVITIES);
                this.currentPackageUninstall = PHONE_TRIAL_PACKAGE;
            } catch (PackageManager.NameNotFoundException e) {
                this.uninstallLayout.setTaskComplete();
                this.currentTask = ENABLE_TASK;
            }
        } else {
            this.uninstallLayout.setTaskComplete();
            this.currentTask = ENABLE_TASK;
        }

        //if we set the current task to the next one, then check agian for the other trial package
        //to be 100% sure they are both uninstalled
        if (currentTask == ENABLE_TASK) {
            if (!pname.equals(TABLET_TRIAL_PACKAGE)) {
                PackageManager pm = getPackageManager();
                //try to get info for tablet trial package, if we can't
                //then it isn't installed. If we can, then set it as the current package to be
                //uninstalled
                try {
                    pm.getPackageInfo(TABLET_TRIAL_PACKAGE, PackageManager.GET_ACTIVITIES);
                    this.currentPackageUninstall = TABLET_TRIAL_PACKAGE;
                } catch (PackageManager.NameNotFoundException e) {
                    this.uninstallLayout.setTaskComplete();
                    this.currentTask = ENABLE_TASK;
                }
            } else {
                this.uninstallLayout.setTaskComplete();
                this.currentTask = ENABLE_TASK;
            }
        }
    }

    private void checkEnabled() {
        InputMethodManager imm = (InputMethodManager) this.getSystemService(INPUT_METHOD_SERVICE);
        List<InputMethodInfo> enabledInputs = imm.getEnabledInputMethodList();

        //this.currentTask = ENABLE_TASK;	//flag this as the current task
        String name = "";
        for (int i=0; i < enabledInputs.size(); i++) {
            name = enabledInputs.get(i).getServiceName();
            if (name.equals(PACKAGE_NAME)) {
                this.enableIknowu.setTaskComplete();
                this.currentTask = SET_DEFAULT_TASK;	//if this has been completed, flag the next as the current task
                break;
            }
        }
    }

    private void openKeyboardSettings() {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setCancelable(true);
        builder.setIcon(R.drawable.iknowulogo);
        builder.setTitle("Wait!");
        builder.setMessage(getString(R.string.wait_hit_back));

        builder.setPositiveButton("OK",  new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int set) {
                startActivity(new Intent(android.provider.Settings.ACTION_INPUT_METHOD_SETTINGS));
                dialog.dismiss();
            }
        });

        this.handedDialog = builder.create();
        this.handedDialog.show();
    }

    private void checkSetAsDefault(boolean needsAnimation) {
        String id = Settings.Secure.getString(getContentResolver(), Settings.Secure.DEFAULT_INPUT_METHOD);

        //Log.e("Default Id = ", ""+id);

        if (id.equals(this.getPackageName() + INPUT_NAME) || id.equals(this.getPackageName() + "/.IKnowUKeyboardService")) {
            this.setAsDefault.setTaskComplete();
            this.currentTask = HANDEDNESS_TASK;
            this.handedness.setAsCurrentTask();

            this.mHandler.removeMessages(MSG_CHECK_PICKED);
            if (needsAnimation)
                this.setAsDefault.startCompleteAnimation();
            //this.refresh();
        } else {
            Message msg = mHandler.obtainMessage(MSG_CHECK_PICKED);
            mHandler.sendMessageDelayed(msg, 500);
        }
    }

    private void chooseDefault() {
        InputMethodManager imm = (InputMethodManager) this.getSystemService(INPUT_METHOD_SERVICE);
        imm.showInputMethodPicker();
    }

    private void checkCloudSignup() {
        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(this);

        ParseUser user = ParseUser.getCurrentUser();

        //if there is a currentUser then we are good to go
        //set this as completed
        if (user != null) {
            this.cloudSignup.setTaskComplete();
            this.currentTask = HANDEDNESS_TASK;
            //otherwise check to see if the preferences have been set
            //and if so, then the process has been completed as well
        } else {
            String username = sp.getString(PREF_SIGNUP_FIRST, "");
            String pass = sp.getString(PREF_SIGNUP_SECOND, "");

            if (username.length() > 0 && pass.length() > 0) {
                this.cloudSignup.setTaskComplete();
                this.currentTask = HANDEDNESS_TASK;
            }
        }
    }

    private void openCloudSignup() {
        LayoutInflater inflater = (LayoutInflater) this.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        LinearLayout layout = (LinearLayout) inflater.inflate(R.layout.cloud_signup, null);

        this.emailText = (EditText) layout.findViewById(R.id.setup_email_box);
        this.passText = (EditText) layout.findViewById(R.id.setup_password_box);

        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setView(layout);
        builder.setCancelable(true);
        builder.setIcon(R.drawable.iknowulogo);
        builder.setTitle("Cloud Sync");

        builder.setPositiveButton("SIGNUP",  new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int set) {
                tryLogin();
                dialog.dismiss();
            }
        });

        this.signupDialog = builder.create();
        this.signupDialog.show();
    }

    private void tryLogin() {
        String username = this.emailText.getText().toString();
        String pass = this.passText.getText().toString();

        //need to check if they have already created an account with us
        //if successful then yes they have, and we can proceed to the next step
        //otherwise try to sign them up
        ParseUser.logInInBackground(username, pass,
                new LogInCallback() {
                    public void done(ParseUser user, ParseException e) {
                        if (user != null) {
                            //do this because they might not actually want to be logged in
                            //let them change this in the preferences
                            ParseUser.logOut();
                            loginComplete();
                        } else {
                            if (e.getCode() == ParseException.OBJECT_NOT_FOUND) {
                                signUp();
                            }
                        }
                    }
                });
    }

    private void signUp() {
        //Log.d("CAN'T LOG IN", "SIGNING UP");
        String username = this.emailText.getText().toString();
        String pass = this.passText.getText().toString();

        ParseUser user = new ParseUser();
        user.setUsername(username);
        user.setPassword(pass);
        user.setEmail(username);

        user.signUpInBackground(new SignUpCallback() {
            public void done(ParseException e) {
                if (e == null) {
                    loginComplete();
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
    }

    private void loginComplete() {
        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(this);
        SharedPreferences.Editor editor = sp.edit();
        editor.putString(PREF_SIGNUP_FIRST, emailText.getText().toString());
        editor.putString(PREF_SIGNUP_SECOND, passText.getText().toString());
        editor.commit();
        showAlert("Sign up successful!");
        refresh();
    }

    /**
     * Show an alert on the screen via a toast message
     * @param text
     */
    private void showAlert(String text) {
        Toast toast = Toast.makeText(this, "IKnowU Keyboard: "+text, Toast.LENGTH_SHORT);
        toast.setGravity(Gravity.CENTER_VERTICAL | Gravity.CENTER_HORIZONTAL, 0, 0);
        toast.show();
    }

    private void checkHandedness() {
        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(this);

        boolean complete = sp.getBoolean(PREF_HANDEDNESS_COMPLETE, false);
        if (complete) {
            this.handedness.setTaskComplete();
            this.currentTask = TASKS_COMPLETE;
        }
    }

    private void openSetHandedness() {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setCancelable(true);
        builder.setIcon(R.drawable.iknowulogo);
        builder.setTitle("Select Handedness");

        builder.setSingleChoiceItems(new String[]{"Right Handed","Left Handed"}, -1, new DialogInterface.OnClickListener() {

            @Override
            public void onClick(DialogInterface dialog, int which) {
                switch (which) {
                    case 0:
                        setHandedness(true);
                        dialog.dismiss();
                        break;
                    case 1:
                        setHandedness(false);
                        dialog.dismiss();
                        break;
                }
            }
        });

        this.handedDialog = builder.create();
        this.handedDialog.show();
    }

    private void setHandedness(boolean rightHand) {
        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(this);
        SharedPreferences.Editor editor = sp.edit();
        if (rightHand) {
            editor.putBoolean(PREF_HANDEDNESS, true);
        } else {
            editor.putBoolean(PREF_HANDEDNESS, false);
        }
        editor.putBoolean(PREF_HANDEDNESS_COMPLETE, true);
        editor.commit();
        this.refresh();
    }

    private void openHelpVideos() {
        String url = "http://www.iknowu.net/video.html";
        Intent intent = new Intent(Intent.ACTION_VIEW);
        intent.setData(Uri.parse(url));
        //intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        startActivity(intent);
    }

    private void gotoSettings() {
        Intent intent = new Intent();
        intent.setClass(this, IKnowUSettings.class);
        //intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        startActivity(intent);
    }

    /*
    private void showHelpScreen() {
        LayoutInflater inflater = (LayoutInflater) this.getSystemService(LAYOUT_INFLATER_SERVICE);

        LinearLayout linlay = (LinearLayout) inflater.inflate(R.layout.setup_expandable_list, null);
        ExpandableListView exList = (ExpandableListView) linlay.findViewById(R.id.ExpList);
        exList.setCacheColorHint(0x00000000);
        //exList.setSelector(android.R.color.transparent);

        ArrayList<ExpandableListGroup> groups = new ArrayList<ExpandableListGroup>();

        TextView groupText;
        TextView childText;
        ImageView childImage;

        int totalChildren = 0;

        for (int i = 0; i < this.numChildren.length; i++) {
            ExpandableListGroup group = (ExpandableListGroup) inflater.inflate(R.layout.expandable_list_group, null);
            group.setPosition(i);
            groupText = (TextView) group.findViewById(R.id.expandable_group_name);
            groupText.setText(this.getString(this.groupTextIds[i]));

            ArrayList<ExpandableListItem> children = new ArrayList<ExpandableListItem>();
            for (int j=0; j < this.numChildren[i]; j++) {
                ExpandableListItem child = (ExpandableListItem) inflater.inflate(R.layout.expandable_list_item, null);

                childText = (TextView) child.findViewById(R.id.expandable_item_text);
                childText.setText(this.getString(this.childTextIds[totalChildren]));
                childImage = (ImageView) child.findViewById(R.id.expandable_item_image);
                childImage.setImageResource(this.childImageIds[totalChildren]);
                child.setPosition(j);
                children.add(child);
                group.setItems(children);

                totalChildren++;
            }
            groups.add(group);
        }

        exList.setAdapter(new ExpandListAdapter(this, groups));

        this.setContentView(linlay);
    }
    */

    public void close() {
        this.finish();
    }
}
