package com.iknowu.setup;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.FragmentManager;
import android.app.FragmentTransaction;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.net.Uri;
import android.os.Bundle;
import android.provider.Settings;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Gravity;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Toast;

import com.iknowu.IKnowUKeyboardService;
import com.iknowu.R;
import com.iknowu.downloader.DownloadActivity;
import com.iknowu.preferences.IKnowUSettings;

public class TutorialActivity extends Activity {

    private static final String PACKAGE_NAME = "com.iknowu.IKnowUKeyboardService";
    private static final String INPUT_NAME = "/com.iknowu.IKnowUKeyboardService";

    private StartupViewPager svp;

    private TourIndexFragment tourIndexFragment;
    private TourContentFragment tourContentFragment;
	
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

        IKnowUKeyboardService.log(Log.VERBOSE, "TutorialActivity.onResume()", "start func");

        this.setContentView(R.layout.tour_base_linear_layout);

        FragmentManager fm = this.getFragmentManager();

        this.tourIndexFragment = (TourIndexFragment) fm.findFragmentById(R.id.tour_index_fragment);
        this.tourContentFragment = (TourContentFragment) fm.findFragmentById(R.id.tour_content_fragment);

        this.tourIndexFragment.setActivity(this);
        this.tourContentFragment.setActivity(this);

        /*
        FragmentManager fragmentManager = getFragmentManager();
        FragmentTransaction fragmentTransaction = fragmentManager.beginTransaction();

        TourIndexFragment tif = new TourIndexFragment();

        fragmentTransaction.add(R.layout.tour_base_linear_layout, tif);
        fragmentTransaction.commit();

        /*
        LinearLayout lin = (LinearLayout) this.getLayoutInflater().inflate(R.layout.startup_view_pager, null);
        assert lin != null;
        this.svp = (StartupViewPager) lin.findViewById(R.id.startup_view_pager);
        this.svp.init(this);

        this.setContentView(lin);
        */
	}

    @Override
    public void onBackPressed() {
        /*
        if (this.svp.getCurrentItem() <= 0 ) this.close();
        else this.svp.decrementView();
        */
        this.close();
    }
	
	@Override
	public void onPause() {
		super.onPause();

        FragmentManager fm = this.getFragmentManager();
        FragmentTransaction ft = fm.beginTransaction();

        ft.remove(this.tourIndexFragment);
        ft.remove(this.tourContentFragment);

        ft.commit();
	}

    public void changeContent(int pos) {
        this.tourContentFragment.changeContent( pos );
    }

    private boolean checkSetAsDefault() {
        String id = Settings.Secure.getString(getContentResolver(), Settings.Secure.DEFAULT_INPUT_METHOD);
        //Log.e("Default Id = ", ""+id);
        if (id.equals(this.getPackageName() + INPUT_NAME) || id.equals(this.getPackageName() + "/.IKnowUKeyboardService")) {
            IKnowUKeyboardService.log(Log.VERBOSE, "Default detected", "returning true");
            return true;
        } else {
            return false;
        }
    }
	
	/**
	 * Show an alert on the screen via a toast message
	 * @param text
	 */
	public void showAlert(String text) {
		Toast toast = Toast.makeText(this, "IKnowU Keyboard: "+text, Toast.LENGTH_SHORT);
		toast.setGravity(Gravity.CENTER_VERTICAL | Gravity.CENTER_HORIZONTAL, 0, 0);
		toast.show();
	}
	
	public void openHelpVideos() {
		String url = "http://www.iknowu.net/video.html";
		Intent intent = new Intent(Intent.ACTION_VIEW);
		intent.setData(Uri.parse(url));
		//intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
		startActivity(intent);
	}

	public void gotoSettings(String action) {
		Intent intent = new Intent(action);
		intent.setClass(this, IKnowUSettings.class);
		//intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
		startActivity(intent);
	}

    public void goToLanguages() {
        Intent intent = new Intent();
        intent.setClass(this, DownloadActivity.class);
        //intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        startActivity(intent);
    }
	
	public void close() {
        boolean noPrompt;

        noPrompt = this.checkSetAsDefault();

        if (noPrompt) {
		    this.finish();
        } else {
            this.promptForSetup();
        }
	}

    private void promptForSetup() {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setCancelable(false);
        builder.setIcon(R.drawable.iknowulogo);
        builder.setTitle(R.string.tour_title_wait);

        builder.setMessage(R.string.tour_not_default);

        builder.setPositiveButton(R.string.label_ok_key, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                launchSetup();
            }
        });

        builder.setNegativeButton(R.string.label_cancel, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                dialog.dismiss();
                finish();
            }
        });

        AlertDialog dialog = builder.create();
        dialog.show();
    }

    private void launchSetup() {
        Intent intent = new Intent();
        intent.setClass(this, SetupActivity.class);
        //intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        startActivity(intent);
        this.finish();
    }
}
