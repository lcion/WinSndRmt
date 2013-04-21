package com.lion.rmtsndcli;

import android.os.Bundle;
import android.app.Activity;
import android.content.Intent;
import android.view.Menu;
import android.view.View;
import android.widget.TextView;

public class MainActivity extends Activity {

	public final static String EXTRA_MESSAGE = "com.lion.rmtsndcli.MESSAGE";
	
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
    }

    /** Called when the user clicks the Send button */
    public void onLuciBtn(View view) {
        // create new activity
    	Intent intent = new Intent(this, ClientActivity.class);
    	String message = "192.168.1.12";
    	intent.putExtra(EXTRA_MESSAGE, message);
    	startActivity(intent);
    }
    
    public void onAlexBtn(View view) {
        // create new activity
    	Intent intent = new Intent(this, ClientActivity.class);
    	String message = "192.168.1.13";
    	intent.putExtra(EXTRA_MESSAGE, message);
    	startActivity(intent);
    }
    
    public void onAndreiBtn(View view) {
        // create new activity
    	Intent intent = new Intent(this, ClientActivity.class);
    	String message = "192.168.1.11";
    	intent.putExtra(EXTRA_MESSAGE, message);
    	startActivity(intent);
    }
    
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }
    
}
