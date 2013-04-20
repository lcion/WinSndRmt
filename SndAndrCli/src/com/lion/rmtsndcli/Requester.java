package com.lion.rmtsndcli;

import java.io.*;
import java.net.*;

public class Requester{
	Socket requestSocket;
	OutputStream out;
	InputStream inpStream;
	String message;
	Requester(){}
	
	void run()
	{
		try{
			//1. creating a socket to connect to the server
			requestSocket = new Socket("192.168.1.12", 27015);
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
		}
		catch(IOException ioException){
			ioException.printStackTrace();
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
				requestSocket.shutdownInput();
				requestSocket.shutdownOutput();
				requestSocket.close();
				requestSocket = null;
			}
		}
		catch(IOException ioException){
			ioException.printStackTrace();
		}
		System.out.println("run exit");
	}
	
	void sendMessage(String msg)
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
		}
	}
}
