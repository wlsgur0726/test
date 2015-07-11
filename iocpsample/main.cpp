#include <stdio.h>
#include <winsock2.h>
#include <windows.h>
#include <stdint.h>

#include <memory>
#include <thread>
#include <conio.h>

const char* IP = "127.0.0.1";
const int Port = 12345;
const int ThreadCount = 4;

void Error(const char* msg = "error") {
	printf("%s\n", msg);
	exit(1);
}


WSADATA wsaData;
SOCKET g_listeningSocket;
SOCKADDR_IN g_serverAddr;
HANDLE g_iocpHandle;
volatile bool g_run = true;

struct ClientInfo {
	struct IoObject : OVERLAPPED {
		const static int BufferSize = 512;
		char m_buffer[BufferSize];
		IoObject() {
			Init();
		}
		void Init() {
			memset(this, 0, sizeof(IoObject));
		}
	};

	SOCKET m_socket = INVALID_SOCKET;
	SOCKADDR_IN m_address;
	IoObject m_readBuffer;
	IoObject m_writeBuffer;

	ClientInfo() {
		memset(&m_address, 0, sizeof(m_address));
	}
	~ClientInfo() {
		if ( m_socket!=INVALID_SOCKET )
			closesocket(m_socket);
	}

	bool Recv() {
		DWORD recvlen = 0;
		DWORD flag = 0;
		WSABUF buf;
		buf.buf = m_readBuffer.m_buffer;
		buf.len = IoObject::BufferSize;

		do {
			auto r = WSARecv(
				m_socket, 
				&buf, 
				1, 
				&recvlen, 
				&flag, 
				&m_readBuffer, 
				NULL );
			if ( r == SOCKET_ERROR ) {
				int err = WSAGetLastError();
				if ( err == WSAEINTR )
					continue;
				else if ( err!=WSAEWOULDBLOCK && err!=WSA_IO_PENDING )
					Error();
			}
			break;
		} while ( true );
		return true;
	}

	bool Send(int len) {
		WSABUF buf;
		buf.buf = m_writeBuffer.m_buffer;
		buf.len = len;

		auto r = WSASend(
			m_socket, 
			&buf, 
			1, 
			NULL, 
			0, 
			&m_writeBuffer, 
			NULL );
		if ( r == SOCKET_ERROR ) {
			int err = WSAGetLastError();
			if ( err != WSA_IO_PENDING ) {
				Error();
			}
		}
		return true;
	}
};

void ListenerThread() {
	while ( g_run ) {
		ClientInfo* clientInfo = new ClientInfo;
		int addrlen = sizeof(clientInfo->m_address);

		clientInfo->m_socket = accept(
			g_listeningSocket, 
			(SOCKADDR*)&clientInfo->m_address, 
			&addrlen );

		if ( clientInfo->m_socket == INVALID_SOCKET ) {
			delete clientInfo;
			break;
		}

		auto r = CreateIoCompletionPort(
			(HANDLE)clientInfo->m_socket,
			g_iocpHandle,
			(ULONG_PTR)clientInfo,
			0 );
		if ( r == NULL ) {
			delete clientInfo;
			Error();
		}

		clientInfo->Recv();
	}
}

void WorkerThread() {
	while ( g_run ) {
		DWORD transLength = 0;
		ClientInfo* clientInfo = nullptr;
		OVERLAPPED* ov = nullptr;
		auto r = GetQueuedCompletionStatus(
			g_iocpHandle, 
			&transLength, 
			(PULONG_PTR)&clientInfo, 
			&ov, 
			100 );
		if ( r == FALSE ) {
			DWORD err = GetLastError();
			if ( err == WAIT_TIMEOUT )
				continue;
			Error();
		}

		if ( transLength <= 0 ) {
			DWORD t = 0;
			DWORD f = 0;
			
			BOOL result = WSAGetOverlappedResult(clientInfo->m_socket, ov, &t, FALSE, &f);
			int err = WSAGetLastError();

			delete clientInfo;
			continue;
		}

		bool isReadCompletion = (OVERLAPPED*)&clientInfo->m_readBuffer == ov;
		bool isWriteCompletion = (OVERLAPPED*)&clientInfo->m_writeBuffer == ov;
		if ( isReadCompletion ) {
			printf("recv : %s\n", clientInfo->m_readBuffer.m_buffer);
			clientInfo->m_writeBuffer.Init();
			memcpy(
				clientInfo->m_writeBuffer.m_buffer,
				clientInfo->m_readBuffer.m_buffer, 
				ClientInfo::IoObject::BufferSize );
			clientInfo->Send(transLength);
		}
		else if ( isWriteCompletion ) {
			clientInfo->m_readBuffer.Init();
			clientInfo->Recv();
		}
		else {
			Error();
		}
	}
}


int main() {
	int r;

	if ( WSAStartup(MAKEWORD(2, 2), &wsaData) != 0 ) {
		Error();
	}

	g_listeningSocket = socket(AF_INET, SOCK_STREAM, 0);
	if ( g_listeningSocket == INVALID_SOCKET ) {
		Error();
	}

	memset(&g_serverAddr, 0, sizeof(g_serverAddr));
	g_serverAddr.sin_family = AF_INET;
	g_serverAddr.sin_addr.S_un.S_addr = INADDR_ANY;
	g_serverAddr.sin_port = htons(Port);
	
	r = bind(g_listeningSocket, (SOCKADDR*)&g_serverAddr, sizeof(g_serverAddr));
	if ( r == SOCKET_ERROR ) {
		Error();
	}

	r = listen(g_listeningSocket, 5);
	if ( r == SOCKET_ERROR ) {
		Error();
	}

	g_iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, ThreadCount);
	if ( g_iocpHandle == NULL ) {
		Error();
	}

	std::thread listenThread(ListenerThread);
	std::thread workerThreads[ThreadCount];
	for ( int i=0; i<ThreadCount; ++i ) {
		workerThreads[i] = std::thread(WorkerThread);
	}

	while ( true ) {
		if ( _getch() == 27 )
			break;
	}

	closesocket(g_listeningSocket);
	g_run = false;

	listenThread.join();
	for ( int i=0; i<ThreadCount; ++i ) {
		workerThreads[i].join();
	}

	CloseHandle(g_iocpHandle);

	return 0;
}
