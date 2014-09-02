package com.lion.rmtsndcli;


import android.app.Activity;
import android.content.Intent;
import android.graphics.Color;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewConfiguration;
import android.view.View.OnTouchListener;
import android.widget.LinearLayout;

public class TouchPad extends Activity {
	private LinearLayout touchpad;
	private String message;

	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_touch_pad);
		Intent intent = getIntent();
		message = intent.getStringExtra(MainActivity.EXTRA_MESSAGE);
		touchpad = (LinearLayout) findViewById(R.id.touchpad);
		touchpad.setOnTouchListener(touchListener);
	}

	@Override
	protected void onStart() {
		NetwrkComm nwrkCls = NetwrkComm.getNetwrkCommCls();
		nwrkCls.setNetworkEvent(new NetwrkEvent() {
			@Override
			public void OnSuccessConnect() {
				touchpad.setBackgroundColor(Color.GREEN);
			}
			@Override
			public void OnFailedConnect() {
				touchpad.setBackgroundColor(Color.DKGRAY);
			}
			@Override
			public void OnSetVolume(byte b) {
				// nothing to do in here
			}
			@Override
			public void OnSetMute(boolean b) {
				// nothing to do in here
			}
		});
		nwrkCls.Connect(message);

		super.onStart();
	};

	@Override
	public void onStop(){
		//call super
		super.onStop();
	}
	
    private void sendBytes(byte[] buffer) {
		NetwrkComm nwrkCls = NetwrkComm.getNetwrkCommCls();
		nwrkCls.sendBytes(buffer);
	}
    
	private void sendMove(float dx, float dy){
    	byte buffer[] = new byte[4];
    	buffer[0] = 4;			// package size
    	buffer[1] = 7;			// RMT_MOUSE_MOVE
    	buffer[2] = (byte)dx;	// value
    	buffer[3] = (byte)dy;	// reserved
    	sendBytes(buffer);
	}

	private void sendClick(byte msBtn){
    	byte buffer[] = new byte[4];
    	buffer[0] = 4;      // package size
    	buffer[1] = 6;      // RMT_MOUSE_DOWN_UP
    	buffer[2] = msBtn;  // mouse button
    	buffer[3] = 0;      // reserved
    	sendBytes(buffer);
   	}
	
	// Mouse listeners.
	OnTouchListener touchListener = new OnTouchListener() {
		protected Action action = null;

		public boolean onTouch(View v, MotionEvent e) {
			switch(e.getAction() & MotionEvent.ACTION_MASK) {
			case MotionEvent.ACTION_POINTER_DOWN:
				if(action != null)
					if(!action.cancel(e))
						return true;
			case MotionEvent.ACTION_DOWN:
				action = null;

				// If the action is still null, it is a plain move action.
				if(action == null)
					action = new MoveAction();

				if(action != null)
					return action.onDown(e);
				return true;
			case MotionEvent.ACTION_UP:
			case MotionEvent.ACTION_POINTER_UP:
				if(action != null)
					action.onUp(e);
				action = null;
				return true;
			case MotionEvent.ACTION_MOVE:
				if(action != null)
					return action.onMove(e);
				return true;
			default:
				return false;
			}
		}
	};
	
	// Mouse actions.
	protected class Action {
		protected float downX, downY;
		protected float oldX, oldY;
		protected int pointerId;
		protected long downTime;
		protected boolean moving;
		
		public boolean isClick(MotionEvent e) {
			ViewConfiguration vc = ViewConfiguration.get(touchpad.getContext());

			int index = e.findPointerIndex(pointerId);
			return	Math.abs(e.getX(index) - downX) < vc.getScaledTouchSlop() && 
					Math.abs(e.getY(index) - downY) < vc.getScaledTouchSlop() && 
					e.getEventTime() - downTime < vc.getTapTimeout();
		}
		
		public boolean onDown(MotionEvent e) {
			pointerId = e.getPointerId(0);
			int index = e.findPointerIndex(pointerId);
			oldX = downX = e.getX(index);
			oldY = downY = e.getY(index);
			downTime = e.getEventTime();
			moving = false;
			return true;
		}
		public boolean onUp(MotionEvent e) { 
			if (isClick(e))
				onClick();
			return true;
		}
		
		public boolean acceptMove(MotionEvent e) {
			return true;
		}
		
		public boolean onMove(MotionEvent e) { 
			if(!acceptMove(e))
				return false;
				
			int index = e.findPointerIndex(pointerId);
			int Sensitivity = 1;

			float X = e.getX(index);
			float Y = e.getY(index);
			if(moving)
				onMoveDelta((X - oldX) * Sensitivity, (Y - oldY) * Sensitivity);
			else
				moving = true;
			oldX = X;
			oldY = Y;
			return true;
		}
		public boolean cancel(MotionEvent e) {
			return false;
		}
		
		public void onMoveDelta(float dx, float dy) { }
		public void onClick() { }
	};
	protected class MoveAction extends Action {
		public void onMoveDelta(float dx, float dy) 
		{
			System.out.printf("Send Move %f %f\n", dx, dy);
			sendMove(dx, dy);
		}
		public void onClick()
		{
			System.out.println("Send Click");
			/*if (button[0].isChecked()) 
				button[0].toggle();*/
			sendClick((byte)0);
		}
		
		public boolean cancel(MotionEvent e) {
			return true;
		}
	};
	
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
