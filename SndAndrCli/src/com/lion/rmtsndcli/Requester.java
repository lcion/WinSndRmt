package com.lion.rmtsndcli;

import java.io.*;
import java.net.*;
import java.nio.charset.Charset;

import android.os.StrictMode;

public class Requester{
	Socket requestSocket;
	OutputStream sockOut;
	InputStream sockIn;
	String message;
	String hostName;
	Requester(String hostName){
		this.hostName = hostName;
		if (android.os.Build.VERSION.SDK_INT > 9) {
			StrictMode.ThreadPolicy policy = 
			        new StrictMode.ThreadPolicy.Builder().permitAll().build();
			StrictMode.setThreadPolicy(policy);
		}
	}

	public int connect(){
		try{
			//1. creating a socket to connect to the server
			requestSocket = new Socket();
			requestSocket.connect(new InetSocketAddress(hostName, 27015), 5000);
			System.out.println("Connected to localhost in port 27015");
			//2. get Input and Output streams
			sockOut = requestSocket.getOutputStream();
			sockOut.flush();
			sockIn = requestSocket.getInputStream();
		}
		catch(UnknownHostException unknownHost){
			System.err.println("You are trying to connect to an unknown host!");
			return 1;
		}
		catch(IOException ioException){
			ioException.printStackTrace();
			return 1;
		}
		System.out.println("connected");
		return 0;
	}
	
	public boolean read(int result[]){
		result[0]=0; result[2]=0;
		//String result = "Fail";
		int bytesAvailable = 0;
			if(sockIn != null){
			try {
				bytesAvailable = sockIn.available();
				if(bytesAvailable > 0){
					byte b[] = new byte[64];
					while(bytesAvailable > 52){
						sockIn.read(b, 0, 40);
						bytesAvailable -= 40;
						System.out.println("server read drop 30>");
					}
					int lastRead = sockIn.read(b);
					int pkgSize = 0;
					for(int i = 0; i < lastRead; i += pkgSize){
						pkgSize = b[0];
						if(pkgSize < 3) break;
						int function = b[1];
						switch(function){
						case 1: //RMT_VOLUME
							result[0] = 1;
							result[1] = b[2];
							break;
						case 2: //RMT_MUTE
							result[2] = 1;
							result[3] = b[2];
							break;
						}
					}
				}
			} catch (IOException e) {
				e.printStackTrace();
				System.out.println("read exception>" + e);
				return false;
			}
		}
		return true;
	}
	
	void close(){
		try{
			if(sockIn != null){
				sockIn.close();
				sockIn = null;
			}

			if(sockOut != null){
				sockOut.close();
				sockOut = null;
			}
			
			if(requestSocket != null){
				requestSocket.close();
				requestSocket = null;
			}
		}
		catch(IOException ioException){
			ioException.printStackTrace();
			System.out.println("close exception>" + ioException);
		}
	}
	
	String run()
	{
		String returnString = "Result Ok";
		try{
			//1. creating a socket to connect to the server
			requestSocket = new Socket();
			requestSocket.connect(new InetSocketAddress(hostName, 27015), 5000);
			System.out.println("Connected to localhost in port 27015");
			//2. get Input and Output streams
			sockOut = requestSocket.getOutputStream();
			sockOut.flush();
			sockIn = requestSocket.getInputStream();
			int bytesAvailable = sockIn.available();
			if(bytesAvailable > 0){
				byte b[] = new byte[100];
				sockIn.read(b);
				System.out.println("server>" + b.toString());
			}
			//3: Communicating with the server
			//do{
				//sendMessage("20u\n");
			//}while(!message.equals("20u\n"));
			bytesAvailable = sockIn.available();
			System.out.println("inpBytesAvaliable" + bytesAvailable);
		}
		catch(UnknownHostException unknownHost){
			System.err.println("You are trying to connect to an unknown host!");
			returnString = "UnknownHostException";
		}
		catch(IOException ioException){
			ioException.printStackTrace();
			returnString = "ioException1";
		}
		//4: Closing connection
		try{
			if(sockIn != null){
				sockIn.close();
				sockIn = null;
			}

			if(sockOut != null){
				sockOut.close();
				sockOut = null;
			}
			
			if(requestSocket != null){
				requestSocket.close();
				requestSocket = null;
			}
		}
		catch(IOException ioException){
			ioException.printStackTrace();
			returnString = "ioException";
		}
		System.out.println("run exit");
		return returnString;
	}
	
	public int sendBytes(byte buffer[])
	{
		try{
			sockOut.write(buffer);
			sockOut.flush();
		}
		catch(IOException ioException){
			ioException.printStackTrace();
			return 1;
		}
		return 0;
	}

	public int sendMessageObsolete(String msg)
	{
		try{
			byte b[], buffer[] = new byte[msg.length()+1];
			b = msg.getBytes();
			for(int i = 0; i < msg.length(); i++) buffer[i] = b[i];
			buffer[msg.length()] = 0;
			System.out.println("client>" + buffer);
			
			sockOut.write(buffer);
			sockOut.flush();
			System.out.println("client>" + msg);
		}
		catch(IOException ioException){
			ioException.printStackTrace();
			return 1;
		}
		return 0;
	}
}
