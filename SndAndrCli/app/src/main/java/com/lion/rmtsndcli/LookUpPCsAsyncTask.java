package com.lion.rmtsndcli;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;
import java.net.SocketTimeoutException;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;
import java.util.List;
import java.util.Map;

import android.app.Activity;
import android.app.ProgressDialog;
import android.net.DhcpInfo;
import android.net.wifi.WifiManager;
import android.os.AsyncTask;

public class LookUpPCsAsyncTask extends AsyncTask<Integer, Void, String>{
	private WifiManager wifi;
	private List<Map<String, String>> data;
	private StringBuilder pcName;
	private StringBuilder ipAddr;
	private Activity activity;
    private ProgressDialog progressDialog;
    private Callback callback;

	public void SetupInternals(WifiManager w, List<Map<String, String>> d, StringBuilder pcN,
			StringBuilder ipA, Activity act, Callback cbk){
		wifi = w;
		data = d;
		pcName = pcN;
		ipAddr = ipA;
		activity = act;
		callback = cbk;
	}
	
    @Override
    protected void onPreExecute() {
        progressDialog = ProgressDialog.show(activity, "", activity.getString(R.string.netwrk_discovery));
    }
	
	// Get broadcast address for LAN.
	private InetAddress getBroadcastAddress()  {
		// WifiManager wifi = (WifiManager) getSystemService(Context.WIFI_SERVICE);
		DhcpInfo dhcp = wifi.getDhcpInfo();
		// handle null, should probably ask user for local network...
		int broadcast = (dhcp.ipAddress & dhcp.netmask) | ~dhcp.netmask;
		if(dhcp.ipAddress == 0 && dhcp.netmask == 0)
			broadcast = 0xFF00A8C0; //192.168.0.255
		else if(dhcp.ipAddress != 0 && dhcp.netmask != 0)
			broadcast = (dhcp.ipAddress & dhcp.netmask) | ~dhcp.netmask;
		else if(dhcp.ipAddress != 0)
			broadcast = (dhcp.ipAddress & 0x00FFFFFF) | 0xFF000000;
		else
			broadcast = 0xFF00A8C0; //192.168.0.255 - we only have netmask

		byte[] quads = new byte[4];
		for (int k = 0; k < 4; k++)
			quads[k] = (byte) ((broadcast >> k * 8) & 0xFF);
		try {
			return InetAddress.getByAddress(quads);
		} catch (UnknownHostException e) {
			e.printStackTrace();
		}
		return null;
	}

	@Override
	protected String doInBackground(Integer... params) {
		// Broadcast ping to look for servers.
    	int Timeout = 1500;
    	int Port = 27015;
		DatagramSocket beacon;
		try {
			beacon = new DatagramSocket(null);
			beacon.setBroadcast(true);
			beacon.setSoTimeout(Timeout);
		} catch (SocketException e1) {
			e1.printStackTrace();
			return null;
		}

		InetAddress broadcast = getBroadcastAddress();

		byte[] buffer = new byte[] { 0x04, 0x08, 0x00, 0x00 };
		try {
			beacon.send(new DatagramPacket(buffer, 4, broadcast, Port));
		} catch (IOException e1) {
			e1.printStackTrace();
			return null;
		}

		try {
			// Add each ack to the menu. receive up to 9
			for (int i = 0; i < 9; ++i) {
				byte[] port = new byte[256];
				DatagramPacket ack = new DatagramPacket(port, 256);
				beacon.receive(ack);

				ByteBuffer parser = ByteBuffer.wrap(port);

				if (parser.get(1) == 0x09)
				{
					String addr = ack.getAddress().toString().substring(1);
					//System.out.println("got back udp from addr " + addr);
					String string;
					// find the address in existing map
			    	boolean found=false;
			    	for(int j = 0; j < data.size() ; j++)
			    	{
			    		string = data.get(j).get("ip");
			    		if(string.equals(addr)){
			    			found = true;
			    			break; // IP already in the list
			    		}
			    	}
			    	if(found == false){
			    		if(ack.getLength() > 4){
			    			//we have received PC name
			    			string = "";
			    			char nextChar;
			    			int j = 0;
			    			do{
			    				j++;
			    				nextChar = (char)parser.get();
			    				if(j>4 && nextChar != 0x0)
			    					string += nextChar;
			    			}while((j<5 || nextChar != 0x0) && j<(256-4));
			    			System.out.println(string);
			    			pcName.append(string);
			    		}
		    			ipAddr.append(addr);
			    		return addr;
			    	}
				}
			}
		} catch (SocketTimeoutException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}
		return null;
	}
    @Override
    protected void onPostExecute(String result) {
        progressDialog.dismiss();
        callback.execute();
    }
}
