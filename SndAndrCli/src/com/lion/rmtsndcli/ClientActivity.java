package com.lion.rmtsndcli;

import android.os.Bundle;
import android.os.Handler;
import android.app.Activity;
import android.content.Intent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
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

		textViewS = (TextView)findViewById(R.id.textViewAStatus);
		
		muteCheckBox = (CheckBox)findViewById(R.id.checkBoxMute);
		volSeekBar = (SeekBar)findViewById(R.id.seekBarVol);
		volSeekBar.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {
			
			@Override
			public void onStopTrackingTouch(SeekBar seekBar) {
				// get the position, get the mute, send message to server
				int prog = volSeekBar.getProgress();
		    	byte buffer[] = new byte[4];
		    	buffer[0] = 4;			// package size
		    	buffer[1] = 1;			// RMT_VOLUME
		    	buffer[2] = (byte) prog;// value
		    	buffer[3] = 0;			// reserved
		    	client.sendBytes(buffer);
			}
			
			@Override
			public void onStartTrackingTouch(SeekBar seekBar) {
				// I have no interest at the moment to process this
			}
			
			@Override
			public void onProgressChanged(SeekBar seekBar, int progress,
					boolean fromUser) {
				// I have no interest at the moment to process this
			}
		});

	    // Set the text view as the activity layout
	    //setContentView(textView);
	    //create networking
    	//send message to the network 
		client = new Requester(message);
		if(client.connect()!=0){
			//update status to failed to connect
			textViewS.setText("Failed to connect!");
			//disable controls
			volSeekBar.setEnabled(false);
			muteCheckBox.setEnabled(false);
			Button send20Btn = (Button)findViewById(R.id.send20);
			send20Btn.setEnabled(false);
		}else{
		    //create timer
		    handler = new Handler();
		    timer = new UITimer(handler, runMethod, 1000);
	        timer.start();
		}
	}

	public Handler handler;
	
	private TextView textView;
	private TextView textViewS;
	private SeekBar volSeekBar;
	private CheckBox muteCheckBox;
	private UITimer timer;
	Requester client;
	private Runnable runMethod = new Runnable()
	    {
	        public void run()
	        {
	            // read from network
	        	//System.out.println("timer event in my class");
	        	int result[] = new int[4];
	        	if(client.read(result) == true){
	        		updateUIFromServer(result);
	        		if(!textViewS.getText().equals("Connected."))
	        			textViewS.setText("Connected.");
	        	}
	        }

			private void updateUIFromServer(int result[]) {
				if(result[0] != 0)
					volSeekBar.setProgress(result[1]);
				if(result[2] != 0)
					muteCheckBox.setChecked(result[3] == 1);
			}
	    };
	
	@Override
	public void onStop(){
		if(timer != null)
		timer.stop();
		//cleanup the networking
		if(client != null)
		client.close();
		//call super
		super.onStop();
	}
	
	public void onMuteChkBox(View view) {
    	byte buffer[] = new byte[4];
    	buffer[0] = 4;  // package size
    	buffer[1] = 2;  // RMT_MUTE
    	buffer[2] = 0;  // value = unmute
    	buffer[3] = 0;  // reserved
    	if(muteCheckBox.isChecked())
    		buffer[2] = 1;  // value = mute
    		
    	client.sendBytes(buffer);
    }
	
    public void onSend20Btn(View view) {
    	byte buffer[] = new byte[4];
    	buffer[0] = 4;  // package size
    	buffer[1] = 3;  // RMT_LOCK
    	buffer[2] = 1;  // value
    	buffer[3] = 0;  // reserved
    	client.sendBytes(buffer);
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
