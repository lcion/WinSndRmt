package com.lion.rmtsndcli;

import java.util.concurrent.BlockingQueue;

public class NetwrkThread extends Thread{
	//network queue to receive messages from UI
	private BlockingQueue<DataUnit> ntwrkQueue;
	//UI queue for the network to update UI
	private BlockingQueue<DataUnit> uiQueue;
	private boolean mRun = false;
	private Requester client;
	private String ipAddress;
	
	NetwrkThread(BlockingQueue<DataUnit> ntwrkQ, BlockingQueue<DataUnit> uikQ, String msg){
		ntwrkQueue = ntwrkQ;
		uiQueue = uikQ;
		ipAddress = msg;
	}
	//The thread start
	@Override
	public void run() {
		openConnection();
		while(mRun){
			
			//read UI queue and send to network
			boolean dataProcessed = processUICommand();
			
			//read network data and write to UI thread
			dataProcessed |= processNetwrkData();
			
			//if there is work to do keep working, only sleep when idle
			if(dataProcessed) continue;
			
			//take a break to avoid spinning the processor here
			try {
				Thread.sleep(10);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		}
		closeConnection();
	}
	
	private boolean processNetwrkData() {
    	int[] result = new int[4];
    	DataUnit value = null;
    	if(client.read(result) == true){
    		for(int i = 0; i<2;i++){
	    		if(result[i*2] == 1){
	    			byte[] bValue = new byte[1];
	    			bValue[0] = (byte)result[i*2+1];
	    			value = new DataUnit(i+1, bValue);
		    		try {
		    			ntwrkQueue.put(value);
		    		} catch (InterruptedException e) {
		    			e.printStackTrace();
		    		}
	    		}
    		}
    		return true;
    	}
    	return false;
	}
	
	private void openConnection() {
		client = new Requester(ipAddress);
		byte[] data = new byte[1];
		if(client.connect() == 0) data[0] = 1;
		else data[0] = 0;
		DataUnit value = new DataUnit(0, data);
		try {
			System.out.println("DataUnit from UI send back test");
			ntwrkQueue.put(value);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
	}
	private boolean processUICommand() {
		DataUnit value = uiQueue.poll();
		if(value != null){
			client.sendBytes(value.data);
			return true;
		}
		return false;
	}
	
	private void closeConnection() {
		//cleanup the networking
		if(client != null)
			client.close();		
	}
	
	public void setRunning(boolean running) {
		mRun = running;
	}
}
