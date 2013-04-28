package com.lion.rmtsndcli;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import android.os.Bundle;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.view.Menu;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.ListView;

public class MainActivity extends Activity {

	public final static String EXTRA_MESSAGE = "com.lion.rmtsndcli.MESSAGE";
	private ListView listView;
	private ArrayList<String> listNames;
	private ArrayList<String> listIps;
	private StableArrayAdapter adapter;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
		listView = (ListView)findViewById(R.id.listView1);
		String[] namesValues = new String[] { "Luci PC", "Alex PC", "Andrei PC"};
		String[] ipValues = new String[] { "192.168.1.12", "192.168.1.13", "192.168.1.11"};
		listNames = new ArrayList<String>();
	    for (int i = 0; i < namesValues.length; ++i) {
	      listNames.add(namesValues[i]);
	    }
	    listIps = new ArrayList<String>();
	    for (int i = 0; i < ipValues.length; ++i) {
	    	listIps.add(ipValues[i]);
		    }
	    
	    adapter = new StableArrayAdapter(this,
	        android.R.layout.simple_list_item_1, listNames);
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
		        String item = (String) parent.getItemAtPosition(position);
		    	listNames.remove(item);
				listIps.remove(position);
		    	adapter.updateMap(listNames);
	            adapter.notifyDataSetChanged();
				return true;
			}
		});
    }

    private class StableArrayAdapter extends ArrayAdapter<String> {

        HashMap<String, Integer> mIdMap = new HashMap<String, Integer>();

        public StableArrayAdapter(Context context, int textViewResourceId,
            List<String> objects) {
          super(context, textViewResourceId, objects);
          for (int i = 0; i < objects.size(); ++i) {
            mIdMap.put(objects.get(i), i);
          }
        }

        @Override
        public long getItemId(int position) {
          String item = getItem(position);
          return mIdMap.get(item);
        }

        public void updateMap(List<String> objects){
        	mIdMap.clear();
            for (int i = 0; i < objects.size(); ++i) {
                mIdMap.put(objects.get(i), i);
              }
        }

        @Override
        public boolean hasStableIds() {
          return true;
        }

      }
    /** Called when the user clicks the Send button */
    public void onAddNewPCBtn(View view) {
    	getNameIpFromUser();
    }
    
    private int getNameIpFromUser(){
    	AlertDialog.Builder alert = new AlertDialog.Builder(this);

    	alert.setTitle("Add new PC");
    	alert.setMessage("IP Address:");

    	// Set an EditText view to get user input 
    	final EditText input = new EditText(this);
    	alert.setView(input);

    	alert.setPositiveButton("Ok", new DialogInterface.OnClickListener() {
    	public void onClick(DialogInterface dialog, int whichButton) {
    		String value = input.getText().toString();
    	  	// Do something with value!
	      	listNames.add(value);
	      	listIps.add(value);
	      	adapter.updateMap(listNames);
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
