package com.lion.rmtsndcli;

import java.io.*;
import java.net.*;
import java.nio.charset.Charset;

public class Requester{
	Socket requestSocket;
	OutputStream out;
	InputStream inpStream;
	String message;
	String hostName;
	Requester(String hostName){
		this.hostName = hostName;
	}

	public int connect(){
		try{
			//1. creating a socket to connect to the server
			requestSocket = new Socket();
			requestSocket.connect(new InetSocketAddress(hostName, 27015), 5000);
			System.out.println("Connected to localhost in port 27015");
			//2. get Input and Output streams
			out = requestSocket.getOutputStream();
			out.flush();
			inpStream = requestSocket.getInputStream();
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
	
	public String read(){
		String result = "Fail";
		int bytesAvailable = 0;
			if(inpStream != null){
			try {
				bytesAvailable = inpStream.available();
				if(bytesAvailable > 0){
					byte b[] = new byte[64];
					while(bytesAvailable > 40){
						inpStream.read(b, 0, 30);
						bytesAvailable -= 30;
						System.out.println("server read drop 30>");
					}
					inpStream.read(b);
					String receivedBytes = new String(b, 0, bytesAvailable, Charset.defaultCharset());
					System.out.println("server> " + receivedBytes + " A" + bytesAvailable);
					if(receivedBytes.length() > 2)
						result = receivedBytes.substring(0, receivedBytes.length()-2);
				}
			} catch (IOException e) {
				e.printStackTrace();
				System.out.println("read exception>" + e);
			}
		}
		return result;
	}
	
	void close(){
		try{
			if(inpStream != null){
				inpStream.close();
				inpStream = null;
			}

			if(out != null){
				out.close();
				out = null;
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
			out = requestSocket.getOutputStream();
			out.flush();
			inpStream = requestSocket.getInputStream();
			int bytesAvailable = inpStream.available();
			if(bytesAvailable > 0){
				byte b[] = new byte[100];
				inpStream.read(b);
				System.out.println("server>" + b.toString());
			}
			//3: Communicating with the server
			//do{
				sendMessage("20u\n");
			//}while(!message.equals("20u\n"));
			bytesAvailable = inpStream.available();
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
			if(inpStream != null){
				inpStream.close();
				inpStream = null;
			}

			if(out != null){
				out.close();
				out = null;
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
	
	public int sendMessage(String msg)
	{
		try{
			byte b[], buffer[] = new byte[msg.length()+1];
			b = msg.getBytes();
			for(int i = 0; i < msg.length(); i++) buffer[i] = b[i];
			buffer[msg.length()] = 0;
			System.out.println("client>" + buffer);
			
			out.write(buffer);
			out.flush();
			System.out.println("client>" + msg);
		}
		catch(IOException ioException){
			ioException.printStackTrace();
			return 1;
		}
		return 0;
	}
}
