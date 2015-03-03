package com.iknowu.miniapp;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.content.pm.ServiceInfo;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.Log;
import android.view.View;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.RemoteViews;
import android.widget.Toast;

import com.iknowu.IKnowUKeyboardService;
import com.iknowu.KeyboardLinearLayout;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Iterator;
import java.util.List;

public class MiniAppManager implements View.OnClickListener {
	
	private static final String LOG_TAG = "MiniAppManager";
	
	public static final String ACTION_MINIAPP_INTERFACE = "com.iknowu.miniapp.IKnowUMiniAppInterface";
	//public static final String ACTION_MINIAPP_START = "com.iknowu.miniapp.action.START";
	
	public static final String CATEGORY_MINIAPP_PROPER_NOUN = "com.iknowu.miniapp.category.PROPER_NOUN";
    public static final String CATEGORY_MINIAPP_PROPER_NOUN_EXTENDED = "com.iknowu.miniapp.category.PROPER_NOUN_EXTENDED";
	public static final String CATEGORY_MINIAPP_PHONE_NUMBER = "com.iknowu.miniapp.category.PHONE_NUMBER";
	public static final String CATEGORY_MINIAPP_EMAIL_ADDRESS = "com.iknowu.miniapp.category.EMAIL_ADDRESS";
	public static final String CATEGORY_MINIAPP_PASSWORD_FIELD = "com.iknowu.miniapp.category.PASSWORD_FIELD";
	public static final String CATEGORY_MINIAPP_WEB_URL = "com.iknowu.miniapp.category.WEB_URI";
	public static final String CATEGORY_MINIAPP_LAST_WORD = "com.iknowu.miniapp.category.LAST_WORD";
	public static final String CATEGORY_MINIAPP_ALL_TEXT = "com.iknowu.miniapp.category.ALL_TEXT";
	public static final String CATEGORY_MINIAPP_NUMBER = "com.iknowu.miniapp.category.NUMBER";
	public static final String CATEGORY_MINIAPP_MEASUREMENT = "com.iknowu.miniapp.category.MEASUREMENT";
	public static final String CATEGORY_MINIAPP_SCHEDULE = "com.iknowu.miniapp.category.SCHEDULE";
	public static final String CATEGORY_MINIAPP_DATE = "com.iknowu.miniapp.category.DATE";
	
	private static final int ACTION_GET_INFO = 0;
	private static final int ACTION_START_APP = 1;
	//private static final String KEY_PKG = "pkg";
	//private static final String KEY_SERVICENAME = "servicename";
	//private static final String KEY_ACTIONS = "actions";
	//public static final String KEY_CATEGORIES = "categories";
	//private static final String BUNDLE_EXTRAS_CATEGORY = "category";
	
	private ArrayList<MiniAppConnection> serConns;
	private IKnowUKeyboardService inputService;
	private ArrayList<MiniApp> miniApps;	//a local array list of all mini apps installed. used for quick lookups of icons
    private ArrayList<String> categoriesToSearch;
	
	private int currentAction = 0;
	
	private String currentCategory;
	private String currentData;
    public int prevTotal;
	
	private MiniApp currentMiniApp;
	
	private MiniAppScreen miniAppScreen;
	
	private KeyboardLinearLayout kbLinearLayout;
    //private KeyboardScreen kbScreen;
	
	public MiniAppManager(IKnowUKeyboardService service) {
		this.inputService = service;
		this.serConns = new ArrayList<MiniAppConnection>();
		//this.detectMiniAppList();
	}
	
	public void setMiniAppScreen(MiniAppScreen drawer) {
		this.miniAppScreen = drawer;
		this.miniAppScreen.setMiniAppManager(this);
	}
	
	public void setKeyboardScreen(KeyboardLinearLayout kbs) {
		this.kbLinearLayout = kbs;
	}

    public boolean needsCategory(String cat) {
        return this.categoriesToSearch.contains(cat);
    }
	
	/**
	 * Find all of the services installed on this device that are listening on the
	 * interface for miniapps.
	 */
	public void detectMiniAppList() {
		try {
			if (this.miniApps != null) {
				this.removePreviousMiniApps();
			}
			this.miniApps = new ArrayList<MiniApp>();
            this.categoriesToSearch = new ArrayList<String>();
			//categories = new ArrayList<String>();
	        PackageManager packageManager = this.inputService.getPackageManager();
	        Intent baseIntent = new Intent( ACTION_MINIAPP_INTERFACE );
			baseIntent.setFlags( Intent.FLAG_DEBUG_LOG_RESOLUTION );
	        List<ResolveInfo> list = packageManager.queryIntentServices(baseIntent, PackageManager.GET_RESOLVED_FILTER );
			//Log.d( LOG_TAG, "fillPluginList: "+list );
	        for( int i = 0 ; i < list.size() ; ++i ) {
	            ResolveInfo info = list.get( i );
	            ServiceInfo sinfo = info.serviceInfo;
				IntentFilter filter = info.filter;
				//Log.d( LOG_TAG, "fillPluginList: i: "+i+"; sinfo: "+sinfo+";filter: "+filter );
	            if( sinfo != null && filter != null && filter.countCategories() > 0) {
	                MiniApp item = new MiniApp();
	                item.setPackageName( sinfo.packageName );
	                item.setClassName( sinfo.name );
	                item.setId(i);
	                
					//retrieve all of the categories that this miniapp is registered to;
					StringBuilder categories = new StringBuilder();
					for( Iterator<String> categoryIterator = filter.categoriesIterator() ; categoryIterator.hasNext() ; ) {
						String category = categoryIterator.next();
                        //add this category to our list of ones to search for if it is not already in there.
                        if (!this.categoriesToSearch.contains(category)) this.categoriesToSearch.add(category);

						if( categories.length() > 0 )
							categories.append( "," );
						categories.append( category );
					}
					//item.put( KEY_ACTIONS, new String( actions ) );
					item.setCategories( new String ( categories ) );
					
	                this.miniApps.add( item );
	                
	                MiniAppConnection conn = new MiniAppConnection(i);
	                this.initService(sinfo.packageName, sinfo.name, conn, ACTION_GET_INFO);
	            }
	        }
			//Log.d( LOG_TAG, "Total MiniApps: "+this.miniApps.size() );
			//Log.d( LOG_TAG, "categories: "+categories );
		} catch( Exception e ) {
			IKnowUKeyboardService.sendErrorMessage(e);
		}
    }

	private void removePreviousMiniApps() {
        if ((this.kbLinearLayout != null) && (this.kbLinearLayout.getActionBar() != null)) {
		    this.kbLinearLayout.getActionBar().removeAllViews();
        }
        if ((this.miniAppScreen != null) && (this.miniAppScreen.getActionBar() != null)) {
		    this.miniAppScreen.getActionBar().removeAllViews();
        }
        if (this.miniApps != null) {
		    this.miniApps.clear();
        }
	}
	
	private void getMiniAppIcon(MiniAppConnection conn) {
		try {
			//get the icon for this miniapp and store it locally as an imageview
			//Log.v(LOG_TAG, "Get mini app icon, index = "+conn.index);
	        if (conn.miniAppInterface != null) {
	        	//inflate the small icon
	        	LinearLayout vg = new LinearLayout(this.inputService);
	        	RemoteViews rm = conn.miniAppInterface.getSmallIcon();
	        	View inflatedView = rm.apply( this.inputService, vg );
	            if( vg.getChildCount() > 0 )
	                vg.removeAllViews();
	            vg.addView( inflatedView );
	            vg.requestLayout();
	            //if (this.kbScreen != null) {
		            ImageView sicon = (ImageView) vg.getChildAt(0);
		            vg.removeView(sicon);

                    if (this.kbLinearLayout ==  null) {
                        throw new NullPointerException("this.kbLinearLayout is null");
                    }
                    if (this.kbLinearLayout.getActionBar() == null) {
                        throw new NullPointerException("this.kbLinearLayout.getActionBar() is null");
                    }

                    int childDimen = 0;
                    if (this.kbLinearLayout != null) {
                        MiniAppActionBar actionBar = this.kbLinearLayout.getActionBar();
                        if (actionBar != null) {
                            childDimen = actionBar.childDimens;
                        }
                    }

		            //LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(this.kbLinearLayout.getActionBar().childDimens, this.kbLinearLayout.getActionBar().childDimens);
                    LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(childDimen, childDimen);
					params.rightMargin = 20;
					sicon.setLayoutParams(params);
		            //Log.d(LOG_TAG, "small icon source = "+sicon.getDrawable());
		            this.miniApps.get(conn.index).setSmallIcon(sicon/*.getDrawable()*/);
		            //sicon.setVisibility(View.GONE);
		            IKnowUKeyboardService.log( Log.DEBUG, LOG_TAG, "Adding mini-app to bar, small icon = "+ sicon);
		            //this.kbScreen.getActionBar().addView(this.miniApps.get(conn.index).getSmallIcon());
	            //}
	            //inflate the large icon
	            rm = conn.miniAppInterface.getLargeIcon();
	        	View inflatedView2 = rm.apply( this.inputService, vg );
	            if( vg.getChildCount() > 0 )
	                vg.removeAllViews();
	            vg.addView( inflatedView2 );
	            vg.requestLayout();
	            
	            //if (this.miniAppScreen != null) {
	            	ImageView licon = (ImageView) vg.getChildAt(0);
		            vg.removeView(licon);

                    childDimen = 0;
                    if (this.miniAppScreen != null) {
                        MiniAppActionBar miniAppActionBar = this.miniAppScreen.getActionBar();
                        if (miniAppActionBar != null) {
                            childDimen = miniAppActionBar.childDimens;
                        }
                    }
		            /*LinearLayout.LayoutParams*/ //params = new LinearLayout.LayoutParams(this.miniAppScreen.getActionBar().childDimens, this.miniAppScreen.getActionBar().childDimens);
                    params = new LinearLayout.LayoutParams(childDimen, childDimen);
					params.rightMargin = 20;
					licon.setLayoutParams(params);
					licon.setOnClickListener(this);
		            //Log.d(LOG_TAG, "large icon source = "+licon.getDrawable());
		            this.miniApps.get(conn.index).setLargeIcon(licon/*.getDrawable()*/);
		            //licon.setVisibility(View.GONE);
		            IKnowUKeyboardService.log( Log.DEBUG, LOG_TAG, "Adding mini-app to bar, large icon = "+ licon);
		            //this.miniAppScreen.getActionBar().addView(this.miniApps.get(conn.index).getLargeIcon());
	            //}
	        }
	        this.releaseService(conn);
		} catch (Exception e) {
			IKnowUKeyboardService.sendErrorMessage(e);
		}
	}
	
	/**
	 * Get a populated list of miniapps that can be used in accordance with the
	 * given category string.
	 *
	 * @return a populated list of miniapps for the specified category
	 */
	public int getMiniApps(ArrayList<String[]> cats) {
		try {
			this.clearMiniAppList(true);

			int[] added = new int[this.miniApps.size()];
            Arrays.fill(added, -1);

			int total = 0;
            for (int i = 0; i < this.miniApps.size(); i++) {
                MiniApp app = this.miniApps.get(i);

                for (int j = 0; j < cats.size(); j++) {

                    String[] cat = cats.get(j);

                    this.currentCategory = cat[0];
                    this.currentData = cat[1];
                    //IKnowUKeyboardService.log(Log.VERBOSE, LOG_TAG, "getMiniApps, currentData = "+this.currentData);

                    //Log.v(LOG_TAG, "ID = "+ app.getId() +", App = "+app.getClassName() + ", categories = "+app.getCategories());
                    if (app.getCategories().contains(cat[0]) && !this.containsId(added, app.getId())) {
                        app.setDatafound(this.currentData);
                        app.setCategoryfound(this.currentCategory);

                        if (app.getSmallIcon() != null) {
                            this.kbLinearLayout.getActionBar().addView(app.getSmallIcon());
                        }
                        if (app.getLargeIcon() != null) {
                            ImageView imgView = app.getLargeIcon();
                            // We need a separate index for the action bar view. In some cases, this index won't be the same
                            // as the MiniApp id. Store the index as part of the ImageView object.
                            imgView.setTag(Integer.valueOf(total));
                            imgView.setBackgroundColor(0xffffffff);
                            this.miniAppScreen.getActionBar().addView(imgView);
                        }

                        added[total] = app.getId();
                        total++;
                    }
                }
            }

            if (this.prevTotal != total) this.inputService.kbContainerView.getSideRelativeLayout().startAnimation();
			this.kbLinearLayout.getActionBar().invalidate();
			this.miniAppScreen.getActionBar().invalidate();

            this.prevTotal = total;
			return total;
		} catch (Exception e) {
			IKnowUKeyboardService.sendErrorMessage(e);
			return 0;
		}
	}
	
	private boolean containsId( int[] vals, int id ) {
		for ( int i = 0; i < vals.length; i++ ) {
			if (vals[i] == id) return true;
		}
		return false;
	}
	
	public void clearMiniAppList(boolean removeActionBarIcons) {
		if (this.miniAppScreen != null && this.miniAppScreen.getContentLayout() != null) {
            this.inputService.kbContainerView.getSideRelativeLayout().getMiniAppScreen().getActionBar().unhighlightItem();
			this.miniAppScreen.getContentLayout().removeAllViews();

            if (removeActionBarIcons) {
                this.kbLinearLayout.getActionBar().removeAllViews();
                this.miniAppScreen.getActionBar().removeAllViews();
            }

			while (this.serConns.size() > 0) {
				this.releaseService(this.serConns.get(0));
			}
		}
	}
	
	public ArrayList<MiniApp> getDetectedMiniApps() {
		return this.miniApps;
	}
	
	public void startMiniApp(int id, int hid) {
		try {
			this.currentMiniApp = this.miniApps.get(id);
            this.inputService.kbContainerView.getSideRelativeLayout().getMiniAppScreen().getActionBar().unhighlightItem();
            this.inputService.kbContainerView.getSideRelativeLayout().getMiniAppScreen().getActionBar().highlightItem(hid);
			if (this.miniAppScreen.getState() == MiniAppScreen.STATE_OPEN) {
				//this.miniAppScreen.setState(MiniAppScreen.STATE_CLOSED);
			} else {
				
				MiniAppConnection conn = new MiniAppConnection(id);
				ProgressBar pb = new ProgressBar(this.inputService);
				LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(100,100);
				pb.setLayoutParams(params);
				this.miniAppScreen.setContentLayout(pb, -1);
				this.miniAppScreen.setState(MiniAppScreen.STATE_OPEN);
				this.initService(this.currentMiniApp.getPackageName(), this.currentMiniApp.getClassName(), conn, ACTION_START_APP);
			}
		} catch ( Exception e ) {
			IKnowUKeyboardService.sendErrorMessage(e);
		}
	}
	
	private void switchView(IKnowUMiniAppInterface app) {
		try {
			Log.d(LOG_TAG, "Switch view id = "+app+", size = "+this.serConns.size());
			RemoteViews rm = app.getView(this.inputService.getPackageName(), this.currentMiniApp.getDataFound(), this.currentMiniApp.getCategoryFound());
			LinearLayout vg = new LinearLayout(this.inputService);
			LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.MATCH_PARENT);
			vg.setLayoutParams(params);
			vg.setOrientation(LinearLayout.VERTICAL);
	    	View inflatedView = rm.apply( this.inputService, vg );
	    	params = new LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.MATCH_PARENT);
	    	inflatedView.setLayoutParams(params);
	        vg.addView( inflatedView );
	        //vg.requestLayout();
	        Log.d(LOG_TAG, "Switch View, view = "+rm);
			this.miniAppScreen.setContentLayout(vg, MiniAppScreen.ANIM_NONE);
			this.miniAppScreen.setState(MiniAppScreen.STATE_OPEN);
			//this.inputService.switchToMiniAppView();
		} catch (Exception e) {
			Toast.makeText(this.inputService, "Error reaching for app", Toast.LENGTH_SHORT).show();
			IKnowUKeyboardService.sendErrorMessage(e);
		}
	}
	
	public void updateView( RemoteViews rm, int anim ) {
		Log.i(LOG_TAG, "Updating View = "+rm);
		LinearLayout vg = new LinearLayout(this.inputService);
		LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.MATCH_PARENT);
		vg.setLayoutParams(params);
		vg.setOrientation(LinearLayout.VERTICAL);
    	View inflatedView = rm.apply( this.inputService, vg );
    	params = new LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.MATCH_PARENT);
    	inflatedView.setLayoutParams(params);
        vg.addView( inflatedView );
        //vg.requestLayout();
        Log.d(LOG_TAG, "Switch View, view = "+rm);
        if (this.miniAppScreen != null) {
		    this.miniAppScreen.setContentLayout(vg, anim);
		    this.miniAppScreen.setState(MiniAppScreen.STATE_OPEN);
        }
	}
	
	private void initService(String packageName, String className, MiniAppConnection conn, int action) {
		this.currentAction = action;
		this.serConns.add(conn);
		//Log.v(LOG_TAG, "Package = "+packageName+", Class = "+className);
        Intent i = new Intent( ACTION_MINIAPP_INTERFACE );
        i.setClassName(packageName, className);
        this.inputService.bindService( i, conn, Context.BIND_AUTO_CREATE);
        //Log.d( LOG_TAG, "bindService()" );
	}

	private void releaseService(MiniAppConnection mcon) {
		//Log.d( LOG_TAG, "releaseService(), index = "+mcon.index+", connSize = "+this.serConns.size() );
		//Log.d(LOG_TAG, "isbound = "+mcon.isBound);
		if (mcon.isBound) {
			try {
				mcon.miniAppInterface.onFinishConnection();
			} catch (RemoteException e) {
				e.printStackTrace();
			}
			this.inputService.unbindService( mcon );
		}
		this.removeConn(mcon);
	}
	
	private void removeConn(MiniAppConnection mcon) {
		if (mcon != null) {
			mcon.isBound = false;
	        this.serConns.remove(mcon);
	        mcon = null;
		}
		
		if (this.miniAppScreen != null) {
			this.miniAppScreen.setState(MiniAppScreen.STATE_CLOSED);
		}
        //Log.d(LOG_TAG, "rmoveConn, connSize = "+this.serConns.size());
	}
	
	private class MiniAppConnection implements ServiceConnection {
		
		private int index;
		private boolean isBound;
		private IKnowUMiniAppInterface miniAppInterface;
		
		public MiniAppConnection(int idx) {
			this.index = idx;
		}
		
		@Override
		public void onServiceConnected(ComponentName className, IBinder boundService ) {
			this.miniAppInterface = IKnowUMiniAppInterface.Stub.asInterface(boundService);
            this.isBound = true;
            Log.d( LOG_TAG,"onServiceConnected" );
            if (currentAction == ACTION_GET_INFO) {
            	getMiniAppIcon(this);
                // Mini app icons are now available, so update mini-app action bar.
                inputService.updateCandidates();
            } else if (currentAction == ACTION_START_APP) {
            	switchView(this.miniAppInterface);
            }
        }
		
		public void onServiceDisconnected(ComponentName className) {
			this.isBound = false;
            Log.d( LOG_TAG,"onServiceDisconnected" );
            removeConn(this);
        }
	}

	@Override
	public void onClick(View v) {
        Integer i = (Integer)v.getTag();
		this.startMiniApp(v.getId(), i);
	}
}
