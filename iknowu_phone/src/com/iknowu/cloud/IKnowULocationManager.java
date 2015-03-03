package com.iknowu.cloud;

import android.content.Context;
import android.location.Criteria;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.location.LocationProvider;
import android.os.AsyncTask;
import android.os.Bundle;
import android.util.Log;

import com.iknowu.IKnowUKeyboardService;

import org.json.JSONObject;

import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.URL;
import java.util.ArrayList;
import java.util.Iterator;

public class IKnowULocationManager {
	
	private static final String NEARBY_PLACES_FILE = "nearby.txt";
	
	public double latitude;
	public double longitude;
	private LocationManager locationManager;
	private Context context;
	private Location location;
	private LocationProvider locationProvider;
	public ArrayList<String> places;
	
	private LocationListener locationListener = new LocationListener() {
		@Override
		public void onLocationChanged(Location location) {
			locationChanged(location);
		}

		@Override
		public void onProviderDisabled(String provider) {
		}

		@Override
		public void onProviderEnabled(String provider) {
		}

		@Override
		public void onStatusChanged(String provider, int status, Bundle extras) {
		}
	};
	
	public IKnowULocationManager(Context ctx) {
		this.context = ctx;
		//String poiQuery = "https://maps.google.com/maps/api/place/nearbysearch/json?location=-33.8670522,151.1957362&radius=500&types=food&name=harbour";
	}
	
	public void getLocation() {
		this.locationManager = (LocationManager) this.context.getSystemService(Context.LOCATION_SERVICE);
		
		Criteria crit = new Criteria();
		crit.setAccuracy(Criteria.ACCURACY_COARSE);
		String provName = this.locationManager.getBestProvider(crit, true);
		
		//if provider is enabled then use it for location updates
		if (this.locationManager.isProviderEnabled(provName)) {
	    	//Log.v(LOG_TAG, "Starting Updates for = "+provName);
	    	this.locationProvider = this.locationManager.getProvider(provName);
	    	
	    	this.locationManager.requestLocationUpdates(provName,
	  		      0,          // 1-second interval.
	  		      0f,         // 0 meters.
	  		      this.locationListener);
	    }
	}
	
	private void locationChanged(Location lctn) {
		this.location = lctn;
		
		this.latitude = this.location.getLatitude();
		this.longitude = this.location.getLongitude();
		
		IKnowUKeyboardService.log(Log.VERBOSE, "LocationManager", "location = "+this.latitude+", "+this.longitude);
		
		this.locationManager.removeUpdates(this.locationListener);
		
		//new DownloadPlacesTask().execute("https://maps.googleapis.com/maps/api/place/nearbysearch/json?location="+this.latitude+","+this.longitude+
		//		"&radius=1000&sensor=true&key=AIzaSyASEQhWU69IlMEA3Y8tXvlshNRVO-OxE2w");
	}
	
	private void parseLocations(String locs) {
		try {
			if (locs != null) {
				JSONObject json = new JSONObject(locs);
				
				Iterator itr = json.keys();
				
				while (itr.hasNext()) {
					String key = (String) itr.next();
					
					IKnowUKeyboardService.log(Log.VERBOSE, "LocationManager", "key = "+ key );
					IKnowUKeyboardService.log(Log.DEBUG, "LocationManager", "value = " + json.getString(key));
				}
            }
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
	
	// usually, subclasses of AsyncTask are declared inside the activity class.
	// that way, you can easily modify the UI thread from here
	private class DownloadPlacesTask extends AsyncTask<String, Void, String> {
	    @Override
	    protected String doInBackground(String... sUrl) {
	        try {
	            URL url = new URL(sUrl[0]);
	            IKnowUKeyboardService.log(Log.DEBUG, "Starting download file =", ""+url.toString());
	            //URLConnection connection = url.openConnection();
	            //connection.connect();
	            InputStream in = url.openStream();
	            
	            IKnowUKeyboardService.log(Log.DEBUG, "got input stream available =", ""+in.available());
	            
	            BufferedReader reader = new BufferedReader(new InputStreamReader(in));
	            StringBuilder sb = new StringBuilder();

	            String line = null;
                while ((line = reader.readLine()) != null) {
                    sb.append(line + "\n");
                }
                
                IKnowUKeyboardService.log(Log.DEBUG, "json return", "= "+sb.toString());
                
                in.close();
                reader.close();
                
                return sb.toString();
	            /*
	            // this will be useful so that you can show a typical 0-100% progress bar
	            int fileLength = connection.getContentLength();
	            
	            // download the file
	            InputStream input = new BufferedInputStream(url.openStream());
	            
	            String dir = IKnowUKeyboardService.filesDir + "/" + NEARBY_PLACES_FILE;
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
	            */
	            
	        } catch (Exception e) {
	        	IKnowUKeyboardService.sendErrorMessage(e);
	        }
	        return null;
	    }
	    
	    @Override
	    protected void onPostExecute(String result) {
	    	//IKnowUKeyboardService.log(Log.VERBOSE, "Post execute", "result = "+result);
	    	parseLocations(result);
	    }
	}
}