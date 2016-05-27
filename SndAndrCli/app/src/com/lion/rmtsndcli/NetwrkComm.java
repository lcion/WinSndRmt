package com.lion.rmtsndcli;

import java.util.concurrent.LinkedBlockingQueue;
import android.os.Handler;

//we only need one instance of this class in the entire application
public class NetwrkComm {

	// the persistent instance
	static NetwrkComm netComPrstInst;
	private String addressStr;
	private LinkedBlockingQueue<DataUnit> recNtwrkQ;
	private LinkedBlockingQueue<DataUnit> sndNtwrkQ;
	private Handler handler;
	private Runnable runMethod;
	private UITimer timer;
	private NetwrkThread netThread;

	// the function to get the persistent instance
	static NetwrkComm getNetwrkCommCls(){
		if(netComPrstInst == null )
			netComPrstInst = new NetwrkComm();
		return netComPrstInst;
	}

	private void Connect(){
	    //create timer
	    handler = new Handler();
	    createTimerRunMethod();
	    timer = new UITimer(handler, runMethod, 1000);
        timer.start();
        recNtwrkQ = new LinkedBlockingQueue<DataUnit>();
        sndNtwrkQ = new LinkedBlockingQueue<DataUnit>();
        netThread = new NetwrkThread(recNtwrkQ, sndNtwrkQ, addressStr);
        netThread.setRunning(true);
        netThread.start();
	}
	
	public void Connect(String message) {
		if(message.equals(addressStr)){
			OnSuccessConnect();
			return;
		}
		DisConnect();
		addressStr = message;
		Connect();
	}
	
	public void DisConnect() {
		addressStr = "";
		if(timer != null)
			timer.stop();
		timer = null;
		handler = null;
		if(netThread!=null)
			netThread.setRunning(false);
		netThread = null;
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
						OnSetVolume(data.data[0]);
					else if(data.type == 2)
						OnSetMute(data.data[0] == 1);
					data = recNtwrkQ.poll();
	        	}
	        }

			private void updateConnectionStatus(byte b) {
				if(b==0){
					// failed to connect
					OnFailedToConnect();
				} else
				{
					//connected to the server
					OnSuccessConnect();
				}
			}
	    };
	}

    // network events
	private NetwrkEvent netwrkEvent;

	private void OnSuccessConnect() {
		if(netwrkEvent != null)
			netwrkEvent.OnSuccessConnect();
	}

	private void OnFailedToConnect() {
		if(netwrkEvent != null)
			netwrkEvent.OnFailedConnect();
	}
	
	private void OnSetVolume(byte b) {
		if(netwrkEvent != null)
			netwrkEvent.OnSetVolume(b);
	}

	private void OnSetMute(boolean b) {
		if(netwrkEvent != null)
			netwrkEvent.OnSetMute(b);
	}
	
	public void setNetworkEvent(NetwrkEvent netEvt){
		netwrkEvent = netEvt;
	}
	
	//send data
    public void sendBytes(byte[] buffer) {
		try {
			sndNtwrkQ.put(new DataUnit(0, buffer));
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
	}
}
