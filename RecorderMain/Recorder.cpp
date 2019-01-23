// Recorder.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include "..\RecorderServer\RecorderServer.h"


//// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
//#pragma comment (lib, "Mswsock.lib")
int main()
{
	RecorderServer recorderServer;
	try
	{
		recorderServer.StartServer({ "16778" },1, L"d:\\Idei\\POC\\RecorderGitHub\\output\\");
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		exit(1);
	}

	std::cin.ignore();
	recorderServer.StopServer();
    return 0;
}

