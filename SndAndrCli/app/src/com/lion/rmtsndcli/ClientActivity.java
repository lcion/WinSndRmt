package com.lion.rmtsndcli;

import android.os.Bundle;
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
	private TextView textView;
	private TextView textViewS;
	private SeekBar volSeekBar;
	private Button send20Btn;
	private Button touchPatBtn;
	private Button pauseBtn;
	private CheckBox muteCheckBox;
	private String message;
	private NetwrkComm nwrkCls;

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
		
		//get references to the UI controls
		muteCheckBox = (CheckBox)findViewById(R.id.checkBoxMute);
		volSeekBar = (SeekBar)findViewById(R.id.seekBarVol);
		send20Btn = (Button)findViewById(R.id.send20);
		touchPatBtn = (Button)findViewById(R.id.buttonTouch);
		pauseBtn = (Button)findViewById(R.id.btnPause);
		
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
		send20Btn.setEnabled(false);
		touchPatBtn.setEnabled(false);
		pauseBtn.setEnabled(false);
	}

	@Override
	protected void onStart() {
	    //create timer
		nwrkCls = NetwrkComm.getNetwrkCommCls();
		nwrkCls.setNetworkEvent(new NetwrkEvent() {
			@Override
			public void OnSuccessConnect() {
				textViewS.setText("Connected.");
				//enable controls
				volSeekBar.setEnabled(true);
				muteCheckBox.setEnabled(true);
				send20Btn.setEnabled(true);
				touchPatBtn.setEnabled(true);
				pauseBtn.setEnabled(true);
			}
			@Override
			public void OnFailedConnect() {
				textViewS.setText("Failed to connect!");
				nwrkCls.DisConnect();
			}
			@Override
			public void OnSetVolume(byte b) {
				volSeekBar.setProgress(b);
			}
			@Override
			public void OnSetMute(boolean b) {
				muteCheckBox.setChecked(b);
			}
		});
		nwrkCls.Connect(message);
		
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
		nwrkCls.sendBytes(buffer);
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
    	Intent intent = new Intent(this, TouchPadActivity.class);
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
