// Recorder.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include "..\RecorderServer\RecorderServer.h"
#include "..\RecorderServer\LoggerFactory.h"

//// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
//#pragma comment (lib, "Mswsock.lib")
int main()
{
	RecorderServer recorderServer;
	LoggerFactory::InitFileLogger("d:\\Idei\\POC\\RecorderGitHub\\output\\Recorder.Log");
	try
	{
		recorderServer.StartServer({ "16778" },1, L"d:\\Idei\\POC\\RecorderGitHub\\output\\");
	}
	catch (const std::exception& e)
	{
		LoggerFactory::Logger()->LogError(e, "RecordingServer failed ");
		exit(1);
	}

	std::cin.ignore();
	recorderServer.StopServer();

	LoggerFactory::Logger()->LogInfo("RecordingServer stopped ");
    return 0;
}

