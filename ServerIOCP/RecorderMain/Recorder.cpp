// Recorder.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include "..\RecorderServer\RecorderServer.h"
#include "..\RecorderServer\LoggerFactory.h"

//// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
//#pragma comment (lib, "Mswsock.lib")

#include <locale> 
#include <codecvt>

class InputParser {
public:
	InputParser(int &argc, char **argv) {
		for (int i = 1; i < argc; ++i)
			this->tokens.push_back(std::string(argv[i]));
	}
	const std::string& getCmdOption(const std::string &option) const {
		std::vector<std::string>::const_iterator itr;
		itr = std::find(this->tokens.begin(), this->tokens.end(), option);
		if (itr != this->tokens.end() && ++itr != this->tokens.end()) {
			return *itr;
		}
		static const std::string empty_string("");
		return empty_string;
	}
	bool cmdOptionExists(const std::string &option) const {
		return std::find(this->tokens.begin(), this->tokens.end(), option)
			!= this->tokens.end();
	}
private:
	std::vector <std::string> tokens;
};

int main(int argc, char* argv[])
{
	for (int i = 0; i<argc; i++)
		std::cout<< "Param" <<i << ":"<< argv[i] << std::endl;

	InputParser parser(argc, argv);

	// Validate the parameters
	if (argc != 13) {
		printf("usage: --hostip [HOST IP] --portFrom [PORT_FROM] --portTo [PORT_TO] --logfile [FULLPATH WITH EXTENSION] --threads [number of threads] --destFolder [DEST_FOLDER]\n");
		return 1;
	}


	RecorderServer recorderServer;
	LoggerFactory::InitFileLogger(parser.getCmdOption("--logfile")); //"d:\\Idei\\POC\\RecorderGitHub\\output\\Recorder.Log");

	int from = std::stoi(parser.getCmdOption("--portFrom"));
	int to = std::stoi(parser.getCmdOption("--portTo"));
	uint8_t threads = std::stoi(parser.getCmdOption("--threads"));
	std::string destFolder = parser.getCmdOption("--destFolder");
	std::string host = parser.getCmdOption("--hostip");

	//setup converter
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;

	std::wstring destFolderW = converter.from_bytes(destFolder);

	std::vector<int> fromTo;
	for (auto i = from; i <= to; i++)
	{
		fromTo.emplace_back(i);
	}

	try
	{
		recorderServer.StartServer(host, fromTo,threads, destFolderW);
		std::cin.ignore();
		recorderServer.StopServer();
	}
	catch (const std::exception& e)
	{
		LoggerFactory::Logger()->LogError(e, "RecordingServer failed ");
		exit(1);
	}
	catch (...)
	{
		LoggerFactory::Logger()->LogError(std::exception(), "Fatal Error ");
	}

	LoggerFactory::Logger()->LogInfo("RecordingServer stopped ");
    return 0;
}

