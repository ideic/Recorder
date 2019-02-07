#include "stdafx.h"
#include "RecorderServer.h"

#include <stdexcept>
#include <algorithm>
#include "ws2tcpip.h"
#include <string>
#include <functional>
#include "LoggerFactory.h"



LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HWND hWnd;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_SOCKET)
	{

		if (WSAGETSELECTERROR(lParam))

		{
			LoggerFactory::Logger()->LogWarning("Socket failed with error " + std::to_string(WSAGETSELECTERROR(lParam)));

			//FreeSocketInformation(wParam);
			return 0;
		}

		switch (WSAGETSELECTEVENT(lParam))

		{
		case FD_READ:
			{
				RecorderServer* instance = reinterpret_cast<RecorderServer*> (GetWindowLongPtr(hWnd, GWLP_USERDATA));
				auto found = std::find_if(std::begin(instance->_openPorts), std::end(instance->_openPorts), [&wParam](std::shared_ptr<SocketHandler> &socketHandler) {
					return socketHandler->Ctx->Socket == (SOCKET)wParam;
				});

				if (found == std::end(instance->_openPorts)) {
					LoggerFactory::Logger()->LogWarning("Socket not found ");
					return 0;
				}
			// Read data only if the receive buffer is empty

				auto& overlappedContext = found->operator*().Ctx;
				if (overlappedContext->ReceivedBytes > 0)
				{
					overlappedContext->ResetBuffer();
					PostMessage(hwnd, WM_SOCKET, wParam, FD_READ);
					return 0;
					instance->_fileServer->SaveData(overlappedContext->Buffer, overlappedContext->ReceivedBytes, overlappedContext->From, overlappedContext->DstIp, overlappedContext->DstPort);
					overlappedContext->ResetBuffer();
				}
				else
				{
					overlappedContext->ResetBuffer();

					int iresult = WSARecvFrom(overlappedContext->Socket, &overlappedContext->Buffer, 1, &overlappedContext->ReceivedBytes, &overlappedContext->Flags, (sockaddr*)& overlappedContext->From, &overlappedContext->FromLength, NULL, NULL);

					if (iresult != 0) {
						iresult = WSAGetLastError();
						if (iresult != WSAEWOULDBLOCK) {
							LoggerFactory::Logger()->LogWarning("RecordingServer WindowHandle 1st WSARecvFrom init failed with Code:" + iresult);
							//socket closed;
							return 0;
						}
					}

					instance->_fileServer->SaveData(overlappedContext->Buffer, overlappedContext->ReceivedBytes, overlappedContext->From, overlappedContext->DstIp, overlappedContext->DstPort);
					//overlappedContext->ResetBuffer();
				}

				return 0;
			}
		}
		
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

RecorderServer::RecorderServer():_terminate(false)
{
	hWnd = MakeWorkerWindow();
	LONG_PTR iret = SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
	if (!iret) {
		//throw std::runtime_error("SetWindowLongPtr failed with error: " + std::to_string(GetLastError()));
	}

	WSADATA wsaData;
	int iResult;
	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		throw std::runtime_error("WSAStartup failed with error: " + iResult);
	}
}

RecorderServer::~RecorderServer()
{
	//CloseHandle(_completionPort);
	WSACleanup();
}

void RecorderServer::StartServer(const std::string &host, const std::vector<int>& ports, uint8_t numberOfThreads, std::wstring workDir)
{

	_fileServer = std::make_unique<FileServer>(workDir);
	_terminate = false;

	LoggerFactory::Logger()->LogInfo("Start RecordingServer Worker threads:" + std::to_string(numberOfThreads));

	

	LoggerFactory::Logger()->LogInfo("Start FileServer Worker. Threads:" + std::to_string(numberOfThreads));

	_fileServer->StartServer(numberOfThreads);

	LoggerFactory::Logger()->LogInfo("Start Listening");

	std::for_each(ports.begin(), ports.end(), [this, &host](const int port) {
		CreatePort(host, port);
	});

	LoggerFactory::Logger()->LogInfo("RecordingServer is running");

	StartWorkers(numberOfThreads);
}

void RecorderServer::StartWorkers(uint8_t numberOfThreads)
{
	Worker();
	//for (int i = 0; i < 1; ++i) {
	//	_workers.emplace_back([&]() {Worker(); });
	//}
}

HWND RecorderServer::MakeWorkerWindow()
{
	WNDCLASS wndclass;
	CHAR *ProviderClass = "AsyncSelect";
	HWND Window;

	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = (WNDPROC)WindowProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = NULL;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = (LPCWSTR)ProviderClass;

	if (RegisterClass(&wndclass) == 0)
	{
		throw std::runtime_error("RegisterClass() failed with error: " + std::to_string(GetLastError()));

	}

	// Create a window

	if ((Window = CreateWindow(
		(LPCWSTR)ProviderClass,
		L"",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL,
		NULL,
		NULL,
		NULL)) == NULL)
	{
		throw std::runtime_error("CreateWindow() failed with error:" + std::to_string(GetLastError()));
	}

	return Window;
}

void RecorderServer::Worker() {
	DWORD ret;
	MSG msg;

	while ((!_terminate) && (ret = GetMessage(&msg, hWnd, 0, 0))) {

		if (ret == -1)

		{
			LoggerFactory::Logger()->LogWarning("GetMessage() failed with error " + std::to_string(GetLastError()));
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);

	}
}

void RecorderServer::CreatePort(const std::string &host, int port) {

	std::shared_ptr<SocketHandler>  socket = std::make_shared<SocketHandler>();

	try
	{

		socket->CreateSocket(host, port, hWnd);

		_openPorts.push_back(socket);
	}
	catch (const std::exception& e)
	{
		LoggerFactory::Logger()->LogError(e, "Create socker error on host:" + host + "port:" + std::to_string(port));
		return;
	}

	return;

	/*auto & ctx = socket->Ctx;
	ctx->ResetBuffer();

	int iresult = WSARecvFrom(ctx->Socket, &ctx->Buffer, 1, &ctx->ReceivedBytes, &ctx->Flags, (sockaddr*)& ctx->From, &ctx->FromLength, NULL, NULL);

	if (iresult != 0) {
		iresult = WSAGetLastError();
		if (iresult != WSAEWOULDBLOCK) {
			LoggerFactory::Logger()->LogWarning("RecordingServer IOCP 1st WSARecvFrom init failed with Code:" + iresult);
		}
	}*/


}

void RecorderServer::StopServer()
{
	_terminate = true;
	LoggerFactory::Logger()->LogInfo("RecordingServer Stop Workers");

	PostMessage(hWnd, WM_QUIT, NULL, NULL);

	for (auto& worker : _workers) {
		worker.join();
	};
	LoggerFactory::Logger()->LogInfo("RecordingServer Stop File Server");
	_fileServer->StopServer();

	LoggerFactory::Logger()->LogInfo("RecordingServer Clear OpenPorts");
	_openPorts.clear();

	LoggerFactory::Logger()->LogInfo("RecordingServer Clear Workers");
	_workers.clear();

}
