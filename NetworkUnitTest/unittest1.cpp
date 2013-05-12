#include "stdafx.h"
#include "CppUnitTest.h"
#include "../SndWinSrv/srvApp.h"
#include "../SndWinSrv/srvNetwrk.h"
#include "../SndWinSrv/srvAudio.h"
#include "../SndWinCli/cliNetwork.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace NetworkUnitTest
{		
	TEST_CLASS(UnitTest1)
	{
	public:

		//all tests
		TEST_METHOD(TestCreateServer)
		{
			//server classes
			CSrvApp myApp;
			CSrvAudio myAudio;
			CSrvNetwrk mySrvNetwork(myAudio, myApp);

			Assert::AreEqual(S_OK, mySrvNetwork.Initialize("127.0.0.1", 27321));
			Logger::WriteMessage("In TestCreateServer");

		}

		TEST_METHOD(TestCreateClient)
		{
			//client classes
			CCliNetwork myCliNetwork;

			Assert::AreEqual(0,  myCliNetwork.Initialize());
			Logger::WriteMessage("In TestCreateClient");

		}

		TEST_METHOD(TestConnectClientAcceptServer)
		{
			//server classes
			CSrvApp myApp;
			CSrvAudio myAudio;
			CSrvNetwrk mySrvNetwork(myAudio, myApp);
			//client classes
			CCliNetwork myCliNetwork;

			Assert::AreEqual(S_OK, mySrvNetwork.Initialize("127.0.0.1", 27321));

			Assert::AreEqual(0,  myCliNetwork.Initialize());

			Assert::AreEqual(0,  myCliNetwork.Connect("127.0.0.1", 27321));

			Assert::AreEqual(S_OK, mySrvNetwork.Accept());
			Logger::WriteMessage("In TestConnectClientAcceptServer");
		}

		TEST_METHOD(TestComunicationData)
		{
			//server classes
			CSrvApp myApp;
			CSrvAudio myAudio;
			CSrvNetwrk mySrvNetwork(myAudio, myApp);
			//client classes
			CCliNetwork myCliNetwork;

			Assert::AreEqual(S_OK, mySrvNetwork.Initialize("127.0.0.1", 27321));

			Assert::AreEqual(0,  myCliNetwork.Initialize());

			Assert::AreEqual(0,  myCliNetwork.Connect("127.0.0.1", 27321));

			Assert::AreEqual(S_OK, mySrvNetwork.Accept());

			WSAEVENT EventArray[WSA_MAXIMUM_WAIT_EVENTS];
			DWORD EventTotal = 0;
			Assert::AreEqual(S_OK, mySrvNetwork.CreateEvents(EventArray, EventTotal));

			Logger::WriteMessage("In TestComunicationData");
		}
	};
}
