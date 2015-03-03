package com.iknowu.autocorrect;

import java.io.IOException;
import java.util.Random;

import android.os.Bundle;
import android.app.Activity;
import android.util.Log;
import android.view.Menu;
import android.view.View;

public class MainActivity extends Activity {
	private AutoCorrect autocorrect;
	private boolean go;
	
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        
        try {
			autocorrect = new AutoCorrect(this);
		} catch (IOException e) {
			e.printStackTrace();
			finish();
			return;
		}
    }
    
    private String randomWord(Random r) {
    	int c;
    	int l = r.nextInt(5)+1+r.nextInt(2);
    	StringBuilder sb = new StringBuilder();
    	char ch;
    	
    	for(c = 0;c < l;c++) {
    		ch = (char)(r.nextInt(26) + 'a');
    		sb.append(ch);
    	}
    	
    	return sb.toString();
    }
    
    public void btnAuto_click(View view) {
    	go = true;
    	
    	new Thread(new Runnable() {
            public void run() {
            	ResultSet rs;
            	int cc;
            	Random r = new Random();
            	int c;
            	String s;
            	String pre;
            	long time;
            	
            	/*rs = autocorrect.correct("pleas", 10);
            	
            	for(cc = 0;cc < rs.size();cc++) {
            		Log.v("MainActivity","got "+rs.get(cc).toString());
            	}*/
            	
            	//autocorrect.walk();
                
                for(c = 0;(c < 100000) && go;c++) {
                	s = randomWord(r);
                	
                	if(r.nextBoolean()) {
                	pre = "the";
                	
                	Log.v("MainActivity","running auto-correct on '"+s+"' with previous '"+pre+"' (" + Integer.toString(c)+")");
                	} else {
                		Log.v("MainActivity","running auto-correct on '"+s+"' (" + Integer.toString(c)+")");
                		pre = null;
                	}
                	
                	time = System.currentTimeMillis();
                	
                	rs = autocorrect.correct(pre,s,10,4);
                	
                	time = System.currentTimeMillis() - time;
                	
                	Log.v("MainActivity","took "+Long.toString(time)+" ms");
                	
                	for(cc = 0;cc < rs.size();cc++) {
                		Log.v("MainActivity","got "+rs.get(cc).toString());
                	}
                }
            	
            	
            }
        }).start();
    }
    
    public void btnStop_click(View view) {
    	go = false;
    }
}
