package com.lion.rmtsndcli;

import android.os.Bundle;
import android.os.Handler;
import android.app.Activity;
import android.content.Intent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.TextView;

public class ClientActivity extends Activity {

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_client_layout);
		Intent intent = getIntent();
		String message = intent.getStringExtra(MainActivity.EXTRA_MESSAGE);
		
	    // Create the text view
	    //textView = new TextView(this);
	    //textView.setTextSize(40);
		textView = (TextView)findViewById(R.id.textViewStatus);
	    textView.setText(message);

	    // Set the text view as the activity layout
	    //setContentView(textView);
	    //create networking
    	//send message to the network 
		client = new Requester("192.168.1.12");
		client.connect();
		
	    //create timer
	    handler = new Handler();
	    timer = new UITimer(handler, runMethod, 1000);
        timer.start();
	}

	public Handler handler;
	
	private TextView textView;
	private UITimer timer;
	Requester client;
	private Runnable runMethod = new Runnable()
	    {
	        public void run()
	        {
	            // read from network
	        	System.out.println("timer event in my class");
	        	client.read();
	        }
	    };

	@Override
	public void onBackPressed(){
		timer.stop();
		//cleanup the networking
		client.close();
		//call super
		super.onBackPressed();
	}
	
    public void onSend20Btn(View view) {
    	client.write20u();
    }
    
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.client, menu);
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		switch (item.getItemId()) {
		//case android.R.id.home:
			// This ID represents the Home or Up button. In the case of this
			// activity, the Up button is shown. Use NavUtils to allow users
			// to navigate up one level in the application structure. For
			// more details, see the Navigation pattern on Android Design:
			//
			// http://developer.android.com/design/patterns/navigation.html#up-vs-back
			//
			//NavUtils.navigateUpFromSameTask(this);
			//return true;
		}
		return super.onOptionsItemSelected(item);
	}
}
