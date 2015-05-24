package com.lion.rmtsndcli;

import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketTimeoutException;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;
import java.nio.charset.Charset;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import android.net.DhcpInfo;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.os.StrictMode;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.view.Menu;
import android.view.View;
import android.widget.AdapterView;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.SimpleAdapter;

public class MainActivity extends Activity {

	public final static String EXTRA_MESSAGE = "com.lion.rmtsndcli.MESSAGE";
	private ListView listView;
	private SimpleAdapter adapter;
	private List<Map<String, String>> data;
	private MainActivity mainActPtr;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
		listView = (ListView)findViewById(R.id.pcListView);
	    data = new ArrayList<Map<String, String>>();
	    mainActPtr = this;

	    //read saved data from file into data
	    readDataFromFile();
	    
	    adapter = new SimpleAdapter(this, data, android.R.layout.simple_list_item_2,
                new String[] {"name", "ip"},
                new int[] {android.R.id.text1,
                           android.R.id.text2});
	    listView.setAdapter(adapter);

	    listView.setOnItemClickListener(new AdapterView.OnItemClickListener() {

	      @Override
	      public void onItemClick(AdapterView<?> parent, final View view, int position, long id) {

	    	Map<String, String> datum = data.get(position);
	    	String message = datum.get("ip");
	        startClientActivity(message);
	      }
	    });
	    listView.setOnItemLongClickListener(new AdapterView.OnItemLongClickListener() {

			@Override
			public boolean onItemLongClick(AdapterView<?> parent, View view,
					int position, long id) {
				
				final int positionInList = position;
		    	AlertDialog.Builder alert = new AlertDialog.Builder(mainActPtr);

		    	alert.setTitle("Modify");
		    	final Object item = parent.getItemAtPosition(positionInList);
		    	if(item instanceof HashMap){
		    		HashMap<String, String> datum = (HashMap<String,String>)item;
		    		alert.setMessage(datum.get("name"));
		    	}

		    	alert.setPositiveButton("Delete", new DialogInterface.OnClickListener() {
		    	public void onClick(DialogInterface dialog, int whichButton) {
					data.remove(positionInList);
					writeDataToFile();
		            adapter.notifyDataSetChanged();
		    	  }
		    	});
		    	
		    	alert.setNegativeButton("Edit", new DialogInterface.OnClickListener() {
		      	  public void onClick(DialogInterface dialog, int whichButton) {
		      	    // edit.
					data.remove(positionInList);
			    	HashMap<String, String> datum = (HashMap<String,String>)item;
		      		String name = datum.get("name");
		      		String ip = datum.get("ip");
		      		getNameIpFromUser(name, ip);
		      	  }
		      	});

		    	alert.show();
								
				return true;
			}
		});
    }

    private void readDataFromFile() {
		// write to "addrList.txt";
    	String filename = "addrList.txt";
    	String string = "";
    	FileInputStream inputStream;

    	try {
    		inputStream = openFileInput(filename);
    		//inputStream.write(string.getBytes());
    		byte buffer[] = new byte[64];
    		int bytesAvailable = 0;
    		while((bytesAvailable = inputStream.read(buffer)) > 0){
    			string += new String(buffer, 0, bytesAvailable, Charset.defaultCharset());
    		}
    		inputStream.close();
    	} catch (Exception e) {
    	  e.printStackTrace();
    	}
    	//process string
    	while(string.length() > 0){
    		//find first pc
    		int sep = string.indexOf(',');
    		if(sep<0)break;
    		int eol = string.indexOf('\n');
    		if(eol<0 || eol<sep)break;
    		String name = string.substring(0, sep);
    		String ip = string.substring(sep+1, eol);
    		Map<String, String> datum = new HashMap<String, String>(2);
    		datum.put("name", name);
    		datum.put("ip", ip);
    		data.add(datum);
    		string = string.substring(eol+1);
    	}
	}

    /** Called when the user clicks the "Add New PC" button */
    public void onAddNewPCBtn(View view) {
    	String ip = "";
    	StringBuilder pcName = new StringBuilder("");
    	try {
    		ip = detectServer(pcName);
		} catch (Exception e) {
			e.printStackTrace();
		}
    	getNameIpFromUser(pcName.toString(), ip);
    }
    
	// Get broadcast address for LAN.
	private InetAddress getBroadcastAddress()  {
		WifiManager wifi = (WifiManager) getSystemService(Context.WIFI_SERVICE);
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

    private String detectServer(StringBuilder pcName) throws Exception{
		// Broadcast ping to look for servers.
    	int Timeout = 1500;
    	int Port = 27015;
		DatagramSocket beacon = new DatagramSocket(null);
		beacon.setBroadcast(true);
		beacon.setSoTimeout(Timeout);

		if (android.os.Build.VERSION.SDK_INT > 9) {
			StrictMode.ThreadPolicy policy = new StrictMode.ThreadPolicy.Builder().permitAll().build();
			StrictMode.setThreadPolicy(policy); 
		}
		InetAddress broadcast = getBroadcastAddress();

		byte[] buffer = new byte[] { 0x04, 0x08, 0x00, 0x00 };
		beacon.send(new DatagramPacket(buffer, 4, broadcast, Port));

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
			    		return addr;
			    	}
				}
			}
		} catch (SocketTimeoutException e) { }
		return "";
    }
    
    private int getNameIpFromUser(String name, String ip){
    	AlertDialog.Builder alert = new AlertDialog.Builder(this);

    	alert.setTitle("Add new PC");
    	alert.setMessage("Name, IP Address:");

    	// Set an EditText view to get user input 
    	final LinearLayout linlay = new LinearLayout(this);
    	linlay.setOrientation(1); //vertical
    	final EditText nameInp = new EditText(this);
    	nameInp.setSingleLine();
    	nameInp.setText(name);
    	final EditText adrInp = new EditText(this);
    	adrInp.setSingleLine();
    	adrInp.setText(ip);
    	linlay.addView(nameInp);
    	linlay.addView(adrInp);
    	alert.setView(linlay);

    	alert.setPositiveButton("Ok", new DialogInterface.OnClickListener() {
    	public void onClick(DialogInterface dialog, int whichButton) {
    		String namesValue = nameInp.getText().toString();
    		String ipValue = adrInp.getText().toString();
    	  	// Do something with value!
	      	// update the ui list
	      	Map<String, String> datum = new HashMap<String, String>(2);
	        datum.put("name", namesValue);
	        datum.put("ip", ipValue);
	        data.add(datum);
	        // save updated data to file
	      	writeDataToFile();
	      	adapter.notifyDataSetChanged();
    	  }
    	});

    	alert.setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
    	  public void onClick(DialogInterface dialog, int whichButton) {
    	    // Canceled.
    	  }
    	});

    	alert.show();
    	return 0;
    }
    
    protected void writeDataToFile() {
		// write to "addrList.txt";
    	String filename = "addrList.txt";
    	String string = "";
    	FileOutputStream outputStream;
    	for(int i = 0; i < data.size() ; i++)
    	{
    		string += data.get(i).get("name");
    		string += ",";
    		string += data.get(i).get("ip");
    		string += "\n";
    	}

    	try {
    	  outputStream = openFileOutput(filename, Context.MODE_PRIVATE);
    	  outputStream.write(string.getBytes());
    	  outputStream.close();
    	} catch (Exception e) {
    	  e.printStackTrace();
    	}
	}

	public void startClientActivity(String message) {
        // create new activity
    	Intent intent = new Intent(this, ClientActivity.class);
    	intent.putExtra(EXTRA_MESSAGE, message);
    	startActivity(intent);
    }
        
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }
    
	@Override
	protected void onStart() {
		NetwrkComm nwrkCls = NetwrkComm.getNetwrkCommCls();
		nwrkCls.DisConnect();
		super.onStart();
	};

}
