package com.lion.rmtsndcli;

import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.nio.charset.Charset;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import android.os.Bundle;
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
	private ArrayList<String> listNames;
	private ArrayList<String> listIps;
	private SimpleAdapter adapter;
	private List<Map<String, String>> data;
	MainActivity mainActPtr;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
		listView = (ListView)findViewById(R.id.listView1);
		listNames = new ArrayList<String>();
	    listIps = new ArrayList<String>();
	    mainActPtr = this;

	    readDataFromFile();
	    
	    data = new ArrayList<Map<String, String>>();
	    int i = 0;
	    for (String itemName : listNames) {
	        Map<String, String> datum = new HashMap<String, String>(2);
	        datum.put("name", itemName);
	        datum.put("ip", listIps.get(i++));
	        data.add(datum);
	    }
	    adapter = new SimpleAdapter(this, data, android.R.layout.simple_list_item_2,
                new String[] {"name", "ip"},
                new int[] {android.R.id.text1,
                           android.R.id.text2});
	    listView.setAdapter(adapter);

	    listView.setOnItemClickListener(new AdapterView.OnItemClickListener() {

	      @Override
	      public void onItemClick(AdapterView<?> parent, final View view,
	          int position, long id) {
	        String message = listIps.get(position);
	        startClientActivity(message);
	      }
	    });
	    listView.setOnItemLongClickListener(new AdapterView.OnItemLongClickListener() {

			@Override
			public boolean onItemLongClick(AdapterView<?> parent, View view,
					int position, long id) {
				
				final int positionInList = position;
		    	AlertDialog.Builder alert = new AlertDialog.Builder(mainActPtr);

		    	alert.setTitle("Delete");
		    	Object item = parent.getItemAtPosition(positionInList);
		    	if(item instanceof String )
		    		alert.setMessage((String)item);

		    	alert.setPositiveButton("Ok", new DialogInterface.OnClickListener() {
		    	public void onClick(DialogInterface dialog, int whichButton) {
			    	listNames.remove(positionInList);
					listIps.remove(positionInList);
					writeDataToFile();
					data.remove(positionInList);
		            adapter.notifyDataSetChanged();
		    	  }
		    	});
		    	
		    	alert.setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
		      	  public void onClick(DialogInterface dialog, int whichButton) {
		      	    // Canceled.
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
    		listNames.add(name);
    		String ip = string.substring(sep+1, eol);
    		listIps.add(ip);
    		string = string.substring(eol+1);
    	}
	}

    /** Called when the user clicks the Send button */
    public void onAddNewPCBtn(View view) {
    	getNameIpFromUser();
    }
    
    private int getNameIpFromUser(){
    	AlertDialog.Builder alert = new AlertDialog.Builder(this);

    	alert.setTitle("Add new PC");
    	alert.setMessage("Name, IP Address:");

    	// Set an EditText view to get user input 
    	final LinearLayout linlay = new LinearLayout(this);
    	linlay.setOrientation(1); //vertical
    	final EditText nameInp = new EditText(this);
    	nameInp.setSingleLine();
    	final EditText adrInp = new EditText(this);
    	adrInp.setSingleLine();
    	linlay.addView(nameInp);
    	linlay.addView(adrInp);
    	alert.setView(linlay);

    	alert.setPositiveButton("Ok", new DialogInterface.OnClickListener() {
    	public void onClick(DialogInterface dialog, int whichButton) {
    		String namesValue = nameInp.getText().toString();
    		String ipValue = adrInp.getText().toString();
    	  	// Do something with value!
	      	listNames.add(namesValue);
	      	listIps.add(ipValue);
	      	writeDataToFile();
	      	//update the ui list
	      	Map<String, String> datum = new HashMap<String, String>(2);
	        datum.put("name", namesValue);
	        datum.put("ip", ipValue);
	        data.add(datum);
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
    	for(int i = 0; i < listNames.size() ; i++)
    	{
    		string += listNames.get(i);
    		string += ",";
    		string += listIps.get(i);
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
}
