package com.lion.rmtsndcli;

import java.util.concurrent.LinkedBlockingQueue;

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
	private Handler handler;
	private TextView textView;
	private TextView textViewS;
	private SeekBar volSeekBar;
	private Button send20Btn;
	private Button touchPatBtn;
	private CheckBox muteCheckBox;
	//need to move to network class
	private UITimer timer;
	private NetwrkThread netThread;
	private LinkedBlockingQueue<DataUnit> recNtwrkQ;
	private LinkedBlockingQueue<DataUnit> sndNtwrkQ;
	private String message;
	private Runnable runMethod;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_client_layout);
		Intent intent = getIntent();
		message = intent.getStringExtra(MainActivity.EXTRA_MESSAGE);
		
	    // Create the text view
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
		    	sendBytes(buffer);
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
		//update status to failed to connect
		textViewS.setText("Connecting...");
		//disable controls
		volSeekBar.setEnabled(false);
		muteCheckBox.setEnabled(false);
		send20Btn = (Button)findViewById(R.id.send20);
		touchPatBtn = (Button)findViewById(R.id.buttonTouch);
		send20Btn.setEnabled(false);
		touchPatBtn.setEnabled(false);
	}

	private void createTimerRunMethod(){
		runMethod = new Runnable(){
	        public void run()
	        {
	            // read from network
	        	// System.out.println("timer event in my class");
	        	DataUnit data = recNtwrkQ.poll();
	        	while(data != null){
					System.out.println("DataUnit from network received");
					if(data.type == 0)
						updateConnectionStatus(data.data[0]);
					else if(data.type == 1)
						volSeekBar.setProgress(data.data[0]);
					else if(data.type == 2)
						muteCheckBox.setChecked(data.data[0] == 1);
					data = recNtwrkQ.poll();
	        	}
	        }

			private void updateConnectionStatus(byte b) {
				if(b==0){
					textViewS.setText("Failed to connect!");
				}else
				{
					textViewS.setText("Connected.");
					//disable controls
					volSeekBar.setEnabled(true);
					muteCheckBox.setEnabled(true);
					send20Btn.setEnabled(true);
					touchPatBtn.setEnabled(true);
				}
			}
	    };
	}

	@Override
	public void onStop(){
		if(timer != null)
			timer.stop();
		timer = null;
		handler = null;
		if(netThread!=null)
			netThread.setRunning(false);
		netThread = null;
		//call super
		super.onStop();
	}

	@Override
	protected void onStart() {
	    //create timer
	    handler = new Handler();
	    createTimerRunMethod();
	    timer = new UITimer(handler, runMethod, 1000);
        timer.start();
        recNtwrkQ = new LinkedBlockingQueue<DataUnit>();
        sndNtwrkQ = new LinkedBlockingQueue<DataUnit>();
        netThread = new NetwrkThread(recNtwrkQ, sndNtwrkQ, message);
        netThread.setRunning(true);
        netThread.start();
		super.onStart();
	};

	public void onMuteChkBox(View view) {
    	byte buffer[] = new byte[4];
    	buffer[0] = 4;  // package size
    	buffer[1] = 2;  // RMT_MUTE
    	buffer[2] = 0;  // value = unmute
    	buffer[3] = 0;  // reserved
    	if(muteCheckBox.isChecked())
    		buffer[2] = 1;  // value = mute

    	sendBytes(buffer);
    }

    private void sendBytes(byte[] buffer) {
    	try {
			sndNtwrkQ.put(new DataUnit(0, buffer));
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
	}

	public void onLockPcBtn(View view) {
    	byte buffer[] = new byte[4];
    	buffer[0] = 4;  // package size
    	buffer[1] = 3;  // RMT_LOCK
    	buffer[2] = 1;  // value
    	buffer[3] = 0;  // reserved
    	sendBytes(buffer);
    }

	public void onPauseBtn(View view) {
    	byte buffer[] = new byte[4];
    	buffer[0] = 4;  // package size
    	buffer[1] = 4;  // RMT_CTRLP
    	buffer[2] = 1;  // value
    	buffer[3] = 0;  // reserved
    	sendBytes(buffer);
    }

	public void onTouchPadBtn(View view) {
		//launch touch pad activity
    	Intent intent = new Intent(this, TouchPad.class);
    	intent.putExtra(MainActivity.EXTRA_MESSAGE, message);
    	startActivity(intent);
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
